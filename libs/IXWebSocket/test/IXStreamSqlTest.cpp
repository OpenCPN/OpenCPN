/*
 *  IXStreamSqlTest.cpp
 *  Author: Benjamin Sergeant
 *  Copyright (c) 2020 Machine Zone. All rights reserved.
 */

#include "IXTest.h"
#include "catch.hpp"
#include <iostream>
#include <ixsnake/IXStreamSql.h>
#include <string.h>

using namespace ix;

namespace ix
{
    TEST_CASE("stream_sql", "[streamsql]")
    {
        SECTION("expression A")
        {
            snake::StreamSql streamSql(
                "select * from subscriber_republished_v1_neo where session LIKE '%123456%'");

            nlohmann::json msg = {{"session", "123456"}, {"id", "foo_id"}, {"timestamp", 12}};

            CHECK(streamSql.match(msg));
        }

        SECTION("expression A")
        {
            snake::StreamSql streamSql("select * from `subscriber_republished_v1_neo` where "
                                       "session = '30091320ed8d4e50b758f8409b83bed7'");

            nlohmann::json msg = {{"session", "30091320ed8d4e50b758f8409b83bed7"},
                                  {"id", "foo_id"},
                                  {"timestamp", 12}};

            CHECK(streamSql.match(msg));
        }
    }

} // namespace ix
