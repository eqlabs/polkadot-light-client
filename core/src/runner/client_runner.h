#pragma once

#include <concepts>

#include <boost/asio/io_service.hpp>

#include <cppcoro/task.hpp>

#include "utils/result.h"

namespace plc {
namespace core {
namespace runner {

class ClientRunner {
public:
    void run();

    void post(std::invocable<> auto&& func) {
        //_io_service.post(std::forward<decltype(func)>(func));
    }

    void dispatch(std::invocable<> auto&& func) {
        //_io_service.dispatch(std::forward<decltype(func)>(func));
    }

private:
    boost::asio::io_service _io_service;
};

} // runner
} // core
} // namespace plc