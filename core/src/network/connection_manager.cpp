#include "network/connection_manager.h"

#include <iostream>

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
#include <libp2p/network/impl/dialer_impl.hpp>
#include <libp2p/network/impl/connection_manager_impl.hpp>
#include <libp2p/network/impl/listener_manager_impl.hpp>
#include <libp2p/network/impl/router_impl.hpp>
#include <libp2p/network/impl/transport_manager_impl.hpp>
#include <libp2p/muxer/yamux/yamux.hpp>
#include <libp2p/security/noise.hpp>
#include <libp2p/transport/impl/upgrader_impl.hpp>
#include <libp2p/transport/tcp/tcp_transport.hpp>
#include <libp2p/protocol_muxer/multiselect.hpp>

#include "utils/callback_to_coro.h"
#include "utils/propagate.h"

namespace plc::core::network {

namespace {

std::shared_ptr<libp2p::network::Dialer> makem_dialer(std::shared_ptr<boost::asio::io_context> io_context) noexcept {
    auto multiselect = std::make_shared<libp2p::protocol_muxer::multiselect::Multiselect>();

    auto scheduler = std::make_shared<libp2p::basic::SchedulerImpl>(
        std::make_shared<libp2p::basic::AsioSchedulerBackend>(io_context),
        libp2p::basic::Scheduler::Config {}
    );

    auto bus = std::make_shared<libp2p::event::Bus>();
    auto connection_manager = std::make_shared<libp2p::network::ConnectionManagerImpl>(bus);

    auto csprng = std::make_shared<libp2p::crypto::random::BoostRandomGenerator>();
    auto ed25519_provider =
        std::make_shared<libp2p::crypto::ed25519::Ed25519ProviderImpl>();
    auto rsa_provider = std::make_shared<libp2p::crypto::rsa::RsaProviderImpl>();
    auto ecdsa_provider = std::make_shared<libp2p::crypto::ecdsa::EcdsaProviderImpl>();
    auto secp256k1_provider =
        std::make_shared<libp2p::crypto::secp256k1::Secp256k1ProviderImpl>();
    auto hmac_provider = std::make_shared<libp2p::crypto::hmac::HmacProviderImpl>();
    auto crypto_provider =
        std::make_shared<libp2p::crypto::CryptoProviderImpl>(
            csprng, ed25519_provider, rsa_provider, ecdsa_provider,
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

    return std::make_shared<libp2p::network::DialerImpl>(multiselect, transport_manager,
        connection_manager, listener_manager, scheduler);
}

} // namespace

ConnectionManager::ConnectionManager(runner::ClientRunner& runner,
    const std::vector<std::string>& peers) noexcept {
    m_dialer = makem_dialer(runner.get_service());
    for (const auto& peer: peers) {
        runner.post_task(connect_to(peer));
    }
}

cppcoro::task<void> ConnectionManager::connect_to(std::string peer) noexcept {
    // TODO: handle parse error
    auto multiaddr = libp2p::multi::Multiaddress::create(peer)
        .value();
    auto peerId = libp2p::peer::PeerId::fromBase58(multiaddr.getPeerId().value()).value();
    libp2p::peer::PeerInfo peerInfo {
        peerId,
        {multiaddr}
    };
    auto dial_result = co_await resume_in_callback<libp2p::network::Dialer::DialResult>([&](auto&& callback){
        m_dialer->dial(peerInfo, std::move(callback), std::chrono::milliseconds{1000});
    });

    // TODO: use logger
    if (dial_result.has_value()) {
        std::cout << "connection to " << peer << " succeded" << std::endl;
        m_connections.emplace(move(peer), move(dial_result.value()));
    } else {
        // TODO: redial on failure?
        std::cout << "connection to " << peer << " failed: " << dial_result.error() << std::endl;
    }
}

} // namespace plc::core::network
