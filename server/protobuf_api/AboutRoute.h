#pragma once

// Dependencies
#include "../IVersionService.h"
#include "IAboutRoute.h"
#include "neo-calibration-server-api/calibration_server_api.pb.h"
namespace ncs {
/**
 * @class AboutRoute
 * @brief Handles the routing and response generation for "About" information
 * requests.
 *
 * This class implements the IAboutRoute interface to provide specific details
 * regarding the application's version, build, or status.
 */
class AboutRoute : public IAboutRoute {
   public:
    /**
     * @brief Constructs a new AboutRoute object.
     */
    explicit AboutRoute(const IVersionService& versionService);

    /**
     * @brief Provides a response to the GetAbout request.
     */
    ByteString createResponse(
        const CalibrationApi::GetAbout& aboutRequest) override;

   private:
    const IVersionService& mVersionService;
};
}  // namespace ncs