// Under test
#include "CameraService.h"

// Dependencies
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <vector>

#include "ICamera.h"
#include "ICameraArray.h"

using namespace ncs;

// =======================================================================================
// Mocks
// =======================================================================================

class MockCamera : public ICamera {
   public:
    MOCK_METHOD(const std::string&, deviceName, (), (const, override));
    MOCK_METHOD(bool, isConnected, (), (const, override));
    MOCK_METHOD(CameraRole, deviceRole, (), (const, override));
    MOCK_METHOD(bool, connect, (), (override));
    MOCK_METHOD(bool, disconnect, (), (override));
    MOCK_METHOD(void, startStreaming, (), (override));
    MOCK_METHOD(void, stopStreaming, (), (override));
    MOCK_METHOD(SingleCapture, waitForNextFrame,
                (std::chrono::milliseconds timeout), (override));
    MOCK_METHOD(double, getGain, (), (override));
    MOCK_METHOD(double, setGain, (double gain), (override));
    MOCK_METHOD(double, getExposure, (), (override));
    MOCK_METHOD(double, setExposure, (double exposureMicroSeconds), (override));
    MOCK_METHOD(IRegionOfInterest&, getRegionOfInterest, (), (override));
    MOCK_METHOD(void, flushBuffer, (int), (override));
};

class MockCameraArray : public ICameraArray {
   public:
    MOCK_METHOD(bool, isAssembled, (), (const, override));
    MOCK_METHOD(void, assemble, (), (override));
    MOCK_METHOD(void, disband, (), (override));
    MOCK_METHOD(bool, hasCamera, (CameraRole role), (override));
    MOCK_METHOD(ICamera&, getCamera, (CameraRole role), (override));
};

// =======================================================================================
// Fixture
// =======================================================================================

class CameraServiceTest : public ::testing::Test {
   protected:
    // cppcheck-suppress unusedFunction
    void SetUp() override {
        auto mockArray = std::make_unique<MockCameraArray>();
        mpMockArrayRaw = mockArray.get();
        testing::Mock::AllowLeak(mpMockArrayRaw);

        // Provide a default mock camera instance
        mpMockCamera = std::make_unique<MockCamera>();
        testing::Mock::AllowLeak(mpMockCamera.get());

        // Default behavior for mock camera calls to prevent segfaults during
        // streaming
        ON_CALL(*mpMockCamera, startStreaming())
            .WillByDefault(testing::Return());
        ON_CALL(*mpMockCamera, stopStreaming())
            .WillByDefault(testing::Return());
        ON_CALL(*mpMockCamera, isConnected())
            .WillByDefault(testing::Return(true));
        ON_CALL(*mpMockCamera, deviceRole())
            .WillByDefault(testing::Return(ncs::CameraRole::Left));

        // Return the mock camera by reference when requested from the array
        ON_CALL(*mpMockArrayRaw, getCamera(testing::_))
            .WillByDefault(testing::ReturnRef(*mpMockCamera));

        // Default stubbing for array queries
        ON_CALL(*mpMockArrayRaw, isAssembled())
            .WillByDefault(testing::Return(true));
        ON_CALL(*mpMockArrayRaw, hasCamera(testing::_))
            .WillByDefault(testing::Return(true));

        std::vector<ncs::OwnedCameraArray> arrays;
        arrays.push_back(std::move(mockArray));

        mService = std::make_unique<ncs::CameraService>(std::move(arrays));
    }

    // cppcheck-suppress unusedFunction
    void TearDown() override { mService.reset(); }

    MockCameraArray* mpMockArrayRaw;
    std::unique_ptr<MockCamera> mpMockCamera;
    std::unique_ptr<ncs::CameraService> mService;
};

// =======================================================================================
// Test Cases
// =======================================================================================

TEST_F(CameraServiceTest, InitialStateIsNotReady) {
    EXPECT_FALSE(mService->isReady());
    EXPECT_EQ(mService->leftCamera(), nullptr);
    EXPECT_EQ(mService->rightCamera(), nullptr);
    EXPECT_EQ(mService->wideCamera(), nullptr);
}

TEST_F(CameraServiceTest, StartupAndShutdownLifecycle) {
    EXPECT_CALL(*mpMockArrayRaw, assemble()).Times(1);
    mService->startup();

    EXPECT_CALL(*mpMockArrayRaw, disband()).Times(1);
    mService->shutdown();
}
TEST_F(CameraServiceTest, StreamControlDelegation) {
    mService->startup();

    EXPECT_NO_THROW(mService->openStream());
    EXPECT_NO_THROW(mService->closeStream());

    EXPECT_CALL(*mpMockArrayRaw, disband()).Times(1);
    mService->shutdown();
}

TEST_F(CameraServiceTest, WideStreamControlDelegation) {
    mService->startup();
    EXPECT_NO_THROW(mService->openWideStream());
    EXPECT_NO_THROW(mService->closeWideStream());
}

TEST_F(CameraServiceTest, SetExposureDetailsHandlesMissingCameraGracefully) {
    bool result =
        mService->setExposureDetails(ncs::CameraRole::Left, 1.0, 50.0);
    EXPECT_FALSE(result);
}