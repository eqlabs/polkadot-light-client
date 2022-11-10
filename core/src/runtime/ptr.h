#pragma once

namespace plc::core::runtime {

struct Ptr {
    Ptr() = default;
    Ptr(uint64_t wasmPtr) {
        addr = wasmPtr & 0xFFFFFFFFLLU;
        size = (wasmPtr >> 32u) & 0xFFFFFFFFLLU;
    }

    uint32_t addr;
    uint32_t size;
};

} //namespace plc::core::runtime {
