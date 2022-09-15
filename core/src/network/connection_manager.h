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

class ConnectionManager {
public:
    ConnectionManager(runner::ClientRunner& runner,
        const std::vector<std::string>& peers);

private:
    using ConnectionSPtr = std::shared_ptr<libp2p::connection::CapableConnection>;

private:
    cppcoro::task<void> connect_to(std::string peer);

private:
    std::shared_ptr<libp2p::network::Dialer> _dialer;
    std::unordered_map<std::string, ConnectionSPtr> _connections;
};

} // namespace plc::core::network
