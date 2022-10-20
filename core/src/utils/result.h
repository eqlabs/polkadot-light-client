#pragma once

#include <libp2p/outcome/outcome.hpp>

namespace plc::core {

template <class R, class S = std::error_code>
using Result = libp2p::outcome::result<R, S>;

using libp2p::outcome::success;

} // namespace plc::core
