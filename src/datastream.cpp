/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  NMEA Data Stream Object
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

#include "dychart.h"

#include "datastream.h"
#include "OCPN_DataStreamEvent.h"
#include "OCP_DataStreamInput_Thread.h"
// #include "garmin/jeeps/garmin_wrapper.h"
#include "GarminProtocolHandler.h"

#ifdef __OCPN__ANDROID__
#include "androidUTIL.h"
#endif

#include <vector>
#include <wx/socket.h>
#include <wx/log.h>
#include <wx/memory.h>
#include <wx/chartype.h>
#include <wx/wx.h>
#include <wx/sckaddr.h>

#if !defined(NAN)
static const long long lNaN = 0xfff8000000000000;
#define NAN (*(double*)&lNaN)
#endif

const wxEventType wxEVT_OCPN_DATASTREAM = wxNewEventType();

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


bool CheckSumCheck(const std::string& sentence)
{
    size_t check_start = sentence.find('*');
    if(check_start == wxString::npos || check_start > sentence.size() - 3)
        return false; // * not found, or it didn't have 2 characters following it.
        
    std::string check_str = sentence.substr(check_start+1,2);
    unsigned long checksum;
    //    if(!check_str.ToULong(&checksum,16))
    if(!(checksum = strtol(check_str.c_str(), 0, 16)))
        return false;
    
    unsigned char calculated_checksum = 0;
    for(std::string::const_iterator i = sentence.begin()+1; i != sentence.end() && *i != '*'; ++i)
        calculated_checksum ^= static_cast<unsigned char> (*i);
    
    return calculated_checksum == checksum;
    
}


DataStream* makeDataStream(wxEvtHandler *input_consumer, const ConnectionParams* params)
{
    wxLogMessage( wxString::Format(_T("makeSerialDataStream %s"),
            params->GetDSPort().c_str()) );
    switch (params->Type) {
        case SERIAL:
            return new SerialDataStream(input_consumer, params);
        case NETWORK:
            return new NetworkDataStream(input_consumer, params);
        case INTERNAL_GPS:
            return new InternalGPSDataStream(input_consumer, params);
        case INTERNAL_BT:
            return new InternalBTDataStream(input_consumer, params);
        default:
            return new NullDataStream(input_consumer, params);
    }
}

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
                                DS_HANDSHAKE_NONE,
                                NULL);
}


//------------------------------------------------------------------------------
//    DataStream Implementation
//------------------------------------------------------------------------------

BEGIN_EVENT_TABLE(DataStream, wxEvtHandler)

    EVT_SOCKET(DS_SOCKET_ID, DataStream::OnSocketEvent)
    EVT_SOCKET(DS_SERVERSOCKET_ID, DataStream::OnServerSocketEvent)
    EVT_TIMER(TIMER_SOCKET, DataStream::OnTimerSocket)
    EVT_TIMER(TIMER_SOCKET + 1, DataStream::OnSocketReadWatchdogTimer)
END_EVENT_TABLE()

// constructor
DataStream::DataStream(wxEvtHandler *input_consumer,
             const ConnectionType conn_type,         
             const wxString& Port,
             const wxString& BaudRate,
             dsPortType io_select,
             int priority,
             bool bGarmin,
             int EOS_type,
             int handshake_type,
             void *user_data)
    :
    m_Thread_run_flag(-1),
    m_bok(false),
    m_consumer(input_consumer),
    m_portstring(Port),
    m_BaudRate(BaudRate),
    m_io_select(io_select),
    m_priority(priority),
    m_handshake(handshake_type),
    m_user_data(user_data),
    m_pSecondary_Thread(NULL),
    m_sock(0),
    m_tsock(0),
    m_is_multicast(false),
    // m_mrq
    m_socket_server(0),
    // m_socket_server_active(false),
    // m_sock_buffer
    // m_net_addr
    // m_net_port
    m_net_protocol(GPSD),
    m_connection_type(conn_type),
    // m_bchecksumCheck
    // m_input_filter
    // m_input_filter_type
    // m_output_filter
    // m_output_filter_type
    m_bGarmin_GRMN_mode(bGarmin),
    m_GarminHandler(NULL),
    // m_connect_time
    // m_brx_connect_event
    m_txenter(0),
    // m_socketread_watchdog_timer
    // m_dog_value
    m_params()
{
    wxLogMessage( _T("Classic CTOR"));
    if(m_connection_type == NETWORK) {
        ConfigNetworkParams();
    }

    SetSecThreadInActive();
    
    m_socket_timer.SetOwner(this, TIMER_SOCKET);
    m_socketread_watchdog_timer.SetOwner(this, TIMER_SOCKET + 1);

    Open();
}

