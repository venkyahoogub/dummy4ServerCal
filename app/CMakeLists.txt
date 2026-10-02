find_package(asio REQUIRED)

add_executable(neo_calibration_server_app server.cpp)
add_executable(neo_calibration_tester_app tester.cpp)
add_executable(neo_calibration_tester_video_app tester_video.cpp)
add_executable(neo_calibration_tester_motor_app tester_motor.cpp)
add_executable(neo_calibration_tester_uv_on_off_app tester_uv_on_off.cpp)
add_executable(neo_calibration_tester_uv_app tester_uv.cpp)
add_executable(neo_calibration_tester_hbc_app tester_hbc.cpp)
add_executable(neo_calibration_tester_calibration_app tester_calibration.cpp)
add_executable(neo_calibration_tester_depthsensor_app tester_depthsensor.cpp)
add_executable(neo_calibration_tester_uv_device_settings_app tester_uv_device_settings.cpp)

target_link_libraries(neo_calibration_server_app
    PRIVATE
        dmd_driver
        neo_calibration_camera
        neo_calibration_common        
        neo_calibration_proto         
        neo_calibration_server   
)

target_compile_definitions(neo_calibration_server_app PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
)

target_link_libraries(neo_calibration_tester_app 
    PUBLIC asio::asio 
    PRIVATE 
        neo_calibration_proto
        neo_calibration_common        
)

find_package(OpenCV REQUIRED)

target_link_libraries(neo_calibration_tester_video_app 
    PUBLIC asio::asio 
    PRIVATE 
        neo_calibration_proto
        neo_calibration_common
        opencv_core
        opencv_imgcodecs         
)

target_link_libraries(neo_calibration_tester_motor_app 
    PUBLIC asio::asio 
    PRIVATE 
        neo_calibration_proto
        neo_calibration_common
        opencv_core
        opencv_imgcodecs         
)

target_link_libraries(neo_calibration_tester_uv_app 
    PUBLIC asio::asio 
    PRIVATE 
        neo_calibration_proto
        neo_calibration_common
        opencv_core
        opencv_imgcodecs         
)

find_package(nlohmann_json 3.2.0 REQUIRED)

target_link_libraries(neo_calibration_tester_hbc_app 
    PUBLIC asio::asio 
    PRIVATE 
        neo_calibration_proto
        neo_calibration_common
        opencv_core
        opencv_imgcodecs
        nlohmann_json::nlohmann_json  
)

target_link_libraries(neo_calibration_tester_calibration_app 
    PUBLIC asio::asio 
    PRIVATE 
        neo_calibration_proto
        neo_calibration_common
        opencv_core
        opencv_imgcodecs
        nlohmann_json::nlohmann_json  
)

target_link_libraries(neo_calibration_tester_uv_on_off_app 
    PUBLIC asio::asio 
    PRIVATE 
        neo_calibration_proto
        neo_calibration_common
        opencv_core
        opencv_imgcodecs         
)

target_link_libraries(neo_calibration_tester_depthsensor_app 
    PUBLIC asio::asio 
    PRIVATE 
        neo_calibration_proto
        neo_calibration_common
        opencv_core
        opencv_imgcodecs         
)

target_link_libraries(neo_calibration_tester_uv_device_settings_app 
    PUBLIC asio::asio 
    PRIVATE 
        neo_calibration_proto
        neo_calibration_common
        opencv_core
        opencv_imgcodecs         
)

foreach(target
    neo_calibration_server_app
    neo_calibration_tester_app
    neo_calibration_tester_video_app
    neo_calibration_tester_motor_app
    neo_calibration_tester_uv_app
    neo_calibration_tester_uv_on_off_app
    neo_calibration_tester_hbc_app
    neo_calibration_tester_calibration_app
    neo_calibration_tester_depthsensor_app
    neo_calibration_tester_uv_device_settings_app)
    set_target_properties(${target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY 
        "${CMAKE_BINARY_DIR}/bin"
    )
endforeach()

set(SERVER_TARGET neo_calibration_server_app)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_custom_command(
        TARGET ${SERVER_TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${SERVER_TARGET}>/assets"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/assets" "$<TARGET_FILE_DIR:${SERVER_TARGET}>/assets"
        COMMENT "Copying mocked camera assets to the Debug binary folder..."
    )

    foreach(target
    neo_calibration_server_app
    neo_calibration_tester_app
    neo_calibration_tester_video_app
    neo_calibration_tester_motor_app
    neo_calibration_tester_uv_app
    neo_calibration_tester_uv_on_off_app
    neo_calibration_tester_hbc_app
    neo_calibration_tester_calibration_app
    neo_calibration_tester_depthsensor_app
    neo_calibration_tester_uv_device_settings_app)
        target_compile_options(${target} PRIVATE --coverage -O0)
        target_link_options(${target} PRIVATE --coverage)
    endforeach()
endif()