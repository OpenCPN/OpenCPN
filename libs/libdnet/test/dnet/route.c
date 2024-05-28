/*
 * route.c
 *
 * Copyright (c) 2001 Dug Song <dugsong@monkey.org>
 *
 * $Id$
 */

#include "config.h"

#include <sys/types.h>

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dnet.h"
#include "mod.h"

static void
usage(void)
{
	fprintf(stderr, "Usage: dnet route show\n"
	                "       dnet route get <dst>\n"
	                "       dnet route add <dst> <gw>\n"
			"       dnet route delete <dst>\n");
	exit(1);
}

static int
print_route(const struct route_entry *entry, void *arg)
{
	printf("%-20s %-20s\n",
	    addr_ntoa(&entry->route_dst), addr_ntoa(&entry->route_gw));
	return (0);
}

int
route_main(int argc, char *argv[])
{
	struct route_entry entry;
	route_t *r;
	char *cmd;

	if (argc < 2)
		usage();

	cmd = argv[1];
	
	if ((r = route_open()) == NULL)
		err(1, "route_open");

	if (strcmp(cmd, "show") == 0) {
		printf("%-20s %-20s\n", "Destination", "Gateway");
		if (route_loop(r, print_route, NULL) < 0)
			err(1, "route_loop");
	} else if (strcmp(cmd, "get") == 0) {
		if (addr_aton(argv[2], &entry.route_dst) < 0)
			err(1, "addr_aton");
		if (route_get(r, &entry) < 0)
			err(1, "route_get");
		printf("get %s %s: gateway %s\n",
		    (entry.route_dst.addr_bits < IP_ADDR_BITS) ?
		    "net" : "host", addr_ntoa(&entry.route_dst),
		    addr_ntoa(&entry.route_gw));
	} else if (strcmp(cmd, "add") == 0) {
		if (argc < 4 ||
		    addr_aton(argv[2], &entry.route_dst) < 0 ||
		    addr_aton(argv[3], &entry.route_gw) < 0)
			err(1, "addr_aton");
		if (route_add(r, &entry) < 0)
			err(1, "route_add");
		printf("add %s %s: gateway %s\n",
		    (entry.route_dst.addr_bits < IP_ADDR_BITS) ?
		    "net" : "host", addr_ntoa(&entry.route_dst),
		    addr_ntoa(&entry.route_gw));
	} else if (strcmp(cmd, "delete") == 0) {
		if (addr_aton(argv[2], &entry.route_dst) < 0)
			err(1, "addr_aton");
		if (route_delete(r, &entry) < 0)
			err(1, "route_delete");
		printf("delete %s %s\n",
		    (entry.route_dst.addr_bits < IP_ADDR_BITS) ?
		    "net" : "host", addr_ntoa(&entry.route_dst));
	} else
		usage();
	
	route_close(r);
	
	exit(0);
}

struct mod mod_route = {
	"route",
	MOD_TYPE_KERN,
	route_main
};
