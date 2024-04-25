/*
 * tcp.c
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
tcp_usage(void)
{
	fprintf(stderr, "Usage: dnet tcp [sport|dport|flags|seq|ack|"
	    "win|urp <value>] ...\n");
	exit(1);
}

int
tcp_main(int argc, char *argv[])
{
	struct tcp_hdr *tcp;
	u_char *p, buf[IP_LEN_MAX];	/* XXX */
	char *name, *value;
	int c, len;
	
	srand(time(NULL));
	
	tcp = (struct tcp_hdr *)buf;
	
	memset(tcp, 0, sizeof(*tcp));
	tcp->th_sport = rand() & 0xffff;
	tcp->th_dport = rand() & 0xffff;
	tcp->th_seq = rand();
	tcp->th_ack = 0;
	tcp->th_off = 5;
	tcp->th_flags = TH_SYN;
	tcp->th_win = TCP_WIN_MAX;
	tcp->th_urp = 0;

	for (c = 1; c + 1 < argc; c += 2) {
		name = argv[c];
		value = argv[c + 1];
		
		if (strcmp(name, "sport") == 0) {
			if (port_aton(value, &tcp->th_sport) < 0)
				tcp_usage();
		} else if (strcmp(name, "dport") == 0) {
			if (port_aton(value, &tcp->th_dport) < 0)
				tcp_usage();
		} else if (strcmp(name, "flags") == 0) {
			if (flags_aton(value, &tcp->th_flags) < 0)
				tcp_usage();
		} else if (strcmp(name, "seq") == 0) {
			if (seq_aton(value, &tcp->th_seq) < 0)
				tcp_usage();
		} else if (strcmp(name, "ack") == 0) {
			if (seq_aton(value, &tcp->th_ack) < 0)
				tcp_usage();
		} else if (strcmp(name, "win") == 0) {
			if (port_aton(value, &tcp->th_win) < 0)
				tcp_usage();
		} else if (strcmp(name, "urp") == 0) {
			if (port_aton(value, &tcp->th_urp) < 0)
				tcp_usage();
		} else
			tcp_usage();
	}
	argc -= c;
	argv += c;
	
	if (argc != 0)
		tcp_usage();

	p = buf + TCP_HDR_LEN;
	
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

struct mod mod_tcp = {
	"tcp",
	MOD_TYPE_ENCAP,
	&tcp_main
};
