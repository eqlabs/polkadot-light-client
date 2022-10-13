#pragma once

#include <memory>

#include <cppcoro/task.hpp>

#include <libp2p/connection/stream.hpp>
#include <libp2p/host/host.hpp>
#include <libp2p/log/logger.hpp>

#include "network/light2/messages.h"
#include "network/protobuf/utils.h"
#include "utils/result.h"

namespace plc::core::runner {

class ClientRunner;

} // namespace plc::core::runner

namespace plc::core::network::light2 {

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

    Protocol(
        libp2p::Host& host,
        runner::ClientRunner& client_runner);

    task<Result<RemoteCallResponse>> send(RemoteCallRequest&& request, const PeerId& peerId);
    task<Result<RemoteReadResponse>> send(RemoteReadRequest&& request, const PeerId& peerId);
    task<Result<RemoteReadResponse>> send(RemoteReadChildRequest&& request, const PeerId& peerId);
    task<Result<RemoteHeaderResponse>> send(RemoteHeaderRequest&& request, const PeerId& peerId);
    task<Result<RemoteChangesResponse>> send(RemoteChangesRequest&& request, const PeerId& peerId);

private:
    template <protobuf::ConvertibleToProtobuf Request, protobuf::ConvertibleFromProtobuf Response>
    task<Result<Response>> send(Request&& request, const PeerId& peerId);

private:
    static const libp2p::peer::Protocol protocol;

    libp2p::Host& m_host;
    runner::ClientRunner& m_runner;

    libp2p::log::Logger m_log = libp2p::log::createLogger("Protocol", "light_v2_protocol");
};

} // plc::core::network::light2