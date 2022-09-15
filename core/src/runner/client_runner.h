#pragma once

#include <concepts>
#include <optional>

#include <boost/asio/io_service.hpp>

#include <cppcoro/task.hpp>

#include "utils/result.h"

namespace plc::core::runner {

class ClientRunner {
public:
    ClientRunner();

    void run();

    void post_func(std::invocable<> auto&& func) {
        _io_service->post(std::forward<decltype(func)>(func));
    }

    void dispatch_func(std::invocable<> auto&& func) {
        _io_service->dispatch(std::forward<decltype(func)>(func));
    }

    void post_task(cppcoro::task<void>&& task);

    std::shared_ptr<boost::asio::io_service> get_service();

private:
    std::shared_ptr<boost::asio::io_service> _io_service;
    std::optional<boost::asio::io_service::work> _work;
};

} // plc::core::runner
