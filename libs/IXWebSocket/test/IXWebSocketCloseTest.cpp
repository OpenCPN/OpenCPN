/*
 *  IXWebSocketCloseTest.cpp
 *  Author: Alexandre Konieczny
 *  Copyright (c) 2019 Machine Zone. All rights reserved.
 */

#include "IXTest.h"
#include "catch.hpp"
#include <iostream>
#include <ixwebsocket/IXWebSocket.h>
#include <ixwebsocket/IXWebSocketServer.h>
#include <queue>
#include <sstream>

using namespace ix;

namespace
{
    class WebSocketClient
    {
    public:
        WebSocketClient(int port);

        void subscribe(const std::string& channel);
        void start();
        void stop();
        void stop(uint16_t code, const std::string& reason);
        bool isReady() const;

        uint16_t getCloseCode();
        const std::string& getCloseReason();
        bool getCloseRemote();

        bool hasConnectionError() const;

    private:
        ix::WebSocket _webSocket;
        int _port;

        mutable std::mutex _mutexCloseData;
        uint16_t _closeCode;
        std::string _closeReason;
        bool _closeRemote;
        std::atomic<bool> _connectionError;
    };

    WebSocketClient::WebSocketClient(int port)
        : _port(port)
        , _closeCode(0)
        , _closeReason(std::string(""))
        , _closeRemote(false)
        , _connectionError(false)
    {
        ;
    }

    bool WebSocketClient::hasConnectionError() const
    {
        return _connectionError;
    }

    bool WebSocketClient::isReady() const
    {
        return _webSocket.getReadyState() == ix::ReadyState::Open;
    }

    uint16_t WebSocketClient::getCloseCode()
    {
        std::lock_guard<std::mutex> lck(_mutexCloseData);

        return _closeCode;
    }

    const std::string& WebSocketClient::getCloseReason()
    {
        std::lock_guard<std::mutex> lck(_mutexCloseData);

        return _closeReason;
    }

    bool WebSocketClient::getCloseRemote()
    {
        std::lock_guard<std::mutex> lck(_mutexCloseData);

        return _closeRemote;
    }

    void WebSocketClient::stop()
    {
        _webSocket.stop();
    }

    void WebSocketClient::stop(uint16_t code, const std::string& reason)
    {
        _webSocket.stop(code, reason);
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
                std::stringstream ss;
                ss << "client disconnected(" << msg->closeInfo.code << "," << msg->closeInfo.reason
                   << ")";
                log(ss.str());

                std::lock_guard<std::mutex> lck(_mutexCloseData);

                _closeCode = msg->closeInfo.code;
                _closeReason = std::string(msg->closeInfo.reason);
                _closeRemote = msg->closeInfo.remote;
            }
            else if (msg->type == ix::WebSocketMessageType::Error)
            {
                _connectionError = true;
                ss << "Error ! " << msg->errorInfo.reason;
                log(ss.str());
            }
            else if (msg->type == ix::WebSocketMessageType::Pong)
            {
                ss << "Received pong message " << msg->str;
                log(ss.str());
            }
            else if (msg->type == ix::WebSocketMessageType::Ping)
            {
                ss << "Received ping message " << msg->str;
                log(ss.str());
            }
            else if (msg->type == ix::WebSocketMessageType::Message)
            {
                ss << "Received message " << msg->str;
                log(ss.str());
            }
            else
            {
                ss << "Invalid ix::WebSocketMessageType";
                log(ss.str());
            }
        });

        _webSocket.start();
    }

    bool startServer(ix::WebSocketServer& server,
                     uint16_t& receivedCloseCode,
                     std::string& receivedCloseReason,
                     bool& receivedCloseRemote,
                     std::mutex& mutexWrite)
    {
        // A dev/null server
        server.setOnClientMessageCallback(
            [&receivedCloseCode, &receivedCloseReason, &receivedCloseRemote, &mutexWrite](
                std::shared_ptr<ConnectionState> connectionState,
                WebSocket& /*webSocket*/,
                const ix::WebSocketMessagePtr& msg) {
                auto remoteIp = connectionState->getRemoteIp();
                if (msg->type == ix::WebSocketMessageType::Open)
                {
                    TLogger() << "New server connection";
                    TLogger() << "remote ip: " << remoteIp;
                    TLogger() << "id: " << connectionState->getId();
                    TLogger() << "Uri: " << msg->openInfo.uri;
                    TLogger() << "Headers:";
                    for (auto it : msg->openInfo.headers)
                    {
                        TLogger() << it.first << ": " << it.second;
                    }
                }
                else if (msg->type == ix::WebSocketMessageType::Close)
                {
                    std::stringstream ss;
                    ss << "Server closed connection(" << msg->closeInfo.code << ","
                       << msg->closeInfo.reason << ")";
                    log(ss.str());

                    std::lock_guard<std::mutex> lck(mutexWrite);

                    receivedCloseCode = msg->closeInfo.code;
                    receivedCloseReason = std::string(msg->closeInfo.reason);
                    receivedCloseRemote = msg->closeInfo.remote;
                }
            });

        auto res = server.listen();
        if (!res.first)
        {
            log(res.second);
            return false;
        }

        server.start();
        return true;
    }
} // namespace

