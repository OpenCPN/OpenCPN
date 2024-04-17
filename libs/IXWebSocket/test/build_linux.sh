#!/bin/sh
#
# Author: Benjamin Sergeant
# Copyright (c) 2017-2018 Machine Zone, Inc. All rights reserved.
#

# 'manual' way of building. You can also use cmake.

g++ --std=c++11 \
    -DIXWEBSOCKET_USE_TLS \
    -g \
    ../ixwebsocket/IXEventFd.cpp	\
    ../ixwebsocket/IXSocket.cpp	\
    ../ixwebsocket/IXSetThreadName.cpp	\
    ../ixwebsocket/IXWebSocketTransport.cpp \
    ../ixwebsocket/IXWebSocket.cpp \
    ../ixwebsocket/IXWebSocketServer.cpp \
    ../ixwebsocket/IXDNSLookup.cpp \
    ../ixwebsocket/IXSocketConnect.cpp \
    ../ixwebsocket/IXSocketOpenSSL.cpp \
    ../ixwebsocket/IXWebSocketPerMessageDeflate.cpp \
    ../ixwebsocket/IXWebSocketPerMessageDeflateOptions.cpp \
    -I ../.. \
    -I Catch2/single_include \
    test_runner.cpp \
    cmd_websocket_chat.cpp \
    IXTest.cpp \
    msgpack11.cpp \
    -o ixwebsocket_unittest \
    -lcrypto -lssl -lz -lpthread
