/*
 * aton.c
 *
 * Copyright (c) 2001 Dug Song <dugsong@monkey.org>
 *
 * $Id$
 */

#include "config.h"

#include <sys/types.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dnet.h"
#include "aton.h"

int
type_aton(char *string, uint16_t *type)
{
	u_int i;

	if (strcmp(string, "ip") == 0)
		*type = htons(ETH_TYPE_IP);
	else if (strcmp(string, "arp") == 0)
		*type = htons(ETH_TYPE_ARP);
	else {
		if (sscanf(string, "%i", &i) != 1 || i > 0xffff)
			return (-1);
		*type = htons(i & 0xffff);
	}
	return (0);
}

int
op_aton(char *string, uint16_t *op)
{
	u_int i;

	if (strncasecmp(string, "req", 3) == 0)
		*op = htons(ARP_OP_REQUEST);
	else if (strncasecmp(string, "rep", 3) == 0)
		*op = htons(ARP_OP_REPLY);
	else if (strncasecmp(string, "revreq", 6) == 0)
		*op = htons(ARP_OP_REVREQUEST);
	else if (strncasecmp(string, "revrep", 6) == 0)
		*op = htons(ARP_OP_REVREPLY);
	else {
		if (sscanf(string, "%i", &i) != 1 || i > 0xffff)
			return (-1);
		*op = htons(i & 0xffff);
	}
	return (0);
}

int
proto_aton(char *string, uint8_t *proto)
{
	struct protoent *pp;
	u_int i;
	
	if ((pp = getprotobyname(string)) != NULL)
		*proto = pp->p_proto;
	else {
		if (sscanf(string, "%i", &i) != 1 || i > 0xff)
			return (-1);
		*proto = i & 0xff;
	}
	return (0);
}

int
off_aton(char *string, uint16_t *off)
{
	int i;
	char *p;

	if (strncmp(string, "0x", 2) == 0) {
		if (sscanf(string, "%i", &i) != 1 || i > IP_OFFMASK)
			return (-1);
		*off = htons(i);
	} else {
		i = strtol(string, &p, 10);
		if (*string == '\0' || (*p != '\0' && *p != '+') ||
		    i > (IP_OFFMASK << 3))
			return (-1);
		*off = htons(((*p == '+') ? IP_MF : 0) | (i >> 3));
	}
	return (0);
}

int
port_aton(char *string, uint16_t *port)
{
	struct servent *sp;
	u_int i;
	
	/* XXX */
	if ((sp = getservbyname(string, "tcp")) != NULL) {
		*port = sp->s_port;
	} else if ((sp = getservbyname(string, "udp")) != NULL) {
		*port = sp->s_port;
	} else {
		if (sscanf(string, "%i", &i) != 1 || i > 0xffff)
			return (-1);
		*port = htons(i & 0xffff);
	}
	return (0);
}

int
seq_aton(char *string, uint32_t *seq)
{
	char *p;
	
	*seq = strtoul(string, &p, 10);
	if (*string == '\0' || *p != '\0')
		return (-1);

	*seq = htonl(*seq);
	return (0);
}

int
flags_aton(char *string, uint8_t *flags)
{
	char *p;
	u_int i;

	*flags = 0;
	
	if (strncmp(string, "0x", 2) == 0) {
		if (sscanf(string, "%i", &i) != 1 || i > 0xff)
			return (-1);
		*flags = i & 0xff;
		
		return (0);
	}
	for (p = string; *p != '\0'; p++) {
		switch (*p) {
		case 'S':
			*flags |= TH_SYN;
			break;
		case 'A':
			*flags |= TH_ACK;
			break;
		case 'F':
			*flags |= TH_FIN;
			break;
		case 'R':
			*flags |= TH_RST;
			break;
		case 'P':
			*flags |= TH_PUSH;
			break;
		case 'U':
			*flags |= TH_URG;
			break;
		default:
			return (-1);
		}
	}
	return (0);
}

static u_char
hex2num(char ch, char cl)
{
	ch = tolower(ch), cl = tolower(cl);
	
	if (ch >= '0' && ch <= '9')
		ch -= '0';
	else if (ch >= 'a' && ch <= 'f')
		ch -= 'a' - 10;
	else
		return (0);
	
	if (cl >= '0' && cl <= '9')
		cl -= '0';
	else if (cl >= 'a' && cl <= 'f')
		cl -= 'a' - 10;
	else
		return (0);
	
	return ((u_char)((ch << 4) | cl));
}

int
fmt_aton(char *string, u_char *buf)
{
	u_char *u;
	char *p;

	for (u = buf, p = string; *p != '\0'; p++) {
		if (*p != '\\') {
			*u++ = *p;
			continue;
		}
		if (*++p == '\0')
			break;

		if (*p == '\\')		*u++ = '\\';
		else if (*p == '\'')	*u++ = '\'';
		else if (*p == '"')	*u++ = '"';
		else if (*p == 'a')	*u++ = '\a';
		else if (*p == 'b')	*u++ = '\b';
		else if (*p == 'e')	*u++ = 033;
		else if (*p == 'f')	*u++ = '\f';
		else if (*p == 'n')	*u++ = '\n';
		else if (*p == 'r')	*u++ = '\r';
		else if (*p == 't')	*u++ = '\t';
		else if (*p == 'v')	*u++ = '\v';
		else if (*p == 'x' && p[1] != '\0' && p[2] != '\0') {
			*u++ = hex2num(p[1], p[2]);
			p += 2;
		} else
			return (-1);
	}
	return (u - buf);
}
