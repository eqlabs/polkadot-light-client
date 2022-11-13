#pragma once

#include <memory>

#include <cppcoro/task.hpp>

#include <libp2p/connection/stream.hpp>
#include <libp2p/host/host.hpp>
#include <libp2p/log/logger.hpp>

#include "network/protobuf/utils.h"

#include <sync.pb.h>

namespace plc::core::runner {

    class ClientRunner;

} // namespace plc::core::runner

namespace plc::core::network::sync2 {

/**
 * @brief Protocol for syncing blocks (/dot/sync/2).
 * Class similar to all other protocol classes in the project.
 */
class Protocol : public std::enable_shared_from_this<Protocol> {
    template <typename T>
    using task = cppcoro::task<T>;
    using PeerId = libp2p::peer::PeerId;
    using Stream = libp2p::connection::Stream;

public:
    Protocol() = delete;
    Protocol(Protocol&&) noexcept = delete;
    Protocol(const Protocol&) = delete;
    Protocol &operator=(Protocol&&) noexcept = delete;
    Protocol &operator=(const Protocol&) = delete;

    /**
     * @brief Construct a new Protocol object
     * @param[in] host libp2p host
     * @param[in] client_runner client runner for task execution
     */
    Protocol(
            libp2p::Host& host,
            runner::ClientRunner& client_runner);

    /**
     * @brief Send a block request to a peer and receive a response.
     * This method sends
     * @param[in] request block request
     * @param[in] peerId peer id according to libp2p
     * @return block response
     */
    task<Result<proto::BlockResponse>> send(proto::BlockRequest&& request, const PeerId& peerId);

private:
    static const libp2p::peer::Protocol protocol;

    libp2p::Host& m_host;
    runner::ClientRunner& m_runner;

    libp2p::log::Logger m_log = libp2p::log::createLogger("Protocol", "sync_v2_protocol");
};

} // plc::core::network::sync2
