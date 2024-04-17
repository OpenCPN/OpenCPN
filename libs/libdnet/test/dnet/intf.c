/*
 * intf.c
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

static intf_t	*intf;

static void
usage(void)
{
	fprintf(stderr, "Usage: dnet intf show\n"
	    "       dnet intf get <name>\n"
	    "       dnet intf set <name> "
	    "[alias|dst|inet|link <addr> ...] [up|down|arp|noarp ...]\n"
	    "       dnet intf src <ip>\n"
	    "       dnet intf dst <ip>\n"
);
	exit(1);
}

static char *
flags2string(u_short flags)
{
	static char buf[256];

	buf[0] = '\0';
	
	if (flags & INTF_FLAG_UP)
		strlcat(buf, ",UP", sizeof(buf));
	if (flags & INTF_FLAG_LOOPBACK)
		strlcat(buf, ",LOOPBACK", sizeof(buf));
	if (flags & INTF_FLAG_POINTOPOINT)
		strlcat(buf, ",POINTOPOINT", sizeof(buf));
	if (flags & INTF_FLAG_NOARP)
		strlcat(buf, ",NOARP", sizeof(buf));
	if (flags & INTF_FLAG_BROADCAST)
		strlcat(buf, ",BROADCAST", sizeof(buf));
	if (flags & INTF_FLAG_MULTICAST)
		strlcat(buf, ",MULTICAST", sizeof(buf));
	
	if (buf[0] != '\0')
		return (buf + 1);

	return (buf);
}

static int
print_intf(const struct intf_entry *entry, void *arg)
{
	int i;
	
	printf("%s:", entry->intf_name);
	
	printf(" flags=0x%x<%s>", entry->intf_flags,
	    flags2string(entry->intf_flags));
	
	if (entry->intf_mtu != 0)
		printf(" mtu %d", entry->intf_mtu);

	printf("\n");
	
	if (entry->intf_addr.addr_type == ADDR_TYPE_IP) {
		if (entry->intf_dst_addr.addr_type == ADDR_TYPE_IP) {
			printf("\tinet %s --> %s\n",
			    addr_ntoa(&entry->intf_addr),
			    addr_ntoa(&entry->intf_dst_addr));
		} else
			printf("\tinet %s\n", addr_ntoa(&entry->intf_addr));
	}
	if (entry->intf_link_addr.addr_type == ADDR_TYPE_ETH)
		printf("\tlink %s\n", addr_ntoa(&entry->intf_link_addr));

	for (i = 0; i < entry->intf_alias_num; i++)
		printf("\talias %s\n", addr_ntoa(&entry->intf_alias_addrs[i]));
	
	return (0);
}

int
intf_main(int argc, char *argv[])
{
	struct intf_entry *entry;
	struct addr *ap, addr;
	char *cmd, buf[1024];
	
	if (argc < 2)
		usage();

	cmd = argv[1];
	
	entry = (struct intf_entry *)buf;
	memset(entry, 0, sizeof(*entry));
	entry->intf_len = sizeof(buf);

	if ((intf = intf_open()) == NULL)
		err(1, "intf_open");

	if (strcmp(cmd, "show") == 0) {
		if (intf_loop(intf, print_intf, NULL) < 0)
			err(1, "intf_loop");
	} else if (strcmp(cmd, "get") == 0) {
		if (argc < 3)
			usage();
		strlcpy(entry->intf_name, argv[2], sizeof(entry->intf_name));
		if (intf_get(intf, entry) < 0)
			err(1, "intf_get");
		print_intf(entry, NULL);
	} else if (strcmp(cmd, "src") == 0) {
		if (argc < 3 || addr_aton(argv[2], &addr) < 0)
			usage();
		if (intf_get_src(intf, entry, &addr) < 0)
			err(1, "intf_get_src");
		print_intf(entry, NULL);
	} else if (strcmp(cmd, "dst") == 0) {
		if (argc < 3 || addr_aton(argv[2], &addr) < 0)
			usage();
		if (intf_get_dst(intf, entry, &addr) < 0)
			err(1, "intf_get_dst");
		print_intf(entry, NULL);
	} else if (strcmp(cmd, "set") == 0) {
		if (argc < 4)
			usage();
		
		strlcpy(entry->intf_name, argv[2], sizeof(entry->intf_name));

		for (argv += 3, argc -= 3; argc > 1; argv += 2, argc -= 2) {
			if (strcmp(argv[0], "alias") == 0) {
				ap = &entry->intf_alias_addrs
				    [entry->intf_alias_num++];
			} else if (strcmp(argv[0], "dst") == 0) {
				ap = &entry->intf_dst_addr;
			} else if (strcmp(argv[0], "inet") == 0) {
				ap = &entry->intf_addr;
			} else if (strcmp(argv[0], "link") == 0) {
				ap = &entry->intf_link_addr;
			} else
				break;
			
			if (addr_pton(argv[1], ap) < 0)
				err(1, "invalid address: %s", argv[1]);
		}
		for ( ; argc > 0; argv++, argc--) {
			if (strcmp(argv[0], "up") == 0)
				entry->intf_flags |= INTF_FLAG_UP;
			else if (strcmp(argv[0], "down") == 0)
				entry->intf_flags &= ~INTF_FLAG_UP;
			else if (strcmp(argv[0], "arp") == 0)
				entry->intf_flags &= ~INTF_FLAG_NOARP;
			else if (strcmp(argv[0], "noarp") == 0)
				entry->intf_flags |= INTF_FLAG_NOARP;
			else
				usage();
		}
		if (intf_set(intf, entry) < 0)
			err(1, "intf_set");
	} else
		usage();
	
	intf_close(intf);

	exit(0);
}

struct mod mod_intf = {
	"intf",
	MOD_TYPE_KERN,
	intf_main
};
