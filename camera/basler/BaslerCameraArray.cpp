// Implements...
#include "BaslerCameraArray.h"

// Dependencies
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>
#include <stdexcept>

#include "HardwareTriggerConfiguration.h"
#include "SoftwareTriggerConfiguration.h"

using namespace ncs;

BaslerCameraArray::BaslerCameraArray()
    : mCameras(), mDeviceArray(), mAssembled(false) {}

void BaslerCameraArray::assemble() {
    if (mAssembled) {
        return;
    }

    Pylon::PylonInitialize();

    auto& factory = Pylon::CTlFactory::GetInstance();
    Pylon::DeviceInfoList_t devices;

    int count = factory.EnumerateDevices(devices);
    mDeviceArray.Initialize(count);

    for (auto i = 0; i < count; i += 1) {
        auto device = factory.CreateDevice(devices[i]);
        mDeviceArray[i].Attach(device);
        mDeviceArray[i].SetCameraContext(i);
        
        mDeviceArray[i].RegisterConfiguration(
            new HardwareTriggerConfiguration,
            Pylon::RegistrationMode_ReplaceAll, Pylon::Cleanup_Delete);
        auto camera = std::make_unique<BaslerCameraArray::Camera>(mDeviceArray[i]);
        
        if (camera->connect()) {
            // Applying appropriate trigger configuration based on role of camera.
            if (camera->deviceRole() == CameraRole::Wide) {
                std::cout << "Configuring Wide camera for software trigger...\n";
                SoftwareTriggerConfiguration::ApplyConfiguration(
                    mDeviceArray[i].GetNodeMap());
            } else {
                std::cout << "Configuring Left/Right camera for hardware trigger...\n";
                HardwareTriggerConfiguration::ApplyConfiguration(
                    mDeviceArray[i].GetNodeMap());
            }
            mCameras.try_emplace(camera->deviceRole(), std::move(camera));
        }
    }

    mAssembled = true;
}

void BaslerCameraArray::disband() {
    if (!mAssembled) return;

    for (auto& kvp : mCameras) {
        auto& camera = kvp.second;
        camera->disconnect();
    }

    Pylon::PylonTerminate();
    mCameras.clear();
    mAssembled = false;
}

bool BaslerCameraArray::hasCamera(CameraRole role) {
    return mCameras.find(role) != mCameras.end();
}

ICamera& BaslerCameraArray::getCamera(CameraRole role) {
    return *mCameras.at(role);
}
