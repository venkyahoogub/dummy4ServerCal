#pragma once

// Dependencies
#include <cstdint>

namespace ncs {

/**
 * @interface IRegionOfInterest
 * @brief Abstract interface defining the contract for managing a camera's
 * region of interest (RoI) windowing and axis configuration.
 *
 * This interface abstracts hardware-specific parameter mapping, allowing client
 * code and services to configure dimensions, offsets, and image mirroring
 * uniformly across different camera implementations or mocks.
 */
class IRegionOfInterest {
   public:
    /**
     * @brief Virtual destructor to ensure proper cleanup of derived classes.
     */
    virtual ~IRegionOfInterest() = default;

    /**
     * @brief Gets the current image height.
     * @return Height in pixels.
     */
    virtual std::int64_t getHeight() = 0;

    /**
     * @brief Sets the image height.
     * @param value Desired height in pixels.
     */
    virtual void setHeight(std::int64_t value) = 0;

    /**
     * @brief Gets the current image width.
     * @return Width in pixels.
     */
    virtual std::int64_t getWidth() = 0;

    /**
     * @brief Sets the image width.
     * @param value Desired width in pixels.
     */
    virtual void setWidth(std::int64_t value) = 0;

    /**
     * @brief Gets the horizontal offset (X-offset) from the coordinate origin.
     * @return Offset value in pixels.
     */
    virtual std::int64_t getXOffset() = 0;

    /**
     * @brief Sets the horizontal offset (X-offset).
     * @param value Desired horizontal offset in pixels.
     */
    virtual void setXOffset(std::int64_t value) = 0;

    /**
     * @brief Gets the vertical offset (Y-offset) from the coordinate origin.
     * @return Offset value in pixels.
     */
    virtual std::int64_t getYOffset() = 0;

    /**
     * @brief Sets the vertical offset (Y-offset).
     * @param value Desired vertical offset in pixels.
     */
    virtual void setYOffset(std::int64_t value) = 0;

    /**
     * @brief Queries whether image mirroring along the X-axis is enabled.
     * @return true if reverse X is active, false otherwise.
     */
    virtual bool isReverseX() = 0;

    /**
     * @brief Enables or disables image mirroring along the X-axis.
     * @param value True to enable horizontal mirroring, false to disable.
     */
    virtual void setReverseX(bool value) = 0;

    /**
     * @brief Queries whether image mirroring along the Y-axis is enabled.
     * @return true if reverse Y is active, false otherwise.
     */
    virtual bool isReverseY() = 0;

    /**
     * @brief Enables or disables image mirroring along the Y-axis.
     * @param value True to enable vertical mirroring, false to disable.
     */
    virtual void setReverseY(bool value) = 0;
};

}  // namespace ncs