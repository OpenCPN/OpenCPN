/*
 * send.c
 *
 * Copyright (c) 2002 Dug Song <dugsong@monkey.org>
 *
 * $Id$
 */

#include "config.h"

#include <sys/types.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "dnet.h"
#include "aton.h"
#include "mod.h"

void
send_usage(void)
{
	fprintf(stderr, "Usage: dnet send [<device>]\n");
	exit(1);
}

int
send_main(int argc, char *argv[])
{
	eth_t *eth;
	ip_t *ip;
	u_char *p, buf[IP_LEN_MAX];	/* XXX */
	int c, len;

	if (argc == 2 && *(argv[1]) == '-')
		send_usage();
	
	if (isatty(STDIN_FILENO))
		errx(1, "can't read packet to send from tty");

	p = buf;
	len = sizeof(buf) - (p - buf);
	
	while ((c = read(STDIN_FILENO, p, len)) > 0) {
		p += c;
		len -= c;
	}
	len = p - buf;
	
	if (argc == 1) {
		if ((ip = ip_open()) == NULL)
			err(1, "ip_open");
		if (ip_send(ip, buf, len) != len)
			err(1, "ip_send");
		ip_close(ip);
	} else if (argc == 2) {
		if ((eth = eth_open(argv[1])) == NULL)
			err(1, "eth_open");
		if (eth_send(eth, buf, len) != len)
			err(1, "eth_send");
		eth_close(eth);
	} else
		send_usage();
	
	exit(0);
}

struct mod mod_send = {
	"send",
	MOD_TYPE_XMIT,
	send_main
};
