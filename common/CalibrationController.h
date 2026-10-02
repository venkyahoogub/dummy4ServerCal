#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include <functional>

namespace ncs {

struct CalibrationPoint {
    uint32_t target_irradiance_mw_per_cm2 = 0;
    uint32_t dac_count = 0;
    uint32_t pd1_count = 0;
    uint32_t pd2_count = 0;
    uint32_t pd3_count = 0;
    uint32_t current_count = 0;
    double measured_irradiance = 0.0;
    bool success = false;
    std::string error_message;
};

class CalibrationController {
   public:
    explicit CalibrationController(const std::string& logFilePath);
    ~CalibrationController();

    // Find optimal DAC count for a target irradiance using Ophir meter feedback
    CalibrationPoint findDacForIrradiance(
        uint32_t target_irradiance_mw_per_cm2,
        uint32_t starting_dac_count,
        std::function<double()> ophirMeterReadFn);

    // Serialize calibration table to JSON
    std::string calibrationTableToJson(
        const std::vector<CalibrationPoint>& calibrationTable) const;

    // Log message to console and file
    void log(const std::string& message);

   private:
    std::string logFilePath_;

    void logToFile(const std::string& message);
    bool isStable(double reading, double previousReading, double threshold = 0.02); // cppcheck-suppress unusedPrivateFunction
    bool isInTolerance(double reading, uint32_t target);
};

}  // namespace ncs