# Parameters
version="${1:-"0dev"}"

# Stop immediately if any build or test command fails
set -e

# Dependencies
conan install . \
    --output-folder=build/Release \
    -s build_type=Release -s compiler.cppstd=17 -s:b compiler.cppstd=17 \
    --build=missing 

# Configuration
cmake -S . -B build/Release \
      -DCMAKE_TOOLCHAIN_FILE=build/Release/generators/conan_toolchain.cmake \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_BUILD_SERVER_VERSION="$version"

# Build
cmake --build build/Release