#pragma once

// Dependencies
#include <memory>
#include <vector>

#include "ICamera.h"

namespace ncs {
using OwnedCamera = std::unique_ptr<ICamera>;

/**
 * @brief Represents an array of cameras managed by the same infrastructure.
 * @note This design mirrors the pylon design.
 */
class ICameraArray {
   public:
    virtual ~ICameraArray() = default;

    /**
     * @brief Determines if the camera array has been assembled and is active.
     */
    virtual bool isAssembled() const = 0;

    /**
     * @brief Identifies, locates and connects to the array of cameras that are
     * managed by the same infrastructure.
     */
    virtual void assemble() = 0;

    /**
     * @brief Gracefully shuts down the camera array.
     */
    virtual void disband() = 0;

    /**
     * @brief Checks if the array has the camera of that role. Highly impacted
     * by if the array can identify a connected device that meets the role.
     */
    virtual bool hasCamera(CameraRole role) = 0;

    /**
     * @brief Gets the requested camera by its role.
     * @returns A non-owning reference for the camera role. Will throw if it
     * cannot find the role. Should couple this call with 'hasCamera'.
     */
    virtual ICamera& getCamera(CameraRole role) = 0;
};
}  // namespace ncs