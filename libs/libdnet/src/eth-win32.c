/*
 * eth-win32.c
 *
 * Copyright (c) 2000 Dug Song <dugsong@monkey.org>
 *
 * $Id$
 */

#include "config.h"

/* XXX - VC++ 6.0 bogosity */
#define sockaddr_storage sockaddr
#include <Packet32.h>
#undef sockaddr_storage
#include <Ntddndis.h>

#include <errno.h>
#include <stdlib.h>

#include "dnet.h"

struct eth_handle {
	LPADAPTER	 lpa;
	LPPACKET	 pkt;
};

eth_t *
eth_open(const char *device)
{
	eth_t *eth;
	intf_t *intf;
	struct intf_entry ifent;
	eth_addr_t ea;
	char *p, *buf;
	ULONG len;

	/* Get interface entry. */
	memset(&ifent, 0, sizeof(ifent));
	if ((intf = intf_open()) != NULL) {
		strlcpy(ifent.intf_name, device, sizeof(ifent.intf_name));
		intf_get(intf, &ifent);
		intf_close(intf);
	}
	if (ifent.intf_link_addr.addr_type != ADDR_TYPE_ETH)
		return (NULL);

	/* Get Packet driver adapter name/desc lists. */
	buf = NULL;
	PacketGetAdapterNames(buf, &len);
	if (len > 0 && (buf = malloc(len)) != NULL) {
		if (!PacketGetAdapterNames(buf, &len)) {
			free(buf);
			buf = NULL;
		}
	}
	if (buf == NULL)
		return (NULL);
	
	/* XXX - find adapter with matching interface MAC address. */
	if ((eth = calloc(1, sizeof(*eth))) == NULL) {
		free(buf);
		return (NULL);
	}
	for (p = buf; *p != '\0'; p += strlen(p) + 1) {
		if ((eth->lpa = PacketOpenAdapter(p)) != NULL) {
			if (eth->lpa->hFile != INVALID_HANDLE_VALUE &&
			    eth_get(eth, &ea) == 0 &&
			    memcmp(&ea, &ifent.intf_link_addr.addr_eth,
				ETH_ADDR_LEN) == 0) {
				PacketSetBuff(eth->lpa, 512000);
				eth->pkt = PacketAllocatePacket();
				break;
			}
			PacketCloseAdapter(eth->lpa);
		}
	}
	free(buf);
	if (eth->pkt == NULL)
		eth = eth_close(eth);
	
	return (eth);
}

ssize_t
eth_send(eth_t *eth, const void *buf, size_t len)
{
	PacketInitPacket(eth->pkt, (void *)buf, len);
	PacketSendPacket(eth->lpa, eth->pkt, TRUE);
	return (len);
}

eth_t *
eth_close(eth_t *eth)
{
	if (eth != NULL) {
		if (eth->pkt != NULL)
			PacketFreePacket(eth->pkt);
		if (eth->lpa != NULL)
			PacketCloseAdapter(eth->lpa);
		free(eth);
	}
	return (NULL);
}

int
eth_get(eth_t *eth, eth_addr_t *ea)
{
	PACKET_OID_DATA *data;
	u_char buf[512];

	data = (PACKET_OID_DATA *)buf;
	data->Oid = OID_802_3_CURRENT_ADDRESS;
	data->Length = ETH_ADDR_LEN;

	if (PacketRequest(eth->lpa, FALSE, data) == TRUE) {
		memcpy(ea, data->Data, ETH_ADDR_LEN);
		return (0);
	}
	return (-1);
}

int
eth_set(eth_t *eth, const eth_addr_t *ea)
{
	PACKET_OID_DATA *data;
	u_char buf[512];

	data = (PACKET_OID_DATA *)buf;
	data->Oid = OID_802_3_CURRENT_ADDRESS;
	memcpy(data->Data, ea, ETH_ADDR_LEN);
	data->Length = ETH_ADDR_LEN;
	
	if (PacketRequest(eth->lpa, TRUE, data) == TRUE)
		return (0);
	
	return (-1);
}
