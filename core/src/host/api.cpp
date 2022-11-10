#include "host/api.h"

#include <boost/assert.hpp>
#include <wasm-interpreter.h>
#include "runtime/ptr.h"

namespace plc::core::host {

soralog::Level wasmLevelToLogLevel(int level) {
    switch (level) {
        case 0: return soralog::Level::ERROR;
        case 1: return soralog::Level::WARN;
        case 2: return soralog::Level::INFO;
        case 3: return soralog::Level::VERBOSE;
        case 4: return soralog::Level::DEBUG;
        case 5: return soralog::Level::TRACE;
    }
    return soralog::Level::OFF;
}

wasm::Literals Api::ext_logging_max_level_version_1(const wasm::LiteralList& arguments) {    
    BOOST_ASSERT(arguments.size() == 0);
    switch (m_log->level()) {
        case soralog::Level::OFF:
        case soralog::Level::CRITICAL:
        case soralog::Level::ERROR:
            return wasm::Literals({wasm::Literal(0)});
        case soralog::Level::WARN:
            return wasm::Literals({wasm::Literal(1)});
        case soralog::Level::INFO:
            return wasm::Literals({wasm::Literal(2)});
        case soralog::Level::VERBOSE:
            return wasm::Literals({wasm::Literal(3)});
        case soralog::Level::DEBUG:
            return wasm::Literals({wasm::Literal(4)});
        case soralog::Level::TRACE:
            return wasm::Literals({wasm::Literal(5)});
    }    
    return wasm::Literals();
}

wasm::Literals Api::ext_logging_log_version_1(const wasm::LiteralList& arguments) {
    BOOST_ASSERT(arguments.size() == 3);
    uint32_t level = arguments[0].geti32();
    plc::core::runtime::Ptr target(arguments[1].geti64());
    plc::core::runtime::Ptr message(arguments[2].geti64());

    auto soralogLevel = wasmLevelToLogLevel(level);
    std::string targetStr = m_memory->loadString(target);
    std::string messageStr = m_memory->loadString(message);

    m_log->log(soralogLevel, "{}: {}", targetStr, messageStr);

    return wasm::Literals();
}

wasm::Literals Api::ext_allocator_malloc_version_1(const wasm::LiteralList& arguments) {
    BOOST_ASSERT(arguments.size() == 1);
    uint32_t size = arguments[0].geti32();

    return wasm::Literals({wasm::Literal(m_memory->allocate(size))});
}

wasm::Literals Api::ext_allocator_free_version_1(const wasm::LiteralList& arguments) {
    BOOST_ASSERT(arguments.size() == 1);
    uint32_t ptr = arguments[0].geti32();

    auto result = m_memory->deallocate(ptr);
    if (!result) {
        m_log->warn("Ptr {} does not point to any memory chunk in wasm memory. Nothing deallocated");
    }
    return wasm::Literals();
}

} //namespace plc::core::host
