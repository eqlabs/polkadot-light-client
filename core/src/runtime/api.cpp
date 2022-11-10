#include "api.h"

namespace plc::core::runtime {
    Result<Version> Api::coreVersion() {
        return m_executor->call<Version>("Core_version");
    }
} //namespace plc::core::runtime
