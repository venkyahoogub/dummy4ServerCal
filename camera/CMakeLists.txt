# ========================================================================================
# Libraries
# ========================================================================================

find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)
find_package(OpenCV REQUIRED)
find_package(Pylon REQUIRED)

add_library(neo_calibration_camera STATIC 
    CameraService.cpp
    basler/BaslerCamera.cpp
    basler/BaslerCameraArray.cpp
    basler/BaslerImageFrame.cpp
    basler/BaslerRegionOfInterest.cpp
    mock/MockCamera.cpp
    mock/MockCameraArray.cpp
    mock/PngImageFrame.cpp
)

target_link_directories(neo_calibration_camera PUBLIC /opt/pylon/lib)
target_link_libraries(neo_calibration_camera        
    PRIVATE    
        fmt::fmt
        spdlog::spdlog
        pylon::pylon
        pylonbase
        pylonutility    
        ${OpenCV_LIBS}        
        neo_calibration_common
)

target_include_directories(neo_calibration_camera
    PUBLIC 
        "${CMAKE_CURRENT_SOURCE_DIR}/.."
        /opt/pylon/include # <-- Tied explicitly to target include pipeline        
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
    target_link_libraries(neo_calibration_camera PRIVATE stdc++fs)
endif()

# ========================================================================================
# Tests
# ========================================================================================
find_package(GTest REQUIRED)
find_package(nlohmann_json REQUIRED) # <-- Add this line
include(GoogleTest)

add_executable(neo_calibration_camera_tests
    CameraServiceTests.cpp
)

target_compile_features(neo_calibration_camera_tests PRIVATE cxx_std_17)
target_compile_options(neo_calibration_camera_tests PRIVATE --coverage -O0)
target_link_options(neo_calibration_camera_tests PRIVATE --coverage)

set_target_properties(neo_calibration_camera_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests/unittests"
)

target_link_libraries(neo_calibration_camera_tests
    PRIVATE
        neo_calibration_camera       
        nlohmann_json::nlohmann_json    
        neo_calibration_common        
        GTest::gtest_main
        GTest::gmock            
)

gtest_discover_tests(neo_calibration_camera_tests
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/tests/unittests"
)