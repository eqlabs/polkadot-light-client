#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include <libp2p/peer/peer_id.hpp>
#include <libp2p/network/connection_manager.hpp>

#include <libp2p/protocol/kademlia/impl/content_routing_table_impl.hpp>
#include <libp2p/protocol/kademlia/impl/kademlia_impl.hpp>
#include <libp2p/protocol/kademlia/impl/peer_routing_table_impl.hpp>
#include <libp2p/protocol/kademlia/impl/storage_impl.hpp>
#include <libp2p/protocol/kademlia/impl/storage_backend_default.hpp>
#include <libp2p/protocol/kademlia/impl/validator_default.hpp>

#include "runner/client_runner.h"

namespace boost::asio {
    class io_context;
} // namespace boost::asio

namespace libp2p {
namespace host {
    class BasicHost;
} // namespace host

namespace protocol {
    class Identify;
    class Ping;
} // namespace protocol

namespace event {
    class Handle;
} // namespace event
} // namespace libp2p

namespace plc::core::network {

namespace grandpa {
class Protocol;
} // namespace grandpa

namespace light2 {
class Protocol;
} // namespace light2

class PeerManager : public Stoppable {
public:
    struct Config {
        size_t max_peers_to_connect;
    };

public:
    PeerManager(std::shared_ptr<runner::ClientRunner> runner,
        const std::vector<std::string>& boot_nodes,
        std::shared_ptr<plc::core::StopHandler> stop_handler);
    PeerManager(std::shared_ptr<runner::ClientRunner> runner,
        const std::vector<libp2p::multi::Multiaddress>& peers,
        std::shared_ptr<plc::core::StopHandler> stop_handler);
    ~PeerManager();
    void stop() noexcept override;
    std::vector<libp2p::peer::PeerId> getPeersInfo() const;
    std::shared_ptr<libp2p::host::BasicHost> getHost() const;

protected:
    PeerManager() = default;

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
    void initProtocols();
    void startAndUpdateConnections();
    PeerState makePeerState() const;
    void onDiscoveredPeer(const libp2p::peer::PeerId& peer_id);
    void onConnectedPeer(const libp2p::peer::PeerId& peer_id);
    void connect(const libp2p::peer::PeerId& peer_id);
    void disconnect(const libp2p::peer::PeerId& peer_id);
    void updateTick(PeerState& state);
    size_t getConnectionsCount() const noexcept;
    void updateConnections();

private:
    std::shared_ptr<runner::ClientRunner> m_runner;
    // -Wunused-private-field
    // Config m_config;
    std::shared_ptr<libp2p::host::BasicHost> m_host;
    std::unique_ptr<libp2p::protocol::kademlia::Config> m_kademlia_config;
    std::shared_ptr<libp2p::protocol::kademlia::Kademlia> m_kademlia;
    std::shared_ptr<libp2p::protocol::Identify> m_identify;
    std::shared_ptr<libp2p::protocol::Ping> m_ping;
    std::shared_ptr<grandpa::Protocol> m_grandpa;
    std::shared_ptr<light2::Protocol> m_light;
    std::unordered_map<libp2p::peer::PeerId, PeerState> m_peers_info;
    std::vector<libp2p::event::Handle> m_event_handlers;

    size_t m_current_tick = 0;
    std::unique_ptr<runner::PeriodicTimer> m_timer;
    libp2p::log::Logger m_log = libp2p::log::createLogger("PeerManager","network");
    std::shared_ptr<StopHandler> m_stop_handler;
};

} // namespace plc::core::network
