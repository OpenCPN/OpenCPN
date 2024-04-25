/*
 *  IXSocketTest.cpp
 *  Author: Benjamin Sergeant
 *  Copyright (c) 2019 Machine Zone. All rights reserved.
 */

#include "IXTest.h"
#include "catch.hpp"
#include <iostream>
#include <ixwebsocket/IXCancellationRequest.h>
#include <ixwebsocket/IXSocket.h>
#include <ixwebsocket/IXSocketFactory.h>
#include <string.h>

using namespace ix;

namespace ix
{
    void testSocket(const std::string& host,
                    int port,
                    const std::string& request,
                    std::shared_ptr<Socket> socket,
                    int expectedStatus,
                    int timeoutSecs)
    {
        std::string errMsg;
        static std::atomic<bool> requestInitCancellation(false);
        auto isCancellationRequested =
            makeCancellationRequestWithTimeout(timeoutSecs, requestInitCancellation);

        bool success = socket->connect(host, port, errMsg, isCancellationRequested);
        TLogger() << "errMsg: " << errMsg;
        REQUIRE(success);

        TLogger() << "Sending request: " << request << "to " << host << ":" << port;
        REQUIRE(socket->writeBytes(request, isCancellationRequested));

        auto lineResult = socket->readLine(isCancellationRequested);
        auto lineValid = lineResult.first;
        auto line = lineResult.second;

        TLogger() << "read error: " << strerror(Socket::getErrno());

        REQUIRE(lineValid);

        int status = -1;
        REQUIRE(sscanf(line.c_str(), "HTTP/1.1 %d", &status) == 1);
        REQUIRE(status == expectedStatus);
    }
} // namespace ix

TEST_CASE("socket", "[socket]")
{
    SECTION("Connect to a local websocket server over a free port. Send GET request without "
            "header. Should return 400")
    {
        // Start a server first which we'll hit with our socket code
        int port = getFreePort();
        ix::WebSocketServer server(port);
        REQUIRE(startWebSocketEchoServer(server));

        std::string errMsg;
        bool tls = false;
        SocketTLSOptions tlsOptions;
        std::shared_ptr<Socket> socket = createSocket(tls, -1, errMsg, tlsOptions);
        std::string host("127.0.0.1");

        std::stringstream ss;
        ss << "GET / HTTP/1.1\r\n";
        ss << "Host: " << host << "\r\n";
        ss << "\r\n";
        std::string request(ss.str());

        int expectedStatus = 400;
        int timeoutSecs = 3;

        testSocket(host, port, request, socket, expectedStatus, timeoutSecs);
    }

#if defined(IXWEBSOCKET_USE_TLS)
    SECTION("Connect to google HTTPS server over port 443. Send GET request without header. Should "
            "return 200")
    {
        std::string errMsg;
        bool tls = true;
        SocketTLSOptions tlsOptions;
        tlsOptions.caFile = "cacert.pem";
        std::shared_ptr<Socket> socket = createSocket(tls, -1, errMsg, tlsOptions);
        std::string host("www.google.com");
        int port = 443;
        std::string request("GET / HTTP/1.1\r\n\r\n");
        int expectedStatus = 200;
        int timeoutSecs = 3;

        testSocket(host, port, request, socket, expectedStatus, timeoutSecs);
    }
#endif
}
