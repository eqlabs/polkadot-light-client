#pragma once

#include "gmock/gmock.h"

#include "network/peer_manager.h"

namespace plc::core::ut::mocks {

class PeerManagerMock : public plc::core::network::PeerManager {
public:
    PeerManagerMock() : PeerManager() {}
};

} //namespace plc::core::ut::mocks
