#include "network/peer_manager.h"

#include <chrono>

#include <boost/asio/io_context.hpp>

#include <libp2p/basic/scheduler/scheduler_impl.hpp>
#include <libp2p/basic/scheduler/asio_scheduler_backend.hpp>
#include <libp2p/crypto/aes_ctr/aes_ctr_impl.hpp>
#include <libp2p/crypto/crypto_provider/crypto_provider_impl.hpp>
#include <libp2p/crypto/ecdsa_provider/ecdsa_provider_impl.hpp>
#include <libp2p/crypto/ed25519_provider/ed25519_provider_impl.hpp>
#include <libp2p/crypto/hmac_provider/hmac_provider_impl.hpp>
#include <libp2p/crypto/key_marshaller/key_marshaller_impl.hpp>
#include <libp2p/crypto/key_validator/key_validator_impl.hpp>
#include <libp2p/crypto/random_generator/boost_generator.hpp>
#include <libp2p/crypto/rsa_provider/rsa_provider_impl.hpp>
#include <libp2p/crypto/secp256k1_provider/secp256k1_provider_impl.hpp>
#include <libp2p/host/basic_host/basic_host.hpp>
#include <libp2p/network/impl/dialer_impl.hpp>
#include <libp2p/network/impl/dnsaddr_resolver_impl.hpp>
#include <libp2p/network/impl/connection_manager_impl.hpp>
#include <libp2p/network/cares/cares.hpp>
#include <libp2p/network/impl/listener_manager_impl.hpp>
#include <libp2p/network/impl/router_impl.hpp>
#include <libp2p/network/impl/transport_manager_impl.hpp>
#include <libp2p/network/impl/network_impl.hpp>
#include <libp2p/muxer/yamux/yamux.hpp>
#include <libp2p/peer/address_repository/inmem_address_repository.hpp>
#include <libp2p/peer/key_repository/inmem_key_repository.hpp>
#include <libp2p/peer/protocol_repository/inmem_protocol_repository.hpp>
#include <libp2p/peer/impl/identity_manager_impl.hpp>
#include <libp2p/peer/impl/peer_repository_impl.hpp>
#include <libp2p/protocol_muxer/multiselect.hpp>
#include <libp2p/protocol/kademlia/impl/content_routing_table_impl.hpp>
#include <libp2p/protocol/kademlia/impl/kademlia_impl.hpp>
#include <libp2p/protocol/kademlia/impl/peer_routing_table_impl.hpp>
#include <libp2p/protocol/kademlia/impl/storage_impl.hpp>
#include <libp2p/protocol/kademlia/impl/storage_backend_default.hpp>
#include <libp2p/protocol/kademlia/impl/validator_default.hpp>
#include <libp2p/protocol/identify/identify.hpp>
#include <libp2p/protocol/ping/ping.hpp>
#include <libp2p/security/noise.hpp>
#include <libp2p/transport/impl/upgrader_impl.hpp>
#include <libp2p/transport/tcp/tcp_transport.hpp>

#include "utils/callback_to_coro.h"
#include "utils/propagate.h"
#include "utils/result.h"

