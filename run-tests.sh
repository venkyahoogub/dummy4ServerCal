# Stop immediately if any build or test command fails
set -e

# Start with fresh build
./build-debug.sh

# Run tests
ctest --test-dir build/Debug --output-on-failure

# Generate coverage report. 
mkdir -p ./build/Analysis/coverage/
gcovr --root . \
      --filter src/ \
      --merge-mode-functions=separate \
      --html-details build/Analysis/coverage/coverage.html \
      --print-summary