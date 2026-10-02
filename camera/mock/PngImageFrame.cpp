// Implements...
#include "PngImageFrame.h"

// Dependencies
#include <string>

#include "common/PngUtils.h"

using namespace ncs;

PngImageFrame::PngImageFrame(const std::string& source, std::string imageData,
                             ImageTimestamp timestamp)
    : mFormat(ImageFormat::PNG),
      mImageData(std::move(imageData)),
      mSource(source),
      mTimestamp(timestamp) {
    auto ridata = reinterpret_cast<const uint8_t* const>(mImageData.data());
    auto dimensions = decodePngDimensions(ridata, mImageData.length());
    mWidth = dimensions.width;
    mHeight = dimensions.height;
}