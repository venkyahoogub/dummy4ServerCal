#pragma once

#include <neo-calibration-server-api/calibration_server_api.pb.h>
#include <string>

namespace ncs {

using ByteString = std::string;

class IUvDeviceSettingsRoute {
   public:
    virtual ~IUvDeviceSettingsRoute() = default;

    virtual ByteString setUvDeviceSettings(
        const CalibrationApi::SetUvDeviceSettings& request) = 0;

    virtual ByteString getDeviceSettings(
        const CalibrationApi::GetDeviceSettings& request) = 0;
};

}  // namespace ncs