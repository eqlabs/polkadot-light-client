#include "network/json_rpc/json_rpc_server.h"

// #include <chrono>

#include <boost/asio/io_context.hpp>

#include <boost/asio/signal_set.hpp>

namespace plc::core::network::json_rpc {

JsonRpcServer::JsonRpcServer(uint16_t port, std::shared_ptr<boost::asio::io_service> io)
    : m_port(port)
    , m_io_service(io)
    , m_acceptor(*io)
    , m_socket(*io)
    , m_client_id(1000)
     {
}

int JsonRpcServer::getNextId() noexcept {
    m_client_id++;
    return m_client_id;
}

void JsonRpcServer::onClose(int id) {
    m_log->warn("JsonRpcServer::onClose id {}", id);
}
void JsonRpcServer::onMessage(int id, std::string message) {
    m_log->warn("JsonRpcServer::onMessage id {}, message {}", id, message);
}


void JsonRpcServer::connect() {
    std::string localhost = "127.0.0.1";
    boost::system::error_code ec;
    boost::asio::ip::address ip_address = boost::asio::ip::address::from_string(localhost, ec);
    boost::asio::ip::tcp::endpoint bind_ep(ip_address, m_port);

    auto self = shared_from_this();
    websocket_session::onClose = [self](int id) {
        self->onClose(id);
    };
    websocket_session::onMessage = [self](int id, std::string message) {
        self->onMessage(id, message);
    };

    m_acceptor.open(bind_ep.protocol(), ec);
    if(ec) {
        fail(ec, "open");
        return;
    }

    // Allow address reuse
    m_acceptor.set_option(net::socket_base::reuse_address(true));
    if(ec) {
        fail(ec, "set_option");
        return;
    }

    // Bind to the server address
    m_acceptor.bind(bind_ep, ec);
    if(ec) {
        fail(ec, "bind");
        return;
    }

    // Start listening for connections
    m_acceptor.listen(
        net::socket_base::max_listen_connections, ec);
    if(ec) {
        fail(ec, "listen");
        return;
    }

    run();

    m_log->warn("done connect-------------");
}

void JsonRpcServer::run()
{
    // Start accepting a connection
    m_log->warn("start run");
    auto self = shared_from_this();
    m_log->warn("got self");
    m_acceptor.async_accept(m_socket,
        [self](error_code ec) {
            self->onAccept(ec);
        });
}

// Report a failure

void JsonRpcServer::fail(error_code ec, char const* what) noexcept {
    // Don't report on canceled operations
    if(ec == net::error::operation_aborted) {
        return;
    }
    m_log->warn("JsonRpcServer::fail: {}: {}", what, ec.message());
}

void JsonRpcServer::onAccept(error_code ec)
{
    m_log->warn("JKL: listener onAccept");
    if(ec) {
        return fail(ec, "accept");
    } else {
        // Launch a new session for this connection
        auto id = getNextId();
        auto client = std::make_shared<JsonRpcClient>(id, m_io_service);
        m_clients.emplace(id,client);
        std::make_shared<http_session>(
            std::move(m_socket), id)->run();
    }

    // Accept another connection
    m_acceptor.async_accept(m_socket,
        [self = shared_from_this()](error_code ec) {
            self->onAccept(ec);
        });
}


} // namespace plc::core::network
