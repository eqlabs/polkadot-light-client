#include "network/peer_manager.h"

#include <chrono>

#include <boost/asio/io_context.hpp>


#include <libp2p/basic/scheduler/scheduler_impl.hpp>
#include <libp2p/basic/scheduler/asio_scheduler_backend.hpp>
#include <libp2p/crypto/aes_ctr/aes_ctr_impl.hpp>
#include <libp2p/crypto/crypto_provider/crypto_provider_impl.hpp>
#include <libp2p/crypto/ecdsa_provider/ecdsa_provider_impl.hpp>
#include <libp2p/crypto/ed25519_provider/ed25519_provider_impl.hpp>
#include <libp2p/crypto/hmac_provider/hmac_provider_impl.hpp>
#include <libp2p/crypto/key_marshaller/key_marshaller_impl.hpp>
#include <libp2p/crypto/key_validator/key_validator_impl.hpp>
#include <libp2p/crypto/random_generator/boost_generator.hpp>
#include <libp2p/crypto/rsa_provider/rsa_provider_impl.hpp>
#include <libp2p/crypto/secp256k1_provider/secp256k1_provider_impl.hpp>
#include <libp2p/host/basic_host/basic_host.hpp>
#include <libp2p/network/impl/dialer_impl.hpp>
#include <libp2p/network/impl/dnsaddr_resolver_impl.hpp>
#include <libp2p/network/impl/connection_manager_impl.hpp>
#include <libp2p/network/cares/cares.hpp>
#include <libp2p/network/impl/listener_manager_impl.hpp>
#include <libp2p/network/impl/router_impl.hpp>
#include <libp2p/network/impl/transport_manager_impl.hpp>
#include <libp2p/network/impl/network_impl.hpp>
#include <libp2p/muxer/yamux/yamux.hpp>
#include <libp2p/peer/address_repository/inmem_address_repository.hpp>
#include <libp2p/peer/key_repository/inmem_key_repository.hpp>
#include <libp2p/peer/protocol_repository/inmem_protocol_repository.hpp>
#include <libp2p/peer/impl/identity_manager_impl.hpp>
#include <libp2p/peer/impl/peer_repository_impl.hpp>
#include <libp2p/protocol_muxer/multiselect.hpp>
#include <libp2p/protocol/kademlia/impl/content_routing_table_impl.hpp>
#include <libp2p/protocol/kademlia/impl/kademlia_impl.hpp>
#include <libp2p/protocol/kademlia/impl/peer_routing_table_impl.hpp>
#include <libp2p/protocol/kademlia/impl/storage_impl.hpp>
#include <libp2p/protocol/kademlia/impl/storage_backend_default.hpp>
#include <libp2p/protocol/kademlia/impl/validator_default.hpp>
#include <libp2p/protocol/identify/identify.hpp>
#include <libp2p/protocol/ping/ping.hpp>
#include <libp2p/security/noise.hpp>
#include <libp2p/transport/impl/upgrader_impl.hpp>
#include <libp2p/transport/tcp/tcp_transport.hpp>

#include <packio/packio.h>
#include <packio/extra/websocket.h>

#include "network/common/format_peer_id.h"
#include "network/grandpa/protocol.h"
#include "network/light2/protocol.h"
#include "utils/callback_to_coro.h"
#include "utils/propagate.h"
#include "utils/result.h"


// using packio::arg;
using packio::json_rpc::completion_handler;
// using packio::json_rpc::make_client;
using packio::json_rpc::make_server;
// using packio::json_rpc::rpc;

libp2p::log::Logger jsonrpcLogger;

namespace plc::core::network {
    // using packio::msgpack_rpc::make_client;
    // using packio::msgpack_rpc::make_server;

    using packio::net::ip::make_address;

    using awaitable_tcp_stream = decltype(packio::net::use_awaitable_t<>::as_default_on(
        std::declval<boost::beast::tcp_stream>()));
    using websocket = packio::extra::
        websocket_adapter<boost::beast::websocket::stream<awaitable_tcp_stream>, false>;
    using ws_acceptor =
        packio::extra::websocket_acceptor_adapter<packio::net::ip::tcp::acceptor, websocket>;

    
    void startJsonRpcServer(std::shared_ptr<runner::ClientRunner> runner) {
        jsonrpcLogger = libp2p::log::createLogger("JSON-RPC","network");
        auto io = runner->getService();

        std::string raw_ip_address = "127.0.0.1";
        // std::string raw_ip_address = "localhost";
        unsigned short port_num = 2584;

        // Used to store information about error that happens
        // while parsing the raw IP-address.
        boost::system::error_code ec;
        // Step 2. Using IP protocol version independent address
        // representation.
        boost::asio::ip::address ip_address =
        boost::asio::ip::address::from_string(raw_ip_address, ec);

        if (ec.value() != 0) {
            // Provided IP address is invalid. Breaking execution.
            std::cout 
            << "Failed to parse the IP address. Error code = "
            << ec.value() << ". Message: " << ec.message();
            return; // ec.value();
        }

        // Step 3.
        boost::asio::ip::tcp::endpoint bind_ep(ip_address, port_num);

        // auto server = make_server(packio::net::ip::tcp::acceptor{*io, bind_ep});
        auto server = make_server(ws_acceptor{*io, bind_ep});

        // auto client = make_client(packio::net::ip::tcp::socket{io});

        // Declare a synchronous callback with named arguments
        server->dispatcher()->add(
            "add", {"a", "b"}, [](int a, int b) -> int { 
                printf("add: a is %d, b is %d\n", a, b);
                return a + b; 
                });

        // server->dispatcher()->add_async(
        // "multiply", {"a", "b"}, [io](completion_handler complete, int a, int b) {
        //     // Call the completion handler later
        //         printf("multiply a is %d, b is %d\n", a, b);
        //     packio::net::post(
        //         *io, [a, b, complete = std::move(complete)]() mutable {
        //             complete(a * b);
        //         });
        // });

    server->dispatcher()->add_coro(
        "pow", *io, [](int a, int b) -> packio::net::awaitable<int> {
            printf("pow: a is %d, b is %d\n", a, b);
            co_return std::pow(a, b);
        });
    // server->dispatcher()->add_coro(
    //     "fibonacci", *io, [&](int n) -> packio::net::awaitable<int> {
    //         if (n <= 1) {
    //             co_return n;
    //         }

    //         // auto r1 = co_await client->async_call("fibonacci", std::tuple{n - 1});
    //         // auto r2 = co_await client->async_call("fibonacci", std::tuple{n - 2});

    //         co_return r1.result.as<int>() + r2.result.as<int>();
    //     });



        // Declare an asynchronous callback with named arguments
        server->dispatcher()->add_async(
            "multiply", {"a", "b"}, [io](completion_handler complete, int a, int b) {
                // Call the completion handler later
                printf("multiply a is %d, b is %d\n", a, b);
                packio::net::post(
                    *io, [a, b, complete = std::move(complete)]() mutable {
                        complete(a * b);
                    });
            });
        // Declare a coroutine with unnamed arguments
        server->dispatcher()->add_coro(
            "pow", *io, [](int a, int b) -> packio::net::awaitable<int> {
                printf("pow a is %d, b is %d\n", a, b);
                co_return std::pow(a, b);
            });

        // Connect the client
        // client->socket().connect(server->acceptor().local_endpoint());
        // Accept connections
        server->async_serve_forever();




    }


} // namespace plc::core::network
