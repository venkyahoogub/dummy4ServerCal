#pragma once

#include <neo-calibration-server-api/calibration_server_api.pb.h>
#include <string>

namespace ncs {

using ByteString = std::string;

/**
 * @class IGetDeviceSettingsRoute
 * @brief Interface for handling GetDeviceSettings requests.
 */
class IGetDeviceSettingsRoute {
   public:
    virtual ~IGetDeviceSettingsRoute() = default;

    /**
     * @brief Get current device settings (UV and distance sensor settings).
     * @param request GetDeviceSettings request message
     * @return Serialized DeviceSettings response
     */
    virtual ByteString getDeviceSettings(
        const CalibrationApi::GetDeviceSettings& request) = 0;
};

}  // namespace ncs