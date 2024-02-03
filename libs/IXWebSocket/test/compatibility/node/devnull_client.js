//
// With ws@7.3.1
// and 
// node --version
// v13.11.0
//
// In a different terminal, start a push server:
// $ ws push_server -q
//
// $ node devnull_client.js
// messages received per second: 16643
// messages received per second: 28065
// messages received per second: 28432
// messages received per second: 22207
// messages received per second: 28805
// messages received per second: 28694
// messages received per second: 28180
// messages received per second: 28601
// messages received per second: 28698
// messages received per second: 28931
// messages received per second: 27975
//
const WebSocket = require('ws');

const ws = new WebSocket('ws://localhost:8008');

ws.on('open', function open() {
  ws.send('hello from node');
});

var receivedMessages = 0;

setInterval(function timeout() {
  console.log(`messages received per second: ${receivedMessages}`)
  receivedMessages = 0;
}, 1000);

ws.on('message', function incoming(data) {
  receivedMessages += 1;
});


