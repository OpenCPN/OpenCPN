/*
 * fw.c
 *
 * Copyright (c) 2001 Dug Song <dugsong@monkey.org>
 * Copyright (c) 2020 Oliver Falk <oliver@linux-kernel.at>
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

#include "dnet.h"
#include "mod.h"

static void
usage(void)
{
	fprintf(stderr, "Usage: dnet fw show\n"
	                "       dnet fw add|delete allow|block in|out "
	    "<device>|any <proto> <src>[:<sport>[-<max>]] "
	    "<dst>[:<dport>[-<max>]] [<type>[/<code>]]\n");
	exit(1);
}

static int
print_rule(const struct fw_rule *fr, void *arg)
{
	struct protoent *pr;
	char proto[16], sport[32], dport[32], typecode[16];

	if (fr->fw_proto == 0)
		proto[0] = '\0';
	else if ((pr = getprotobynumber(fr->fw_proto)) == NULL)
		snprintf(proto, sizeof(proto), "%d ", fr->fw_proto);
	else
		snprintf(proto, sizeof(proto), "%s ", pr->p_name);

	sport[0] = dport[0] = typecode[0] = '\0';
	
	switch (fr->fw_proto) {
	case IP_PROTO_ICMP:
		if (fr->fw_sport[1] && fr->fw_dport[1]) 
			snprintf(typecode, sizeof(typecode), " %d/%d",
			    fr->fw_sport[0], fr->fw_dport[0]);
		else if (fr->fw_sport[1])
			snprintf(typecode, sizeof(typecode), " %d",
			    fr->fw_sport[0]);
		break;
	case IP_PROTO_TCP:
	case IP_PROTO_UDP:
		if (fr->fw_sport[0] == fr->fw_sport[1]) {
			snprintf(sport, sizeof(sport), ":%d", fr->fw_sport[0]);
		} else
			snprintf(sport, sizeof(sport), ":%d-%d",
			    fr->fw_sport[0], fr->fw_sport[1]);
		
		if (fr->fw_dport[0] == fr->fw_dport[1]) {
			snprintf(dport, sizeof(dport), ":%d", fr->fw_dport[0]);
		} else
			snprintf(dport, sizeof(dport), ":%d-%d",
			    fr->fw_dport[0], fr->fw_dport[1]);
		break;
	}
	printf("%s %s %s %s%s%s %s%s%s\n",
	    fr->fw_op == FW_OP_ALLOW ? "allow" : "block",
	    fr->fw_dir == FW_DIR_IN ? "in" : "out",
	    *fr->fw_device ? fr->fw_device : "any",
	    proto,
	    fr->fw_src.addr_type ? addr_ntoa(&fr->fw_src) : "",
	    sport,
	    fr->fw_dst.addr_type ? addr_ntoa(&fr->fw_dst) : "",
	    dport,
	    typecode);

	return (0);
}

static int
arg_to_fr(int argc, char *argv[], struct fw_rule *fr)
{
	struct protoent *pr;
	char *p;

	if (argc < 6) {
		errno = EINVAL;
		return (-1);
	}
	memset(fr, 0, sizeof(*fr));

	fr->fw_op = strcmp(argv[0], "allow") ? FW_OP_BLOCK : FW_OP_ALLOW;
	
	fr->fw_dir = strcmp(argv[1], "in") ? FW_DIR_OUT : FW_DIR_IN;

	if (strcmp(argv[2], "any") != 0)
		strncpy(fr->fw_device, argv[2], sizeof(fr->fw_device)-1);
	
	if ((pr = getprotobyname(argv[3])) != NULL)
		fr->fw_proto = pr->p_proto;
	else
		fr->fw_proto = atoi(argv[3]);

	p = strtok(argv[4], ":");
	
	if (!p || addr_aton(p, &fr->fw_src) < 0)
		return (-1);

	if ((p = strtok(NULL, ":")) != NULL) {
		fr->fw_sport[0] = (uint16_t)strtol(p, &p, 10);
		if (*p == '-')
			fr->fw_sport[1] = (uint16_t)strtol(p + 1, NULL, 10);
		else
			fr->fw_sport[1] = fr->fw_sport[0];
	} else if (fr->fw_proto == IP_PROTO_TCP || fr->fw_proto == IP_PROTO_UDP) {
		fr->fw_sport[0] = 0;
		fr->fw_sport[1] = TCP_PORT_MAX;
	}
	p = strtok(argv[5], ":");
	
	if (addr_aton(p, &fr->fw_dst) < 0)
		return (-1);

	if ((p = strtok(NULL, ":")) != NULL) {
		fr->fw_dport[0] = (uint16_t)strtol(p, &p, 10);
		if (*p == '-')
			fr->fw_dport[1] = (uint16_t)strtol(p + 1, NULL, 10);
		else
			fr->fw_dport[1] = fr->fw_dport[0];
	} else if (fr->fw_proto == IP_PROTO_TCP || fr->fw_proto == IP_PROTO_UDP) {
		fr->fw_dport[0] = 0;
		fr->fw_dport[1] = TCP_PORT_MAX;
	}	
	if (argc > 6) {
		if (fr->fw_proto != IP_PROTO_ICMP &&
		    fr->fw_proto != IP_PROTO_IGMP) {
			errno = EINVAL;
			return (-1);
		}
		fr->fw_sport[0] = (uint16_t)strtol(argv[6], &p, 10);
		fr->fw_sport[1] = 0xff;
		if (*p == '/') {
			fr->fw_dport[0] = (uint16_t)strtol(p + 1, NULL, 10);
			fr->fw_dport[1] = 0xff;
		}
	}
	return (0);
}

int
fw_main(int argc, char *argv[])
{
	struct fw_rule fr;
	fw_t *fw;
	
	if (argc < 2 || *(argv[1]) == '-')
		usage();

	if ((fw = fw_open()) == NULL)
		err(1, "fw_open");
	
	if (argc == 2 && strcmp(argv[1], "show") == 0) {
		if (fw_loop(fw, print_rule, NULL) < 0)
			err(1, "fw_loop");
	} else if (argc > 2 && strcmp(argv[1], "add") == 0) {
		if (arg_to_fr(argc - 2, argv + 2, &fr) < 0)
			err(1, "arg_to_fr");
		printf("+ ");
		print_rule(&fr, NULL);
		if (fw_add(fw, &fr) < 0)
			err(1, "fw_add");
	} else if (argc > 2 && strcmp(argv[1], "delete") == 0) {
		if (arg_to_fr(argc - 2, argv + 2, &fr) < 0)
			err(1, "arg_to_fr");
		printf("- ");
		print_rule(&fr, NULL);
		if (fw_delete(fw, &fr) < 0)
			err(1, "fw_delete");
	} else
		usage();
	
	fw_close(fw);

	exit(0);
}

struct mod mod_fw = {
	"fw",
	MOD_TYPE_KERN,
	fw_main
};
