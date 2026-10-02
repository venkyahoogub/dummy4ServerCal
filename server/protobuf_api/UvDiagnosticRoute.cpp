#include "UvDiagnosticRoute.h"

#include <iostream>

using namespace ncs;
using namespace CalibrationApi;

UvDiagnosticRoute::UvDiagnosticRoute(IHbcService& hbcService)
    : mHbcService(hbcService) {}

ByteString UvDiagnosticRoute::startUvDiagnostic(
    const CalibrationApi::StartUvDiagnostic& request) {
    
    FromServer response;
    auto* uvResponse = response.mutable_uv_diagnostic_response();

    try {
        // Extract parameters from the request
        const uint32_t dacCounts = request.dac_counts();
        const uint32_t durationMs = request.duration_ms();

        // Validate parameters
        if (dacCounts == 0) {
            uvResponse->set_success(false);
            uvResponse->set_message("DAC count must be greater than 0");
            return response.SerializeAsString();
        }

        if (durationMs == 0) {
            uvResponse->set_success(false);
            uvResponse->set_message("Duration must be greater than 0");
            return response.SerializeAsString();
        }

        // Call IHbcService to start UV diagnostic
        mHbcService.startUvDiagnostic(dacCounts, durationMs);

        uvResponse->set_success(true);
        uvResponse->set_message("UV diagnostic started successfully");

    } catch (const std::exception& e) {
        uvResponse->set_success(false);
        uvResponse->set_message(
            std::string("Error starting UV diagnostic: ") + e.what());
        std::cerr << "Exception in startUvDiagnostic: " << e.what() << "\n";
    }

    return response.SerializeAsString();
}

ByteString UvDiagnosticRoute::cancelUvDiagnostic(
    const CalibrationApi::CancelUvDiagnostic& request) {
    
    FromServer response;
    auto* uvResponse = response.mutable_uv_diagnostic_response();

    try {
        // Call IHbcService to cancel UV diagnostic
        mHbcService.cancelUvDiagnostic();

        uvResponse->set_success(true);
        uvResponse->set_message("UV diagnostic cancelled successfully");

    } catch (const std::exception& e) {
        uvResponse->set_success(false);
        uvResponse->set_message(
            std::string("Error cancelling UV diagnostic: ") + e.what());
        std::cerr << "Exception in cancelUvDiagnostic: " << e.what() << "\n";
    }

    return response.SerializeAsString();
}