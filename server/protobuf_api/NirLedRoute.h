#pragma once

#include <neo-calibration-server-api/calibration_server_api.pb.h>

#include "INirLedRoute.h"
#include "../../services/IHbcService.h"

namespace ncs {

/**
 * @class NirLedRoute
 * @brief Implementation of INirLedRoute that delegates to IHbcService.
 */
class NirLedRoute : public INirLedRoute {
   public:
    explicit NirLedRoute(IHbcService& hbcService);

    ~NirLedRoute() = default;

    ByteString setNirLedControls(
        const CalibrationApi::SetNirLedControls& request) override;

   private:
    IHbcService& mHbcService;
};

}  // namespace ncs