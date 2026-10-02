#include "pbSocketService.h"

#include <cstring>

pbSocketService::pbSocketService(const std::string& ip, uint16_t port)
    : ioContext_(), socket_(ioContext_) {
    asio::ip::tcp::resolver resolver(ioContext_);
    auto endpoints = resolver.resolve(ip, std::to_string(port));
    asio::connect(socket_, endpoints);
}

void pbSocketService::sendMessage(const std::string& data) {
    uint32_t len = static_cast<uint32_t>(data.size());
    std::vector<uint8_t> packet;
    packet.resize(4 + data.size());

    // little-endian length prefix
    packet[0] = len & 0xFF;
    packet[1] = (len >> 8) & 0xFF;
    packet[2] = (len >> 16) & 0xFF;
    packet[3] = (len >> 24) & 0xFF;

    std::copy(data.begin(), data.end(), packet.begin() + 4);

    asio::write(socket_, asio::buffer(packet));
}

std::vector<uint8_t> pbSocketService::receiveMessage() {
    uint8_t lenBuf[4];
    asio::read(socket_, asio::buffer(lenBuf, 4));
    uint32_t len =
        lenBuf[0] | (lenBuf[1] << 8) | (lenBuf[2] << 16) | (lenBuf[3] << 24);
    std::vector<uint8_t> data(len);
    asio::read(socket_, asio::buffer(data.data(), len));

    return data;
}