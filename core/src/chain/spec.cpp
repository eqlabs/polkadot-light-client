#include "spec.h"

#include <charconv>

#include <boost/algorithm/hex.hpp>
#include <boost/outcome/try.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <libp2p/outcome/outcome.hpp>
#include <libp2p/peer/peer_id.hpp>

#include "../utils/hex.h"

OUTCOME_CPP_DEFINE_CATEGORY(plc::core::chain, Spec::Error, e) {
    using E = plc::core::chain::Spec::Error;
    switch (e) {
    case E::MissingEntry:
        return "A required entry is missing in the config file";
    case E::MissingPeerId:
        return "Peer id is missing in a multiaddress provided in the config file";
    case E::ParserError:
        return "Internal parser error";
    case E::NotImplemented:
        return "Known entry name, but parsing not implemented";
    }
    return "Unknown error";
}

namespace plc::core::chain {

Result<void> Spec::loadFromFile(const std::string &file_path) {
    boost::property_tree::ptree tree;
    try {
        boost::property_tree::read_json(file_path, tree);
    } catch (boost::property_tree::json_parser_error &error) {
        m_log->error("Could not load chain spec from file {}. Error at line {}: {}", error.filename(), error.line(), error.message());
        return Error::ParserError;
    }

    OUTCOME_TRY(loadFields(tree));
    OUTCOME_TRY(loadGenesis(tree));
    OUTCOME_TRY(loadBootNodes(tree));

    return libp2p::outcome::success();
}

Result<void> Spec::loadFields(const boost::property_tree::ptree &tree) {
    OUTCOME_TRY(name, ensure("name", tree.get_child_optional("name")));
    m_name = name.get<std::string>("");

    OUTCOME_TRY(id, ensure("id", tree.get_child_optional("id")));
    m_id = id.get<std::string>("");
    
    if (auto entry = tree.get_child_optional("chainType"); entry.has_value()) {
        m_chain_type = entry.value().get<std::string>("");
    } else {
        m_log->info("Missing field 'chainType' in chain spec, setting to default - live");
        m_chain_type = std::string("Live");
    }

    if (auto protocol_id_opt = tree.get_child_optional("protocolId"); protocol_id_opt.has_value()) {      
        if (auto protocol_id = protocol_id_opt.value().get<std::string>(""); protocol_id != "null") {
            m_protocol_id = std::move(protocol_id);
        }
    }
    
    if (auto telemetry_endpoints_opt = tree.get_child_optional("telemetryEndpoints");
        telemetry_endpoints_opt.has_value() && telemetry_endpoints_opt.value().get<std::string>("") != "null") {
        for (auto &[_, endpoint] : telemetry_endpoints_opt.value()) {
            if (auto it = endpoint.begin(); endpoint.size() >= 2) {
                auto &uri = it->second;
                auto &priority = (++it)->second;
                m_telemetry_endpoints.emplace_back(std::move(uri.get<std::string>("")),
                                                   std::move(priority.get<size_t>("")));
            }
        }
    }
    
    if (auto properties_opt = tree.get_child_optional("properties"); properties_opt.has_value()
            && properties_opt.value().get<std::string>("") != "null") {
        for (auto &[property_name, property_value] : properties_opt.value()) {
            m_properties.emplace(std::move(property_name), std::move(property_value.get<std::string>("")));
        }
    }
    
    if (auto consensus_engine_opt = tree.get_child_optional("consensusEngine"); consensus_engine_opt.has_value()) {
        if (auto consensus_engine = consensus_engine_opt.value().get<std::string>("");
            consensus_engine != "null") {
            m_consensus_engine.emplace(std::move(consensus_engine));
        }
    }
    
    if (auto code_substitutes_opt = tree.get_child_optional("codeSubstitutes"); code_substitutes_opt.has_value()) {
        for (const auto &[block_id, code] : code_substitutes_opt.value()) {
            OUTCOME_TRY(block_id_parsed, parseBlockId(block_id));
            m_known_code_substitutes.emplace(std::move(block_id_parsed));
        }
    }

    auto fork_blocks_opt = tree.get_child_optional("forkBlocks");
    if (fork_blocks_opt.has_value() && fork_blocks_opt.value().get<std::string>("") != "null") {
        for (auto &[_, fork_block] : fork_blocks_opt.value()) {
            OUTCOME_TRY(hash, unhexWith0xToBlockHash(fork_block.get<std::string>("")));
            m_fork_blocks.emplace(std::move(hash));
        }
    }

    auto bad_blocks_opt = tree.get_child_optional("badBlocks");
    if (bad_blocks_opt.has_value() && bad_blocks_opt.value().get<std::string>("") != "null") {
        for (auto &[_, bad_block] : bad_blocks_opt.value()) {
            OUTCOME_TRY(hash, unhexWith0xToBlockHash(bad_block.get<std::string>("")));
            m_bad_blocks.emplace(std::move(hash));
        }
    }

    return libp2p::outcome::success();
}

Result<void> Spec::loadGenesis(const boost::property_tree::ptree &tree) {
    OUTCOME_TRY(genesis, ensure("genesis", tree.get_child_optional("genesis")));
    OUTCOME_TRY(genesis_raw, ensure("genesis/raw", genesis.get_child_optional("raw")));
    boost::property_tree::ptree top_tree;

    if (auto top_tree_opt = genesis_raw.get_child_optional("top"); top_tree_opt.has_value()) {
        top_tree = top_tree_opt.value();
    } else {    
        top_tree = genesis_raw.begin()->second;
    }

    auto read_key_block = [](const auto &tree, GenesisRawData &data) -> Result<void> {
        for (const auto &[child_key, child_value] : tree) {
            OUTCOME_TRY(key_processed, unhexWith0x(child_key));
            OUTCOME_TRY(value_processed, unhexWith0x(child_value.data()));
            data.emplace_back(std::move(key_processed), std::move(value_processed));
        }
        return libp2p::outcome::success();
    };

    if (auto children_default_tree_opt = genesis_raw.get_child_optional("childrenDefault");
        children_default_tree_opt.has_value()) {
        for (const auto &[key, value] : children_default_tree_opt.value()) {
            GenesisRawData child;
            OUTCOME_TRY(read_key_block(value, child));
            OUTCOME_TRY(key_processed, unhexWith0x(key));
            m_children_default.emplace(std::move(key_processed), std::move(child));
        }
    }

    OUTCOME_TRY(read_key_block(top_tree, m_genesis));

    return libp2p::outcome::success();
}

Result<void> Spec::loadBootNodes(const boost::property_tree::ptree &tree) {
    OUTCOME_TRY(boot_nodes, ensure("bootNodes", tree.get_child_optional("bootNodes")));
    for (auto &node : boot_nodes) {
        if (auto ma_res = libp2p::multi::Multiaddress::create(node.second.data())) {
            auto &&multiaddr = ma_res.value();
            if (auto peer_id_base58 = multiaddr.getPeerId(); peer_id_base58.has_value()) {
                OUTCOME_TRY(libp2p::peer::PeerId::fromBase58(peer_id_base58.value()));
                m_boot_nodes.emplace_back(std::move(multiaddr));
            } else {
                return Error::MissingPeerId;
            }
        } else {
            m_log->warn("Unsupported multiaddress '{}'. Ignoring that boot node", node.second.data());
        }
    }
    return libp2p::outcome::success();
}

Result<BlockId> Spec::parseBlockId(std::string_view block_id_str) const {
    BlockId block_id;
    if (block_id_str.rfind("0x", 0) != std::string::npos) {
        OUTCOME_TRY(block_hash, unhexWith0xToBlockHash(block_id_str));
        block_id = block_hash;
    }
    else {
        BlockNumber block_num;
        auto res = std::from_chars(block_id_str.data(),
                                 block_id_str.data() + block_id_str.size(),
                                 block_num);
        if (res.ec != std::errc()) {
            return Error::ParserError;
        }
        block_id = block_num;
    }
    return block_id;
}

} //namespace plc::core::chain