DataStream::DataStream(wxEvtHandler *input_consumer,
             const ConnectionParams* params)
    :
    m_Thread_run_flag(-1),
    m_bok(false),
    m_consumer(input_consumer),
    m_portstring(params->GetDSPort()),
    // m_BaudRate(BaudRate),
    m_io_select(params->IOSelect),
    m_priority(params->Priority),
    m_handshake(DS_HANDSHAKE_NONE),
    m_user_data(NULL),
    m_pSecondary_Thread(NULL),
    m_sock(0),
    m_tsock(0),
    m_is_multicast(false),
    // m_mrq
    m_socket_server(0),
    // m_socket_server_active(false),
    // m_sock_buffer
    // m_net_addr
    // m_net_port
    m_net_protocol(params->NetProtocol),
    m_connection_type(params->Type),
    // m_bchecksumCheck
    // m_input_filter
    // m_input_filter_type
    // m_output_filter
    // m_output_filter_type
    m_bGarmin_GRMN_mode(params->Garmin),
    m_GarminHandler(NULL),
    // m_connect_time
    // m_brx_connect_event
    m_txenter(0),
    // m_socketread_watchdog_timer
    // m_dog_value
    m_params(*params)
{
    if(m_connection_type == NETWORK) {
        m_net_addr = params->NetworkAddress;
        m_net_port = wxString::Format(wxT("%i"), params->NetworkPort);
        m_net_protocol = params->NetProtocol;
        m_addr.Hostname(m_net_addr);
        m_addr.Service(m_net_port);
    }

    m_BaudRate = wxString::Format(wxT("%i"), params->Baudrate),
    SetSecThreadInActive();
    
    m_socket_timer.SetOwner(this, TIMER_SOCKET);
    m_socketread_watchdog_timer.SetOwner(this, TIMER_SOCKET + 1);
    wxLogMessage( _T("ConnectionParams CTOR"));

    // Open();

    SetInputFilter(params->InputSentenceList);
    SetInputFilterType(params->InputSentenceListType);
    SetOutputFilter(params->OutputSentenceList);
    SetOutputFilterType(params->OutputSentenceListType);
    SetChecksumCheck(params->ChecksumCheck);
}

void DataStream::Open(void)
{
    //  Open a port
    wxLogMessage( wxString::Format(_T("Opening NMEA Datastream %s"), m_portstring.c_str()) );
    SetOk(false);
    m_connect_time = wxDateTime::Now();
}

void InternalBTDataStream::OpenInternalBT() const {
#ifdef __OCPN__ANDROID__
    SetOk(androidStartBT(m_consumer, m_portstring ));
#endif
}

void InternalGPSDataStream::OpenInternalGPS() const {
#ifdef __OCPN__ANDROID__
    androidStartNMEA(m_consumer);
    SetOk(true)
#endif

}

void NetworkDataStream::OpenNetwork() {


#ifdef __UNIX__
# if wxCHECK_VERSION(3,0,0)
    in_addr_t addr = ((struct sockaddr_in *) GetAddr().GetAddressData())->sin_addr.s_addr;
# else
    in_addr_t addr = ((struct sockaddr_in *) GetAddr().GetAddress()->m_addr)->sin_addr.s_addr;
# endif
#else
    unsigned int addr = inet_addr(GetAddr().IPAddress().mb_str());
#endif
    // Create the socket
    switch(GetConnectionParams()->NetProtocol){
        case GPSD: {
            OpenNetworkGPSD();
            break;
        }
        case TCP: {
            OpenNetworkTCP(addr);
            break;
        }
        case UDP: {
            OpenNetworkUDP(addr);
            break;
        }
        default:
            break;

    }
    SetOk(true);
}

