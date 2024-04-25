/*
 *  IXUnityBuildsTest.cpp
 *  Author: Benjamin Sergeant
 *  Copyright (c) 2019 Machine Zone. All rights reserved.
 */

#include "catch.hpp"
#include <ixwebsocket/IXCancellationRequest.h>
#include <ixwebsocket/IXConnectionState.h>
#include <ixwebsocket/IXDNSLookup.h>
#include <ixwebsocket/IXHttp.h>
#include <ixwebsocket/IXHttpClient.h>
#include <ixwebsocket/IXHttpServer.h>
#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXProgressCallback.h>
#include <ixwebsocket/IXSelectInterrupt.h>
#include <ixwebsocket/IXSelectInterruptFactory.h>
#include <ixwebsocket/IXSetThreadName.h>
#include <ixwebsocket/IXSocket.h>
#include <ixwebsocket/IXSocketAppleSSL.h>
#include <ixwebsocket/IXSocketConnect.h>
#include <ixwebsocket/IXSocketFactory.h>
#include <ixwebsocket/IXSocketMbedTLS.h>
#include <ixwebsocket/IXSocketOpenSSL.h>
#include <ixwebsocket/IXSocketServer.h>
#include <ixwebsocket/IXUrlParser.h>
#include <ixwebsocket/IXWebSocket.h>
#include <ixwebsocket/IXWebSocketCloseConstants.h>
#include <ixwebsocket/IXWebSocketCloseInfo.h>
#include <ixwebsocket/IXWebSocketErrorInfo.h>
#include <ixwebsocket/IXWebSocketHandshake.h>
#include <ixwebsocket/IXWebSocketHandshakeKeyGen.h>
#include <ixwebsocket/IXWebSocketHttpHeaders.h>
#include <ixwebsocket/IXWebSocketMessage.h>
#include <ixwebsocket/IXWebSocketMessageType.h>
#include <ixwebsocket/IXWebSocketOpenInfo.h>
#include <ixwebsocket/IXWebSocketPerMessageDeflate.h>
#include <ixwebsocket/IXWebSocketPerMessageDeflateCodec.h>
#include <ixwebsocket/IXWebSocketPerMessageDeflateOptions.h>
#include <ixwebsocket/IXWebSocketSendInfo.h>
#include <ixwebsocket/IXWebSocketServer.h>
#include <ixwebsocket/IXWebSocketTransport.h>

using namespace ix;

TEST_CASE("unity build", "[unity_build]")
{
    SECTION("dummy test")
    {
        REQUIRE(true);
    }
}
