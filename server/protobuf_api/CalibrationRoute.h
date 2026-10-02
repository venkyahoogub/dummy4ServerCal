#pragma once

#include <neo-calibration-server-api/calibration_server_api.pb.h>

#include "ICalibrationRoute.h"
#include "../../services/IHbcService.h"

namespace ncs {

/**
 * @class CalibrationRoute
 * @brief Implementation of ICalibrationRoute that delegates to IHbcService.
 */
class CalibrationRoute : public ICalibrationRoute {
   public:
    explicit CalibrationRoute(IHbcService& hbcService);

    ~CalibrationRoute() = default;

    ByteString startCalibration(
        const CalibrationApi::StartCalibration& request) override;

   private:
    IHbcService& mHbcService;
    static constexpr uint32_t FIXED_IRRADIANCE_MW_PER_CM2 = 30;
};

}  // namespace ncs