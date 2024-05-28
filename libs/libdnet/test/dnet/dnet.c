/*
 * dnet.c
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
#include "mod.h"

/*
 * XXX - new modules should be registered here
 */
extern struct mod mod_addr;
extern struct mod mod_hex;
extern struct mod mod_rand;
extern struct mod mod_eth;
extern struct mod mod_arp;
extern struct mod mod_ip;
extern struct mod mod_icmp;
extern struct mod mod_tcp;
extern struct mod mod_udp;
extern struct mod mod_send;
extern struct mod mod_fw;
extern struct mod mod_intf;
extern struct mod mod_route;

static struct mod *modules[] = {
	&mod_addr, &mod_hex, &mod_rand, &mod_eth, &mod_arp, &mod_ip, &mod_icmp,
	&mod_tcp, &mod_udp, &mod_send, &mod_fw, &mod_intf, &mod_route, NULL
};

static void
print_modules(int type, char *string)
{
	struct mod **m;
	int i;
	
	fprintf(stderr, "%s commands:\n", string);
	for (i = 1, m = modules; *m != NULL; m++) {
		if ((m[0]->type & type) != 0) {
			fprintf(stderr, "%-10s", m[0]->name);
			if ((i++ % 8) == 0)
				fprintf(stderr, "\n");
		}
	}
	fprintf(stderr, "\n\n");
}

static void
print_usage(void)
{
	fprintf(stderr, "Usage: dnet <command> <args> ...\n\n");

	print_modules(MOD_TYPE_DATA, "Payload generation");
	print_modules(MOD_TYPE_ENCAP, "Packet encapsulation");
	print_modules(MOD_TYPE_XMIT, "Packet transmission");
	print_modules(MOD_TYPE_KERN, "Kernel interface");
}

static int
do_command(int argc, char *argv[])
{
	struct mod **m;

	for (m = modules; *m != NULL; m++) {
		if (strcmp(argv[0], m[0]->name) == 0)
			return (m[0]->main(argc, argv));
	}
	return (-1);
}

int
main(int argc, char *argv[])
{
	if (argc < 2) {
		print_usage();
		exit(1);
	}
	if (do_command(argc - 1, argv + 1) < 0) {
		print_usage();
		exit(1);
	}
	exit(0);
}
