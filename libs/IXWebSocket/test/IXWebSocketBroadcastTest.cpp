/*
 *  IXWebSocketServerTest.cpp
 *  Author: Benjamin Sergeant
 *  Copyright (c) 2019 Machine Zone. All rights reserved.
 */

#include "IXTest.h"
#include "catch.hpp"
#include "msgpack11.hpp"
#include <iostream>
#include <ixwebsocket/IXSocket.h>
#include <ixwebsocket/IXSocketFactory.h>
#include <ixwebsocket/IXWebSocket.h>
#include <ixwebsocket/IXWebSocketServer.h>

using msgpack11::MsgPack;
using namespace ix;

namespace
{
    class WebSocketBroadcastChat
    {
    public:
        WebSocketBroadcastChat(const std::string& user, const std::string& session, int port);

        void subscribe(const std::string& channel);
        void start();
        void stop();
        bool isReady() const;

        void sendMessage(const std::string& text);
        size_t getReceivedMessagesCount() const;
        const std::vector<std::string>& getReceivedMessages() const;

        std::string encodeMessage(const std::string& text);
        std::pair<std::string, std::string> decodeMessage(const std::string& str);
        void appendMessage(const std::string& message);

    private:
        std::string _user;
        std::string _session;
        int _port;

        ix::WebSocket _webSocket;

        std::vector<std::string> _receivedMessages;
        mutable std::mutex _mutex;
    };

    WebSocketBroadcastChat::WebSocketBroadcastChat(const std::string& user,
                                                   const std::string& session,
                                                   int port)
        : _user(user)
        , _session(session)
        , _port(port)
    {
        _webSocket.setTLSOptions(makeClientTLSOptions());
    }

    size_t WebSocketBroadcastChat::getReceivedMessagesCount() const
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _receivedMessages.size();
    }

    const std::vector<std::string>& WebSocketBroadcastChat::getReceivedMessages() const
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _receivedMessages;
    }

    void WebSocketBroadcastChat::appendMessage(const std::string& message)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _receivedMessages.push_back(message);
    }

    bool WebSocketBroadcastChat::isReady() const
    {
        return _webSocket.getReadyState() == ix::ReadyState::Open;
    }

    void WebSocketBroadcastChat::stop()
    {
        _webSocket.stop();
    }

    void WebSocketBroadcastChat::start()
    {
        //
        // Which server ??
        //
        std::string url;
        _webSocket.setUrl(url);

        std::stringstream ss;
        log(std::string("Connecting to url: ") + url);

        _webSocket.setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg) {
            std::stringstream ss;
            if (msg->type == ix::WebSocketMessageType::Open)
            {
                ss << "websocket_broadcast_client: " << _user << " Connected !";
                log(ss.str());
            }
            else if (msg->type == ix::WebSocketMessageType::Close)
            {
                ss << "websocket_broadcast_client: " << _user << " disconnected !";
                log(ss.str());
            }
            else if (msg->type == ix::WebSocketMessageType::Message)
            {
                auto result = decodeMessage(msg->str);

                // Our "chat" / "broacast" node.js server does not send us
                // the messages we send, so we don't need to have a msg_user != user
                // as we do for the satori chat example.

                // store text
                appendMessage(result.second);

                std::string payload = result.second;
                if (payload.size() > 2000)
                {
                    payload = "<message too large>";
                }

                ss << std::endl << result.first << " > " << payload << std::endl << _user << " > ";
                log(ss.str());
            }
            else if (msg->type == ix::WebSocketMessageType::Error)
            {
                ss << "websocket_broadcast_client: " << _user << " Error ! "
                   << msg->errorInfo.reason;
                log(ss.str());
            }
            else if (msg->type == ix::WebSocketMessageType::Ping)
            {
                log("websocket_broadcast_client: received ping message");
            }
            else if (msg->type == ix::WebSocketMessageType::Pong)
            {
                log("websocket_broadcast_client: received pong message");
            }
            else if (msg->type == ix::WebSocketMessageType::Fragment)
            {
                log("websocket_broadcast_client: received message fragment");
            }
            else
            {
                ss << "Unexpected ix::WebSocketMessageType";
                log(ss.str());
            }
        });

        _webSocket.start();
    }

    std::pair<std::string, std::string> WebSocketBroadcastChat::decodeMessage(
        const std::string& str)
    {
        std::string errMsg;
        MsgPack msg = MsgPack::parse(str, errMsg);

        std::string msg_user = msg["user"].string_value();
        std::string msg_text = msg["text"].string_value();

        return std::pair<std::string, std::string>(msg_user, msg_text);
    }

    std::string WebSocketBroadcastChat::encodeMessage(const std::string& text)
    {
        std::map<MsgPack, MsgPack> obj;
        obj["user"] = _user;
        obj["text"] = text;

        MsgPack msg(obj);

        std::string output = msg.dump();
        return output;
    }

    void WebSocketBroadcastChat::sendMessage(const std::string& text)
    {
        _webSocket.sendBinary(encodeMessage(text));
    }

    bool startServer(ix::WebSocketServer& server, std::string& connectionId)
    {
        bool preferTLS = true;
        server.setTLSOptions(makeServerTLSOptions(preferTLS));

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
} // namespace

TEST_CASE("Websocket_broadcast_server", "[websocket_server]")
{
    SECTION("Connect to the server, do not send anything. Should timeout and return 400")
    {
        int port = getFreePort();
        ix::WebSocketServer server(port);
        std::string connectionId;
        REQUIRE(startServer(server, connectionId));

        std::string session = ix::generateSessionId();
        std::vector<std::shared_ptr<WebSocketBroadcastChat>> chatClients;
        for (int i = 0; i < 10; ++i)
        {
            std::string user("user_" + std::to_string(i));
            chatClients.push_back(std::make_shared<WebSocketBroadcastChat>(user, session, port));
            chatClients[i]->start();
            ix::msleep(50);
        }

        // Wait for all chat instance to be ready
        while (true)
        {
            bool allReady = true;
            for (size_t i = 0; i < chatClients.size(); ++i)
            {
                allReady &= chatClients[i]->isReady();
            }
            if (allReady) break;
            ix::msleep(10);
        }

        for (int j = 0; j < 1000; j++)
        {
            for (size_t i = 0; i < chatClients.size(); ++i)
            {
                chatClients[i]->sendMessage("hello world");
            }

            if (j == 250)
            {
                server.stop();
                ix::msleep(100);
            }
            if (j == 500)
            {
                server.start();
                ix::msleep(100);
            }
        }

        // wait 1 second
        ix::msleep(2000);

        // Stop all clients
        size_t messageCount = chatClients.size() * 50;
        for (size_t i = 0; i < chatClients.size(); ++i)
        {
            REQUIRE(chatClients[i]->getReceivedMessagesCount() >= messageCount);
            chatClients[i]->stop();
        }

        // Give us 500ms for the server to notice that clients went away
        ix::msleep(500);
        server.stop();
        REQUIRE(server.getClients().size() == 0);
    }
}
