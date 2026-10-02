// Implements...
#include "BaslerImageFrame.h"

#include <stdexcept>

using namespace ncs;

BaslerImageFrame::BaslerImageFrame(const std::string& source,
                                   Pylon::CGrabResultPtr grabResult)
    : mGrabResult(grabResult), mSource(source) {
    // TODO: Better errors.
    // if (!mGrabResult.IsValid())
    // if (!mGrabResult->GrabSucceeded())

    mFormat = mapPylonFormat(mGrabResult->GetPixelType());
    mTimestamp = extractTimestamp(mGrabResult);
}

std::uint32_t BaslerImageFrame::width() const {
    return mGrabResult->GetWidth();
}

std::uint32_t BaslerImageFrame::height() const {
    return mGrabResult->GetHeight();
}

const uint8_t* BaslerImageFrame::data() const {
    return static_cast<const uint8_t*>(mGrabResult->GetBuffer());
}

size_t BaslerImageFrame::size() const { return mGrabResult->GetImageSize(); }

ImageFormat BaslerImageFrame::mapPylonFormat(Pylon::EPixelType pixelType) {
    switch (pixelType) {
        case Pylon::PixelType_Mono8:
            return ImageFormat::Mono8;
        case Pylon::PixelType_BayerGB8:
            return ImageFormat::GB8;
        default:
            return ImageFormat::Unknown;  // TODO: Maybe throw an error?
    }
}

ImageTimestamp BaslerImageFrame::extractTimestamp(
    const Pylon::CGrabResultPtr& grabResult) {
    // In Pylon, a timestamp of 0 means no hardware timestamp was attached by
    // the camera
    if (grabResult->GetTimeStamp() > 0) {
        uint64_t pylonTicks = grabResult->GetTimeStamp();

        // Pylon hardware timestamps are natively represented in nanoseconds for
        // modern Basler cameras
        auto duration = std::chrono::nanoseconds(pylonTicks);
        return ncs::ImageTimestamp(
            std::chrono::duration_cast<ncs::ImageTimestamp::duration>(
                duration));
    }

    // Fallback to the host computer's system time if the camera didn't provide
    // a valid hardware tick
    return std::chrono::system_clock::now();
}