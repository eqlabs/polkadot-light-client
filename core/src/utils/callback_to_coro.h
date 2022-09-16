#pragma once

#include <coroutine>
#include <optional>

namespace plc::core {

template <typename Result, typename F>
struct CallbackAwaiter {
    CallbackAwaiter(F&& f): f(std::forward<F>(f)) {}

    bool await_ready() { return false; }

    void await_suspend(std::coroutine_handle<> h) {
        f([this, h](Result r) {
            res = r;
            h();
        });
    }

    Result await_resume() {
        return std::move(*res);
    }

    F f;
    // Since Result maybe not default-constructible we use std::optional here.
    // But maybe we can just use raw memory buffer in case when between `await_suspend`
    // and `await_resume` no exceptions happen.
    // Need to investigate more about coroutines workflow.
    std::optional<Result> res;
};

template <typename F>
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

template <typename Result, typename F>
auto resume_in_callback(F&& f) {
    return CallbackAwaiter<Result, F>{std::forward<F>(f)};
}

} // namespace plc::core
