#include "NirLedRoute.h"

#include <iostream>

using namespace ncs;
using namespace CalibrationApi;

NirLedRoute::NirLedRoute(IHbcService& hbcService)
    : mHbcService(hbcService) {}

ByteString NirLedRoute::setNirLedControls(
    const CalibrationApi::SetNirLedControls& request) {
    
    FromServer response;
    auto* nirResponse = response.mutable_nir_led_controls_response();

    try {
        // Extract intensity values from the request
        const uint32_t topPercent = request.controls().top_intensity_percent();
        const uint32_t botPercent = request.controls().bottom_intensity_percent();

        // Validate ranges (0-100%)
        if (topPercent > 100 || botPercent > 100) {
            nirResponse->set_success(false);
            nirResponse->set_message(
                "Intensity percentages must be between 0 and 100");
            return response.SerializeAsString();
        }

        // Call IHbcService to set the NIR LED controls
        mHbcService.setNirLedControls(topPercent, botPercent);

        nirResponse->set_success(true);
        nirResponse->set_message("NIR LED controls set successfully");

    } catch (const std::exception& e) {
        nirResponse->set_success(false);
        nirResponse->set_message(
            std::string("Error setting NIR LED controls: ") + e.what());
        std::cerr << "Exception in setNirLedControls: " << e.what() << "\n";
    }

    return response.SerializeAsString();
}