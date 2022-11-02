#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include <boost/asio.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <packio/packio.h>
#include <packio/extra/websocket.h>

#include <libp2p/peer/peer_id.hpp>
#include <libp2p/network/connection_manager.hpp>

#include "runner/client_runner.h"

namespace boost::asio {
    class io_context;
} // namespace boost::asio

using packio_server = std::shared_ptr<packio::server<packio::json_rpc::rpc, packio::extra::websocket_acceptor_adapter
    <boost::asio::basic_socket_acceptor<boost::asio::ip::tcp>, packio::extra::websocket_adapter
    <boost::beast::websocket::stream<boost::beast::basic_stream<boost::asio::ip::tcp, boost::asio::use_awaitable_t
    <>::executor_with_default<boost::asio::any_io_executor>>, true>, false>>>>;

namespace plc::core::network {

class JsonRpcServer final {
public:
    JsonRpcServer(uint16_t port, std::shared_ptr<boost::asio::io_service> io);
    ~JsonRpcServer() = default;
    std::shared_ptr<boost::asio::io_service> const getIoService() { return m_io_service; }
    const packio_server getServer() const { return m_packio_server; }

private:
    void connect();
    libp2p::log::Logger m_log = libp2p::log::createLogger("JsonRpcServer","network");
    const std::shared_ptr<boost::asio::io_service> m_io_service;
    const uint16_t m_port;
    packio_server m_packio_server;
};

} // namespace plc::core::network
