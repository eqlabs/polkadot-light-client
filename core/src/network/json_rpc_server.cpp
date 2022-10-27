#include "network/json_rpc_server.h"

// #include <chrono>

#include <boost/asio/io_context.hpp>
#include <packio/packio.h>
#include <packio/extra/websocket.h>


namespace plc::core::network {

JsonRpcServer::JsonRpcServer(std::string ip_address, uint16_t port,
    std::shared_ptr<boost::asio::io_service> io)
    : m_ip_address(ip_address)
    , m_port(port)
    , m_io_service(io) {
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

    m_packio_server = packio::json_rpc::make_server(ws_acceptor{*m_io_service, bind_ep});
    
    
    m_packio_server->dispatcher()->add_coro(
        "pow", *m_io_service, [](int a, int b) -> packio::net::awaitable<int> {
            printf("pow: a is %d, b is %d\n", a, b);
            co_return std::pow(a, b);
        });

    m_packio_server->async_serve_forever();

    return true;
}


} // namespace plc::core::network
