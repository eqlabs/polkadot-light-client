#pragma once

//
// Copyright (c) 2018 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/CppCon2018
//



#include "http_session.h"

#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

#include "utils/ws_logger.h"

class websocket_session : public std::enable_shared_from_this<websocket_session>
{
    beast::flat_buffer m_buffer;
    websocket::stream<tcp::socket> m_ws;
    std::vector<std::shared_ptr<std::string const>> m_queue;

    void fail(error_code ec, char const* what);
    void onAccept(error_code ec);
    void onRead(error_code ec, std::size_t bytes_transferred);
    void onWrite(error_code ec, std::size_t bytes_transferred);

public:
    websocket_session(
        tcp::socket socket);
    ~websocket_session();
    template<class Body, class Allocator>
    void run(http::request<Body, http::basic_fields<Allocator>> req);
    void send(std::shared_ptr<std::string const> const& ss);
};

template<class Body, class Allocator>
void websocket_session::run(http::request<Body, http::basic_fields<Allocator>> req) {
    plc::core::WsLogger::getLogger()->warn("websocket_session::run");
    m_ws.async_accept(req,std::bind(
        &websocket_session::onAccept,
        shared_from_this(),
        std::placeholders::_1));
}