TEST_CASE("Websocket_client_close_default", "[close]")
{
    SECTION("Make sure that close code and reason was used and sent to server.")
    {
        ix::setupWebSocketTrafficTrackerCallback();

        int port = getFreePort();
        ix::WebSocketServer server(port);

        uint16_t serverReceivedCloseCode(0);
        bool serverReceivedCloseRemote(false);
        std::string serverReceivedCloseReason("");
        std::mutex mutexWrite;

        REQUIRE(startServer(server,
                            serverReceivedCloseCode,
                            serverReceivedCloseReason,
                            serverReceivedCloseRemote,
                            mutexWrite));

        std::string session = ix::generateSessionId();
        WebSocketClient webSocketClient(port);

        webSocketClient.start();

        // Wait for all chat instance to be ready
        while (true)
        {
            REQUIRE(!webSocketClient.hasConnectionError());
            if (webSocketClient.isReady()) break;
            ix::msleep(10);
        }

        REQUIRE(server.getClients().size() == 1);

        ix::msleep(500);

        webSocketClient.stop();

        ix::msleep(500);

        // ensure client close is the same as values given
        REQUIRE(webSocketClient.getCloseCode() == 1000);
        REQUIRE(webSocketClient.getCloseReason() == "Normal closure");
        REQUIRE(webSocketClient.getCloseRemote() == false);

        {
            std::lock_guard<std::mutex> lck(mutexWrite);

            // Here we read the code/reason received by the server, and ensure that remote is true
            REQUIRE(serverReceivedCloseCode == 1000);
            REQUIRE(serverReceivedCloseReason == "Normal closure");
            REQUIRE(serverReceivedCloseRemote == true);
        }

        // Give us 1000ms for the server to notice that clients went away
        ix::msleep(1000);
        REQUIRE(server.getClients().size() == 0);

        ix::reportWebSocketTraffic();
    }
}

TEST_CASE("Websocket_client_close_params_given", "[close]")
{
    SECTION("Make sure that close code and reason was used and sent to server.")
    {
        ix::setupWebSocketTrafficTrackerCallback();

        int port = getFreePort();
        ix::WebSocketServer server(port);

        uint16_t serverReceivedCloseCode(0);
        bool serverReceivedCloseRemote(false);
        std::string serverReceivedCloseReason("");
        std::mutex mutexWrite;

        REQUIRE(startServer(server,
                            serverReceivedCloseCode,
                            serverReceivedCloseReason,
                            serverReceivedCloseRemote,
                            mutexWrite));

        std::string session = ix::generateSessionId();
        WebSocketClient webSocketClient(port);

        webSocketClient.start();

        // Wait for all chat instance to be ready
        while (true)
        {
            if (webSocketClient.isReady()) break;
            ix::msleep(10);
        }

        REQUIRE(server.getClients().size() == 1);

        ix::msleep(500);

        webSocketClient.stop(4000, "My reason");

        ix::msleep(500);

        // ensure client close is the same as values given
        REQUIRE(webSocketClient.getCloseCode() == 4000);
        REQUIRE(webSocketClient.getCloseReason() == "My reason");
        REQUIRE(webSocketClient.getCloseRemote() == false);

        {
            std::lock_guard<std::mutex> lck(mutexWrite);

            // Here we read the code/reason received by the server, and ensure that remote is true
            REQUIRE(serverReceivedCloseCode == 4000);
            REQUIRE(serverReceivedCloseReason == "My reason");
            REQUIRE(serverReceivedCloseRemote == true);
        }

        // Give us 1000ms for the server to notice that clients went away
        ix::msleep(1000);
        REQUIRE(server.getClients().size() == 0);

        ix::reportWebSocketTraffic();
    }
}

