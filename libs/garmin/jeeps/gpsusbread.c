/*
    Decompose an incoming USB packet to make it look like a serial one.

    Copyright (C) 2004, 2006, 2006 Robert Lipe, robertlipe@usa.net

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
#include <ctype.h>
#include "garmin_gps.h"
#include "garminusb.h"
#include "gpsusbint.h"

/*
 * Return values are:
 * Negative on error.
 * 1 if read success - even if empty packet.
 */
int32 GPS_Packet_Read_usb(gpsdevh *dh, GPS_PPacket *packet, int eat_bulk)
{
	int32  n;
	int32 payload_size;

	garmin_usb_packet pkt;

	memset(&pkt, 0, sizeof(pkt));
do_over:
	n = gusb_cmd_get(&pkt, sizeof(pkt));

	if ( n < 0 ) {
		/*
		 * We (probably) used to have a GPS and it went away
	 	 * while we were speaking with it.  Perhaps batteries
		 * died or it was unplugged or something.
		 */
		gps_errno = PROTOCOL_ERROR;
		return n;
	}

	/*
	 * This is a horrible hack for 276/296.   This family sometimes
	 * switches between bulk and interrupt on EVERY packet.   Rather
	 * than bother all the callers with that bit of unpleasantness,
	 * silently consume zero byte "switch back to intr"  packets here.
	 *
	 * The one caller that doesn't want this hidden is device discovery
	 * in the A000 handler.
	 */
	if ((n == 0) && eat_bulk)  {
		goto do_over;
	}

	/* We sometimes get corrupted packets during a track log transfer
	 * where the first byte in a packet is lost, causing all remaining
	 * bytes in this packet to be shifted. So far, this has only been
	 * observed on a Forerunner 305 (both on Linux and Windows). The
	 * cause is unknown, but it seems to be timing dependent.
	 * We try to detect the corruption mainly by checking reserved bytes
	 * 3 and 7 which normally should be 0, the remaining comparisons are
	 * only sanity checks and they alone could also trigger in case of
	 * valid packets. Note: We can't detect corrupted packets with an ID
	 * or length that's a multiple of 256, but such corrupted packets
	 * haven't been observed so far.
	 */
	if (gps_save_id == 484
	    && pkt.gusb_pkt.type == 0 && pkt.gusb_pkt.reserved1 == 0
	    && pkt.gusb_pkt.reserved2 == 0 && pkt.gusb_pkt.reserved3 != 0
	    && pkt.gusb_pkt.pkt_id[0] <= 4 && pkt.gusb_pkt.pkt_id[1] == 0
	    && pkt.gusb_pkt.reserved6 == 0 && pkt.gusb_pkt.reserved7 != 0) {
		memmove(&pkt.dbuf[1], &pkt.dbuf[0], sizeof(pkt) - 1);
		pkt.gusb_pkt.type = 20;
	}

	/*
	 * Populate members of serial packet from USB packet.   The
	 * copy here seems wasteful, but teaching all the callers about
	 * a structure with the "data" member being in a different place
	 * (Since the protocol packets was badly exposed in the core
	 * design of jeeps) is even more painful.
	 */
	(*packet)->type = le_read16(&pkt.gusb_pkt.pkt_id);
	payload_size = le_read32(&pkt.gusb_pkt.datasz);
	if (payload_size<0 || payload_size>MAX_GPS_PACKET_SIZE)
	{
		/* If you get this, the packet might have been corrupted
		 * by the unit. Have a look at the corruption detection
		 * code above.
		 */
		GPS_Error("GPS_Packet_Read_usb: Bad payload size %d", payload_size);
		gps_errno = FRAMING_ERROR;
		return 0;
	}
	(*packet)->n = payload_size;
	memcpy((*packet)->data, &pkt.gusb_pkt.databuf, payload_size);

	return 1;
}
