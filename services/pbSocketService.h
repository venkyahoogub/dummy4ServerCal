#pragma once

#include <asio.hpp>
#include <cstdint>
#include <string>
#include <vector>

class pbSocketService {
   public:
    pbSocketService(const std::string& ip, uint16_t port);
    void sendMessage(const std::string& data);
    std::vector<uint8_t> receiveMessage();

   private:
    asio::io_context ioContext_;
    asio::ip::tcp::socket socket_;
};