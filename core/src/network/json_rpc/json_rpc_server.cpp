#include "network/json_rpc/json_rpc_server.h"

// #include <chrono>

#include <boost/asio/io_context.hpp>

#include "listener.h"
#include <boost/asio/signal_set.hpp>

namespace plc::core::network::json_rpc {

JsonRpcServer::JsonRpcServer(uint16_t port, std::shared_ptr<boost::asio::io_service> io)
    : m_port(port)
    , m_io_service(io)
    , acceptor_(*io)
    , socket_(*io)
    
     {
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

    // Open the acceptor
    acceptor_.open(bind_ep.protocol(), ec);
    m_log->warn("JKL: listener open");
    if(ec)
    {
        fail(ec, "open");
        return;
    }

    // Allow address reuse
    acceptor_.set_option(net::socket_base::reuse_address(true));
    if(ec)
    {
        fail(ec, "set_option");
        return;
    }

    // Bind to the server address
    acceptor_.bind(bind_ep, ec);
    if(ec)
    {
        fail(ec, "bind");
        return;
    }

    // Start listening for connections
    acceptor_.listen(
        net::socket_base::max_listen_connections, ec);
    if(ec)
    {
        fail(ec, "listen");
        return;
    }

    run();



    // // Create and launch a listening port
    // std::make_shared<listener>(
    //     *m_io_service,
    //     bind_ep)->run();

    m_log->warn("done connect-------------");

}

void
JsonRpcServer::
run()
{
    // Start accepting a connection
    m_log->warn("start run");
    auto self = shared_from_this();
    m_log->warn("got self");
    acceptor_.async_accept(
        socket_,
        [self](error_code ec)
        {
            self->on_accept(ec);
        });
}

// Report a failure

void
JsonRpcServer::
fail(error_code ec, char const* what)
{
    // Don't report on canceled operations
    if(ec == net::error::operation_aborted)
        return;
    m_log->warn("{}: {}", what, ec.message());
    // std::cerr << what << ": " << ec.message() << "\n";
}

// Handle a connection
void
JsonRpcServer::
on_accept(error_code ec)
{
    m_log->warn("JKL: listener on_accept");
    if(ec)
        return fail(ec, "accept");
    else
        // Launch a new session for this connection
        std::make_shared<http_session>(
            std::move(socket_))->run();

    // Accept another connection
    acceptor_.async_accept(
        socket_,
        [self = shared_from_this()](error_code ec)
        {
            self->on_accept(ec);
        });
}


} // namespace plc::core::network
