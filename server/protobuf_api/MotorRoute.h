#pragma once

#include <neo-calibration-server-api/calibration_server_api.pb.h>

#include "IMotorRoute.h"
#include "../../services/IHbcService.h"

namespace ncs {

/**
 * @class MotorRoute
 * @brief Implementation of IMotorRoute that delegates to IHbcService.
 */
class MotorRoute : public IMotorRoute {
   public:
    explicit MotorRoute(IHbcService& hbcService);

    ~MotorRoute() = default;

    ByteString homeMotors(
        const CalibrationApi::HomeMotors& request) override;

   private:
    IHbcService& mHbcService;
};

}  // namespace ncs