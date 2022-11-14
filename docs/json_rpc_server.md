# JSON-RPC Server

Our JSON-RPC server is build on top of a boost websocket server. The http_session and websocket_session code is based on Vinnie Falco's demo project here: https://github.com/vinniefalco/CppCon2018


## Usage

To instantiate a server, you need a port and a Boost io_context, the latter being typically grabbed from our client runner.

```
auto jrpc_server = std::make_shared<network::json_rpc::JrpcServer>(2584, runner->getService());
jrpc_server->connect();

```

Although message parsing and handlers are not yet implemented, you can connect to websocket clients, receiving messages from said clients which will get printed to the console.

This is an example JavaScript node client that sends the same message every 5 seconds. Note that the port (2584) must match the port that the server is listening on.

```
var WebSocketClient = require('websocket').client;

var client = new WebSocketClient();

client.on('connectFailed', function(error) {
    console.log('Connect Error: ' + error.toString());
});

client.on('connect', function(connection) {
    console.log('WebSocket Client Connected');
    connection.on('error', function(error) {
        console.log("Connection Error: " + error.toString());
    });
    connection.on('close', function() {
        console.log('echo-protocol Connection Closed');
    });
    connection.on('message', function(message) {
        console.log('got message', message)
    });

    function sendMessage() {
        if (connection.connected) {
            let msg = {
                jsonrpc: '2.0',
                id: 42,
                method: 'multiply',
                params: [ 21, 12]
            };
            let msgtext = JSON.stringify(msg);
            console.log('sending message: ', msgtext);
            connection.send(msgtext);
        }
    }
    function doSomething() {
        sendMessage();
        setTimeout(doSomething, 5000);
    }
    
    setTimeout(doSomething, 5000);
});

client.connect('ws://127.0.0.1:2584');
```



