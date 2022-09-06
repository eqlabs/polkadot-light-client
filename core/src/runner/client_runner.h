#pragma once

#include <concepts>

#include <boost/asio/io_service.hpp>

#include "utils/result.h"

namespace plc {
namespace core {
namespace runner {

class ClientRunner {
public:
    void run();

    auto spawn(std::invocable<> auto func) -> 

private:
    boost::asio::io_service _io_service;
};

} // runner
} // core
} // namespace plc