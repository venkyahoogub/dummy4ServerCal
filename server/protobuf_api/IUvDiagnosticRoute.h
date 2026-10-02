#pragma once

#include <neo-calibration-server-api/calibration_server_api.pb.h>

namespace ncs {

using ByteString = std::string;

/**
 * @class IUvDiagnosticRoute
 * @brief Interface for UV diagnostic control operations.
 */
class IUvDiagnosticRoute {
   public:
    virtual ~IUvDiagnosticRoute() = default;

    /**
     * @brief Starts the UV diagnostic.
     *
     * @param request The StartUvDiagnostic request containing dac_counts and duration_ms.
     * @returns A serialized UvDiagnosticResponse.
     */
    virtual ByteString startUvDiagnostic(
        const CalibrationApi::StartUvDiagnostic& request) = 0;

    /**
     * @brief Cancels the UV diagnostic.
     *
     * @param request The CancelUvDiagnostic request.
     * @returns A serialized UvDiagnosticResponse.
     */
    virtual ByteString cancelUvDiagnostic(
        const CalibrationApi::CancelUvDiagnostic& request) = 0;
};

}  // namespace ncs