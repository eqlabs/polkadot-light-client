#include "scale_reader_writer.h"

namespace plc::core::network {
    ScaleMessageReadWriter::ScaleMessageReadWriter(
        std::shared_ptr<libp2p::basic::MessageReadWriter> read_writer)
        : m_read_writer{std::move(read_writer)} {}

    ScaleMessageReadWriter::ScaleMessageReadWriter(
        std::shared_ptr<libp2p::basic::ReadWriter> read_writer)
        : m_read_writer{std::make_shared<libp2p::basic::MessageReadWriterUvarint>(
            std::move(read_writer))} {}
}  // namespace plc::core::network
