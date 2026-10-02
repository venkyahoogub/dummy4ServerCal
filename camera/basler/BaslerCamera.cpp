// Implements a nested class in..
#include "BaslerCameraArray.h"

// Dependencies
#include <algorithm>

#include "BaslerImageFrame.h"

using namespace ncs;
#define NOT_CONNECTED_DEVICE_NAME "NOT CONNECTED"

BaslerCameraArray::Camera::Camera(Pylon::CInstantCamera& pylonDevice)
    : mPylonDevice(pylonDevice),
      mDeviceName(NOT_CONNECTED_DEVICE_NAME),
      mDeviceRole(CameraRole::Other),
      mIsConnected(false),
      mRoI(pylonDevice) {}

bool BaslerCameraArray::Camera::connect() {
    if (isConnected()) {
        return true;
    }

    mPylonDevice.Open();

    if (!mPylonDevice.IsOpen()) {
        std::cout << "Camera failed to open" << std::endl;
        mIsConnected = false;
        return false;
    }

    GenApi::INodeMap& nodemap = mPylonDevice.GetNodeMap();
    GenApi::CStringPtr userIdNode = nodemap.GetNode("DeviceUserID");

    // If DeviceUserID is empty, set it based on order or other logic
    if (userIdNode && GenApi::IsWritable(userIdNode)) {
        std::string currentId = std::string(userIdNode->GetValue());
        if (currentId.empty()) {
            // Set unnamed camera to "Wide"
            userIdNode->SetValue("Wide");
            mDeviceName = "Wide";
            std::cout << "Set unnamed camera to: Wide" << std::endl;
        } else {
            mDeviceName = currentId;
        }
    } else if (userIdNode && GenApi::IsReadable(userIdNode)) {
        mDeviceName = std::string(userIdNode->GetValue());
    }

    std::string lower = mDeviceName;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    std::cout << "Detected camera device name: '" << mDeviceName
              << "' (lowercase: '" << lower << "')" << std::endl;

    if (lower.rfind("left", 0) == 0) {
        mDeviceRole = CameraRole::Left;
    } else if (lower.rfind("right", 0) == 0) {
        mDeviceRole = CameraRole::Right;
    } else if (lower.rfind("wide", 0) == 0) {
        mDeviceRole = CameraRole::Wide;
    } else {
        mDeviceRole = CameraRole::Other;
    }

    std::cout << "  Assigned role: ";
    switch (mDeviceRole) {
        case CameraRole::Left:
            std::cout << "Left";
            break;
        case CameraRole::Right:
            std::cout << "Right";
            break;
        case CameraRole::Wide:
            std::cout << "Wide";
            break;
        case CameraRole::Other:
            std::cout << "Other";
            break;
    }
    std::cout << std::endl;

    mIsConnected = true;
    return true;
}

bool BaslerCameraArray::Camera::disconnect() {
    if (!isConnected()) {
        return true;
    }

    if (mPylonDevice.IsGrabbing()) {
        mPylonDevice.StopGrabbing();
    }

    if (mPylonDevice.IsOpen()) {
        mPylonDevice.Close();
    }

    mDeviceName = "";
    mDeviceRole = CameraRole::Other;
    mIsConnected = false;

    return true;
}

void BaslerCameraArray::Camera::startStreaming() {
    if (!mPylonDevice.IsGrabbing()) {
        // Open host RAM buffers and listen for the physical hardware trigger
        // line
        mPylonDevice.StartGrabbing( Pylon::GrabStrategy_LatestImageOnly,
                                   Pylon::GrabLoop_ProvidedByUser);
    }
}

void BaslerCameraArray::Camera::stopStreaming() {
    if (mPylonDevice.IsGrabbing()) {
        mPylonDevice.StopGrabbing();
    }
}

SingleCapture BaslerCameraArray::Camera::waitForNextFrame(
    std::chrono::milliseconds maxWait) {
    SingleCapture captureResult;
    captureResult.pImage = nullptr;
    captureResult.triggeredTimestamp = std::chrono::system_clock::time_point{};

    if (!mIsConnected) {
        return captureResult;
    }

    try {
        auto timeoutMs = static_cast<uint32_t>(maxWait.count());

        // Drain any stale buffers present prior to waiting
        flushBuffer(timeoutMs);

        Pylon::CGrabResultPtr ptrGrabResult;

        bool isGrabbed = mPylonDevice.RetrieveResult(
            timeoutMs, ptrGrabResult, Pylon::TimeoutHandling_Return);

        if (isGrabbed && ptrGrabResult->GrabSucceeded()) {
            // Pass ptrGrabResult by value to move/transfer Pylon buffer
            // ownership
            auto frame =
                std::make_unique<BaslerImageFrame>(mDeviceName, ptrGrabResult);
            captureResult.pImage = std::move(frame);
            captureResult.triggeredTimestamp = std::chrono::system_clock::now();
        }
    } catch (const Pylon::GenericException& e) {
        captureResult.pImage = nullptr;
    } catch (const std::exception& e) {
        captureResult.pImage = nullptr;
    }

    return captureResult;
}

