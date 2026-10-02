#pragma once

// Dependencies
#include <neo-calibration-server-api/calibration_server_api.pb.h>

#include <atomic>
#include <memory>
#include <thread>

#include "../IStreamContext.h"
#include "ICameraRoute.h"
#include "camera/CameraService.h"

namespace ncs {
/**
 * @class CameraRoute
 * @brief Represents a route to the camera service implementation.
 */
class CameraRoute : public ICameraRoute {
   public:
    explicit CameraRoute(CameraService& cameraService);

    ~CameraRoute();

    /**
     * @brief Works with an expected hardware trigger to pull the latest two
     * images capture off the camera's image queue.
     *
     * @returns A serialized capture that contains data for the left and right
     * images if they were retrieved.
     */
    ByteString createResponse(
        const CalibrationApi::GetSynchronizedCapture& request) override;

    ByteString startStream(IStreamContext& streamContext) override;

    ByteString stopStream(IStreamContext& streamContext) override;

    ByteString setExposure(
        const CalibrationApi::SetExposureSettings& request) override;

    ByteString setRegionOfInterest(
        const CalibrationApi::SetRegionOfInterest& request) override;

    // Wide camera handlers
    ByteString getWideCapture(
        const CalibrationApi::GetWideCapture& request) override;

    ByteString startWideStream(IStreamContext& streamContext) override;

    ByteString stopWideStream(IStreamContext& streamContext) override;

   private:
    CameraService& mCameraService;

    // Stream management for narrow cameras
    std::atomic<bool> mStreamingActive{false};
    std::unique_ptr<std::thread> mStreamThread;

    // Stream management for wide camera
    std::atomic<bool> mWideStreamingActive{false};
    std::unique_ptr<std::thread> mWideStreamThread;

    /**
     * @brief Background thread worker that continuously captures and sends
     * narrow camera frames.
     */
    void streamWorker(IStreamContext& streamContext);

    /**
     * @brief Background thread worker that continuously captures and sends
     * wide camera frames.
     */
    void wideStreamWorker(IStreamContext& streamContext);
};
}  // namespace ncs