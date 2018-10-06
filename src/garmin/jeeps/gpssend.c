/********************************************************************
** @source JEEPS packet construction, sending and ack functions
**
** @author Copyright (C) 1999 Alan Bleasby
** @version 1.0
** @modified Dec 28 1999 Alan Bleasby. First version
** @modified Copyright (C) 2006 Robert Lipe
** @@
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** You should have received a copy of the GNU Library General Public
** License along with this library; if not, write to the
** Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
** Boston, MA 02110-1301,  USA.
********************************************************************/
#include "garmin_gps.h"
#include "gpsserial.h"

#include <stdio.h>
#include <errno.h>
#include <ctype.h>


/* @funcstatic Build_Serial_Packet *************************************
**
** Forms a complete packet to send on serial port
*
** @param [r] in [GPS_PPacket *] packet string with portable packet data
** @param [w] out [GPS_Serial_PPacket *] packet string suitable for serial port
**
** @return [US] number of data bytes to send
************************************************************************/
static US
Build_Serial_Packet(GPS_PPacket in, GPS_Serial_PPacket out)
{
    UC *p;
    UC *q;

    uint32 i;
    UC  chk=0;
    US  bytes=0;

    p = in->data;
    q = out->data;

    out->dle   = DLE;
    out->edle  = DLE;
    out->etx   = ETX;
    out->n     = (UC)in->n;
    out->type  = (UC)in->type;

    chk -= in->type;

    if(in->n == DLE)
    {
	++bytes;
	*q++ = DLE;
    }

    chk -= (UC)in->n;

    for(i = 0; i < in->n; ++i)
    {
	if(*p == DLE)
	{
	    ++bytes;
	    *q++ = DLE;
	}
	chk -= *p;
	*q++ = *p++;
	++bytes;
    }

    if(chk == DLE)
    {
	*q++ = DLE;
	++bytes;
    }

    out->chk = chk;

    return bytes;
}


void
Diag(void *buf, size_t sz)
{
	unsigned char *cbuf = (unsigned char *) buf;
	while (sz--) {
		GPS_Diag("%02x ", *cbuf++);
	}
}

void
DiagS(void *buf, size_t sz)
{
	unsigned char *cbuf = (unsigned char *) buf;

	while (sz--) {
		unsigned char c = *cbuf++;
		GPS_Diag("%c", isalnum(c) ? c : '.');
	}
}

/* @func GPS_Write_Packet ***********************************************
**
** Forms a complete packet to send
**
** @param [w] fd [int32] file descriptor
** @param [r] packet [GPS_PPacket] packet
**
** @return [int32] number of bytes in the packet
************************************************************************/

int32 GPS_Serial_Write_Packet(gpsdevh *fd, GPS_PPacket packet)
{
    size_t ret;
    const char *m1, *m2;
    GPS_Serial_OPacket ser_pkt;
    UC ser_pkt_data[MAX_GPS_PACKET_SIZE * sizeof(UC)];
    US bytes;
    
    if(!fd)
        return 0;

    if (packet->type >= 0xff || packet->n >= 0xff) {
	GPS_Error("SEND: Unsupported packet type/size for serial protocol");
        return 0;
    }

    ser_pkt.data = ser_pkt_data;
    bytes = Build_Serial_Packet(packet, &ser_pkt);

    GPS_Diag("Tx Data:");
    Diag(&ser_pkt.dle, 3);
    if((ret=GPS_Serial_Write(fd,(const void *) &ser_pkt.dle,(size_t)3)) == -1)
    {
	perror("write");
	GPS_Error("SEND: Write to GPS failed");
	return 0;
    }
    if(ret!=3)
    {
	GPS_Error("SEND: Incomplete write to GPS");
	return 0;
    }

    Diag(ser_pkt.data, bytes);
    if((ret=GPS_Serial_Write(fd,(const void *)ser_pkt.data,(size_t)bytes)) == -1)
    {
	perror("write");
	GPS_Error("SEND: Write to GPS failed");
	return 0;
    }
    if(ret!=bytes)
    {
	GPS_Error("SEND: Incomplete write to GPS");
	return 0;
    }


    Diag(&ser_pkt.chk, 3);

    GPS_Diag(": ");
    DiagS(ser_pkt.data, bytes);
    DiagS(&ser_pkt.chk, 3);
    m1 = Get_Pkt_Type(ser_pkt.type, ser_pkt.data[0], &m2);
    GPS_Diag("(%-8s%s)\n", m1, m2 ? m2 : "");

    if((ret=GPS_Serial_Write(fd,(const void *)&ser_pkt.chk,(size_t)3)) == -1)
    {
	perror("write");
	GPS_Error("SEND: Write to GPS failed");
	return 0;
    }
    if(ret!=3)
    {
	GPS_Error("SEND: Incomplete write to GPS");
	return 0;
    }


    return 1;
}


/* @func GPS_Send_Ack ***********************************************
**
** Send an acknowledge packet
**
** @param [w] fd [int32] file descriptor
** @param [r] tra [GPS_PPacket *] packet to transmit
** @param [r] rec [GPS_PPacket *] last packet received
**
** @return [int32] success
************************************************************************/

int32 GPS_Serial_Send_Ack(gpsdevh *fd, GPS_PPacket *tra, GPS_PPacket *rec)
{
    UC data[2];

    GPS_Util_Put_Short(data,(US)(*rec)->type);
    GPS_Make_Packet(tra,LINK_ID[0].Pid_Ack_Byte,data,2);
    if(!GPS_Write_Packet(fd,*tra))
    {
	GPS_Error("Error acknowledging packet");
	gps_errno = SERIAL_ERROR;
	return 0;
    }

    return 1;
}
