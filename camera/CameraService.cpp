// Implements...
#include "CameraService.h"

// Dependencies
#include <chrono>
#include <future>
#include <iostream>
#include <thread>

using namespace ncs;
using namespace std::chrono_literals;

constexpr std::chrono::milliseconds TRIGGER_TIMEOUT = 5000ms;
constexpr std::chrono::milliseconds TRIGGER_CHECK_INTERVAL = 100ms;

CameraService::CameraService(std::vector<OwnedCameraArray> cameraArrays)
    : mCameraArrays(std::move(cameraArrays)),
      mpLeftCamera(nullptr),
      mpRightCamera(nullptr),
      mpWideCamera(nullptr),
      mReady(false),
      mKeepStreamsOpen(false),
      mWideStreamOpen(false) {}

void CameraService::startup() {
    for (auto& array : mCameraArrays) {
        try {
            array->assemble();

            if (!mpLeftCamera && array->hasCamera(CameraRole::Left)) {
                mpLeftCamera = &(array->getCamera(CameraRole::Left));
            }

            if (!mpRightCamera && array->hasCamera(CameraRole::Right)) {
                mpRightCamera = &(array->getCamera(CameraRole::Right));
            }

            if (!mpWideCamera && array->hasCamera(CameraRole::Wide)) {
                mpWideCamera = &(array->getCamera(CameraRole::Wide));
            }
        } catch (...) {
        }
    }

    if (mpLeftCamera && mpRightCamera) {
        mReady = true;
    }
}

void CameraService::shutdown() {
    for (auto& array : mCameraArrays) {
        if (!array->isAssembled()) {
            continue;
        }
        array->disband();
    }

    mpLeftCamera = nullptr;
    mpRightCamera = nullptr;
    mpWideCamera = nullptr;
    mReady = false;
}

void CameraService::openWideStream() {
    if (mpWideCamera && !mWideStreamOpen) {
        std::cout << "Opening wide camera stream...\n";
        mpWideCamera->startStreaming();
        mWideStreamOpen = true;
    }
}

void CameraService::closeWideStream() {
    if (mpWideCamera && mWideStreamOpen) {
        std::cout << "Closing wide camera stream...\n";
        mpWideCamera->stopStreaming();
        mWideStreamOpen = false;
    }
}

