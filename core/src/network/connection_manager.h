#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include <boost/asio/io_service.hpp>

#include <cppcoro/task.hpp>

#include <libp2p/connection/capable_connection.hpp>
#include <libp2p/network/dialer.hpp>

#include "runner/client_runner.h"

namespace plc::core::network {

class ConnectionManager final {
public:
    ConnectionManager(runner::ClientRunner& runner,
        const std::vector<std::string>& peers) noexcept;

private:
    using ConnectionSPtr = std::shared_ptr<libp2p::connection::CapableConnection>;

private:
    cppcoro::task<void> connectTo(std::string peer) noexcept;

private:
    std::shared_ptr<libp2p::network::Dialer> m_dialer;
    std::unordered_map<std::string, ConnectionSPtr> m_connections;
};

} // namespace plc::core::network
