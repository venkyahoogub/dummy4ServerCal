#pragma once

// Dependencies
#include <cstdint>
#include <string>

namespace ncs {
/**
 * @brief Represents a string of bytes.
 *
 * Serializaton libraries (such as protobuf) often use strings
 * and characters as useful representations of bytes.
 */
using ByteString = std::string;

/**
 * @brief Prepends the length of the message at the beginning of the message.
 * @param serialized The message to check and append the length to.
 *
 * This will help retrieval over the network by allowing the consumers to first
 * read the first four bytes to know how long the message is. Otherwise all
 * consumers will be forced to wait for a signal such as the
 * socket closing or a terminating character.
 */
ByteString prependMessageLength(const ByteString& serialized);

/**
 * @brief Encodes to a common network byte order.
 * @param value A native length value to convert to network byte order.
 */
std::uint32_t encodeToNetworkByteOrder(std::uint32_t value);

/**
 * @brief Converts endiness after going over the network.
 * @param value A value in network byte order.
 */
std::uint32_t decodeToHostByteOrder(std::uint32_t value);
}  // namespace ncs