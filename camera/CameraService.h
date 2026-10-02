#pragma once

// Dependencies
#include "Capture.h"
#include "ICameraArray.h"

namespace ncs {
/**
 * @brief Unique ownership alias for a single managed camera array.
 */
using OwnedCameraArray = std::unique_ptr<ICameraArray>;

/**
 * @class CameraService
 * @brief The core orchestration service managing the calibration camera
 * hardware system.
 * @note Release mode, this will have a single array. In debug mode, this will
 * have two arrays. This structure also allows for future extensions and cameras
 * also coming from other vendors.
 *
 * This service owns the collection of camera arrays, drives their lifecycle
 * initialization sequences, and exposes direct access to the active primary
 * stereo pair (Left/Right) along with facilities to grab synchronized frames.
 */
class CameraService {
   public:
    /**
     * @brief Constructs the service by taking exclusive ownership of a
     * collection of camera arrays.
     *
     * @param cameraArrays A vector of unique pointers to camera arrays.
     * @note Because OwnedCameraArray contains unique_ptr elements, this vector
     * cannot be copied and must be passed into the constructor using std::move.
     */
    explicit CameraService(std::vector<OwnedCameraArray> cameraArrays);

    /**
     * @brief Power-on routine that initializes underlying resources and
     * establishes connections.
     *
     * This method shifts the service state and changes the value returned by
     * isReady() upon successful completion.
     */
    void startup();

    /**
     * @brief Gracefully winds down active camera streams and safely severs
     * hardware links.
     */
    void shutdown();

    /**
     * @brief Queries whether the service is fully initialized, connected, and
     * ready to capture data.
     * @return true if all the cameras that are necessary have been found and
     * connected to successfully.
     */
    bool isReady() const { return mReady; }

    /**
     * @brief Provides non-owning access to the designated left eye camera
     * instance.
     * @return A raw pointer to the left ICamera interface, or nullptr if no
     * device is mapped.
     */
    ICamera* leftCamera() const { return mpLeftCamera; }

    /**
     * @brief Provides non-owning access to the designated right eye camera
     * instance.
     * @return A raw pointer to the right ICamera interface, or nullptr if no
     * device is mapped.
     */
    ICamera* rightCamera() const { return mpRightCamera; }

    /**
     * @brief Gets the current camera role.
     * @return A non-owning pointer to the camera. May be null.
     */
    ICamera* getCamera(CameraRole camera);

    /**
     * @brief Tells all active cameras to start listening for images and filling
     * their internal buffers for data collection.
     */
    void openStream();

    /**
     * @brief Tells all active cameras to stop listening for new images.
     */
    void closeStream();

    /**
     * @brief Triggers and gathers a perfectly synchronized stereo capture from
     * the primary camera pair.
     * @return A SynchronizedCapture instance holding ownership of both
     * resulting left and right image frames.
     * @note This function leverages move semantics to return the frame
     * containers efficiently without deep copies.
     */
    SynchronizedCapture synchronizedCapture();

    /**
     * @brief Finds the specified camera and adjusts the gain and exposure.
     * @returns Will return true if the camera is found and the settings are
     * applied. otherwise false.
     */
    bool setExposureDetails(CameraRole camera, double gain, double exposure);

    /**
     * @brief Provides non-owning access to the wide field-of-view camera instance.
     * @return A raw pointer to the wide ICamera interface, or nullptr if no device is mapped.
     */
    ICamera* wideCamera() const { return mpWideCamera; }

    /**
     * @brief Captures a single frame from the wide camera using software trigger.
     * @return A SingleCapture instance holding ownership of the image frame.
     */
    SingleCapture wideCapture();

    /**
     * @brief Starts continuous video streaming from the wide camera.
     */
    void openWideStream();

    /**
     * @brief Stops video streaming from the wide camera.
     */
    void closeWideStream();

     /**
     * @brief Captures a frame from the wide camera during continuous streaming.
     * Assumes the stream is already open and configured.
     * @return A SingleCapture instance holding ownership of the image frame.
     */
    SingleCapture wideStreamCapture();

    private:
    bool mReady;
    bool mKeepStreamsOpen;
    bool mWideStreamOpen;  // ADD THIS LINE
    ICamera* mpLeftCamera;
    ICamera* mpRightCamera;
    std::vector<OwnedCameraArray> mCameraArrays;
    ICamera* mpWideCamera;

};
}  // namespace ncs