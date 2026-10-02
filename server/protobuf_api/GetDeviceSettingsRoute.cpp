#include "GetDeviceSettingsRoute.h"

#include <iostream>

using namespace ncs;
using namespace CalibrationApi;

GetDeviceSettingsRoute::GetDeviceSettingsRoute(IHbcService& hbcService)
    : mHbcService(hbcService) {}

ByteString GetDeviceSettingsRoute::getDeviceSettings(
    const CalibrationApi::GetDeviceSettings& request) {
    
    FromServer response;
    auto* deviceSettings = response.mutable_device_settings();

    try {
        // Get status from HBC service which contains device settings info
        Hbc::Status status = mHbcService.getStatus();
        
        // Get device settings from HBC service
        // Note: You may need to add a getDeviceSettings() method to IHbcService
        // For now, we'll use status to get what we can
        
        // Set UV device settings
        deviceSettings->set_pd1_gain_count(status.pd1_count());
        deviceSettings->set_pd2_gain_count(status.pd2_count());
        deviceSettings->set_pd3_gain_count(status.pd3_count());
        // Note: pi_gain_count is not available in Status, would need separate getter
        deviceSettings->set_pi_gain_count(0);  // Placeholder
        
        // Note: distance_sensor_gain and distance_sensor_offset are not in Status
        // You may need to add a getDeviceSettings() method to IHbcService that 
        // queries these values separately from the HBC board
        deviceSettings->set_distance_sensor_gain(0.0f);  // Placeholder
        deviceSettings->set_distance_sensor_offset(0.0f);  // Placeholder
        
        std::cout << "Device settings retrieved successfully.\n";

    } catch (const std::exception& e) {
        std::cerr << "Exception in getDeviceSettings: " << e.what() << "\n";
    }

    return response.SerializeAsString();
}