void NetworkDataStream::OpenNetworkUDP(unsigned int addr)
{
    if (GetPortType() != DS_TYPE_OUTPUT) {
        //  We need a local (bindable) address to create the Datagram receive socket
        // Set up the receive socket
        wxIPV4address conn_addr;
        conn_addr.Service(GetNetPort());
        conn_addr.AnyAddress();
        SetSock(new wxDatagramSocket(conn_addr, wxSOCKET_NOWAIT | wxSOCKET_REUSEADDR));

        // Test if address is IPv4 multicast
        if ((ntohl(addr) & 0xf0000000) == 0xe0000000) {
            SetMulticast(true);
            SetMrqAddr(addr);
            GetSock()->SetOption(IPPROTO_IP, IP_ADD_MEMBERSHIP, &GetMrq(), sizeof(GetMrq()));
        }

        GetSock()->SetEventHandler(*this, DS_SOCKET_ID);

        GetSock()->SetNotify(wxSOCKET_CONNECTION_FLAG |
                             wxSOCKET_INPUT_FLAG |
                             wxSOCKET_LOST_FLAG);
        GetSock()->Notify(TRUE);
        GetSock()->SetTimeout(1);              // Short timeout
    }

    // Set up another socket for transmit
    if (GetPortType() != DS_TYPE_INPUT) {
        wxIPV4address tconn_addr;
        tconn_addr.Service(0);          // use ephemeral out port
        tconn_addr.AnyAddress();
        SetTSock(new wxDatagramSocket(tconn_addr, wxSOCKET_NOWAIT | wxSOCKET_REUSEADDR));
        // Here would be the place to disable multicast loopback
        // but for consistency with broadcast behaviour, we will
        // instead rely on setting priority levels to ignore
        // sentences read back that have just been transmitted
        if ((!GetMulticast()) && (GetAddr().IPAddress().EndsWith(_T("255")))) {
            int broadcastEnable = 1;
            bool bam = GetTSock()->SetOption(SOL_SOCKET, SO_BROADCAST,
                    &broadcastEnable, sizeof(broadcastEnable));
        }
    }
}

void NetworkDataStream::OpenNetworkTCP(unsigned int addr)
{
    int isServer = ((addr == INADDR_ANY) ? 1 : 0);
    wxLogMessage(wxString::Format(_T("Opening TCP Server %d"), isServer));

    if (isServer) {
        SetSockServer(new wxSocketServer(GetAddr(), wxSOCKET_REUSEADDR));
    } else {
        SetSock(new wxSocketClient());
    }

    if (isServer) {
        GetSockServer()->SetEventHandler(*this, DS_SERVERSOCKET_ID);
        GetSockServer()->SetNotify(wxSOCKET_CONNECTION_FLAG );
        GetSockServer()->Notify(TRUE);
        GetSockServer()->SetTimeout(1);    // Short timeout
    }
    else {
        GetSock()->SetEventHandler(*this, DS_SOCKET_ID);
        int notify_flags=(wxSOCKET_CONNECTION_FLAG | wxSOCKET_LOST_FLAG );
        if (GetPortType() != DS_TYPE_INPUT)
            notify_flags |= wxSOCKET_OUTPUT_FLAG;
        if (GetPortType() != DS_TYPE_OUTPUT)
            notify_flags |= wxSOCKET_INPUT_FLAG;
        GetSock()->SetNotify(notify_flags);
        GetSock()->Notify(TRUE);
        GetSock()->SetTimeout(1);              // Short timeout

        SetBrxConnectEvent(false);
        GetSocketTimer()->Start(100, wxTIMER_ONE_SHOT);    // schedule a connection
    }
}

