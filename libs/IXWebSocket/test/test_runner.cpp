/*
 *  test_runner.cpp
 *  Author: Benjamin Sergeant
 *  Copyright (c) 2018 Machine Zone. All rights reserved.
 */

#define CATCH_CONFIG_RUNNER
#include "catch.hpp"
#include <ixwebsocket/IXNetSystem.h>
#include <spdlog/spdlog.h>

#ifndef _WIN32
#include <signal.h>
#endif

int main(int argc, char* argv[])
{
    ix::initNetSystem();

#ifndef _WIN32
    signal(SIGPIPE, SIG_IGN);
#endif
    spdlog::set_level(spdlog::level::debug);

    int result = Catch::Session().run(argc, argv);

    ix::uninitNetSystem();
    return result;
}
