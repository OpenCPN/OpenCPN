/*
 *  IXSocketTest.cpp
 *  Author: Benjamin Sergeant
 *  Copyright (c) 2019 Machine Zone. All rights reserved.
 */

#include "catch.hpp"
#include <iostream>
#include <ixwebsocket/IXGetFreePort.h>
#include <ixwebsocket/IXHttpClient.h>
#include <ixwebsocket/IXHttpServer.h>

using namespace ix;

TEST_CASE("http server", "[httpd]")
{
    SECTION("Connect to a local HTTP server")
    {
        int port = getFreePort();
        ix::HttpServer server(port, "127.0.0.1");

        auto res = server.listen();
        REQUIRE(res.first);
        server.start();

        HttpClient httpClient;
        WebSocketHttpHeaders headers;

        std::string url("http://127.0.0.1:");
        url += std::to_string(port);
        url += "/data/foo.txt";
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
        REQUIRE(response->headers["Accept-Encoding"] == "gzip");

        server.stop();
    }

    SECTION("Posting plain text data to a local HTTP server")
    {
        int port = getFreePort();
        ix::HttpServer server(port, "127.0.0.1");

        server.setOnConnectionCallback(
            [](HttpRequestPtr request, std::shared_ptr<ConnectionState>) -> HttpResponsePtr {
                if (request->method == "POST")
                {
                    return std::make_shared<HttpResponse>(
                        200, "OK", HttpErrorCode::Ok, WebSocketHttpHeaders(), request->body);
                }

                return std::make_shared<HttpResponse>(400, "BAD REQUEST");
            });

        auto res = server.listen();
        REQUIRE(res.first);
        server.start();

        HttpClient httpClient;
        WebSocketHttpHeaders headers;
        headers["Content-Type"] = "text/plain";

        std::string url("http://127.0.0.1:");
        url += std::to_string(port);
        auto args = httpClient.createRequest(url);

        args->extraHeaders = headers;
        args->connectTimeout = 60;
        args->transferTimeout = 60;
        args->verbose = true;
        args->logger = [](const std::string& msg) { std::cout << msg; };
        args->body = "Hello World!";

        auto response = httpClient.post(url, args->body, args);

        std::cerr << "Status: " << response->statusCode << std::endl;
        std::cerr << "Error message: " << response->errorMsg << std::endl;
        std::cerr << "Body: " << response->body << std::endl;

        REQUIRE(response->errorCode == HttpErrorCode::Ok);
        REQUIRE(response->statusCode == 200);
        REQUIRE(response->body == args->body);

        server.stop();
    }
}

TEST_CASE("http server redirection", "[httpd_redirect]")
{
    SECTION(
        "Connect to a local HTTP server, with redirection enabled, but we do not follow redirects")
    {
        int port = getFreePort();
        ix::HttpServer server(port, "127.0.0.1");
        server.makeRedirectServer("http://example.com");

        auto res = server.listen();
        REQUIRE(res.first);
        server.start();

        HttpClient httpClient;
        WebSocketHttpHeaders headers;

        std::string url("http://127.0.0.1:");
        url += std::to_string(port);
        url += "/data/foo.txt";
        auto args = httpClient.createRequest(url);

        args->extraHeaders = headers;
        args->connectTimeout = 60;
        args->transferTimeout = 60;
        args->followRedirects = false; // we dont want to follow redirect during testing
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
        REQUIRE(response->statusCode == 301);
        REQUIRE(response->headers["Location"] == "http://example.com");

        server.stop();
    }

    SECTION("Connect to a local HTTP server, with redirection enabled, but we do follow redirects")
    {
        int port = getFreePort();
        ix::HttpServer server(port, "127.0.0.1");
        server.makeRedirectServer("http://www.google.com");

        auto res = server.listen();
        REQUIRE(res.first);
        server.start();

        HttpClient httpClient;
        WebSocketHttpHeaders headers;

        std::string url("http://127.0.0.1:");
        url += std::to_string(port);
        url += "/data/foo.txt";
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

        server.stop();
    }
}
