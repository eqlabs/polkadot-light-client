#pragma once

#include <map>
#include <set>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include <boost/property_tree/ptree.hpp>
#include <boost/variant.hpp>

#include <libp2p/multi/multiaddress.hpp>

#include "../utils/result.h"
#include "../utils/types.h"

namespace boost {
    template<typename T>
    class optional;
}

namespace plc::core::chain {

using GenesisRawData = std::vector<std::pair<ByteArray, ByteArray>>;
using ChildrenDefaultRawData = std::map<ByteArray, GenesisRawData>;
using BlockId = boost::variant<BlockHash, BlockNumber>;

class Spec final {
public:

    enum class Error {
      MissingEntry = 1,
      MissingPeerId,
      ParserError,
      NotImplemented
    };

    Result<void> loadFromFile(const std::string &file_path);

    const std::string &getName() const {
        return m_name;
    }

    const std::string &getId() const {
        return m_id;
    }

    const std::string &getChainType() const {
        return m_chain_type;
    }

    const std::vector<libp2p::multi::Multiaddress> &getBootNodes() const {
        return m_boot_nodes;
    }

    const std::unordered_set<BlockId> &getKnownCodeSubstitutes() const {
        return m_known_code_substitutes;
    }

    const std::string &getProtocolId() const {
        return m_protocol_id;
    }

    const std::map<std::string, std::string> &getProperties() const {
        return m_properties;
    }

    const std::set<BlockHash> &getForkBlocks() const {
        return m_fork_blocks;
    }

    const std::set<BlockHash> &getBadBlocks() const {
        return m_bad_blocks;
    }

    std::optional<std::string> getConsensusEngine() const {
        return m_consensus_engine;
    }

    const GenesisRawData &getGenesis() const {
        return m_genesis;
    }

    const ChildrenDefaultRawData &getChildrenDefault() const {
        return m_children_default;
    }

    const std::vector<std::pair<std::string, size_t> > &getTelemetryEndpoints() const {
        return m_telemetry_endpoints;
    }

private:

    Result<void> loadFields(const boost::property_tree::ptree &tree);
    Result<void> loadGenesis(const boost::property_tree::ptree &tree);
    Result<void> loadBootNodes(const boost::property_tree::ptree &tree);

    template <typename T>
    Result<std::decay_t<T>> ensure(std::string_view entry_name,
                                            boost::optional<T> opt_entry) const {
      if (!opt_entry) {
        //log error        
        return Error::MissingEntry;
      }
      return opt_entry.value();
    }
    Result<BlockId> parseBlockId(const std::string_view &block_id_str) const;    

    std::string m_name;
    std::string m_id;
    std::string m_chain_type;
    std::vector<libp2p::multi::Multiaddress> m_boot_nodes;
    std::vector<std::pair<std::string, size_t>> m_telemetry_endpoints;
    std::string m_protocol_id{"sup"};
    std::map<std::string, std::string> m_properties;
    std::set<BlockHash> m_fork_blocks;
    std::set<BlockHash> m_bad_blocks;
    std::optional<std::string> m_consensus_engine;
    GenesisRawData m_genesis;
    ChildrenDefaultRawData m_children_default;
    std::unordered_set<BlockId> m_known_code_substitutes;
};

}

OUTCOME_HPP_DECLARE_ERROR(plc::core::chain, Spec::Error);
