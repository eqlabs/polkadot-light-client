#pragma once

#include <memory.h>

#include <boost/asio/io_service.hpp>

#include <cppcoro/task.hpp>

namespace plc::core::network {

cppcoro::task<void> handshake(std::shared_ptr<boost::asio::io_context> io_context);

} // namespace plc::core::network