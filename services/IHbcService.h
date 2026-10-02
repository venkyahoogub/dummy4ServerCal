#pragma once

#include <neo-calibration-server-api/calibration_server_api.pb.h>
#include <neo-hbc-api/hbc.pb.h>
#include <cstdint>

namespace ncs {
class IHbcService {
   public:
    virtual ~IHbcService() = default;

    virtual void getAbout() = 0;
    virtual Hbc::Status getStatus() = 0;
    virtual void getControls() = 0;
    virtual void setNirLedControls(std::uint32_t topPercent,
                                   std::uint32_t botPercent) = 0;
    virtual void setCameraTriggerControls(std::uint32_t frequency_hz, 
                                  std::uint32_t duration_ms) = 0;
    virtual CalibrationApi::CalibrationTableEntry startCalibration(
                          std::uint32_t irradiance_mw_per_cm2, 
                          std::uint32_t dac_count) = 0;
    virtual void homeMotors() = 0;
    virtual void startUvDiagnostic(std::uint32_t dac_counts, 
                                   std::uint32_t duration_ms) = 0;
    virtual void cancelUvDiagnostic() = 0;
    
    // Distance Sensor Methods
    virtual void distanceSensorStartRanging() = 0;
    virtual void distanceSensorStopRanging() = 0;
    virtual void setDistanceSensorDeviceSettings(float gain, float offset) = 0;
    
    // UV Device Settings Methods
    virtual void setUvDeviceSettings(std::uint32_t pd1_gain_count,
                                     std::uint32_t pd2_gain_count,
                                     std::uint32_t pd3_gain_count,
                                     std::uint32_t pi_gain_count) = 0;
    // Get UV Device Settings    
    virtual Hbc::DeviceSettings getDeviceSettings() = 0;
};
}  // namespace ncs