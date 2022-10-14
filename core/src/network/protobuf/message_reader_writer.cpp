#include "message_reader_writer.h"

#include <libp2p/basic/message_read_writer_uvarint.hpp>

namespace plc::core::network::protobuf {

MessageReadWriter::MessageReadWriter(
    std::shared_ptr<libp2p::basic::ReadWriter> read_writer)
    : m_read_writer(
        std::make_shared<libp2p::basic::MessageReadWriterUvarint>(
            std::move(read_writer))) {}

} // namespace plc::core::network::protobuf
