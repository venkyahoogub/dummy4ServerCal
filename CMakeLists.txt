# Project Setup
cmake_minimum_required(VERSION 3.15)
project(neo-calibration-tool-server)

# Output directories
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(CMAKE_BUILD_SERVER_VERSION)
    add_compile_definitions(BUILD_SERVER_VERSION=${CMAKE_BUILD_SERVER_VERSION})
endif()

# Build proto library
find_package(protobuf REQUIRED)
find_package(OpenCV REQUIRED)
get_filename_component(PROTO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/src" ABSOLUTE)
set(PROTO_FILE "${PROTO_ROOT}/neo-calibration-server-api/calibration_server_api.proto")
set(PROTO_GEN_DIR "${CMAKE_BINARY_DIR}/src")
file(MAKE_DIRECTORY "${PROTO_GEN_DIR}")
set_source_files_properties(${PROTO_SRCS} PROPERTIES COMPILE_FLAGS "-g0")

protobuf_generate(
    OUT_VAR PROTO_SRCS
    OUT_HDR PROTO_HDRS
    LANGUAGE cpp
    IMPORT_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/src"
    PROTOS "${PROTO_FILE}"
    PROTOC_OUT_DIR "${PROTO_GEN_DIR}"
)

add_library(neo_calibration_proto STATIC ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(neo_calibration_proto PUBLIC protobuf::libprotobuf)
target_include_directories(neo_calibration_proto 
    PUBLIC "${PROTO_GEN_DIR}"
)

# -------------------------------------------------------------------
# HBC protobuf generation
# -------------------------------------------------------------------
get_filename_component(HBC_PROTO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/neo-hbc-api" ABSOLUTE)
set(HBC_PROTO_FILE "${HBC_PROTO_DIR}/hbc.proto")
set(HBC_PROTO_GEN_DIR "${CMAKE_BINARY_DIR}/src")
file(MAKE_DIRECTORY "${HBC_PROTO_GEN_DIR}")
protobuf_generate(
    OUT_VAR HBC_PROTO_SRCS
    OUT_HDR HBC_PROTO_HDRS
    LANGUAGE cpp
    IMPORT_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/src"
    PROTOS "${HBC_PROTO_FILE}"
    PROTOC_OUT_DIR "${HBC_PROTO_GEN_DIR}"
)

add_library(hbc_proto STATIC
    ${HBC_PROTO_SRCS}
    ${HBC_PROTO_HDRS}
)

target_include_directories(hbc_proto PUBLIC "${HBC_PROTO_GEN_DIR}")
target_link_libraries(hbc_proto PUBLIC protobuf::libprotobuf)

enable_testing()

# Assemble the applications
add_subdirectory(src/neo-dmd-driver) # Builds DMD Driver
add_subdirectory(src/camera)         # Builds to the camera library
add_subdirectory(src/common)         # Builds to the common library
add_subdirectory(src/server)         # Builds to the server library that combines protobuf with the core capabilities.
add_subdirectory(src/app)            # Builds to the application