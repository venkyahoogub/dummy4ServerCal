// Implements
#include "PngUtils.h"

// Dependencies
#include <arpa/inet.h>  // Might be in a different location on windows.

#include <cstring>
#define PNG_HEADER_SIZE 24

using namespace ncs;

PngMetadata ncs::decodePngDimensions(const std::uint8_t* const buffer,
                                     size_t fileSize) {
    PngMetadata dimensions;
    dimensions.height = 0;
    dimensions.width = 0;

    // The PNG specification guarantees that the width and height are always
    // stored in the exact same byte positions right at the beginning of the
    // file (inside the IHDR chunk).
    if (fileSize >= PNG_HEADER_SIZE) {
        // Read the width (bytes 16-19) and height (bytes 20-23) from the buffer
        uint32_t rawWidth, rawHeight;
        std::memcpy(&rawWidth, &buffer[16], 4);
        std::memcpy(&rawHeight, &buffer[20], 4);

        // PNG files store integers in Big-Endian format, so we must swap them
        // to Little-Endian
        dimensions.width = ntohl(rawWidth);
        dimensions.height = ntohl(rawHeight);
    }

    return dimensions;
}