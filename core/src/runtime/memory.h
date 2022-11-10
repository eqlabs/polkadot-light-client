#pragma once

#include <string>

#include <shell-interface.h>

#include "runtime/memory_allocator.h"
#include "runtime/ptr.h"
#include "utils/types.h"

namespace plc::core::runtime {

class Memory final {
public:
    Memory(wasm::ShellExternalInterface::Memory *memory, WasmSize heap_base) : m_memory(memory) {        
        m_memory_allocator = std::make_unique<MemoryAllocator>(heap_base, [this](size_t new_size) {
            m_memory->resize(new_size);
        });
    }

    WasmPtr allocate(WasmSize size);
    std::optional<WasmSize> deallocate(WasmPtr ptr);

    std::string loadString(Ptr ptr);
    ByteBuffer loadBytes(Ptr ptr);
    Ptr storeBytes(const ByteBuffer &bytes);

private:
    wasm::ShellExternalInterface::Memory *m_memory;
    std::unique_ptr<MemoryAllocator> m_memory_allocator;
};

} //namespace plc::core::runtime
