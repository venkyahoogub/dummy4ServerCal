#pragma once

#include <neo-calibration-server-api/calibration_server_api.pb.h>

namespace ncs {

using ByteString = std::string;

/**
 * @class ICameraTriggerRoute
 * @brief Interface for Camera trigger control operations.
 */
class ICameraTriggerRoute {
   public:
    virtual ~ICameraTriggerRoute() = default;

    /**
     * @brief Sets the camera trigger controls (frequency in hz and duration in ms).
     *
     * @param request The SetCameraTriggerControls request containing frequency and duration.
     * @returns A serialized CameraTriggerControlsResponse.
     */
    virtual ByteString setCameraTriggerControls(
        const CalibrationApi::SetCameraTriggerControls& request) = 0;
};

}  // namespace ncs