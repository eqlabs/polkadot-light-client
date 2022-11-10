#pragma once

#include <map>
#include <unordered_map>

#include <libp2p/log/logger.hpp>

namespace plc::core::runtime {

// Alignment for pointers, same with substrate:
// https://github.com/paritytech/substrate/blob/743981a083f244a090b40ccfb5ce902199b55334/primitives/allocator/src/freeing_bump.rs#L56
constexpr uint8_t alignment = sizeof(size_t);
constexpr size_t initial_memory_size = 2097152UL; //MB
constexpr static uint32_t max_memory_size = std::numeric_limits<uint32_t>::max();

class MemoryAllocator final {
public:
    MemoryAllocator(uint32_t heap_base, std::function<void(size_t)> resize_handle) 
        : m_offset(heap_base), m_size(initial_memory_size), m_resize_handle(resize_handle) {}

    uint32_t allocate(uint32_t size);
    std::optional<uint32_t> deallocate(uint32_t ptr);

private:
    uint32_t freealloc(uint32_t size);
    uint32_t growAlloc(uint32_t size);
    void resize(uint32_t new_size);

    uint32_t m_offset;
    size_t m_size;
    std::unordered_map<uint32_t, uint32_t> m_allocated;
    std::map<uint32_t, uint32_t> m_deallocated;
    
    libp2p::log::Logger m_log = libp2p::log::createLogger("Module", "runtime");

    std::function<void(size_t)> m_resize_handle;
};

} //namespace plc::core::runtime
