// Rewritten and refactored based on code from here:
// https://github.com/vinniefalco/CppCon2018
//
// Copyright (c) 2018 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#include "websocket_session.h"

std::function<void(int, std::shared_ptr<WebSocketSession>)> WebSocketSession::onOpen;
std::function<void(int, std::string message)> WebSocketSession::onMessage;
std::function<void(int)> WebSocketSession::onClose;

WebSocketSession::WebSocketSession(tcp::socket socket, int id)
    : m_ws(std::move(socket))
    , m_id(id) {
    m_log->warn("WebSocketSession::WebSocketSession");
}

WebSocketSession::~WebSocketSession() {
    m_log->warn("WebSocketSession::~WebSocketSession");
    onClose(m_id);
}

void WebSocketSession::fail(error_code ec, char const* what) {
    m_log->warn("WebSocketSession::fail what {}", what);
    // Don't report these
    if( ec == net::error::operation_aborted ) {
        m_log->warn("WebSocketSession::fail: net::error::operation_aborted");
        return;
    }
    if( ec == websocket::error::closed) {
        m_log->warn("WebSocketSession::fail: websocket::error::closed");
        return;
    }
    if( ec == net::error::operation_aborted ||
        ec == websocket::error::closed) {
        return;
    }
}

void WebSocketSession::onAccept(error_code ec){
    m_log->warn("WebSocketSession::onAccept");
    // Handle the error, if any
    if(ec) {
        return fail(ec, "accept");
    }

    onOpen(m_id, shared_from_this());

    // Read a message
    m_ws.async_read(m_buffer,[sp = shared_from_this()](
            error_code ec, std::size_t bytes) {
            sp->onRead(ec, bytes);
        });
}

void WebSocketSession::onRead(error_code ec, std::size_t size) {
    m_log->warn("WebSocketSession::onAccept");
    // Handle the error, if any
    if(ec) {
        return fail(ec, "read");
    }

    // pass the message up to the server
    const std::string ss = beast::buffers_to_string(m_buffer.data());
    onMessage(m_id, ss);

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

void WebSocketSession::send(std::shared_ptr<std::string const> const& ss) {
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

void WebSocketSession::onWrite(error_code ec, std::size_t size) {
    m_log->warn("WebSocketSession::onWrite size {}", size);
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
            [sp = shared_from_this(), logger = m_log](
                error_code ec, std::size_t bytes) {
                logger->warn("WebSocketSession::onwrite");
                sp->onWrite(ec, bytes);
            });
    }
}
