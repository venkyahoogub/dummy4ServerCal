#pragma once

#include <neo-calibration-server-api/calibration_server_api.pb.h>

namespace ncs {

using ByteString = std::string;

/**
 * @class ICalibrationRoute
 * @brief Interface for calibration control operations.
 */
class ICalibrationRoute {
   public:
    virtual ~ICalibrationRoute() = default;

    /**
     * @brief Starts the calibration process.
     *
     * @param request The StartCalibration request containing irradiance and dac_count.
     * @returns A serialized CalibrationResult.
     */
    virtual ByteString startCalibration(
        const CalibrationApi::StartCalibration& request) = 0;
};

}  // namespace ncs