// Implements...
#include "MockCameraArray.h"

using namespace ncs;

MockCameraArray::MockCameraArray(const std::string& mockDataFolder)
    : mAssembled(false), mMockDataFolder(mockDataFolder) {}

void MockCameraArray::assemble() {
    mCameras.reserve(2);

    mCameras.try_emplace(CameraRole::Left,
                         std::make_unique<MockCamera>(
                             "Mock Left", CameraRole::Left, mMockDataFolder));
    mCameras[CameraRole::Left]->connect();

    mCameras.try_emplace(CameraRole::Right,
                         std::make_unique<MockCamera>(
                             "Mock Right", CameraRole::Right, mMockDataFolder));
    mCameras[CameraRole::Right]->connect();

    mAssembled = true;
}

void MockCameraArray::disband() {
    mCameras.clear();
    mAssembled = false;
}

bool MockCameraArray::hasCamera(CameraRole role) {
    return mCameras.find(role) != mCameras.end();
}

ICamera& MockCameraArray::getCamera(CameraRole role) {
    return *mCameras.at(role);
}
