#include "MotorRoute.h"

#include <iostream>

using namespace ncs;
using namespace CalibrationApi;

MotorRoute::MotorRoute(IHbcService& hbcService)
    : mHbcService(hbcService) {}

ByteString MotorRoute::homeMotors(
    const CalibrationApi::HomeMotors& request) {

    (void)request;  // suppress unused parameter warning/checkers if enabled

    FromServer response;
    response.mutable_motors_homed();

    try {
        // Call IHbcService to home the motors
        mHbcService.homeMotors();

        // MotorsHomed is an empty message, so just return the response
        // The response itself indicates successful homing

    } catch (const std::exception& e) {
        std::cerr << "Exception in homeMotors: " << e.what() << "\n";
        throw;
    }

    return response.SerializeAsString();
}