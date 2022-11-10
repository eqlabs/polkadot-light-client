//
// Copyright (c) 2018 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/CppCon2018
//

#include "http_session.h"
#include "websocket_session.h"
#include "utils/ws_logger.h"


// Append an HTTP rel-path to a local filesystem path.
// The returned path is normalized for the platform.
std::string path_cat(
    boost::beast::string_view base,
    boost::beast::string_view path)
{
    if(base.empty())
        return path.to_string();
    std::string result = base.to_string();
#if BOOST_MSVC
    char constexpr path_separator = '\\';
    if(result.back() == path_separator)
        result.resize(result.size() - 1);
    result.append(path.data(), path.size());
    for(auto& c : result)
        if(c == '/')
            c = path_separator;
#else
    char constexpr path_separator = '/';
    if(result.back() == path_separator)
        result.resize(result.size() - 1);
    result.append(path.data(), path.size());
#endif
    return result;
}

// This function produces an HTTP response for the given
// request. The type of the response object depends on the
// contents of the request, so the interface requires the
// caller to pass a generic lambda for receiving the response.
template<
    class Body, class Allocator,
    class Send>
void
handle_request(
    boost::beast::string_view doc_root,
    http::request<Body, http::basic_fields<Allocator>>&& req,
    Send&& send)
{
    // Returns a bad request response
    auto const bad_request =
    [&req](boost::beast::string_view why)
    {
        http::response<http::string_body> res{http::status::bad_request, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.body() = why.to_string();
        res.prepare_payload();
        return res;
    };

    // Returns a not found response
    auto const not_found =
    [&req](boost::beast::string_view target)
    {
        http::response<http::string_body> res{http::status::not_found, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.body() = "The resource '" + target.to_string() + "' was not found.";
        res.prepare_payload();
        return res;
    };

    // Returns a server error response
    auto const server_error =
    [&req](boost::beast::string_view what)
    {
        http::response<http::string_body> res{http::status::internal_server_error, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.body() = "An error occurred: '" + what.to_string() + "'";
        res.prepare_payload();
        return res;
    };

    // Make sure we can handle the method
    if( req.method() != http::verb::get &&
        req.method() != http::verb::head)
        return send(bad_request("Unknown HTTP-method"));

    // Request path must be absolute and not contain "..".
    if( req.target().empty() ||
        req.target()[0] != '/' ||
        req.target().find("..") != boost::beast::string_view::npos)
        return send(bad_request("Illegal request-target"));

    // Build the path to the requested file
    std::string path = path_cat(doc_root, req.target());
    if(req.target().back() == '/')
        path.append("index.html");

    // Attempt to open the file
    boost::beast::error_code ec;
    http::file_body::value_type body;
    body.open(path.c_str(), boost::beast::file_mode::scan, ec);

    // Handle the case where the file doesn't exist
    if(ec == boost::system::errc::no_such_file_or_directory)
        return send(not_found(req.target()));

    // Handle an unknown error
    if(ec)
        return send(server_error(ec.message()));

    // Cache the size since we need it after the move
    auto const size = body.size();

    // Respond to HEAD request
    if(req.method() == http::verb::head) {
        http::response<http::empty_body> res{http::status::ok, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "application/text");
        res.content_length(size);
        res.keep_alive(req.keep_alive());
        return send(std::move(res));
    }

    // Respond to GET request
    http::response<http::file_body> res{
        std::piecewise_construct,
        std::make_tuple(std::move(body)),
        std::make_tuple(http::status::ok, req.version())};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "application/text");
    res.content_length(size);
    res.keep_alive(req.keep_alive());
    return send(std::move(res));
}

//------------------------------------------------------------------------------

http_session::http_session(
    tcp::socket socket)
    : m_socket(std::move(socket)) {
}

void http_session::run() {
    // Read a request
    http::async_read(m_socket, m_buffer, m_req,
        [self = shared_from_this()]
            (error_code ec, std::size_t bytes)
        {
            self->onRead(ec, bytes);
        });
}

// Report a failure
void http_session::fail(error_code ec, char const* what) {
    // Don't report on canceled operations
    if(ec == net::error::operation_aborted) {
        return;
    }

    plc::core::WsLogger::getLogger()->warn("http_session::fail {}: {}", what, ec.message());
}

void http_session::onRead(error_code ec, std::size_t) {
    // This means they closed the connection
    if(ec == http::error::end_of_stream) {
        m_socket.shutdown(tcp::socket::shutdown_send, ec);
        return;
    }

    // Handle the error, if any
    if(ec) {
        return fail(ec, "read");
    }

    // See if it is a WebSocket Upgrade
    if(websocket::is_upgrade(m_req)) {
        // Create a WebSocket session by transferring the socket
        plc::core::WsLogger::getLogger()->warn("http_session::onRead: upgrade to websocket");
        std::make_shared<websocket_session>(
            std::move(m_socket))->run(std::move(m_req));
        plc::core::WsLogger::getLogger()->warn("http_session::onRead: done upgrade to websocket");
        return;
    }

    // Send the response
    handle_request(".", std::move(m_req), [this](auto&& response) {
        // The lifetime of the message has to extend
        // for the duration of the async operation so
        // we use a shared_ptr to manage it.
        using response_type = typename std::decay<decltype(response)>::type;
        auto sp = std::make_shared<response_type>(std::forward<decltype(response)>(response));

        // Write the response
        auto self = shared_from_this();
        http::async_write(this->m_socket, *sp,
            [self, sp](error_code ec, std::size_t bytes) {
                self->onWrite(ec, bytes, sp->need_eof()); 
            });
    });
}

void http_session::onWrite(error_code ec, std::size_t, bool close) {
    // Handle the error, if any
    if(ec) {
        return fail(ec, "write");
    }

    if(close) {
        // This means we should close the connection, usually because
        // the response indicated the "Connection: close" semantic.
        m_socket.shutdown(tcp::socket::shutdown_send, ec);
        return;
    }

    // Clear contents of the request message,
    // otherwise the read behavior is undefined.
    m_req = {};

    // Read another request
    http::async_read(m_socket, m_buffer, m_req,
        [self = shared_from_this()]
            (error_code ec, std::size_t bytes)
        {
            self->onRead(ec, bytes);
        });
}
