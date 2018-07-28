/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  NMEA Data Object
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
 *
 *
 *
 *
 */


#ifndef __SERIALDATASTREAM_H__
#define __SERIALDATASTREAM_H__

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled header


#include <wx/datetime.h>


#ifdef __WXGTK__
// newer versions of glib define its own GSocket but we unfortunately use this
// name in our own (semi-)public header and so can't change it -- rename glib
// one instead
//#include <gtk/gtk.h>
#define GSocket GlibGSocket
#include "wx/socket.h"
#undef GSocket
#else
#include "wx/socket.h"
#endif

#ifndef __WXMSW__
#include <sys/socket.h>                 // needed for (some) Mac builds
#include <netinet/in.h>
#endif

#ifdef __WXMSW__
#include <windows.h>
#include <dbt.h>
#include <winioctl.h>
#include <initguid.h>
#endif
#include <string>
#include "ConnectionParams.h"
#include "dsPortType.h"
#include "datastream.h"


class SerialDataStream : public DataStream {
public:
    SerialDataStream(wxEvtHandler *input_consumer,
                     const ConnectionType conn_type,
                     const wxString &Port,
                     const wxString &BaudRate,
                     dsPortType io_select,
                     int priority = 0,
                     bool bGarmin = false,
                     int EOS_type = DS_EOS_CRLF,
                     int handshake_type = DS_HANDSHAKE_NONE) : DataStream(input_consumer,
                                                                          conn_type,
                                                                          Port,
                                                                          BaudRate,
                                                                          io_select,
                                                                          priority,
                                                                          bGarmin,
                                                                          EOS_type,
                                                                          handshake_type) {
        Open();
    }

    SerialDataStream(wxEvtHandler *input_consumer,
                     const ConnectionParams *params) : DataStream(input_consumer, params) {
        Open();
    }

    bool SendSentence( const wxString &sentence ) {
        wxString payload = sentence;
        if( !sentence.EndsWith(_T("\r\n")) )
            payload += _T("\r\n");
        return SendSentenceSerial(payload);
    }
private:
    void Open();
    virtual bool SendSentenceSerial(const wxString &payload);
};

DataStream *makeSerialDataStream(wxEvtHandler *input_consumer,
                                 const ConnectionType conn_type,
                                 const wxString &Port,
                                 const wxString &BaudRate,
                                 dsPortType io_select,
                                 int priority,
                                 bool bGarmin);

#endif // __SERIALDATASTREAM_H__