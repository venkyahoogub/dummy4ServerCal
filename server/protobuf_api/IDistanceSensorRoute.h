#pragma once

#include <neo-calibration-server-api/calibration_server_api.pb.h>
#include <string>

namespace ncs {

using ByteString = std::string;

class IDistanceSensorRoute {
   public:
    virtual ~IDistanceSensorRoute() = default;

    virtual ByteString startRanging(
        const CalibrationApi::DistanceSensorStartRanging& request) = 0;

    virtual ByteString stopRanging(
        const CalibrationApi::DistanceSensorStopRanging& request) = 0;

    virtual ByteString setDeviceSettings(
        const CalibrationApi::SetDistanceSensorDeviceSettings& request) = 0;
};

}  // namespace ncs