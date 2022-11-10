#include "runtime/memory.h"

namespace plc::core::runtime {

uint32_t Memory::allocate(uint32_t size) {
    return m_memory_allocator->allocate(size);
}

std::optional<uint32_t> Memory::deallocate(uint32_t ptr) {
    return m_memory_allocator->deallocate(ptr);
}

std::string Memory::loadString(Ptr ptr) {
    std::string res;
    res.reserve(ptr.size);
    for (auto i = ptr.addr; i < ptr.addr + ptr.size; ++i) {
        res.push_back(static_cast<char>(m_memory->get<uint8_t>(i)));
    }
    return res;
}

} //namespace plc::core::runtime
