#pragma once

// Dependencies
#include <memory>
#include <string>
#include "../IServiceRouter.h"
#include "../IStreamContext.h"
#include "IAboutRoute.h"
#include "ICameraRoute.h"
#include "IDmdRoute.h"
#include "INirLedRoute.h"
#include "ICameraTriggerRoute.h"
#include "ICalibrationRoute.h"
#include "IMotorRoute.h"
#include "IUvDiagnosticRoute.h"
#include "ICalibrationTableRoute.h"
#include "IDistanceSensorRoute.h"
#include "IUvDeviceSettingsRoute.h"
#include "IGetDeviceSettingsRoute.h"

namespace ncs {
/**
 * @class ProtobufRouter
 * @brief Implementation of IServiceRouter that uses Protocol Buffers for
 * message serialization.
 *
 * This router acts as the translation layer between raw byte streams and
 * high-level service logic. It deserializes incoming @ref ByteString data into
 * Protobuf messages, dispatches them to the appropriate route, and serializes
 * the result back to bytes.
 */

using ByteString = std::string;

class ProtobufRouter : public IServiceRouter {
   public:
    /**
     * @brief Constructs a ProtobufRouter with the specific routes provided.
     * @param AboutRoute Reference to the handler for metadata and versioning
     * queries.
     */
    ProtobufRouter(IAboutRoute& aboutRoute, ICameraRoute& cameraRoute,
                   INirLedRoute& nirLedRoute, IDmdRoute& dmdRoute, ICameraTriggerRoute& cameraTriggerRoute, 
                   ICalibrationRoute& calibrationRoute, IMotorRoute& motorRoute, 
                   IUvDiagnosticRoute& uvDiagnosticRoute, ICalibrationTableRoute& calibrationTableRoute,
                   IDistanceSensorRoute& distanceSensorRoute, IUvDeviceSettingsRoute& uvDeviceSettingsRoute,
                   IGetDeviceSettingsRoute& getDeviceSettingsRoute);

    /**
     * @brief Processes a raw byte request and returns a serialized response.
     *
     * This method implements the core routing logic:
     * 1. Deserializes @p rawData into a Protobuf message.
     * 2. Identifies the target service/method.
     * 3. Executes the logic and captures the response.
     * 4. Serializes the response back into a @ref ByteString.
     *
     * @param rawData The incoming binary data payload.
     * @return A const ByteString containing the serialized Protobuf response.
     * @throws ProtobufException (or similar) if the data is malformed or
     *         deserialization fails.
     */
    ByteString getResponse(const ByteString& rawData,
                           IStreamContext& streamContext) override;

   private:
    IAboutRoute& mAboutRoute;
    ICameraRoute& mCameraRoute;
    INirLedRoute& mNirLedRoute;
    IDmdRoute& mDmdRoute;
    ICameraTriggerRoute& mCameraTriggerRoute;
    ICalibrationRoute& mCalibrationRoute;
    IMotorRoute& mMotorRoute;
    IUvDiagnosticRoute& mUvDiagnosticRoute;
    ICalibrationTableRoute& mCalibrationTableRoute;
    IDistanceSensorRoute& mDistanceSensorRoute;
    IUvDeviceSettingsRoute& mUvDeviceSettingsRoute;
    IGetDeviceSettingsRoute& mGetDeviceSettingsRoute;
    // TODO: DMD and Database routing
};
}  // namespace ncs