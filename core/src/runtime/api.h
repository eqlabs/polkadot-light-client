#pragma once

#include "runtime/executor.h"

namespace plc::core::runtime {

class Api final {
public:
    Api(std::shared_ptr<Executor> executor) : m_executor(executor) {}
    Result<Ptr> coreVersion();

private:
    std::shared_ptr<Executor> m_executor;
};
    
} //namespace plc::core::runtime
