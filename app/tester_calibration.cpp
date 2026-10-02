#include <common/SerializationUtils.h>
#include <neo-calibration-server-api/calibration_server_api.pb.h>

#include <asio.hpp>
#include <exception>
#include <string>
#include <iostream>
#include <iomanip>
#include <vector>

static void sendMessage(asio::ip::tcp::socket& skt,
                       const CalibrationApi::ToServer& request) {
    std::string protoMsg = request.SerializeAsString();
    std::string message = ncs::prependMessageLength(protoMsg);
    asio::write(skt, asio::buffer(message, message.size()));
}

static CalibrationApi::FromServer receiveMessage(asio::ip::tcp::socket& skt) {
    std::uint32_t networkLen = 0;
    asio::read(skt, asio::buffer(&networkLen, sizeof(networkLen)));

    std::uint32_t msgSize = ncs::decodeToHostByteOrder(networkLen);
    std::string responseData(msgSize, '\0');
    asio::read(skt, asio::buffer(responseData.data(), responseData.size()));

    CalibrationApi::FromServer resp;
    if (!resp.ParseFromString(responseData)) {
        throw std::runtime_error("Could not decode response packet");
    }
    return resp;
}

int main() {
    try {
        asio::io_context io_ctx;
        asio::ip::tcp::resolver resolver(io_ctx);
        auto endpoints = resolver.resolve("127.0.0.1", "50051");

        asio::ip::tcp::socket skt(io_ctx);
        asio::connect(skt, endpoints);

        std::cout << "Connected to server!" << std::endl << std::endl;

        // Print header
        std::cout << std::left 
                  << std::setw(8) << "mW/cm2" << " | "
                  << std::setw(8) << "DAC" << " | "
                  << std::setw(8) << "PD1" << " | "
                  << std::setw(8) << "PD2" << " | "
                  << std::setw(8) << "PD3" << " | "
                  << std::setw(8) << "Current"
                  << std::endl;
        std::cout << std::string(70, '-') << std::endl;

        std::vector<CalibrationApi::CalibrationTableEntry> calibrationTable;

        // Calibrate from irradiance 3 to 10
        for (uint32_t irradiance = 3; irradiance <= 10; ++irradiance) {
            // Calculate DAC: starting at 1212, increment by 100 for each step
            uint32_t dac = 1212 + ((irradiance - 3) * 100);

            CalibrationApi::ToServer toServer;
            auto* msg = toServer.mutable_start_calibration();
            msg->set_irradiance_mw_per_cm2(irradiance);
            msg->set_dac_count(dac);

            sendMessage(skt, toServer);
            auto resp = receiveMessage(skt);

            if (resp.has_calibration_result()) {
                const auto& entry = resp.calibration_result().entry();
                calibrationTable.push_back(entry);

                // Print the results
                std::cout << std::left
                          << std::setw(8) << irradiance << " | "
                          << std::setw(8) << entry.dac_count() << " | "
                          << std::setw(8) << entry.pd1_count() << " | "
                          << std::setw(8) << entry.pd2_count() << " | "
                          << std::setw(8) << entry.pd3_count() << " | "
                          << std::setw(8) << entry.current_count()
                          << std::endl;
            }
        }

        std::cout << std::string(70, '-') << std::endl;
        std::cout << "\n✓ Calibration test completed. " << calibrationTable.size() 
                  << " points recorded." << std::endl;

        {
            CalibrationApi::ToServer uploadRequest;
            auto* upload = uploadRequest.mutable_upload_calibration_table();
            auto* calibTable = upload->mutable_table();

            for (const auto& entry : calibrationTable) {
                auto* newEntry = calibTable->add_entries();
                newEntry->set_irradiance_mw_per_cm2(entry.irradiance_mw_per_cm2());
                newEntry->set_dac_count(entry.dac_count());
                newEntry->set_pd1_count(entry.pd1_count());
                newEntry->set_pd2_count(entry.pd2_count());
                newEntry->set_pd3_count(entry.pd3_count());
                newEntry->set_current_count(entry.current_count());
            }

            sendMessage(skt, uploadRequest);
            auto resp = receiveMessage(skt);

            if (resp.has_upload_calibration_table_response()) {
                const auto& uploadResp = resp.upload_calibration_table_response();
                std::cout << "Upload Status: " << (uploadResp.success() ? "SUCCESS" : "FAILED") << std::endl;
                if (!uploadResp.message().empty()) {
                    std::cout << "Message: " << uploadResp.message() << std::endl;
                }
            }
        }

        std::cout << "\nPress Enter to close...";
        std::cin.get();

        skt.shutdown(asio::ip::tcp::socket::shutdown_both);
        skt.close();

    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}