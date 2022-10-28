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
    m_log->info("JsonRpcServer destructor");
}

void JsonRpcServer::stop() noexcept {
    // TODO: not sure yet how to stop the server
    m_log->info("JSON-RPC server: stop");
}

void JsonRpcServer::connect() {

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
        m_log->error("Failed to parse the IP address. Error code = {}, Message: {}", ec.value(), ec.message());
        return;
    }

    boost::asio::ip::tcp::endpoint bind_ep(ip_address, m_port);

    try {
        m_packio_server = packio::json_rpc::make_server(ws_acceptor{*m_io_service, bind_ep});
    } catch (boost::wrapexcept<boost::system::system_error> e) {
        m_log->error("could not make connection for JSON-RPC server");
        return;
    }
    m_packio_server->async_serve_forever();
    m_connected = true;
}

} // namespace plc::core::network
