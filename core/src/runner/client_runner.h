#pragma once

#include <concepts>
#include <optional>

#include <boost/asio/io_service.hpp>

#include <cppcoro/task.hpp>

#include "utils/result.h"

namespace plc::core::runner {

class ClientRunner {
public:
    ClientRunner() noexcept;

    void run() noexcept;

    void post_func(std::invocable<> auto&& func) noexcept {
        m_io_service->post(std::forward<decltype(func)>(func));
    }

    void dispatch_func(std::invocable<> auto&& func) noexcept {
        m_io_service->dispatch(std::forward<decltype(func)>(func));
    }

    void post_task(cppcoro::task<void>&& task) noexcept;

    std::shared_ptr<boost::asio::io_service> get_service() const noexcept;

private:
    // NOTE: some libp2p methods require shared pointers to service, that's why
    // we store it by shared pointer now
    std::shared_ptr<boost::asio::io_service> m_io_service;
    std::optional<boost::asio::io_service::work> m_work;
};

} // plc::core::runner
