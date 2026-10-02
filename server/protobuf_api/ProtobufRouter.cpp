// Implements
#include "ProtobufRouter.h"

// Dependencies
#include <exception>
#include <string>

#include "neo-calibration-server-api/calibration_server_api.pb.h"

using namespace ncs;
using namespace CalibrationApi;

ProtobufRouter::ProtobufRouter(IAboutRoute& aboutRoute,
                               ICameraRoute& cameraRoute,
                               INirLedRoute& nirLedRoute, IDmdRoute& dmdRoute,
                               ICameraTriggerRoute& cameraTriggerRoute,
                               ICalibrationRoute& calibrationRoute,
                               IMotorRoute& motorRoute,
                               IUvDiagnosticRoute& uvDiagnosticRoute,
                               ICalibrationTableRoute& calibrationTableRoute,
                               IDistanceSensorRoute& distanceSensorRoute,
                               IUvDeviceSettingsRoute& uvDeviceSettingsRoute,
                               IGetDeviceSettingsRoute& getDeviceSettingsRoute)
    : mAboutRoute(aboutRoute),
      mCameraRoute(cameraRoute),
      mNirLedRoute(nirLedRoute),
      mDmdRoute(dmdRoute),
      mCameraTriggerRoute(cameraTriggerRoute),
      mCalibrationRoute(calibrationRoute),
      mMotorRoute(motorRoute),
      mUvDiagnosticRoute(uvDiagnosticRoute),
      mCalibrationTableRoute(calibrationTableRoute),
      mDistanceSensorRoute(distanceSensorRoute),
      mUvDeviceSettingsRoute(uvDeviceSettingsRoute),
      mGetDeviceSettingsRoute(getDeviceSettingsRoute) {}

ByteString ProtobufRouter::getResponse(const ByteString& rawData,
                                       IStreamContext& streamContext) {
    ToServer request;
    if (!request.ParseFromString(rawData)) {
        throw IServiceRouter::ParseError(rawData);
    }

    switch (request.msg_case()) {
        case ToServer::kGetAbout:
            return mAboutRoute.createResponse(request.get_about());
        case ToServer::kGetSynchronizedCapture:
            return mCameraRoute.createResponse(
                request.get_synchronized_capture());
        case ToServer::kStartVideoStream:
            return mCameraRoute.startStream(streamContext);
        case ToServer::kStopVideoStream:
            return mCameraRoute.stopStream(streamContext);
        case ToServer::kSetExposureSettings:
            return mCameraRoute.setExposure(request.set_exposure_settings());
        case ToServer::kSetRegionOfInterest:
            return mCameraRoute.setRegionOfInterest(
                request.set_region_of_interest());
        case ToServer::kSetNirLedControls:
            return mNirLedRoute.setNirLedControls(
                request.set_nir_led_controls());
        case ToServer::kCalibrateDmd:
            return mDmdRoute.createResponse(request.calibrate_dmd());
        case ToServer::kSetCameraTriggerControls:
            return mCameraTriggerRoute.setCameraTriggerControls(
                request.set_camera_trigger_controls());
        case ToServer::kStartCalibration:
            return mCalibrationRoute.startCalibration(
                request.start_calibration());
        case ToServer::kHomeMotors:
            return mMotorRoute.homeMotors(request.home_motors());
        case ToServer::kStartUvDiagnostic:
            return mUvDiagnosticRoute.startUvDiagnostic(
                request.start_uv_diagnostic());
        case ToServer::kCancelUvDiagnostic:
            return mUvDiagnosticRoute.cancelUvDiagnostic(
                request.cancel_uv_diagnostic());
        case ToServer::kUploadCalibrationTable:
            return mCalibrationTableRoute.uploadCalibrationTable(
                request.upload_calibration_table());
        case ToServer::kGetWideCapture:
            return mCameraRoute.getWideCapture(request.get_wide_capture());
        case ToServer::kGetWideVideoStream:
            return mCameraRoute.startWideStream(streamContext);
        case ToServer::kStopWideVideoStream:
            return mCameraRoute.stopWideStream(streamContext);
        case ToServer::kDistanceSensorStartRanging:
            return mDistanceSensorRoute.startRanging(
                request.distance_sensor_start_ranging());
        case ToServer::kDistanceSensorStopRanging:
            return mDistanceSensorRoute.stopRanging(
                request.distance_sensor_stop_ranging());
        case ToServer::kSetDistanceSensorDeviceSettings:
            return mDistanceSensorRoute.setDeviceSettings(
                request.set_distance_sensor_device_settings());
        case ToServer::kSetUvDeviceSettings:
            return mUvDeviceSettingsRoute.setUvDeviceSettings(
                request.set_uv_device_settings());
        case ToServer::kGetDeviceSettings:
            return mGetDeviceSettingsRoute.getDeviceSettings(
                request.get_device_settings());
            // TODO: Add more routes...
            // Anticipating; DMD, and Database Access as additional
            // routes.
        default:
            throw IServiceRouter::NoRouteFoundError(request.msg_case());
    }
}