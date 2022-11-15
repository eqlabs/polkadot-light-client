var WebSocket = require('ws');

const ws = new WebSocket('ws://127.0.0.1:2584');

ws.on('open', function open() {
  ws.send(process.argv[2]);
});

ws.on('message', function message(data) {
  console.log('received: %s', data);
  ws.close()
});
