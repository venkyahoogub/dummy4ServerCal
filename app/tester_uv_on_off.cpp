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
static void processStartUvDiagnostic(asio::ip::tcp::socket& skt);
static void processCancelUvDiagnostic(asio::ip::tcp::socket& skt);

int main() {
    try {
        asio::io_context io_ctx;
        asio::ip::tcp::resolver resolver(io_ctx);
        auto endpoints = resolver.resolve("127.0.0.1", "50051");

        asio::ip::tcp::socket skt(io_ctx);
        asio::connect(skt, endpoints);

        std::cout << "Connected to server!" << std::endl;

        processAbout(skt);

        // Start the UV diode and override the duration by turning it off after 5 seconds.
        processStartUvDiagnostic(skt);
                std::cout << "Waiting for 5 seconds before calling UV cancel\n";
        // std::this_thread::sleep_for(std::chrono::seconds(5));
        // processCancelUvDiagnostic(skt);


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

static void processStartUvDiagnostic(asio::ip::tcp::socket& skt) {
    CalibrationApi::ToServer uvDiagRequest;
    auto* startUvDiag = uvDiagRequest.mutable_start_uv_diagnostic();
    startUvDiag->set_dac_counts(8000);
    startUvDiag->set_duration_ms(5000);

    std::string protoMsg = uvDiagRequest.SerializeAsString();
    std::string request = ncs::prependMessageLength(protoMsg);
    std::cout << "Starting UV diagnostic: dac_counts=8000, duration=180000 ms\n";
    std::cout << "If you wish to turn off UV before use the CLI\n";
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

    if (resp.has_uv_diagnostic_response()) {
        const auto& uvResp = resp.uv_diagnostic_response();
        std::cout << "Success: " << (uvResp.success() ? "true" : "false") << std::endl;
        std::cout << "Message: " << uvResp.message() << std::endl;
    }
}

static void processCancelUvDiagnostic(asio::ip::tcp::socket& skt) {
    CalibrationApi::ToServer cancelUvDiagRequest;
    cancelUvDiagRequest.mutable_cancel_uv_diagnostic();

    std::string protoMsg = cancelUvDiagRequest.SerializeAsString();
    std::string request = ncs::prependMessageLength(protoMsg);
    std::cout << "Cancelling UV diagnostic...\n";
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

    if (resp.has_uv_diagnostic_response()) {
        const auto& uvResp = resp.uv_diagnostic_response();
        std::cout << "Success: " << (uvResp.success() ? "true" : "false") << std::endl;
        std::cout << "Message: " << uvResp.message() << std::endl;
    }
}