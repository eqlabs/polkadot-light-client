#pragma once

#include <libp2p/outcome/outcome.hpp>

namespace plc {
namespace core {

template <class R, class S = std::error_code>
using Result = libp2p::outcome::result<R, S>;

} // namespace core
} // namespace plc