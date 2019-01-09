/*
    Garmin USB layer - OS independent component.

    Copyright (C) 2006 Robert Lipe, robertlipe@usa.net

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02111 USA

 */

#include "garmin_gps.h"
#include "garminusb.h"
#include "gpsusbcommon.h"

/*
 * This receive logic is a little convoluted as we go to some efforts here
 * to hide most of the differences between the bulk only and bulk-interrupt
 * protocols as exhibited in the handhelds and dashtops.
 */

enum {
	rs_fromintr,
	rs_frombulk
} receive_state;

static gusb_llops_t *gusb_llops;

/* Decide when to truncate packets for debug output */
#define DEBUG_THRESH  ((global_opts.debug_level < 5) && (i > 10))

/* Called from OS layer to register its low-level entry points. */
void
gusb_register_ll(gusb_llops_t *p)
{
	gusb_llops = p;
}

int
gusb_close(gpsdevh *dh)
{
	garmin_usb_packet scratch;

	memset(&scratch, 0, sizeof(scratch));

	switch (receive_state) {
	case rs_frombulk:
		gusb_cmd_get(&scratch, sizeof(scratch));
		break;
	default:
		break;
	}

	gusb_llops->llop_close(dh);
	return 1;

#if BOOGER
	garmin_usb_packet scratch = {0};
abort();
	switch (receive_state) {
	case rs_frombulk:
		gusb_cmd_get(dh, &scratch, sizeof(scratch));
		break;
	}

	return 1;
#endif
}


int
gusb_cmd_get(garmin_usb_packet *ibuf, size_t sz)
{
	int rv = 0;
	unsigned char *buf = (unsigned char *) &ibuf->dbuf;
	int orig_receive_state;
	unsigned short pkt_id;
top:
	orig_receive_state = receive_state;
	switch (receive_state) {
	case rs_fromintr:
		rv = gusb_llops->llop_get_intr(ibuf, sz);
		break;
	case rs_frombulk:
		rv = gusb_llops->llop_get_bulk(ibuf, sz);
		break;
	default:
		fatal("Unknown receiver state %d\n", receive_state);
	}

	pkt_id = le_read16(&ibuf->gusb_pkt.pkt_id);
	if (gps_show_bytes) {
		int i;
		const char *m1, *m2;
		unsigned short pkttype = le_read16(&ibuf->gusb_pkt.databuf[0]);

		GPS_Diag("RX (%s) [%d]:",
			receive_state == rs_fromintr ? "intr" : "bulk", rv);

		for(i=0;i<rv;i++) {
/*dsr
			if (DEBUG_THRESH) {
				GPS_Diag("[...]");
				break;
			}
*/
			GPS_Diag("%02x ", buf[i]);
		}

		for(i=0;i<rv;i++) {
/*dsr
			if (DEBUG_THRESH) {
				GPS_Diag("[...]");
				break;
			}
*/
			GPS_Diag("%c", isalnum(buf[i])? buf[i] : '.');
		}

		m1 = Get_Pkt_Type(pkt_id, pkttype, &m2);
if ((rv == 0)  &&  (receive_state == rs_frombulk) ) {m1= "RET2INTR";m2=NULL;};
		GPS_Diag("(%-8s%s)\n", m1, m2 ? m2 : "");
	}

	/* Adjust internal state and retry the read */
	if ((rv > 0) && (pkt_id == GUSB_REQUEST_BULK)) {
		receive_state = rs_frombulk;
		goto top;
	}
	/*
	 * If we were reading from the bulk pipe and we just got
	 * a zero request, adjust our internal state.
	 * It's tempting to retry the read here to hide this "stray"
	 * packet from our callers, but that only works when you know
	 * there's another packet coming.   That works in every case
	 * except the A000 discovery sequence.
	*/
	if ((receive_state == rs_frombulk) && (rv <= 0)) {
		receive_state = rs_fromintr;
	}

	return rv;
}

