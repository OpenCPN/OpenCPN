/***************************************************************************
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
 **************************************************************************/

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif //precompiled headers

#include "dychart.h"

#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "dychart.h"

#include "wificlient.h"
#include "chart1.h"
#include "statwin.h"

static int              wifi_s_dns_test_flag;


//------------------------------------------------------------------------------
//    WIFI Window Implementation
//------------------------------------------------------------------------------
BEGIN_EVENT_TABLE(WIFIWindow, wxWindow)
        EVT_PAINT(WIFIWindow::OnPaint)
        EVT_ACTIVATE(WIFIWindow::OnActivate)
        EVT_CLOSE(WIFIWindow::OnCloseWindow)

        EVT_SOCKET(WIFI_SOCKET_ID, WIFIWindow::OnSocketEvent)
        EVT_TIMER(TIMER_WIFI1, WIFIWindow::OnTimer1)

        END_EVENT_TABLE()

// A constructor
WIFIWindow::WIFIWindow(wxFrame *frame, const wxString& WiFiServerName):
        wxWindow(frame, wxID_ANY,     wxPoint(20,20), wxSize(5,5), wxSIMPLE_BORDER)

{
    parent_frame = (MyFrame *)frame;
    m_sock = NULL;

    m_pdata_server_string = new wxString(WiFiServerName);

    m_watchtick = 0;
    m_timer_active = false;

//    Decide upon Server source
    wxString msg(_T("WiFi Server is...."));
    msg.Append(*m_pdata_server_string);
    wxLogMessage(msg);

    if(m_pdata_server_string->Contains(_T("TCP/IP")))
      {
          wxString WIFI_data_ip;
          WIFI_data_ip = m_pdata_server_string->Mid(7);         // extract the IP

          if(!WIFI_data_ip.IsEmpty())
          {
// Create the socket
                  m_sock = new wxSocketClient();

// Setup the event handler and subscribe to most events
                m_sock->SetEventHandler(*this, WIFI_SOCKET_ID);

                m_sock->SetNotify(wxSOCKET_CONNECTION_FLAG |
                    wxSOCKET_INPUT_FLAG |
                    wxSOCKET_LOST_FLAG);
                m_sock->Notify(TRUE);

                m_busy = FALSE;

//    Build the target address

//    n.b. Win98
//    wxIPV4address::Hostname() uses sockets function gethostbyname() for address resolution
//    Implications...Either name target must exist in c:\windows\hosts, or
//                            a DNS server must be active on the network.
//    If neither true, then wxIPV4address::Hostname() will block (forever?)....
//
//    Workaround....
//    Use a thread to try the name lookup, in case it hangs

                WIFIDNSTestThread *ptest_thread = NULL;
                ptest_thread = new WIFIDNSTestThread(WIFI_data_ip);

                ptest_thread->Run();                      // Run the thread from ::Entry()


//    Sleep and loop for N seconds
#define SLEEP_TEST_SEC  2

                for(int is=0 ; is<SLEEP_TEST_SEC * 10 ; is++)
                {
                    wxMilliSleep(100);
                    if(wifi_s_dns_test_flag)
                    break;
                }

                if(!wifi_s_dns_test_flag)
                {

                    wxString msg(WIFI_data_ip);
                    msg.Prepend(_T("Could not resolve TCP/IP host '"));
                    msg.Append(_T("'\n Suggestion: Try 'xxx.xxx.xxx.xxx' notation"));
                    OCPNMessageDialog md(this, msg, _T("OpenCPN Message"), wxICON_ERROR );
                    md.ShowModal();

                    m_sock->Notify(FALSE);
                    m_sock->Destroy();

                    return;
                }

                addr.Hostname(WIFI_data_ip);
                addr.Service(SERVER_PORT);

        // It is considered safe to block GUI during socket IO, since WIFI data activity is infrequent
                m_sock->SetFlags(wxSOCKET_WAITALL | wxSOCKET_BLOCK );
                m_sock->Connect(addr, FALSE);       // Non-blocking connect

                    //  Initialize local data stores
                for(int ilocal = 0 ; ilocal < NLOCALSTORE ; ilocal++)
                {
                    station_data[ilocal].bisvalid = false;
                }

                Timer1.SetOwner(this, TIMER_WIFI1);
                m_scan_interval_msec = 10000;
                Timer1.Start(m_scan_interval_msec,wxTIMER_CONTINUOUS);
                m_timer_active = true;
          }         // !Isempty()
      }

      Hide();
}


WIFIWindow::~WIFIWindow()
{
    delete m_pdata_server_string;
}

void WIFIWindow::OnCloseWindow(wxCloseEvent& event)
{
//    Kill off the WIFI Client Socket if alive
    if(m_sock)
    {
        m_sock->Notify(FALSE);
        m_sock->Destroy();
        Timer1.Stop();
    }
}


