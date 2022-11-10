#pragma once

#include <map>

#include <libp2p/log/logger.hpp>
#include <literal.h>

#include "runtime/memory.h"

namespace wasm {
    typedef std::vector<Literal> LiteralList;
}

namespace plc::core::host {

class Api final {
public:
    Api();

    void setMemory(std::shared_ptr<plc::core::runtime::Memory> memory) {
        m_memory = memory;
    }

    wasm::Literals ext_logging_max_level_version_1(const wasm::LiteralList& arguments);
    wasm::Literals ext_logging_log_version_1(const wasm::LiteralList& arguments);
    wasm::Literals ext_allocator_malloc_version_1(const wasm::LiteralList& arguments);
    wasm::Literals ext_allocator_free_version_1(const wasm::LiteralList& arguments);

    libp2p::log::Logger m_log = libp2p::log::createLogger("Module", "host");

private:
    void ensureArgumentsSize(int argsSize, int size) {
        BOOST_ASSERT(argsSize == size);
    }
    std::shared_ptr<plc::core::runtime::Memory> m_memory;
    std::map<soralog::Level, int> m_wasm_log_level_map;
};

} //namespace plc::core::host
