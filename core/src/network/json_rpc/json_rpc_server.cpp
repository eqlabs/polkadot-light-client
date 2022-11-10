#include "network/json_rpc/json_rpc_server.h"

// #include <chrono>

#include <boost/asio/io_context.hpp>

#include "listener.hpp"
#include <boost/asio/signal_set.hpp>

namespace plc::core::network::json_rpc {

JsonRpcServer::JsonRpcServer(uint16_t port, std::shared_ptr<boost::asio::io_service> io)
    : m_port(port)
    , m_io_service(io) {
   connect();
}


void JsonRpcServer::connect() {

    // using awaitable_tcp_stream = decltype(packio::net::use_awaitable_t<>::as_default_on(
    //     std::declval<boost::beast::tcp_stream>()));
    // using websocket = packio::extra::
    //     websocket_adapter<boost::beast::websocket::stream<awaitable_tcp_stream>, false>;
    // using ws_acceptor =
    //     packio::extra::websocket_acceptor_adapter<packio::net::ip::tcp::acceptor, websocket>;

    std::string localhost = "127.0.0.1";
    boost::system::error_code ec;
    boost::asio::ip::address ip_address = boost::asio::ip::address::from_string(localhost, ec);
    boost::asio::ip::tcp::endpoint bind_ep(ip_address, m_port);


    // Create and launch a listening port
    std::make_shared<listener>(
        *m_io_service,
        bind_ep)->run();

}

} // namespace plc::core::network
