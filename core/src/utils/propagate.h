#pragma once

#include <concepts>

namespace plc::core {

// Utility methods for passing values as rvalue arguments
template <std::copy_constructible T>
T copy(const T& value) {
    return value;
}

template <std::move_constructible T>
T&& move(T& value) {
    return std::move(value);
}

} // namespace plc::core
