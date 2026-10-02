#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>

namespace ncs {
/**
 * @enum ImageFormat
 * @brief Supported pixel data layouts and compression formats for the image
 * frames.
 */
enum class ImageFormat {
    Mono8,  /**< 8-bit grayscale uncompressed format. */
    GB8,    /**< 8-bit Green/Blue Bayer pattern format. */
    PNG,    /**< Compressed Portable Network Graphics format. */
    Unknown /**< Fallback or unsupported format identifier. */
};

/**
 * @brief Alias for the system clock time point used to timestamp individual
 * image frames.
 */
using ImageTimestamp = std::chrono::system_clock::time_point;

/**
 * @class IImageFrame
 * @brief Abstract interface representing a single captured video or image
 * frame.
 *
 * This interface decouples backend image sources (e.g., OpenCV, system camera
 * APIs, or disk-loaded images) from the core calibration processing pipeline.
 */
class IImageFrame {
   public:
    virtual ~IImageFrame() = default;

    /**
     * @brief Gets the identifier or origin string of the image source.
     * @return A reference to a string identifying the hardware camera, topic,
     * or file path.
     */
    virtual const std::string& source() const = 0;

    /**
     * @brief Gets the horizontal resolution of the image frame.
     * @return The frame width in pixels.
     */
    virtual std::uint32_t width() const = 0;

    /**
     * @brief Gets the vertical resolution of the image frame.
     * @return The frame height in pixels.
     */
    virtual std::uint32_t height() const = 0;

    /**
     * @brief Gets the specific image encoding format of the internal buffer.
     * @return The ImageFormat enum value representing the pixel layout.
     */
    virtual ImageFormat format() const = 0;

    /**
     * @brief Gets the timestamp indicating when the frame was acquired.
     * @return The ImageTimestamp of the frame.
     */
    virtual ImageTimestamp timestamp() const = 0;

    /**
     * @brief Accesses the raw byte buffer containing the underlying image data.
     * @note The lifetime of the returned memory buffer is bound to this
     * IImageFrame instance.
     * @return A constant pointer to the raw byte sequence.
     */
    virtual const uint8_t* data() const = 0;

    /**
     * @brief Gets the total memory footprint of the raw image buffer.
     * @return The size of the data buffer in bytes.
     */
    virtual size_t size() const = 0;
};
}  // namespace ncs