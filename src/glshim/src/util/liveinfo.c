#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

int liveinfo_send(const char *cmd) {
    static int init = 0;
    static int sock = 0;
    struct sockaddr_un sun = {
        .sun_family = AF_UNIX,
        .sun_path = "\0liveinfo",
    };
    if (! init) {
        init = 1;
        sock = socket(PF_UNIX, SOCK_DGRAM, 0);
        int ret = sendto(sock, cmd, strlen(cmd), 0, (struct sockaddr *)&sun, sizeof(sun));
        fcntl(sock, F_SETFL, O_NONBLOCK);
        return ret;
    } else if (sock == -1) {
        return -1;
    }
    sendto(sock, cmd, strlen(cmd), 0, (struct sockaddr *)&sun, sizeof(sun));
    return 0;
}
