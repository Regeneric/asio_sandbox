#pragma once

#include <memory>
#include <thread>
#include <mutex>
#include <deque>
#include <optional>
#include <vector>
#include <iostream>
#include <algorithm>
#include <chrono>

#include <cstdint>

#define ASIO_STANDALONE
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>

using Byte = uint8_t;
using Word = uint16_t;
using Dword = uint32_t;
using Qword = uint64_t;