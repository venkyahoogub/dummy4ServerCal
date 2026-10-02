# Stop immediately if any build or test command fails
set -e

# Dependencies
conan install . \
    --output-folder=build/Debug \
    -s build_type=Debug -s compiler.cppstd=17 -s:b compiler.cppstd=17 \
    --build=missing

# Configuration
cmake -S . -B build/Debug \
      -DCMAKE_TOOLCHAIN_FILE=build/Debug/generators/conan_toolchain.cmake \
      -DCMAKE_BUILD_TYPE=Debug

# Build
cmake --build build/Debug