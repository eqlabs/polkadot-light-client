#include "host/api.h"

#include <boost/assert.hpp>
#include <wasm-interpreter.h>
#include "runtime/ptr.h"

namespace plc::core::host {

Api::Api() {
    m_wasm_log_level_map[soralog::Level::ERROR] = 0;
    m_wasm_log_level_map[soralog::Level::WARN] = 1;
    m_wasm_log_level_map[soralog::Level::INFO] = 2;
    m_wasm_log_level_map[soralog::Level::VERBOSE] = 3;
    m_wasm_log_level_map[soralog::Level::DEBUG] = 4;
    m_wasm_log_level_map[soralog::Level::TRACE] = 5;
}

wasm::Literals Api::ext_logging_max_level_version_1(const wasm::LiteralList& arguments) {    
    ensureArgumentsSize(arguments.size(), 0);
    return wasm::Literals({wasm::Literal(m_wasm_log_level_map[m_log->level()])});
}

wasm::Literals Api::ext_logging_log_version_1(const wasm::LiteralList& arguments) {
    ensureArgumentsSize(arguments.size(), 3);
    uint32_t level = arguments[0].geti32();
    plc::core::runtime::Ptr target(arguments[1].geti64());
    plc::core::runtime::Ptr message(arguments[2].geti64());

    auto it = std::find_if(m_wasm_log_level_map.begin(), m_wasm_log_level_map.end(), [level](auto kv){
        return level == kv.second;
    });

    auto soralog_level = soralog::Level::OFF;
    if (it != m_wasm_log_level_map.end()) {
        soralog_level = it->first;
    }    
    std::string targetStr = m_memory->loadString(target);
    std::string messageStr = m_memory->loadString(message);

    m_log->log(soralog_level, "{}: {}", targetStr, messageStr);

    return wasm::Literals();
}

wasm::Literals Api::ext_allocator_malloc_version_1(const wasm::LiteralList& arguments) {
    ensureArgumentsSize(arguments.size(), 1);
    uint32_t size = arguments[0].geti32();

    return wasm::Literals({wasm::Literal(m_memory->allocate(size))});    
}

wasm::Literals Api::ext_allocator_free_version_1(const wasm::LiteralList& arguments) {
    ensureArgumentsSize(arguments.size(), 1);
    uint32_t ptr = arguments[0].geti32();

    auto result = m_memory->deallocate(ptr);
    if (!result) {
        m_log->warn("Ptr {} does not point to any memory chunk in wasm memory. Nothing deallocated");
    }
    return wasm::Literals();
}

} //namespace plc::core::host