void WIFIWindow::GetSource(wxString& source)
{
    source = *m_pdata_server_string;
}



void WIFIWindow::OnActivate(wxActivateEvent& event)
{
}

void WIFIWindow::OnPaint(wxPaintEvent& event)
{
    wxPaintDC dc(this);
}

void WIFIWindow::Pause(void)
{
    if(m_timer_active)
        Timer1.Stop();

    if(m_sock)
        m_sock->Notify(FALSE);
}

void WIFIWindow::UnPause(void)
{
    if(m_timer_active)
        Timer1.Start(m_scan_interval_msec,wxTIMER_CONTINUOUS);

    if(m_sock)
        m_sock->Notify(TRUE);
}

///////////////////////////////
void WIFIWindow::OnSocketEvent(wxSocketEvent& event)
{

    wifi_scan_data *pt;
    unsigned char response_type;

    int i, ilocal;
    unsigned char *pbuffer;
    int *pcnt;
    int cnt;
    unsigned char buf[100];
    int pt_eaten[64];

    if(event.GetSocketEvent() == wxSOCKET_INPUT)
    {

//          Read the first 5 bytes of the reply, getting its total type and total length
        m_sock->Read(buf, 5);

            //  Read the rest
        response_type = buf[0];
        int *pint =(int *)(&buf[1]);
        int total_length = *pint;

//  get some memory to read the rest
        pbuffer = (unsigned char*) malloc(total_length * sizeof(unsigned char));

        m_sock->Read(pbuffer, total_length-5);

        switch(response_type - 0x80)
        {
            case 'D' :
                m_bRX = true;                       // reset watchdog
                m_watchtick = 0;

            //  Get the scan results station count
                pcnt = (int *)&pbuffer[0];
                cnt = *pcnt;

                if(cnt > 64)
                    cnt = 64;                       // be safe

            //  Manage the data input
            //  Some setup
                for(i=0 ; i < cnt ; i++)
                    pt_eaten[i] = false;

            //  First, check to see if any input station data is already present in local store
            //  If it is (ESSID matches), then simply update the signal quality, and refresh the age.
            //  Also, flag the fact that the input data has been eaten.

                for(i=0 ; i < cnt ; i++)
                {
                    pt = (wifi_scan_data *)(&pbuffer[(sizeof(int) + i * 256)]);           // skipping the first int
                    if(strlen(pt->ESSID))
                    {
                        for(int ilocal = 0 ; ilocal < NLOCALSTORE ; ilocal++)
                        {
                            if((!strcmp(pt->ESSID, station_data[ilocal].ESSID)) && (station_data[ilocal].bisvalid))
                            {
                                station_data[ilocal].sig_quality = pt->sig_quality;
                                station_data[ilocal].age = -1;
                                pt_eaten[i] = true;
                            }
                        }
                    }
                }

            //  Now, age the local store by one
                for(ilocal = 0 ; ilocal < NLOCALSTORE ; ilocal++)
                    if(station_data[ilocal].bisvalid)
                        station_data[ilocal].age ++;

            //  and free any entries that are over the specified age
                for(ilocal = 0 ; ilocal < NLOCALSTORE ; ilocal++)
                {
                    if((station_data[ilocal].bisvalid) && (station_data[ilocal].age >= N_AGEDEATH))
                    {
                        station_data[ilocal].bisvalid = false;
                        station_data[ilocal].ESSID[0] = 0;
                    }
                }


            //  Now, check to see if any input data is un-eaten
            //  If found, then try to allocate to a local store item
                for(i=0 ; i < cnt ; i++)
                {
                    if(pt_eaten[i] == false)
                    {
                        pt = (wifi_scan_data *)(&pbuffer[(sizeof(int) + i * 256)]);
                        if(strlen(pt->ESSID))
                        {
                            for(ilocal = 0 ; ilocal < NLOCALSTORE ; ilocal++)
                            {
                                if(station_data[ilocal].bisvalid == false)
                                {
                                    strcpy(station_data[ilocal].ESSID, pt->ESSID);
                                    station_data[ilocal].sig_quality = pt->sig_quality;
                                    station_data[ilocal].secure = pt->secure;
                                    station_data[ilocal].bisvalid = true;
                                    station_data[ilocal].age = 0;
                                    pt_eaten[i] = true;
                                    break;
                                }
                            }
                        }
                    }
                }

            //  There may still be un-eaten input data at this point......
            //  For now, ignore it.  If it is real, it will appear as soon as something else dies

            // Finally, send the data to the display window
                for(ilocal = 0 ; ilocal < NLOCALSTORE ; ilocal++)
                {
                    if(station_data[ilocal].bisvalid)
                    {
//                        g_ChartBarWin->pWiFi->SetStationQuality(ilocal, station_data[ilocal].sig_quality);
//                        g_ChartBarWin->pWiFi->SetStationSecureFlag(ilocal, station_data[ilocal].secure);
//                        g_ChartBarWin->pWiFi->SetStationAge(ilocal, station_data[ilocal].age);
                    }
//                    else
//                        g_ChartBarWin->pWiFi->SetStationQuality(ilocal, 0);
                }
            g_ChartBarWin->Refresh(true);

            break;


            case 'S' :
            {
                /*
                StatusString = wxString(&buf[5]);

                        //  This may be useful later....
                fi_status_data *status = (wifi_status_data *)&buf[5];

                memcpy(&connected_ap_mac_addr, &status->currently_connected_ap, sizeof(struct sockaddr));

                        //  Check for re-connect, if needed
                if(StatusString.StartsWith("Not"))
                {
                    if(s_do_reconnect)
                    {
                        time_t tnow = wxDateTime::GetTimeNow();
                        last_connect_seconds = tnow - last_connect_time;

                        do_reconnect();
                    }
                }

                m_statWindow->Refresh();
                */
                break;
            }

            case 'R' :
            {
                /*
                wxString wr(&buf[5]);
                m_logWindow->WriteText(wr);
                long ac_compass, ac_brg_commanded, ac_brg_current, ac_motor_dir;

                        //  Parse the Antenna Controller string
                if(!strncmp((const char *)&buf[5], "ANTC", 4))                // valid string
                {
                    wxStringTokenizer tk(wr, wxT(":"));

                    wxString token = tk.GetNextToken();              // skip ANTC

                    token = tk.GetNextToken();
                    token.ToLong(&ac_compass);                     // compass heading

                    token = tk.GetNextToken();
                    token.ToLong(&ac_brg_commanded);               // last commanded antenna bearing

                    token = tk.GetNextToken();
                    token.ToLong(&ac_brg_current);                 // current antenna brg

                    token = tk.GetNextToken();
                    token.ToLong(&ac_motor_dir);                   // current motor state

                    s_ac_compass       = ac_compass;
                    s_ac_brg_commanded = ac_brg_commanded;
                    s_ac_brg_current   = ac_brg_current;
                    s_ac_motor_dir     = ac_motor_dir;


                    m_antWindow->Refresh();
                }
 */
                break;
            }

            case 'K' :
            {

                break;
            }


            default:
                break;
        }       //switch

        free(pbuffer);

    }       // if


    event.Skip();
}


