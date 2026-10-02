#pragma once

// Dependencies
#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>

#include "Capture.h"
#include "IRegionOfInterest.h"

namespace ncs {
/**
 * @enum CameraRole
 * @brief Assigns a functional designation to a camera within the calibration
 * environment.
 */
enum class CameraRole {
    Left,  /**< Left camera in a stereo-paired setup. */
    Right, /**< Right camera in a stereo-paired setup. */
    Wide,  /**< Wide view camera */
    Other  /**< Unassigned or custom secondary camera role. */
};

/**
 * @class ICamera
 * @brief Abstract interface representing a physical or virtual camera device.
 *
 * This interface handles the connection lifecycle, runtime status, image
 * acquisition and frame-grabbing capabilities of individual capture units in
 * the system.
 */
class ICamera {
   public:
    virtual ~ICamera() = default;

    /**
     * @brief Gets the human-readable identifier or hardware path of the device.
     * @return A reference to a string representing the camera's assigned name,
     * serial, or URI.
     */
    virtual const std::string& deviceName() const = 0;

    /**
     * @brief Checks whether the physical link to the camera hardware is
     * currently active.
     * @return true if the camera is connected and ready to stream, false
     * otherwise.
     */
    virtual bool isConnected() const = 0;

    /**
     * @brief Gets the assigned role of this specific camera instance.
     * @return The CameraRole enum indicating if this is a left, right, or wide
     * view camera.
     */
    virtual CameraRole deviceRole() const = 0;

    /**
     * @brief Initializes the hardware connection and primes the camera for
     * capturing frames.
     * @return true if the connection was established successfully, false if
     * initialization failed.
     */
    virtual bool connect() = 0;

    /**
     * @brief Safely tears down the hardware connection and terminates any
     * active data streams.
     * @return true if the disconnection sequence completed gracefully, false
     * otherwise.
     */
    virtual bool disconnect() = 0;

    /**
     * @brief Starts streaming and tells the cameras to start filling internal
     * buffers for image collection.
     */
    virtual void startStreaming() = 0;

    /**
     * @brief Stops streaming and shuts down any buffering that might be
     * happening.
     */
    virtual void stopStreaming() = 0;

    /**
     * @brief Blocks execution until a new image frame is acquired or a timeout
     * is reached.
     *
     * @param maxWait The maximum duration to block waiting for an incoming
     * frame.
     * @return A SingleCapture object containing the acquired image frame and
     * its metadata.
     * @note Because SingleCapture is a move-only type, this method leverages
     * move semantics to safely pass ownership of the image data up to the
     * caller without deep copying.
     */
    virtual SingleCapture waitForNextFrame(
        std::chrono::milliseconds maxWait) = 0;

    /**
     * @brief Gets the current gain setting used by the camera.
     */
    virtual double getGain() = 0;

    /**
     * @brief Sets the current gain being used by the camera.
     * @returns The gain accepted as it could be clamped to fit within a range.
     */
    virtual double setGain(double gain) = 0;

    /**
     * @brief Gets the current exposure setting used by the camera.
     */
    virtual double getExposure() = 0;

    /**
     * @brief Sets the current exposure being used by the camera.
     * @returns The exposure accepted as it could be clamped to fit within a
     * range.
     */
    virtual double setExposure(double exposureMicroSeconds) = 0;

    /**
     * @brief Gets the current region of interest values and allows
     * mutation of the values.
     * @returns The current region of interest values.
     */
    virtual IRegionOfInterest& getRegionOfInterest() = 0;

    /**
     * @brief Executes a software trigger for cameras configured with software
     * trigger mode.
     * @note Only use this for cameras with software trigger configuration.
     * Default implementation is a no-op for backward compatibility.
     */
    virtual void executeSoftwareTrigger() {}

    /**
     * @brief Waits for the camera to be ready to accept a software trigger.
     * @param maxWait Maximum time to wait.
     * @return true if camera is ready, false on timeout.
     * @note Default implementation returns true for backward compatibility.
     */
    virtual bool waitForTriggerReady(std::chrono::milliseconds maxWait) {
        return true;
    }

    /**
     * @brief Flushes stale images.
     */
    virtual void flushBuffer(int timeout = 0) = 0;
};
}  // namespace ncs
