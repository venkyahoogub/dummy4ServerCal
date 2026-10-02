// Implements
#include "CameraRoute.h"

// Dependencies
#include <common/SerializationUtils.h>

#include <chrono>
#include <fstream>
#include <iostream>

using namespace ncs;
using namespace CalibrationApi;

static void map(CalibrationApi::CameraImage* outImage,
                const ncs::IImageFrame& inImage);
static CameraImageFormat convertFormat(ImageFormat inFormat);
static std::int64_t convertTimeStamp(ImageTimestamp inTimeStamp);
static CameraRole convertToCameraRole(CalibrationApi::Camera camera);

CameraRoute::CameraRoute(CameraService& cameraService)
    : mCameraService(cameraService) {}

CameraRoute::~CameraRoute() {
    // Ensure streaming is stopped and thread is joined
    if (mStreamingActive.load()) {
        mStreamingActive.store(false);
    }
    if (mStreamThread && mStreamThread->joinable()) {
        mStreamThread->join();
    }

    // Ensure wide streaming is stopped
    if (mWideStreamingActive.load()) {
        mWideStreamingActive.store(false);
    }
    if (mWideStreamThread && mWideStreamThread->joinable()) {
        mWideStreamThread->join();
    }
}

ByteString CameraRoute::createResponse(
    const GetSynchronizedCapture& imageRequest) {
    FromServer outgoing;
    auto capture = mCameraService.synchronizedCapture();

    if (!capture.left || !capture.right) {
        std::cerr << "Error: One or both camera frames are null!" << std::endl;
        return outgoing.SerializeAsString();
    }

    map(outgoing.mutable_synchronized_capture()->mutable_left(), *capture.left);
    map(outgoing.mutable_synchronized_capture()->mutable_right(),
        *capture.right);

    return outgoing.SerializeAsString();
}

ByteString CameraRoute::setExposure(const SetExposureSettings& request) {
    FromServer outgoing;
    auto failureEncountered = false;
    auto response = outgoing.mutable_exposure_response();

    for (const auto& setting : request.settings()) {
        try {
            auto cameraId = setting.camera();
            auto gain = setting.gain();
            auto exposureUs = setting.exposure_us();
            auto role = convertToCameraRole(cameraId);

            auto wasSet =
                mCameraService.setExposureDetails(role, gain, exposureUs);

            if (!wasSet) {
                failureEncountered = true;
            }

            auto camera = mCameraService.getCamera(role);
            if (camera) {
                auto newSetting = response->add_settings();
                newSetting->set_camera(setting.camera());
                newSetting->set_gain(camera->getGain());
                newSetting->set_exposure_us(camera->getExposure());
            } else {
                failureEncountered = true;
            }
        } catch (...) {
            failureEncountered = true;
        }
    }

    response->set_failures(failureEncountered);
    return outgoing.SerializeAsString();
}

ByteString CameraRoute::setRegionOfInterest(
    const SetRegionOfInterest& request) {
    FromServer outgoing;
    auto failureEncountered = false;
    auto response = outgoing.mutable_region_of_interest();

    for (const auto& setting : request.regions()) {
        try {
            auto cameraId = setting.camera();
            auto role = convertToCameraRole(cameraId);
            auto camera = mCameraService.getCamera(role);

            if (camera) {
                auto& roi = camera->getRegionOfInterest();
                roi.setReverseX(setting.reversex());
                roi.setReverseY(setting.reversey());
                roi.setHeight(setting.height());
                roi.setWidth(setting.width());
                roi.setXOffset(setting.xoffset());
                roi.setYOffset(setting.yoffset());

                auto newSetting = response->add_regions();
                newSetting->set_camera(setting.camera());
                newSetting->set_reversex(roi.isReverseX());
                newSetting->set_reversey(roi.isReverseY());
                newSetting->set_xoffset(roi.getXOffset());
                newSetting->set_yoffset(roi.getYOffset());
                newSetting->set_height(roi.getHeight());
                newSetting->set_width(roi.getWidth());
            } else {
                failureEncountered = true;
            }

        } catch (...) {
            failureEncountered = true;
        }
    }

    response->set_failures(failureEncountered);
    return outgoing.SerializeAsString();
}

