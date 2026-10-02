#pragma once

// Dependencies
#include <pylon/InstantCamera.h>

#include "../IImageFrame.h"

namespace ncs {
class BaslerImageFrame : public IImageFrame {
   public:
    explicit BaslerImageFrame(const std::string& source,
                              Pylon::CGrabResultPtr grabResult);
    ~BaslerImageFrame() override = default;

    /**
     * @brief Gets the identifier or origin string of the image source.
     * @return A reference to a string identifying the hardware camera, topic,
     * or file path.
     */
    const std::string& source() const override { return mSource; }

    /**        /**
     * @brief Gets the total memory footprint of the raw image buffer.
     * @return The size of the data buffer in bytes.
     *
     * @brief Gets the horizontal resolution of the image frame.
     * @return The frame width in pixels.
     */
    std::uint32_t width() const override;

    /**
     * @brief Gets the vertical resolution of the image frame.
     * @return The frame height in pixels.
     */
    std::uint32_t height() const override;

    /**
     * @brief Gets the specific image encoding format of the internal buffer.
     * @return The ImageFormat enum value representing the pixel layout.
     */
    inline ImageFormat format() const override { return mFormat; }

    /**
     * @brief Gets the timestamp indicating when the frame was acquired.
     * @return The ImageTimestamp of the frame.
     */
    inline ImageTimestamp timestamp() const override { return mTimestamp; }

    /**
     * @brief Accesses the raw byte buffer containing the underlying image data.
     * @note The lifetime of the returned memory buffer is bound to this
     * IImageFrame instance.
     * @return A constant pointer to the raw byte sequence.
     */
    const uint8_t* data() const override;

    /**
     * @brief Gets the total memory footprint of the raw image buffer.
     * @return The size of the data buffer in bytes.
     */
    size_t size() const override;

   private:
    Pylon::CGrabResultPtr mGrabResult;
    ImageFormat mFormat;
    ImageTimestamp mTimestamp;
    std::string mSource;

    // Helper to map Pylon pixel types to ImageFormat enum
    static ImageFormat mapPylonFormat(Pylon::EPixelType pixelType);

    // Helper to extract Pylon grab timestamp to std::chrono
    static ImageTimestamp extractTimestamp(
        const Pylon::CGrabResultPtr& grabResult);
};
}  // namespace ncs