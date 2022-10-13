#pragma once

#include <memory>

#include <cppcoro/task.hpp>

#include <libp2p/basic/message_read_writer_uvarint.hpp>
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
        auto read_res = co_await resumeInCallback<Result<std::shared_ptr<std::vector<uint8_t>>>>(
            [read_writer = m_read_writer](auto&& func) {
                read_writer->read(std::move(func));
        });

        if (!read_res) {
            co_return read_res.error();
        }

        using ProtoMessageType = typename MsgType::ProtoMessageType;
        ProtoMessageType proto_msg;
        auto& buffer = *read_res.value();
        if (!proto_msg.ParseFromArray(buffer.data(), buffer.size())) {
            co_return ProtocolError::ProtobufParsingFailed;
        }

        co_return MsgType::fromProto(std::move(proto_msg));
    }

    template <ConvertibleToProtobuf MsgType>
    cppcoro::task<Result<void>> write(MsgType&& msg) {
        std::vector<uint8_t> buffer;
        writeToVec(std::move(msg), buffer);

        auto write_res = co_await resumeInCallback<Result<size_t>>(
            [read_writer = m_read_writer, &buffer](auto&& func){
                read_writer->write(buffer, std::move(func));
        });

        if (!write_res) {
            co_return write_res.error();
        } else {
            co_return success();
        }
    }

private:
    std::shared_ptr<libp2p::basic::MessageReadWriter> m_read_writer;
};

} // namespace plc::core::network::protobuf
