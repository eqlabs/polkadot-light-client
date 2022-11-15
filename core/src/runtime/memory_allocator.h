#pragma once

#include <map>
#include <unordered_map>

#include <libp2p/log/logger.hpp>

#include "runtime/ptr.h"
#include "utils/types.h"

/*
 * Code of this class was based on MemoryAllocator class from Kagome: https://github.com/soramitsu/kagome/blob/master/core/runtime/common/memory_allocator.hpp
 */

namespace plc::core::runtime {

// Alignment for pointers, same with substrate:
// https://github.com/paritytech/substrate/blob/743981a083f244a090b40ccfb5ce902199b55334/primitives/allocator/src/freeing_bump.rs#L56
constexpr uint8_t alignment = sizeof(size_t);
constexpr size_t initial_memory_size = 2097152UL; //2 MB
constexpr static uint32_t max_memory_size = std::numeric_limits<uint32_t>::max();

class MemoryAllocator final {
public:
    MemoryAllocator(WasmSize heap_base, std::function<void(size_t)> resize_handle) 
        : m_offset(heap_base), m_size(initial_memory_size), m_resize_handle(resize_handle) {}

    WasmPtr allocate(WasmSize size);
    std::optional<WasmSize> deallocate(WasmPtr ptr);
    bool checkAddress(Ptr ptr) {
        return m_offset > ptr.m_addr && m_offset - ptr.m_addr >= ptr.m_size;
    }

private:
    WasmPtr freealloc(WasmSize size);
    WasmPtr growAlloc(WasmSize size);
    void resize(WasmSize new_size);

    WasmSize m_offset;
    size_t m_size;
    std::unordered_map<WasmPtr, WasmSize> m_allocated;
    std::map<WasmPtr, WasmSize> m_deallocated;
    
    libp2p::log::Logger m_log = libp2p::log::createLogger("runtime::MemoryAllocator", "runtime");

    std::function<void(size_t)> m_resize_handle;
};

} //namespace plc::core::runtime
