//
// Copyright (c) 2018 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/CppCon2018
//

#include "websocket_session.h"
#include "utils/ws_logger.h"


websocket_session::websocket_session(
    tcp::socket socket)
    : m_ws(std::move(socket)) {
    plc::core::WsLogger::getLogger()->warn("websocket_session::websocket_session");
}

websocket_session::~websocket_session() {
    // Remove this session from the list of active sessions
    // jkl state_->leave(*this);
    plc::core::WsLogger::getLogger()->warn("websocket_session::~websocket_session");
}

void websocket_session::fail(error_code ec, char const* what) {
    plc::core::WsLogger::getLogger()->warn("websocket_session::fail");
    // Don't report these
    if( ec == net::error::operation_aborted ||
        ec == websocket::error::closed) {
        return;
    }

    // std::cerr << what << ": " << ec.message() << "\n";
    plc::core::WsLogger::getLogger()->warn("websocket_session::fail {}: {}", what, ec.message());
}

void websocket_session::onAccept(error_code ec){
    // Handle the error, if any
    plc::core::WsLogger::getLogger()->warn("websocket_session::onAccept");
    if(ec) {
        return fail(ec, "accept");
    }

    // Add this session to the list of active sessions
    // jkl state_->join(*this);

    // Read a message
    m_ws.async_read(m_buffer,[sp = shared_from_this()](
            error_code ec, std::size_t bytes) {
            sp->onRead(ec, bytes);
        });
}

void websocket_session::onRead(error_code ec, std::size_t size) {
    plc::core::WsLogger::getLogger()->warn("websocket_session::onAccept");
    // Handle the error, if any
    if(ec) {
        return fail(ec, "read");
    }

    // Send to all connections
    auto const ss = std::make_shared<std::string const>(std::move("JKL: " + beast::buffers_to_string(m_buffer.data())));

    //for(auto session : sessions_)
    this->send(ss);

    // Clear the buffer
    m_buffer.consume(m_buffer.size());

    // Read another message
    m_ws.async_read(
        m_buffer,
        [sp = shared_from_this()](
            error_code ec, std::size_t bytes)
        {
            sp->onRead(ec, bytes);
        });
}

void websocket_session::send(std::shared_ptr<std::string const> const& ss) {
    // Always add to queue
    m_queue.push_back(ss);

    // Are we already writing?
    if(m_queue.size() > 1) {
        return;
    }

    // We are not currently writing, so send this immediately
    m_ws.async_write(
        net::buffer(*m_queue.front()),
        [sp = shared_from_this()](
            error_code ec, std::size_t bytes) {
            sp->onWrite(ec, bytes);
        });
}

void websocket_session::onWrite(error_code ec, std::size_t size) {
    plc::core::WsLogger::getLogger()->warn("websocket_session::onWrite size {}", size);
    // Handle the error, if any
    if(ec) {
        return fail(ec, "write");
    }

    // Remove the string from the queue
    m_queue.erase(m_queue.begin());

    // Send the next message if any
    if(!m_queue.empty()) {
        m_ws.async_write(
            net::buffer(*m_queue.front()),
            [sp = shared_from_this()](
                error_code ec, std::size_t bytes)
            {
                plc::core::WsLogger::getLogger()->warn("websocket_session::onwrite");
                sp->onWrite(ec, bytes);
            });
    }
}
