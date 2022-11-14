#pragma once

// Rewritten and refactored based on code from here:
// https://github.com/vinniefalco/CppCon2018
//
// Copyright (c) 2018 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)



#include <cstdlib>
#include <memory>
#include <boost/beast.hpp>
#include <boost/asio.hpp>



namespace beast = boost::beast;
namespace http = boost::beast::http;
namespace websocket = boost::beast::websocket;


namespace net = boost::asio;
using tcp = net::ip::tcp;
using error_code = boost::system::error_code;

namespace plc::core::network::json_rpc {

class HttpSession : public std::enable_shared_from_this<HttpSession>
{
    tcp::socket m_socket;
    beast::flat_buffer m_buffer;
    http::request<http::string_body> m_req;
    int m_id;

    void fail(error_code ec, char const* what);
    void onRead(error_code ec, std::size_t);
    void onWrite(error_code ec, std::size_t, bool close);

public:
    HttpSession(tcp::socket socket, int id);
    ~HttpSession();
    void run();
};

} // namespace plc::core::network::json_rpc