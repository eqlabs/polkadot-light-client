#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include <libp2p/peer/peer_id.hpp>

#include "runner/client_runner.h"

namespace boost::asio {
    class io_context;
} // namespace boost::asio

namespace libp2p {
namespace host {
    class BasicHost;
} // namespace host

namespace protocol {
namespace kademlia {
    class Kademlia;
} // namespace kademlia
    class Identify;
} // namespace protocol

namespace event {
    class Handle;
} // namespace event
} // namespace libp2p

namespace plc::core::network {

class PeerManager final {
public:
    struct Config {
        size_t max_peers_to_connect;
    };

public:
    PeerManager(runner::ClientRunner& runner,
        const std::vector<std::string>& boot_nodes) noexcept;
    ~PeerManager();

private:
    enum class ConnectionState {
        Disconnected,
        Connecting,
        Connected,
        Disconnecting
    };

    struct PeerState {
        ConnectionState connection_state;
        size_t last_activity;
    };

private:
    void initProtocols(std::shared_ptr<boost::asio::io_context> io_context);

private:
    Config m_config;
    std::shared_ptr<libp2p::host::BasicHost> m_host;
    std::shared_ptr<libp2p::protocol::kademlia::Kademlia> m_kademlia;
    std::shared_ptr<libp2p::protocol::Identify> m_identify;
    std::unordered_map<libp2p::peer::PeerId, PeerState> m_peer_info;
    std::vector<libp2p::event::Handle> m_event_handels;
};

} // namespace plc::core::network
