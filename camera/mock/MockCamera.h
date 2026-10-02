#pragma once

// Dependencies
#include <filesystem>
#include <string>

#include "../ICamera.h"
#include "MockRegionOfInterest.h"
namespace ncs {
/**
 * @class MockCamera
 * @brief
 */
class MockCamera : public ICamera {
   public:
    MockCamera(const std::string& deviceName, CameraRole role,
               std::filesystem::path readFromPath);

    /**
     * @brief Creates the mock array simulating the 'connection' process.
     * @returns Always true as a mock array should reliably create its cameras.
     */
    bool connect() override;

    /**
     * @brief Emulates the disconnect process.
     * @returns Always true.
     */
    bool disconnect() override;

    /**
     * @brief Checks for the first file it can find in the provided folder.
     */
    SingleCapture waitForNextFrame(std::chrono::milliseconds maxWait) override;

    /**
     * @brief The role this camera is simulating.
     */
    inline CameraRole deviceRole() const override { return mDeviceRole; }

    /**
     * @brief The device name. Generally prefixed by 'Mock' to identify it as a
     * fake camera.
     */
    inline const std::string& deviceName() const override {
        return mDeviceName;
    }

    /**
     * @brief If connect() was called this should return true. If disconnect()
     * is called this should return false.
     */
    inline bool isConnected() const override { return mIsConnected; }

    /**
     * @brief Pretends to activate the camera buffering process.
     */
    void startStreaming() override {};

    /**
     * @brief Pretends to stop the camera buffering process.
     */
    void stopStreaming() override {};

    double getGain() override;
    double setGain(double gain) override;
    double getExposure() override;
    void flushBuffer(int timeout = 0) override;
    double setExposure(double exposureMicroSeconds) override;
    inline IRegionOfInterest& getRegionOfInterest() override { return mRoI; }

   private:
    std::filesystem::path mReadFromPath;
    std::string mDeviceName;
    CameraRole mDeviceRole;
    bool mIsConnected;
    double mGain;
    double mExposureMicroSeconds;
    MockRegionOfInterest mRoI;
};
}  // namespace ncs