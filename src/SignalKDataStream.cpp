//
// Created by balp on 2018-07-28.
//
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
#include "NetworkDataStream.h"
#include "OCPN_SignalKEvent.h"



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
#include <wx/sckstrm.h>
#include "wx/jsonreader.h"
#include "wx/jsonwriter.h"

#include "SignalKDataStream.h"
const wxEventType wxEVT_OCPN_SIGNALKSTREAM = wxNewEventType();

BEGIN_EVENT_TABLE(SignalKDataStream, wxEvtHandler)
                EVT_TIMER(TIMER_SOCKET + 2, SignalKDataStream::OnTimerSocket)
                EVT_SOCKET(SIGNALK_SOCKET_ID, SignalKDataStream::OnSocketEvent)
                EVT_TIMER(TIMER_SOCKET + 3, SignalKDataStream::OnSocketReadWatchdogTimer)
END_EVENT_TABLE()


void SignalKDataStream::Open(void) {
    wxLogMessage(wxString::Format(_T("Opening Signal K client: %s"),
            m_params->GetDSPort().c_str()));
    SetSock(new wxSocketClient());
    GetSock()->SetEventHandler(*this, SIGNALK_SOCKET_ID);
    GetSock()->SetNotify(wxSOCKET_CONNECTION_FLAG | wxSOCKET_INPUT_FLAG | wxSOCKET_LOST_FLAG);
    GetSock()->Notify(TRUE);
    GetSock()->SetTimeout(1);              // Short timeout

    wxSocketClient* tcp_socket = static_cast<wxSocketClient*>(GetSock());
    tcp_socket->Connect(GetAddr(), FALSE);
    SetBrxConnectEvent(false);
}



void SignalKDataStream::OnSocketReadWatchdogTimer(wxTimerEvent& event)
{
    m_dog_value--;
    if( m_dog_value <= 0 ) {            // No receive in n seconds, assume connection lost
        wxLogMessage( wxString::Format(_T("    TCP SignalKDataStream watchdog timeout: %s"), GetPort().c_str()) );

        if(GetProtocol() == TCP ) {
            wxSocketClient* tcp_socket = dynamic_cast<wxSocketClient*>(GetSock());
            if(tcp_socket) {
                tcp_socket->Close();
            }
            GetSocketTimer()->Start(5000, wxTIMER_ONE_SHOT);    // schedule a reconnect
            GetSocketThreadWatchdogTimer()->Stop();
        }
    }
}

void SignalKDataStream::OnTimerSocket(wxTimerEvent& event)
{
    //  Attempt a connection
    wxSocketClient* tcp_socket = dynamic_cast<wxSocketClient*>(GetSock());
    if(tcp_socket) {
        if(tcp_socket->IsDisconnected() ) {
            SetBrxConnectEvent(false);
            tcp_socket->Connect(GetAddr(), FALSE);
            GetSocketTimer()->Start(5000, wxTIMER_ONE_SHOT);    // schedule another attempt
        }
    }
}

