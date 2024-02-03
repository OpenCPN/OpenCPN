/*
 *  IXWebSocketServerTest.cpp
 *  Author: Benjamin Sergeant
 *  Copyright (c) 2019 Machine Zone. All rights reserved.
 */

#include "IXTest.h"
#include "catch.hpp"
#include <iostream>
#include <ixwebsocket/IXSocket.h>
#include <ixwebsocket/IXSocketFactory.h>
#include <ixwebsocket/IXWebSocket.h>
#include <ixwebsocket/IXWebSocketServer.h>

using namespace ix;

namespace ix
{
    // Test that we can override the connectionState impl to provide our own
    class ConnectionStateCustom : public ConnectionState
    {
        void computeId()
        {
            // a very boring invariant id that we can test against in the unittest
            _id = "foobarConnectionId";
        }
    };

    bool startServer(ix::WebSocketServer& server, std::string& connectionId)
    {
        auto factory = []() -> std::shared_ptr<ConnectionState> {
            return std::make_shared<ConnectionStateCustom>();
        };
        server.setConnectionStateFactory(factory);

        server.setOnClientMessageCallback(
            [&server, &connectionId](std::shared_ptr<ConnectionState> connectionState,
                                     WebSocket& webSocket,
                                     const ix::WebSocketMessagePtr& msg) {
                auto remoteIp = connectionState->getRemoteIp();

                if (msg->type == ix::WebSocketMessageType::Open)
                {
                    TLogger() << "New connection";
                    connectionState->computeId();
                    TLogger() << "remote ip: " << remoteIp;
                    TLogger() << "id: " << connectionState->getId();
                    TLogger() << "Uri: " << msg->openInfo.uri;
                    TLogger() << "Headers:";
                    for (auto it : msg->openInfo.headers)
                    {
                        TLogger() << it.first << ": " << it.second;
                    }

                    connectionId = connectionState->getId();
                }
                else if (msg->type == ix::WebSocketMessageType::Close)
                {
                    TLogger() << "Closed connection";
                }
                else if (msg->type == ix::WebSocketMessageType::Message)
                {
                    for (auto&& client : server.getClients())
                    {
                        if (client.get() != &webSocket)
                        {
                            client->send(msg->str, msg->binary);
                        }
                    }
                }
            });

        auto res = server.listen();
        if (!res.first)
        {
            TLogger() << res.second;
            return false;
        }

        server.start();
        return true;
    }
} // namespace ix

TEST_CASE("Websocket_server", "[websocket_server]")
{
    SECTION("Connect to the server, do not send anything. Should timeout and return 400")
    {
        int port = getFreePort();
        ix::WebSocketServer server(port);
        std::string connectionId;
        REQUIRE(startServer(server, connectionId));

        std::string errMsg;
        bool tls = false;
        SocketTLSOptions tlsOptions;
        std::shared_ptr<Socket> socket = createSocket(tls, -1, errMsg, tlsOptions);
        std::string host("127.0.0.1");
        auto isCancellationRequested = []() -> bool { return false; };
        bool success = socket->connect(host, port, errMsg, isCancellationRequested);
        REQUIRE(success);

        auto lineResult = socket->readLine(isCancellationRequested);
        auto lineValid = lineResult.first;
        REQUIRE(lineValid);

        auto line = lineResult.second;

        int status = -1;
        REQUIRE(sscanf(line.c_str(), "HTTP/1.1 %d", &status) == 1);
        REQUIRE(status == 400);

        // FIXME: explicitely set a client timeout larger than the server one (3)

        // Give us 500ms for the server to notice that clients went away
        ix::msleep(500);
        server.stop();
        REQUIRE(server.getClients().size() == 0);
    }

    SECTION("Connect to the server. Send GET request without header. Should return 400")
    {
        int port = getFreePort();
        ix::WebSocketServer server(port);
        std::string connectionId;
        REQUIRE(startServer(server, connectionId));

        std::string errMsg;
        bool tls = false;
        SocketTLSOptions tlsOptions;
        std::shared_ptr<Socket> socket = createSocket(tls, -1, errMsg, tlsOptions);
        std::string host("127.0.0.1");
        auto isCancellationRequested = []() -> bool { return false; };
        bool success = socket->connect(host, port, errMsg, isCancellationRequested);
        REQUIRE(success);

        TLogger() << "writeBytes";
        socket->writeBytes("GET /\r\n", isCancellationRequested);

        auto lineResult = socket->readLine(isCancellationRequested);
        auto lineValid = lineResult.first;
        REQUIRE(lineValid);

        auto line = lineResult.second;

        int status = -1;
        REQUIRE(sscanf(line.c_str(), "HTTP/1.1 %d", &status) == 1);
        REQUIRE(status == 400);

        // FIXME: explicitely set a client timeout larger than the server one (3)

        // Give us 500ms for the server to notice that clients went away
        ix::msleep(500);
        server.stop();
        REQUIRE(server.getClients().size() == 0);
    }

    SECTION("Connect to the server. Send GET request with correct header")
    {
        int port = getFreePort();
        ix::WebSocketServer server(port);
        std::string connectionId;
        REQUIRE(startServer(server, connectionId));

        std::string errMsg;
        bool tls = false;
        SocketTLSOptions tlsOptions;
        std::shared_ptr<Socket> socket = createSocket(tls, -1, errMsg, tlsOptions);
        std::string host("127.0.0.1");
        auto isCancellationRequested = []() -> bool { return false; };
        bool success = socket->connect(host, port, errMsg, isCancellationRequested);
        REQUIRE(success);

        socket->writeBytes("GET / HTTP/1.1\r\n"
                           "Upgrade: websocket\r\n"
                           "Sec-WebSocket-Version: 13\r\n"
                           "Sec-WebSocket-Key: foobar\r\n"
                           "\r\n",
                           isCancellationRequested);

        auto lineResult = socket->readLine(isCancellationRequested);
        auto lineValid = lineResult.first;
        REQUIRE(lineValid);

        auto line = lineResult.second;

        int status = -1;
        REQUIRE(sscanf(line.c_str(), "HTTP/1.1 %d", &status) == 1);
        REQUIRE(status == 101);

        // Give us 500ms for the server to notice that clients went away
        ix::msleep(500);

        server.stop();
        REQUIRE(connectionId == "foobarConnectionId");
        REQUIRE(server.getClients().size() == 0);
    }
}
