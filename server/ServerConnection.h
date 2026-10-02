#pragma once

#include <asio.hpp>
#include <atomic>
#include <memory>
#include <system_error>

#include "IServiceRouter.h"
#include "common/TracedError.h"

namespace ncs {
using asiotcp = asio::ip::tcp;

/**
 * @class ServiceConnection
 * @brief Represents a single connection over a socket.
 *
 * This connection handler processes incoming requests and routes them to the
 * appropriate service. It supports both request/response and streaming patterns
 * through IStreamContext.
 */
class ServiceConnection {
   public:
    ServiceConnection(asio::io_context& context, IServiceRouter& pRouter);

    ~ServiceConnection();

    /**
     * @brief Processes data coming over the socket in a request/response loop.
     *
     * For regular requests, responses are sent back directly.
     * For streaming requests (e.g., video), the route handler uses sendContext
     * to continuously send frames without blocking this loop.
     */
    void process();

    /**
     * @brief Grants mutable access to the socket.
     */
    inline asiotcp::socket& socket() { return mSocket; }

   private:
    asiotcp::socket mSocket;
    IServiceRouter& mRouter;
};
}  // namespace ncs