double BaslerCameraArray::Camera::getGain() {
    if (!mIsConnected) {
        return 0.0;
    }

    GenApi::INodeMap& nodemap = mPylonDevice.GetNodeMap();
    GenApi::CFloatPtr gainNode(nodemap.GetNode("Gain"));
    return gainNode->GetValue();
}

double BaslerCameraArray::Camera::setGain(double gain) {
    if (!mIsConnected) {
        return 0.0;
    }

    GenApi::INodeMap& nodemap = mPylonDevice.GetNodeMap();
    GenApi::CEnumerationPtr gainAuto(nodemap.GetNode("GainAuto"));

    if (GenApi::IsWritable(gainAuto)) {
        gainAuto->FromString("Off");
    }

    GenApi::CFloatPtr gainNode(nodemap.GetNode("Gain"));
    if (GenApi::IsWritable(gainNode)) {
        double clampedGain =
            std::max(gainNode->GetMin(), std::min(gain, gainNode->GetMax()));
        gainNode->SetValue(clampedGain);
    }

    return gainNode->GetValue();
}

double BaslerCameraArray::Camera::getExposure() {
    if (!mIsConnected) {
        return 0.0;
    }

    GenApi::INodeMap& nodemap = mPylonDevice.GetNodeMap();
    GenApi::CFloatPtr exposureTimeNode(nodemap.GetNode("ExposureTime"));
    return exposureTimeNode->GetValue();
}

double BaslerCameraArray::Camera::setExposure(double exposureMicroSeconds) {
    if (!mIsConnected) {
        return 0.0;
    }

    GenApi::INodeMap& nodemap = mPylonDevice.GetNodeMap();

    GenApi::CEnumerationPtr exposureAuto(nodemap.GetNode("ExposureAuto"));
    if (GenApi::IsWritable(exposureAuto)) {
        exposureAuto->FromString("Off");
    }

    GenApi::CFloatPtr exposureTimeNode(nodemap.GetNode("ExposureTime"));
    if (GenApi::IsWritable(exposureTimeNode)) {
        double clampedExposure = std::max(
            exposureTimeNode->GetMin(),
            std::min(exposureMicroSeconds, exposureTimeNode->GetMax()));
        exposureTimeNode->SetValue(clampedExposure);
    }

    return exposureTimeNode->GetValue();
}

void BaslerCameraArray::Camera::executeSoftwareTrigger() {
    if (!mIsConnected || !mPylonDevice.IsOpen()) {
        return;
    }

    try {
        mPylonDevice.ExecuteSoftwareTrigger();
    } catch (const Pylon::GenericException& e) {
        std::cerr << "Pylon error: " << e.GetDescription() << '\n';
    }
}

bool BaslerCameraArray::Camera::waitForTriggerReady(
    std::chrono::milliseconds maxWait) {
    if (!mIsConnected || !mPylonDevice.IsOpen()) {
        return false;
    }

    try {
        auto timeoutMs = static_cast<uint32_t>(maxWait.count());
        return mPylonDevice.WaitForFrameTriggerReady(
            timeoutMs, Pylon::TimeoutHandling_Return);
    } catch (const Pylon::GenericException& e) {
        return false;
    }
}

void BaslerCameraArray::Camera::flushBuffer(int timeout) {
    if (!mIsConnected) {
        return;
    }

    try {
        Pylon::CGrabResultPtr ptrGrabResult;
        // Drain all stale frames remaining in the grab engine queue
        while (mPylonDevice.RetrieveResult(0, ptrGrabResult,
                                           Pylon::TimeoutHandling_Return)) {
            // Intentionally discard stale frame
        }
        // discard also first frame
        while(mPylonDevice.RetrieveResult(timeout, ptrGrabResult, Pylon::TimeoutHandling_Return)){break;};
    } catch (const Pylon::GenericException& e) {
        // Log or handle exception as needed
    }
}