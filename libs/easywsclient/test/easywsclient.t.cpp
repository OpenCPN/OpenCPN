#include "easywsclient.hpp"
#ifdef _WIN32
#pragma comment( lib, "ws2_32" )
#include <WinSock2.h>
#endif
#include <assert.h>
#include <stdio.h>
#include <string>
#include <memory>
#include <vector>
#include <utility>
#include <iostream>
#include <sstream>
#include <gtest/gtest.h>

using easywsclient::WebSocket;

namespace {

#ifdef _WIN32
class WSAInit
{
  public:
    WSAInit()
    {
        INT rc;
        WSADATA wsaData;
        rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (rc) {
            throw std::runtime_error("WSAStartup failed");
        }
    }
    ~WSAInit()
    {
        WSACleanup();
    }
};
#endif

class KillServer
{
  public:
    KillServer() : d_ws(WebSocket::from_url("ws://localhost:8123/killServer"))
    {
        assert(d_ws);
    }
    ~KillServer()
    {
        d_ws->send("exit"); // sending any message instructs the server to die
        d_ws->poll();
        d_ws->poll();
    }
  private:
    std::unique_ptr<WebSocket> d_ws;
};

std::string makeString(size_t length)
{
    std::vector<char> v;
    v.reserve(length + 1);
    for (size_t i = 0; i < length; ++i) {
        v.push_back('0' + (i % 10));
    }
    v.push_back(0);
    return &v[0];
}

}

TEST(easywsclient, textFramesWork)
{

    std::unique_ptr<WebSocket> ws(WebSocket::from_url("ws://localhost:8123/echoWithSize"));
    assert(ws);
    ws->send("four");
    std::string message;
    while (ws->getReadyState() != WebSocket::CLOSED) {
        bool gotMessage = false;
        ws->poll();
        ws->dispatch([gotMessageOut=&gotMessage, messageOut=&message, ws=ws.get()](const std::string& message) {
            *gotMessageOut = true;
            *messageOut = message;
        });
        if (gotMessage) {
            break;
        }
    }
    ASSERT_EQ("4\nfour", message);
    ws->close(); // hmmm... shouldn't this be RAII?
}

TEST(easywsclient, longTextFramesWork)
{
    std::unique_ptr<WebSocket> ws(WebSocket::from_url("ws://localhost:8123/echoWithSize"));
    assert(ws);
    std::vector<std::pair<std::string, std::string> > v;
    v.emplace_back(    "0",     makeString(0));
    v.emplace_back(    "1",     makeString(1));
    v.emplace_back(  "123",   makeString(123));
    v.emplace_back(  "124",   makeString(124));
    v.emplace_back(  "125",   makeString(125));
    v.emplace_back(  "126",   makeString(126));
    v.emplace_back(  "127",   makeString(127));
    v.emplace_back(  "128",   makeString(128));
    v.emplace_back("10000", makeString(10000));
    v.emplace_back("32767", makeString(32767));
    v.emplace_back("65500", makeString(65500));
    v.emplace_back("65529", makeString(65529));
    v.emplace_back("65530", makeString(65530));
    v.emplace_back("65531", makeString(65531));
    v.emplace_back("65532", makeString(65532));
    v.emplace_back("65533", makeString(65533));
    v.emplace_back("65534", makeString(65534));
    v.emplace_back("65535", makeString(65535));
    v.emplace_back("65536", makeString(65536));
    v.emplace_back("65537", makeString(65537));
    for (auto i = v.begin(); i != v.end(); ++i) {
        ws->send(i->second);
        std::string message;
        while (ws->getReadyState() != WebSocket::CLOSED) {
            bool gotMessage = false;
            ws->poll();
            ws->dispatch([gotMessageOut=&gotMessage, messageOut=&message, ws=ws.get()](const std::string& message) {
                *gotMessageOut = true;
                *messageOut = message;
            });
            if (gotMessage) {
                break;
            }
        }
        ASSERT_EQ(i->first + "\n" + i->second, message);
    }
    ws->close(); // hmmm... shouldn't this be RAII?
}

TEST(easywsclient, binaryFramesWork)
{
    std::unique_ptr<WebSocket> ws(WebSocket::from_url("ws://localhost:8123/binaryEchoWithSize"));
    assert(ws);
    ws->sendBinary(std::vector<uint8_t>({1, 2, 3}));
    std::vector<uint8_t> message;
    while (ws->getReadyState() != WebSocket::CLOSED) {
        bool gotMessage = false;
        ws->poll();
        ws->dispatchBinary([gotMessageOut=&gotMessage, messageOut=&message, ws=ws.get()](const std::vector<uint8_t>& message) {
            *gotMessageOut = true;
            *messageOut = message;
        });
        if (gotMessage) {
            break;
        }
    }
    ASSERT_EQ(std::vector<uint8_t>({0, 0, 0, 3, 1, 2, 3}), message);
    ws->close(); // hmmm... shouldn't this be RAII?
}

int main(int argc, char **argv)
{
#ifdef _WIN32
    WSAInit wsaInit;
#endif
    KillServer killServer; // RAII to ensure server gets terminated when tests terminate
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
