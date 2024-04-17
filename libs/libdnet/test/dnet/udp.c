/*
 * udp.c
 *
 * Copyright (c) 2001 Dug Song <dugsong@monkey.org>
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
udp_usage(void)
{
	fprintf(stderr, "Usage: dnet udp [sport|dport <value>] ...\n");
	exit(1);
}

int
udp_main(int argc, char *argv[])
{
	struct udp_hdr *udp;
	u_char *p, buf[IP_LEN_MAX];	/* XXX */
	char *name, *value;
	int c, len;

	srand(time(NULL));
	
	udp = (struct udp_hdr *)buf;
	
	memset(udp, 0, sizeof(*udp));
	udp->uh_sport = rand() & 0xffff;
	udp->uh_dport = rand() & 0xffff;
	
	for (c = 1; c + 1 < argc; c += 2) {
		name = argv[c];
		value = argv[c + 1];

		if (strcmp(name, "sport") == 0) {
			if (port_aton(value, &udp->uh_sport) < 0)
				udp_usage();
		} else if (strcmp(name, "dport") == 0) {
			if (port_aton(value, &udp->uh_dport) < 0)
				udp_usage();
		} else
			udp_usage();
	}
	argc -= c;
	argv += c;

	if (argc != 0)
		udp_usage();

	p = buf + UDP_HDR_LEN;
	
	if (!isatty(STDIN_FILENO)) {
		len = sizeof(buf) - (p - buf);
		while ((c = read(STDIN_FILENO, p, len)) > 0) {
			p += c;
			len -= c;
		}
	}
	len = p - buf;
	udp->uh_ulen = htons(len);
	
	if (write(STDOUT_FILENO, buf, len) != len)
		err(1, "write");

	return (0);
}

struct mod mod_udp = {
	"udp",
	MOD_TYPE_ENCAP,
	udp_main
};
