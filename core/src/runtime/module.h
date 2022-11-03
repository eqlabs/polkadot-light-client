#pragma once 

#include <string>

#include <libp2p/log/logger.hpp>
#include <wasm.h>

#include "utils/result.h"
#include "utils/types.h"

namespace plc::core::runtime {

class Module final {
public:
    enum class Error {
        ParsingError = 1
    };
    Result<void> parseCode(const ByteBuffer &code);

private:
    std::unique_ptr<wasm::Module> m_module;

    libp2p::log::Logger m_log = libp2p::log::createLogger("Module", "runtime");
};

} //namespace plc::core::runtime

OUTCOME_HPP_DECLARE_ERROR(plc::core::runtime, Module::Error);