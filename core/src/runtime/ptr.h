#pragma once

#include "utils/types.h"

namespace plc::core::runtime {

struct Ptr {
    Ptr() = default;
    Ptr(uint64_t wasmPtr) {
        m_addr = wasmPtr & 0xFFFFFFFFLLU;
        m_size = (wasmPtr >> 32u) & 0xFFFFFFFFLLU;
    }
    Ptr(WasmPtr addr, WasmSize size) : m_addr(addr), m_size(size) {}

    WasmPtr m_addr;
    WasmSize m_size;
};

} //namespace plc::core::runtime {
