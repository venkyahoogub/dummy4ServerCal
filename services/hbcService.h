#pragma once

#include <neo-hbc-api/hbc.pb.h>
#include <neo-calibration-server-api/calibration_server_api.pb.h>

#include <atomic>
#include <functional>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "IHbcService.h"
#include "pbSocketService.h"

namespace ncs {

class HbcService : public IHbcService {
   public:
    using MessageType = Hbc::FromHbc::MsgCase;

    using RxHandler = std::function<void(MessageType, const Hbc::FromHbc&)>;

    explicit HbcService(RxHandler handler);
    ~HbcService();

    void getAbout() override;
    Hbc::Status getStatus() override;
    void getControls() override;
    void setNirLedControls(uint32_t topPercent, uint32_t botPercent) override;
    void setCameraTriggerControls(uint32_t frequency_hz, uint32_t duration_ms) override;
    CalibrationApi::CalibrationTableEntry startCalibration(
        uint32_t irradiance_mw_per_cm2, uint32_t dac_count) override;
    void homeMotors() override;
    void startUvDiagnostic(uint32_t dac_counts, uint32_t duration_ms) override;
    void cancelUvDiagnostic() override;
    
    // Distance Sensor Methods
    void distanceSensorStartRanging() override;
    void distanceSensorStopRanging() override;
    void setDistanceSensorDeviceSettings(float gain, float offset) override;
    
    // UV Device Settings Methods
    void setUvDeviceSettings(uint32_t pd1_gain_count,
                             uint32_t pd2_gain_count,
                             uint32_t pd3_gain_count,
                             uint32_t pi_gain_count) override;

    // Get Device Settings   
    Hbc::DeviceSettings getDeviceSettings() override;

   private:
    void streamReader();

   private:
    pbSocketService socket_;
    RxHandler rxHandler_;
    std::atomic<bool> running_;
    std::thread receiver_;
    
    // For calibration result handling
    std::mutex calibrationMutex_;
    std::condition_variable calibrationCV_;
    CalibrationApi::CalibrationTableEntry calibrationResult_;
    bool calibrationReady_;
    
    // For status result handling
    std::mutex statusMutex_;
    std::condition_variable statusCV_;
    Hbc::Status currentStatus_;
    bool statusReady_;
    
    // For device settings result handling
    std::mutex deviceSettingsMutex_;
    std::condition_variable deviceSettingsCV_;
    Hbc::DeviceSettings currentDeviceSettings_;
    bool deviceSettingsReady_;

    // For UV device settings confirmation
    std::mutex uvSettingsMutex_;
    std::condition_variable uvSettingsCV_;
    bool uvSettingsApplied_;
};

}  // namespace ncs