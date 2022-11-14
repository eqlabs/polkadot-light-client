#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include <boost/asio.hpp>

#include "runner/client_runner.h"
#include "http_session.h"
#include "jrpc_client.h"
#include <boost/property_tree/json_parser.hpp>

namespace boost::asio {
    class io_context;
} // namespace boost::asio



namespace plc::core::network::json_rpc {

namespace pt = boost::property_tree;

using JrpcHandler = std::function<void(std::shared_ptr<JrpcClient>, pt::ptree &params)>;

class JrpcServer : public std::enable_shared_from_this<JrpcServer> {
public:
    JrpcServer(uint16_t port, std::shared_ptr<boost::asio::io_service> io);
    ~JrpcServer() = default;
    const std::shared_ptr<boost::asio::io_service> getIoService() noexcept { return m_io_service; }
    const libp2p::log::Logger getLogger() noexcept { return m_log; }
    void connect();
    void run();
    void initSessionCallbacks();
    void onOpen(int id, std::shared_ptr<WebSocketSession> session);
    void onMessage(int id, std::string message);
    void onClose(int id);
    void registerHandler(std::string, JrpcHandler);

private:
    void fail(error_code ec, char const* what) noexcept;
    void onAccept(error_code ec);
    int getNextId() noexcept;
 
    libp2p::log::Logger m_log = libp2p::log::createLogger("JrpcServer","network");
    const std::shared_ptr<boost::asio::io_service> m_io_service;
    const uint16_t m_port;
    int m_client_id;
    tcp::acceptor m_acceptor;
    tcp::socket m_socket;
    std::unordered_map<int,std::shared_ptr<JrpcClient>> m_clients;
    std::unordered_map<std::string,JrpcHandler> m_handlers;
};

} // namespace plc::core::network::json_rpc
