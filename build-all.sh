# Parameters
version="${1:-"0dev"}"

# Stop immediately if any build or test command fails
set -e

# Build a debug build and run the unit tests
./run-tests.sh

set +e
./static-checks.sh
STATIC_EXIT_CODE=$?

# If static checks failed, print the hidden report to the TeamCity screen
if [ $STATIC_EXIT_CODE -ne 0 ]; then
    echo "===================================================="
    echo " !!! STATIC CHECKS FAILED: PRINTING CPPCHECK REPORT !!!"
    echo "===================================================="
    cat ./build/Analysis/cppcheck_report.txt
    cat ./build/Analysis/format_report.txt
    exit $STATIC_EXIT_CODE
fi

# Re-enable exit-on-error for the final step
set -e

# Then run a release build if it passes.
./build-release.sh ${version}