void WIFIWindow::OnTimer1(wxTimerEvent& event)
{
    Timer1.Stop();

    if(m_sock->IsConnected())
    {
        //      Keep a watchdog on received data
        if(g_ChartBarWin)
        {
            if(m_watchtick++ > WIFI_DOG_TIMEOUT)       // nothing received recently
            {
//                g_ChartBarWin->pWiFi->SetServerStatus(false);
                g_ChartBarWin->Refresh(true);

                // Try to totally reset the socket
                m_sock->Destroy();

                m_sock = new wxSocketClient();
                m_sock->SetEventHandler(*this, WIFI_SOCKET_ID);

                m_sock->SetNotify(wxSOCKET_CONNECTION_FLAG |
                        wxSOCKET_INPUT_FLAG |
                        wxSOCKET_LOST_FLAG);
                m_sock->Notify(TRUE);
                m_sock->SetFlags(wxSOCKET_WAITALL | wxSOCKET_BLOCK );

                m_watchtick = 0;
            }
//            else
//                g_ChartBarWin->pWiFi->SetServerStatus(true);
        }

        unsigned char c = WIFI_TRANSMIT_DATA_EXT;       // and call for more data
        m_sock->Write(&c, 1);
    }
    else                                     // try to connect
    {
        if(g_ChartBarWin)
        {
//            g_ChartBarWin->pWiFi->SetServerStatus(false);
            g_ChartBarWin->Refresh(true);
        }
        m_sock->Connect(addr, FALSE);       // Non-blocking connect
    }

    m_bRX = false;
    Timer1.Start(m_scan_interval_msec,wxTIMER_CONTINUOUS);
}


//-------------------------------------------------------------------------------------------------------------
//
//    A simple thread to test host name resolution without blocking the main thread
//
//-------------------------------------------------------------------------------------------------------------
WIFIDNSTestThread::WIFIDNSTestThread(const wxString &name_or_ip)
{
    m_pip = new wxString(name_or_ip);

    Create();
}

WIFIDNSTestThread::~WIFIDNSTestThread()
{
    delete m_pip;
}


void *WIFIDNSTestThread::Entry()
{
    wifi_s_dns_test_flag = 0;

    wxIPV4address     addr;
    addr.Hostname(*m_pip);                          // this may block forever if DNS is not active

    wifi_s_dns_test_flag = 1;                       // came back OK
    return NULL;
}

