#include "UvDeviceSettingsRoute.h"

#include <iostream>

using namespace ncs;
using namespace CalibrationApi;

UvDeviceSettingsRoute::UvDeviceSettingsRoute(IHbcService& hbcService)
    : mHbcService(hbcService) {}

ByteString UvDeviceSettingsRoute::setUvDeviceSettings(
    const CalibrationApi::SetUvDeviceSettings& request) {
    
    FromServer response;

    try {
        // Extract UV device settings from the request
        const uint32_t pd1GainCount = request.pd1_gain_count();
        const uint32_t pd2GainCount = request.pd2_gain_count();
        const uint32_t pd3GainCount = request.pd3_gain_count();
        const uint32_t piGainCount = request.pi_gain_count();

        // Call IHbcService to set UV device settings
        mHbcService.setUvDeviceSettings(pd1GainCount, pd2GainCount, 
                                        pd3GainCount, piGainCount);

    } catch (const std::exception& e) {
        std::cerr << "Exception in setUvDeviceSettings: " << e.what() << "\n";
    }

    return response.SerializeAsString();
}

ByteString UvDeviceSettingsRoute::getDeviceSettings(
    const CalibrationApi::GetDeviceSettings& request) {
    
    FromServer response;

    try {
        // Call IHbcService to retrieve device settings
        // Note: This is a placeholder - you need to implement getDeviceSettings in IHbcService
        // For now, we'll return an empty response
        // cppcheck-suppress unreadVariable
        auto* deviceSettingsResponse = response.mutable_device_settings();
        // Set default values or retrieve from service once implemented
        
    } catch (const std::exception& e) {
        std::cerr << "Exception in getDeviceSettings: " << e.what() << "\n";
    }

    return response.SerializeAsString();
}