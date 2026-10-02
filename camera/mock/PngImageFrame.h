#pragma once

// Dependencies
#include <memory>
#include <string>

#include "../IImageFrame.h"

namespace ncs {
/**
 * @class PngImageFrame
 * @brief A concrete implementation of IImageFrame that stores image data as an
 * encoded PNG string.
 *
 * This class acts as an immutable container for PNG network buffers or
 * disk-loaded images, fulfilling the IImageFrame interface to allow uniform
 * processing down the pipeline.
 */
class PngImageFrame : public IImageFrame {
   public:
    /**
     * @brief Constructs a PngImageFrame by consuming or copying the source
     * parameters and image buffer.
     *
     * @param source The identifier or origin of this image (e.g., camera name,
     * file path).
     * @param image The raw encoded string payload containing the complete PNG
     * binary data.
     * @param timestamp The system clock time point at which the image was
     * generated or captured.
     */
    PngImageFrame(const std::string& source, std::string image,
                  ImageTimestamp timestamp);

    /**
     * @brief Gets the identifier or origin string of the image source.
     * @return A reference to the underlying source identifier string.
     */
    const std::string& source() const override { return mSource; }

    /**
     * @brief Gets the horizontal resolution of the image frame.
     * @return The frame width in pixels.
     */
    std::uint32_t width() const override { return mWidth; }

    /**
     * @brief Gets the vertical resolution of the image frame.
     * @return The frame height in pixels.
     */
    std::uint32_t height() const override { return mHeight; }

    /**
     * @brief Gets the explicit encoding format identifier for this frame.
     * @return ImageFormat::PNG.
     */
    ImageFormat format() const override { return mFormat; }

    /**
     * @brief Gets the timestamp indicating when the frame was acquired.
     * @return The ImageTimestamp assigned at construction.
     */
    ImageTimestamp timestamp() const override { return mTimestamp; }

    /**
     * @brief Accesses the underlying data string as a contiguous sequence of
     * raw bytes.
     *
     * @note This performs a safe reinterpret_cast from `const char*` to `const
     * uint8_t*`. The lifetime of this raw pointer is tied entirely to the
     * lifecycle of this object instance.
     * @return A constant pointer to the first byte of the raw PNG buffer data.
     */
    const uint8_t* data() const override {
        return reinterpret_cast<const uint8_t*>(mImageData.data());
    }

    /**
     * @brief Gets the total memory footprint of the PNG image buffer.
     * @return The size of the data string buffer in bytes.
     */
    size_t size() const override { return mImageData.length(); }

   private:
    std::string mSource;
    std::uint32_t mWidth;
    std::uint32_t mHeight;
    ImageFormat mFormat;
    ImageTimestamp mTimestamp;
    std::string mImageData;
};
}  // namespace ncs