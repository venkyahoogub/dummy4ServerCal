#include <common/SerializationUtils.h>
#include <neo-calibration-server-api/calibration_server_api.pb.h>

#include <asio.hpp>
#include <exception>
#include <iostream>
#include <string>
#include <chrono>
#include <thread>

static void processSetDistanceSensorDeviceSettings(asio::ip::tcp::socket& skt);
static void processDistanceSensorStartRanging(asio::ip::tcp::socket& skt);
static void processDistanceSensorStopRanging(asio::ip::tcp::socket& skt);

int main() {
    try {
        asio::io_context io_ctx;
        asio::ip::tcp::resolver resolver(io_ctx);
        auto endpoints = resolver.resolve("127.0.0.1", "50051");

        asio::ip::tcp::socket skt(io_ctx);
        asio::connect(skt, endpoints);

        std::cout << "Connected to server!" << std::endl;

        processSetDistanceSensorDeviceSettings(skt);
        // Using 2 ranging calls to confirm they are within tolerance
        processDistanceSensorStartRanging(skt);
        std::this_thread::sleep_for(std::chrono::seconds(3));
        processDistanceSensorStartRanging(skt);

        // Stopping the ranging once done with tests, to avoid log clogging.
        processDistanceSensorStopRanging(skt);

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

static void processDistanceSensorStartRanging(asio::ip::tcp::socket& skt) {
    std::cout << "Running processDistanceSensorStartRanging!" << std::endl;
    CalibrationApi::ToServer distanceSensorRequest;
    distanceSensorRequest.mutable_distance_sensor_start_ranging();

    std::string protoMsg = distanceSensorRequest.SerializeAsString();
    std::string request = ncs::prependMessageLength(protoMsg);
    std::cout << "Starting distance sensor ranging...\n";
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

    if (resp.has_distance_sensor_ranging_result()) {
        const auto& distanceResp = resp.distance_sensor_ranging_result();
        std::cout << "Distance to target: " << distanceResp.distance_to_target_mm() << " mm\n";
    }
}

static void processDistanceSensorStopRanging(asio::ip::tcp::socket& skt) {
    std::cout << "Running processDistanceSensorStopRanging!" << std::endl;
    CalibrationApi::ToServer distanceSensorRequest;
    distanceSensorRequest.mutable_distance_sensor_stop_ranging();

    std::string protoMsg = distanceSensorRequest.SerializeAsString();
    std::string request = ncs::prependMessageLength(protoMsg);
    std::cout << "Stopping distance sensor ranging...\n";
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

    if (resp.has_distance_sensor_ranging_result()) {
        const auto& distanceResp = resp.distance_sensor_ranging_result();
        std::cout << "Final distance to target: " << distanceResp.distance_to_target_mm() << " mm\n";
    }
}

static void processSetDistanceSensorDeviceSettings(asio::ip::tcp::socket& skt) {
    std::cout << "Running processSetDistanceSensorDeviceSettings!" << std::endl;
    CalibrationApi::ToServer distanceSensorSettingsRequest;
    auto* distanceSensorSettings = distanceSensorSettingsRequest.mutable_set_distance_sensor_device_settings();
    distanceSensorSettings->set_gain(1);
    distanceSensorSettings->set_offset(1);

    std::string protoMsg = distanceSensorSettingsRequest.SerializeAsString();
    std::string request = ncs::prependMessageLength(protoMsg);
    std::cout << "Setting distance sensor device settings: gain=1, offset=1\n";
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

    std::cout << "Distance sensor device settings applied.\n";
}