void NetworkDataStream::OpenNetworkGPSD()
{
    SetSock(new wxSocketClient());
    GetSock()->SetEventHandler(*this, DS_SOCKET_ID);
    GetSock()->SetNotify(wxSOCKET_CONNECTION_FLAG | wxSOCKET_INPUT_FLAG | wxSOCKET_LOST_FLAG);
    GetSock()->Notify(TRUE);
    GetSock()->SetTimeout(1);              // Short timeout

    wxSocketClient* tcp_socket = static_cast<wxSocketClient*>(GetSock());
    tcp_socket->Connect(GetAddr(), FALSE);
    SetBrxConnectEvent(false);
}

void DataStream::ConfigNetworkParams() {
    if(m_portstring.Contains(_T("GPSD"))){
            m_net_addr = _T("127.0.0.1");              // defaults
            m_net_port = _T("2947");
            m_net_protocol = GPSD;
        }
        else if(m_portstring.StartsWith(_T("TCP"))) {
            m_net_addr = _T("0.0.0.0");              // defaults
            m_net_port = _T("10110");
            m_net_protocol = TCP;
        }
        else if(m_portstring.StartsWith(_T("UDP"))) {
            m_net_addr =  _T("0.0.0.0");              // any address
            m_net_port = _T("10110");
            m_net_protocol = UDP;
        }
        else {
            m_net_addr =  _T("0.0.0.0");              // any address
            m_net_port = _T("0");
            m_net_protocol = UDP;
        }

    //  Capture the  parameters from the portstring

    wxStringTokenizer tkz(m_portstring, _T(":"));
    wxString token = tkz.GetNextToken();                //GPSD, TCP or UDP

    token = tkz.GetNextToken();                         //ip
    if(!token.IsEmpty())
            m_net_addr = token;

    token = tkz.GetNextToken();                         //port
    if(!token.IsEmpty())
            m_net_port = token;


    m_addr.Hostname(m_net_addr);
    m_addr.Service(m_net_port);
}

