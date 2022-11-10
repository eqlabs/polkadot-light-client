#include "network/json_rpc/json_rpc_client.h"

// #include <chrono>

#include <boost/asio/io_context.hpp>

#include <boost/asio/signal_set.hpp>

namespace plc::core::network::json_rpc {

JsonRpcClient::JsonRpcClient(int id, std::shared_ptr<boost::asio::io_service> io)
    : m_id(id)
    , m_io_service(io) {
}




} // namespace plc::core::network
