// Under Test
#include "CameraRoute.h"

// Dependencies
#include <camera/CameraService.h>
#include <camera/ICamera.h>
#include <camera/ICameraArray.h>
#include <camera/IRegionOfInterest.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <neo-calibration-server-api/calibration_server_api.pb.h>

#include <memory>
#include <vector>

#include "../IStreamContext.h"

using namespace ncs;

// ===========================================================================================
// Mocks
// ===========================================================================================

class MockRegionOfInterest : public IRegionOfInterest {
   public:
    MOCK_METHOD(void, setHeight, (std::int64_t height), (override));
    MOCK_METHOD(void, setWidth, (std::int64_t width), (override));
    MOCK_METHOD(void, setXOffset, (std::int64_t xOffset), (override));
    MOCK_METHOD(void, setYOffset, (std::int64_t yOffset), (override));
    MOCK_METHOD(void, setReverseX, (bool reverseX), (override));
    MOCK_METHOD(void, setReverseY, (bool reverseY), (override));

    MOCK_METHOD(std::int64_t, getHeight, (), (override));
    MOCK_METHOD(std::int64_t, getWidth, (), (override));
    MOCK_METHOD(std::int64_t, getXOffset, (), (override));
    MOCK_METHOD(std::int64_t, getYOffset, (), (override));
    MOCK_METHOD(bool, isReverseX, (), (override));
    MOCK_METHOD(bool, isReverseY, (), (override));
};

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

// ===========================================================================================
// Fixture
// ===========================================================================================

class CameraRouteRegionOfInterestTest : public ::testing::Test {
   protected:
    void SetUp() override {
        auto mockArray = std::make_unique<MockCameraArray>();
        mpMockArrayRaw = mockArray.get();

        // Instantiate mocks as raw/unique pointers managed cleanly within the
        // fixture
        mpMockCamera = std::make_unique<MockCamera>();
        mpMockRoi = std::make_unique<MockRegionOfInterest>();

        // Default stubbing: Array has the camera and returns our mock camera
        // reference
        ON_CALL(*mpMockArrayRaw, hasCamera(testing::_))
            .WillByDefault(testing::Return(true));
        ON_CALL(*mpMockArrayRaw, getCamera(testing::_))
            .WillByDefault(testing::ReturnRef(*mpMockCamera));

        // Stub connection status and device role
        ON_CALL(*mpMockCamera, isConnected())
            .WillByDefault(testing::Return(true));
        ON_CALL(*mpMockCamera, deviceRole())
            .WillByDefault(testing::Return(CameraRole::Left));

        // Stub region of interest reference
        ON_CALL(*mpMockCamera, getRegionOfInterest())
            .WillByDefault(testing::ReturnRef(*mpMockRoi));

        // Suppress uninteresting call warnings for setters by default
        ON_CALL(*mpMockRoi, setWidth(testing::_))
            .WillByDefault(testing::Return());
        ON_CALL(*mpMockRoi, setHeight(testing::_))
            .WillByDefault(testing::Return());
        ON_CALL(*mpMockRoi, setXOffset(testing::_))
            .WillByDefault(testing::Return());
        ON_CALL(*mpMockRoi, setYOffset(testing::_))
            .WillByDefault(testing::Return());
        ON_CALL(*mpMockRoi, setReverseX(testing::_))
            .WillByDefault(testing::Return());
        ON_CALL(*mpMockRoi, setReverseY(testing::_))
            .WillByDefault(testing::Return());

        std::vector<OwnedCameraArray> arrays;
        arrays.push_back(std::move(mockArray));

        mCameraService = std::make_unique<CameraService>(std::move(arrays));
        mCameraService->startup();

        mCameraRoute = std::make_unique<CameraRoute>(*mCameraService);
    }

    void TearDown() override {
        mCameraRoute.reset();
        mCameraService.reset();
        mpMockRoi.reset();
        mpMockCamera.reset();
    }

    MockCameraArray* mpMockArrayRaw;
    std::unique_ptr<MockCamera> mpMockCamera;
    std::unique_ptr<MockRegionOfInterest> mpMockRoi;
    std::unique_ptr<CameraService> mCameraService;
    std::unique_ptr<CameraRoute> mCameraRoute;
};

// ===========================================================================================
// Tests
// ===========================================================================================

