#include "ws_logger.h"

namespace plc::core {

    libp2p::log::Logger WsLogger::m_log = {};

    libp2p::log::Logger WsLogger::getLogger() {
        if (m_log == nullptr) {
            m_log = libp2p::log::createLogger("WsLogger","network");
        }
        return m_log;
    }

}