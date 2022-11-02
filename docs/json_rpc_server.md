# JSON-RPC Server

Our JSON-RPC server uses the packio library:  `https://github.com/qchateau/packio`

This is a header-only library, which we have patched to work with Boost 1.80, as well as our own logger.  It is built on top of boost::asio, and therefor integrates easily into our library.

## Usage

To instantiate a server, you need a port and a Boost io_context, the latter being typically grabbed from the client runner.

```
auto json_rpc_server = std::make_shared<network::JsonRpcServer>(2584, runner->getService());
```

To add handlers, obtain the packio_server from our JSON-RPC server.

```
auto packio_server = json_rpc_server->getServer();
```

With this object, you can add message handlers, which may be synchronous, asynchronous, or coroutines.
This example shows the use of basic coroutines

```
packio_server->dispatcher()->add_coro(
    "add", *json_rpc_server->getIoService(), [](int a, int b) -> packio::net::awaitable<int> {
        printf("add: a is %d, b is %d\n", a, b);
        co_return a + b;
    });
packio_server->dispatcher()->add_coro(
    "multiply", *json_rpc_server->getIoService(), [](int a, int b) -> packio::net::awaitable<int> {
        printf("multiply: a is %d, b is %d\n", a, b);
        co_return a * b;
    });
packio_server->dispatcher()->add_coro(
    "pow", *json_rpc_server->getIoService(), [](int a, int b) -> packio::net::awaitable<int> {
        printf("pow: a is %d, b is %d\n", a, b);
        co_return std::pow(a, b);
    });

```

To test a running instance of the server, you can pass in a message to this test web client.  You would need to install the node libraries prior to use (`npm i`).

[demo node.js JSON-RPC client](../core/test/nodeutils/json-rpc-client.js)

For example, to perform an `add` using the handler above, you would call the app like this:

```
node json-rpc-client.js '{"id": 12, "method": "add", "params": [21,34]}'
```
The app writes the response to the console.
