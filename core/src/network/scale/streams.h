#pragma once

namespace plc::core::network::scale {

template <typename Stream>
concept EncoderStream = Stream::is_encoder_stream;

template <typename Stream>
concept DecoderStream = Stream::is_decoder_stream;

template <EncoderStream Stream>
struct Encoder {
    template <typename T>
    void operator()(const T& value) {
        m_stream << value;
    }

    Stream& m_stream;
};

template <DecoderStream Stream>
struct Decoder {
    template <typename T>
    void operator()(T& value) {
        m_stream >> value;
    }

    Stream& m_stream;
};

#define PLC_DEFINE_STREAM_OPERATORS(Type) \
template <plc::core::network::scale::EncoderStream Stream> \
Stream& operator<<(Stream& stream, const Type& val) { \
    plc::core::network::scale::Encoder<Stream> encoder{stream}; \
    Type::serialize(val, encoder); \
    return stream; \
} \
\
template <plc::core::network::scale::DecoderStream Stream> \
Stream& operator>>(Stream& stream, Type& val) { \
    plc::core::network::scale::Decoder<Stream> decoder{stream}; \
    Type::serialize(val, decoder); \
    return stream; \
}

} // namespace plc::core::network