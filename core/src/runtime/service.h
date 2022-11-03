#pragma once

#include <libp2p/log/logger.hpp>

#include "chain/spec.h"
#include "network/peer_manager.h"
#include "runner/client_runner.h"
#include "runtime/module.h"

namespace plc::core::runtime {

class Service final {
public:
    enum class Error {
        DecompresionError = 1,
        ObtainingRuntimeError,
        MissingInitialRuntime,
    };

    Service(std::shared_ptr<plc::core::chain::Spec> spec,
            std::shared_ptr<plc::core::network::PeerManager> connection_manager, 
            std::shared_ptr<plc::core::runner::ClientRunner> runner) : 
            m_spec(spec), m_connection_manager(connection_manager), m_runner(runner) {}

    Result<void> loadInitialRuntime();
    cppcoro::task<Result<void>> loadRuntimeForBlock(libp2p::peer::PeerId, BlockHash block);

private:
    Result<void> uncompressCodeIfNeeded(const ByteBuffer &in, ByteBuffer &out);

    std::shared_ptr<plc::core::chain::Spec> m_spec;    
    std::shared_ptr<plc::core::network::PeerManager> m_connection_manager;
    std::shared_ptr<plc::core::runner::ClientRunner> m_runner;

    Module m_module;

    libp2p::log::Logger m_log = libp2p::log::createLogger("Module", "runtime");
};

} //namespace plc::core::runtime

OUTCOME_HPP_DECLARE_ERROR(plc::core::runtime, Service::Error);