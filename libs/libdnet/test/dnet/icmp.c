/*
 * icmp.c
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
#include <unistd.h>

#include "dnet.h"
#include "aton.h"
#include "mod.h"

void
icmp_usage(void)
{
	fprintf(stderr, "Usage: dnet icmp [type|code <value>] ...\n");
	exit(1);
}

int
icmp_main(int argc, char *argv[])
{
	struct icmp_hdr *icmp;
	u_char *p, buf[IP_LEN_MAX];	/* XXX */
	char *name, *value;
	int c, len;

	icmp = (struct icmp_hdr *)buf;
	
	memset(icmp, 0, sizeof(*icmp));
	icmp->icmp_type = ICMP_ECHO;
	icmp->icmp_code = 0;
	
	for (c = 1; c + 1 < argc; c += 2) {
		name = argv[c];
		value = argv[c + 1];

		if (strcmp(name, "type") == 0) {
			icmp->icmp_type = atoi(value);
		} else if (strcmp(name, "code") == 0) {
			icmp->icmp_code = atoi(value);
		} else
			icmp_usage();
	}
	argc -= c;
	argv += c;

	if (argc != 0)
		icmp_usage();

	p = buf + ICMP_HDR_LEN;
	
	if (!isatty(STDIN_FILENO)) {
		len = sizeof(buf) - (p - buf);
		while ((c = read(STDIN_FILENO, p, len)) > 0) {
			p += c;
			len -= c;
		}
	}
	len = p - buf;
	
	if (write(STDOUT_FILENO, buf, len) != len)
		err(1, "write");

	return (0);
}

struct mod mod_icmp = {
	"icmp",
	MOD_TYPE_ENCAP,
	icmp_main
};
