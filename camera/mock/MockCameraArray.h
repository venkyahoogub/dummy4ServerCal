#pragma once

// Dependencies
#include <string>
#include <unordered_map>

#include "../ICamera.h"
#include "../ICameraArray.h"
#include "MockCamera.h"

namespace ncs {
/**
 * @brief A mocked camera array that provides a fallback in case the physical
 * cameras aren't there in debug builds.
 */
class MockCameraArray : public ICameraArray {
   public:
    /**
     * @brief Builds a mock array using data from the provided folder.
     * @param mockDataFolder Where to look for the mock images.
     */
    explicit MockCameraArray(const std::string& mockDataFolder);

    /**
     * @brief Determines if the camera array has been assembled and is active.
     */
    inline bool isAssembled() const override { return mAssembled; }

    /**
     * @brief Identifies, locates and connects to the mock array of cameras.
     */
    void assemble() override;

    /**
     * @brief For a mock array this just clears the list and forces assemble
     * to be called again. Simulating the teardown of a camera array.
     */
    void disband() override;

    /**
     * @brief Checks if the array has the camera of that role. Expectation from
     * this mock collection is that it has a backup for each required camera.
     */
    bool hasCamera(CameraRole role) override;

    /**
     * @brief Gets the requested camera by its role.
     */
    ICamera& getCamera(CameraRole role) override;

   private:
    using OwnedCamera = std::unique_ptr<MockCamera>;
    std::string mMockDataFolder;
    std::unordered_map<CameraRole, OwnedCamera> mCameras;
    bool mAssembled;
};
}  // namespace ncs