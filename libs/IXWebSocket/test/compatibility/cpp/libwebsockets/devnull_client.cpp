/*
 * lws-minimal-ws-client
 *
 * Written in 2010-2019 by Andy Green <andy@warmcat.com>
 *
 * This file is made available under the Creative Commons CC0 1.0
 * Universal Public Domain Dedication.
 *
 * This demonstrates the a minimal ws client using lws.
 *
 * Original programs connects to https://libwebsockets.org/ and makes a
 * wss connection to the dumb-increment protocol there.  While
 * connected, it prints the numbers it is being sent by
 * dumb-increment protocol.
 *
 * This is modified to make a test client which counts how much messages
 * per second can be received.
 *
 * libwebsockets$ make && ./a.out
 * g++ --std=c++14 -I/usr/local/opt/openssl/include devnull_client.cpp -lwebsockets
 * messages received: 0 per second 0 total
 * [2020/08/02 19:22:21:4774] U: LWS minimal ws client rx [-d <logs>] [--h2]
 * [2020/08/02 19:22:21:4814] U: callback_dumb_increment: established
 * messages received: 0 per second 0 total
 * messages received: 180015 per second 180015 total
 * messages received: 172866 per second 352881 total
 * messages received: 176177 per second 529058 total
 * messages received: 174191 per second 703249 total
 * messages received: 193397 per second 896646 total
 * messages received: 196385 per second 1093031 total
 * messages received: 194593 per second 1287624 total
 * messages received: 189484 per second 1477108 total
 * messages received: 200825 per second 1677933 total
 * messages received: 183542 per second 1861475 total
 * ^C[2020/08/02 19:22:33:4450] U: Completed OK
 *
 */

#include <atomic>
#include <iostream>
#include <libwebsockets.h>
#include <signal.h>
#include <string.h>
#include <thread>

static int interrupted;
static struct lws* client_wsi;

std::atomic<uint64_t> receivedCount(0);

static int callback_dumb_increment(
    struct lws* wsi, enum lws_callback_reasons reason, void* user, void* in, size_t len)
{
    switch (reason)
    {
        /* because we are protocols[0] ... */
        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
            lwsl_err("CLIENT_CONNECTION_ERROR: %s\n", in ? (char*) in : "(null)");
            client_wsi = NULL;
            break;

        case LWS_CALLBACK_CLIENT_ESTABLISHED: lwsl_user("%s: established\n", __func__); break;

        case LWS_CALLBACK_CLIENT_RECEIVE: receivedCount++; break;

        case LWS_CALLBACK_CLIENT_CLOSED: client_wsi = NULL; break;

        default: break;
    }

    return lws_callback_http_dummy(wsi, reason, user, in, len);
}

static const struct lws_protocols protocols[] = {{
                                                     "dumb-increment-protocol",
                                                     callback_dumb_increment,
                                                     0,
                                                     0,
                                                 },
                                                 {NULL, NULL, 0, 0}};

static void sigint_handler(int sig)
{
    interrupted = 1;
}

int main(int argc, const char** argv)
{
    uint64_t receivedCountTotal(0);
    uint64_t receivedCountPerSecs(0);

    auto timer = [&receivedCountTotal, &receivedCountPerSecs] {
        while (!interrupted)
        {
            std::cerr << "messages received: " << receivedCountPerSecs << " per second "
                      << receivedCountTotal << " total" << std::endl;

            receivedCountPerSecs = receivedCount - receivedCountTotal;
            receivedCountTotal += receivedCountPerSecs;

            auto duration = std::chrono::seconds(1);
            std::this_thread::sleep_for(duration);
        }
    };

    std::thread t1(timer);

    struct lws_context_creation_info info;
    struct lws_client_connect_info i;
    struct lws_context* context;
    const char* p;
    int n = 0, logs = LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE
        /* for LLL_ verbosity above NOTICE to be built into lws, lws
         * must have been configured with -DCMAKE_BUILD_TYPE=DEBUG
         * instead of =RELEASE */
        /* | LLL_INFO */ /* | LLL_PARSER */ /* | LLL_HEADER */
        /* | LLL_EXT */ /* | LLL_CLIENT */  /* | LLL_LATENCY */
        /* | LLL_DEBUG */;

    signal(SIGINT, sigint_handler);
    if ((p = lws_cmdline_option(argc, argv, "-d"))) logs = atoi(p);

    lws_set_log_level(logs, NULL);
    lwsl_user("LWS minimal ws client rx [-d <logs>] [--h2]\n");

    memset(&info, 0, sizeof info);      /* otherwise uninitialized garbage */
    info.port = CONTEXT_PORT_NO_LISTEN; /* we do not run any server */
    info.protocols = protocols;
    info.timeout_secs = 10;

    /*
     * since we know this lws context is only ever going to be used with
     * one client wsis / fds / sockets at a time, let lws know it doesn't
     * have to use the default allocations for fd tables up to ulimit -n.
     * It will just allocate for 1 internal and 1 (+ 1 http2 nwsi) that we
     * will use.
     */
    info.fd_limit_per_thread = 1 + 1 + 1;

    context = lws_create_context(&info);
    if (!context)
    {
        lwsl_err("lws init failed\n");
        return 1;
    }

    memset(&i, 0, sizeof i); /* otherwise uninitialized garbage */
    i.context = context;
    i.port = 8008;
    i.address = "127.0.0.1";
    i.path = "/";
    i.host = i.address;
    i.origin = i.address;
    i.protocol = protocols[0].name; /* "dumb-increment-protocol" */
    i.pwsi = &client_wsi;

    if (lws_cmdline_option(argc, argv, "--h2")) i.alpn = "h2";

    lws_client_connect_via_info(&i);

    while (n >= 0 && client_wsi && !interrupted)
        n = lws_service(context, 0);

    lws_context_destroy(context);

    lwsl_user("Completed %s\n", receivedCount > 10 ? "OK" : "Failed");

    t1.join();

    return receivedCount > 10;
}