void SignalKDataStream::OnSocketEvent(wxSocketEvent& event)
{
#define RD_BUF_SIZE    4096


    switch(event.GetSocketEvent())
    {
        case wxSOCKET_CONNECTION :
        {
            wxLogMessage( wxString::Format(_T("SignalKDataStream connection established: %s"),
                                           GetPort().c_str()) );
            m_dog_value = N_DOG_TIMEOUT;                // feed the dog
            if (GetSock()->IsOk())
                (void) SetOutputSocketOptions(GetSock());
            GetSocketTimer()->Stop();
            SetBrxConnectEvent(true);
            SetConnectTime(wxDateTime::Now());
            break;
        }

        case wxSOCKET_LOST:
        {

            if (GetBrxConnectEvent())
                wxLogMessage(wxString::Format(_T("SignalKDataStream connection lost: %s"), GetPort().c_str()));
            wxDateTime now = wxDateTime::Now();
            wxTimeSpan since_connect = now - GetConnectTime();

            int retry_time = 5000;          // default

            //  If the socket has never connected, and it is a short interval since the connect request
            //  then stretch the time a bit.  This happens on Windows if there is no dafault IP on any interface

            if(!GetBrxConnectEvent() && (since_connect.GetSeconds() < 5) )
                retry_time = 10000;         // 10 secs

            GetSocketThreadWatchdogTimer()->Stop();
            GetSocketTimer()->Start(retry_time, wxTIMER_ONE_SHOT);     // Schedule a re-connect attempt

            break;
        }

        case wxSOCKET_INPUT:
        {
            wxLogMessage(wxString::Format(_T("SignalKDataStream input: %s"),
                                          GetPort().c_str()));
            wxJSONReader jsonReader;
            wxJSONValue root;
            wxSocketInputStream stream(*event.GetSocket());
            int errors = jsonReader.Parse(stream, &root);
            if (errors > 0) {
                wxLogMessage(
                            wxString::Format(_T("SignalKDataStream ERROR: the JSON document is not well-formed:%d: %s"),
                                             errors,
                                             GetPort().c_str()));

            } else {
                wxString dbg;
                wxJSONWriter writer;
                writer.Write(root, dbg);
                wxLogMessage(dbg);
            }
            OCPN_SignalKEvent signalKEvent(event.GetId(), wxEVT_OCPN_SIGNALKSTREAM, root);
            GetConsumer()->AddPendingEvent(signalKEvent);
            break;
        }

#if 0
        case wxSOCKET_INPUT :                     // from gpsd Daemon
        {
            // TODO determine if the follwing SetFlags needs to be done at every socket event or only once when socket is created, it it needs to be done at all!
            //m_sock->SetFlags(wxSOCKET_WAITALL | wxSOCKET_BLOCK);      // was (wxSOCKET_NOWAIT);

            // We use wxSOCKET_BLOCK to avoid Yield() reentrancy problems
            // if a long ProgressDialog is active, as in S57 SENC creation.


            //    Disable input event notifications to preclude re-entrancy on non-blocking socket
            //           m_sock->SetNotify(wxSOCKET_LOST_FLAG);

            std::vector<char> data(RD_BUF_SIZE+1);
            event.GetSocket()->Read(&data.front(),RD_BUF_SIZE);
            if(!event.GetSocket()->Error())
            {
                size_t count = event.GetSocket()->LastCount();
                if(count)
                {
                    if(!g_benableUDPNullHeader){
                        data[count]=0;
                        m_sock_buffer += (&data.front());
                    }
                    else{
                        // XXX FIXME: is it reliable?
                        // copy all received bytes
                        // there's 0 in furuno UDP tags before NMEA sentences.
                        m_sock_buffer.append(&data.front(), count);
                    }
                }
            }

            bool done = false;

            while(!done){
                int nmea_tail = 2;
                size_t nmea_end = m_sock_buffer.find_first_of("*\r\n"); // detect the potential end of a NMEA string by finding the checkum marker or EOL

                if (nmea_end == wxString::npos) // No termination characters: continue reading
                    break;

                if (m_sock_buffer[nmea_end] != '*')
                    nmea_tail = -1;

                if(nmea_end < m_sock_buffer.size() - nmea_tail){
                    nmea_end += nmea_tail + 1; // move to the char after the 2 checksum digits, if present
                    if ( nmea_end == 0 ) //The first character in the buffer is a terminator, skip it to avoid infinite loop
                        nmea_end = 1;
                    std::string nmea_line = m_sock_buffer.substr(0,nmea_end);

                    //  If, due to some logic error, the {nmea_end} parameter is larger than the length of the
                    //  socket buffer, then std::string::substr() will throw an exception.
                    //  We don't want that, so test for it.
                    //  If found, the simple solution is to clear the socket buffer, and carry on
                    //  This has been seen on high volume TCP feeds, Windows only.
                    //  Hard to catch.....
                    if(nmea_end > m_sock_buffer.size())
                        m_sock_buffer.clear();
                    else
                        m_sock_buffer = m_sock_buffer.substr(nmea_end);

                    size_t nmea_start = nmea_line.find_last_of("$!"); // detect the potential start of a NMEA string, skipping preceding chars that may look like the start of a string.
                    if(nmea_start != wxString::npos){
                        nmea_line = nmea_line.substr(nmea_start);
                        nmea_line += "\r\n";        // Add cr/lf, possibly superfluous
                        if( GetConsumer() && ChecksumOK(nmea_line)){
                            OCPN_DataStreamEvent Nevent(wxEVT_OCPN_DATASTREAM, 0);
                            if(nmea_line.size()) {
                                Nevent.SetNMEAString( nmea_line );
                                Nevent.SetStream( this );

                                GetConsumer()->AddPendingEvent(Nevent);
                            }
                        }
                    }
                }
                else
                    done = true;
            }

            // Prevent non-nmea junk from consuming to much memory by limiting carry-over buffer size.
            if(m_sock_buffer.size()>RD_BUF_SIZE)
                m_sock_buffer = m_sock_buffer.substr(m_sock_buffer.size()-RD_BUF_SIZE);

            m_dog_value = N_DOG_TIMEOUT;                // feed the dog
            break;
        }
#endif


        default :
            break;
    }

}


bool SignalKDataStream::SetOutputSocketOptions(wxSocketBase* sock)
{
    int ret;
    int nagleDisable=1;
    ret = sock->SetOption(IPPROTO_TCP,TCP_NODELAY,&nagleDisable, sizeof(nagleDisable));
    unsigned long outbuf_size = 1024;
    return (sock->SetOption(SOL_SOCKET,SO_SNDBUF,&outbuf_size, sizeof(outbuf_size)) && ret);
}