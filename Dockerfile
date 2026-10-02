# ==========================================
# STAGE 1: Build Environment
# ==========================================
FROM torizon/debian:4 AS builder

ARG BUILD_TYPE=Release
ARG VERSION=0dev

ENV DEBIAN_FRONTEND=noninteractive
ENV SSL_CERT_FILE=/etc/ssl/certs/ca-certificates.crt
ENV SSL_CERT_DIR=/etc/ssl/certs
ENV REQUESTS_CA_BUNDLE=/etc/ssl/certs/ca-certificates.crt
ENV CURL_CA_BUNDLE=/etc/ssl/certs/ca-certificates.crt
ENV CONAN_CACERT_PATH=/etc/ssl/certs/ca-certificates.crt

# Added libusb-1.0-0-dev for libalp43 dependency resolution
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    ca-certificates \
    cmake \
    curl \
    git \
    libusb-1.0-0-dev \
    openssl \
    pkg-config \
    python3 \
    python3-pip \
    python3-venv \
    && rm -rf /var/lib/apt/lists/*

RUN --mount=type=secret,id=corporate_ca,target=/tmp/zs1.crt,required=false \
    if [ -f /tmp/zs1.crt ]; then \
    cp /tmp/zs1.crt /usr/local/share/ca-certificates/zs1.crt && \
    update-ca-certificates; \
    fi

RUN python3 -m venv /opt/venv
ENV PATH="/opt/venv/bin:$PATH"

RUN --mount=type=secret,id=corporate_ca,target=/tmp/zs1.crt,required=false \
    if [ -f /tmp/zs1.crt ]; then \
    pip install --no-cache-dir --cert /tmp/zs1.crt conan==2.21.0 certifi && \
    python3 -c "import certifi; open(certifi.where(), 'a').write(open('/tmp/zs1.crt').read())"; \
    else \
    pip install --no-cache-dir conan==2.21.0 certifi; \
    fi

RUN conan profile detect --force && \
    conan config set global.cacert_path=/etc/ssl/certs/ca-certificates.crt 2>/dev/null || true

WORKDIR /workspace
COPY . .

RUN --mount=type=bind,from=pylon-sdk,target=/opt/pylon \
    --mount=type=bind,from=dmd-headers,target=/usr/include/alp43 \
    --mount=type=bind,from=dmd-libs,target=/tmp/alp43-libs \
    mkdir -p /usr/local/lib/alp43 && \
    find /tmp/alp43-libs -maxdepth 2 -name "libalp*.so*" -exec cp -d {} /usr/local/lib/alp43/ \; && \
    export LIBRARY_PATH="/usr/local/lib/alp43:/opt/pylon/lib:${LIBRARY_PATH:-}" && \
    export LD_LIBRARY_PATH="/opt/pylon/lib:/usr/local/lib/alp43:${LD_LIBRARY_PATH:-}" && \
    export LDFLAGS="-Wl,-rpath-link,/lib/x86_64-linux-gnu:-Wl,-rpath-link,/usr/lib/x86_64-linux-gnu:-Wl,-rpath,/usr/local/lib/alp43 ${LDFLAGS:-}" && \
    chmod +x build-release.sh build-debug.sh && \
    if [ "$BUILD_TYPE" = "Debug" ]; then \
    ./build-debug.sh; \
    else \
    ./build-release.sh "$VERSION"; \
    fi

# Clean host-provided system GLIBC libraries prior to image copy
RUN find /usr/local/lib/alp43 -type f \( -name "libc.so*" -o -name "libm.so*" -o -name "ld-linux*" \) -delete

# ==========================================
# STAGE 2: Runtime Container
# ==========================================
FROM torizon/debian:4 AS runner

ARG BUILD_TYPE=Release
ENV DEBIAN_FRONTEND=noninteractive
ENV SSL_CERT_FILE=/etc/ssl/certs/ca-certificates.crt
ENV REQUESTS_CA_BUNDLE=/etc/ssl/certs/ca-certificates.crt

RUN --mount=type=secret,id=corporate_ca,target=/tmp/zs1.crt,required=false \
    mkdir -p /usr/local/share/ca-certificates && \
    if [ -f /tmp/zs1.crt ]; then \
    cp /tmp/zs1.crt /usr/local/share/ca-certificates/zs1.crt; \
    fi

RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
    libgl1 \
    libglib2.0-0 \
    libusb-1.0-0 \
    && update-ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Copy Pylon and ALP43 libraries, and register paths via ldconfig
RUN --mount=type=bind,from=pylon-sdk,target=/tmp/pylon \
    --mount=type=bind,from=dmd-libs,target=/tmp/alp43-libs \
    mkdir -p /usr/local/lib/pylon /usr/local/lib/alp43 && \
    if [ -d "/tmp/pylon/lib" ]; then cp -d /tmp/pylon/lib/*.so* /usr/local/lib/pylon/ 2>/dev/null || true; fi && \
    if [ -d "/tmp/pylon/lib64" ]; then cp -d /tmp/pylon/lib64/*.so* /usr/local/lib/pylon/ 2>/dev/null || true; fi && \
    find /tmp/alp43-libs -maxdepth 2 -name "libalp*.so*" -exec cp -d {} /usr/local/lib/alp43/ \; && \
    echo "/usr/local/lib/pylon" > /etc/ld.so.conf.d/pylon.conf && \
    echo "/usr/local/lib/alp43" > /etc/ld.so.conf.d/alp43.conf && \
    ldconfig

WORKDIR /app

COPY --from=builder /workspace/build/${BUILD_TYPE}/bin/ /app/
COPY --from=builder /usr/local/lib/alp43 /usr/local/lib/alp43

ENV LD_LIBRARY_PATH="/usr/local/lib/pylon:/usr/local/lib/alp43"
ENV GENICAM_GENTI_64="/usr/local/lib/pylon/gentl/usb3vision.cti"
ENV PYLON_ROOT="/opt/pylon"
ENV PATH="/app:${PATH}"

CMD ["/app/neo_calibration_server_app"]