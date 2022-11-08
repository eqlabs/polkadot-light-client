#include "api.h"

namespace plc::core::runtime {
    Result<Ptr> Api::coreVersion() {
        return m_executor->call("Core_version");
    }
} //namespace plc::core::runtime