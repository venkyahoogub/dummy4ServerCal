// Implements
#include "DmdRoute.h"

#include <cstdint>

using namespace ncs;

ByteString DmdRoute::createResponse(
    const CalibrationApi::CalibrateDmd& calibrateRequest) {
    const std::string& rawBytes = calibrateRequest.image();

    size_t dataSize = rawBytes.size();
    Frame imageData = std::make_unique<uint8_t[]>(dataSize);

    std::memcpy(imageData.get(), rawBytes.data(), dataSize);

    mProjector.update_projection(imageData);

    CalibrationApi::FromServer response;
    CalibrationApi::DmdCalibrated* status = response.mutable_dmd_calibrated();
    status->set_success(true);

    return response.SerializeAsString();
}