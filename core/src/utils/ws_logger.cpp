#include "ws_logger.h"

namespace plc::core {

    libp2p::log::Logger PackIoLogger::m_log = {};

    libp2p::log::Logger PackIoLogger::getLogger() {
        if (m_log == nullptr) {
            m_log = libp2p::log::createLogger("PackIo JSON-RPC","network");
        }
        return m_log;
    }

}