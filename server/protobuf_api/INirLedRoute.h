#pragma once

#include <neo-calibration-server-api/calibration_server_api.pb.h>

namespace ncs {

using ByteString = std::string;

/**
 * @class INirLedRoute
 * @brief Interface for NIR LED control operations.
 */
class INirLedRoute {
   public:
    virtual ~INirLedRoute() = default;

    /**
     * @brief Sets the NIR LED controls (top and bottom intensity percentages).
     *
     * @param request The SetNirLedControls request containing intensity values.
     * @returns A serialized NirLedControlsResponse.
     */
    virtual ByteString setNirLedControls(
        const CalibrationApi::SetNirLedControls& request) = 0;
};

}  // namespace ncs