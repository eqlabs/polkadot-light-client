#include "host/api.h"

namespace plc::core::host {

uint32_t Api::ext_logging_max_level_version_1() {    
    switch (m_log->level()) {
        case soralog::Level::OFF:
        case soralog::Level::CRITICAL:
        case soralog::Level::ERROR:
            return 0;
        case soralog::Level::WARN:
            return 1;
        case soralog::Level::INFO:
            return 2;
        case soralog::Level::VERBOSE:
            return 3;
        case soralog::Level::DEBUG:
            return 4;
        case soralog::Level::TRACE:
            return 5;
    }    
    return 0;
}

} //namespace plc::core::host
