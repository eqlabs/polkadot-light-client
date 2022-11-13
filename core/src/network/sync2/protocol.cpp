#include "protocol.h"

#include <memory>
#include <cppcoro/task.hpp>
#include <libp2p/basic/protobuf_message_read_writer.hpp>

#include "network/common/errors.h"
#include "network/protobuf/message_reader_writer.h"
#include "utils/result.h"
#include "utils/callback_to_coro.h"

#include <libp2p/basic/message_read_writer_uvarint.hpp>

#include <iostream>

namespace plc::core::network::sync2 {

// TODO: use prefix from chainspec (use of {} instead of dot)
const libp2p::peer::Protocol Protocol::protocol = "/dot/sync/2";

Protocol::Protocol( libp2p::Host& host, runner::ClientRunner& client_runner)
        : m_host(host), m_runner(client_runner) {
    m_host.setProtocolHandler(protocol, [](auto&& stream) {
        // ToDo: implement
    });
}

cppcoro::task<Result<proto::BlockResponse>> Protocol::send(proto::BlockRequest&& request_ref, const PeerId& peerId) {
    // open a new outgoing stream
    auto request = std::move(request_ref); // capture request parameter
    auto stream_res = co_await resumeInCallback<Result<std::shared_ptr<Stream>>>(
            [&peerId, &host = m_host](auto&& func) {
                host.newStream(
                        peerId,
                        protocol,
                        std::move(func));
            });

    if (!stream_res) {
        m_log->debug("Getting stream caused an error: {}", stream_res.error().message());
        co_return stream_res.error();
    }

    // send request
    auto reader_writer = std::make_shared<libp2p::basic::ProtobufMessageReadWriter>(std::move(stream_res.value()));
    auto write_res = co_await resumeInCallback<Result<size_t>>(
            [&reader_writer, &request](auto&& func){
                reader_writer->write(request, std::move(func));
            });

    if (!write_res) {
        m_log->debug("Writing to a stream caused error: {}", write_res.error().message());
        co_return write_res.error();
    }

    // read response
    auto read_res = co_await resumeInCallback<Result<proto::BlockResponse>>(
            [&reader_writer](auto&& func) {
                reader_writer->read<proto::BlockResponse>(std::move(func));
            });

    if (!read_res) {
        m_log->debug("Reading from stream caused error: {}", read_res.error().message());
        co_return read_res.error();
    }

    co_return std::move(read_res.value());
}

} // namespace plc::core::network::sync2