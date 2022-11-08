#pragma once

#include <memory>

#include <cppcoro/task.hpp>

#include <libp2p/connection/stream.hpp>
#include <libp2p/host/host.hpp>
#include <libp2p/log/logger.hpp>

#include "network/protobuf/utils.h"

namespace plc::core::network::sync2 {

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

        //Protocol() {}

    private:
        static const libp2p::peer::Protocol protocol;

        libp2p::log::Logger m_log = libp2p::log::createLogger("Protocol", "sync_v2_protocol");
    };

} // plc::core::network::sync2
