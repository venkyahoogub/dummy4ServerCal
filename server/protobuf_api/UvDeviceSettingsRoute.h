#pragma once

#include <neo-calibration-server-api/calibration_server_api.pb.h>

#include "IUvDeviceSettingsRoute.h"
#include "../../services/IHbcService.h"

namespace ncs {

/**
 * @class UvDeviceSettingsRoute
 * @brief Implementation of IUvDeviceSettingsRoute that delegates to IHbcService.
 */
class UvDeviceSettingsRoute : public IUvDeviceSettingsRoute {
   public:
    explicit UvDeviceSettingsRoute(IHbcService& hbcService);

    ~UvDeviceSettingsRoute() = default;

    ByteString setUvDeviceSettings(
        const CalibrationApi::SetUvDeviceSettings& request) override;

    ByteString getDeviceSettings(
        const CalibrationApi::GetDeviceSettings& request) override;

   private:
    IHbcService& mHbcService;
};

}  // namespace ncs