#pragma once

#include "IDmdRoute.h"
#include "neo-dmd-driver/src/iProjector.h"

namespace ncs {
class DmdRoute : public IDmdRoute {
   public:
    explicit DmdRoute(IProjector& projector) : mProjector(projector) {}

    ByteString createResponse(
        const CalibrationApi::CalibrateDmd& calibrateRequest) override;

   private:
    IProjector& mProjector;
};
}  // namespace ncs