/*
 *  IXWebSocketServer.cpp
 *  Author: Benjamin Sergeant, @marcelkauf
 *  Copyright (c) 2020 Machine Zone, Inc. All rights reserved.
 */

#include "IXTest.h"
#include "catch.hpp"
#include <ixwebsocket/IXWebSocket.h>
#include <ixwebsocket/IXWebSocketServer.h>
#include <memory>
#include <sstream>

using namespace ix;

namespace
{
    class WebSocketClient
    {
    public:
        WebSocketClient(int port);
        void start();
        void stop();
        bool isReady() const;
        bool hasConnectionError() const;

    private:
        ix::WebSocket _webSocket;
        int _port;
        std::atomic<bool> _connectionError;
    };

    WebSocketClient::WebSocketClient(int port)
        : _port(port)
        , _connectionError(false)
    {
    }

    bool WebSocketClient::hasConnectionError() const
    {
        return _connectionError;
    }

    bool WebSocketClient::isReady() const
    {
        return _webSocket.getReadyState() == ix::ReadyState::Open;
    }

    void WebSocketClient::stop()
    {
        _webSocket.stop();
    }

    void WebSocketClient::start()
    {
        std::string url;
        {
            std::stringstream ss;
            ss << "ws://localhost:" << _port << "/";

            url = ss.str();
        }

        _webSocket.setUrl(url);
        _webSocket.disableAutomaticReconnection();

        std::stringstream ss;
        log(std::string("Connecting to url: ") + url);

        _webSocket.setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg) {
            std::stringstream ss;
            if (msg->type == ix::WebSocketMessageType::Open)
            {
                log("client connected");
            }
            else if (msg->type == ix::WebSocketMessageType::Close)
            {
                log("client disconnected");
            }
            else if (msg->type == ix::WebSocketMessageType::Error)
            {
                _connectionError = true;
                log("error");
            }
            else if (msg->type == ix::WebSocketMessageType::Pong)
            {
                log("pong");
            }
            else if (msg->type == ix::WebSocketMessageType::Ping)
            {
                log("ping");
            }
            else if (msg->type == ix::WebSocketMessageType::Message)
            {
                log("message");
            }
            else
            {
                log("invalid type");
            }
        });

        _webSocket.start();
    }
} // namespace

TEST_CASE("Websocket leak test")
{
    SECTION("Websocket destructor is called when closing the connection.")
    {
        // stores the server websocket in order to check the use_count
        std::shared_ptr<WebSocket> webSocketPtr;

        {
            int port = getFreePort();
            WebSocketServer server(port);

            server.setOnConnectionCallback(
                [&webSocketPtr](std::shared_ptr<ix::WebSocket> webSocket,
                                std::shared_ptr<ConnectionState> connectionState,
                                std::unique_ptr<ConnectionInfo> connectionInfo) {
                    // original ptr in WebSocketServer::handleConnection and the callback argument
                    REQUIRE(webSocket.use_count() == 2);
                    webSocket->setOnMessageCallback([&webSocketPtr, webSocket, connectionState](
                                                        const ix::WebSocketMessagePtr& msg) {
                        if (msg->type == ix::WebSocketMessageType::Open)
                        {
                            log(std::string("New connection id: ") + connectionState->getId());
                            // original ptr in WebSocketServer::handleConnection, captured ptr of
                            // this callback, and ptr in WebSocketServer::_clients
                            REQUIRE(webSocket.use_count() == 3);
                            webSocketPtr = std::shared_ptr<WebSocket>(webSocket);
                            REQUIRE(webSocket.use_count() == 4);
                        }
                        else if (msg->type == ix::WebSocketMessageType::Close)
                        {
                            log(std::string("Client closed connection id: ") +
                                connectionState->getId());
                        }
                        else
                        {
                            log(std::string(msg->str));
                        }
                    });
                    // original ptr in WebSocketServer::handleConnection, argument of this callback,
                    // and captured ptr in websocket callback
                    REQUIRE(webSocket.use_count() == 3);
                });

            server.listen();
            server.start();

            WebSocketClient webSocketClient(port);
            webSocketClient.start();

            while (true)
            {
                REQUIRE(!webSocketClient.hasConnectionError());
                if (webSocketClient.isReady()) break;
                ix::msleep(10);
            }

            REQUIRE(server.getClients().size() == 1);
            // same value as in Open-handler above
            REQUIRE(webSocketPtr.use_count() == 4);

            ix::msleep(500);
            webSocketClient.stop();
            ix::msleep(500);
            REQUIRE(server.getClients().size() == 0);

            // websocket should only be referenced by webSocketPtr but is still used by the
            // websocket callback
            REQUIRE(webSocketPtr.use_count() == 1);
            webSocketPtr->setOnMessageCallback(nullptr);
            // websocket should only be referenced by webSocketPtr
            REQUIRE(webSocketPtr.use_count() == 1);
            server.stop();
        }
        // websocket should only be referenced by webSocketPtr
        REQUIRE(webSocketPtr.use_count() == 1);
    }
}
