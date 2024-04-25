/*
 *  IXHttpTest.cpp
 *  Author: Benjamin Sergeant
 *  Copyright (c) 2019 Machine Zone. All rights reserved.
 */

#include "catch.hpp"
#include <iostream>
#include <ixwebsocket/IXHttp.h>
#include <string.h>

namespace ix
{
    TEST_CASE("http", "[http]")
    {
        SECTION("Normal case")
        {
            std::string line = "HTTP/1.1 200";
            auto result = Http::parseStatusLine(line);

            REQUIRE(result.first == "HTTP/1.1");
            REQUIRE(result.second == 200);
        }

        SECTION("http/1.0 case")
        {
            std::string line = "HTTP/1.0 200";
            auto result = Http::parseStatusLine(line);

            REQUIRE(result.first == "HTTP/1.0");
            REQUIRE(result.second == 200);
        }

        SECTION("empty case")
        {
            std::string line = "";
            auto result = Http::parseStatusLine(line);

            REQUIRE(result.first == "");
            REQUIRE(result.second == -1);
        }

        SECTION("empty case")
        {
            std::string line = "HTTP/1.1";
            auto result = Http::parseStatusLine(line);

            REQUIRE(result.first == "HTTP/1.1");
            REQUIRE(result.second == -1);
        }
    }

} // namespace ix
