#pragma once 

#include <string>

#include <libp2p/log/logger.hpp>
#include <wasm-binary.h>

#include "utils/result.h"
#include "utils/types.h"

namespace plc::core::runtime {

class Module final {
public:
    enum class Error {
        ParsingError = 1
    };
    Result<void> parseCode(const ByteBuffer &code);

    std::shared_ptr<wasm::Module> getWasmModule() noexcept {
        return m_module;
    }
private:
    std::shared_ptr<wasm::Module> m_module;
    libp2p::log::Logger m_log = libp2p::log::createLogger("runtime::Module", "runtime");
};

} //namespace plc::core::runtime

OUTCOME_HPP_DECLARE_ERROR(plc::core::runtime, Module::Error);
