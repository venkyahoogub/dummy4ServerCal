#include "hbcService.h"
#include <iostream>
#include <chrono>

#define HBC_PORT 9760
#define HBC_IP "10.10.10.2"

namespace ncs {

HbcService::HbcService(RxHandler handler) try
    : socket_(HBC_IP, HBC_PORT),
      rxHandler_(handler),
      running_(true),
      receiver_(&HbcService::streamReader, this),
      calibrationReady_(false),
      statusReady_(false),
      deviceSettingsReady_(false),
      uvSettingsApplied_(false) {
} catch (const std::exception& e) {
    std::cerr << "Failed to create socket: " << e.what() << std::endl;
    throw;
} catch (...) {
    std::cerr << "Unknown error while creating socket" << std::endl;
    throw;
}

HbcService::~HbcService() {
    running_ = false;
    if (receiver_.joinable()) {
        receiver_.join();
    }
}

void HbcService::getAbout() {
    Hbc::ToHbc msg;
    msg.mutable_get_about();

    std::string encoded;
    msg.SerializeToString(&encoded);

    socket_.sendMessage(encoded);
}

Hbc::Status HbcService::getStatus() {
    Hbc::ToHbc msg;
    msg.mutable_get_status();

    std::string encoded;
    msg.SerializeToString(&encoded);

    socket_.sendMessage(encoded);

    // Wait for status result from HBC board (with 5 second timeout)
    {
        std::unique_lock<std::mutex> lock(statusMutex_);
        statusReady_ = false;
        
        if (statusCV_.wait_for(lock, std::chrono::seconds(5),
                               [this] { return statusReady_; })) {
            std::cout << "Status received from HBC board. Distance: " 
                      << currentStatus_.distance_to_target_mm() << " mm\n";
            return currentStatus_;
        } else {
            std::cerr << "Timeout waiting for status from HBC board\n";
            // Return empty status on timeout
            return Hbc::Status();
        }
    }
}

void HbcService::setNirLedControls(uint32_t topPercent, uint32_t botPercent) {
    Hbc::ToHbc msg;

    auto* ctrl = msg.mutable_set_nir_led_controls();
    ctrl->set_top_intensity_percent(topPercent);
    ctrl->set_bot_intensity_percent(botPercent);

    std::string encoded;
    msg.SerializeToString(&encoded);

    socket_.sendMessage(encoded);
}

void HbcService::setCameraTriggerControls(uint32_t frequency_hz, uint32_t duration_ms) {
    Hbc::ToHbc msg;

    auto* ctrl = msg.mutable_set_camera_trigger_controls();
    ctrl->set_frequency_hz(frequency_hz);
    ctrl->set_duration_ms(duration_ms);

    std::string encoded;
    msg.SerializeToString(&encoded);

    socket_.sendMessage(encoded);
}

CalibrationApi::CalibrationTableEntry HbcService::startCalibration(
    uint32_t irradiance_mw_per_cm2, uint32_t dac_count) {
    Hbc::ToHbc msg;

    auto* calib = msg.mutable_start_calibration();
    calib->set_irradiance_mw_per_cm2(irradiance_mw_per_cm2);
    calib->set_dac_count(dac_count);

    std::string encoded;
    msg.SerializeToString(&encoded);

    socket_.sendMessage(encoded);

    // Wait for calibration result from HBC board (with 10 second timeout)
    {
        std::unique_lock<std::mutex> lock(calibrationMutex_);
        calibrationReady_ = false;
        
        if (calibrationCV_.wait_for(lock, std::chrono::seconds(10),
                                    [this] { return calibrationReady_; })) {
            std::cout << "Calibration result received from HBC board\n";
            return calibrationResult_;
        } else {
            std::cerr << "Timeout waiting for calibration result from HBC board\n";
            // Return default entry on timeout
            CalibrationApi::CalibrationTableEntry entry;
            entry.set_irradiance_mw_per_cm2(irradiance_mw_per_cm2);
            entry.set_dac_count(dac_count);
            entry.set_pd1_count(0);
            entry.set_pd2_count(0);
            entry.set_pd3_count(0);
            entry.set_current_count(0);
            return entry;
        }
    }
}

void HbcService::homeMotors() {
    Hbc::ToHbc msg;
    msg.mutable_home_motors();

    std::string encoded;
    msg.SerializeToString(&encoded);

    socket_.sendMessage(encoded);
}

void HbcService::startUvDiagnostic(uint32_t dac_counts, uint32_t duration_ms) {
    Hbc::ToHbc msg;

    auto* uvDiag = msg.mutable_start_uv_diagnostic();
    uvDiag->set_dac_counts(dac_counts);
    uvDiag->set_duration_ms(duration_ms);

    std::string encoded;
    msg.SerializeToString(&encoded);
    
    socket_.sendMessage(encoded);
}

void HbcService::cancelUvDiagnostic() {
    Hbc::ToHbc msg;
    msg.mutable_cancel_uv_diagnostic();

    std::string encoded;
    msg.SerializeToString(&encoded);

    socket_.sendMessage(encoded);
}

void HbcService::getControls() {
    Hbc::ToHbc msg;
    msg.mutable_get_controls();

    std::string encoded;
    msg.SerializeToString(&encoded);

    socket_.sendMessage(encoded);
}

void HbcService::streamReader() {
    while (running_) {
        auto data = socket_.receiveMessage();

        Hbc::FromHbc from;
        if (!from.ParseFromArray(data.data(), data.size())) {
            std::cerr << "Failed to parse HBC message\n";
            continue;
        }

        // Check if this is a status message
        if (from.msg_case() == Hbc::FromHbc::kStatus) {
            std::cout << "Status received from HBC board\n";
            {
                std::unique_lock<std::mutex> lock(statusMutex_);
                currentStatus_ = from.status();
                statusReady_ = true;
            }
            statusCV_.notify_one();
        }

        // Check if this is a device settings message
        if (from.msg_case() == Hbc::FromHbc::kDeviceSettings) {
            std::cout << "Device settings received from HBC board\n";
            {
                std::unique_lock<std::mutex> lock(deviceSettingsMutex_);
                currentDeviceSettings_ = from.device_settings();
                deviceSettingsReady_ = true;
            }
            deviceSettingsCV_.notify_one();
        }

        // Check if this is a calibration result message
        if (from.msg_case() == Hbc::FromHbc::kCalibrationResult) {
            std::cout << "Calibration result received from HBC board\n";
            {
                std::unique_lock<std::mutex> lock(calibrationMutex_);
                
                // Extract the calibration result entry from the nested structure
                const auto& calibResult = from.calibration_result();
                const auto& entry = calibResult.entry();
                
                calibrationResult_.set_irradiance_mw_per_cm2(entry.irradiance_mw_per_cm2());
                calibrationResult_.set_dac_count(entry.dac_count());
                calibrationResult_.set_pd1_count(entry.pd1_count());
                calibrationResult_.set_pd2_count(entry.pd2_count());
                calibrationResult_.set_pd3_count(entry.pd3_count());
                calibrationResult_.set_current_count(entry.current_count());
                
                calibrationReady_ = true;
            }
            calibrationCV_.notify_one();
        }

        // Call the user's rx handler
        rxHandler_(from.msg_case(), from);
    }
}

void HbcService::distanceSensorStartRanging() {
    Hbc::ToHbc msg;
    msg.mutable_distance_sensor_start_ranging();

    std::string encoded;
    msg.SerializeToString(&encoded);

    socket_.sendMessage(encoded);
}

void HbcService::distanceSensorStopRanging() {
    Hbc::ToHbc msg;
    msg.mutable_distance_sensor_stop_ranging();

    std::string encoded;
    msg.SerializeToString(&encoded);

    socket_.sendMessage(encoded);
}

void HbcService::setDistanceSensorDeviceSettings(float gain, float offset) {
    Hbc::ToHbc msg;

    auto* settings = msg.mutable_set_distance_sensor_device_settings();
    settings->set_gain(gain);
    settings->set_offset(offset);

    std::string encoded;
    msg.SerializeToString(&encoded);

    socket_.sendMessage(encoded);
}

void HbcService::setUvDeviceSettings(uint32_t pd1_gain_count,
                                     uint32_t pd2_gain_count,
                                     uint32_t pd3_gain_count,
                                     uint32_t pi_gain_count) {
    Hbc::ToHbc msg;

    auto* uvSettings = msg.mutable_set_uv_device_settings();
    uvSettings->set_pd1_gain_count(pd1_gain_count);
    uvSettings->set_pd2_gain_count(pd2_gain_count);
    uvSettings->set_pd3_gain_count(pd3_gain_count);
    uvSettings->set_pi_gain_count(pi_gain_count);

    std::string encoded;
    msg.SerializeToString(&encoded);

    socket_.sendMessage(encoded);
    
    // Wait for settings to be applied (with 5 second timeout)
    {
        std::unique_lock<std::mutex> lock(uvSettingsMutex_);
        uvSettingsApplied_ = false;
        
        if (uvSettingsCV_.wait_for(lock, std::chrono::seconds(5),
                                   [this] { return uvSettingsApplied_; })) {
            std::cout << "UV device settings applied successfully\n";
        } else {
            std::cerr << "Timeout waiting for UV device settings to be applied\n";
        }
    }
}

Hbc::DeviceSettings HbcService::getDeviceSettings() {
    Hbc::ToHbc msg;
    msg.mutable_get_device_settings();

    std::string encoded;
    msg.SerializeToString(&encoded);

    socket_.sendMessage(encoded);

    // Wait for device settings result from HBC board (with 5 second timeout)
    {
        std::unique_lock<std::mutex> lock(deviceSettingsMutex_);
        deviceSettingsReady_ = false;
        
        if (deviceSettingsCV_.wait_for(lock, std::chrono::seconds(5),
                                       [this] { return deviceSettingsReady_; })) {
            std::cout << "Device settings received from HBC board\n";
            return currentDeviceSettings_;
        } else {
            std::cerr << "Timeout waiting for device settings from HBC board\n";
            return Hbc::DeviceSettings();
        }
    }
}

}  // namespace ncs