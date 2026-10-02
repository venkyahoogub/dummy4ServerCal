#include <common/SerializationUtils.h>
#include <neo-calibration-server-api/calibration_server_api.pb.h>

#include <asio.hpp>
#include <exception>
#include <filesystem>
#include <fstream>
#include <string>
#include <chrono>
#include <thread>

static void processAbout(asio::ip::tcp::socket& skt);
static void processSetNirLedControls(asio::ip::tcp::socket& skt);
static void processSetCameraTriggerControls(asio::ip::tcp::socket& skt);
static void processStartCalibration(asio::ip::tcp::socket& skt);

int main() {
    try {
        asio::io_context io_ctx;
        asio::ip::tcp::resolver resolver(io_ctx);
        auto endpoints = resolver.resolve("127.0.0.1", "50051");

        asio::ip::tcp::socket skt(io_ctx);
        asio::connect(skt, endpoints);

        std::cout << "Connected to server!" << std::endl;

        processAbout(skt);
        processSetNirLedControls(skt); 
        processSetCameraTriggerControls(skt);
        processStartCalibration(skt);

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

static void processSetCameraTriggerControls(asio::ip::tcp::socket& skt) {
    CalibrationApi::ToServer cameraTriggerRequest;
    auto* cameraTriggerControls = cameraTriggerRequest.mutable_set_camera_trigger_controls();
    cameraTriggerControls->mutable_controls()->set_frequency_hz(6);
    cameraTriggerControls->mutable_controls()->set_duration_ms(60);

    std::string protoMsg = cameraTriggerRequest.SerializeAsString();
    std::string request = ncs::prependMessageLength(protoMsg);
    std::cout << "Setting Camera trigger controls: frequency=6 hz, duration=60 ms\n";
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

    if (resp.has_camera_trigger_controls_response()) {
        const auto& cameraTriggerResp = resp.camera_trigger_controls_response();
        std::cout << "Success: " << (cameraTriggerResp.success() ? "true" : "false") << std::endl;
        std::cout << "Message: " << cameraTriggerResp.message() << std::endl;
    }
}

static void processStartCalibration(asio::ip::tcp::socket& skt) {
    CalibrationApi::ToServer calibrationRequest;
    auto* startCalibration = calibrationRequest.mutable_start_calibration();
    startCalibration->set_dac_count(6000);

    std::string protoMsg = calibrationRequest.SerializeAsString();
    std::string request = ncs::prependMessageLength(protoMsg);
    std::cout << "Starting calibration: dac_count=6000\n";
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

    if (resp.has_calibration_response()) {
        const auto& calibResp = resp.calibration_response();
        std::cout << "Success: " << (calibResp.success() ? "true" : "false") << std::endl;
        std::cout << "Message: " << calibResp.message() << std::endl;
    }
}