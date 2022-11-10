#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include <boost/asio.hpp>
#include <boost/asio/use_awaitable.hpp>

#include <libp2p/peer/peer_id.hpp>
#include <libp2p/network/connection_manager.hpp>

#include "runner/client_runner.h"

namespace boost::asio {
    class io_context;
} // namespace boost::asio


namespace plc::core::network::json_rpc {

class JsonRpcServer : std::enable_shared_from_this<JsonRpcServer> {
public:
    JsonRpcServer(uint16_t port, std::shared_ptr<boost::asio::io_service> io);
    ~JsonRpcServer() = default;
    std::shared_ptr<boost::asio::io_service> const getIoService() { return m_io_service; }
    const libp2p::log::Logger getLogger() const { return m_log; }
    void connect();

private:
    libp2p::log::Logger m_log = libp2p::log::createLogger("JsonRpcServer","network");
    const std::shared_ptr<boost::asio::io_service> m_io_service;
    const uint16_t m_port;
};

} // namespace plc::core::network