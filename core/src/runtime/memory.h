#pragma once

#include <string>

#include <shell-interface.h>

#include "memory_allocator.h"
#include "runtime/ptr.h"

namespace plc::core::runtime {

class Memory final {
public:
    Memory(wasm::ShellExternalInterface::Memory *memory, uint32_t heap_base) : m_memory(memory) {        
        m_memory_allocator = std::make_unique<MemoryAllocator>(heap_base, [this](size_t new_size) {
            m_memory->resize(new_size);
        });
    }

    uint32_t allocate(uint32_t size);
    std::optional<uint32_t> deallocate(uint32_t ptr);

    std::string loadString(Ptr ptr);

private:
    wasm::ShellExternalInterface::Memory *m_memory;
    std::unique_ptr<MemoryAllocator> m_memory_allocator;
};

} //namespace plc::core::runtime
