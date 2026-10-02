mkdir -p ./build/Analysis

# Stop immediately if any build or test command fails
set -e

# Resolve includes for conan and standard library for cppcheck
SYS_INCLUDES=""
for path in /usr/include/c++/* /usr/include/x86_64-linux-gnu/c++/*; do
    if [ -d "$path" ]; then
        SYS_INCLUDES="$SYS_INCLUDES -isystem $path"
    fi
done

CONAN_INCLUDES=""
if [ -f "conanfile.txt" ]; then
    echo "Resolving Conan 2.X dependency include paths..."
        CONAN_FLAGS="-s build_type=Release -s compiler.cppstd=17 -s:b compiler.cppstd=17"
    
    if command -v jq &> /dev/null; then
        for pkg in $(conan graph info . $CONAN_FLAGS --format=json | jq -r '.. | .package_folder? // empty'); do
            if [ -d "$pkg/include" ]; then
                CONAN_INCLUDES="$CONAN_INCLUDES -isystem $pkg/include"
            fi
        done
    else
        for pkg in $(conan graph info . $CONAN_FLAGS --format=json | awk -F'"package_folder": "' '{if(NF>1) {split($2,a,"\""); if(a[1] != "null" && a[1] != "") print a[1]}}'); do
            if [ -d "$pkg/include" ]; then
                CONAN_INCLUDES="$CONAN_INCLUDES -isystem $pkg/include"
            fi
        done
    fi
fi

echo "Running Cppcheck..."
# --error-exitcode to get the build to fail.
cppcheck --enable=all \
         --error-exitcode=1 \
         --xml \
         --inline-suppr \
         --suppress=checkersReport \
         --suppress=missingInclude \
         --suppress=missingIncludeSystem \
         --force \
         -I src/ \
         $SYS_INCLUDES \
         $CONAN_INCLUDES \
         -i src/neo-dmd-driver/ \
         -DEXPECT_THROW \
         src/ 2> ./build/Analysis/cppcheck_report.xml

if command -v cppcheck-htmlreport &> /dev/null; then
    echo "Generating Cppcheck HTML report..."
    mkdir -p ./build/Analysis/cppcheck-html
    cppcheck-htmlreport --file=./build/Analysis/cppcheck_report.xml \
                        --report-dir=./build/Analysis/cppcheck-html \
                        --title="Neo Calibration Tool Server - Cppcheck Report"
else
    echo "WARNING: cppcheck-htmlreport utility not found. Keeping raw XML."
fi

echo "Running Clang-Format check..."
# Ensure the target directory exists
mkdir -p ./build/Analysis/clang_format

set +e
find src -type d -path 'src/neo-dmd-driver' -prune -o \
     \( -name '*.cpp' -o -name '*.h' -o -name '*.hpp' \) -print \
  | xargs clang-format --dry-run --Werror 2> ./build/Analysis/clang_format/format_errors.tmp
set -e

HTML_REPORT="./build/Analysis/clang_format/results.html"

# Write out the base HTML template with updated dark mode styling foundations
echo "<html><head><title>Clang-Format Report</title>" > "$HTML_REPORT"
echo "<style>body{font-family:'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background-color:#121212; color:#e0e0e0; padding:30px;}" >> "$HTML_REPORT"
echo "pre{background:#1e1e1e; padding:15px; border-radius:4px; font-family:monospace; font-size:14px; overflow:auto; line-height:1.5;}</style>" >> "$HTML_REPORT"
echo "</head><body>" >> "$HTML_REPORT"

if [ $CLANG_EXIT -eq 0 ]; then
    # PASSED: Vivid green status banner and text
    echo "<div style='background-color:#1b5e20; border-left:6px solid #4caf50; padding:15px 20px; border-radius:4px; margin-bottom:20px;'>" >> "$HTML_REPORT"
    echo "<h2 style='color:#a5d6a7; margin:0 0 5px 0;'>Clang-Format Status: PASSED</h2>" >> "$HTML_REPORT"
    echo "<p style='color:#e8f5e9; margin:0;'>All scanned source files conform perfectly to the project style guide rules.</p>" >> "$HTML_REPORT"
    echo "</div>" >> "$HTML_REPORT"
else
    # FAILED: Deep red status banner and error display container
    echo "<div style='background-color:#b71c1c; border-left:6px solid #f44336; padding:15px 20px; border-radius:4px; margin-bottom:20px;'>" >> "$HTML_REPORT"
    echo "<h2 style='color:#ffcdd2; margin:0 0 5px 0;'>Clang-Format Status: FAILED</h2>" >> "$HTML_REPORT"
    echo "<p style='color:#ffebee; margin:0;'>Style mismatches detected. Please review and fix the formatting anomalies outlined below:</p>" >> "$HTML_REPORT"
    echo "</div>" >> "$HTML_REPORT"
    
    echo "<h3 style='color:#f44336; margin-top:25px;'>Formatting Divergences:</h3>" >> "$HTML_REPORT"
    echo "<pre style='border-left:4px solid #f44336; color:#f44336;'>" >> "$HTML_REPORT"
    cat ./build/Analysis/clang_format/format_errors.tmp >> "$HTML_REPORT"
    echo "</pre>" >> "$HTML_REPORT"
fi

echo "</body></html>" >> "$HTML_REPORT"

# Clean up the intermediate temporary file safely
rm -f ./build/Analysis/clang_format/format_errors.tmp

echo "Static checks done..."