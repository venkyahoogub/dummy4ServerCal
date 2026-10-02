#include <common/SerializationUtils.h>
#include <neo-calibration-server-api/calibration_server_api.pb.h>

#include <asio.hpp>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>
#include <string>
#include <chrono>
#include <thread>

static std::string displayFormat(CalibrationApi::CameraImageFormat format);
static void processAbout(asio::ip::tcp::socket& skt);
static void processStreamStart(asio::ip::tcp::socket& skt);
static void processStreamStop(asio::ip::tcp::socket& skt);
static void receiveStreamFrames(asio::ip::tcp::socket& skt, int frameCount);
static void saveStreamFrameAsPNG(const CalibrationApi::CameraImage& cameraImage,
                                 const std::string& filePathString);
static void processGetWideCapture(asio::ip::tcp::socket& skt);
static void processStartWideVideoStream(asio::ip::tcp::socket& skt);
static void processStopWideVideoStream(asio::ip::tcp::socket& skt);
static void processWideStreamFrames(asio::ip::tcp::socket& skt, int frameCount);

int main() {
    try {
        asio::io_context io_ctx;
        asio::ip::tcp::resolver resolver(io_ctx);
        auto endpoints = resolver.resolve("127.0.0.1", "50051");

        asio::ip::tcp::socket skt(io_ctx);
        asio::connect(skt, endpoints);

        std::cout << "Connected to server!" << std::endl;

        processAbout(skt);

        std::cout << "\n=== Starting Stream ===\n";
        processStreamStart(skt);

        std::cout << "\n=== Receiving 5 Stream Frames ===\n";
        receiveStreamFrames(skt, 5);

        std::cout << "\n=== Stopping Stream ===\n";
        processStreamStop(skt);

        // Test wide camera single capture
        std::cout << "\n=== Testing Wide Camera Single Capture ===" << std::endl;
        processGetWideCapture(skt);

        // Test wide camera video streaming
        std::cout << "\n=== Testing Wide Camera Video Stream ===" << std::endl;
        processStartWideVideoStream(skt);
        
        // Capture 5 frames from the stream
        std::cout << "Capturing 5 frames from wide camera stream..." << std::endl;
        processWideStreamFrames(skt, 5);
        
        processStopWideVideoStream(skt);

        std::cout << "Press Enter to close connection...";
        std::cin.get();

        skt.shutdown(asio::ip::tcp::socket::shutdown_both);
        skt.close();

    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

static std::string displayFormat(CalibrationApi::CameraImageFormat format) {
    switch (format) {
        case CalibrationApi::CameraImageFormat::GB8:
            return "GB8";
        case CalibrationApi::CameraImageFormat::MONO8:
            return "Mono8";
        case CalibrationApi::CameraImageFormat::PNG:
            return "PNG";
        default:
            return "Unsupported";
    }
}

static void processAbout(asio::ip::tcp::socket& skt) {
    CalibrationApi::ToServer aboutRequest;
    aboutRequest.mutable_get_about();

    std::string protoMsg = aboutRequest.SerializeAsString();
    std::string request = ncs::prependMessageLength(protoMsg);
    asio::write(skt, asio::buffer(request, request.size()));

    std::uint32_t networkLen = 0;
    asio::read(skt, asio::buffer(&networkLen, sizeof(networkLen)));

    std::uint32_t msgSize = ncs::decodeToHostByteOrder(networkLen);
    std::string responseData(msgSize, '\0');

    asio::read(skt, asio::buffer(responseData.data(), responseData.size()));

    CalibrationApi::FromServer response;
    if (!response.ParseFromString(responseData)) {
        std::cout << "Could not decode packet" << std::endl;
        return;
    }

    if (response.has_about()) {
        std::cout << "Version is: " << response.about().version() << std::endl;
    }
}
namespace fs = std::filesystem;

static void processStreamStart(asio::ip::tcp::socket& skt) {
    CalibrationApi::ToServer request;
    request.mutable_start_video_stream();

    std::string protoMsg = request.SerializeAsString();
    std::string payload = ncs::prependMessageLength(protoMsg);
    asio::write(skt, asio::buffer(payload));

    // Read the response to the start request
    std::uint32_t networkLen = 0;
    asio::read(skt, asio::buffer(&networkLen, sizeof(networkLen)));

    std::uint32_t msgSize = ncs::decodeToHostByteOrder(networkLen);
    std::string responseData(msgSize, '\0');
    asio::read(skt, asio::buffer(responseData.data(), responseData.size()));

    std::cout << "Stream start command acknowledged.\n";
}

static void processStreamStop(asio::ip::tcp::socket& skt) {
    CalibrationApi::ToServer request;
    request.mutable_stop_video_stream();

    std::string protoMsg = request.SerializeAsString();
    std::string payload = ncs::prependMessageLength(protoMsg);
    asio::write(skt, asio::buffer(payload));

    // Read the response to the stop request
    std::uint32_t networkLen = 0;
    asio::read(skt, asio::buffer(&networkLen, sizeof(networkLen)));

    std::uint32_t msgSize = ncs::decodeToHostByteOrder(networkLen);
    std::string responseData(msgSize, '\0');
    asio::read(skt, asio::buffer(responseData.data(), responseData.size()));

    std::cout << "Stream stop command acknowledged.\n";
}

static void receiveStreamFrames(asio::ip::tcp::socket& skt, int frameCount) {
    int framesReceived = 0;

    while (framesReceived < frameCount) {
        std::uint32_t networkLen = 0;
        asio::read(skt, asio::buffer(&networkLen, sizeof(networkLen)));

        std::uint32_t msgSize = ncs::decodeToHostByteOrder(networkLen);
        std::string responseData(msgSize, '\0');

        asio::read(skt, asio::buffer(responseData.data(), responseData.size()));

        CalibrationApi::FromServer response;
        if (!response.ParseFromString(responseData)) {
            std::cout << "Could not decode packet" << std::endl;
            continue;
        }

        if (response.has_stream_frame()) {
            const auto& frame = response.stream_frame().frame();
            framesReceived++;

            /* clang-format off */
            std::cout << "Frame " << framesReceived
                        << ": left=" << frame.left().source()
                        << " (" << frame.left().width() << "x"
                        << frame.left().height()
                        << ", " << displayFormat(frame.left().format()) << "), "
                        << "right=" << frame.right().source()
                        << " (" << frame.right().width() << "x"
                        << frame.right().height()
                        << ", " << displayFormat(frame.right().format()) << ")\n";
            /* clang-format on */

            // SAVE TO DISK AS PNG
            saveStreamFrameAsPNG(
                frame.left(), "stream_captures/frame_" +
                                  std::to_string(framesReceived) + "_left.png");
            saveStreamFrameAsPNG(frame.right(),
                                 "stream_captures/frame_" +
                                     std::to_string(framesReceived) +
                                     "_right.png");
        }
    }
}

static void saveStreamFrameAsPNG(const CalibrationApi::CameraImage& cameraImage,
                                 const std::string& filePathString) {
    try {
        fs::path targetPath(filePathString);
        fs::path parentDir = targetPath.parent_path();

        if (!parentDir.empty() && !fs::exists(parentDir)) {
            fs::create_directories(parentDir);
        }

        // Create OpenCV Mat from raw image data
        int cvType;
        switch (cameraImage.format()) {
            case CalibrationApi::CameraImageFormat::MONO8:
                cvType = CV_8UC1;  // 8-bit grayscale
                break;
            case CalibrationApi::CameraImageFormat::GB8:
                cvType = CV_8UC1;  // Bayer pattern (treat as grayscale for now)
                break;
            default:
                std::cerr << "Unsupported image format for PNG conversion\n";
                return;
        }

        // Important: make a copy of the data since protobuf data may be freed
        std::vector<uint8_t> dataCopy(cameraImage.image().begin(),
                                      cameraImage.image().end());
        cv::Mat mat(cameraImage.height(), cameraImage.width(), cvType,
                    dataCopy.data());

        // Write to PNG file (this makes a copy internally)
        std::vector<int> compression_params;
        compression_params.push_back(cv::IMWRITE_PNG_COMPRESSION);
        compression_params.push_back(9);  // Compression level (0-9)

        if (cv::imwrite(targetPath.string(), mat, compression_params)) {
            std::cout << "Saved PNG: " << targetPath << " ("
                      << cameraImage.image().size() << " bytes raw)\n";
        } else {
            std::cerr << "Error: Failed to save PNG to " << targetPath << "\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception while saving PNG: " << e.what() << "\n";
    }
}

static void processGetWideCapture(asio::ip::tcp::socket& skt) {
    CalibrationApi::ToServer wideRequest;
    wideRequest.mutable_get_wide_capture();

    std::string protoMsg = wideRequest.SerializeAsString();
    std::string request = ncs::prependMessageLength(protoMsg);
    std::cout << "Requesting single wide camera capture...\n";
    asio::write(skt, asio::buffer(request, request.size()));

    std::uint32_t networkLen = 0;
    asio::read(skt, asio::buffer(&networkLen, sizeof(networkLen)));

    std::uint32_t msgSize = ncs::decodeToHostByteOrder(networkLen);
    std::string responseData(msgSize, '\0');

    asio::read(skt, asio::buffer(responseData.data(), responseData.size()));
    std::cout << "Wide capture response received.\n";

    CalibrationApi::FromServer resp;
    if (!resp.ParseFromString(responseData)) {
        std::cout << "Could not decode packet" << std::endl;
        return;
    }

    if (resp.has_wide_capture()) {
        const auto& wideCapture = resp.wide_capture();
        const auto& cameraImage = wideCapture.wide();
        
        std::cout << "Wide camera image captured successfully!" << std::endl;
        std::cout << "  Source: " << cameraImage.source() << std::endl;
        std::cout << "  Width: " << cameraImage.width() << " px" << std::endl;
        std::cout << "  Height: " << cameraImage.height() << " px" << std::endl;
        std::cout << "  Size: " << cameraImage.image().size() << " bytes" << std::endl;
        std::cout << "  Timestamp: " << cameraImage.timestamp_ms() << " ms" << std::endl;

        // Save the image as PNG
        std::string filename = "stream_captures/wide_single_capture_" + 
                               std::to_string(cameraImage.timestamp_ms()) + ".png";
        saveStreamFrameAsPNG(cameraImage, filename);
    } else {
        std::cout << "No wide capture data in response" << std::endl;
    }
}

static void processStartWideVideoStream(asio::ip::tcp::socket& skt) {
    CalibrationApi::ToServer streamRequest;
    streamRequest.mutable_get_wide_video_stream();

    std::string protoMsg = streamRequest.SerializeAsString();
    std::string request = ncs::prependMessageLength(protoMsg);
    std::cout << "Starting wide camera video stream...\n";
    asio::write(skt, asio::buffer(request, request.size()));

    std::uint32_t networkLen = 0;
    asio::read(skt, asio::buffer(&networkLen, sizeof(networkLen)));

    std::uint32_t msgSize = ncs::decodeToHostByteOrder(networkLen);
    std::string responseData(msgSize, '\0');

    asio::read(skt, asio::buffer(responseData.data(), responseData.size()));
    std::cout << "Stream started.\n";

    CalibrationApi::FromServer resp;
    if (!resp.ParseFromString(responseData)) {
        std::cout << "Could not decode packet" << std::endl;
        return;
    }

    if (resp.has_uv_diagnostic_response()) {
        const auto& streamResp = resp.uv_diagnostic_response();
        std::cout << "Stream response - Success: " << (streamResp.success() ? "true" : "false") << std::endl;
        std::cout << "Message: " << streamResp.message() << std::endl;
    }
}

static void processWideStreamFrames(asio::ip::tcp::socket& skt, int frameCount) {
    for (int i = 0; i < frameCount; ++i) {
        std::cout << "Receiving frame " << (i + 1) << "..." << std::endl;

        std::uint32_t networkLen = 0;
        asio::read(skt, asio::buffer(&networkLen, sizeof(networkLen)));

        std::uint32_t msgSize = ncs::decodeToHostByteOrder(networkLen);
        std::string responseData(msgSize, '\0');

        asio::read(skt, asio::buffer(responseData.data(), responseData.size()));

        CalibrationApi::FromServer resp;
        if (!resp.ParseFromString(responseData)) {
            std::cout << "Could not decode frame packet" << std::endl;
            continue;
        }

        if (resp.has_wide_stream_frame()) {
            const auto& streamFrame = resp.wide_stream_frame();
            const auto& wideImage = streamFrame.wide();

            std::cout << "  Frame " << (i + 1) << " received:" << std::endl;
            std::cout << "    Source: " << wideImage.source() << std::endl;
            std::cout << "    Width: " << wideImage.width() << " px" << std::endl;
            std::cout << "    Height: " << wideImage.height() << " px" << std::endl;
            std::cout << "    Size: " << wideImage.image().size() << " bytes" << std::endl;
            std::cout << "    Timestamp: " << wideImage.timestamp_ms() << " ms" << std::endl;
            std::cout << "    Final frame: " << (streamFrame.is_final() ? "yes" : "no") << std::endl;

            // Save the frame as PNG
            std::string filename = "stream_captures/wide_stream_frame_" + 
                                   std::to_string(i) + "_" + 
                                   std::to_string(wideImage.timestamp_ms()) + ".png";
            saveStreamFrameAsPNG(wideImage, filename);

            if (streamFrame.is_final()) {
                std::cout << "Stream ended." << std::endl;
                break;
            }
        } else {
            std::cout << "  No stream frame data in response" << std::endl;
        }

        // Small delay between frames
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

static void processStopWideVideoStream(asio::ip::tcp::socket& skt) {
    CalibrationApi::ToServer stopRequest;
    stopRequest.mutable_stop_wide_video_stream();

    std::string protoMsg = stopRequest.SerializeAsString();
    std::string request = ncs::prependMessageLength(protoMsg);
    std::cout << "Stopping wide camera video stream...\n";
    asio::write(skt, asio::buffer(request, request.size()));

    std::uint32_t networkLen = 0;
    asio::read(skt, asio::buffer(&networkLen, sizeof(networkLen)));

    std::uint32_t msgSize = ncs::decodeToHostByteOrder(networkLen);
    std::string responseData(msgSize, '\0');

    asio::read(skt, asio::buffer(responseData.data(), responseData.size()));
    std::cout << "Stream stopped.\n";

    CalibrationApi::FromServer resp;
    if (!resp.ParseFromString(responseData)) {
        std::cout << "Could not decode packet" << std::endl;
        return;
    }

    if (resp.has_uv_diagnostic_response()) {
        const auto& stopResp = resp.uv_diagnostic_response();
        std::cout << "Stop response - Success: " << (stopResp.success() ? "true" : "false") << std::endl;
        std::cout << "Message: " << stopResp.message() << std::endl;
    }
}