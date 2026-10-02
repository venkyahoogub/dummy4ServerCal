#pragma once

// Dependencies
#include "IStreamContext.h"
#include "common/TracedError.h"

namespace ncs {
/**
 * @class IServiceRouter
 * @brief Takes the raw bytes from the server, decodes them and then picks
 * the backend service to process the message.
 */
class IServiceRouter {
   public:
    virtual ~IServiceRouter() = default;

    /**
     * @brief Reads the header of the request and then routes the data to
     * the correct module to get a response.
     * @throws RoutingError, NoRouteFoundError, ParseError
     */
    virtual ByteString getResponse(const ByteString& rawData,
                                   IStreamContext& streamContext) = 0;

    //
    // Errors that are tied to the interface. Consumers should check for these
    // and implementations should throw only variations of these.
    //

    /**
     * @class RoutingError
     * @brief Base exception for all failures occurring within the Service
     * Router.
     *
     * Use this class to catch any generic routing failure. It ensures that
     * a stack trace is captured and a descriptive message is provided.
     */
    class RoutingError : public TracedError {
       public:
        explicit RoutingError()
            : TracedError(
                  "Could not route the request to the proper service.") {}

        using TracedError::TracedError;
    };

    /**
     * @class NoRouteFoundError
     * @brief Thrown when the router cannot find a service mapped to a specific
     * message type.
     */
    class NoRouteFoundError : public RoutingError {
       public:
        explicit NoRouteFoundError(std::uint32_t msgType)
            : RoutingError("No route found for message type: " +
                           std::to_string(msgType)) {}
    };

    /**
     * @class ParseError
     * @brief Thrown when the incoming ByteString is malformed or cannot be
     * decoded.
     */
    class ParseError : public RoutingError {
       public:
        explicit ParseError(const ByteString& requestData)
            : RoutingError("Could not parse the request."),
              mRequestData(
                  // To keep the data predictable and lightweight, only copy
                  // part of the message.
                  requestData.begin(),
                  requestData.begin() +
                      std::min<std::size_t>(requestData.size(), 128)) {}

        /**
         * @brief The offending bytes.
         */
        [[nodiscard]]
        inline const ByteString& requestData() const noexcept {
            return mRequestData;
        }

       private:
        ByteString mRequestData;
    };
};
}  // namespace ncs