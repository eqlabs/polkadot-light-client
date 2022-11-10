//
// Copyright (c) 2018 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/CppCon2018
//

#include "websocket_session.hpp"
#include "utils/ws_logger.h"


websocket_session::
websocket_session(
    tcp::socket socket)
    : ws_(std::move(socket))
{
    plc::core::WsLogger::getLogger()->warn("websocket_session::websocket_session");
}

websocket_session::
~websocket_session()
{
    // Remove this session from the list of active sessions
    // jkl state_->leave(*this);
    plc::core::WsLogger::getLogger()->warn("websocket_session::~websocket_session");
}

void
websocket_session::
fail(error_code ec, char const* what)
{
    plc::core::WsLogger::getLogger()->warn("websocket_session::fail");
    // Don't report these
    if( ec == net::error::operation_aborted ||
        ec == websocket::error::closed)
        return;

    std::cerr << what << ": " << ec.message() << "\n";
}

void
websocket_session::
on_accept(error_code ec)
{
    // Handle the error, if any
    plc::core::WsLogger::getLogger()->warn("websocket_session::on_accept");
    if(ec)
        return fail(ec, "accept");

    // Add this session to the list of active sessions
    // jkl state_->join(*this);

    // Read a message
    ws_.async_read(
        buffer_,
        [sp = shared_from_this()](
            error_code ec, std::size_t bytes)
        {
            sp->on_read(ec, bytes);
        });
}

void
websocket_session::
on_read(error_code ec, std::size_t size)
{
    plc::core::WsLogger::getLogger()->warn("websocket_session::on_accept");
    // Handle the error, if any
    if(ec)
        return fail(ec, "read");

    // Send to all connections
    // state_->send("JKL: " + beast::buffers_to_string(buffer_.data()));
    auto const ss = std::make_shared<std::string const>(std::move("JKL: " + beast::buffers_to_string(buffer_.data())));

    //for(auto session : sessions_)
    this->send(ss);

    // Clear the buffer
    buffer_.consume(buffer_.size());

    // Read another message
    ws_.async_read(
        buffer_,
        [sp = shared_from_this()](
            error_code ec, std::size_t bytes)
        {
            sp->on_read(ec, bytes);
        });
}

void
websocket_session::
send(std::shared_ptr<std::string const> const& ss)
{
    // Always add to queue
    queue_.push_back(ss);

    // Are we already writing?
    if(queue_.size() > 1)
        return;

    // We are not currently writing, so send this immediately
    ws_.async_write(
        net::buffer(*queue_.front()),
        [sp = shared_from_this()](
            error_code ec, std::size_t bytes)
        {
            sp->on_write(ec, bytes);
        });
}

void
websocket_session::
on_write(error_code ec, std::size_t size)
{
    plc::core::WsLogger::getLogger()->warn("websocket_session::on_write size {}", size);
    // Handle the error, if any
    if(ec)
        return fail(ec, "write");

    // Remove the string from the queue
    queue_.erase(queue_.begin());

    // Send the next message if any
    if(! queue_.empty())
        ws_.async_write(
            net::buffer(*queue_.front()),
            [sp = shared_from_this()](
                error_code ec, std::size_t bytes)
            {
                plc::core::WsLogger::getLogger()->warn("websocket_session::onwrite");
                sp->on_write(ec, bytes);
            });
}
