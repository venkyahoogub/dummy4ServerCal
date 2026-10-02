#pragma once

// Dependencies
#include "common/SerializationUtils.h"
#include "neo-calibration-server-api/calibration_server_api.pb.h"

namespace ncs {
/**
 * @class IAboutRoute
 * @brief Interface for the "About" service route.
 *
 * This interface specializes @ref IServiceRoute for the @c
 * CalibrationApi::GetAbout message type. It is responsible for providing
 * service metadata, such as version information, build dates, and system
 * status, to requesting clients.
 *
 * @see IServiceRoute
 */
class IAboutRoute {
   public:
    virtual ~IAboutRoute() = default;

    virtual ByteString createResponse(
        const CalibrationApi::GetAbout& aboutRequest) = 0;
};
}  // namespace ncs