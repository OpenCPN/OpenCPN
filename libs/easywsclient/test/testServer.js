// To run,
//
//   npm install # to install the 'ws' dependency
//   node testServer.js

"use strict"; // http://ejohn.org/blog/ecmascript-5-strict-mode-json-and-more/
var WebSocketServer = require('ws').Server;
var http = require('http');
var url = require('url');
var app = http.createServer();
var server;

var wssEchoWithSize = new WebSocketServer({ noServer: true });
wssEchoWithSize.on('connection', function(ws) {
    ws.on('message', function(data) {
        if (data instanceof Buffer) { return; }
        ws.send(data.length + '\n' + data);
    });
    ws.on('close', function() {
    });
    ws.on('error', function(e) {
    });
});

var wssBinaryEchoWithSize = new WebSocketServer({ noServer: true });
wssBinaryEchoWithSize.on('connection', function(ws) {
    ws.on('message', function(data) {
        if (!data instanceof Buffer) { return; }
        var result = new Buffer(data.length + 4);
        result.writeInt32BE(data.length, 0);
        data.copy(result, 4, 0, data.length);
        ws.send(result, { binary: true });
    });
    ws.on('close', function() {
    });
    ws.on('error', function(e) {
    });
});

var wssKillServer = new WebSocketServer({ noServer: true });
wssKillServer.on('connection', function(ws) {
    ws.on('message', function(data) {
        if (data instanceof Buffer) { return; }
        server.close();
    });
    ws.on('close', function() {
    });
    ws.on('error', function(e) {
    });
});

app.on('upgrade', function(request, socket, head) {
    var pathname = url.parse(request.url).pathname;

    if ('/echoWithSize' === pathname) {
        wssEchoWithSize.handleUpgrade(request, socket, head, function(ws) {
            wssEchoWithSize.emit('connection', ws, request);
        });
    }

    else if ('/binaryEchoWithSize' === pathname) {
        wssBinaryEchoWithSize.handleUpgrade(request, socket, head, function(ws) {
            wssBinaryEchoWithSize.emit('connection', ws, request);
        });
    }

    else if ('/killServer' === pathname) {
        wssKillServer.handleUpgrade(request, socket, head, function(ws) {
            wssKillServer.emit('connection', ws, request);
        });
    }

    else {
        socket.destroy();
    }
});

var PORT = parseInt(process.env.PORT || '8123');
server = app.listen(PORT);
console.log('Listening on port ' + PORT + '...');
