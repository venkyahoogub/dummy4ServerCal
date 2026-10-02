#pragma once

// Dependencies
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include "IImageFrame.h"

namespace ncs {
/**
 * @brief Alias for the system clock time point used to timestamp image
 * captures.
 */
using CaptureTimestamp = std::chrono::system_clock::time_point;

/**
 * @struct SingleCapture
 * @brief Represents a single container for a monocular image frame and its
 * metadata.
 * @note This object is move-only due to the unique_ptr membership. Since the
 * image can be large, the enforcement of move-semantics by the compiler will be
 * beneficial.
 */
struct SingleCapture {
   public:
    /**
     * @brief Smart pointer to the captured image frame data.
     */
    std::unique_ptr<IImageFrame> pImage;

    /**
     * @brief The exact system timestamp when the capture was triggered.
     */
    CaptureTimestamp triggeredTimestamp;
};

/**
 * @struct SynchronizedCapture
 * @brief Represents a synchronized pair of image frames, typically used for
 * stereo calibration.
 * @note This object is move-only due to the unique_ptr membership. Since the
 * image can be large, the enforcement of move-semantics by the compiler will be
 * beneficial.
 */
struct SynchronizedCapture {
   public:
    /**
     * @brief An owned pointer to the left camera's image frame data.
     */
    std::unique_ptr<IImageFrame> left;

    /**
     * @brief An owned pointer to the right camera's image frame data.
     * */
    std::unique_ptr<IImageFrame> right;

    /**
     * @brief The shared system timestamp when the synchronized stereo capture
     * was triggered.
     */
    CaptureTimestamp triggeredTimestamp;
};
}  // namespace ncs