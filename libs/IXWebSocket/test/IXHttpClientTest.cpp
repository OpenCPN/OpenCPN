/*
 *  IXSocketTest.cpp
 *  Author: Benjamin Sergeant
 *  Copyright (c) 2019 Machine Zone. All rights reserved.
 */

#include "catch.hpp"
#include <cstdint>
#include <iostream>
#include <ixwebsocket/IXGetFreePort.h>
#include <ixwebsocket/IXHttpClient.h>
#include <ixwebsocket/IXHttpServer.h>

using namespace ix;

TEST_CASE("http_client", "[http]")
{
    SECTION("Connect to a remote HTTP server")
    {
        HttpClient httpClient;
        WebSocketHttpHeaders headers;

        std::string url("http://httpbin.org/");
        auto args = httpClient.createRequest(url);

        args->extraHeaders = headers;
        args->connectTimeout = 60;
        args->transferTimeout = 60;
        args->followRedirects = true;
        args->maxRedirects = 10;
        args->verbose = true;
        args->compress = true;
        args->logger = [](const std::string& msg) { std::cout << msg; };
        args->onProgressCallback = [](int current, int total) -> bool {
            std::cerr << "\r"
                      << "Downloaded " << current << " bytes out of " << total;
            return true;
        };

        auto response = httpClient.get(url, args);

        for (auto it : response->headers)
        {
            std::cerr << it.first << ": " << it.second << std::endl;
        }

        std::cerr << "Upload size: " << response->uploadSize << std::endl;
        std::cerr << "Download size: " << response->downloadSize << std::endl;
        std::cerr << "Status: " << response->statusCode << std::endl;
        std::cerr << "Error message: " << response->errorMsg << std::endl;

        REQUIRE(response->errorCode == HttpErrorCode::Ok);
        REQUIRE(response->statusCode == 200);
    }

#ifdef IXWEBSOCKET_USE_TLS
    SECTION("Connect to a remote HTTPS server")
    {
        HttpClient httpClient;
        WebSocketHttpHeaders headers;

        SocketTLSOptions tlsOptions;
        tlsOptions.caFile = "cacert.pem";
        httpClient.setTLSOptions(tlsOptions);

        std::string url("https://httpbin.org/");
        auto args = httpClient.createRequest(url);

        args->extraHeaders = headers;
        args->connectTimeout = 60;
        args->transferTimeout = 60;
        args->followRedirects = true;
        args->maxRedirects = 10;
        args->verbose = true;
        args->compress = true;
        args->logger = [](const std::string& msg) { std::cout << msg; };
        args->onProgressCallback = [](int current, int total) -> bool {
            std::cerr << "\r"
                      << "Downloaded " << current << " bytes out of " << total;
            return true;
        };

        auto response = httpClient.get(url, args);

        for (auto it : response->headers)
        {
            std::cerr << it.first << ": " << it.second << std::endl;
        }

        std::cerr << "Upload size: " << response->uploadSize << std::endl;
        std::cerr << "Download size: " << response->downloadSize << std::endl;
        std::cerr << "Status: " << response->statusCode << std::endl;
        std::cerr << "Error message: " << response->errorMsg << std::endl;

        REQUIRE(response->errorCode == HttpErrorCode::Ok);
        REQUIRE(response->statusCode == 200);
    }
#endif

#if defined(IXWEBSOCKET_USE_TLS) && !defined(IXWEBSOCKET_USE_SECURE_TRANSPORT)
    SECTION("Disable hostname validation")
    {
        static auto test_cert_with_wrong_name = [](bool validate_hostname)
        {
            int port = getFreePort();
            ix::HttpServer server(port, "127.0.0.1");

            SocketTLSOptions tlsOptionsServer;
            tlsOptionsServer.tls = true;
            tlsOptionsServer.caFile = "NONE";
            tlsOptionsServer.certFile = "./.certs/wrong-name-server-crt.pem";
            tlsOptionsServer.keyFile = "./.certs/wrong-name-server-key.pem";
            server.setTLSOptions(tlsOptionsServer);

            auto res = server.listen();
            REQUIRE(res.first);
            server.start();

            HttpClient httpClient;
            SocketTLSOptions tlsOptionsClient;
            tlsOptionsClient.caFile = "./.certs/trusted-ca-crt.pem";
            tlsOptionsClient.disable_hostname_validation = validate_hostname;
            httpClient.setTLSOptions(tlsOptionsClient);

            std::string url("https://localhost:" + std::to_string(port));
            auto args = httpClient.createRequest(url);
            args->connectTimeout = 10;
            args->transferTimeout = 10;

            auto response = httpClient.get(url, args);

            std::cerr << "Status: " << response->statusCode << std::endl;
            std::cerr << "Error code: " << (int) response->errorCode << std::endl;
            std::cerr << "Error message: " << response->errorMsg << std::endl;

            server.stop();
            return std::make_tuple(response->errorCode, response->statusCode);
        };

        REQUIRE(test_cert_with_wrong_name(false) ==
                std::make_tuple(HttpErrorCode::CannotConnect, 0));
        REQUIRE(test_cert_with_wrong_name(true) == std::make_tuple(HttpErrorCode::Ok, 404));
    }
#endif

    SECTION("Async API, one call")
    {
        bool async = true;
        HttpClient httpClient(async);
        WebSocketHttpHeaders headers;

        SocketTLSOptions tlsOptions;
        tlsOptions.caFile = "cacert.pem";
        httpClient.setTLSOptions(tlsOptions);

        std::string url("http://httpbin.org/");
        auto args = httpClient.createRequest(url);

        args->extraHeaders = headers;
        args->connectTimeout = 60;
        args->transferTimeout = 60;
        args->followRedirects = true;
        args->maxRedirects = 10;
        args->verbose = true;
        args->compress = true;
        args->logger = [](const std::string& msg) { std::cout << msg; };
        args->onProgressCallback = [](int current, int total) -> bool {
            std::cerr << "\r"
                      << "Downloaded " << current << " bytes out of " << total;
            return true;
        };

        std::atomic<bool> requestCompleted(false);
        std::atomic<int> statusCode(0);

        httpClient.performRequest(
            args, [&requestCompleted, &statusCode](const HttpResponsePtr& response) {
                std::cerr << "Upload size: " << response->uploadSize << std::endl;
                std::cerr << "Download size: " << response->downloadSize << std::endl;
                std::cerr << "Status: " << response->statusCode << std::endl;
                std::cerr << "Error message: " << response->errorMsg << std::endl;

                // In case of failure, print response->errorMsg
                statusCode = response->statusCode;
                requestCompleted = true;
            });

        int wait = 0;
        while (wait < 5000)
        {
            if (requestCompleted) break;

            std::chrono::duration<double, std::milli> duration(10);
            std::this_thread::sleep_for(duration);
            wait += 10;
        }

        std::cerr << "Done" << std::endl;
        REQUIRE(statusCode == 200);
    }

    SECTION("Async API, multiple calls")
    {
        bool async = true;
        HttpClient httpClient(async);
        WebSocketHttpHeaders headers;

        std::string url("http://httpbin.org/");
        auto args = httpClient.createRequest(url);

        args->extraHeaders = headers;
        args->connectTimeout = 60;
        args->transferTimeout = 60;
        args->followRedirects = true;
        args->maxRedirects = 10;
        args->verbose = true;
        args->compress = true;
        args->logger = [](const std::string& msg) { std::cout << msg; };
        args->onProgressCallback = [](int current, int total) -> bool {
            std::cerr << "\r"
                      << "Downloaded " << current << " bytes out of " << total;
            return true;
        };

        std::atomic<bool> requestCompleted(false);
        std::atomic<int> statusCode0(0);
        std::atomic<int> statusCode1(0);
        std::atomic<int> statusCode2(0);

        for (int i = 0; i < 3; ++i)
        {
            httpClient.performRequest(
                args,
                [i, &requestCompleted, &statusCode0, &statusCode1, &statusCode2](
                    const HttpResponsePtr& response) {
                    std::cerr << "Upload size: " << response->uploadSize << std::endl;
                    std::cerr << "Download size: " << response->downloadSize << std::endl;
                    std::cerr << "Status: " << response->statusCode << std::endl;
                    std::cerr << "Error message: " << response->errorMsg << std::endl;

                    // In case of failure, print response->errorMsg
                    if (i == 0)
                    {
                        statusCode0 = response->statusCode;
                    }
                    else if (i == 1)
                    {
                        statusCode1 = response->statusCode;
                    }
                    else if (i == 2)
                    {
                        statusCode2 = response->statusCode;
                        requestCompleted = true;
                    }
                });
        }

        int wait = 0;
        while (wait < 10000)
        {
            if (requestCompleted) break;

            std::chrono::duration<double, std::milli> duration(10);
            std::this_thread::sleep_for(duration);
            wait += 10;
        }

        std::cerr << "Done" << std::endl;
        REQUIRE(statusCode0 == 200);
        REQUIRE(statusCode1 == 200);
        REQUIRE(statusCode2 == 200);
    }

    SECTION("Async API, cancel")
    {
        bool async = true;
        HttpClient httpClient(async);
        WebSocketHttpHeaders headers;

        SocketTLSOptions tlsOptions;
        tlsOptions.caFile = "cacert.pem";
        httpClient.setTLSOptions(tlsOptions);

        std::string url("http://httpbin.org/delay/10");
        auto args = httpClient.createRequest(url);

        args->extraHeaders = headers;
        args->connectTimeout = 60;
        args->transferTimeout = 60;
        args->followRedirects = true;
        args->maxRedirects = 10;
        args->verbose = true;
        args->compress = true;
        args->logger = [](const std::string& msg) { std::cout << msg; };
        args->onProgressCallback = [](int current, int total) -> bool {
            std::cerr << "\r"
                      << "Downloaded " << current << " bytes out of " << total;
            return true;
        };

        std::atomic<bool> requestCompleted(false);
        std::atomic<HttpErrorCode> errorCode(HttpErrorCode::Invalid);

        httpClient.performRequest(
            args, [&requestCompleted, &errorCode](const HttpResponsePtr& response) {
                errorCode = response->errorCode;
                requestCompleted = true;
            });

        // cancel immediately
        args->cancel = true;

        int wait = 0;
        while (wait < 5000)
        {
            if (requestCompleted) break;

            std::chrono::duration<double, std::milli> duration(10);
            std::this_thread::sleep_for(duration);
            wait += 10;
        }

        std::cerr << "Done" << std::endl;
        REQUIRE(errorCode == HttpErrorCode::Cancelled);
    }

    SECTION("Async API, streaming transfer")
    {
        bool async = true;
        HttpClient httpClient(async);
        WebSocketHttpHeaders headers;

        SocketTLSOptions tlsOptions;
        tlsOptions.caFile = "cacert.pem";
        httpClient.setTLSOptions(tlsOptions);

        std::string url("http://speedtest.belwue.net/random-100M");
        auto args = httpClient.createRequest(url);

        args->extraHeaders = headers;
        args->connectTimeout = 60;
        args->transferTimeout = 120;
        args->followRedirects = true;
        args->maxRedirects = 10;
        args->verbose = true;
        args->compress = false;
        args->logger = [](const std::string& msg) { std::cout << msg; };
        args->onProgressCallback = [](int current, int total) -> bool {
            std::cerr << "\r"
                      << "Downloaded " << current << " bytes out of " << total;
            return true;
        };

        // compute Adler-32 checksum of received data
        uint32_t a = 1, b = 0;
        args->onChunkCallback = [&](const std::string& data) {
            for (const char c: data)
            {
                a = (a + (unsigned char)c) % 65521;
                b = (b + a) % 65521;
            }
        };

        std::atomic<bool> requestCompleted(false);
        std::atomic<HttpErrorCode> errorCode(HttpErrorCode::Invalid);
        std::atomic<int> statusCode(0);

        httpClient.performRequest(
            args, [&](const HttpResponsePtr& response) {
                errorCode = response->errorCode;
                statusCode = response->statusCode;
                requestCompleted = true;
            });

        int wait = 0;
        while (wait < 120000)
        {
            if (requestCompleted) break;

            std::chrono::duration<double, std::milli> duration(10);
            std::this_thread::sleep_for(duration);
            wait += 10;
        }

        std::cerr << "Done" << std::endl;
        REQUIRE(errorCode == HttpErrorCode::Ok);
        REQUIRE(statusCode == 200);

        // compare checksum with a known good value
        uint32_t checksum = (b << 16) | a;
        REQUIRE(checksum == 1440194471);
    }
}
