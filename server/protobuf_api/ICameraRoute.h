#pragma once

// Dependencies
#include "../IStreamContext.h"
#include "neo-calibration-server-api/calibration_server_api.pb.h"

namespace ncs {
/**
 * @class ICameraRoute
 * @brief Represents a route to the camera service implementation.
 */
class ICameraRoute {
   public:
    virtual ~ICameraRoute() = default;

    virtual ByteString createResponse(
        const CalibrationApi::GetSynchronizedCapture& request) = 0;

    virtual ByteString startStream(IStreamContext& streamContext) = 0;

    virtual ByteString stopStream(IStreamContext& streamContext) = 0;

    virtual ByteString setExposure(
        const CalibrationApi::SetExposureSettings& request) = 0;

    virtual ByteString setRegionOfInterest(
        const CalibrationApi::SetRegionOfInterest& request) = 0;

    // Wide camera handlers
    virtual ByteString getWideCapture(
        const CalibrationApi::GetWideCapture& request) = 0;

    virtual ByteString startWideStream(IStreamContext& streamContext) = 0;

    virtual ByteString stopWideStream(IStreamContext& streamContext) = 0;
};
}  // namespace ncs