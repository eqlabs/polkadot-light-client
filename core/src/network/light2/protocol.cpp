#include "protocol.h"

#include "network/common/errors.h"
#include "network/protobuf/message_reader_writer.h"
#include "utils/callback_to_coro.h"

namespace plc::core::network::light2 {

// TODO: use prefix from chainspec
const libp2p::peer::Protocol Protocol::protocol = "dot/light/2";

Protocol::Protocol(
    libp2p::Host& host,
    runner::ClientRunner& client_runner)
    : m_host(host), m_runner(client_runner) {
    m_host.setProtocolHandler(protocol, [](auto&& stream) {
        // close the incoming stream since we're light client
        // and do not have ability to answer the requests
        stream->close([](auto&&) {});
    });
}

template <protobuf::ConvertibleToProtobuf Request, protobuf::ConvertibleFromProtobuf Response>
cppcoro::task<Result<Response>> Protocol::send(Request&& request_ref, const PeerId& peerId) {
    // open a new outgoing stream
    auto request = std::move(request_ref); // capture request parameter
    auto stream_res = co_await resumeInCallback<Result<std::shared_ptr<Stream>>>(
        [peerId, &host = m_host](auto&& func) {
            host.newStream(
                peerId,
                protocol,
                std::move(func));
    });
    if (!stream_res) {
        co_return stream_res.error();
    }

    // send request
    auto reader_writer = protobuf::MessageReadWriter(stream_res.value());
    auto write_res = co_await reader_writer.write(std::move(request));
    if (!write_res) {
        co_return write_res.error();
    }

    co_return co_await reader_writer.read<Response>();
}

cppcoro::task<Result<RemoteCallResponse>> Protocol::send(RemoteCallRequest&& request, const PeerId& peerId) {
    co_return co_await send<RemoteCallRequest, RemoteCallResponse>(std::move(request), peerId);
}

cppcoro::task<Result<RemoteReadResponse>> Protocol::send(RemoteReadRequest&& request, const PeerId& peerId) {
    co_return co_await send<RemoteReadRequest, RemoteReadResponse>(std::move(request), peerId);
}

cppcoro::task<Result<RemoteReadResponse>> Protocol::send(RemoteReadChildRequest&& request, const PeerId& peerId) {
    co_return co_await send<RemoteReadChildRequest, RemoteReadResponse>(std::move(request), peerId);
}

cppcoro::task<Result<RemoteHeaderResponse>> Protocol::send(RemoteHeaderRequest&& request, const PeerId& peerId) {
    co_return co_await send<RemoteHeaderRequest, RemoteHeaderResponse>(std::move(request), peerId);
}

cppcoro::task<Result<RemoteChangesResponse>> Protocol::send(RemoteChangesRequest&& request, const PeerId& peerId) {
    co_return co_await send<RemoteChangesRequest, RemoteChangesResponse>(std::move(request), peerId);
}



} // namespace plc::core::network::light2