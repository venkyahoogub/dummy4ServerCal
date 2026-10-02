// Implements
#include "SerializationUtils.h"

// Dependencies
#include <arpa/inet.h>  // If ever on windows, we may need to swap this include.

using namespace ncs;

std::uint32_t ncs::encodeToNetworkByteOrder(std::uint32_t value) {
    return htonl(value);
}

std::uint32_t ncs::decodeToHostByteOrder(std::uint32_t value) {
    return ntohl(value);
}

ByteString ncs::prependMessageLength(const ByteString& serialized) {
    std::uint32_t len = static_cast<std::uint32_t>(serialized.size());

    // Convert to Network Byte Order (Big-Endian)
    std::uint32_t networkLen = encodeToNetworkByteOrder(len);

    ByteString result;
    result.reserve(sizeof(networkLen) + serialized.size());
    result.append(reinterpret_cast<const char*>(&networkLen),
                  sizeof(networkLen));
    result.append(serialized);

    return result;
}