#include "client_runner.h"

#include <cppcoro/async_manual_reset_event.hpp>

#include <boost/asio/deadline_timer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "utils/move_on_copy.h"
#include "utils/callback_to_coro.h"

namespace plc::core::runner {


ClientRunner::ClientRunner() : _io_service(std::make_shared<boost::asio::io_service>()) {
}

namespace {
struct fire_and_forget_t {
    struct promise_type {
        fire_and_forget_t get_return_object() const noexcept {
          return {};
        }

        void return_void() const noexcept {}

        std::suspend_never initial_suspend() const noexcept {
          return {};
        }

        std::suspend_never final_suspend() const noexcept {
          return {};
        }

        void unhandled_exception() noexcept {
          // TODO: need error handling here
          std::terminate();
        }
    };
};

// Run coroutine task that doesn't return a value.
// Assumption: task was NOT started for execution before.
// TODO: investigate if this can be checked in compile or run time
fire_and_forget_t fire_and_forget(cppcoro::task<void>&& task) {
    // store coroutine awaitable on the current stack
    auto local_task = std::move(task);
    co_await local_task;
}

} // namespace

void ClientRunner::run() {
    // TODO: implement proper stop logic
    _work.emplace(*_io_service);

    _io_service->run();
}

void ClientRunner::post_task(cppcoro::task<void>&& task) {
    _io_service->post([task = MoveOnCopy<cppcoro::task<void>>{std::move(task)}]() mutable {
        fire_and_forget(task.take());
    });
}

std::shared_ptr<boost::asio::io_service> ClientRunner::get_service() {
    return _io_service;
}

} // namespace plc::core::runner
