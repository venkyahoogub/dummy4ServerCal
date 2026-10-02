#pragma once

// Dependencies
#include <cstddef>
#include <cstdint>

namespace ncs {
/**
 * @brief Common metadata such as width and height.
 */
struct PngMetadata {
   public:
    std::uint32_t width;
    std::uint32_t height;
};

/**
 * @brief Looks at the header data for a PNG and pulls any useful
 * metadata from it.
 */
PngMetadata decodePngDimensions(const std::uint8_t* const buffer,
                                size_t fileSize);
}  // namespace ncs