ByteString CameraRoute::startStream(IStreamContext& streamContext) {
    // Prevent multiple streams
    if (mStreamingActive.load()) {
        std::cout << "Stream already active, ignoring start request\n";
        FromServer response;
        return response.SerializeAsString();
    }

    // Mark stream as active and start the streaming thread
    mStreamingActive.store(true);
    mCameraService.openStream();

    mStreamThread = std::make_unique<std::thread>(
        [this, &streamContext]() { this->streamWorker(streamContext); });

    std::cout << "Stream started\n";

    // Return acknowledgment
    FromServer response;
    return response.SerializeAsString();
}

ByteString CameraRoute::stopStream(IStreamContext& streamContext) {
    // Signal the stream thread to stop
    mStreamingActive.store(false);

    // Close camera streams
    mCameraService.closeStream();

    // Wait for thread to finish
    if (mStreamThread && mStreamThread->joinable()) {
        mStreamThread->join();
    }

    mStreamThread.reset();

    std::cout << "Stream stopped\n";

    // Return acknowledgment
    FromServer response;
    return response.SerializeAsString();
}

void CameraRoute::streamWorker(IStreamContext& streamContext) {
    std::cout << "Stream worker thread started\n";

    while (mStreamingActive.load()) {
        try {
            // Capture synchronized frames
            auto capture = mCameraService.synchronizedCapture();

            if (!capture.left || !capture.right) {
                std::cerr << "Error: Failed to capture synchronized frames\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }

            // Create response with stream frame
            FromServer response;
            auto* streamFrame = response.mutable_stream_frame();
            auto* frame = streamFrame->mutable_frame();

            // Map left and right images
            map(frame->mutable_left(), *capture.left);
            map(frame->mutable_right(), *capture.right);

            // Serialize the response
            ByteString serializedResponse = response.SerializeAsString();

            // Prepend message length and send
            ByteString messageWithLength =
                ncs::prependMessageLength(serializedResponse);
            streamContext.send(messageWithLength);

            // Small delay to avoid overwhelming the network
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        } catch (const std::exception& e) {
            // Suppress "Broken pipe" and "Bad file descriptor" errors
            // TODO: Figure out why this happens and then remove the suppression
            // of the error. I think it could be when the client disconnects
            // while the worker is sending, there seems to be a race condition.
            std::string error_msg = e.what();
            if (error_msg.find("Broken pipe") == std::string::npos &&
                error_msg.find("Bad file descriptor") == std::string::npos) {
                std::cerr << "Exception in stream worker: " << e.what() << "\n";
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    std::cout << "Stream worker thread stopped\n";
}

static void map(CalibrationApi::CameraImage* outImage,
                const ncs::IImageFrame& inImage) {
    outImage->set_image(reinterpret_cast<const char*>(inImage.data()),
                        inImage.size());
    outImage->set_source(inImage.source());
    outImage->set_width(inImage.width());
    outImage->set_height(inImage.height());
    outImage->set_format(convertFormat(inImage.format()));
    outImage->set_timestamp_ms(convertTimeStamp(inImage.timestamp()));
}

static CameraImageFormat convertFormat(ImageFormat inFormat) {
    switch (inFormat) {
        case ncs::ImageFormat::GB8:
            return CameraImageFormat::GB8;
        case ncs::ImageFormat::Mono8:
            return CameraImageFormat::MONO8;
        case ncs::ImageFormat::PNG:
            return CameraImageFormat::PNG;
        default:
            // Debug: log unknown formats
            std::cerr << "WARNING: Unknown ImageFormat enum value: "
                      << static_cast<int>(inFormat) << ", treating as MONO8\n";
            return CameraImageFormat::MONO8;
    }
}

static std::int64_t convertTimeStamp(ImageTimestamp inTimeStamp) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               inTimeStamp.time_since_epoch())
        .count();
}

static CameraRole convertToCameraRole(CalibrationApi::Camera camera) {
    switch (camera) {
        case CalibrationApi::Camera::LEFT:
            return CameraRole::Left;
        case CalibrationApi::Camera::RIGHT:
            return CameraRole::Right;
        case CalibrationApi::Camera::WIDE:
            return CameraRole::Wide;
        default:
            return CameraRole::Other;
    }
}

ByteString CameraRoute::getWideCapture(
    const CalibrationApi::GetWideCapture& request) {
    FromServer outgoing;

    if (!mCameraService.wideCamera()) {
        std::cerr << "Error: Wide camera not available!" << std::endl;
        return outgoing.SerializeAsString();
    }

    auto capture = mCameraService.wideCapture();

    if (!capture.pImage) {
        std::cerr << "Error: Failed to capture wide camera frame!" << std::endl;
        return outgoing.SerializeAsString();
    }

    // DEBUG: Check what format the image frame has BEFORE conversion
    std::cout << "DEBUG: Raw image frame format value = "
              << static_cast<int>(capture.pImage->format()) << std::endl;

    auto* wideImage = outgoing.mutable_wide_capture()->mutable_wide();
    map(wideImage, *capture.pImage);

    std::cout << "DEBUG: After map() in getWideCapture, format enum value = "
              << static_cast<int>(wideImage->format()) << std::endl;
    std::cout << "DEBUG: Image source = " << wideImage->source() << std::endl;
    std::cout << "DEBUG: Image width = " << wideImage->width() << std::endl;
    std::cout << "DEBUG: Image height = " << wideImage->height() << std::endl;

    // Save the frame to stream_captures folder
    std::string filename =
        "stream_captures/wide_single_capture_" +
        std::to_string(capture.pImage->timestamp().time_since_epoch().count()) +
        ".raw";
    std::ofstream outFile(filename, std::ios::binary);
    outFile.write(reinterpret_cast<const char*>(capture.pImage->data()),
                  capture.pImage->size());
    outFile.close();
    std::cout << "Wide single capture saved to: " << filename << std::endl;

    return outgoing.SerializeAsString();
}

ByteString CameraRoute::startWideStream(IStreamContext& streamContext) {
    if (mWideStreamingActive.load()) {
        std::cout << "Wide stream already active, ignoring start request\n";
        FromServer response;
        return response.SerializeAsString();
    }

    // Check if wide camera exists
    if (!mCameraService.wideCamera()) {
        std::cerr << "Error: Wide camera not available!\n";
        FromServer response;
        return response.SerializeAsString();
    }

    mWideStreamingActive.store(true);

    mCameraService.openWideStream();

    // Give camera time to stabilize
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    mWideStreamThread = std::make_unique<std::thread>(
        [this, &streamContext]() { this->wideStreamWorker(streamContext); });

    std::cout << "Wide stream started\n";

    FromServer response;
    return response.SerializeAsString();
}

ByteString CameraRoute::stopWideStream(IStreamContext& streamContext) {
    mWideStreamingActive.store(false);

    mCameraService.closeWideStream();

    if (mWideStreamThread && mWideStreamThread->joinable()) {
        mWideStreamThread->join();
    }

    mWideStreamThread.reset();

    std::cout << "Wide stream stopped\n";

    FromServer response;
    return response.SerializeAsString();
}

void CameraRoute::wideStreamWorker(IStreamContext& streamContext) {
    std::cout << "Wide stream worker thread started\n";

    while (mWideStreamingActive.load()) {
        try {
            std::cout << "Wide: Attempting to capture frame...\n";

            // Use wideStreamCapture which assumes stream is already open
            auto capture = mCameraService.wideStreamCapture();

            if (!capture.pImage) {
                std::cerr
                    << "Wide: Error - Failed to capture wide camera frame\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }

            std::cout << "Wide: Frame captured successfully! Size: "
                      << capture.pImage->size() << " bytes\n";

            FromServer response;
            auto* streamFrame = response.mutable_wide_stream_frame();

            map(streamFrame->mutable_wide(), *capture.pImage);

            std::cout << "DEBUG: After map() in wideStreamWorker, format enum "
                         "value = "
                      << static_cast<int>(streamFrame->mutable_wide()->format())
                      << std::endl;

            streamFrame->set_is_final(false);

            ByteString serializedResponse = response.SerializeAsString();

            ByteString messageWithLength =
                ncs::prependMessageLength(serializedResponse);
            streamContext.send(messageWithLength);

            std::cout << "Wide: Frame sent to client\n";

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        } catch (const std::exception& e) {
            std::string error_msg = e.what();
            if (error_msg.find("Broken pipe") == std::string::npos &&
                error_msg.find("Bad file descriptor") == std::string::npos) {
                std::cerr << "Exception in wide stream worker: " << e.what()
                          << "\n";
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    std::cout << "Wide stream worker thread stopped\n";
}