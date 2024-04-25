/*
 *  IXTest.cpp
 *  Author: Benjamin Sergeant
 *  Copyright (c) 2018 Machine Zone. All rights reserved.
 */

#include "IXTest.h"

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXUuid.h>
#include <ixwebsocket/IXWebSocket.h>
#include <mutex>
#include <random>
#include <stack>
#include <stdlib.h>
#include <string>
#include <thread>


namespace ix
{
    std::atomic<size_t> incomingBytes(0);
    std::atomic<size_t> outgoingBytes(0);
    std::mutex TLogger::_mutex;
    std::stack<int> freePorts;

    void setupWebSocketTrafficTrackerCallback()
    {
        ix::WebSocket::setTrafficTrackerCallback([](size_t size, bool incoming) {
            if (incoming)
            {
                incomingBytes += size;
            }
            else
            {
                outgoingBytes += size;
            }
        });
    }

    void reportWebSocketTraffic()
    {
        TLogger() << incomingBytes;
        TLogger() << "Incoming bytes: " << incomingBytes;
        TLogger() << "Outgoing bytes: " << outgoingBytes;
    }

    void msleep(int ms)
    {
        std::chrono::duration<double, std::milli> duration(ms);
        std::this_thread::sleep_for(duration);
    }

    std::string generateSessionId()
    {
        auto now = std::chrono::system_clock::now();
        auto seconds =
            std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();

        return std::to_string(seconds);
    }

    void log(const std::string& msg)
    {
        TLogger() << msg;
    }

    void hexDump(const std::string& prefix, const std::string& s)
    {
        std::ostringstream ss;
        bool upper_case = false;

        for (std::string::size_type i = 0; i < s.length(); ++i)
        {
            ss << std::hex << std::setfill('0') << std::setw(2)
               << (upper_case ? std::uppercase : std::nouppercase) << (int) s[i];
        }

        std::cout << prefix << ": " << s << " => " << ss.str() << std::endl;
    }

    bool startWebSocketEchoServer(ix::WebSocketServer& server)
    {
        server.setOnClientMessageCallback(
            [&server](std::shared_ptr<ConnectionState> connectionState,
                      WebSocket& webSocket,
                      const ix::WebSocketMessagePtr& msg) {
                auto remoteIp = connectionState->getRemoteIp();
                if (msg->type == ix::WebSocketMessageType::Open)
                {
                    TLogger() << "New connection";
                    TLogger() << "Remote ip: " << remoteIp;
                    TLogger() << "Uri: " << msg->openInfo.uri;
                    TLogger() << "Headers:";
                    for (auto it : msg->openInfo.headers)
                    {
                        TLogger() << it.first << ": " << it.second;
                    }
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

    std::vector<uint8_t> load(const std::string& path)
    {
        std::vector<uint8_t> memblock;

        std::ifstream file(path);
        if (!file.is_open()) return memblock;

        file.seekg(0, file.end);
        std::streamoff size = file.tellg();
        file.seekg(0, file.beg);

        memblock.reserve((size_t) size);
        memblock.insert(
            memblock.begin(), std::istream_iterator<char>(file), std::istream_iterator<char>());

        return memblock;
    }

    std::string readAsString(const std::string& path)
    {
        auto vec = load(path);
        return std::string(vec.begin(), vec.end());
    }

    SocketTLSOptions makeClientTLSOptions()
    {
        SocketTLSOptions tlsOptionsClient;
        tlsOptionsClient.certFile = ".certs/trusted-client-crt.pem";
        tlsOptionsClient.keyFile = ".certs/trusted-client-key.pem";
        tlsOptionsClient.caFile = ".certs/trusted-ca-crt.pem";

        return tlsOptionsClient;
    }

    SocketTLSOptions makeServerTLSOptions(bool preferTLS)
    {
        // Start a fake sentry http server
        SocketTLSOptions tlsOptionsServer;
        tlsOptionsServer.certFile = ".certs/trusted-server-crt.pem";
        tlsOptionsServer.keyFile = ".certs/trusted-server-key.pem";
        tlsOptionsServer.caFile = ".certs/trusted-ca-crt.pem";

#if defined(IXWEBSOCKET_USE_MBED_TLS) || defined(IXWEBSOCKET_USE_OPEN_SSL)
        tlsOptionsServer.tls = preferTLS;
#else
        tlsOptionsServer.tls = false;
#endif
        return tlsOptionsServer;
    }

    std::string getHttpScheme()
    {
#if defined(IXWEBSOCKET_USE_MBED_TLS) || defined(IXWEBSOCKET_USE_OPEN_SSL)
        std::string scheme("https://");
#else
        std::string scheme("http://");
#endif
        return scheme;
    }

    std::string getWsScheme(bool preferTLS)
    {
        std::string scheme;
#if defined(IXWEBSOCKET_USE_MBED_TLS) || defined(IXWEBSOCKET_USE_OPEN_SSL)
        if (preferTLS)
        {
            scheme = "wss://";
        }
        else
        {
            scheme = "ws://";
        }
#else
        scheme = "ws://";
#endif
        return scheme;
    }
} // namespace ix
