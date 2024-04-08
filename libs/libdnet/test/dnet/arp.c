/*
 * arp.c
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
arp_usage(void)
{
	fprintf(stderr, "Usage: dnet arp [op|sha|spa|tha|tpa <value>] ...\n"
	                "       dnet arp show\n"
	                "       dnet arp get <host>\n"
	                "       dnet arp add <host> <mac>\n"
	                "       dnet arp delete <host>\n");
	exit(1);
}

static int
print_arp(const struct arp_entry *entry, void *arg)
{
	printf("%s at %s\n", addr_ntoa(&entry->arp_pa),
	    addr_ntoa(&entry->arp_ha));
	return (0);
}

static int
arp_kern_main(int argc, char *argv[])
{
	struct arp_entry entry;
	arp_t *arp;
	char *cmd;

	if (argc < 2)
		arp_usage();
	
	cmd = argv[1];

	if ((arp = arp_open()) == NULL)
		err(1, "arp_open");
	
	if (strcmp(cmd, "show") == 0) {
		if (arp_loop(arp, print_arp, NULL) < 0)
			err(1, "arp_loop");
	} else if (strcmp(cmd, "get") == 0) {
		if (addr_pton(argv[2], &entry.arp_pa) < 0)
			err(1, "addr_pton");
		if (arp_get(arp, &entry) < 0)
			err(1, "arp_get");
		print_arp(&entry, NULL);
	} else if (strcmp(cmd, "add") == 0) {
		if (addr_pton(argv[2], &entry.arp_pa) < 0 ||
		    addr_pton(argv[3], &entry.arp_ha) < 0)
			err(1, "addr_pton");
		if (arp_add(arp, &entry) < 0)
			err(1, "arp_add");
		printf("%s added\n", addr_ntoa(&entry.arp_pa));
	} else if (strcmp(cmd, "delete") == 0) {
		if (addr_pton(argv[2], &entry.arp_pa) < 0)
			err(1, "addr_pton");
		if (arp_delete(arp, &entry) < 0)
			err(1, "arp_delete");
		printf("%s deleted\n", addr_ntoa(&entry.arp_pa));
	} else
		arp_usage();
	
	arp_close(arp);

	return (0);
}

int
arp_main(int argc, char *argv[])
{
	struct arp_hdr *arp;
	struct arp_ethip *ethip;
	struct addr addr;
	u_char *p, buf[ETH_MTU];	/* XXX */
	char *name, *value;
	int c, len;

	if (argc == 1 || *(argv[1]) == '-')
		arp_usage();
	
	/* XXX - total trash */
	if (argc > 1 &&
	    (strcmp(argv[1], "show") == 0 || strcmp(argv[1], "get") == 0 ||
		strcmp(argv[1], "add") == 0 || strcmp(argv[1], "delete") == 0))
		return (arp_kern_main(argc, argv));
	
	srand(time(NULL));

	arp = (struct arp_hdr *)buf;
	arp->ar_hrd = htons(ARP_HRD_ETH);
	arp->ar_pro = htons(ARP_PRO_IP);
	arp->ar_hln = ETH_ADDR_LEN;
	arp->ar_pln = IP_ADDR_LEN;
	arp->ar_op = ARP_OP_REQUEST;

	ethip = (struct arp_ethip *)(buf + ARP_HDR_LEN);
	memset(ethip, 0, sizeof(*ethip));

	for (c = 1; c + 1 < argc; c += 2) {
		name = argv[c];
		value = argv[c + 1];
		
		if (strcmp(name, "op") == 0) {
			if (op_aton(value, &arp->ar_op) < 0)
				arp_usage();
		} else if (strcmp(name, "sha") == 0) {
			if (addr_aton(value, &addr) < 0)
				arp_usage();
			memcpy(ethip->ar_sha, &addr.addr_eth, ETH_ADDR_LEN);
		} else if (strcmp(name, "spa") == 0) {			
			if (addr_aton(value, &addr) < 0)
				arp_usage();
			memcpy(ethip->ar_spa, &addr.addr_ip, IP_ADDR_LEN);
		} else if (strcmp(name, "tha") == 0) {
			if (addr_aton(value, &addr) < 0)
				arp_usage();
			memcpy(ethip->ar_tha, &addr.addr_eth, ETH_ADDR_LEN);
		} else if (strcmp(name, "tpa") == 0) {
			if (addr_aton(value, &addr) < 0)
				arp_usage();
			memcpy(ethip->ar_tpa, &addr.addr_ip, IP_ADDR_LEN);
		}
		else
			arp_usage();
	}
	argc -= c;
	argv += c;

	if (argc != 0)
		arp_usage();

	p = buf + ARP_HDR_LEN + ARP_ETHIP_LEN;
	
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

struct mod mod_arp = {
	"arp",
	MOD_TYPE_ENCAP|MOD_TYPE_KERN,
	arp_main
};
