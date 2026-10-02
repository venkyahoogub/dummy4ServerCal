// DistanceSensorRoute.cpp
#include "DistanceSensorRoute.h"

#include <iostream>
#include <chrono>
#include <thread>

using namespace ncs;
using namespace CalibrationApi;

DistanceSensorRoute::DistanceSensorRoute(IHbcService& hbcService)
    : mHbcService(hbcService) {}

ByteString DistanceSensorRoute::startRanging(
    const CalibrationApi::DistanceSensorStartRanging& request) {
    
    FromServer response;
    auto* distanceSensorResponse = response.mutable_distance_sensor_ranging_result();

    try {
        // Call IHbcService to start distance sensor ranging
        mHbcService.distanceSensorStartRanging();

        // Give the sensor time to collect distance data
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        // Get status which contains the current distance reading
        Hbc::Status status = mHbcService.getStatus();
        float distance = status.distance_to_target_mm();
        
        std::cout << "Distance from HBC status: " << distance << " mm\n";
        distanceSensorResponse->set_distance_to_target_mm(distance);

    } catch (const std::exception& e) {
        std::cerr << "Exception in startRanging: " << e.what() << "\n";
    }

    return response.SerializeAsString();
}

ByteString DistanceSensorRoute::stopRanging(
    const CalibrationApi::DistanceSensorStopRanging& request) {
    
    FromServer response;
    auto* distanceSensorResponse = response.mutable_distance_sensor_ranging_result();

    try {
        // Call IHbcService to stop distance sensor ranging
        mHbcService.distanceSensorStopRanging();

        // Give sensor time to finalize
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Get final distance value
        Hbc::Status status = mHbcService.getStatus();
        float distance = status.distance_to_target_mm();
        
        std::cout << "Final distance from HBC status: " << distance << " mm\n";
        distanceSensorResponse->set_distance_to_target_mm(distance);

    } catch (const std::exception& e) {
        std::cerr << "Exception in stopRanging: " << e.what() << "\n";
    }

    return response.SerializeAsString();
}

ByteString DistanceSensorRoute::setDeviceSettings(
    const CalibrationApi::SetDistanceSensorDeviceSettings& request) {
    
    FromServer response;
    auto* distanceSensorResponse = response.mutable_distance_sensor_ranging_result();

    try {
        const float gain = request.gain();
        const float offset = request.offset();

        if (gain < 0.0f || offset < 0.0f) {
            std::cerr << "Invalid gain or offset values\n";
            distanceSensorResponse->set_distance_to_target_mm(0.0f);
            return response.SerializeAsString();
        }

        mHbcService.setDistanceSensorDeviceSettings(gain, offset);
        std::cout << "Distance sensor settings applied: gain=" << gain << ", offset=" << offset << "\n";
        
        // Return placeholder distance (settings don't require a distance reading)
        distanceSensorResponse->set_distance_to_target_mm(0.0f);

    } catch (const std::exception& e) {
        std::cerr << "Exception in setDeviceSettings: " << e.what() << "\n";
    }

    return response.SerializeAsString();
}