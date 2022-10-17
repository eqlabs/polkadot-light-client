#pragma once

#include <string_view>

#include <fmt/format.h>

#include <libp2p/peer/peer_id.hpp>

// TODO: possibly we may want more enhanced formatting support
template <>
struct fmt::formatter<libp2p::peer::PeerId> : public fmt::formatter<std::string_view> {
  template <typename FormatContext>
  auto format(const libp2p::peer::PeerId& peer_id, FormatContext& ctx) {
    return fmt::formatter<string_view>::format(peer_id.toBase58(), ctx);
  }
};
