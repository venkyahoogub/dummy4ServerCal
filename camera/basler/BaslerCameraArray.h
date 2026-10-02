#pragma once

// Dependencies
#include <pylon/PylonIncludes.h>
#include <pylon/usb/BaslerUsbInstantCamera.h>

#include <memory>
#include <unordered_map>

#include "../ICamera.h"
#include "../ICameraArray.h"
#include "BaslerRegionOfInterest.h"

namespace ncs {
/**
 * @brief Defines the camera array which the Pylon libraries couple
 * strongly with their concept of a camera.
 */
class BaslerCameraArray : public ICameraArray {
   public:
    /**
     * @brief Represents a Basler camera.
     * @note Since Pylon couples the camera and camera array pretty
     * strongly, I nested the classes to match that.
     */
    class Camera : public ICamera {
       public:
        /**
         * @brief Built from discovered a discovered device after
         * initializing the array.
         *
         * @param pylonDevice The discovered device.
         */
        explicit Camera(Pylon::CInstantCamera& pylonDevice);

        /**
         * @brief Connects to the camera.
         */
        bool connect() override;

        /**
         * @brief Politely disconnects from the camera.
         */
        bool disconnect() override;

        /**
         * @brief Tells all active cameras to start listening for new images.
         */
        void startStreaming() override;

        /**
         * @brief Tells all active cameras to stop listening for new images.
         */
        void stopStreaming() override;

        /**
         * @brief Starts listening and waits for a frame to show up within the
         * wait time.
         *
         * @param maxWait How long to wait before giving up.
         * @note Expected to work with a separately fired hardware trigger.
         */
        SingleCapture waitForNextFrame(
            std::chrono::milliseconds maxWait) override;

        /**
         * @brief Gets the role of the camera determined based off the device
         * name.
         */
        inline CameraRole deviceRole() const override { return mDeviceRole; }

        /**
         * @brief Gets the device name that the camera provided.
         */
        inline const std::string& deviceName() const override {
            return mDeviceName;
        }

        /**
         * @brief Returns true if actively connected to the camera; otherwise
         * false.
         */
        inline bool isConnected() const override { return mIsConnected; }

        /**
         * @brief Gets the current gain setting used by the camera.
         */
        double getGain() override;

        /**
         * @brief Sets the current gain being used by the camera.
         * @returns The gain accepted as it could be clamped to fit within a
         * range.
         */
        double setGain(double gain) override;

        /**
         * @brief Gets the current exposure setting used by the camera.
         */
        double getExposure() override;

        /**
         * @brief Sets the current exposure being used by the camera.
         * @returns The exposure accepted as it could be clamped to fit within a
         * range.
         */
        double setExposure(double exposureMicroSeconds) override;

        /**
         * @brief Gets the current region of interest values and allows
         * mutation of the values.
         * @returns The current region of interest values.
         */
        inline IRegionOfInterest& getRegionOfInterest() override {
            return mRoI;
        }

        /**
         * @brief Executes a software trigger for cameras configured with
         * software trigger mode.
         * @note Only use this for cameras with software trigger configuration.
         */
        void executeSoftwareTrigger() override;

        /**
         * @brief Waits for the camera to be ready to accept a software trigger.
         * @param maxWait Maximum time to wait.
         * @return true if camera is ready, false on timeout.
         */
        bool waitForTriggerReady(std::chrono::milliseconds maxWait) override;

        /**
         * @brief Flushes stale images.
         */
        void flushBuffer(int timeout = 0) override;

       private:
        Pylon::CInstantCamera& mPylonDevice;
        std::string mDeviceName;
        CameraRole mDeviceRole;
        bool mIsConnected;
        BaslerRegionOfInterest mRoI;
    };

   public:
    BaslerCameraArray();

    /**
     * @brief Returns true if the cameras have been discovered.
     */
    inline bool isAssembled() const override { return mAssembled; }

    /**
     * @brief Assembles the array by discovering what cameras are available
     * over the network.
     */
    void assemble() override;

    /**
     * @brief Cleanly disassembles the array by first disconnecting from
     * all of the cameras.
     */
    void disband() override;

    /**
     * @brief Determines if the array has a camera that represents the requested
     * role.
     *
     * @param role The role to check if the array has.
     * @note If not all of the cameras are available, debug builds should
     * fallback to something else.
     */
    bool hasCamera(CameraRole role) override;

    /**
     * @brief Gets the camera of the specific role.
     * @param role The role to fetch.
     * @note May throw an exception if the array doesn't have the requested
     * camera role.
     */
    ICamera& getCamera(CameraRole role) override;

   private:
    using OwnedCamera = std::unique_ptr<Camera>;

    std::unordered_map<CameraRole, OwnedCamera> mCameras;
    Pylon::CInstantCameraArray mDeviceArray;
    bool mAssembled;  // Cameras initialized flag
};
}  // namespace ncs