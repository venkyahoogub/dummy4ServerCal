#include <common/SerializationUtils.h>
#include <neo-calibration-server-api/calibration_server_api.pb.h>

#include <asio.hpp>
#include <exception>
#include <filesystem>
#include <fstream>
#include <string>

static void processAbout(asio::ip::tcp::socket& skt);
static void processRegionOfInterest(asio::ip::tcp::socket& skt);
static void processExposureSettings(asio::ip::tcp::socket& skt);
static void processCoordinatedCapture(asio::ip::tcp::socket& skt);
static void processSetNirLedControls(asio::ip::tcp::socket& skt);
static void processDmd(asio::ip::tcp::socket& skt);
static std::string loadBinaryFile(const std::string& path);
bool saveProtobufImageToFile(const CalibrationApi::CameraImage& cameraImage,
                             const std::string& filePathString);

int main() {
    try {
        asio::io_context io_ctx;
        asio::ip::tcp::resolver resolver(io_ctx);
        auto endpoints = resolver.resolve("127.0.0.1", "50051");

        asio::ip::tcp::socket skt(io_ctx);
        asio::connect(skt, endpoints);

        std::cout << "Connected to server!" << std::endl;

        processAbout(skt);
        processDmd(skt);
        processExposureSettings(skt);
        processRegionOfInterest(skt);
        processCoordinatedCapture(skt);
        processSetNirLedControls(skt);

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

static void processAbout(asio::ip::tcp::socket& skt) {
    CalibrationApi::ToServer aboutRequest;
    aboutRequest.mutable_get_about();

    std::string protoMsg = aboutRequest.SerializeAsString();
    std::string request = ncs::prependMessageLength(protoMsg);
    asio::write(skt, asio::buffer(request, request.size()));

    asio::streambuf receive_buffer;
    asio::error_code ec;

    std::uint32_t networkLen = 0;
    asio::read(skt, asio::buffer(&networkLen, sizeof(networkLen)));

    std::uint32_t msgSize = ncs::decodeToHostByteOrder(networkLen);
    std::string responseData(msgSize, '\0');

    asio::read(skt, asio::buffer(responseData.data(), responseData.size()));
    if (ec && ec != asio::error::eof) {
        throw asio::system_error(ec);
    }

    CalibrationApi::FromServer response;
    if (!response.ParseFromString(responseData)) {
        std::cout << "Could not decode packet" << std::endl;
    }

    if (response.has_about()) {
        std::cout << "Version is: " << response.about().version() << std::endl;
    }

    if (ec && ec != asio::error::eof) {
        throw asio::system_error(ec);
    }
}

static void processDmd(asio::ip::tcp::socket& skt) {
    std::cout << "Processing Dmd..." << std::endl;
    CalibrationApi::ToServer dmdRequest;
    auto* calDmd = dmdRequest.mutable_calibrate_dmd();

    calDmd->set_image(loadBinaryFile("assets/dotImage_3_5x3-r.bmp"));

    const std::string protoMsg = dmdRequest.SerializeAsString();
    const std::string request = ncs::prependMessageLength(protoMsg);

    asio::write(skt, asio::buffer(request));
    asio::error_code ec;

    std::uint32_t networkLen = 0;
    asio::read(skt, asio::buffer(&networkLen, sizeof(networkLen)));

    std::uint32_t msgSize = ncs::decodeToHostByteOrder(networkLen);
    std::string responseData(msgSize, '\0');

    asio::read(skt, asio::buffer(responseData.data(), responseData.size()));
    if (ec && ec != asio::error::eof) {
        throw asio::system_error(ec);
    }

    CalibrationApi::FromServer response;
    if (!response.ParseFromString(responseData)) {
        std::cout << "Could not decode packet" << std::endl;
    }

    if (ec && ec != asio::error::eof) {
        throw asio::system_error(ec);
    }
}

static void processExposureSettings(asio::ip::tcp::socket& skt) {
    CalibrationApi::ToServer settingsRequest;
    auto settings = settingsRequest.mutable_set_exposure_settings();

    auto leftSettings = settings->add_settings();
    leftSettings->set_camera(CalibrationApi::Camera::LEFT);
    leftSettings->set_gain(0.0);
    leftSettings->set_exposure_us(20000.0);

    auto rightSettings = settings->add_settings();
    rightSettings->set_camera(CalibrationApi::Camera::RIGHT);
    rightSettings->set_gain(1.0);
    rightSettings->set_exposure_us(30000.0);

    std::string protoMsg = settingsRequest.SerializeAsString();
    std::string request = ncs::prependMessageLength(protoMsg);
    asio::write(skt, asio::buffer(request, request.size()));

    asio::error_code ec;  // Used to capture network errors cleanly

    std::uint32_t networkLen = 0;
    asio::read(skt, asio::buffer(&networkLen, sizeof(networkLen)), ec);
    if (ec) {
        std::cout << "Failed to read packet length: " << ec.message()
                  << std::endl;
        return;
    }

    std::uint32_t msgSize = ncs::decodeToHostByteOrder(networkLen);
    std::string responseData(msgSize, '\0');

    asio::read(skt, asio::buffer(&responseData[0], responseData.size()), ec);
    if (ec) {
        if (ec != asio::error::eof) {
            std::cout << "Network error during payload read: " << ec.message()
                      << std::endl;
        }
        return;
    }

    CalibrationApi::FromServer response;
    if (!response.ParseFromString(responseData)) {
        std::cout << "Could not decode packet" << std::endl;
        return;
    }

    if (response.has_exposure_response()) {
        const auto& exp_response = response.exposure_response();

        std::cout << "Set Exposure Settings failed? "
                  << (exp_response.failures() ? "Yes" : "No") << "\n\n";

        std::cout << "--- Applied Camera Adjustments ---\n";

        for (int i = 0; i < exp_response.settings_size(); ++i) {
            const auto& target_settings = exp_response.settings(i);

            std::string camera_name = "UNKNOWN";
            switch (target_settings.camera()) {
                case CalibrationApi::Camera::LEFT:
                    camera_name = "LEFT";
                    break;
                case CalibrationApi::Camera::RIGHT:
                    camera_name = "RIGHT";
                    break;
                case CalibrationApi::Camera::WIDE:
                    camera_name = "WIDE";
                    break;
                default:
                    camera_name = "UNKNOWN_CAMERA";
                    break;
            }

            std::cout << "Camera: " << camera_name << "\n"
                      << "  Gain:     " << target_settings.gain() << "\n"
                      << "  Exposure: " << target_settings.exposure_us()
                      << " us\n"
                      << "---------------------------------\n";
        }
    }
}

static void processRegionOfInterest(asio::ip::tcp::socket& skt) {
    CalibrationApi::ToServer settingsRequest;
    auto settings = settingsRequest.mutable_set_region_of_interest();

    auto leftSettings = settings->add_regions();
    leftSettings->set_camera(CalibrationApi::Camera::LEFT);
    leftSettings->set_reversex(true);
    leftSettings->set_reversey(true);
    leftSettings->set_xoffset(0);
    leftSettings->set_yoffset(0);
    leftSettings->set_height(2060);
    leftSettings->set_width(1452);

    auto rightSettings = settings->add_regions();
    rightSettings->set_camera(CalibrationApi::Camera::RIGHT);
    rightSettings->set_reversex(false);
    rightSettings->set_reversey(false);
    rightSettings->set_xoffset(0);
    rightSettings->set_yoffset(0);
    rightSettings->set_height(2060);
    rightSettings->set_width(1452);

    std::string protoMsg = settingsRequest.SerializeAsString();
    std::string request = ncs::prependMessageLength(protoMsg);
    asio::write(skt, asio::buffer(request, request.size()));

    asio::error_code ec;  // Used to capture network errors cleanly

    std::uint32_t networkLen = 0;
    asio::read(skt, asio::buffer(&networkLen, sizeof(networkLen)), ec);
    if (ec) {
        std::cout << "Failed to read packet length: " << ec.message()
                  << std::endl;
        return;
    }

    std::uint32_t msgSize = ncs::decodeToHostByteOrder(networkLen);
    std::string responseData(msgSize, '\0');

    asio::read(skt, asio::buffer(&responseData[0], responseData.size()), ec);
    if (ec) {
        if (ec != asio::error::eof) {
            std::cout << "Network error during payload read: " << ec.message()
                      << std::endl;
        }
        return;
    }

    CalibrationApi::FromServer response;
    if (!response.ParseFromString(responseData)) {
        std::cout << "Could not decode packet" << std::endl;
        return;
    }

    if (response.has_region_of_interest()) {
        const auto& roi_response = response.region_of_interest();

        std::cout << "Set Region of Interest failed? "
                  << (roi_response.failures() ? "Yes" : "No") << "\n\n";

        std::cout << "--- Applied Camera Adjustments ---\n";

        for (int i = 0; i < roi_response.regions_size(); ++i) {
            const auto& target_settings = roi_response.regions(i);

            std::string camera_name = "UNKNOWN";
            switch (target_settings.camera()) {
                case CalibrationApi::Camera::LEFT:
                    camera_name = "LEFT";
                    break;
                case CalibrationApi::Camera::RIGHT:
                    camera_name = "RIGHT";
                    break;
                case CalibrationApi::Camera::WIDE:
                    camera_name = "WIDE";
                    break;
                default:
                    camera_name = "UNKNOWN_CAMERA";
                    break;
            }

            std::cout << "Camera: " << camera_name << "\n"
                      << "  X Offset: " << target_settings.xoffset() << "\n"
                      << "  Y Offset: " << target_settings.yoffset() << "\n"
                      << "  Width:    " << target_settings.width() << "\n"
                      << "  Height:   " << target_settings.height() << "\n"
                      << "---------------------------------" << std::endl;
        }
    }
}

static void processCoordinatedCapture(asio::ip::tcp::socket& skt) {
    CalibrationApi::ToServer ccRequest;
    ccRequest.mutable_get_synchronized_capture();

    std::string protoMsg = ccRequest.SerializeAsString();
    std::string request = ncs::prependMessageLength(protoMsg);
    std::cout << "Making request to get synchronized capture.\n";
    asio::write(skt, asio::buffer(request, request.size()));

    asio::streambuf receive_buffer;
    asio::error_code ec;

    std::uint32_t networkLen = 0;
    std::cout << "Waiting for response...\n";
    asio::read(skt, asio::buffer(&networkLen, sizeof(networkLen)));

    std::uint32_t msgSize = ncs::decodeToHostByteOrder(networkLen);
    std::string responseData(msgSize, '\0');

    asio::read(skt, asio::buffer(responseData.data(), responseData.size()));
    std::cout << "Full response retrieved.\n";

    if (ec && ec != asio::error::eof) {
        throw asio::system_error(ec);
    }

    CalibrationApi::FromServer resp;
    if (!resp.ParseFromString(responseData)) {
        std::cout << "Could not decode packet" << std::endl;
    }

    if (resp.has_synchronized_capture()) {
        auto left = resp.synchronized_capture().left();
        auto right = resp.synchronized_capture().right();
        std::cout << "Capture 1: " << left.source() << "-" << left.width()
                  << "x" << left.height() << std::endl;
        std::cout << "Capture 2: " << right.source() << "-" << right.width()
                  << "x" << right.height() << std::endl;

        // saveProtobufImageToFile(left,
        // "captures/dump/left_camera_frame.png");
        // saveProtobufImageToFile(right,
        // "captures/dump/right_camera_frame.png");
    }

    if (ec && ec != asio::error::eof) {
        throw asio::system_error(ec);
    }
}

static void processSetNirLedControls(asio::ip::tcp::socket& skt) {
    CalibrationApi::ToServer nirRequest;
    auto* nirControls = nirRequest.mutable_set_nir_led_controls();
    nirControls->mutable_controls()->set_top_intensity_percent(41);
    nirControls->mutable_controls()->set_bottom_intensity_percent(51);

    std::string protoMsg = nirRequest.SerializeAsString();
    std::string request = ncs::prependMessageLength(protoMsg);
    std::cout << "Setting NIR LED controls: top=41%, bot=51%\n";
    asio::write(skt, asio::buffer(request, request.size()));

    std::uint32_t networkLen = 0;
    std::cout << "Waiting for response...\n";
    asio::read(skt, asio::buffer(&networkLen, sizeof(networkLen)));

    std::uint32_t msgSize = ncs::decodeToHostByteOrder(networkLen);
    std::string responseData(msgSize, '\0');

    asio::read(skt, asio::buffer(responseData.data(), responseData.size()));
    std::cout << "Response retrieved.\n";

    CalibrationApi::FromServer resp;
    if (!resp.ParseFromString(responseData)) {
        std::cout << "Could not decode packet" << std::endl;
        return;
    }

    if (resp.has_nir_led_controls_response()) {
        const auto& nirResp = resp.nir_led_controls_response();
        std::cout << "Success: " << (nirResp.success() ? "true" : "false")
                  << std::endl;
        std::cout << "Message: " << nirResp.message() << std::endl;
    }
}

namespace fs = std::filesystem;

// When testing file based pngs this is useful.
// cppcheck-suppress unusedFunction
bool saveProtobufImageToFile(const CalibrationApi::CameraImage& cameraImage,
                             const std::string& filePathString) {
    try {
        fs::path targetPath(filePathString);
        fs::path parentDir = targetPath.parent_path();

        if (!parentDir.empty() && !fs::exists(parentDir)) {
            fs::create_directories(parentDir);
        }

        std::ofstream outFile(
            targetPath, std::ios::out | std::ios::binary | std::ios::trunc);
        if (!outFile) {
            std::cerr << "Error: Failed to open/create file at " << targetPath
                      << "\n";
            return false;
        }

        const std::string& rawData = cameraImage.image();
        if (rawData.empty()) {
            std::cerr << "Warning: The protobuf image payload is empty!\n";
            return false;
        }

        outFile.write(rawData.data(), rawData.size());

        outFile.close();
        if (!outFile.good()) {
            std::cerr << "Error: Something went wrong while flushing data "
                         "to disk.\n";
            return false;
        }

        std::cout << "Successfully saved/overwrote image (" << rawData.size()
                  << " bytes) to: " << targetPath << "\n";
        return true;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem Error: " << e.what() << "\n";
        return false;
    } catch (const std::exception& e) {
        std::cerr << "General Exception: " << e.what() << "\n";
        return false;
    }
}

static std::string loadBinaryFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) {
        throw std::runtime_error("Failed to open file: " + path);
    }

    const std::streamsize size = file.tellg();
    if (size < 0) {
        throw std::runtime_error("Failed to determine the file size: " + path);
    }

    std::string data(static_cast<size_t>(size), '\0');
    file.seekg(0, std::ios::beg);

    if (!file.read(data.data(), size)) {
        throw std::runtime_error("Failed to read file: " + path);
    }

    return data;
}