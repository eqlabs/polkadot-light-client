#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include <boost/asio.hpp>
#include <boost/asio/use_awaitable.hpp>

#include <libp2p/peer/peer_id.hpp>
#include <libp2p/network/connection_manager.hpp>


#include "websocket_session.hpp"

namespace boost::asio {
    class io_context;
} // namespace boost::asio


namespace plc::core::network::json_rpc {

class JsonRpcClient {
public:
    JsonRpcClient(int id, std::shared_ptr<boost::asio::io_service> io);
    ~JsonRpcClient() = default;
    std::shared_ptr<boost::asio::io_service> const getIoService() { return m_io_service; }
    const libp2p::log::Logger getLogger() const { return m_log; }

private:
    libp2p::log::Logger m_log = libp2p::log::createLogger("JsonRpcClient","network");
    const std::shared_ptr<boost::asio::io_service> m_io_service;
    const std::shared_ptr<websocket_session> m_session;
    const int m_id;
};

} // namespace plc::core::network
