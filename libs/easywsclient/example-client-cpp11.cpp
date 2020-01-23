// Compile with:
// g++ -std=gnu++0x example-client-cpp11.cpp -o example-client-cpp11
#include "easywsclient.hpp"
//#include "easywsclient.cpp" // <-- include only if you don't want compile separately
#ifdef _WIN32
#pragma comment( lib, "ws2_32" )
#include <WinSock2.h>
#endif
#include <assert.h>
#include <stdio.h>
#include <string>
#include <memory>

int main()
{
    using easywsclient::WebSocket;
#ifdef _WIN32
    INT rc;
    WSADATA wsaData;

    rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (rc) {
        printf("WSAStartup Failed.\n");
        return 1;
    }
#endif

    std::unique_ptr<WebSocket> ws(WebSocket::from_url("ws://localhost:8126/foo"));
    assert(ws);
    ws->send("goodbye");
    ws->send("hello");
    while (ws->getReadyState() != WebSocket::CLOSED) {
        WebSocket::pointer wsp = &*ws; // <-- because a unique_ptr cannot be copied into a lambda
        ws->poll();
        ws->dispatch([wsp](const std::string & message) {
            printf(">>> %s\n", message.c_str());
            if (message == "world") { wsp->close(); }
        });
    }
#ifdef _WIN32
    WSACleanup();
#endif
    // N.B. - unique_ptr will free the WebSocket instance upon return:
    return 0;
}
