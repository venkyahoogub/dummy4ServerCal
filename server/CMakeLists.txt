# ========================================================================================
# Library
# ========================================================================================
find_package(asio REQUIRED)

add_library(neo_calibration_server STATIC 
    CalibrationServer.cpp
    ServerConnection.cpp
    VersionService.cpp
    protobuf_api/ProtobufRouter.cpp
    protobuf_api/AboutRoute.cpp
    protobuf_api/CameraRoute.cpp
    protobuf_api/NirLedRoute.cpp
    protobuf_api/DmdRoute.cpp
    protobuf_api/CameraTriggerRoute.cpp
    protobuf_api/CalibrationRoute.cpp
    protobuf_api/MotorRoute.cpp
    protobuf_api/UvDiagnosticRoute.cpp
    protobuf_api/CalibrationTableRoute.cpp
    protobuf_api/DistanceSensorRoute.cpp
    protobuf_api/UvDeviceSettingsRoute.cpp
    protobuf_api/GetDeviceSettingsRoute.cpp
    ../services/hbcService.cpp
    ../services/pbSocketService.cpp
)

target_link_libraries(neo_calibration_server
    PUBLIC
        asio::asio   
    PRIVATE
        neo_calibration_proto
        hbc_proto
        dmd_driver
        neo_calibration_common
)

target_include_directories(neo_calibration_server 
    PUBLIC  
        "${CMAKE_CURRENT_SOURCE_DIR}/.."
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(neo_calibration_server PRIVATE --coverage -O0)
    target_link_options(neo_calibration_server PRIVATE --coverage)
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
    target_link_libraries(neo_calibration_server PRIVATE stdc++fs)
endif()

# ========================================================================================
# Tests
# ========================================================================================
find_package(GTest REQUIRED)
include(GoogleTest)

add_executable(neo_calibration_server_tests
    protobuf_api/CameraRouteTests.cpp
    VersionServiceTests.cpp
)

target_compile_features(neo_calibration_server_tests PRIVATE cxx_std_17)
target_compile_options(neo_calibration_server_tests PRIVATE --coverage -O0)
target_link_options(neo_calibration_server_tests PRIVATE --coverage)

set_target_properties(neo_calibration_server_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests/unittests"
)

target_include_directories(neo_calibration_server 
    PUBLIC  
        "${CMAKE_CURRENT_SOURCE_DIR}/.."
)

target_link_libraries(neo_calibration_server_tests
    PRIVATE
        neo_calibration_camera
        neo_calibration_common
        neo_calibration_proto    
        neo_calibration_server
        GTest::gtest
        GTest::gmock
        GTest::gtest_main   
)

gtest_discover_tests(neo_calibration_server_tests
    TEST_EXECUTABLE "$<TARGET_FILE:neo_calibration_server_tests>"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/tests/unittests"
)