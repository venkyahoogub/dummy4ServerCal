#pragma once

#include "common/SerializationUtils.h"
#include "neo-calibration-server-api/calibration_server_api.pb.h"

namespace ncs {
class IDmdRoute {
   public:
    virtual ~IDmdRoute() = default;

    virtual ByteString createResponse(
        const CalibrationApi::CalibrateDmd& calibrateRequest) = 0;
};
}  // namespace ncs