namespace plc::core::network {

namespace {

std::optional<libp2p::peer::PeerInfo> createPeerInfo(libp2p::multi::Multiaddress multiaddr) {
    if (auto peer_id = libp2p::peer::PeerId::fromBase58(multiaddr.getPeerId().value());
        peer_id.has_value()) {
        return libp2p::peer::PeerInfo{
            peer_id.value(),
            {multiaddr}
        };
    }
    return std::nullopt;
}

std::optional<libp2p::peer::PeerInfo> parsePeerInfo(std::string peer) {
    // TODO: handle parse error
    if (auto multiaddr = libp2p::multi::Multiaddress::create(peer); multiaddr.has_value()) {
        return createPeerInfo(multiaddr.value());
    }

    return std::nullopt;
}

} // namespace

PeerManager::PeerManager(runner::ClientRunner& runner,
    const std::vector<std::string>& peers) {
    initProtocols(runner.getService());

    m_kademlia->addPeer(m_host->getPeerInfo(), true);
    for (const auto& peer: peers) {
        if (const auto peerInfo = parsePeerInfo(peer)) {
            m_kademlia->addPeer(*peerInfo, true);
        }
    }
    startAndUpdateConnections(runner);
}

PeerManager::PeerManager(runner::ClientRunner &runner,
                         const std::vector<libp2p::multi::Multiaddress> &peers) {
    initProtocols(runner.getService());

    m_kademlia->addPeer(m_host->getPeerInfo(), true);
    for (const auto& peer: peers) {
        if (const auto peerInfo = createPeerInfo(peer)) {
            m_kademlia->addPeer(*peerInfo, true);
        }
    }
    startAndUpdateConnections(runner);
}

// TODO: gracefully stop all the connections
PeerManager::~PeerManager() = default;

static const libp2p::network::c_ares::Ares cares = {};

void PeerManager::initProtocols(std::shared_ptr<boost::asio::io_context> io_context) {
    auto multiselect = std::make_shared<libp2p::protocol_muxer::multiselect::Multiselect>();

    auto scheduler = std::make_shared<libp2p::basic::SchedulerImpl>(
        std::make_shared<libp2p::basic::AsioSchedulerBackend>(io_context),
        libp2p::basic::Scheduler::Config {}
    );

    auto bus = std::make_shared<libp2p::event::Bus>();
    auto connection_manager = std::make_shared<libp2p::network::ConnectionManagerImpl>(bus);
    auto random_generator = std::make_shared<libp2p::crypto::random::BoostRandomGenerator>();
    auto ed25519_provider =
        std::make_shared<libp2p::crypto::ed25519::Ed25519ProviderImpl>();
    auto rsa_provider = std::make_shared<libp2p::crypto::rsa::RsaProviderImpl>();
    auto ecdsa_provider = std::make_shared<libp2p::crypto::ecdsa::EcdsaProviderImpl>();
    auto secp256k1_provider =
        std::make_shared<libp2p::crypto::secp256k1::Secp256k1ProviderImpl>();
    auto hmac_provider = std::make_shared<libp2p::crypto::hmac::HmacProviderImpl>();
    auto crypto_provider =
        std::make_shared<libp2p::crypto::CryptoProviderImpl>(
            random_generator, ed25519_provider, rsa_provider, ecdsa_provider,
            secp256k1_provider, hmac_provider);
    auto validator =
        std::make_shared<libp2p::crypto::validator::KeyValidatorImpl>(crypto_provider);
    auto keypair =
        crypto_provider->generateKeys(libp2p::crypto::Key::Type::Ed25519,
        libp2p::crypto::common::RSAKeyType::RSA2048).value();
    auto key_marshaller = std::make_shared<libp2p::crypto::marshaller::KeyMarshallerImpl>(validator);

    std::vector<std::shared_ptr<libp2p::security::SecurityAdaptor>> security_adaptors;
    security_adaptors.push_back(std::make_shared<libp2p::security::Noise>(keypair, crypto_provider, key_marshaller));

    std::vector<std::shared_ptr<libp2p::muxer::MuxerAdaptor>> muxerAdaptors;
    muxerAdaptors.push_back(std::make_shared<libp2p::muxer::Yamux>(
        libp2p::muxer::MuxedConnectionConfig{},
        scheduler, connection_manager
    ));

    auto upgrader = std::make_shared<libp2p::transport::UpgraderImpl>(
        multiselect,
        security_adaptors,
        muxerAdaptors);

    auto transport_manager = std::make_shared<libp2p::network::TransportManagerImpl>(
        std::vector<std::shared_ptr<libp2p::transport::TransportAdaptor>>{
            std::make_shared<libp2p::transport::TcpTransport>(
                io_context, upgrader)});
    auto router = std::make_shared<libp2p::network::RouterImpl>();
    auto listener_manager = std::make_shared<libp2p::network::ListenerManagerImpl>(multiselect, router,
        transport_manager, connection_manager);
    auto dialer = std::make_shared<libp2p::network::DialerImpl>(multiselect, transport_manager,
        connection_manager, listener_manager, scheduler);

    auto identity_manager = std::make_shared<libp2p::peer::IdentityManagerImpl>(keypair, key_marshaller);
    auto network = std::make_unique<libp2p::network::NetworkImpl>(listener_manager, dialer, connection_manager);
    auto dns_address_resolver = std::make_shared<libp2p::network::DnsaddrResolverImpl>(io_context, cares);
    auto peer_address_repository = std::make_shared<libp2p::peer::InmemAddressRepository>(dns_address_resolver);
    auto key_repository = std::make_shared<libp2p::peer::InmemKeyRepository>();
    auto protocol_repository = std::make_shared<libp2p::peer::InmemProtocolRepository>();
    auto peer_repository = std::make_unique<libp2p::peer::PeerRepositoryImpl>(peer_address_repository, key_repository,
        protocol_repository);
    m_host = std::make_shared<libp2p::host::BasicHost>(identity_manager, std::move(network), std::move(peer_repository), bus, transport_manager);
    m_log->info("Local host peer id {}", m_host->getId().toHex());

    m_kademlia_config = std::make_unique<libp2p::protocol::kademlia::Config>();
    // TODO: get protocol id from chain spec
    m_kademlia_config->protocolId = "/dot/kad";
    auto kademlia_storage_backed = std::make_shared<libp2p::protocol::kademlia::StorageBackendDefault>();
    auto kademlia_storage = std::make_shared<libp2p::protocol::kademlia::StorageImpl>(*m_kademlia_config,
        kademlia_storage_backed, scheduler);
    auto kademlia_content_routing_table = std::make_shared<libp2p::protocol::kademlia::ContentRoutingTableImpl>(
        *m_kademlia_config, *scheduler, bus);
    auto kademlia_peer_routing_table = std::make_shared<libp2p::protocol::kademlia::PeerRoutingTableImpl>(
        *m_kademlia_config, identity_manager, bus);
    auto kademlia_validator = std::make_shared<libp2p::protocol::kademlia::ValidatorDefault>();
    m_kademlia = std::make_shared<libp2p::protocol::kademlia::KademliaImpl>(*m_kademlia_config, m_host,
        kademlia_storage, kademlia_content_routing_table, kademlia_peer_routing_table, kademlia_validator,
        scheduler, bus, random_generator);

    auto identify_msg_processor = std::make_shared<libp2p::protocol::IdentifyMessageProcessor>(
        *m_host, *connection_manager, *identity_manager, key_marshaller);
    m_identify = std::make_shared<libp2p::protocol::Identify>(*m_host, identify_msg_processor, *bus);
    m_ping = std::make_shared<libp2p::protocol::Ping>(*m_host, *bus, *io_context, random_generator);

    m_host->setProtocolHandler(m_ping->getProtocolId(), [logger = m_log, ping = std::weak_ptr{m_ping}](auto&& stream) {
        if (auto ping_ptr = ping.lock()) {
            if (auto peer_id = stream->remotePeerId()) {
                logger->debug("Handled {} protocol stream from: {}", ping_ptr->getProtocolId(), peer_id.value().toHex());
                ping_ptr->handle(std::forward<decltype(stream)>(stream));
            }
        }
    });

    m_event_handlers.push_back(
        m_host->getBus()
            .getChannel<libp2p::event::protocol::kademlia::PeerAddedChannel>()
            .subscribe([this](const libp2p::peer::PeerId& peer_id) {
                onDiscoveredPeer(peer_id);
            })
    );

    m_identify->onIdentifyReceived(
        [this](const libp2p::peer::PeerId &peer_id) {
            onConnectedPeer(peer_id);
    });
}

void PeerManager::startAndUpdateConnections(runner::ClientRunner &runner) {
    m_identify->start();
    m_kademlia->start();
    m_timer = std::make_unique<runner::PeriodicTimer>(
        runner.makePeriodicTimer(std::chrono::milliseconds(200), [this]() {
        updateConnections();
    }));
    updateConnections();
}

PeerManager::PeerState PeerManager::makePeerState() const {
    return {ConnectionState::Disconnected, ConnectionAction::None, m_current_tick, false};
}

void PeerManager::onDiscoveredPeer(const libp2p::peer::PeerId& peer_id) {
    if (m_host->getId() == peer_id) {
        return;
    }

    if (m_peers_info.contains(peer_id)) {
        return;
    }

    m_peers_info.emplace(peer_id, makePeerState());

    m_log->debug("New peer discovered: {}", peer_id.toHex());
}

void PeerManager::onConnectedPeer(const libp2p::peer::PeerId& peer_id) {
    if (m_host->getId() == peer_id) {
        return;
    }

    auto it = m_peers_info.find(peer_id);
    if (it == m_peers_info.end()) {
        it = m_peers_info.emplace(peer_id, makePeerState()).first;
    }
    auto& peer_state = it->second;

    if (peer_state.state == ConnectionState::Connected) {
        return;
    }

    m_log->info("Connected to peer_id {}", peer_id.toHex());

    if (auto connection = m_host->getNetwork().getConnectionManager()
        .getBestConnectionForPeer(peer_id)) {
        assert(connection->isInitiator());
        peer_state.state = ConnectionState::Connected;
        if (peer_state.action == ConnectionAction::Connecting) {
            peer_state.action = ConnectionAction::None;
        }
        if (!peer_state.is_pinging) {
            m_ping->startPinging(
                connection,
                [this, peer_id, connection](
                    Result<std::shared_ptr<
                        libp2p::protocol::PingClientSession>> session_res) {
                    if (session_res.has_error()) {
                        this->m_log->error("Pinging stopped because of error: {}, {}", peer_id.toHex(), session_res.error().message());
                        if (auto it = m_peers_info.find(peer_id); it != m_peers_info.end()) {
                            it->second.is_pinging = false;
                            updateTick(it->second);
                        }
                        disconnect(peer_id);
                    } else {
                        if (auto it = m_peers_info.find(peer_id); it != m_peers_info.end()) {
                            it->second.is_pinging = true;
                            m_log->trace("Pinging {}", peer_id.toHex());
                            updateTick(it->second);
                        } else {
                            m_log->error("Received ping from unknown peer: {}", peer_id.toHex());
                        }
                    }
                });
        }
    } else {
        peer_state.state = ConnectionState::Disconnected;
        if (peer_state.action == ConnectionAction::Disconnecting) {
            peer_state.action = ConnectionAction::None;
        }
    }

    auto addresses_res =
        m_host->getPeerRepository().getAddressRepository().getAddresses(peer_id);
    if (addresses_res.has_value()) {
      auto& addresses = addresses_res.value();
      libp2p::peer::PeerInfo peer_info{.id = peer_id, .addresses = std::move(addresses)};
      m_kademlia->addPeer(peer_info, false);
    }
}

void PeerManager::updateTick(PeerState& state) {
    state.last_activity = m_current_tick;
}

void PeerManager::disconnect(const libp2p::peer::PeerId& peer_id) {
    if (auto it = m_peers_info.find(peer_id); it != m_peers_info.end()) {
        it->second.action = ConnectionAction::None;
        it->second.state = ConnectionState::Disconnected;
    }
    if (peer_id != m_host->getId()) {
        m_host->disconnect(peer_id);
    }
}

void PeerManager::connect(const libp2p::peer::PeerId& peer_id) {
    if (peer_id == m_host->getId()) {
        return;
    }

    auto peer_info = m_host->getPeerRepository().getPeerInfo(peer_id);
    if (peer_info.addresses.empty()) {
        m_log->error("No found addresses for peer_id: {}", peer_id.toHex());
        return;
    }

    auto connectedness = m_host->connectedness(peer_info);
    if (connectedness == libp2p::Host::Connectedness::CAN_NOT_CONNECT) {
        m_log->error("Cannot connect to peer_id: {}", peer_id.toHex());
        return;
    }

    std::stringstream buffer;
    buffer << "Try to connect to peer_id " << peer_info.id.toHex();
    for (auto addr : peer_info.addresses) {
        buffer << "  address: " << addr.getStringAddress();
    }
    buffer << std::endl;
    m_log->info(buffer.str());

    m_host->connect(
        peer_info,
        [this, peer_id](auto&& res) mutable {
            if (!res.has_value()) {
                m_log->error("Connecting to peer_id {} failed {}", peer_id.toHex(), res.error().message());
                return;
            }

            auto& connection = res.value();
            auto remote_peer_id_res = connection->remotePeer();
            if (!remote_peer_id_res.has_value()) {
                m_log->error("Connected, but not identified yet, expecting peer_id {}", peer_id.toHex());
                return;
            }

            auto& remote_peer_id = remote_peer_id_res.value();
            if (remote_peer_id == peer_id) {
                onConnectedPeer(peer_id);
            }
        },
        // TODO: make this delay configurable
        std::chrono::seconds{15});
}

void PeerManager::updateConnections() {
    ++m_current_tick;

    // TODO: extract to config
    static constexpr size_t min_connections = 10;
    static constexpr size_t max_connections = 20;
    // TODO: this can be optimized
    size_t connected_count = 0u;
    for (const auto& [peer, state]: m_peers_info) {
        if (state.state == ConnectionState::Connected ||
            state.action == ConnectionAction::Connecting) {
            ++connected_count;
        }
    }

    if (connected_count < min_connections) {
        for (auto& [peer, state]: m_peers_info) {
            if (state.state == ConnectionState::Disconnected &&
                state.action == ConnectionAction::None) {
                state.action = ConnectionAction::Connecting;
                updateTick(state);
                connect(peer);
                ++connected_count;
                if (connected_count == max_connections) {
                    break;
                }
            }
        }
    } else if (connected_count > max_connections) {
        for (auto& [peer, state]: m_peers_info) {
            if (state.state == ConnectionState::Connected) {
                state.action = ConnectionAction::Disconnecting;
                updateTick(state);
                disconnect(peer);
                --connected_count;
                if (connected_count == max_connections) {
                    break;
                }
            }
        }
    }
}

} // namespace plc::core::network
