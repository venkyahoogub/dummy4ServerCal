#include <asio.hpp>

#include <exception>
#include <fstream>
#include <filesystem>
#include <string>
#include <neo-calibration-server-api/calibration_server_api.pb.h>
#include <common/SerializationUtils.h>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>

static std::string displayFormat(CalibrationApi::CameraImageFormat format);
static void processAbout(asio::ip::tcp::socket& skt);
static void processHomeMotors(asio::ip::tcp::socket& skt);

int main() {
    try 
    {
        asio::io_context io_ctx;
        asio::ip::tcp::resolver resolver(io_ctx);
        auto endpoints = resolver.resolve("127.0.0.1", "50051");

        asio::ip::tcp::socket skt(io_ctx);
        asio::connect(skt, endpoints);

        std::cout << "Connected to server!" << std::endl;

        processAbout(skt);
        processHomeMotors(skt); 
        
        skt.shutdown(asio::ip::tcp::socket::shutdown_both);
        skt.close();

    } 
    catch (std::exception& e)
    {
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
    if (ec && ec != asio::error::eof) 
    {
        throw asio::system_error(ec);
    }

    CalibrationApi::FromServer response;
    if (!response.ParseFromString(responseData)) 
    {
        std::cout << "Could not decode packet" << std::endl;
    }

    if (response.has_about()) 
    {
        std::cout << "Version is: " << response.about().version() << std::endl;
    }    

    if (ec && ec != asio::error::eof) 
    {
        throw asio::system_error(ec);
    }    
}

static void processHomeMotors(asio::ip::tcp::socket& skt) {
    CalibrationApi::ToServer homeMotorsRequest;
    homeMotorsRequest.mutable_home_motors();

    std::string protoMsg = homeMotorsRequest.SerializeAsString();
    std::string request = ncs::prependMessageLength(protoMsg);
    std::cout << "Homing motors...\n";
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

    if (resp.has_motors_homed()) {
        std::cout << "Motors homed successfully!" << std::endl;
    }
}