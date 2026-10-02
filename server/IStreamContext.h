#pragma once

// Dependencies
#include "common/SerializationUtils.h"

namespace ncs {
/**
 * @brief A context for streaming actions to be able to
 * continuously stream instead of only expecting the
 * request/response pattern.
 */
class IStreamContext {
   public:
    virtual ~IStreamContext() = default;

    /**
     * Send the bytes over the connection.
     */
    virtual void send(const ByteString& bytes) = 0;
};
}  // namespace ncs