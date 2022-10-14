#pragma once

#include <scale/outcome/outcome.hpp>

namespace plc::core::network {

enum class ProtocolError {
    OwnerDestroyed = 1,
};

} // namespace plc::core::network

OUTCOME_HPP_DECLARE_ERROR(plc::core::network, ProtocolError);
