#pragma once

namespace plc::core {

// A helper class that performs a destructive copy (moves internal value instead).
// This is useful to pass move-only objects into boost handler functions which are passed
// by value in boost asio API.
template <typename T>
class MoveOnCopy {
public:
    template <typename ...Args>
    MoveOnCopy(Args&& ...args): _val(std::forward<Args>(args)...) {}
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