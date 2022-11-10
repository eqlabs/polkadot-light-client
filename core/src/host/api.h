#pragma once

#include <libp2p/log/logger.hpp>
#include <literal.h>

#include "runtime/memory.h"

namespace wasm {
    typedef std::vector<Literal> LiteralList;
}

namespace plc::core::host {

class Api final {
public:
    Api() = default;

    void setMemory(std::shared_ptr<plc::core::runtime::Memory> memory) {
        m_memory = memory;
    }

    wasm::Literals ext_logging_max_level_version_1(const wasm::LiteralList& arguments);
    wasm::Literals ext_logging_log_version_1(const wasm::LiteralList& arguments);
    wasm::Literals ext_allocator_malloc_version_1(const wasm::LiteralList& arguments);
    wasm::Literals ext_allocator_free_version_1(const wasm::LiteralList& arguments);

    libp2p::log::Logger m_log = libp2p::log::createLogger("Module", "host");

private:
    std::shared_ptr<plc::core::runtime::Memory> m_memory;
};

} //namespace plc::core::host
