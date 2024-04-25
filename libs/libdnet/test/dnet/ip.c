/*
 * ip.c
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
ip_usage(void)
{
	fprintf(stderr, "Usage: dnet ip [tos|id|off|ttl|proto|src|dst "
	    "<value>] ... \n");
	exit(1);
}

int
ip_main(int argc, char *argv[])
{
	struct ip_hdr *ip;
	struct addr addr;
	u_char *p, buf[IP_LEN_MAX];	/* XXX */
	char *name, *value;
	int c, len;
	
	srand(time(NULL));

	ip = (struct ip_hdr *)buf;
	ip->ip_hl = 5;
	ip->ip_v = 4;
	ip->ip_tos = 0;
	ip->ip_id = rand() & 0xffff;
	ip->ip_off = 0;
	ip->ip_ttl = IP_TTL_MAX;
	ip->ip_p = rand() & 0xff;
	ip->ip_sum = 0;
	ip->ip_src = rand();
	ip->ip_dst = rand();

	for (c = 1; c + 1 < argc; c += 2) {
		name = argv[c];
		value = argv[c + 1];
		
		if (strcmp(name, "tos") == 0)
			ip->ip_tos = atoi(value);
		else if (strcmp(name, "id") == 0)
			ip->ip_id = ntohs(atoi(value));
		else if (strcmp(name, "off") == 0) {
			if (off_aton(value, &ip->ip_off) < 0)
				ip_usage();
		} else if (strcmp(name, "ttl") == 0)
			ip->ip_ttl = atoi(value);
		else if (strcmp(name, "proto") == 0) {
			if (proto_aton(value, &ip->ip_p) < 0)
				ip_usage();
		} else if (strcmp(name, "src") == 0) {
			if (addr_aton(value, &addr) < 0)
				ip_usage();
			ip->ip_src = addr.addr_ip;
		} else if (strcmp(name, "dst") == 0) {
			if (addr_aton(value, &addr) < 0)
				ip_usage();
			ip->ip_dst = addr.addr_ip;
		} else
			ip_usage();
	}
	argc -= c;
	argv += c;
	
	if (argc != 0)
		ip_usage();
	
	if (isatty(STDIN_FILENO))
		errx(1, "can't read IP payload from tty");
	
	p = buf + IP_HDR_LEN;
	len = sizeof(buf) - (p - buf);
	
	while ((c = read(STDIN_FILENO, p, len)) > 0) {
		p += c;
		len -= c;
	}
	len = p - buf;
	
	ip->ip_len = htons(len);
	
	ip_checksum(buf, len);
	
	if (write(STDOUT_FILENO, buf, len) != len)
		err(1, "write");

	return (0);
}

struct mod mod_ip = {
	"ip",
	MOD_TYPE_ENCAP,
	ip_main
};
