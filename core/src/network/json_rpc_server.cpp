#include "network/json_rpc_server.h"

// #include <chrono>

#include <boost/asio/io_context.hpp>


// #include <libp2p/basic/scheduler/scheduler_impl.hpp>
// #include <libp2p/basic/scheduler/asio_scheduler_backend.hpp>
// #include <libp2p/crypto/aes_ctr/aes_ctr_impl.hpp>
// #include <libp2p/crypto/crypto_provider/crypto_provider_impl.hpp>
// #include <libp2p/crypto/ecdsa_provider/ecdsa_provider_impl.hpp>
// #include <libp2p/crypto/ed25519_provider/ed25519_provider_impl.hpp>
// #include <libp2p/crypto/hmac_provider/hmac_provider_impl.hpp>
// #include <libp2p/crypto/key_marshaller/key_marshaller_impl.hpp>
// #include <libp2p/crypto/key_validator/key_validator_impl.hpp>
// #include <libp2p/crypto/random_generator/boost_generator.hpp>
// #include <libp2p/crypto/rsa_provider/rsa_provider_impl.hpp>
// #include <libp2p/crypto/secp256k1_provider/secp256k1_provider_impl.hpp>
// #include <libp2p/host/basic_host/basic_host.hpp>
// #include <libp2p/network/impl/dialer_impl.hpp>
// #include <libp2p/network/impl/dnsaddr_resolver_impl.hpp>
// #include <libp2p/network/impl/connection_manager_impl.hpp>
// #include <libp2p/network/cares/cares.hpp>
// #include <libp2p/network/impl/listener_manager_impl.hpp>
// #include <libp2p/network/impl/router_impl.hpp>
// #include <libp2p/network/impl/transport_manager_impl.hpp>
// #include <libp2p/network/impl/network_impl.hpp>
// #include <libp2p/muxer/yamux/yamux.hpp>
// #include <libp2p/peer/address_repository/inmem_address_repository.hpp>
// #include <libp2p/peer/key_repository/inmem_key_repository.hpp>
// #include <libp2p/peer/protocol_repository/inmem_protocol_repository.hpp>
// #include <libp2p/peer/impl/identity_manager_impl.hpp>
// #include <libp2p/peer/impl/peer_repository_impl.hpp>
// #include <libp2p/protocol_muxer/multiselect.hpp>
// #include <libp2p/protocol/kademlia/impl/content_routing_table_impl.hpp>
// #include <libp2p/protocol/kademlia/impl/kademlia_impl.hpp>
// #include <libp2p/protocol/kademlia/impl/peer_routing_table_impl.hpp>
// #include <libp2p/protocol/kademlia/impl/storage_impl.hpp>
// #include <libp2p/protocol/kademlia/impl/storage_backend_default.hpp>
// #include <libp2p/protocol/kademlia/impl/validator_default.hpp>
// #include <libp2p/protocol/identify/identify.hpp>
// #include <libp2p/protocol/ping/ping.hpp>
// #include <libp2p/security/noise.hpp>
// #include <libp2p/transport/impl/upgrader_impl.hpp>
// #include <libp2p/transport/tcp/tcp_transport.hpp>

#include <packio/packio.h>
#include <packio/extra/websocket.h>

// #include "network/common/format_peer_id.h"
// #include "network/grandpa/protocol.h"
// #include "network/light2/protocol.h"
// #include "utils/callback_to_coro.h"
// #include "utils/propagate.h"
// #include "utils/result.h"


namespace plc::core::network {

libp2p::log::Logger JsonRpcServer::m_log = {};


JsonRpcServer::JsonRpcServer(std::string ip_address, uint16_t port,
    std::shared_ptr<boost::asio::io_service> io)
    : m_ip_address(ip_address)
    , m_port(port)
    , m_io_service(io) {

    if (m_log == nullptr) {
        m_log = libp2p::log::createLogger("JSON-RPC","network");
    }
    connect();
}


JsonRpcServer::~JsonRpcServer() {

}

void JsonRpcServer::stop() noexcept {
    m_log->debug("JSON-RPC server: stop");
    // for (auto& [peer, state]: m_peers_info) {
    //     if (state.state == ConnectionState::Connected) {
    //         state.action = ConnectionAction::Disconnecting;
    //         m_log->debug("  disconnecting from {}", peer.toHex());
    //         m_host->disconnect(peer);
    //     }
    // }
}


libp2p::log::Logger JsonRpcServer::getLogger() {
    return m_log;
}

bool JsonRpcServer::connect() {

    using awaitable_tcp_stream = decltype(packio::net::use_awaitable_t<>::as_default_on(
        std::declval<boost::beast::tcp_stream>()));
    using websocket = packio::extra::
        websocket_adapter<boost::beast::websocket::stream<awaitable_tcp_stream>, false>;
    using ws_acceptor =
        packio::extra::websocket_acceptor_adapter<packio::net::ip::tcp::acceptor, websocket>;

    boost::system::error_code ec;
    boost::asio::ip::address ip_address =
    boost::asio::ip::address::from_string(m_ip_address, ec);

    if (ec.value() != 0) {
        // Provided IP address is invalid. Breaking execution.
        m_log->error("Failed to parse the IP address. Error code = {}, Message: {}", ec.value(), ec.message());
        return false; // ec.value();
    }

    boost::asio::ip::tcp::endpoint bind_ep(ip_address, m_port);

    auto server = packio::json_rpc::make_server(ws_acceptor{*m_io_service, bind_ep});

    server->dispatcher()->add_coro(
        "pow", *m_io_service, [](int a, int b) -> packio::net::awaitable<int> {
            printf("pow: a is %d, b is %d\n", a, b);
            co_return std::pow(a, b);
        });

    server->async_serve_forever();

    return true;
}


} // namespace plc::core::network
