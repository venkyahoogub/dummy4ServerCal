cmake_minimum_required(VERSION 3.16)
project(dmd_driver)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(src)

target_include_directories(dmd_driver PRIVATE
	/usr/include/alp43
)

target_include_directories(dmd_driver PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(dmd_driver PRIVATE alp43)
