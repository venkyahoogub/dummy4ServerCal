// Implements
#include "CalibrationServer.h"

// Dependencies
#include "ServerConnection.h"

using namespace ncs;

CalibrationServer::CalibrationServer(std::uint32_t port, IServiceRouter& router)
    : mStatus(Status::Stopped),
      mRouter(router),
      mIoContext(),
      mAcceptor(mIoContext,
                asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)) {}

CalibrationServer::~CalibrationServer() { stop(); }

void CalibrationServer::start() {
    if (isInNonInterruptibleStatus()) {
        throw std::runtime_error("Server is already starting or running");
    }

    mStatus = Status::Starting;

    if (mIoContext.stopped()) {
        mIoContext.restart();
    }

    setupConnection();
    try {
        mStatus = Status::Running;
        mIoContext.run();
    } catch (const std::exception& e) {
        mStatus = Status::Error;
        throw;
    }

    if (mStatus != Status::Error) {
        mStatus = Status::Stopped;
    }
}

void CalibrationServer::stop() {
    if (mStatus == Status::Stopped || mStatus == Status::Stopping) return;

    mStatus = Status::Stopping;
    mAcceptor.close();
    mIoContext.stop();
    mStatus = Status::Stopped;
}

CalibrationServer::Status CalibrationServer::getStatus() const {
    return mStatus.load();
}

// Helpers
void CalibrationServer::setupConnection() {
    auto pConnection = std::make_shared<ServiceConnection>(mIoContext, mRouter);

    // The lambda capture will keep the socket and connection object alive.
    mAcceptor.async_accept(
        pConnection->socket(), [this, pConnection](std::error_code ec) {
            // Allow for multiple connetions. Immediately sets up the next
            // listener anticipating a connection.
            this->setupConnection();

            if (!ec) {
                try {
                    pConnection->process();
                } catch (const asio::system_error& e) {
                    if (e.code() != asio::error::eof &&
                        e.code() != asio::error::connection_reset) {
                        throw;  // It's a genuine internal fault or unhandled
                                // network error
                    }
                    // TODO: The connection was closed either gracefully or
                    // abruptly client-side. Probably just want to log this.
                }
            }
        });
}

bool CalibrationServer::isInNonInterruptibleStatus() {
    return mStatus == Status::Running || mStatus == Status::Starting ||
           mStatus == Status::Stopping;
}
