/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  NMEA NetworkData Stream Object
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************

 ***************************************************************************
 *  Parts of this file were adapted from source code found in              *
 *  John F. Waers (jfwaers@csn.net) public domain program MacGPS45         *
 ***************************************************************************
 *
 */
#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif //precompiled headers

#include "wx/tokenzr.h"
#include <wx/datetime.h>

#include <stdlib.h>
#include <math.h>
#include <time.h>

#ifndef __WXMSW__
#include <arpa/inet.h>
#include <netinet/tcp.h>
#endif

#include <vector>
#include <wx/socket.h>
#include <wx/log.h>
#include <wx/memory.h>
#include <wx/chartype.h>
#include <wx/wx.h>
#include <wx/sckaddr.h>

#include "dychart.h"

#include "datastream.h"
#include "SerialDataStream.h"
#include "OCPN_DataStreamEvent.h"
#include "OCP_DataStreamInput_Thread.h"
#include "GarminProtocolHandler.h"


#if !defined(NAN)
static const long long lNaN = 0xfff8000000000000;
#define NAN (*(double*)&lNaN)
#endif

extern bool g_benableUDPNullHeader;

#define N_DOG_TIMEOUT   5

#ifdef __WXMSW__
// {2C9C45C2-8E7D-4C08-A12D-816BBAE722C0}
DEFINE_GUID(GARMIN_GUID1, 0x2c9c45c2L, 0x8e7d, 0x4c08, 0xa1, 0x2d, 0x81, 0x6b, 0xba, 0xe7, 0x22, 0xc0);
#endif

#ifdef __OCPN__ANDROID__
#include <netdb.h>
int gethostbyaddr_r(const char *, int, int, struct hostent *, char *, size_t, struct hostent **, int *)
{
    wxLogMessage(_T("Called stub gethostbyaddr_r()"));
    return 0;
}
#endif

DataStream *makeSerialDataStream(wxEvtHandler *input_consumer,
                                 const ConnectionType conn_type,
                                 const wxString &Port,
                                 const wxString &BaudRate,
                                 dsPortType io_select,
                                 int priority,
                                 bool bGarmin)
{
    return new SerialDataStream(input_consumer,
                                conn_type,
                                Port,
                                BaudRate,
                                io_select,
                                priority,
                                bGarmin,
                                DS_EOS_CRLF,
                                DS_HANDSHAKE_NONE);
}


void SerialDataStream::Open(void)
{
    wxString comx;
    comx =  GetPort().AfterFirst(':');      // strip "Serial:"

    wxString port_uc = GetPort().Upper();

    if( (wxNOT_FOUND != port_uc.Find(_T("USB"))) && (wxNOT_FOUND != port_uc.Find(_T("GARMIN"))) ) {
        SetGarminProtocolHandler(new GarminProtocolHandler(this, GetConsumer(), true));
    }
    else if(GetGarminMode()) {
        SetGarminProtocolHandler(new GarminProtocolHandler(this, GetConsumer(), false));
    }
    else {
        wxString comx;
        comx =  GetPort().AfterFirst(':');      // strip "Serial:"

        comx = comx.BeforeFirst(' ');               // strip off any description provided by Windows

        //    Kick off the DataSource RX thread
        SetSecondaryThread(new OCP_DataStreamInput_Thread(this,
                                                          GetConsumer(),
                                                          comx, GetBaudRate(),
                                                          GetPortType()));
        SetThreadRunFlag(1);
        GetSecondaryThread()->Run();

        SetOk(true);
    }
}


bool SerialDataStream::SendSentenceSerial(const wxString &payload)
{
    if( GetSecondaryThread() ) {
        if( IsSecThreadActive() )
        {
            int retry = 10;
            while( retry ) {
                if( GetSecondaryThread()->SetOutMsg( payload ))
                    return true;
                else
                    retry--;
            }
            return false;   // could not send after several tries....
        }
        else
            return false;
    }
    return true;
}
