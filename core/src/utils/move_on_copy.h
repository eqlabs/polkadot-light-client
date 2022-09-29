#pragma once

#include <type_traits>

namespace plc::core {

// A helper class that performs a destructive copy (moves internal value instead).
// This is useful to pass move-only objects into boost handler functions which are passed
// by value in boost asio API.
template <typename T>
class MoveOnCopy {
public:
    template <typename U>
    MoveOnCopy(U&& val, std::enable_if_t<std::is_same_v<T, std::remove_const_t<U>>>* = nullptr)
        : _val(std::forward<U>(val)) {}
    MoveOnCopy(const MoveOnCopy& other): _val(std::move(other._val)) {}
    MoveOnCopy(MoveOnCopy&& other) = default;

    MoveOnCopy& operator=(const MoveOnCopy& other) {
        _val = std::move(other);
        return *this;
    }
    MoveOnCopy& operator=(MoveOnCopy&& other) = default;

    const T& get() const { return _val; }
    T& get() { return _val; }
    T&& take() { return std::move(_val); }

private:
    mutable T _val;
};

} // namespace plc::core
