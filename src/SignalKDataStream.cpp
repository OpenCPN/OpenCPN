//
// Created by balp on 2018-07-28.
//
#include "zeroconf.hpp"

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

#if defined(__WXMSW__) && !defined(__MINGW32__)
#include <Ws2tcpip.h>           // for ip_mreq
#endif

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

SignalKDataStream::~SignalKDataStream(){

    if (GetSock()->IsOk()){
        char unsub[]  = "{\"context\":\"*\",\"unsubscribe\":[{\"path\":\"*\"}]}\r\n";
        GetSock()->Write(unsub, strlen(unsub));
    }

    Close();
}

void SignalKDataStream::Open(void) {
    
    wxLogMessage(wxString::Format(_T("Opening Signal K client: %s"),
            m_params->GetDSPort().c_str()));

    wxString discoveredIP;
    int discoveredPort;
    if(m_params->AutoSKDiscover){
        if( DiscoverSKServer( discoveredIP, discoveredPort, 1) )        // 1 second scan
            wxLogMessage(wxString::Format(_T("SK server autodiscovery finds: %s:%d"), discoveredIP.c_str(), discoveredPort));
        else
            wxLogMessage(_T("SK server autodiscovery finds no server."));
    }
    
    SetSock(new wxSocketClient());
    GetSock()->SetEventHandler(*this, SIGNALK_SOCKET_ID);
    GetSock()->SetNotify(wxSOCKET_CONNECTION_FLAG | wxSOCKET_INPUT_FLAG | wxSOCKET_LOST_FLAG);
    GetSock()->Notify(TRUE);
    GetSock()->SetTimeout(1);              // Short timeout

    SetConnectTime(wxDateTime::Now());

    wxSocketClient* tcp_socket = static_cast<wxSocketClient*>(GetSock());
    tcp_socket->Connect(GetAddr(), FALSE);
    SetBrxConnectEvent(false);
}

bool SignalKDataStream::DiscoverSKServer( wxString &ip, int &port, int tSec){
    std::vector<Zeroconf::mdns_responce> result;
    bool st = Zeroconf::Resolve("_signalk-tcp._tcp.local", tSec, &result);              // Works for node.js server

    for(size_t i = 0 ; i < result.size() ; i++){
      sockaddr_storage sas = result[i].peer;                 // Address of the responded machine
      sockaddr_in *sai = (sockaddr_in*)&sas;
      ip = wxString(inet_ntoa(sai->sin_addr));
      
      std::vector<uint8_t> data = result[i].data;

      std::vector<Zeroconf::Detail::mdns_record> records = result[i].records;
      for(size_t j = 0 ; j < records.size() ; j++){

        uint16_t type = records[j].type;
        if(type == 33){                                 // SRV
            size_t pos = records[j].pos;
            //size_t len = records[j].len;
            //std::string name = records[j].name;
            
            // TODO This is pretty ugly, but I cannot find a definition of SRV record
            unsigned char portHi = data[pos + 16];
            unsigned char portLo = data[pos + 17];
            port = (portHi * 256) + portLo;
            return true;
        }
      }
    }
    return false;
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
            
            char sub2[]  = "{\"context\":\"vessels.self\",\"subscribe\":[{\"path\":\"navigation.*\"}]}\r\n";
            GetSock()->Write(sub2, strlen(sub2));

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
            #define RD_BUF_SIZE    4096 // Allows handling of high volume data streams.
            wxJSONReader jsonReader;
            wxJSONValue root;

            std::vector<char> data(RD_BUF_SIZE+1);
            event.GetSocket()->Read(&data.front(),RD_BUF_SIZE);
            if(!event.GetSocket()->Error())
            {
                size_t count = event.GetSocket()->LastCount();
                m_sock_buffer.append(&data.front(), count);
            }

            bool done = false;

            while(!done){
                int sk_tail = 2;
                size_t sk_end = m_sock_buffer.find_first_of("\r\n"); // detect the end of an SK string by finding the EOL

                if (sk_end == wxString::npos) // No termination characters: continue reading
                    break;


                size_t bufl = m_sock_buffer.size();
                if(sk_end <= m_sock_buffer.size() - sk_tail){
                    std::string sk_line = m_sock_buffer.substr(0,sk_end + sk_tail);

                    //  If, due to some logic error, the {nmea_end} parameter is larger than the length of the
                    //  socket buffer, then std::string::substr() will throw an exception.
                    //  We don't want that, so test for it.
                    //  If found, the simple solution is to clear the socket buffer, and carry on
                    //  This has been seen on high volume TCP feeds, Windows only.
                    //  Hard to catch.....
                    if(sk_end > m_sock_buffer.size())
                        m_sock_buffer.clear();
                    else
                        m_sock_buffer = m_sock_buffer.substr(sk_end + sk_tail);

                    size_t sk_start = 0; 
                    if(sk_start != wxString::npos){
                        sk_line = sk_line.substr(sk_start);
                        if(sk_line.size()){
                            
                            int errors = jsonReader.Parse(sk_line, &root);
                            if (errors > 0) {
                                wxLogMessage(
                                            wxString::Format(_T("SignalKDataStream ERROR: the JSON document is not well-formed:%d: %s"),
                                                errors,
                                                GetPort().c_str()));

                            } else {
                                if( GetConsumer() ) {
                                    OCPN_SignalKEvent signalKEvent(0, EVT_OCPN_SIGNALKSTREAM, root);
                                    GetConsumer()->AddPendingEvent(signalKEvent);
                                }
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

void SignalKDataStream::Close()
{
    wxLogMessage( wxString::Format(_T("Closing Signal K DataStream %s"), GetPort().c_str()) );
    //    Kill off the TCP Socket if alive
    if(m_sock)
    {
        m_sock->Notify(FALSE);
        m_sock->Destroy();
    }

#if 0    
    if(m_tsock)
    {
        m_tsock->Notify(FALSE);
        m_tsock->Destroy();
    }

    if(m_socket_server)
    {
        m_socket_server->Notify(FALSE);
        m_socket_server->Destroy();
    }
#endif

    m_socket_timer.Stop();
    m_socketread_watchdog_timer.Stop();


    DataStream::Close();
}

