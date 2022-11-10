#pragma once

//
// Copyright (c) 2018 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/CppCon2018
//

#include <memory>
#include <string>
#include "http_session.h"

// Forward declaration

// Accepts incoming connections and launches the sessions
class listener : public std::enable_shared_from_this<listener>
{
    tcp::acceptor acceptor_;
    tcp::socket socket_;

    void fail(error_code ec, char const* what);
    void on_accept(error_code ec);

public:
    listener(
        net::io_context& ioc,
        tcp::endpoint endpoint);

    // Start accepting incoming connections
    void run();
};

