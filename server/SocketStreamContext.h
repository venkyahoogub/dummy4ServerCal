#pragma once

// Dependencies
#include <asio.hpp>

#include "IStreamContext.h"

namespace ncs {
class SocketStreamContext : public IStreamContext {
   public:
    using asiotcp = asio::ip::tcp;
    explicit SocketStreamContext(asiotcp::socket& socket) : mSocket(socket) {}

    void send(const ByteString& bytes) override {
        asio::write(mSocket, asio::buffer(bytes.data(), bytes.size()));
    }

   private:
    asiotcp::socket& mSocket;
};
}  // namespace ncs