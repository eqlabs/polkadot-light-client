#include "network/peer_manager.h"

#include <iostream>

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

namespace plc::core::network {

namespace {

std::optional<libp2p::peer::PeerInfo> parsePeerInfo(std::string peer) noexcept {
    // TODO: handle parse error
    if (auto multiaddr = libp2p::multi::Multiaddress::create(peer); multiaddr.has_value()) {
        if (auto peer_id = libp2p::peer::PeerId::fromBase58(multiaddr.value().getPeerId().value());
            peer_id.has_value()) {
            return libp2p::peer::PeerInfo{
                peer_id.value(),
                {multiaddr.value()}
            };
        }
    }

    return std::nullopt;
}

} // namespace

PeerManager::PeerManager(runner::ClientRunner& runner,
    const std::vector<std::string>& peers) noexcept {
    initProtocols(runner.getService());
    for (const auto& peer: peers) {
        if (const auto peerInfo = parsePeerInfo(peer)) {
            m_kademlia->addPeer(*peerInfo, true);
        }
    }
    m_identify->start();
    m_kademlia->start();
}

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
    m_kademlia_config = std::make_unique<libp2p::protocol::kademlia::Config>();
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
    m_kademlia->addPeer(m_host->getPeerInfo(), true);

    auto identify_msg_processor = std::make_shared<libp2p::protocol::IdentifyMessageProcessor>(
        *m_host, *connection_manager, *identity_manager, key_marshaller);
    m_identify = std::make_shared<libp2p::protocol::Identify>(*m_host, identify_msg_processor, *bus);
    m_ping = std::make_shared<libp2p::protocol::Ping>(*m_host, *bus, *io_context, random_generator);
    m_host->setProtocolHandler(m_ping->getProtocolId(), [ping = std::weak_ptr{m_ping}](auto&& stream) {
        if (auto ping_ptr = ping.lock()) {
            if (auto peer_id = stream->remotePeerId()) {
                std::cout << "Handled " << ping_ptr->getProtocolId() << " protocol stream from: " <<
                    peer_id.value().toBase58() << std::endl;
                ping_ptr->handle(std::forward<decltype(stream)>(stream));
            }
        }
    });
}

} // namespace plc::core::network
