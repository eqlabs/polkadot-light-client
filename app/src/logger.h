#pragma once

#include <string>

#include <libp2p/log/logger.hpp>

namespace plc::app {

    void prepareLogging(soralog::Level log_level, const std::string &log_file);

} // namespace plc::app
