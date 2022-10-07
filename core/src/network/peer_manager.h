#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include <libp2p/peer/peer_id.hpp>
#include <libp2p/network/connection_manager.hpp>

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
    class Config;
    class Kademlia;
} // namespace kademlia
    class Identify;
    class Ping;
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
        const std::vector<std::string>& boot_nodes);
    PeerManager(runner::ClientRunner& runner,
        const std::vector<libp2p::multi::Multiaddress>& peers);
    ~PeerManager();

private:
    enum class ConnectionState {
        Disconnected,
        Connected,
    };

    enum class ConnectionAction {
        None,
        Connecting,
        Disconnecting
    };

    struct PeerState {
        ConnectionState state;
        ConnectionAction action;
        size_t last_activity;
        bool is_pinging;
    };

private:
    void initProtocols(std::shared_ptr<boost::asio::io_context> io_context);
    void startAndUpdateConnections(runner::ClientRunner& runner);
    PeerState makePeerState() const;
    void onDiscoveredPeer(const libp2p::peer::PeerId& peer_id);
    void onConnectedPeer(const libp2p::peer::PeerId& peer_id);
    void connect(const libp2p::peer::PeerId& peer_id);
    void disconnect(const libp2p::peer::PeerId& peer_id);
    void updateTick(PeerState& state);
    void updateConnections();

private:
    Config m_config;
    std::shared_ptr<libp2p::host::BasicHost> m_host;
    std::unique_ptr<libp2p::protocol::kademlia::Config> m_kademlia_config;
    std::shared_ptr<libp2p::protocol::kademlia::Kademlia> m_kademlia;
    std::shared_ptr<libp2p::protocol::Identify> m_identify;
    std::shared_ptr<libp2p::protocol::Ping> m_ping;
    std::unordered_map<libp2p::peer::PeerId, PeerState> m_peers_info;
    std::vector<libp2p::event::Handle> m_event_handlers;

    size_t m_current_tick = 0;
    std::unique_ptr<runner::PeriodicTimer> m_timer;
    libp2p::log::Logger m_log = libp2p::log::createLogger("PeerManager","network");
};

} // namespace plc::core::network
