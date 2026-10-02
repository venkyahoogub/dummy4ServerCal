#pragma once

// Dependencies
#include <asio.hpp>
#include <atomic>
#include <cstdint>
#include <future>
#include <memory>
#include <thread>

#include "IServiceRouter.h"

namespace ncs {
/**
 * @class CalibrationServer
 * @brief Manages the lifecycle and network interface for device calibration
 * services.
 *
 * This server listens on a specified port and routes incoming calibration
 * requests via the provided IServiceRouter.
 */
class CalibrationServer {
   public:
    /**
     * @class Status
     * @brief The valid statuses the Calibration Server can have.
     */
    enum class Status {
        Starting, /**< The service is performing initialization logic. */
        Running,  /**< The service is active and processing tasks. */
        Stopping, /**< A shutdown signal was received; cleaning up resources. */
        Stopped,  /**< The service has gracefully terminated. */
        Error     /**< An unrecoverable failure has occurred. */
    };

    /**
     * @brief Constructs a new Calibration Server instance.
     *
     * @param port The TCP port on which the server will listen for connections.
     * @param router A reference to the service router used to dispatch
     * calibration commands.
     */
    CalibrationServer(std::uint32_t port, IServiceRouter& router);
    ~CalibrationServer();

    /**
     * @brief Retrieves the current operational state of the server.
     * @return The current #Status (e.g., Starting, Running, or Error).
     */
    Status getStatus() const;

    /**
     * @brief Starts the server's listener and processing loops.
     */
    void start();

    /**
     * @brief Signals the server to stop and releases network resources.
     *
     * This is a synchronous call that initiates the shutdown sequence.
     * Use getStatus() to verify when the server has reached the Stopped state.
     */
    void stop();

   private:
    // Helpers
    void setupConnection();  // Setup a connection (or next connection).
    bool isInNonInterruptibleStatus();

    using asiotcp = asio::ip::tcp;
    std::atomic<Status> mStatus;
    IServiceRouter& mRouter;
    asio::io_context mIoContext;
    asiotcp::acceptor mAcceptor;
};
}  // namespace ncs