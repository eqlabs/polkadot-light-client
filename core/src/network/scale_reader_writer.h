#pragma once

#include <memory>

#include <cppcoro/task.hpp>

#include <libp2p/basic/message_read_writer_uvarint.hpp>

#include <scale/scale.hpp>

#include "utils/result.h"
#include "utils/callback_to_coro.h"

namespace plc::core::network {
/**
 * Read and write messages, encoded into SCALE with a prepended varint
 */
class ScaleMessageReadWriter {
    using MessageReadWriter = libp2p::basic::MessageReadWriter;
public:
    explicit ScaleMessageReadWriter(
        std::shared_ptr<libp2p::basic::MessageReadWriter> read_writer);
    explicit ScaleMessageReadWriter(
        std::shared_ptr<libp2p::basic::ReadWriter> read_writer);

    template <typename MsgType>
    cppcoro::task<Result<MsgType>> read() const {
        auto read_res = co_await resumeInCallback<MessageReadWriter::ReadCallback>(
            [reader_writer = m_read_writer](auto func) {
                reader_writer->read(std::move(func)); 
        });

        if (!read_res) {
            co_return read_res.error();
        }

        if (read_res.value()) {
            if (auto msg_res = scale::decode<MsgType>(*read_res.value()); msg_res) {
                co_return std::move(msg_res.value());
            } else {
                co_return msg_res.error();
            }
        } else {
            co_return MsgType{};
        }
    }

    /**
     * SCALE-encode a message and write it to the channel
     */
    template <typename MsgType>
    cppcoro::task<Result<size_t>> write(const MsgType &msg) const {
        auto encoded_msg_res = scale::encode(msg);
        if (!encoded_msg_res) {
            co_return encoded_msg_res.error();
        }
        auto write_res = co_await resumeInCallback<Result<size_t>>(
            [reader_writer = m_read_writer, msg = std::move(encoded_msg_res.value())](auto func) {
                reader_writer->write(msg, std::move(func));
        });

        co_return write_res;
    }

    private:
        std::shared_ptr<libp2p::basic::MessageReadWriter> m_read_writer;
};
}  // namespace pcl::core::network

