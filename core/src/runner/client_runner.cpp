#include "client_runner.h"

#include <cppcoro/async_manual_reset_event.hpp>

#include <boost/asio/deadline_timer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "utils/move_on_copy.h"
#include "utils/callback_to_coro.h"

namespace plc::core::runner {


ClientRunner::ClientRunner(std::shared_ptr<plc::core::StopHandler> stop_handler) noexcept : m_stop_handler(stop_handler),
m_io_service(std::make_shared<boost::asio::io_service>()) {
}

namespace {
struct FireAndForget {
    struct promise_type {
        FireAndForget get_return_object() const noexcept {
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
          auto logger = libp2p::log::createLogger("FireAndForget","network");
          logger->error("unhandled_exception");
          // Here I'm attempting a graceful exit
          // TODO : get access to the stop handler from client runner
          std::terminate();
        }
    };
};

// Run coroutine task that doesn't return a value.
// Assumption: task was NOT started for execution before.
// TODO: investigate if this can be checked in compile or run time
FireAndForget fireAndForget(cppcoro::task<void>&& task) noexcept {
    // store coroutine awaitable on the current stack
    auto local_task = std::move(task);
    co_await local_task;
}

} // namespace



void ClientRunner::run() noexcept {
    m_work.emplace(*m_io_service);

    m_io_service->run();
}

void ClientRunner::dispatchTask(cppcoro::task<void>&& task) noexcept {
    m_io_service->dispatch([task = MoveOnCopy<cppcoro::task<void>>{std::move(task)}]() mutable {
        fireAndForget(task.take());
    });
}

void ClientRunner::stop() noexcept {
    m_log->debug("client runner: stop");
    m_io_service->stop();
}

std::shared_ptr<boost::asio::io_service> ClientRunner::getService() const noexcept {
    return m_io_service;
}

PeriodicTimer ClientRunner::makePeriodicTimer(std::chrono::milliseconds interval,
    PeriodicTimer::Handler&& handler) const noexcept {
    return {*m_io_service, interval, std::move(handler)};
}

} // namespace plc::core::runner
