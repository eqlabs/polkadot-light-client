#include "runtime/service.h"

#include <iostream>

#include <libp2p/host/basic_host/basic_host.hpp>
#include <zstd.h>

#include "network/light2/protocol.h"
#include "utils/hex.h"

OUTCOME_CPP_DEFINE_CATEGORY(plc::core::runtime, Service::Error, e) {
    using E = plc::core::runtime::Service::Error;
    switch (e) {
    case E::DecompresionError:
        return "Error while trying to decompress runtime";
    case E::MissingInitialRuntime:
        return "Genesis block runtime is not present in the chain spec";
    case E::ObtainingRuntimeError:
        return "Could not obtain runtime from remote node";
    }
    return "Unknown error";
}

namespace plc::core::runtime {

const std::string code_hex = std::string("0x3a636f6465"); //:code

// @see
// https://github.com/paritytech/substrate/blob/polkadot-v0.9.8/primitives/maybe-compressed-blob/src/lib.rs#L28
constexpr uint8_t zstd_prefix_size = 8;
constexpr uint8_t zstd_prefix[zstd_prefix_size] = {0x52, 0xBC, 0x53, 0x76, 0x46, 0xDB, 0x8E, 0x05};

// @see
// https://github.com/paritytech/substrate/blob/polkadot-v0.9.8/primitives/maybe-compressed-blob/src/lib.rs#L35
constexpr size_t code_blob_bomb_limit = 50 * 1024 * 1024;

Service::Service(std::shared_ptr<plc::core::chain::Spec> spec,
            std::shared_ptr<plc::core::network::PeerManager> connection_manager, 
            std::shared_ptr<plc::core::runner::ClientRunner> runner) : 
    m_spec(spec), m_connection_manager(connection_manager), m_runner(runner) {
    m_module = std::make_shared<Module>();
    m_executor = std::make_shared<Executor>();
    m_runtime_api = std::make_shared<Api>(m_executor);
    m_host_api = std::make_shared<plc::core::host::Api>();

    auto result = loadGenesisRuntime();
    if (result.has_error()) {
        m_log->error("Error while trying to load genesis runtime: {}", result.error());
    }
    //TODO: start listening for new block events
}

Result<void> Service::loadGenesisRuntime() {
    auto genesis = m_spec->getGenesis();

    for (auto [k, v]: genesis) {        
        if (auto str = hex(k); str == code_hex) {
            auto genesisRuntime = v;

            return processRuntime(genesisRuntime);
        }
    }
    return Error::MissingInitialRuntime;
}

cppcoro::task<Result<void>> Service::loadRuntimeForBlock(libp2p::peer::PeerId peer, BlockHash block) {
    //TODO: this was not tested yet
    auto host = m_connection_manager->getHost();
    auto light2 = plc::core::network::light2::Protocol(*host, *m_runner);

    plc::core::network::light2::RemoteReadRequest req = {block, {":code"}};    

    m_log->debug("Trying to get runtime from peer {}", peer.toBase58());
    auto result = co_await light2.send(std::move(req), peer);

    if (result.has_error()) {
        co_return Error::ObtainingRuntimeError;
    } else {
        auto val = result.value();

        plc::core::ByteBuffer buf(val.proof.size());
        std::copy(val.proof.begin(), val.proof.end(), buf.begin());

        //TODO: check if new runtime differs from the one we currently have and process it if yes
        co_return processRuntime(buf);
    }
}

Result<void> Service::processRuntime(const ByteBuffer &runtime) {
    ByteBuffer res;
    OUTCOME_TRY(uncompressCodeIfNeeded(runtime, res));
    OUTCOME_TRY(m_module->parseCode(res));
    
    m_external_interface = std::make_shared<ExternalInterface>(m_host_api);
    m_module_instance = std::make_shared<wasm::ModuleInstance>(*m_module->getWasmModule(), m_external_interface.get());    

    auto heap_base_res = m_module_instance->getExport("__heap_base"); 
    auto heap_base = 0;
    if (heap_base_res.size() > 0) {
        heap_base = heap_base_res[0].geti32();
        m_log->debug("Setting heap base to {}", heap_base);
    } else {
        m_log->warn("Could not get heap base from the runtime, setting it to 0");
    }
    //init memory and modules using it
    m_external_interface->initMemory(heap_base);
    m_executor->init(m_module_instance, m_external_interface->getMemory());
    m_host_api->init(m_external_interface->getMemory());
    
    return libp2p::outcome::success();
}

Result<void> Service::uncompressCodeIfNeeded(const ByteBuffer &in, ByteBuffer &out) {
    if (in.size() > zstd_prefix_size
        && std::equal(in.begin(),
                      in.begin() + zstd_prefix_size,
                      std::begin(zstd_prefix),
                      std::end(zstd_prefix))) {
                        
        m_log->debug("Decompressing runtime code");
        auto check_size = ZSTD_getFrameContentSize(in.data() + zstd_prefix_size,
                                                 in.size() - zstd_prefix_size);
        if (check_size == ZSTD_CONTENTSIZE_ERROR) {
            m_log->error("Decompression runtime code error");
            return Error::DecompresionError;
        }
        out.resize(code_blob_bomb_limit);
        auto size = ZSTD_decompress(out.data(),
                                    code_blob_bomb_limit,
                                    in.data() + zstd_prefix_size,
                                    in.size() - zstd_prefix_size);
        out.resize(size);
    } else {
        m_log->debug("Runtime code is not compressed");
        out = in;
    }
    return libp2p::outcome::success();
}

void Service::stop() noexcept {
    //TODO: when service will listen for new block events stop it here
}

} //namespace plc::core::runtime
