#pragma once

#include <memory>

#include <cppcoro/task.hpp>

#include <libp2p/basic/protobuf_message_read_writer.hpp>

#include "network/common/errors.h"
#include "network/protobuf/utils.h"

#include "network/common/errors.h"
#include "utils/callback_to_coro.h"
#include "utils/result.h"

namespace plc::core::network::protobuf {

// Read-write protobuf messages
class MessageReadWriter {
public:
    explicit MessageReadWriter(std::shared_ptr<libp2p::basic::ReadWriter> read_writer);

    template <ConvertibleFromProtobuf MsgType>
    cppcoro::task<Result<MsgType>> read() const {
        auto read_writer = m_read_writer;
        using ProtoMessageType = typename MsgType::ProtoMessageType;
        auto read_res = co_await resumeInCallback<Result<ProtoMessageType>>(
            [&read_writer](auto&& func) {
                read_writer->read<ProtoMessageType>(std::move(func));
        });

        if (!read_res) {
            co_return read_res.error();
        }

        co_return MsgType::fromProto(std::move(read_res.value()));
    }

    template <ConvertibleToProtobuf MsgType>
    cppcoro::task<Result<void>> write(MsgType&& msg) {
        const auto proto_msg = (std::move(msg)).toProto();
        auto read_writer = m_read_writer;
        auto write_res = co_await resumeInCallback<Result<size_t>>(
            [&read_writer, &proto_msg](auto&& func){
                read_writer->write(proto_msg, std::move(func));
        });

        if (!write_res) {
            co_return write_res.error();
        } else {
            co_return success();
        }
    }

private:
    std::shared_ptr<libp2p::basic::ProtobufMessageReadWriter> m_read_writer;
};

} // namespace plc::core::network::protobuf