SingleCapture CameraService::wideCapture() {
    SingleCapture captureResult;
    captureResult.pImage = nullptr;
    captureResult.triggeredTimestamp = std::chrono::system_clock::time_point{};

    if (!mpWideCamera) {
        std::cerr << "wideCapture: Wide camera is null\n";
        return captureResult;
    }

    // If stream is not open, open it temporarily for single capture
    bool tempOpen = false;
    if (!mWideStreamOpen) {
        std::cout << "wideCapture: Opening stream for single capture...\n";
        mpWideCamera->startStreaming();
        tempOpen = true;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cout << "wideCapture: Waiting for trigger ready...\n";

    // Wait for trigger ready with interruptible polling
    auto start_time = std::chrono::steady_clock::now();
    bool trigger_ready = false;

    while (std::chrono::steady_clock::now() - start_time < TRIGGER_TIMEOUT) {
        if (mpWideCamera->waitForTriggerReady(TRIGGER_CHECK_INTERVAL)) {
            trigger_ready = true;
            break;
        }
    }

    if (!trigger_ready) {
        std::cerr << "wideCapture: Timeout waiting for trigger ready\n";
        if (tempOpen) {
            std::cout << "wideCapture: Closing temporary stream...\n";
            mpWideCamera->stopStreaming();
        }
        return captureResult;
    }

    std::cout << "wideCapture: Camera ready, executing software trigger...\n";
    mpWideCamera->executeSoftwareTrigger();

    std::cout << "wideCapture: Waiting for frame...\n";
    captureResult = mpWideCamera->waitForNextFrame(TRIGGER_TIMEOUT);

    if (captureResult.pImage) {
        std::cout << "wideCapture: Frame received!\n";
    } else {
        std::cerr << "wideCapture: No frame received after trigger\n";
    }

    captureResult.triggeredTimestamp = std::chrono::system_clock::now();

    // Close stream if we opened it temporarily
    if (tempOpen) {
        std::cout << "wideCapture: Closing temporary stream...\n";
        mpWideCamera->stopStreaming();
    }

    return captureResult;
}

SingleCapture CameraService::wideStreamCapture() {
    SingleCapture captureResult;
    captureResult.pImage = nullptr;
    captureResult.triggeredTimestamp = std::chrono::system_clock::time_point{};

    if (!mpWideCamera || !mWideStreamOpen) {
        return captureResult;
    }

    // For continuous streaming, just wait for next frame
    // The stream is already open and trigger is already configured
    std::cout << "wideStreamCapture: Executing software trigger...\n";
    mpWideCamera->executeSoftwareTrigger();

    std::cout << "wideStreamCapture: Waiting for frame...\n";
    captureResult = mpWideCamera->waitForNextFrame(TRIGGER_TIMEOUT);

    if (captureResult.pImage) {
        std::cout << "wideStreamCapture: Frame received!\n";
    } else {
        std::cerr << "wideStreamCapture: No frame received after trigger\n";
    }

    captureResult.triggeredTimestamp = std::chrono::system_clock::now();
    return captureResult;
}

ICamera* CameraService::getCamera(CameraRole camera) {
    switch (camera) {
        case CameraRole::Left:
            return mpLeftCamera;
        case CameraRole::Right:
            return mpRightCamera;
        case CameraRole::Wide:
            return mpWideCamera;
        default:
            return nullptr;
    }
}

void CameraService::openStream() {
    mpLeftCamera->startStreaming();
    mpRightCamera->startStreaming();
    mKeepStreamsOpen = true;
}

void CameraService::closeStream() {
    mpLeftCamera->stopStreaming();
    mpRightCamera->stopStreaming();
    mKeepStreamsOpen = false;
}

bool CameraService::setExposureDetails(CameraRole camera, double gain,
                                       double exposure) {
    switch (camera) {
        case CameraRole::Left: {
            if (!mpLeftCamera) return false;
            mpLeftCamera->setGain(gain);
            mpLeftCamera->setExposure(exposure);
            return true;
        }
        case CameraRole::Right: {
            if (!mpRightCamera) return false;
            mpRightCamera->setGain(gain);
            mpRightCamera->setExposure(exposure);
            return true;
        }
        case CameraRole::Wide: {
            if (!mpWideCamera) return false;
            mpWideCamera->setGain(gain);
            mpWideCamera->setExposure(exposure);
            return true;
        }
        default:
            return false;
    }
}

SynchronizedCapture CameraService::synchronizedCapture() {
    SynchronizedCapture synchronizedCapture;
    synchronizedCapture.triggeredTimestamp =
        std::chrono::system_clock::time_point{};

    if (!mReady || !mpLeftCamera || !mpRightCamera) {
        return synchronizedCapture;
    }

    if (!mKeepStreamsOpen) {
        mpLeftCamera->startStreaming();
        mpRightCamera->startStreaming();
    }

    std::future<SingleCapture> leftTask = std::async(
        std::launch::async,
        [this]() { return mpLeftCamera->waitForNextFrame(TRIGGER_TIMEOUT); });

    std::future<SingleCapture> rightTask = std::async(
        std::launch::async,
        [this]() { return mpRightCamera->waitForNextFrame(TRIGGER_TIMEOUT); });

    SingleCapture leftCapture = leftTask.get();
    SingleCapture rightCapture = rightTask.get();

    if (!mKeepStreamsOpen) {
        mpLeftCamera->stopStreaming();
        mpRightCamera->stopStreaming();
    }

    synchronizedCapture.left = std::move(leftCapture.pImage);
    synchronizedCapture.right = std::move(rightCapture.pImage);
    synchronizedCapture.triggeredTimestamp = std::chrono::system_clock::now();

    return synchronizedCapture;
}