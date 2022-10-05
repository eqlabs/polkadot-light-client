#pragma once

#include <deque>
#include <libp2p/outcome/outcome.hpp>
#include <libp2p/log/logger.hpp>
#include "utils/move_on_copy.h"

namespace plc::core {
    class StopHandler;
    class Stoppable {
    public:
        virtual void stop() noexcept = 0;
    };

    class StopHandler {
    public:
        void add(std::weak_ptr<Stoppable> stoppable);
        void stop();
    private:
        std::deque<std::weak_ptr<Stoppable>> m_stoppables;
        libp2p::log::Logger m_log = libp2p::log::createLogger("StopHandler","utils");
    };

} // namespace plc::core


