const { exit } = require('process');

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
        exit();
    });

    function sendMessage() {
        if (connection.connected) {
            console.log('sending message: ', process.argv[2])
            connection.send(process.argv[2]);
        }
    }
    sendMessage();
});

client.connect('ws://127.0.0.1:2584');