TEST_CASE("Websocket_server_close", "[close]")
{
    SECTION("Make sure that close code and reason was read from server.")
    {
        ix::setupWebSocketTrafficTrackerCallback();

        int port = getFreePort();
        ix::WebSocketServer server(port);

        uint16_t serverReceivedCloseCode(0);
        bool serverReceivedCloseRemote(false);
        std::string serverReceivedCloseReason("");
        std::mutex mutexWrite;

        REQUIRE(startServer(server,
                            serverReceivedCloseCode,
                            serverReceivedCloseReason,
                            serverReceivedCloseRemote,
                            mutexWrite));

        std::string session = ix::generateSessionId();
        WebSocketClient webSocketClient(port);

        webSocketClient.start();

        // Wait for all chat instance to be ready
        while (true)
        {
            if (webSocketClient.isReady()) break;
            ix::msleep(10);
        }

        REQUIRE(server.getClients().size() == 1);

        ix::msleep(500);

        server.stop();

        ix::msleep(500);

        // ensure client close is the same as values given
        REQUIRE(webSocketClient.getCloseCode() == 1000);
        REQUIRE(webSocketClient.getCloseReason() == "Normal closure");
        REQUIRE(webSocketClient.getCloseRemote() == true);

        {
            std::lock_guard<std::mutex> lck(mutexWrite);

            // Here we read the code/reason received by the server, and ensure that remote is true
            REQUIRE(serverReceivedCloseCode == 1000);
            REQUIRE(serverReceivedCloseReason == "Normal closure");
            REQUIRE(serverReceivedCloseRemote == false);
        }

        // Give us 1000ms for the server to notice that clients went away
        ix::msleep(1000);
        REQUIRE(server.getClients().size() == 0);

        ix::reportWebSocketTraffic();
    }
}

TEST_CASE("Websocket_server_close_immediatly", "[close]")
{
    SECTION("Make sure that close code and reason was read from server.")
    {
        ix::setupWebSocketTrafficTrackerCallback();

        int port = getFreePort();
        ix::WebSocketServer server(port);

        uint16_t serverReceivedCloseCode(0);
        bool serverReceivedCloseRemote(false);
        std::string serverReceivedCloseReason("");
        std::mutex mutexWrite;

        REQUIRE(startServer(server,
                            serverReceivedCloseCode,
                            serverReceivedCloseReason,
                            serverReceivedCloseRemote,
                            mutexWrite));

        std::string session = ix::generateSessionId();
        WebSocketClient webSocketClient(port);

        webSocketClient.start();

        server.stop();

        ix::msleep(500);

        // ensure client close hasn't been called
        REQUIRE(webSocketClient.getCloseCode() == 0);
        REQUIRE(webSocketClient.getCloseReason() == "");
        REQUIRE(webSocketClient.getCloseRemote() == false);

        {
            std::lock_guard<std::mutex> lck(mutexWrite);

            // Here we ensure that the code/reason wasn't received by the server
            REQUIRE(serverReceivedCloseCode == 0);
            REQUIRE(serverReceivedCloseReason == "");
            REQUIRE(serverReceivedCloseRemote == false);
        }

        // Give us 1000ms for the server to notice that clients went away
        ix::msleep(1000);
        REQUIRE(server.getClients().size() == 0);

        ix::reportWebSocketTraffic();
    }
}
