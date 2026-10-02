#pragma once

#include <neo-calibration-server-api/calibration_server_api.pb.h>

#include "IUvDiagnosticRoute.h"
#include "../../services/IHbcService.h"

namespace ncs {

/**
 * @class UvDiagnosticRoute
 * @brief Implementation of IUvDiagnosticRoute that delegates to IHbcService.
 */
class UvDiagnosticRoute : public IUvDiagnosticRoute {
   public:
    explicit UvDiagnosticRoute(IHbcService& hbcService);

    ~UvDiagnosticRoute() = default;

    ByteString startUvDiagnostic(
        const CalibrationApi::StartUvDiagnostic& request) override;

    ByteString cancelUvDiagnostic(
        const CalibrationApi::CancelUvDiagnostic& request) override;

   private:
    IHbcService& mHbcService;
};

}  // namespace ncs