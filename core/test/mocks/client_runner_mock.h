#pragma once

#include "gmock/gmock.h"

#include "runner/client_runner.h"

namespace plc::core::ut::mocks {

class ClientRunnerMock : public plc::core::runner::ClientRunner {
public:
    ClientRunnerMock() : ClientRunner() {}
};

} //namespace plc::core::ut::mocks
