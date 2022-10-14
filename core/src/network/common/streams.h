#pragma once

namespace plc::core::network {

template <typename Stream>
concept EncoderStream = Stream::is_encoder_stream;

template <typename Stream>
concept DecoderStream = Stream::is_decoder_stream;

} // namespace plc::core::network
