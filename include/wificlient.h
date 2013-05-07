/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  wifi Client Data Object
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
 **************************************************************************/

#ifndef __WIFICLIENT_H__
#define __WIFICLIENT_H__


// Include wxWindows' headers

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif //precompiled header

#include "dychart.h"

#include "wx/socket.h"

#ifdef __WXMSW__
#include <wx/datetime.h>
#endif

//----------------------------------------------------------------------------
//   constants
//----------------------------------------------------------------------------
#define WIFI_TRANSMIT_DATA        0x42          //This is the request code
#define WIFI_TRANSMIT_DATA_EXT    'D'          //Extended request code


#define NLOCALSTORE           4
#define N_AGEDEATH            5
// Class declarations


// The MY_FILETIME structure is a 64-bit value
//    representing the number of 100-nanosecond
//    intervals since January 1, 1601 (UTC).
// This is the format used in the NMEA server data packet
//    sigh....

typedef struct  {
    unsigned int low;
    unsigned int high;
} WiFiMyFileTime;

class MyFrame;

//  A local structure for managing station scanning
typedef struct
{
    char        ESSID[64];
    int         sig_quality;
    int         secure;
    bool        bisvalid;
    int         age;

} wifi_local_scan_data;

//----------------------------------------------------------------------------
// WIFIWindow
//----------------------------------------------------------------------------

class WIFIWindow: public wxWindow
{
    public:
        WIFIWindow(wxFrame *frame, const wxString& WiFiServerName);
        ~WIFIWindow();

        void GetSource(wxString & source);

      //    Stop/Start the Socket Client
      //    Used to prevent async interrupts at critical times
        void Pause(void);
        void UnPause(void);


    private:
        void OnPaint(wxPaintEvent& event);
        void OnActivate(wxActivateEvent& event);
        void OnSocketEvent(wxSocketEvent& event);
        void OnTimer1(wxTimerEvent& event);
        void OnCloseWindow(wxCloseEvent& event);
        void wxDTToMyFileTime(wxDateTime *SDT, WiFiMyFileTime *pFileTime);
        void MyFileTimeTowxDT( WiFiMyFileTime *pFileTime, wxDateTime *SDT);

        wxIPV4address     addr;
        wxSocketClient    *m_sock;
        bool              m_busy;
        wxTimer           Timer1;
        MyFrame           *parent_frame;
        bool              m_bRX;
        wxString          *m_pdata_server_string;
        int               m_watchtick;
        int               m_scan_interval_msec;
        bool              m_timer_active;

        wifi_local_scan_data    station_data[NLOCALSTORE];

        DECLARE_EVENT_TABLE()
};


typedef struct _WIFI_DATA_MSG1
{
    int         msg;
    long        time;             // UNIX 64 bit time
    long        time1;
} WIFI_DATA_MSG1;


//-------------------------------------------------------------------------------------------------------------
//
//      WiFi Server Data Definitions
//
//-------------------------------------------------------------------------------------------------------------

//      WiFi server produces messages composed of wifi_scan_data structures
//      in a byte buffer, on 256 byte boundaries.
//      This allows extension of the data structures without radical changes to server protocol


typedef struct
{
    char        ESSID[64];
    int         sig_quality;
    int         secure;
    int         channel;
    sockaddr    ap_addr;
    int         key_flags;
    unsigned char mode;
} wifi_scan_data;

#define SERVER_PORT          3000           // the wifid tcp/ip socket server port

#define WIFI_DOG_TIMEOUT 5


//-------------------------------------------------------------------------------------------------------------
//
//    A simple thread to test host name resolution without blocking the main thread
//
//-------------------------------------------------------------------------------------------------------------
class WIFIDNSTestThread: public wxThread
{
    public:

        WIFIDNSTestThread(const wxString &name_or_ip);
        ~WIFIDNSTestThread();
        void *Entry();


    private:
        wxString *m_pip;
};


#endif
