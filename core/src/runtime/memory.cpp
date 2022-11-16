#include "runtime/memory.h"

#include <boost/assert.hpp>

namespace plc::core::runtime {

WasmSize Memory::allocate(WasmSize size) {
    return m_memory_allocator->allocate(size);
}

std::optional<WasmSize> Memory::deallocate(WasmPtr ptr) {
    return m_memory_allocator->deallocate(ptr);
}

std::string Memory::loadString(Ptr ptr) {
    std::string res;
    res.reserve(ptr.m_size);
    for (auto i = ptr.m_addr; i < ptr.m_addr + ptr.m_size; ++i) {
        res.push_back(static_cast<char>(m_memory->get<uint8_t>(i)));
    }
    return res;
}

ByteBuffer Memory::loadBytes(Ptr ptr) {
    ByteBuffer res;
    res.reserve(ptr.m_size);
    for (auto i = ptr.m_addr; i < ptr.m_addr + ptr.m_size; ++i) {
        res.push_back(m_memory->get<uint8_t>(i));
    }
    return res;
}

Ptr Memory::storeBytes(const ByteBuffer &bytes) {
    auto addr = m_memory_allocator->allocate(bytes.size());
    if (addr == 0) {
        return Ptr();
    }
    Ptr ptr(addr, bytes.size());
    BOOST_ASSERT(m_memory_allocator->checkAddress(ptr));
    for (WasmPtr i = ptr.m_addr, j = 0; i < ptr.m_addr + ptr.m_size; i++, j++) {
        m_memory->set(i, bytes[j]);
    }
    return ptr;
}

} //namespace plc::core::runtime
