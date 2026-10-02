#!/usr/bin/env bash
set -euo pipefail

BUILD_TYPE="${1:-Release}"
CERT_FILE="${2:-}"

# Define host paths for external SDK build contexts
PYLON_SDK_PATH="${PYLON_SDK_PATH:-/opt/pylon}"
ALP43_HEADERS_PATH="${ALP43_HEADERS_PATH:-/usr/include/alp43}"
ALP43_LIBS_PATH="${ALP43_LIBS_PATH:-/usr/lib}"

IMAGE_TAG="neo-calibration-tool-server:latest"

# Validate required build context directories on the host
if [ ! -d "$PYLON_SDK_PATH" ]; then
    echo "Error: Basler Pylon SDK context directory '$PYLON_SDK_PATH' does not exist." >&2
    exit 1
fi

if [ ! -d "$ALP43_HEADERS_PATH" ]; then
    echo "Error: ALP43 headers directory '$ALP43_HEADERS_PATH' does not exist." >&2
    exit 1
fi

if [ ! -d "$ALP43_LIBS_PATH" ]; then
    echo "Error: ALP43 library directory '$ALP43_LIBS_PATH' does not exist." >&2
    exit 1
fi

SECRET_ARGS=()

if [ -n "$CERT_FILE" ]; then
  if [ -f "$CERT_FILE" ]; then
    echo "Using corporate certificate: $CERT_FILE"
    SECRET_ARGS+=(--secret "id=corporate_ca,src=$CERT_FILE")
  else
    echo "Error: Certificate specified at '$CERT_FILE' does not exist." >&2
    exit 1
  fi
else
  echo "No corporate certificate specified. Skipping secret mount."
fi

echo "Building $IMAGE_TAG..."

# Execute BuildKit build passing all three context mounts
docker buildx build \
  "${SECRET_ARGS[@]}" \
  --build-context "pylon-sdk=${PYLON_SDK_PATH}" \
  --build-context "dmd-headers=${ALP43_HEADERS_PATH}" \
  --build-context "dmd-libs=${ALP43_LIBS_PATH}" \
  --build-arg BUILD_TYPE="$BUILD_TYPE" \
  -t "$IMAGE_TAG" \
  --no-cache \
  --load .