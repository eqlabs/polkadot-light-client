#include "runtime/module.h"


#include <wasm-features.h>
#include <shell-interface.h>

#include "utils/hex.h"
#include "utils/string_conversion.h"

OUTCOME_CPP_DEFINE_CATEGORY(plc::core::runtime, Module::Error, e) {
    using E = plc::core::runtime::Module::Error;
    switch (e) {
    case E::ParsingError:
        return "Error while trying to parse the runtime code";
    }
    return "Unknown error";
}

namespace plc::core::runtime {

Result<void> Module::parseCode(const ByteBuffer &code) {
    std::string s;
    s.resize(code.size());
    std::memcpy(s.data(), code.data(), code.size());

    m_module = std::make_shared<wasm::Module>();
    wasm::WasmBinaryBuilder parser(
        *m_module,
        reinterpret_cast<std::vector<char> const&>(code));

    try {
        m_log->debug("Parsing runtime code");
        parser.read();
    } catch (wasm::ParseException &e) {
        std::ostringstream msg;
        e.dump(msg);
        m_log->error("Parsing runtime code error {}", msg.str());
        return Error::ParsingError;
    }
    
    return libp2p::outcome::success();
}

} //namespace plc::core::runtime
