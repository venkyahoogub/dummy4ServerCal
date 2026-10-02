#pragma once

#include "../IRegionOfInterest.h"

namespace ncs {

/**
 * @class MockRegionOfInterest
 * @brief Mock implementation of the IRegionOfInterest interface for unit
 * testing.
 *
 * This class simulates camera region of interest parameters in memory, enabling
 * fast and isolated unit testing of components dependent on windowing and
 * coordinate configurations without requiring physical camera hardware.
 */
class MockRegionOfInterest : public IRegionOfInterest {
   public:
    /**
     * @brief Gets the simulated image height.
     * @return Height in pixels.
     */
    inline std::int64_t getHeight() override { return mHeight; }

    /**
     * @brief Sets the simulated image height.
     * @param value Desired height in pixels.
     */
    inline void setHeight(std::int64_t value) override { mHeight = value; }

    /**
     * @brief Gets the simulated image width.
     * @return Width in pixels.
     */
    inline std::int64_t getWidth() override { return mWidth; }

    /**
     * @brief Sets the simulated image width.
     * @param value Desired width in pixels.
     */
    inline void setWidth(std::int64_t value) override { mWidth = value; }

    /**
     * @brief Gets the simulated horizontal offset (X-offset).
     * @return X-offset value in pixels.
     */
    inline std::int64_t getXOffset() override { return mXOffset; }

    /**
     * @brief Sets the simulated horizontal offset (X-offset).
     * @param value Desired horizontal offset in pixels.
     */
    inline void setXOffset(std::int64_t value) override { mXOffset = value; }

    /**
     * @brief Gets the simulated vertical offset (Y-offset).
     * @return Y-offset value in pixels.
     */
    inline std::int64_t getYOffset() override { return mYOffset; }

    /**
     * @brief Sets the simulated vertical offset (Y-offset).
     * @param value Desired vertical offset in pixels.
     */
    inline void setYOffset(std::int64_t value) override { mYOffset = value; }

    /**
     * @brief Queries whether simulated X-axis mirroring is active.
     * @return true if reverse X is enabled, false otherwise.
     */
    inline bool isReverseX() override { return mIsReverseX; }

    /**
     * @brief Enables or disables simulated X-axis mirroring.
     * @param value True to enable horizontal mirroring, false to disable.
     */
    inline void setReverseX(bool value) override { mIsReverseX = value; }

    /**
     * @brief Queries whether simulated Y-axis mirroring is active.
     * @return true if reverse Y is enabled, false otherwise.
     */
    inline bool isReverseY() override { return mIsReverseY; }

    /**
     * @brief Enables or disables simulated Y-axis mirroring.
     * @param value True to enable vertical mirroring, false to disable.
     */
    inline void setReverseY(bool value) override { mIsReverseY = value; }

   private:
    std::int64_t mHeight = 0;
    std::int64_t mWidth = 0;
    std::int64_t mXOffset = 0;
    std::int64_t mYOffset = 0;
    bool mIsReverseX = false;
    bool mIsReverseY = false;
};

}  // namespace ncs