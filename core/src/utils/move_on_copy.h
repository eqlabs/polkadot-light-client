#pragma once

namespace plc::core {

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