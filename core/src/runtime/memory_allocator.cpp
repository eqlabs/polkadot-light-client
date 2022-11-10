#include "memory_allocator.h"

#include <boost/assert.hpp>

namespace plc::core::runtime {

inline constexpr uint32_t roundUp(uint32_t t) {
    static_assert((alignment & (alignment - 1)) == 0, "Must be POW 2!");
    static_assert(alignment != 0, "Must not be 0!");
    return (t + (alignment - 1)) & ~(alignment - 1);
}


uint32_t MemoryAllocator::allocate(uint32_t size) {
    if (size == 0) {
        return 0;
    }
    const auto ptr = m_offset;
    const auto new_offset = roundUp(ptr + size);  // align

    // Round up allocating chunk of memory
    size = new_offset - ptr;

    BOOST_ASSERT(m_allocated.find(ptr) == m_allocated.end());
    if (max_memory_size - m_offset < size) {  // overflow
        m_log->error("Overflow occured when trying to allocate {} bytes at offset 0x{:x}", size, m_offset);
        return 0;
    }
    if (new_offset <= m_size) {
        m_offset = new_offset;
        m_allocated[ptr] = size;
        return ptr;
    }

    auto res = freealloc(size);
    return res;
}

std::optional<uint32_t> MemoryAllocator::deallocate(uint32_t ptr) {
    auto a_it = m_allocated.find(ptr);
    if (a_it == m_allocated.end()) {
        return std::nullopt;
    }

    auto a_node = m_allocated.extract(a_it);
    auto size = a_node.mapped();
    auto [d_it, is_emplaced] = m_deallocated.emplace(ptr, size);
    BOOST_ASSERT(is_emplaced);

    // Combine with next chunk if it adjacent
    while (true) {
        auto node = m_deallocated.extract(ptr + size);
        if (!node) break;
        d_it->second += node.mapped();
    }

    // Combine with previous chunk if it adjacent
    while (m_deallocated.begin() != d_it) {
        auto d_it_prev = std::prev(d_it);
        if (d_it_prev->first + d_it_prev->second != d_it->first) {
            break;
        }
        d_it_prev->second += d_it->second;
        m_deallocated.erase(d_it);
        d_it = d_it_prev;
    }

    auto d_it_next = std::next(d_it);
    if (d_it_next == m_deallocated.end()) {
        if (d_it->first + d_it->second == m_offset) {
            m_offset = d_it->first;
            m_deallocated.erase(d_it);
        }
    }
    return size;
}

uint32_t MemoryAllocator::freealloc(uint32_t size) {
    if (size == 0) {
        return 0;
    }

    size = roundUp(size);

    auto min_chunk_size = std::numeric_limits<uint32_t>::max();
    uint32_t ptr = 0;
    for (const auto &[chunk_ptr, chunk_size] : m_deallocated) {
        BOOST_ASSERT(chunk_size > 0);
        if (chunk_size >= size and chunk_size < min_chunk_size) {
                min_chunk_size = chunk_size;
                ptr = chunk_ptr;
                if (min_chunk_size == size) {
                    break;
                }
        }
    }
    // if (ptr == 0) {
    //   // if did not find available space among deallocated memory chunks,
    //   // then grow memory and allocate in new space
    //   return growAlloc(size);
    // }

    const auto node = m_deallocated.extract(ptr);
    BOOST_ASSERT_MSG(!node.empty(),
                     "pointer to the node was received by searching list of "
                     "deallocated nodes, must not be none");

    auto old_size = node.mapped();
    if (old_size > size) {
        auto new_ptr = ptr + size;
        auto new_size = old_size - size;
        m_deallocated[new_ptr] = new_size;
    }
    m_allocated[ptr] = size;
    return ptr;
}

uint32_t MemoryAllocator::growAlloc(uint32_t size) {
    // check that we do not exceed max memory size
    if (max_memory_size - m_offset < size) {
        m_log->error("Memory size exceeded when growing it on {} bytes, offset was 0x{:x}", size, m_offset);
        return 0;
    }
    // try to increase memory size up to offset + size * 4 (we multiply by 4
    // to have more memory than currently needed to avoid resizing every time
    // when we exceed current memory)
    if ((max_memory_size - m_offset) / 4 > size) {
        resize(m_offset + size * 4);
    } else {
      // if we can't increase by size * 4 then increase memory size by
      // provided size
      resize(m_offset + size);
    }
    return allocate(size);
}

void MemoryAllocator::resize(uint32_t new_size) {
    /**
     * We use this condition to avoid deallocated_ pointers fixup
     */
    BOOST_ASSERT(m_offset <= max_memory_size - new_size);
    if (new_size >= m_size) {
        m_size = new_size;
        m_resize_handle(new_size);
    }
}

} //namespace plc::core::runtime
