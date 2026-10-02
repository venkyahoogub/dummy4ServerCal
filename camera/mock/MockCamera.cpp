// Implements
#include "MockCamera.h"

// Dependencies
#include <algorithm>
#include <cstring>
#include <fstream>

#include "PngImageFrame.h"

using namespace ncs;

MockCamera::MockCamera(const std::string& deviceName, CameraRole deviceRole,
                       std::filesystem::path readFromPath)
    : mDeviceName(deviceName),
      mDeviceRole(deviceRole),
      mReadFromPath(readFromPath),
      mIsConnected(false),
      mGain(0.0),
      mExposureMicroSeconds(0.0) {}

bool MockCamera::connect() {
    mIsConnected = true;
    return true;
}

bool MockCamera::disconnect() {
    mIsConnected = false;
    return true;
}

SingleCapture MockCamera::waitForNextFrame(std::chrono::milliseconds maxWait) {
    SingleCapture capture;
    capture.pImage = nullptr;

    if (!mIsConnected) {
        capture.triggeredTimestamp = std::chrono::system_clock::now();
        return capture;
    }

    if (!std::filesystem::exists(mReadFromPath) ||
        !std::filesystem::is_directory(mReadFromPath)) {
        capture.triggeredTimestamp = std::chrono::system_clock::now();
        return capture;
    }

    std::filesystem::path targetPngPath;
    bool fileFound = false;

    // Find a file
    auto it = std::find_if(std::filesystem::directory_iterator(mReadFromPath),
                           std::filesystem::directory_iterator(),
                           [](const auto& entry) {
                               return entry.is_regular_file() &&
                                      entry.path().extension() == ".png";
                           });

    if (it != std::filesystem::directory_iterator()) {
        targetPngPath = it->path();
        fileFound = true;
    }

    // Read the file
    if (fileFound) {
        std::ifstream file(targetPngPath, std::ios::binary | std::ios::ate);
        if (file.is_open()) {
            std::streamsize fileSize = file.tellg();
            file.seekg(0, std::ios::beg);

            std::string buffer;
            buffer.resize(fileSize);

            if (file.read(&buffer[0], fileSize)) {
                auto currentTimestamp = std::chrono::system_clock::now();
                capture.pImage = std::make_unique<PngImageFrame>(
                    mDeviceName, std::move(buffer), currentTimestamp);

                capture.triggeredTimestamp = currentTimestamp;
            }
        }
    }

    return capture;
}

double MockCamera::getGain() { return mGain; }

double MockCamera::setGain(double gain) {
    if (!mIsConnected) {
        return mGain;
    }

    mGain = gain;
    return mGain;
}

double MockCamera::getExposure() { return mExposureMicroSeconds; }

double MockCamera::setExposure(double exposureMicroSeconds) {
    if (!mIsConnected) {
        return mExposureMicroSeconds;
    }
    mExposureMicroSeconds = exposureMicroSeconds;
    return exposureMicroSeconds;
}

void MockCamera::flushBuffer(int) {}