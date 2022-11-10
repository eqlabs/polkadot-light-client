#pragma once

#include <libp2p/log/logger.hpp>


namespace plc::core {
    class WsLogger
    {
        public:
            static libp2p::log::Logger getLogger();
        private:
            static libp2p::log::Logger m_log;
    };
}