#pragma once

namespace plc::core::runtime {

struct Ptr {
    Ptr() = default;
    Ptr(uint64_t wasmPtr) {
        const uint32_t ptr = wasmPtr & 0xFFFFFFFFLLU;
        const uint32_t size = (wasmPtr >> 32u) & 0xFFFFFFFFLLU;
    }

    uint32_t ptr;
    uint32_t size;
};

} //namespace plc::core::runtime {