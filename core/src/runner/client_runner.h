#pragma once

#include <concepts>
#include <optional>

#include <boost/asio/io_service.hpp>

#include <cppcoro/task.hpp>

#include "runner/timer.h"
#include "utils/result.h"
#include "utils/stoppable.h"

namespace plc::core::runner {

class ClientRunner final : public Stoppable {
public:
    ClientRunner(std::shared_ptr<plc::core::StopHandler> stop_handler) noexcept;

    void run() noexcept;
    void stop() noexcept override;

    void postFunc(std::invocable<> auto&& func) noexcept {
        m_io_service->post(std::forward<decltype(func)>(func));
    }
    void dispatchFunc(std::invocable<> auto&& func) noexcept {
        m_io_service->dispatch(std::forward<decltype(func)>(func));
    }

    // We have only the dispatch task method without `postTask`
    // because cppcoro::task always suspends before execution.
    // That makes it impossible to "capture" members to local variables in a task method:
    // task<void> someTask() {
    //     auto self = weak_from_this(); // won't be executed if we post task, not dispatch
    //     auto res = co_await someOperation();
    // }
    // When using `dispatchTask` we are executing task code up to the first suspension point.
    void dispatchTask(cppcoro::task<void>&& task) noexcept;

    std::shared_ptr<boost::asio::io_service> getService() const noexcept;

    PeriodicTimer makePeriodicTimer(std::chrono::milliseconds interval, PeriodicTimer::Handler&& handler) const noexcept;

private:
    // NOTE: some libp2p methods require shared pointers to service, that's why
    // we store it by shared pointer now
    std::shared_ptr<boost::asio::io_service> m_io_service;
    std::optional<boost::asio::io_service::work> m_work;
    libp2p::log::Logger m_log = libp2p::log::createLogger("ClientRunner","runner");
    std::shared_ptr<StopHandler> m_stop_handler;
};

} // plc::core::runner
