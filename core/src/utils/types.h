#pragma once

#include <vector>

namespace plc::core {

using ByteBuffer = std::vector<uint8_t>;
using BlockHash = std::array<uint8_t, 32>;
using BlockNumber = uint32_t;
using WasmSize = uint32_t;
using WasmPtr = uint32_t;

} //namespace plc::core {
