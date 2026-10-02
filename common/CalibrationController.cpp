#include "CalibrationController.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <cmath>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace ncs {

CalibrationController::CalibrationController(const std::string& logFilePath)
    : logFilePath_(logFilePath) {
    log("=== Calibration Session Started ===");
    log("Timestamp: " + std::string(__DATE__) + " " + std::string(__TIME__));
}

CalibrationController::~CalibrationController() {
    log("=== Calibration Session Ended ===\n");
}

void CalibrationController::log(const std::string& message) {
    // Print to console
    std::cout << message << std::endl;

    // Log to file
    logToFile(message);
}

void CalibrationController::logToFile(const std::string& message) {
    try {
        std::ofstream logFile(logFilePath_, std::ios::app);
        if (logFile.is_open()) {
            auto now = std::chrono::system_clock::now();
            auto time = std::chrono::system_clock::to_time_t(now);
            logFile << "[" << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S")
                    << "] " << message << "\n";
            logFile.close();
        }
    } catch (const std::exception& e) {
        std::cerr << "Error writing to log file: " << e.what() << "\n";
    }
}

// cppcheck-suppress unusedFunction
bool CalibrationController::isStable(double reading, double previousReading, double threshold) {
    if (previousReading == 0.0) return false;  // First reading, not stable yet
    
    double percentChange = std::abs(reading - previousReading) / previousReading;
    return percentChange < threshold;
}

bool CalibrationController::isInTolerance(double reading, uint32_t target) {
    double tolerance_min = target * 0.9;
    double tolerance_max = target * 1.1;
    return reading >= tolerance_min && reading <= tolerance_max;
}

// cppcheck-suppress unusedFunction
CalibrationPoint CalibrationController::findDacForIrradiance(
    uint32_t target_irradiance_mw_per_cm2,
    uint32_t starting_dac_count,
    std::function<double()> ophirMeterReadFn) {

    CalibrationPoint result{};
    result.target_irradiance_mw_per_cm2 = target_irradiance_mw_per_cm2;
    result.success = false;

    uint32_t dac_count = starting_dac_count;
    uint32_t increment_step = 100;
    bool overshoot = false;
    uint32_t attempt = 0;
    const uint32_t max_attempts = 50;
    const uint32_t max_dac = 15000;
    const uint32_t stabilization_duration_sec = 7;

    std::stringstream ss;
    ss << "\n========================================\n";
    ss << "Calibrating irradiance: " << target_irradiance_mw_per_cm2 << " mW/cm²\n";
    ss << "Target tolerance: " << (target_irradiance_mw_per_cm2 * 0.9) << " - "
       << (target_irradiance_mw_per_cm2 * 1.1) << " mW/cm²\n";
    ss << "Starting DAC: " << starting_dac_count << "\n";
    ss << "========================================";
    log(ss.str());

    while (attempt < max_attempts) {
        attempt++;

        std::stringstream attemptLog;
        attemptLog << "\nAttempt " << attempt << "/" << max_attempts << ": DAC=" << dac_count;
        log(attemptLog.str());

        std::stringstream waitLog;
        waitLog << "  Waiting " << stabilization_duration_sec << " seconds for stabilization...";
        log(waitLog.str());

        double reading = 0.0;

        for (int i = 0; i < static_cast<int>(stabilization_duration_sec); ++i) {
            try {
                reading = ophirMeterReadFn();

                std::stringstream readLog;
                readLog << "    [" << (i + 1) << "s] Ophir reading: " << std::fixed
                        << std::setprecision(2) << reading << " mW/cm²";
                log(readLog.str());

                std::this_thread::sleep_for(std::chrono::seconds(1));
            } catch (const std::exception& e) {
                std::stringstream errLog;
                errLog << "    ERROR reading Ophir meter: " << e.what();
                log(errLog.str());
                result.error_message = "Failed to read Ophir meter";
                continue;
            }
        }

        result.measured_irradiance = reading;

        if (isInTolerance(reading, target_irradiance_mw_per_cm2)) {
            std::stringstream successLog;
            successLog << "  ✓ SUCCESS: Reading " << std::fixed << std::setprecision(2)
                       << reading << " is within tolerance!";
            log(successLog.str());

            result.dac_count = dac_count;
            result.success = true;
            return result;
        }

        if (reading < target_irradiance_mw_per_cm2 * 0.9) {
            std::stringstream adjLog;
            adjLog << "  ✗ Too low (" << std::fixed << std::setprecision(2) << reading
                   << " < " << (target_irradiance_mw_per_cm2 * 0.9) << ") → ";

            if (overshoot) {
                increment_step = 10;
                adjLog << "Fine-tuning (step=10)";
            } else {
                adjLog << "Increase (step=" << increment_step << ")";
            }
            log(adjLog.str());

            dac_count += increment_step;
        } else if (reading > target_irradiance_mw_per_cm2 * 1.1) {
            std::stringstream adjLog;
            adjLog << "  ✗ Too high (" << std::fixed << std::setprecision(2) << reading
                   << " > " << (target_irradiance_mw_per_cm2 * 1.1) << ") → ";

            adjLog << "Reduce by 25, switch to fine-tuning";
            log(adjLog.str());

            overshoot = true;
            dac_count -= 25;
            increment_step = 10;
        }

        if (dac_count > max_dac) {
            std::stringstream limitLog;
            limitLog << "  ✗ FAIL: DAC limit (" << max_dac << ") exceeded";
            log(limitLog.str());

            result.error_message = "DAC count exceeded maximum limit";
            return result;
        }
    }

    std::stringstream failLog;
    failLog << "\n✗ FAIL: Max attempts (" << max_attempts << ") exceeded";
    log(failLog.str());

    result.error_message = "Maximum attempts exceeded";
    return result;
}

// cppcheck-suppress unusedFunction
std::string CalibrationController::calibrationTableToJson(
    const std::vector<CalibrationPoint>& calibrationTable) const {

    json table;
    table["calibration_table"] = json::array();

    for (const auto& point : calibrationTable) {
        json entry;
        entry["target_irradiance_mw_per_cm2"] = point.target_irradiance_mw_per_cm2;
        entry["dac_count"] = point.dac_count;
        entry["pd1_count"] = point.pd1_count;
        entry["pd2_count"] = point.pd2_count;
        entry["pd3_count"] = point.pd3_count;
        entry["current_count"] = point.current_count;

        std::stringstream irradianceStream;
        irradianceStream << std::fixed << std::setprecision(2) << point.measured_irradiance;
        entry["measured_irradiance"] = std::stod(irradianceStream.str());

        entry["success"] = point.success;
        if (!point.error_message.empty()) {
            entry["error_message"] = point.error_message;
        }

        table["calibration_table"].push_back(entry);
    }

    return table.dump(4);
}

}  // namespace ncs