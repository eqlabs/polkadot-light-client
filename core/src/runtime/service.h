#pragma once

#include <libp2p/log/logger.hpp>


#include "chain/spec.h"
#include "network/peer_manager.h"
#include "runner/client_runner.h"
#include "runtime/api.h"
#include "runtime/executor.h"
#include "runtime/external_interface.h"
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
            m_spec(spec), m_connection_manager(connection_manager), m_runner(runner) {
                m_module = std::make_shared<Module>();
                m_executor = std::make_shared<Executor>();
                m_api = std::make_shared<Api>(m_executor);
                m_host_api = std::make_shared<plc::core::host::Api>();
            }

    Result<void> loadGenesisRuntime();
    cppcoro::task<Result<void>> loadRuntimeForBlock(libp2p::peer::PeerId, BlockHash block);

    std::shared_ptr<Api> getRuntimeApi() {
        return m_api;
    }

    std::shared_ptr<plc::core::host::Api> getHostApi() {
        return m_host_api;
    }

private:
    Result<void> uncompressCodeIfNeeded(const ByteBuffer &in, ByteBuffer &out);
    Result<void> processRuntime(const ByteBuffer &runtime);

    std::shared_ptr<plc::core::chain::Spec> m_spec;    
    std::shared_ptr<plc::core::network::PeerManager> m_connection_manager;
    std::shared_ptr<plc::core::runner::ClientRunner> m_runner;

    std::shared_ptr<Module> m_module;
    std::shared_ptr<Executor> m_executor;
    std::shared_ptr<Api> m_api;    
    std::shared_ptr<plc::core::host::Api> m_host_api;

    std::shared_ptr<ExternalInterface> m_external_interface;
    std::shared_ptr<wasm::ModuleInstance> m_module_instance;

    libp2p::log::Logger m_log = libp2p::log::createLogger("runtime::Service", "runtime");
};

} //namespace plc::core::runtime

OUTCOME_HPP_DECLARE_ERROR(plc::core::runtime, Service::Error);
