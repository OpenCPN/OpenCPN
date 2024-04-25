/*
 *  IXSocketConnectTest.cpp
 *  Author: Benjamin Sergeant
 *  Copyright (c) 2018 Machine Zone. All rights reserved.
 */

#include "IXTest.h"
#include "catch.hpp"
#include <iostream>
#include <ixwebsocket/IXSocketConnect.h>

using namespace ix;


TEST_CASE("socket_connect", "[net]")
{
    SECTION("Test connecting to a known hostname")
    {
        int port = getFreePort();
        ix::WebSocketServer server(port);
        REQUIRE(startWebSocketEchoServer(server));

        std::string errMsg;
        int fd = SocketConnect::connect("127.0.0.1", port, errMsg, [] { return false; });
        std::cerr << "Error message: " << errMsg << std::endl;
        REQUIRE(fd != -1);
    }

    SECTION("Test connecting to a non-existing hostname")
    {
        std::string errMsg;
        std::string hostname("12313lhjlkjhopiupoijlkasdckljqwehrlkqjwehraospidcuaposidcasdc");
        int fd = SocketConnect::connect(hostname, 80, errMsg, [] { return false; });
        std::cerr << "Error message: " << errMsg << std::endl;
        REQUIRE(fd == -1);
    }

    SECTION("Test connecting to a good hostname, with cancellation")
    {
        int port = getFreePort();
        ix::WebSocketServer server(port);
        REQUIRE(startWebSocketEchoServer(server));

        std::string errMsg;
        // The callback returning true means we are requesting cancellation
        int fd = SocketConnect::connect("127.0.0.1", port, errMsg, [] { return true; });
        std::cerr << "Error message: " << errMsg << std::endl;
        REQUIRE(fd == -1);
    }
}
