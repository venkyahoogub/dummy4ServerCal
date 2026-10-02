#pragma once
#ifdef DEBUG
#include <cstdint>

#include "IHbcService.h"

namespace ncs {
class MockHbcService : public IHbcService {
   public:
    explicit MockHbcService() {}
    // cppcheck-suppress unusedFunction
    void getAbout() override {}
    // cppcheck-suppress unusedFunction
    Hbc::Status getStatus() override { return Hbc::Status(); }
    // cppcheck-suppress unusedFunction
    void getControls() override {}
    void setNirLedControls(std::uint32_t topPercent,
                           std::uint32_t botPercent) override {}
    void setCameraTriggerControls(std::uint32_t frequency_hz,
                                  std::uint32_t duration_ms) override {}
    CalibrationApi::CalibrationTableEntry startCalibration(
        std::uint32_t irradiance_mw_per_cm2, std::uint32_t dac_count) override {
        return CalibrationApi::CalibrationTableEntry();
    }
    void homeMotors() override {}
    void startUvDiagnostic(std::uint32_t dac_counts,
                           std::uint32_t duration_ms) override {}
    void cancelUvDiagnostic() override {}
    
    // Distance Sensor Methods
    void distanceSensorStartRanging() override {}
    void distanceSensorStopRanging() override {}
    void setDistanceSensorDeviceSettings(float gain, float offset) override {}
    
    // UV Device Settings Methods
    void setUvDeviceSettings(std::uint32_t pd1_gain_count,
                             std::uint32_t pd2_gain_count,
                             std::uint32_t pd3_gain_count,
                             std::uint32_t pi_gain_count) override {}
    
    // Get Device Settings HBC
    Hbc::DeviceSettings getDeviceSettings() override {
        return Hbc::DeviceSettings();
    }
};
}  // namespace ncs
#endif