void SerialDataStream::OpenSerial() {
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


DataStream::~DataStream()
{
    Close();
}

void DataStream::Close()
{
    wxLogMessage( wxString::Format(_T("Closing NMEA Datastream %s"), m_portstring.c_str()) );
    
//    Kill off the Secondary RX Thread if alive
    if(m_pSecondary_Thread)
    {
        if(m_bsec_thread_active)              // Try to be sure thread object is still alive
        {
            wxLogMessage(_T("Stopping Secondary Thread"));

            m_Thread_run_flag = 0;
            int tsec = 10;
            while((m_Thread_run_flag >= 0) && (tsec--))
                wxSleep(1);

            wxString msg;
            if(m_Thread_run_flag < 0)
                  msg.Printf(_T("Stopped in %d sec."), 10 - tsec);
            else
                 msg.Printf(_T("Not Stopped after 10 sec."));
            wxLogMessage(msg);
        }

        m_pSecondary_Thread = NULL;
        m_bsec_thread_active = false;
    }

    //    Kill off the TCP Socket if alive
    if(m_sock)
    {
        if (m_is_multicast)
            m_sock->SetOption(IPPROTO_IP,IP_DROP_MEMBERSHIP,&m_mrq, 
                sizeof(m_mrq));
        m_sock->Notify(FALSE);
        m_sock->Destroy();
    }

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
    
    //  Kill off the Garmin handler, if alive
    if(m_GarminHandler) {
        m_GarminHandler->Close();
        delete m_GarminHandler;
    }
    
    m_socket_timer.Stop();
    m_socketread_watchdog_timer.Stop();
    
    if(m_connection_type == INTERNAL_GPS){
#ifdef __OCPN__ANDROID__
        androidStopNMEA();
#endif
    }
    else if(m_connection_type == INTERNAL_BT){
#ifdef __OCPN__ANDROID__
        androidStopBT();
#endif
    }
    
        
}

void DataStream::OnSocketReadWatchdogTimer(wxTimerEvent& event)
{
    m_dog_value--;
    if( m_dog_value <= 0 ) {            // No receive in n seconds, assume connection lost
        wxLogMessage( wxString::Format(_T("    TCP Datastream watchdog timeout: %s"), m_portstring.c_str()) );

        if(m_net_protocol == TCP ) {
            wxSocketClient* tcp_socket = dynamic_cast<wxSocketClient*>(m_sock);
            if(tcp_socket) {
                tcp_socket->Close();
            }
            m_socket_timer.Start(5000, wxTIMER_ONE_SHOT);    // schedule a reconnect
            m_socketread_watchdog_timer.Stop();
        }
    }
}

void DataStream::OnTimerSocket(wxTimerEvent& event)
{
    //  Attempt a connection
    wxSocketClient* tcp_socket = dynamic_cast<wxSocketClient*>(m_sock);
    if(tcp_socket) {
        if(tcp_socket->IsDisconnected() ) {
            m_brx_connect_event = false;
            tcp_socket->Connect(m_addr, FALSE);
            m_socket_timer.Start(5000, wxTIMER_ONE_SHOT);    // schedule another attempt
        }
    }
}

void DataStream::OnSocketEvent(wxSocketEvent& event)
{
    //#define RD_BUF_SIZE    200
    #define RD_BUF_SIZE    4096 // Allows handling of high volume data streams, such as a National AIS stream with 100s of msgs a second.

    switch(event.GetSocketEvent())
    {
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
                        if( m_consumer && ChecksumOK(nmea_line)){
                            OCPN_DataStreamEvent Nevent(wxEVT_OCPN_DATASTREAM, 0);
                            if(nmea_line.size()) {
                                Nevent.SetNMEAString( nmea_line );
                                Nevent.SetStream( this );
                            
                                m_consumer->AddPendingEvent(Nevent);
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

        case wxSOCKET_LOST:
        {
            //          wxSocketError e = m_sock->LastError();          // this produces wxSOCKET_WOULDBLOCK.
            if(m_net_protocol == TCP || m_net_protocol == GPSD) {
				if (m_brx_connect_event)
					wxLogMessage(wxString::Format(_T("Datastream connection lost: %s"), m_portstring.c_str()));
                if (m_socket_server) {
                    m_sock->Destroy();
                    m_sock=0;
                    break;
                }
                wxDateTime now = wxDateTime::Now();
                wxTimeSpan since_connect = now - m_connect_time;

                int retry_time = 5000;          // default

                //  If the socket has never connected, and it is a short interval since the connect request
                //  then stretch the time a bit.  This happens on Windows if there is no dafault IP on any interface
                
                if(!m_brx_connect_event && (since_connect.GetSeconds() < 5) )
                    retry_time = 10000;         // 10 secs
                
                m_socketread_watchdog_timer.Stop();
                m_socket_timer.Start(retry_time, wxTIMER_ONE_SHOT);     // Schedule a re-connect attempt

                break;
            }
        }

        case wxSOCKET_CONNECTION :
        {
            if(m_net_protocol == GPSD) {
                //      Sign up for watcher mode, Cooked NMEA
                //      Note that SIRF devices will be converted by gpsd into pseudo-NMEA
                char cmd[] = "?WATCH={\"class\":\"WATCH\", \"nmea\":true}";
                m_sock->Write(cmd, strlen(cmd));
            }
            else if(m_net_protocol == TCP) {
                wxLogMessage( wxString::Format(_T("TCP Datastream connection established: %s"), m_portstring.c_str()) );
                m_dog_value = N_DOG_TIMEOUT;                // feed the dog
                if (m_io_select != DS_TYPE_OUTPUT)
                    m_socketread_watchdog_timer.Start(1000);
                if (m_io_select != DS_TYPE_INPUT && m_sock->IsOk())
                    (void) SetOutputSocketOptions(m_sock);
                m_socket_timer.Stop();
                m_brx_connect_event = true;
            }

            m_connect_time = wxDateTime::Now();
            break;
        }

        default :
            break;
    }
}

void DataStream::OnServerSocketEvent(wxSocketEvent& event)
{
    
    switch(event.GetSocketEvent())
    {
        case wxSOCKET_CONNECTION :
        {
            m_sock= m_socket_server->Accept(false);
 
            if( m_sock) {
                m_sock->SetTimeout(2);
                m_sock->SetFlags( wxSOCKET_BLOCK );
                m_sock->SetEventHandler(*this, DS_SOCKET_ID);
                int notify_flags=(wxSOCKET_CONNECTION_FLAG | wxSOCKET_LOST_FLAG );
                if (m_io_select != DS_TYPE_INPUT) {
                    notify_flags |= wxSOCKET_OUTPUT_FLAG;
                    (void) SetOutputSocketOptions(m_sock);
                }
                if (m_io_select != DS_TYPE_OUTPUT)
                    notify_flags |= wxSOCKET_INPUT_FLAG;
                m_sock->SetNotify(notify_flags);
                m_sock->Notify(true);
            }
            
            break;
        }
        
        default :
            break;
    }
}

bool DataStream::SentencePassesFilter(const wxString& sentence, FilterDirection direction)
{
    wxArrayString filter;
    bool listype = false;

    if (direction == FILTER_INPUT)
    {
        filter = m_input_filter;
        if (m_input_filter_type == WHITELIST)
            listype = true;
    }
    else
    {
        filter = m_output_filter;
        if (m_output_filter_type == WHITELIST)
            listype = true;
    }
    if (filter.Count() == 0) //Empty list means everything passes
        return true;

    wxString fs;
    for (size_t i = 0; i < filter.Count(); i++)
    {
        fs = filter.Item(i);
        switch (fs.Length())
        {
            case 2:
                if (fs == sentence.Mid(1, 2))
                    return listype;
                break;
            case 3:
                if (fs == sentence.Mid(3, 3))
                    return listype;
                break;
            case 5:
                if (fs == sentence.Mid(1, 5))
                    return listype;
                break;
        }
    }
    return !listype;
}

bool DataStream::ChecksumOK( const std::string &sentence )
{
    if (!m_bchecksumCheck)
        return true;

    return CheckSumCheck(sentence);
    
}


bool DataStream::SendSentenceSerial(const wxString &payload)
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


bool DataStream::SendSentence( const wxString &sentence )
{
    wxString payload = sentence;
    if( !sentence.EndsWith(_T("\r\n")) )
        payload += _T("\r\n");

    switch( m_connection_type ) {
        case SERIAL:{
            return SendSentenceSerial(payload);
        }
            
        case NETWORK:{
            if(m_txenter)
                return false;                 // do not allow recursion, could happen with non-blocking sockets
            m_txenter++;

            bool ret = true;
            wxDatagramSocket* udp_socket;
                switch(m_net_protocol){
                    case TCP:
                        if( m_sock && m_sock->IsOk() ) {
                            m_sock->Write( payload.mb_str(), strlen( payload.mb_str() ) );
                            if(m_sock->Error()){
                                if (m_socket_server) {
                                    m_sock->Destroy();
                                    m_sock= 0;
                                } else {
                                    wxSocketClient* tcp_socket = dynamic_cast<wxSocketClient*>(m_sock);
                                    if (tcp_socket)
                                        tcp_socket->Close();
                                    if(!m_socket_timer.IsRunning())
                                        m_socket_timer.Start(5000, wxTIMER_ONE_SHOT);    // schedule a reconnect
                                    m_socketread_watchdog_timer.Stop();
                                }
                                ret = false;
                            }

                        }
                        else
                            ret = false;
                        break;
                    case UDP:
                        udp_socket = dynamic_cast<wxDatagramSocket*>(m_tsock);
                        if(udp_socket && udp_socket->IsOk() ) {
                            udp_socket->SendTo(m_addr, payload.mb_str(), payload.size() );
                            if( udp_socket->Error())
                                ret = false;
                        }
                        else
                            ret = false;
                        break;
                    
                    case GPSD:    
                    default:
                        ret = false;
                        break;
            }
            m_txenter--;
            return ret;
            break;
        }
         
        default:
            break;
    }
    
    return true;
}

