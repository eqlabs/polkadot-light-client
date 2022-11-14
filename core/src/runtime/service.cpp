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
    auto host = m_connection_manager->getHost();
    auto light2 = plc::core::network::light2::Protocol(*host, *m_runner);

    //Only for testing purposes, but seems obtaining the runtime from full node doesn't work. Yet...
    auto block_hash = plc::core::unhexWith0xToBlockHash("0x1611aaf014ea221866a309dda02dd97633782d6d6fe0925eaaef9952105da89b");
    //{0x16, 0x11, 0xaa,0xf0,0x14,0xea,0x22,0x18,0x66,0xa3,0x09,0xdd,0xa0,0x2d,0xd9,0x76,0x33,0x78,0x2d,0x6d,0x6f,0xe0,0x92,0x5e,0xaa,0xef,0x99,0x52,0x10,0x5d,0xa8,0x9b};

    plc::core::network::light2::RemoteReadRequest req = {block_hash.value(), {":code"}};    

    m_log->debug("Trying to get runtime from peer {}", peer.toBase58());
    auto result = co_await light2.send(std::move(req), peer);

    if (result.has_error()) {
        co_return Error::ObtainingRuntimeError;
    }
    else {
        auto val = result.value();

        plc::core::ByteBuffer buf;
        for (auto i = 0; i < val.proof.size(); ++i) {
            buf.push_back(val.proof[i]);
        }

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
    int heap_base = 0;
    if (heap_base_res.size() > 0) {
        heap_base = heap_base_res[0].geti32();        
    }
    m_external_interface->initMemory(heap_base);
    m_executor->init(m_module_instance, m_external_interface->getMemory());

    //trying to launch core_version api method
    auto coreVersionResult = m_api->coreVersion();

    if (coreVersionResult) {
        auto version = coreVersionResult.value();
        m_log->info("Got core_version: {}, {}", version.m_spec_name, version.m_impl_name);
    }       
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

} //namespace plc::core::runtime
