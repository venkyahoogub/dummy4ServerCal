#pragma once

#include <neo-calibration-server-api/calibration_server_api.pb.h>

#include "ICameraTriggerRoute.h"
#include "../../services/IHbcService.h"

namespace ncs {

/**
 * @class CameraTriggerRoute
 * @brief Implementation of ICameraTriggerRoute that delegates to IHbcService.
 */
class CameraTriggerRoute : public ICameraTriggerRoute {
   public:
    explicit CameraTriggerRoute(IHbcService& hbcService);

    ~CameraTriggerRoute() = default;

    ByteString setCameraTriggerControls(
        const CalibrationApi::SetCameraTriggerControls& request) override;

   private:
    IHbcService& mHbcService;
};

}  // namespace ncs