# This CMake file is used by Conan as part of the UVC build process.
# It is not used by the HBC firmware build process.

add_library(hbc_protos STATIC hbc.proto)
protobuf_generate(TARGET hbc_protos LANGUAGE cpp)

target_include_directories(hbc_protos PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(hbc_protos protobuf::libprotobuf)
