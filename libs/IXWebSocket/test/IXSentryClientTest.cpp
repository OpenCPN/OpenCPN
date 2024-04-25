/*
 *  IXSentryClientTest.cpp
 *  Author: Benjamin Sergeant
 *  Copyright (c) 2019 Machine Zone. All rights reserved.
 *
 *  (cd .. ; make) && ../build/test/ixwebsocket_unittest sentry
 */

#include "catch.hpp"
#include <iostream>
#include <ixsentry/IXSentryClient.h>
#include <string.h>

using namespace ix;

namespace ix
{
    TEST_CASE("sentry", "[sentry]")
    {
        SECTION("Attempt to index nil")
        {
            SentryClient sentryClient("");
            std::string stack = "Attempt to index nil[overlay]!\nstack traceback:\n\tfoo.lua:2661: "
                                "in function 'getFoo'\n\tfoo.lua:1666: in function "
                                "'onUpdate'\n\tfoo.lua:1751: in function <foo.lua:1728>";
            auto frames = sentryClient.parseLuaStackTrace(stack);

            REQUIRE(frames.size() == 3);
        }

        SECTION("Attempt to perform nil")
        {
            SentryClient sentryClient("");
            std::string stack = "Attempt to perform nil - 1572111278.299\nstack "
                                "traceback:\n\tfoo.lua:57: in function <foo.lua:53>";
            auto frames = sentryClient.parseLuaStackTrace(stack);

            REQUIRE(frames.size() == 1);
        }
    }

} // namespace ix
