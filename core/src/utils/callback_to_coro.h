#pragma once

#include <concepts>
#include <coroutine>
#include <optional>

#include "utils/move_copy.h"

namespace plc::core {

namespace callback_awaitable_details {

template <typename Input>
auto input_lambda = [](Input) {};

template <typename Input>
using InputLambda = decltype(input_lambda<Input>);

template <typename T, typename CallbackArg>
concept CallbackMethod = std::is_invocable_v<T, InputLambda<CallbackArg>&&>;

template <typename Result, CallbackMethod<Result> F>
class CallbackAwaiter {
public:
    CallbackAwaiter(F&& func): m_func(std::forward<F>(func)) {}

    // Since we capture `this` in a callback we need to have a guarantee
    // that awaiter isn't moved or copied.
    PLC_DISABLE_COPY(CallbackAwaiter);
    PLC_DISABLE_MOVE(CallbackAwaiter);

    bool await_ready() { return false; }

    void await_suspend(std::coroutine_handle<> coroutine_handle) {
        m_func([this, coroutine_handle](Result r) {
            m_res = std::move(r);
            coroutine_handle();
        });
    }

    Result await_resume() {
        return std::move(*m_res);
    }

private:
    F m_func;
    // Since Result maybe not default-constructible we use std::optional here.
    // But maybe we can just use raw memory buffer in case when between `await_suspend`
    // and `await_resume` no exceptions happen.
    // Need to investigate more about coroutines workflow.
    std::optional<Result> m_res;
};

template <CallbackMethod<void> F>
struct CallbackAwaiter<void, F> {
    CallbackAwaiter(F&& f): f(std::forward<F>(f)) {}

    bool await_ready() { return false; }

    void await_suspend(std::coroutine_handle<> h) {
        f([this, h]() {
            h();
        });
    }

    void await_resume() {}

    F f;
};

} // namespace callback_awaitable_details

template <typename Result, callback_awaitable_details::CallbackMethod<Result> F>
[[nodiscard]] auto resumeInCallback(F&& func) {
    return callback_awaitable_details::CallbackAwaiter<Result, F>{std::forward<F>(func)};
}

} // namespace plc::core
