#pragma once

#include <algorithm>
#include <arpa/inet.h>
#include <condition_variable>
#include <csignal>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>

constexpr size_t DISPLAY_WIDTH = 1024;
constexpr size_t DISPLAY_HEIGHT = 768;
constexpr size_t BIN_TOPDOWN_COMPRESSION_RATIO = 8;
constexpr size_t FRAME_SIZE_RAW = (DISPLAY_WIDTH * DISPLAY_HEIGHT);
constexpr size_t FRAME_SIZE_BIN_TOPDOWN = (FRAME_SIZE_RAW / BIN_TOPDOWN_COMPRESSION_RATIO);
constexpr double OPTICAL_MAGNIFICATION = -1.5727;
constexpr double DMD_PIXEL_PITCH_UM = 13.68;

using Frame = std::unique_ptr<uint8_t[]>;
