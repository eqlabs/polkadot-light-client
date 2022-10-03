#pragma once

#include <libp2p/outcome/outcome.hpp>

namespace plc::core {
    void setStop(std::function<void()> stop);
    void stop();
} // namespace plc::core
