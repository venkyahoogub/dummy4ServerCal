#pragma once

#include <neo-calibration-server-api/calibration_server_api.pb.h>
#include <neo-hbc-api/hbc.pb.h>

#include "IGetDeviceSettingsRoute.h"
#include "../../services/IHbcService.h"

namespace ncs {

/**
 * @class GetDeviceSettingsRoute
 * @brief Implementation of IGetDeviceSettingsRoute that delegates to IHbcService.
 */
class GetDeviceSettingsRoute : public IGetDeviceSettingsRoute {
   public:
    explicit GetDeviceSettingsRoute(IHbcService& hbcService);

    ~GetDeviceSettingsRoute() = default;

    ByteString getDeviceSettings(
        const CalibrationApi::GetDeviceSettings& request) override;

   private:
    IHbcService& mHbcService;
};

}  // namespace ncs