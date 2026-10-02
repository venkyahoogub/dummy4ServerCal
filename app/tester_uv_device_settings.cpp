#include <common/SerializationUtils.h>
#include <neo-calibration-server-api/calibration_server_api.pb.h>

#include <asio.hpp>
#include <exception>
#include <iostream>
#include <string>
#include <chrono>
#include <thread>

static void processSetUvDeviceSettings(asio::ip::tcp::socket& skt);

int main() {
    try {
        asio::io_context io_ctx;
        asio::ip::tcp::resolver resolver(io_ctx);
        auto endpoints = resolver.resolve("127.0.0.1", "50051");

        asio::ip::tcp::socket skt(io_ctx);
        asio::connect(skt, endpoints);

        std::cout << "Connected to server!" << std::endl;

        processSetUvDeviceSettings(skt);

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

static void processSetUvDeviceSettings(asio::ip::tcp::socket& skt) {
    std::cout << "Running processSetUvDeviceSettings!" << std::endl;
    CalibrationApi::ToServer uvSettingsRequest;
    auto* uvSettings = uvSettingsRequest.mutable_set_uv_device_settings();
    uvSettings->set_pd1_gain_count(100);
    uvSettings->set_pd2_gain_count(1);
    uvSettings->set_pd3_gain_count(1);
    uvSettings->set_pi_gain_count(1);

    std::string protoMsg = uvSettingsRequest.SerializeAsString();
    std::string request = ncs::prependMessageLength(protoMsg);
    std::cout << "Setting UV device settings: pd1=100, pd2=1, pd3=1, pi=1\n";
    asio::write(skt, asio::buffer(request, request.size()));
    
    std::cout << "UV device settings sent.\n";
    
    // Give server time to apply settings
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Now request device settings to verify they were applied
    std::cout << "Requesting device settings...\n";
    CalibrationApi::ToServer getSettingsRequest;
    getSettingsRequest.mutable_get_device_settings();
    
    std::string getSettingsMsg = getSettingsRequest.SerializeAsString();
    std::string getSettingsReq = ncs::prependMessageLength(getSettingsMsg);
    asio::write(skt, asio::buffer(getSettingsReq, getSettingsReq.size()));

    std::uint32_t networkLen = 0;
    std::cout << "Waiting for device settings response...\n";
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

    if (resp.has_device_settings()) {
        const auto& settings = resp.device_settings();
        std::cout << "Device settings retrieved successfully!\n";
        std::cout << "  pd1_gain_count: " << settings.pd1_gain_count() << "\n";
        std::cout << "  pd2_gain_count: " << settings.pd2_gain_count() << "\n";
        std::cout << "  pd3_gain_count: " << settings.pd3_gain_count() << "\n";
        std::cout << "  pi_gain_count: " << settings.pi_gain_count() << "\n";
        std::cout << "  distance_sensor_gain: " << settings.distance_sensor_gain() << "\n";
        std::cout << "  distance_sensor_offset: " << settings.distance_sensor_offset() << "\n";
    } else {
        std::cout << "No device settings in response\n";
    }
}