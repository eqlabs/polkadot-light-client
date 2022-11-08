#pragma once

#include <libp2p/log/logger.hpp>
#include <literal.h>

namespace plc::core::host {

class Api final {
public:
    wasm::Literals ext_logging_max_level_version_1();

    libp2p::log::Logger m_log = libp2p::log::createLogger("Module", "host");
};

} //namespace plc::core::host
