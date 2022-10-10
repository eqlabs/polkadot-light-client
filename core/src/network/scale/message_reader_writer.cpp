#include "message_reader_writer.h"

namespace plc::core::network::scale {
    MessageReadWriter::MessageReadWriter(
        std::shared_ptr<P2PMessageReadWriter> read_writer)
        : m_read_writer{std::move(read_writer)} {}

    MessageReadWriter::MessageReadWriter(
        std::shared_ptr<libp2p::basic::ReadWriter> read_writer)
        : m_read_writer{std::make_shared<libp2p::basic::MessageReadWriterUvarint>(
            std::move(read_writer))} {}
}  // namespace plc::core::network::scale
