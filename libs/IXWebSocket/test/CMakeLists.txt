#
# Author: Benjamin Sergeant
# Copyright (c) 2018 Machine Zone, Inc. All rights reserved.
#
cmake_minimum_required (VERSION 3.14)
project (ixwebsocket_unittest)

set (CMAKE_CXX_STANDARD 11)

option(USE_TLS "Add TLS support" ON)

# Shared sources
set (TEST_TARGET_NAMES
  IXSocketTest
  IXSocketConnectTest
  IXWebSocketServerTest
  IXWebSocketTestConnectionDisconnection
  IXUrlParserTest
  IXHttpClientTest
  IXUnityBuildsTest
  IXHttpTest
  IXDNSLookupTest
  IXWebSocketSubProtocolTest
  # IXWebSocketBroadcastTest ## FIXME was depending on cobra / take a broadcast server from ws
  IXStrCaseCompareTest
  IXExponentialBackoffTest
  IXWebSocketCloseTest
)

# Some unittest don't work on windows yet
# Windows without TLS does not have hmac yet
if (UNIX)
  list(APPEND TEST_TARGET_NAMES
    # Fail on Windows in CI probably because the pathing is wrong and
    # some resource files cannot be found
    IXHttpServerTest
    IXWebSocketChatTest
  )
endif()

if (USE_ZLIB)
  list(APPEND TEST_TARGET_NAMES
    IXWebSocketPerMessageDeflateCompressorTest
  )
endif()

# Ping test fails intermittently, disabling them for now
# IXWebSocketPingTest.cpp
# IXWebSocketPingTimeoutTest.cpp

# IXWebSocketLeakTest.cpp # commented until we have a fix for #224 /
# that was was fixed but now the test does not compile

# Disable tests for now that are failing or not reliable

add_library(ixwebsocket_test)
target_sources(ixwebsocket_test PRIVATE 
    ${JSONCPP_SOURCES}
    test_runner.cpp
    IXTest.cpp
    ../third_party/msgpack11/msgpack11.cpp
)
target_compile_definitions(ixwebsocket_test PRIVATE ${TEST_PROGRAMS_DEFINITIONS})
target_include_directories(ixwebsocket_test PRIVATE
  ${PROJECT_SOURCE_DIR}/Catch2/single_include
  ../third_party
)
target_link_libraries(ixwebsocket_test ixwebsocket)
target_link_libraries(ixwebsocket_test spdlog)

foreach(TEST_TARGET_NAME ${TEST_TARGET_NAMES})
  add_executable(${TEST_TARGET_NAME}
    ${TEST_TARGET_NAME}.cpp
  )

  target_include_directories(${TEST_TARGET_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/Catch2/single_include
    ../third_party
    ../third_party/msgpack11
  )

  target_compile_definitions(${TEST_TARGET_NAME} PRIVATE SPDLOG_COMPILED_LIB=1)

  if (APPLE AND USE_TLS)
    target_link_libraries(${TEST_TARGET_NAME} "-framework foundation" "-framework security")
  endif()

  # library with the most dependencies come first
  target_link_libraries(${TEST_TARGET_NAME} ixwebsocket_test)
  target_link_libraries(${TEST_TARGET_NAME} ixwebsocket)

  target_link_libraries(${TEST_TARGET_NAME} spdlog)

  add_test(NAME ${TEST_TARGET_NAME}
           COMMAND ${TEST_TARGET_NAME}
           WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

endforeach()