int
gusb_cmd_send(const garmin_usb_packet *opkt, size_t sz)
{
	unsigned int rv, i;

	unsigned char *obuf = (unsigned char *) &opkt->dbuf;
	const char *m1, *m2;

	rv = gusb_llops->llop_send(opkt, sz);

	if (gps_show_bytes) {
		const unsigned short pkttype = le_read16(&opkt->gusb_pkt.databuf[0]);
		const unsigned short pkt_id = le_read16(&opkt->gusb_pkt.pkt_id);
		GPS_Diag("TX [%d]:", sz);

		for(i=0;i<sz;i++)
			GPS_Diag("%02x ", obuf[i]);

		for(i=0;i<sz;i++)
			GPS_Diag("%c", isalnum(obuf[i])? obuf[i] : '.');

		m1 = Get_Pkt_Type(pkt_id, pkttype, &m2);

		GPS_Diag("(%-8s%s)\n", m1, m2 ? m2 : "");
        }
	/*
	 * Recursion, when used in a disciplined way, can be our friend.
	 *
	 * The Garmin protocol requires that packets that are exactly
	 * a multiple of the max tx size be followed by a zero length
	 * packet.  Do that here so we can see it in debugging traces.
	 */

	if (sz && !(sz % gusb_llops->max_tx_size)) {
		gusb_cmd_send(opkt, 0);
	}

	return (rv);
}

void
gusb_list_units()
{
	int i;

	for (i = 0; i < GUSB_MAX_UNITS; i++) {
		if (garmin_unit_info[i].serial_number) {
			printf("%d %lu %lu %s\n", i,
				garmin_unit_info[i].serial_number,
				garmin_unit_info[i].unit_id,
				garmin_unit_info[i].product_identifier
			);
		}
	}
}

void
gusb_id_unit(struct garmin_unit_info *gu)
{
	static const char  oid[12] =
		{20, 0, 0, 0, 0xfe, 0, 0, 0, 0, 0, 0, 0};
	garmin_usb_packet iresp;
	int i;

	gusb_cmd_send((garmin_usb_packet *)oid, sizeof(oid));

	for (i = 0; i < 25; i++) {
		iresp.gusb_pkt.type = 0;
		if (gusb_cmd_get(&iresp, sizeof(iresp)) < 0) {
			return;
		}
		if (le_read16(iresp.gusb_pkt.pkt_id) == 0xff) {
			gu->product_identifier = xstrdup((char *) iresp.gusb_pkt.databuf+4);
			gu->unit_id = le_read16(iresp.gusb_pkt.databuf+0);
			gu->unit_version = le_read16(iresp.gusb_pkt.databuf+2);
		}
		/*
		 * My goodnesss, this is fragile.  During command syncup,
		 * we need to know if we're at the end.  The 0xfd packet
		 * is promised by Garmin engineering to be the last.
		 */
		if (le_read16(iresp.gusb_pkt.pkt_id) == 0xfd) return;
	}
	fatal("Unable to sync with Garmin USB device in %d attempts.", i);
}

void
gusb_syncup(void)
{
	static int unit_number;
	static const char  oinit[12] =
		{0, 0, 0, 0, GUSB_SESSION_START, 0, 0, 0, 0, 0, 0, 0};
	garmin_usb_packet iresp;
	int i;

	/*
	 * This is our first communication with the unit.
	 */
	receive_state = rs_fromintr;

	for(i = 0; i < 25; i++) {
		le_write16(&iresp.gusb_pkt.pkt_id, 0);
		le_write32(&iresp.gusb_pkt.datasz, 0);
		le_write32(&iresp.gusb_pkt.databuf, 0);

		gusb_cmd_send((const garmin_usb_packet *) oinit, sizeof(oinit));
		gusb_cmd_get(&iresp, sizeof(iresp));

		if ((le_read16(iresp.gusb_pkt.pkt_id) == GUSB_SESSION_ACK) &&
			(le_read32(iresp.gusb_pkt.datasz) == 4)) {
			unsigned serial_number = le_read32(iresp.gusb_pkt.databuf);
			garmin_unit_info[unit_number].serial_number = serial_number;
			gusb_id_unit(&garmin_unit_info[unit_number]);

			unit_number++;

			return;
		}
	}
	fatal("Unable to establish USB syncup\n");
}
