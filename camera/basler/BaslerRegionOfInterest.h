#pragma once

// Dependencies
#include <pylon/PylonIncludes.h>
#include <pylon/usb/BaslerUsbInstantCamera.h>

#include "../IRegionOfInterest.h"

namespace ncs {

/**
 * @class BaslerRegionOfInterest
 * @brief Concrete implementation of the IRegionOfInterest interface for Basler
 * cameras.
 *
 * This class bridges the generic region of interest abstraction with the
 * underlying Basler Pylon SDK camera parameters via GenICam node maps. It
 * manages hardware-level windowing and coordinate transformations (height,
 * width, offsets, and axis mirroring).
 */
class BaslerRegionOfInterest : public IRegionOfInterest {
   public:
    /**
     * @brief Constructs a BaslerRegionOfInterest bound to a specific Pylon
     * camera instance.
     * @param pylonDevice Reference to the active Pylon instant camera device.
     */
    explicit BaslerRegionOfInterest(Pylon::CInstantCamera& pylonDevice);

    /**
     * @brief Gets the current height of the camera image region.
     * @return Height in pixels.
     */
    std::int64_t getHeight() override;

    /**
     * @brief Sets the height of the camera image region.
     * @param value Desired height in pixels.
     */
    void setHeight(std::int64_t value) override;

    /**
     * @brief Gets the current width of the camera image region.
     * @return Width in pixels.
     */
    std::int64_t getWidth() override;

    /**
     * @brief Sets the width of the camera image region.
     * @param value Desired width in pixels.
     */
    void setWidth(std::int64_t value) override;

    /**
     * @brief Gets the horizontal offset (X-offset) of the region.
     * @return Offset value in pixels from the origin.
     */
    std::int64_t getXOffset() override;

    /**
     * @brief Sets the horizontal offset (X-offset) of the region.
     * @param value Desired horizontal offset in pixels.
     */
    void setXOffset(std::int64_t value) override;

    /**
     * @brief Gets the vertical offset (Y-offset) of the region.
     * @return Offset value in pixels from the origin.
     */
    std::int64_t getYOffset() override;

    /**
     * @brief Sets the vertical offset (Y-offset) of the region.
     * @param value Desired vertical offset in pixels.
     */
    void setYOffset(std::int64_t value) override;

    /**
     * @brief Queries whether image mirroring along the X-axis is enabled.
     * @return true if reverse X is active, false otherwise.
     */
    bool isReverseX() override;

    /**
     * @brief Enables or disables image mirroring along the X-axis.
     * @param value True to enable horizontal mirroring, false to disable.
     */
    void setReverseX(bool value) override;

    /**
     * @brief Queries whether image mirroring along the Y-axis is enabled.
     * @return true if reverse Y is active, false otherwise.
     */
    bool isReverseY() override;

    /**
     * @brief Enables or disables image mirroring along the Y-axis.
     * @param value True to enable vertical mirroring, false to disable.
     */
    void setReverseY(bool value) override;

   private:
    Pylon::CInstantCamera& mPylonDevice;

    /**
     * @brief Retrieves the GenICam node map from the underlying camera device.
     * @return Pointer to the camera's GenApi node map, or nullptr if
     * unavailable.
     */
    GenApi::INodeMap* getNodeMap();
};

}  // namespace ncs