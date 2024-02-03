/*
 *  IXExponentialBackoffTest.cpp
 *  Author: Benjamin Sergeant
 *  Copyright (c) 2022 Machine Zone. All rights reserved.
 */

#include "IXTest.h"
#include "catch.hpp"
#include <iostream>
#include <ixwebsocket/IXExponentialBackoff.h>
#include <string.h>

using namespace ix;

namespace ix
{
    TEST_CASE("exponential_backoff", "[exponential_backoff]")
    {
        SECTION("1")
        {
            // First parameter is retrycount
            REQUIRE(calculateRetryWaitMilliseconds(0, 10000, 100) == 100);
            REQUIRE(calculateRetryWaitMilliseconds(1, 10000, 100) == 200);
            REQUIRE(calculateRetryWaitMilliseconds(2, 10000, 100) == 400);
            REQUIRE(calculateRetryWaitMilliseconds(3, 10000, 100) == 800);
            REQUIRE(calculateRetryWaitMilliseconds(4, 10000, 100) == 1600);
            REQUIRE(calculateRetryWaitMilliseconds(5, 10000, 100) == 3200);
            REQUIRE(calculateRetryWaitMilliseconds(6, 10000, 100) == 6400);
            REQUIRE(calculateRetryWaitMilliseconds(20, 10000, 100) == 10000);
            REQUIRE(calculateRetryWaitMilliseconds(25, 10000, 100) == 10000);
            
            // Things get special after 26 retries
            REQUIRE(calculateRetryWaitMilliseconds(26, 10000, 100) == 10000);
            REQUIRE(calculateRetryWaitMilliseconds(27, 10000, 100) == 10000);
            REQUIRE(calculateRetryWaitMilliseconds(27, 10000, 100) == 10000);
        }
    }

} // namespace ix