TEST_F(CameraRouteRegionOfInterestTest, SetsDimensionsOnCameraRoi) {
    EXPECT_CALL(*mpMockRoi, setWidth(1920)).Times(1);
    EXPECT_CALL(*mpMockRoi, setHeight(1080)).Times(1);

    CalibrationApi::SetRegionOfInterest request;
    auto* regionSetting = request.add_regions();
    regionSetting->set_camera(CalibrationApi::LEFT);
    regionSetting->set_width(1920);
    regionSetting->set_height(1080);

    ncs::ByteString serializedResponse =
        mCameraRoute->setRegionOfInterest(request);

    CalibrationApi::FromServer response;
    ASSERT_TRUE(response.ParseFromString(serializedResponse));
    EXPECT_FALSE(response.region_of_interest().failures());
}

TEST_F(CameraRouteRegionOfInterestTest, SetsOffsetsOnCameraRoi) {
    EXPECT_CALL(*mpMockRoi, setXOffset(50)).Times(1);
    EXPECT_CALL(*mpMockRoi, setYOffset(75)).Times(1);

    CalibrationApi::SetRegionOfInterest request;
    auto* regionSetting = request.add_regions();
    regionSetting->set_camera(CalibrationApi::LEFT);
    regionSetting->set_xoffset(50);
    regionSetting->set_yoffset(75);

    ncs::ByteString serializedResponse =
        mCameraRoute->setRegionOfInterest(request);

    CalibrationApi::FromServer response;
    ASSERT_TRUE(response.ParseFromString(serializedResponse));
    EXPECT_FALSE(response.region_of_interest().failures());
}

TEST_F(CameraRouteRegionOfInterestTest, SetsReversalsOnCameraRoi) {
    EXPECT_CALL(*mpMockRoi, setReverseX(true)).Times(1);
    EXPECT_CALL(*mpMockRoi, setReverseY(false)).Times(1);

    CalibrationApi::SetRegionOfInterest request;
    auto* regionSetting = request.add_regions();
    regionSetting->set_camera(CalibrationApi::LEFT);
    regionSetting->set_reversex(true);
    regionSetting->set_reversey(false);

    ncs::ByteString serializedResponse =
        mCameraRoute->setRegionOfInterest(request);

    CalibrationApi::FromServer response;
    ASSERT_TRUE(response.ParseFromString(serializedResponse));
    EXPECT_FALSE(response.region_of_interest().failures());
}

TEST_F(CameraRouteRegionOfInterestTest, PopulatesResponseValuesFromCameraRoi) {
    EXPECT_CALL(*mpMockRoi, getWidth()).WillOnce(testing::Return(1280));
    EXPECT_CALL(*mpMockRoi, getHeight()).WillOnce(testing::Return(720));
    EXPECT_CALL(*mpMockRoi, getXOffset()).WillOnce(testing::Return(5));
    EXPECT_CALL(*mpMockRoi, getYOffset()).WillOnce(testing::Return(10));

    CalibrationApi::SetRegionOfInterest request;
    auto* regionSetting = request.add_regions();
    regionSetting->set_camera(CalibrationApi::LEFT);

    ncs::ByteString serializedResponse =
        mCameraRoute->setRegionOfInterest(request);

    CalibrationApi::FromServer response;
    ASSERT_TRUE(response.ParseFromString(serializedResponse));

    ASSERT_EQ(response.region_of_interest().regions_size(), 1);
    const auto& resRegion = response.region_of_interest().regions(0);
    EXPECT_EQ(resRegion.width(), 1280);
    EXPECT_EQ(resRegion.height(), 720);
    EXPECT_EQ(resRegion.xoffset(), 5);
    EXPECT_EQ(resRegion.yoffset(), 10);
}

TEST_F(CameraRouteRegionOfInterestTest, ReportsFailureWhenCameraIsNull) {
    CalibrationApi::SetRegionOfInterest request;
    auto* regionSetting = request.add_regions();
    regionSetting->set_camera(CalibrationApi::UKNOWN_CAMERA);

    ncs::ByteString serializedResponse =
        mCameraRoute->setRegionOfInterest(request);

    CalibrationApi::FromServer response;
    ASSERT_TRUE(response.ParseFromString(serializedResponse));
}