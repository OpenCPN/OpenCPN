/*
 *  IXStrCaseCompareTest.cpp
 *  Author: Benjamin Sergeant
 *  Copyright (c) 2020 Machine Zone. All rights reserved.
 */

#include "IXTest.h"
#include "catch.hpp"
#include <iostream>
#include <ixwebsocket/IXUrlParser.h>
#include <string.h>

using namespace ix;

namespace ix
{
    TEST_CASE("str_case_compare", "[str_case_compare]")
    {
        SECTION("1")
        {
            using HttpHeaders = std::map<std::string, std::string, CaseInsensitiveLess>;

            HttpHeaders httpHeaders;

            httpHeaders["foo"] = "foo";

            REQUIRE(httpHeaders["foo"] == "foo");
            REQUIRE(httpHeaders["missing"] == "");

            // Comparison should be case insensitive
            REQUIRE(httpHeaders["Foo"] == "foo");
            REQUIRE(httpHeaders["Foo"] != "bar");
        }

        SECTION("2")
        {
            using HttpHeaders = std::map<std::string, std::string, CaseInsensitiveLess>;

            HttpHeaders headers;

            headers["Upgrade"] = "webSocket";

            REQUIRE(!CaseInsensitiveLess::cmp(headers["upGRADE"], "webSocket"));
        }
    }

} // namespace ix
