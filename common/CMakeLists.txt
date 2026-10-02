# This file builds a library that contains common implementations used across the
# other libraries. The goal is to keep it clean and as dependency free as possible.
find_package(spdlog REQUIRED)
find_package(nlohmann_json 3.2.0 REQUIRED)

add_library(neo_calibration_common STATIC
    PngUtils.cpp
    SerializationUtils.cpp
    Stacktrace.cpp
    TracedError.cpp
    CalibrationController.cpp
)

target_link_libraries(neo_calibration_common
    PUBLIC
        spdlog::spdlog # Ideally I wouldn't have to pull in additional dependencies
                       # in an attempt to keep this common folder as clean as possible.
                       # Making an exception for logging as it is more fundamental and
                       # all over the uvc code that I'm occasionally pulling patterns
                       # and implementations from.
        nlohmann_json::nlohmann_json  # For JSON serialization of calibration tables, I might remove this once I find a better way.
)

target_include_directories(neo_calibration_common 
    PUBLIC 
        "${CMAKE_CURRENT_SOURCE_DIR}/.."
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
    target_link_libraries(neo_calibration_common PRIVATE stdc++fs)
endif()