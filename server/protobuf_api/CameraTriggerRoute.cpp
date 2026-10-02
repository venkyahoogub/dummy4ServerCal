#include "CameraTriggerRoute.h"

#include <iostream>

using namespace ncs;
using namespace CalibrationApi;

CameraTriggerRoute::CameraTriggerRoute(IHbcService& hbcService)
    : mHbcService(hbcService) {}

ByteString CameraTriggerRoute::setCameraTriggerControls(
    const CalibrationApi::SetCameraTriggerControls& request) {
    
    FromServer response;
    auto* cameraTriggerResponse = response.mutable_camera_trigger_controls_response();

    try {
        // Extract control values from the request
        const uint32_t frequencyHz = request.controls().frequency_hz();
        const uint32_t durationMs = request.controls().duration_ms();

        // Validate ranges
        if (frequencyHz == 0) {
            cameraTriggerResponse->set_success(false);
            cameraTriggerResponse->set_message("Frequency must be greater than 0");
            return response.SerializeAsString();
        }

        if (durationMs == 0) {
            cameraTriggerResponse->set_success(false);
            cameraTriggerResponse->set_message("Duration must be greater than 0");
            return response.SerializeAsString();
        }

        // Call IHbcService to set camera trigger controls
        mHbcService.setCameraTriggerControls(frequencyHz, durationMs);

        cameraTriggerResponse->set_success(true);
        cameraTriggerResponse->set_message("Camera Trigger controls set successfully");

    } catch (const std::exception& e) {
        cameraTriggerResponse->set_success(false);
        cameraTriggerResponse->set_message(
            std::string("Error setting camera trigger controls: ") + e.what());
        std::cerr << "Exception in setCameraTriggerControls: " << e.what() << "\n";
    }

    return response.SerializeAsString();
}