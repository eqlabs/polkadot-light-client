#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include <boost/asio.hpp>

#include "runner/client_runner.h"
#include "http_session.h"
#include "json_rpc_client.h"

namespace boost::asio {
    class io_context;
} // namespace boost::asio


namespace plc::core::network::json_rpc {

class JsonRpcServer : public std::enable_shared_from_this<JsonRpcServer> {
public:
    JsonRpcServer(uint16_t port, std::shared_ptr<boost::asio::io_service> io);
    ~JsonRpcServer() = default;
    const std::shared_ptr<boost::asio::io_service> getIoService() noexcept { return m_io_service; }
    const libp2p::log::Logger getLogger() noexcept { return m_log; }
    void connect();
    void run();
    void onOpen(int id, std::shared_ptr<WebSocketSession> session);
    void onMessage(int id, std::string message);
    void onClose(int id);

private:
    void fail(error_code ec, char const* what) noexcept;
    void onAccept(error_code ec);
    int getNextId() noexcept;
 
    libp2p::log::Logger m_log = libp2p::log::createLogger("JsonRpcServer","network");
    const std::shared_ptr<boost::asio::io_service> m_io_service;
    const uint16_t m_port;
    int m_client_id;
    tcp::acceptor m_acceptor;
    tcp::socket m_socket;
    std::unordered_map<int,std::shared_ptr<JsonRpcClient>> m_clients;
};

} // namespace plc::core::network
