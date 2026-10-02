#pragma once

#include <neo-calibration-server-api/calibration_server_api.pb.h>

namespace ncs {

using ByteString = std::string;

/**
 * @class ICalibrationTableRoute
 * @brief Interface for calibration table operations.
 */
class ICalibrationTableRoute {
   public:
    virtual ~ICalibrationTableRoute() = default;

    /**
     * @brief Uploads a calibration table to the server.
     *
     * @param request The UploadCalibrationTable request.
     * @returns A serialized UploadCalibrationTableResponse.
     */
    virtual ByteString uploadCalibrationTable(
        const CalibrationApi::UploadCalibrationTable& request) = 0;
};

}  // namespace ncs