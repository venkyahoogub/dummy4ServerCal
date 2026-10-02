// Implements
#include "ServerConnection.h"

// Dependencies
#include <atomic>
#include <iostream>
#include <memory>

#include "SocketStreamContext.h"

using namespace ncs;

ServiceConnection::ServiceConnection(asio::io_context& context,
                                     IServiceRouter& pRouter)
    : mSocket(context), mRouter(pRouter) {}

ServiceConnection::~ServiceConnection() {}

void ServiceConnection::process() {
    while (mSocket.is_open()) {
        std::uint32_t networkLen = 0;

        try {
            asio::read(mSocket, asio::buffer(&networkLen, sizeof(networkLen)));
        } catch (const std::exception& e) {
            std::cerr << "Error reading message length: " << e.what() << "\n";
            break;
        }

        std::uint32_t requestLength = ncs::decodeToHostByteOrder(networkLen);

        // TODO: Guard against DOS that might send large lengths.
        // TODO: Guard against excessively long reads or responses that may
        // never come.

        std::string requestData(requestLength, '\0');

        try {
            asio::read(mSocket, asio::buffer(requestData, requestData.size()));
        } catch (const std::exception& e) {
            std::cerr << "Error reading request data: " << e.what() << "\n";
            break;
        }

        SocketStreamContext sendContext(mSocket);

        try {
            std::string responseData =
                mRouter.getResponse(requestData, sendContext);

            // If response is not empty, send it (streaming responses are
            // handled by the route via sendContext)
            if (!responseData.empty()) {
                responseData = ncs::prependMessageLength(responseData);
                asio::write(mSocket, asio::buffer(responseData.data(),
                                                  responseData.size()));
            }
        } catch (const std::exception& e) {
            std::cerr << "Error processing request: " << e.what() << "\n";
            break;
        }
    }
}
