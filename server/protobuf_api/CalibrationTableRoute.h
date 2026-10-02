#pragma once

#include <neo-calibration-server-api/calibration_server_api.pb.h>

#include "ICalibrationTableRoute.h"
#include "../../services/IHbcService.h"

namespace ncs {

/**
 * @class CalibrationTableRoute
 * @brief Implementation for uploading calibration table to server.
 */
class CalibrationTableRoute : public ICalibrationTableRoute {
   public:
    explicit CalibrationTableRoute(IHbcService& hbcService);

    ~CalibrationTableRoute() = default;

    ByteString uploadCalibrationTable(
        const CalibrationApi::UploadCalibrationTable& request) override;

   private:
    IHbcService& mHbcService;
};

}  // namespace ncs