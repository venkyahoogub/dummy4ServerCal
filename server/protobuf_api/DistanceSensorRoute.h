#pragma once

#include <neo-calibration-server-api/calibration_server_api.pb.h>

#include "IDistanceSensorRoute.h"
#include "../../services/IHbcService.h"

namespace ncs {

/**
 * @class DistanceSensorRoute
 * @brief Implementation of IDistanceSensorRoute that delegates to IHbcService.
 */
class DistanceSensorRoute : public IDistanceSensorRoute {
   public:
    explicit DistanceSensorRoute(IHbcService& hbcService);

    ~DistanceSensorRoute() = default;

    ByteString startRanging(
        const CalibrationApi::DistanceSensorStartRanging& request) override;

    ByteString stopRanging(
        const CalibrationApi::DistanceSensorStopRanging& request) override;

    ByteString setDeviceSettings(
        const CalibrationApi::SetDistanceSensorDeviceSettings& request) override;

   private:
    IHbcService& mHbcService;
};

}  // namespace ncs