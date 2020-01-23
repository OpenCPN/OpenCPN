/*
  Prerequisites:

    1. Install node.js and npm
    2. npm install ws

  See also,

    http://einaros.github.com/ws/

  To run,

    node example-server.js
*/

"use strict"; // http://ejohn.org/blog/ecmascript-5-strict-mode-json-and-more/
var WebSocketServer = require('ws').Server;
var http = require('http');

var server = http.createServer();
var wss = new WebSocketServer({server: server, path: '/foo'});
wss.on('connection', function(ws) {
    console.log('/foo connected');
    ws.on('message', function(data, flags) {
        if (flags.binary) { return; }
        console.log('>>> ' + data);
        if (data == 'goodbye') { console.log('<<< galaxy'); ws.send('galaxy'); }
        if (data == 'hello') { console.log('<<< world'); ws.send('world'); }
    });
    ws.on('close', function() {
      console.log('Connection closed!');
    });
    ws.on('error', function(e) {
    });
});
server.listen(8126);
console.log('Listening on port 8126...');
