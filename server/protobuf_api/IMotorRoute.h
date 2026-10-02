#pragma once

#include <neo-calibration-server-api/calibration_server_api.pb.h>

namespace ncs {

using ByteString = std::string;

/**
 * @class IMotorRoute
 * @brief Interface for motor control operations.
 */
class IMotorRoute {
   public:
    virtual ~IMotorRoute() = default;

    /**
     * @brief Homes the motors.
     *
     * @param request The HomeMotors request.
     * @returns A serialized MotorsHomed response.
     */
    virtual ByteString homeMotors(
        const CalibrationApi::HomeMotors& request) = 0;
};

}  // namespace ncs