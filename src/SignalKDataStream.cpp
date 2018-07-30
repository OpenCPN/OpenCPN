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
#include "SignalKDataStream.h"
#include "NetworkDataStream.h"
#include "OCPN_SignalKEvent.h"
#include "OCPN_DataStreamEvent.h"


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

// extern const wxEventType wxEVT_OCPN_SIGNALKSTREAM;

// const wxEventType wxEVT_OCPN_SIGNALKSTREAM = wxNewEventType();

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
#if 0
            wxLogMessage(wxString::Format(_T("SignalKDataStream input: %s"),
                                          GetPort().c_str()));
#endif
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
#if 0
                wxString dbg;
                wxJSONWriter writer;
                writer.Write(root, dbg);
                wxLogMessage(dbg);
#endif
                if( GetConsumer() ) {
                    wxLogMessage(wxString::Format(_T("SignalKDataStream send to: %p"),
                                                  GetConsumer()));
                    OCPN_SignalKEvent signalKEvent(0,
                                                   EVT_OCPN_SIGNALKSTREAM,
                                                   root);
                    GetConsumer()->AddPendingEvent(signalKEvent);
                }
            }
            break;
        }
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