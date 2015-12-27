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
#include "garmin/jeeps/garmin_wrapper.h"

#ifdef __OCPN__ANDROID__
#include "androidUTIL.h"
#endif

#include <vector>

#if !defined(NAN)
static const long long lNaN = 0xfff8000000000000;
#define NAN (*(double*)&lNaN)
#endif

const wxEventType wxEVT_OCPN_DATASTREAM = wxNewEventType();

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
             void *user_data )
{
    m_consumer = input_consumer;
    m_portstring = Port;
    m_BaudRate = BaudRate;
    m_io_select = io_select;
    m_priority = priority;
    m_handshake = handshake_type;
    m_user_data = user_data;
    m_bGarmin_GRMN_mode = bGarmin;
    m_connection_type = conn_type;

    Init();

    Open();
}

void DataStream::Init(void)
{
    m_pSecondary_Thread = NULL;
    m_GarminHandler = NULL;
    m_bok = false;
    SetSecThreadInActive();
    m_Thread_run_flag = -1;
    m_sock = 0;
    m_tsock = 0;
    m_is_multicast = false;
    m_socket_server = 0;
    m_txenter = 0;
    m_net_protocol = GPSD;
    
    m_socket_timer.SetOwner(this, TIMER_SOCKET);
    m_socketread_watchdog_timer.SetOwner(this, TIMER_SOCKET + 1);
    
}

void DataStream::Open(void)
{
    //  Open a port
    wxLogMessage( wxString::Format(_T("Opening NMEA Datastream %s"), m_portstring.c_str()) );
    
    //    Data Source is specified serial port
    if(m_portstring.Contains(_T("Serial"))) {
        m_connection_type = SERIAL;
        wxString comx;
        comx =  m_portstring.AfterFirst(':');      // strip "Serial:"

        wxString port_uc = m_portstring.Upper();

        if( (wxNOT_FOUND != port_uc.Find(_T("USB"))) && (wxNOT_FOUND != port_uc.Find(_T("GARMIN"))) ) {
            m_GarminHandler = new GarminProtocolHandler(this, m_consumer,  true);
        }    
        else if( m_bGarmin_GRMN_mode ) {
            m_GarminHandler = new GarminProtocolHandler(this, m_consumer,  false);
        }
        else {
            m_connection_type = SERIAL;
            wxString comx;
            comx =  m_portstring.AfterFirst(':');      // strip "Serial:"

            comx = comx.BeforeFirst(' ');               // strip off any description provided by Windows
            
#if 0
 #ifdef __WXMSW__
            wxString scomx = comx;
            scomx.Prepend(_T("\\\\.\\"));                  // Required for access to Serial Ports greater than COM9

    //  As a quick check, verify that the specified port is available
            HANDLE hSerialComm = CreateFile(scomx.fn_str(),       // Port Name
                                      GENERIC_READ,
                                      0,
                                      NULL,
                                      OPEN_EXISTING,
                                      0,
                                      NULL);

            if(hSerialComm == INVALID_HANDLE_VALUE) {
                m_last_error = DS_ERROR_PORTNOTFOUND;
                wxLogMessage( _T("   Error, comm port open failure, INVALID_HANDLE_VALUE, Datastream skipped.") );
                return;
            }
            else
                CloseHandle(hSerialComm);
#endif
#endif

    //    Kick off the DataSource RX thread
            m_pSecondary_Thread = new OCP_DataStreamInput_Thread(this,
                                                                 m_consumer,
                                                                 comx, m_BaudRate,
                                                                 m_io_select);
            m_Thread_run_flag = 1;
            m_pSecondary_Thread->Run();

            m_bok = true;
        }
    }
    
    else if(m_connection_type == NETWORK){
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
        
#ifdef __UNIX__
# if wxCHECK_VERSION(3,0,0)
        in_addr_t addr = ((struct sockaddr_in *) m_addr.GetAddressData())->sin_addr.s_addr;
# else
        in_addr_t addr = ((struct sockaddr_in *) m_addr.GetAddress()->m_addr)->sin_addr.s_addr;
# endif
#else
        unsigned int addr = inet_addr(m_addr.IPAddress().mb_str());
#endif
        // Create the socket
        switch(m_net_protocol){
            case GPSD: {
                m_sock = new wxSocketClient();
                m_sock->SetEventHandler(*this, DS_SOCKET_ID);
                m_sock->SetNotify(wxSOCKET_CONNECTION_FLAG | wxSOCKET_INPUT_FLAG | wxSOCKET_LOST_FLAG);
                m_sock->Notify(TRUE);
                m_sock->SetTimeout(1);              // Short timeout

                wxSocketClient* tcp_socket = dynamic_cast<wxSocketClient*>(m_sock);
                tcp_socket->Connect(m_addr, FALSE);
                m_brx_connect_event = false;
            
                break;
            }
            case TCP: {
                int isServer = ((addr == INADDR_ANY)?1:0);

                wxSocketBase* tsock;

                if (isServer) {
                    m_socket_server = new wxSocketServer(m_addr, wxSOCKET_REUSEADDR );
                    tsock = m_socket_server;
                } else {
                    m_sock = new wxSocketClient();
                    tsock = m_sock;
                }

                // if((m_io_select != DS_TYPE_INPUT) && (isServer?m_socket_server->IsOk():m_sock->IsOk())) {
                if (isServer) {
                    m_socket_server->SetEventHandler(*this, DS_SERVERSOCKET_ID);
                    m_socket_server->SetNotify( wxSOCKET_CONNECTION_FLAG );
                    m_socket_server->Notify(TRUE);
                    m_socket_server->SetTimeout(1);    // Short timeout
                }
                else {
                    m_sock->SetEventHandler(*this, DS_SOCKET_ID);
                    int notify_flags=(wxSOCKET_CONNECTION_FLAG | wxSOCKET_LOST_FLAG );
                    if (m_io_select != DS_TYPE_INPUT)
                        notify_flags |= wxSOCKET_OUTPUT_FLAG;
                    if (m_io_select != DS_TYPE_OUTPUT)
                        notify_flags |= wxSOCKET_INPUT_FLAG;
                    m_sock->SetNotify(notify_flags);
                    m_sock->Notify(TRUE);
                    m_sock->SetTimeout(1);              // Short timeout

                    m_brx_connect_event = false;
                    m_socket_timer.Start(100, wxTIMER_ONE_SHOT);    // schedule a connection
                }
                
                break;
            }
            case UDP:
                if(m_io_select != DS_TYPE_OUTPUT) {
                    //  We need a local (bindable) address to create the Datagram receive socket
                    // Set up the receive socket
                    wxIPV4address conn_addr;
                    conn_addr.Service(m_net_port);
                    conn_addr.AnyAddress();    
                    m_sock = new wxDatagramSocket(conn_addr, wxSOCKET_NOWAIT | wxSOCKET_REUSEADDR);

                    // Test if address is IPv4 multicast
                    if ((ntohl(addr) & 0xf0000000)  == 0xe0000000) {
                        m_is_multicast=true;
                        m_mrq.imr_multiaddr.s_addr = addr;
                        m_mrq.imr_interface.s_addr = INADDR_ANY;

                        m_sock->SetOption(IPPROTO_IP,IP_ADD_MEMBERSHIP,&m_mrq, sizeof(m_mrq));
                    }
                
                    m_sock->SetEventHandler(*this, DS_SOCKET_ID);
                
                    m_sock->SetNotify(wxSOCKET_CONNECTION_FLAG |
                    wxSOCKET_INPUT_FLAG |
                    wxSOCKET_LOST_FLAG);
                    m_sock->Notify(TRUE);
                    m_sock->SetTimeout(1);              // Short timeout
                }
                
                // Set up another socket for transmit
                if(m_io_select != DS_TYPE_INPUT) {
                    wxIPV4address tconn_addr;
                    tconn_addr.Service(0);          // use ephemeral out port
                    tconn_addr.AnyAddress();    
                    m_tsock = new wxDatagramSocket(tconn_addr, wxSOCKET_NOWAIT | wxSOCKET_REUSEADDR);
                    // Here would be the place to disable multicast loopback
                    // but for consistency with broadcast behaviour, we will
                    // instead rely on setting priority levels to ignore
                    // sentences read back that have just been transmitted
                    if ((!m_is_multicast) && ( m_addr.IPAddress().EndsWith(_T("255")))) {
                        int broadcastEnable=1;
                        bool bam = m_tsock->SetOption(SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));
                    }
                }
                
                break;
                
            default:
                break;
                
        } 
        m_bok = true;
        
    }  // NETWORK       
    
    else if(m_connection_type == INTERNAL_GPS){
#ifdef __OCPN__ANDROID__
        androidStartNMEA(m_consumer);
        m_bok = true;
#endif
        
    }

    else if(m_connection_type == INTERNAL_BT){
#ifdef __OCPN__ANDROID__
        m_bok = androidStartBT(m_consumer, m_portstring );
#endif
    }
    
        
    else
        m_bok = false;

    m_connect_time = wxDateTime::Now();
    
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

            //          Read the reply, one character at a time, looking for 0x0a (lf)
            //          If the reply contains no lf, break on the buffer full

            std::vector<char> data(RD_BUF_SIZE+1);
            event.GetSocket()->Read(&data.front(),RD_BUF_SIZE);
            if(!event.GetSocket()->Error())
            {
                size_t count = event.GetSocket()->LastCount();
                if(count)
                {
                    data[count]=0;
//                    m_sock_buffer.Append(data);
                    m_sock_buffer += (&data.front());
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

bool DataStream::SendSentence( const wxString &sentence )
{
    wxString payload = sentence;
    if( !sentence.EndsWith(_T("\r\n")) )
        payload += _T("\r\n");

    switch( m_connection_type ) {
        case SERIAL:{
            if( m_pSecondary_Thread ) {
                if( IsSecThreadActive() )
                {
                    int retry = 10;
                    while( retry ) {
                        if( m_pSecondary_Thread->SetOutMsg( payload ))
                            return true;
                        else
                            retry--;
                    }
                    return false;   // could not send after several tries....
                }
                else
                    return false;
            }
            break;
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
                                    tcp_socket->Close();
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
                        if( udp_socket->IsOk() ) {
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

//----------------------------------------------------------------------------
// Garmin Device Management
// Handle USB and Serial Port Garmin PVT protocol data interface.
//----------------------------------------------------------------------------

#ifdef __WXMSW__
BOOL IsUserAdmin(VOID)
/*++
 *            Routine Description: This routine returns TRUE if the caller's
 *            process is a member of the Administrators local group. Caller is NOT
 *            expected to be impersonating anyone and is expected to be able to
 *            open its own process and process token.
 *            Arguments: None.
 * Return Value:
 *            TRUE - Caller has Administrators local group.
 *            FALSE - Caller does not have Administrators local group. --
 */
{
    BOOL b;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdministratorsGroup;
    b = AllocateAndInitializeSid(
        &NtAuthority,
        2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0,
        &AdministratorsGroup);
    if(b)
    {
        if (!CheckTokenMembership( NULL, AdministratorsGroup, &b))
        {
            b = FALSE;
        }
        FreeSid(AdministratorsGroup);
    }
    
    return(b);
}

void
le_write16(void *addr, const unsigned value)
{
    unsigned char *p = (unsigned char *)addr;
    p[0] = value;
    p[1] = value >> 8;
    
}

void
le_write32(void *addr, const unsigned value)
{
    unsigned char *p = (unsigned char *)addr;
    p[0] = value;
    p[1] = value >> 8;
    p[2] = value >> 16;
    p[3] = value >> 24;
}

signed int
le_read16(const void *addr)
{
    const unsigned char *p = (const unsigned char *)addr;
    return p[0] | (p[1] << 8);
}

signed int
le_read32(const void *addr)
{
    const unsigned char *p = (const unsigned char *)addr;
    return p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
}


#endif




BEGIN_EVENT_TABLE(GarminProtocolHandler, wxEvtHandler)
EVT_TIMER(TIMER_GARMIN1, GarminProtocolHandler::OnTimerGarmin1)
END_EVENT_TABLE()


GarminProtocolHandler::GarminProtocolHandler(DataStream *parent, wxEvtHandler *MessageTarget, bool bsel_usb)
{
    m_pparent = parent;
    m_pMainEventHandler = MessageTarget;
    m_garmin_serial_thread = NULL;
    m_garmin_usb_thread = NULL;    
    m_bOK = false;
    m_busb = bsel_usb;
    
    //      Connect(wxEVT_OCPN_GARMIN, (wxObjectEventFunction)(wxEventFunction)&GarminProtocolHandler::OnEvtGarmin);
    
    char  pvt_on[14] =
    {20, 0, 0, 0, 10, 0, 0, 0, 2, 0, 0, 0, 49, 0};
    
    char  pvt_off[14] =
    {20, 0, 0, 0, 10, 0, 0, 0, 2, 0, 0, 0, 50, 0};
    
#ifdef __WXMSW__    
    if(m_busb) {
        m_usb_handle = INVALID_HANDLE_VALUE;
    
        m_bneed_int_reset = true;
        m_receive_state = rs_fromintr;
        m_ndelay = 0;
    
        wxLogMessage(_T("Searching for Garmin DeviceInterface and Device..."));
    
        if(!FindGarminDeviceInterface()) {
            wxLogMessage(_T("   Find:Is the Garmin USB driver installed?"));
        }
        else {
            if(!ResetGarminUSBDriver())
                wxLogMessage(_T("   Reset:Is the Garmin USB Device plugged in?"));
        }
    }
#endif
    
    //  Not using USB, so try a Garmin port open and device ident
    if(! m_busb ) {
        m_port =  m_pparent->GetPort().AfterFirst(':');      // strip "Serial:"
        
                // Start handler thread
        m_garmin_serial_thread = new GARMIN_Serial_Thread(this, m_pparent, m_pMainEventHandler, m_port );
        m_Thread_run_flag = 1;
        m_garmin_serial_thread->Run();
    }
    
    TimerGarmin1.SetOwner(this, TIMER_GARMIN1);
    TimerGarmin1.Start(100);
}



GarminProtocolHandler::~GarminProtocolHandler()
{
}

void GarminProtocolHandler::Close( void )
{
    TimerGarmin1.Stop();
    
    StopIOThread(true);
    StopSerialThread();
    
}

void GarminProtocolHandler::StopSerialThread(void)
{
    if(m_garmin_serial_thread)
    {
        wxLogMessage(_T("Stopping Garmin Serial thread"));
        m_Thread_run_flag = 0;
        
        int tsec = 5;
        while((m_Thread_run_flag >= 0) && (tsec--))
        {
            wxSleep(1);
        }
        
        wxString msg;
        if(m_Thread_run_flag < 0)
            msg.Printf(_T("Stopped in %d sec."), 5 - tsec);
        else
            msg.Printf(_T("Not Stopped after 5 sec."));
        wxLogMessage(msg);
    }
    
    m_garmin_serial_thread = NULL;
    
}



void GarminProtocolHandler::StopIOThread(bool b_pause)
{
    if(b_pause)
        TimerGarmin1.Stop();
    
    if(m_garmin_usb_thread)
    {
        wxLogMessage(_T("Stopping Garmin USB thread"));
        m_Thread_run_flag = 0;
        
        int tsec = 5;
        while((m_Thread_run_flag >= 0) && (tsec--))
        {
            wxSleep(1);
        }
        
        wxString msg;
        if(m_Thread_run_flag < 0)
            msg.Printf(_T("Stopped in %d sec."), 5 - tsec);
        else
            msg.Printf(_T("Not Stopped after 5 sec."));
        wxLogMessage(msg);
    }
    
    m_garmin_usb_thread = NULL;
    
#ifdef __WXMSW__    
    if(m_busb && (m_usb_handle != INVALID_HANDLE_VALUE) )
        CloseHandle(m_usb_handle);
    m_usb_handle = INVALID_HANDLE_VALUE;
#endif
    
    m_ndelay = 30;          // Fix delay for next restart
    
}

void GarminProtocolHandler::RestartIOThread(void)
{
    wxLogMessage(_T("Restarting Garmin I/O thread"));
    TimerGarmin1.Start(1000);
}




void GarminProtocolHandler::OnTimerGarmin1(wxTimerEvent& event)
{
    char  pvt_on[14] =
    {20, 0, 0, 0, 10, 0, 0, 0, 2, 0, 0, 0, 49, 0};
    
    TimerGarmin1.Stop();
    
    if(m_busb) {
        #ifdef __WXMSW__    
        //  Try to open the Garmin USB device
        if(INVALID_HANDLE_VALUE == m_usb_handle)
        {
            if(INVALID_HANDLE_VALUE != garmin_usb_start())
            {
                //    Send out a request for Garmin PVT data
                m_receive_state = rs_fromintr;
                gusb_cmd_send((const garmin_usb_packet *) pvt_on, sizeof(pvt_on));
                
                //    Start the pump
                m_garmin_usb_thread = new GARMIN_USB_Thread(this, m_pparent,
						m_pMainEventHandler, (wxIntPtr)m_usb_handle, m_max_tx_size);
                m_Thread_run_flag = 1;
                m_garmin_usb_thread->Run();
            }
        }
        #endif
    }
    
    TimerGarmin1.Start(1000);
}

#ifdef __WXMSW__
bool GarminProtocolHandler::ResetGarminUSBDriver()
{
    OSVERSIONINFO version_info;
    version_info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    
    if(GetVersionEx(&version_info))
    {
        if(version_info.dwMajorVersion > 5)
        {
            if(!IsUserAdmin())
            {
                wxLogMessage(_T("    GarminUSBDriver Reset skipped, requires elevated privileges on Vista and later...."));
                return true;
            }
        }
    }
    
    
    HDEVINFO devs;
    SP_DEVINFO_DATA devInfo;
    SP_PROPCHANGE_PARAMS pchange;
    
    devs = SetupDiGetClassDevs( (GUID *) &GARMIN_GUID1, NULL, NULL,
                                DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);
    if (devs == INVALID_HANDLE_VALUE)
        return false;
    
    devInfo.cbSize = sizeof(devInfo);
    if(!SetupDiEnumDeviceInfo(devs,0,&devInfo))
    {
        wxLogMessage(_T("   GarminUSBDriver Reset0 failed..."));
        return false;
    }
    
    pchange.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
    pchange.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
    pchange.StateChange = DICS_PROPCHANGE;
    pchange.Scope = DICS_FLAG_CONFIGSPECIFIC;
    pchange.HwProfile = 0;
    
    if(!SetupDiSetClassInstallParams(devs,&devInfo,&pchange.ClassInstallHeader,sizeof(pchange)))
    {
        wxLogMessage(_T("   GarminUSBDriver Reset1 failed..."));
        return false;
    }
    
    if(!SetupDiCallClassInstaller(DIF_PROPERTYCHANGE,devs,&devInfo))
    {
        wxLogMessage(_T("   GarminUSBDriver Reset2 failed..."));
        return false;
    }
    
    wxLogMessage(_T("GarminUSBDriver Reset succeeded."));
    
    return true;
    
}



bool GarminProtocolHandler::FindGarminDeviceInterface()
{      //    Search for a useable Garmin Device Interface Class

HDEVINFO hdevinfo;
SP_DEVINFO_DATA devInfo;

hdevinfo = SetupDiGetClassDevs( (GUID *) &GARMIN_GUID1, NULL, NULL,
                                DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);

if (hdevinfo != INVALID_HANDLE_VALUE)
{
    devInfo.cbSize = sizeof(devInfo);
    if(!SetupDiEnumDeviceInfo(hdevinfo,0,&devInfo))
    {
        return false;
    }
}

return true;
}


bool GarminProtocolHandler::IsGarminPlugged()
{
    DWORD size = 0;
    
    HDEVINFO hdevinfo;
    SP_DEVICE_INTERFACE_DATA infodata;
    
    //    Search for the Garmin Device Interface Class
    hdevinfo = SetupDiGetClassDevs( (GUID *) &GARMIN_GUID1, NULL, NULL,
                                    DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);
    
    if (hdevinfo == INVALID_HANDLE_VALUE)
        return INVALID_HANDLE_VALUE;
    
    infodata.cbSize = sizeof(infodata);
    
    bool bgarmin_unit_found = (SetupDiEnumDeviceInterfaces(hdevinfo,
                                                           NULL,(GUID *) &GARMIN_GUID1, 0, &infodata) != 0);
    
    if(!bgarmin_unit_found)
        return false;
    
    PSP_INTERFACE_DEVICE_DETAIL_DATA pdd = NULL;
    SP_DEVINFO_DATA devinfo;
    
    SetupDiGetDeviceInterfaceDetail(hdevinfo, &infodata,
                                    NULL, 0, &size, NULL);
    
    pdd = (PSP_INTERFACE_DEVICE_DETAIL_DATA)malloc(size);
    pdd->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);
    
    devinfo.cbSize = sizeof(SP_DEVINFO_DATA);
    if (!SetupDiGetDeviceInterfaceDetail(hdevinfo, &infodata,
        pdd, size, NULL, &devinfo))
    {
        free(pdd);
        return false;
    }
    
    free(pdd);
    
    return true;
}


HANDLE GarminProtocolHandler::garmin_usb_start()
{
    DWORD size = 0;
    
    HDEVINFO hdevinfo;
    SP_DEVICE_INTERFACE_DATA infodata;
    
    //    Search for the Garmin Device Interface Class
    hdevinfo = SetupDiGetClassDevs( (GUID *) &GARMIN_GUID1, NULL, NULL,
                                    DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);
    
    if (hdevinfo == INVALID_HANDLE_VALUE)
        return INVALID_HANDLE_VALUE;
    
    infodata.cbSize = sizeof(infodata);
    
    bool bgarmin_unit_found = (SetupDiEnumDeviceInterfaces(hdevinfo,
                                                           NULL,(GUID *) &GARMIN_GUID1, 0, &infodata) != 0);
    
    if(!bgarmin_unit_found)
        return INVALID_HANDLE_VALUE;
    
    wxLogMessage(_T("Garmin USB Device Found"));
    
    if((m_usb_handle == INVALID_HANDLE_VALUE) || (m_usb_handle == 0))
    {
        PSP_INTERFACE_DEVICE_DETAIL_DATA pdd = NULL;
        SP_DEVINFO_DATA devinfo;
        
        SetupDiGetDeviceInterfaceDetail(hdevinfo, &infodata,
                                        NULL, 0, &size, NULL);
        
        pdd = (PSP_INTERFACE_DEVICE_DETAIL_DATA)malloc(size);
        pdd->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);
        
        devinfo.cbSize = sizeof(SP_DEVINFO_DATA);
        if (!SetupDiGetDeviceInterfaceDetail(hdevinfo, &infodata,
            pdd, size, NULL, &devinfo))
        {
            wxLogMessage(_T("   SetupDiGetDeviceInterfaceDetail failed for Garmin Device..."));
            free(pdd);
            return INVALID_HANDLE_VALUE;
        }
        
        /* Whew.  All that just to get something we can open... */
        //            wxString msg;
        //            msg.Printf(_T("Windows GUID for interface is %s"),pdd->DevicePath);
        //            wxLogMessage(msg);
        
        if(m_bneed_int_reset)
        {
            ResetGarminUSBDriver();
            m_bneed_int_reset = false;
        }
        
        m_usb_handle = CreateFile(pdd->DevicePath, GENERIC_READ|GENERIC_WRITE,
                                  0, NULL, OPEN_EXISTING, 0, NULL );
        
        
        if (m_usb_handle == INVALID_HANDLE_VALUE)
        {
            wxString msg;
            msg.Printf(_T("   (usb) CreateFile on '%s' failed"), pdd->DevicePath);
            wxLogMessage(msg);
        }

/*        
        DEV_BROADCAST_HANDLE filterData;
        filterData.dbch_size = sizeof(DEV_BROADCAST_HANDLE);
        filterData.dbch_devicetype = DBT_DEVTYP_HANDLE;
        filterData.dbch_reserved = 0;
        filterData.dbch_handle = m_usb_handle;     // file handle used in call to RegisterDeviceNotification
        filterData.dbch_hdevnotify = 0;            // returned from RegisterDeviceNotification
        
        HDEVNOTIFY m_hDevNotify = RegisterDeviceNotification( GetHWND(), &filterData, DEVICE_NOTIFY_WINDOW_HANDLE);
*/        
        
        free(pdd);
    }
    
    m_max_tx_size = 0;
    
    if(!DeviceIoControl(m_usb_handle, IOCTL_GARMIN_USB_BULK_OUT_PACKET_SIZE,
        NULL, 0, &m_max_tx_size, GARMIN_USB_INTERRUPT_DATA_SIZE,
        &size, NULL))
    {
        wxLogMessage(_T("   Couldn't get Garmin USB packet size."));
        CloseHandle(m_usb_handle);
        m_usb_handle = INVALID_HANDLE_VALUE;
        return INVALID_HANDLE_VALUE;
    }
    
    if(!gusb_syncup())
    {
        CloseHandle(m_usb_handle);
        m_usb_handle = INVALID_HANDLE_VALUE;
    }
    
    return m_usb_handle;
}


bool GarminProtocolHandler::gusb_syncup(void)
{
    static int unit_number;
    static const char  oinit[12] =
    {0, 0, 0, 0, GUSB_SESSION_START, 0, 0, 0, 0, 0, 0, 0};
    garmin_usb_packet iresp;
    int i;
    
    /*
     * This is our first communication with the unit.
     */
    
    
    m_receive_state = rs_fromintr;
    
    for(i = 0; i < 25; i++) {
        le_write16(&iresp.gusb_pkt.pkt_id[0], 0);
        le_write32(&iresp.gusb_pkt.datasz[0], 0);
        le_write32(&iresp.gusb_pkt.databuf[0], 0);
        
        if(gusb_cmd_send((const garmin_usb_packet *) oinit, sizeof(oinit)))
        {
            gusb_cmd_get(&iresp, sizeof(iresp));
            
            if ((le_read16(iresp.gusb_pkt.pkt_id) == GUSB_SESSION_ACK) &&
                (le_read32(iresp.gusb_pkt.datasz) == 4))
            {
                //                unsigned serial_number = le_read32(iresp.gusb_pkt.databuf);
                //                garmin_unit_info[unit_number].serial_number = serial_number;
                //                gusb_id_unit(&garmin_unit_info[unit_number]);
                
                unit_number++;
                
                wxLogMessage(_T("Successful Garmin USB syncup."));
                return true;;
            }
        }
    }
    wxLogMessage(_T("   Unable to establish Garmin USB syncup."));
    return false;
}

int GarminProtocolHandler::gusb_cmd_send(const garmin_usb_packet *opkt, size_t sz)
{
    unsigned int rv;
    
    unsigned char *obuf = (unsigned char *) &opkt->dbuf[0];
    
    rv = gusb_win_send(opkt, sz);
    
    /*
     * Recursion, when used in a disciplined way, can be our friend.
     *
     * The Garmin protocol requires that packets that are exactly
     * a multiple of the max tx size be followed by a zero length
     * packet.  Do that here so we can see it in debugging traces.
     */
    
    if (sz && !(sz % m_max_tx_size)) {
        wxLogMessage(_T("win_send_call1"));
        gusb_win_send(opkt, 0);
        wxLogMessage(_T("win_send_ret1"));
    }
    
    return (rv);
}


int GarminProtocolHandler::gusb_cmd_get(garmin_usb_packet *ibuf, size_t sz)
{
    int rv = 0;
    unsigned char *buf = (unsigned char *) &ibuf->dbuf[0];
    int orig_receive_state;
    top:
    orig_receive_state = m_receive_state;
    switch (m_receive_state) {
        case rs_fromintr:
            rv = gusb_win_get(ibuf, sz);
            break;
        case rs_frombulk:
            rv = gusb_win_get_bulk(ibuf, sz);
            break;
            
    }
    
    /* Adjust internal state and retry the read */
    if ((rv > 0) && (ibuf->gusb_pkt.pkt_id[0] == GUSB_REQUEST_BULK)) {
        m_receive_state = rs_frombulk;
        goto top;
    }
    /*
     * If we were reading from the bulk pipe and we just got
     * a zero request, adjust our internal state.
     * It's tempting to retry the read here to hide this "stray"
     * packet from our callers, but that only works when you know
     * there's another packet coming.   That works in every case
     * except the A000 discovery sequence.
     */
    if ((m_receive_state == rs_frombulk) && (rv <= 0)) {
        m_receive_state = rs_fromintr;
    }
    
    return rv;
}





int GarminProtocolHandler::gusb_win_get(garmin_usb_packet *ibuf, size_t sz)
{
    DWORD rxed = GARMIN_USB_INTERRUPT_DATA_SIZE;
    unsigned char *buf = (unsigned char *) &ibuf->dbuf[0];
    int tsz=0;
    
    while (sz)
    {
        /* The driver wrongly (IMO) rejects reads smaller than
         * GARMIN_USB_INTERRUPT_DATA_SIZE
         */
        if(!DeviceIoControl(m_usb_handle, IOCTL_GARMIN_USB_INTERRUPT_IN, NULL, 0,
            buf, GARMIN_USB_INTERRUPT_DATA_SIZE, &rxed, NULL))
        {
            //                GPS_Serial_Error("Ioctl");
            //                fatal("ioctl\n");
        }
        
        buf += rxed;
        sz  -= rxed;
        tsz += rxed;
        if (rxed < GARMIN_USB_INTERRUPT_DATA_SIZE)
            break;
    }
    return tsz;
}

int GarminProtocolHandler::gusb_win_get_bulk(garmin_usb_packet *ibuf, size_t sz)
{
    int n;
    DWORD rsz;
    unsigned char *buf = (unsigned char *) &ibuf->dbuf[0];
    
    n = ReadFile(m_usb_handle, buf, sz, &rsz, NULL);
    
    return rsz;
}

int GarminProtocolHandler::gusb_win_send(const garmin_usb_packet *opkt, size_t sz)
{
    DWORD rsz;
    unsigned char *obuf = (unsigned char *) &opkt->dbuf[0];
    
    /* The spec warns us about making writes an exact multiple
     * of the packet size, but isn't clear whether we can issue
     * data in a single call to WriteFile if it spans buffers.
     */
    WriteFile(m_usb_handle, obuf, sz, &rsz, NULL);
	int err = GetLastError();
    
    //    if (rsz != sz)
    //          fatal ("Error sending %d bytes.   Successfully sent %ld\n", sz, rsz);
    
    
    return rsz;
}

/*
WXLRESULT GarminProtocolHandler::MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam)
{
    // did we process the message?
    bool processed = false;
    
    // the return value
    bool rc;
    PDEV_BROADCAST_HDR pDBHdr;
    PDEV_BROADCAST_HANDLE pDBHandle;
    
    // for most messages we should return 0 when we do process the message
    rc = 0;
    
    switch ( message )
    {
        case WM_DEVICECHANGE:
            switch (wParam)
            {
                case DBT_DEVICEREMOVEPENDING:
                case DBT_DEVICEREMOVECOMPLETE:
                    pDBHdr = (PDEV_BROADCAST_HDR) lParam;
                    switch (pDBHdr->dbch_devicetype)
                        case DBT_DEVTYP_HANDLE:
                            // A Device has been removed
                            // Stop the IO thread and close open handle  to device
                            
                            pDBHandle = (PDEV_BROADCAST_HANDLE) pDBHdr;
                            HANDLE target_handle = pDBHandle->dbch_handle;
                            
                            wxLogMessage(_T("Garmin USB Device Removed"));
                            StopIOThread(false);
                            m_bneed_int_reset = true;
                            processed = true;
                            break;
            }
            
            break;
            
    }
    
    if ( !processed )
    {
        rc = (MSWDefWindowProc(message, wParam, lParam) != 0);
    }
    
    return rc;
}
*/
#endif

D800_Pvt_Data_Type_Aligned mypvt;

//-------------------------------------------------------------------------------------------------------------
//
//    Garmin Serial Port Worker Thread
//
//    This thread manages reading the positioning data stream from the declared Garmin GRMN Mode serial device
//
//-------------------------------------------------------------------------------------------------------------
GARMIN_Serial_Thread::GARMIN_Serial_Thread(GarminProtocolHandler *parent,
                                           DataStream *GParentStream,
                                           wxEvtHandler *MessageTarget,
                                           wxString port)
{
    m_parent = parent;                          // This thread's immediate "parent"
    m_parent_stream = GParentStream;
    m_pMessageTarget = MessageTarget;
    m_port = port;
    
    Create();
}

GARMIN_Serial_Thread::~GARMIN_Serial_Thread(void)
{
}


//    Entry Point
void *GARMIN_Serial_Thread::Entry()
{
 //   m_parent->SetSecThreadActive();               // I am alive
    m_bdetected = false;
    m_bconnected = false;
    
    bool not_done = true;
    wxDateTime last_rx_time;
 
   
#ifdef USE_GARMINHOST
    //    The main loop
    
    while((not_done) && (m_parent->m_Thread_run_flag > 0)) {

        if(TestDestroy()) {
            not_done = false;                               // smooth exit
            goto thread_exit;
        }

        while( !m_bdetected ) {
            
            //  Try to init the port once
            int v_init = Garmin_GPS_Init(m_port);
            if( v_init < 0 ){           //  Open failed, so sleep and try again
                for( int i=0 ; i < 4 ; i++) {
                    wxSleep(1);
                    if(TestDestroy())
                        goto thread_exit;
                    if( !m_parent->m_Thread_run_flag )
                        goto thread_exit;
                }
            }
            else
                m_bdetected = true;
        }                       // while not detected
    
        // Detected OK
        
        //      Start PVT packet transmission
        if( !m_bconnected ) {
            if( !Garmin_GPS_PVT_On( m_port) ) {  
                m_bdetected = false;            // error, would not accept PVT On
                m_bconnected = false;
            }
            else
                m_bconnected = true;
        }
        
        if( m_bconnected ) {
                
                D800_Pvt_Data_Type_Aligned *ppvt = &mypvt;
                int ret = Garmin_GPS_GetPVT(&ppvt);
                if(ret > 0) {
                    if((mypvt.fix) >= 2 && (mypvt.fix <= 5)) {
                        // Synthesize an NMEA GMRMC message
                        SENTENCE snt;
                        NMEA0183 oNMEA0183;
                        oNMEA0183.TalkerID = _T ( "GM" );
                        
                        if ( mypvt.lat < 0. )
                            oNMEA0183.Rmc.Position.Latitude.Set ( -mypvt.lat, _T ( "S" ) );
                        else
                            oNMEA0183.Rmc.Position.Latitude.Set ( mypvt.lat, _T ( "N" ) );
                        
                        if ( mypvt.lon < 0. )
                            oNMEA0183.Rmc.Position.Longitude.Set ( -mypvt.lon, _T ( "W" ) );
                        else
                            oNMEA0183.Rmc.Position.Longitude.Set ( mypvt.lon, _T ( "E" ) );
                        
                        /* speed over ground */
                        double sog = sqrt(mypvt.east*mypvt.east + mypvt.north*mypvt.north) * 3.6 / 1.852;
                        oNMEA0183.Rmc.SpeedOverGroundKnots = sog;
                        
                        /* course over ground */
                        double course = atan2(mypvt.east, mypvt.north);
                        if (course < 0)
                            course += 2*PI;
                        double cog = course * 180 / PI;
                        oNMEA0183.Rmc.TrackMadeGoodDegreesTrue = cog;
                                               
                        oNMEA0183.Rmc.IsDataValid = NTrue;
                            
                        oNMEA0183.Rmc.Write ( snt );
                        wxString message = snt.Sentence;

                        if( m_pMessageTarget ) {
                            OCPN_DataStreamEvent Nevent(wxEVT_OCPN_DATASTREAM, 0);
                            wxCharBuffer buffer=message.ToUTF8();
                            if(buffer.data()) {
                                Nevent.SetNMEAString( buffer.data() );
                                Nevent.SetStream( m_parent_stream );
                                
                                m_pMessageTarget->AddPendingEvent(Nevent);
                            }
                        }
                        
                        last_rx_time = wxDateTime::Now();
                        
                    }
                }
                else {
                    wxDateTime now = wxDateTime::Now();
                    if( last_rx_time.IsValid() ) {
                        wxTimeSpan delta_time = now - last_rx_time;
                        if( delta_time.GetSeconds() > 5 ) {
                            m_bdetected = false;
                            m_bconnected = false;
                            Garmin_GPS_ClosePortVerify();
                        }
                    }
                }
        }
    }                          // the big while...
            
thread_exit:

    Garmin_GPS_PVT_Off( m_port);  
    Garmin_GPS_ClosePortVerify();

#else           //#ifdef USE_GARMINHOST

    while((not_done) && (m_parent->m_Thread_run_flag > 0)) {

        wxSleep(1);
        if(TestDestroy()) {
            not_done = false;                               // smooth exit
            goto thread_exit;
        }
    }
        
thread_exit:

#endif          //#ifdef USE_GARMINHOST

    m_parent->m_Thread_run_flag = -1;   // in GarminProtocolHandler        
    return 0;
}
        
        
//-------------------------------------------------------------------------------------------------------------
//    GARMIN_USB_Thread Implementation
//-------------------------------------------------------------------------------------------------------------

GARMIN_USB_Thread::GARMIN_USB_Thread(GarminProtocolHandler *parent,
                                     DataStream *GParentStream,
                                     wxEvtHandler *MessageTarget,
                                     unsigned int device_handle,
                                     size_t max_tx_size)
{
      m_parent = parent;                        // This thread's immediate "parent"
      m_parent_stream = GParentStream;
      m_pMessageTarget = MessageTarget;
      m_max_tx_size = max_tx_size;

#ifdef __WXMSW__      
      m_usb_handle = (HANDLE)(device_handle & 0xffff);
#endif      

      Create();
}

GARMIN_USB_Thread::~GARMIN_USB_Thread()
{
}

void *GARMIN_USB_Thread::Entry()
{
      garmin_usb_packet iresp;
          int n_short_read = 0;
      m_receive_state = rs_fromintr;

      //    Here comes the big while loop
      while(m_parent->m_Thread_run_flag > 0)
      {
            if(TestDestroy())
                  goto thread_prexit;                               // smooth exit

      //    Get one  packet

            int nr = gusb_cmd_get(&iresp, sizeof(iresp));

            if(iresp.gusb_pkt.pkt_id[0] == GUSB_RESPONSE_SDR)     //Satellite Data Record
            {
                  unsigned char *t = (unsigned char *)&(iresp.gusb_pkt.databuf[0]);
                  for(int i=0 ; i < 12 ; i++)
                  {
                        m_sat_data[i].svid =  *t++;
                        m_sat_data[i].snr =   ((*t)<<8) + *(t+1); t += 2;
                        m_sat_data[i].elev =  *t++;
                        m_sat_data[i].azmth = ((*t)<<8) + *(t+1); t += 2;
                        m_sat_data[i].status = *t++;
                  }

                  m_nSats = 0;
                  for(int i=0 ; i < 12 ; i++)
                  {
                        if(m_sat_data[i].svid != 255)
                              m_nSats++;
                  }
                  
                  // Synthesize an NMEA GMGSV message
                  SENTENCE snt;
                  NMEA0183 oNMEA0183;
                  oNMEA0183.TalkerID = _T ( "GM" );
                  oNMEA0183.Gsv.SatsInView = m_nSats;
                  
                  oNMEA0183.Gsv.Write ( snt );
                  wxString message = snt.Sentence;

                  if( m_pMessageTarget ) {
                    OCPN_DataStreamEvent Nevent(wxEVT_OCPN_DATASTREAM, 0);
                    wxCharBuffer buffer=message.ToUTF8();
                    if(buffer.data()) {
                        Nevent.SetNMEAString( buffer.data() );
                        Nevent.SetStream( m_parent_stream );
                        
                        m_pMessageTarget->AddPendingEvent(Nevent);
                    }
                  }
                  
            }

            if(iresp.gusb_pkt.pkt_id[0] == GUSB_RESPONSE_PVT)     //PVT Data Record
            {


                  D800_Pvt_Data_Type *ppvt = (D800_Pvt_Data_Type *)&(iresp.gusb_pkt.databuf[0]);

                  if((ppvt->fix) >= 2 && (ppvt->fix <= 5)) {
                          // Synthesize an NMEA GMRMC message
                          SENTENCE snt;
                          NMEA0183 oNMEA0183;
                          oNMEA0183.TalkerID = _T ( "GM" );
                          
                          if ( ppvt->lat < 0. )
                              oNMEA0183.Rmc.Position.Latitude.Set ( -ppvt->lat*180./PI, _T ( "S" ) );
                          else
                              oNMEA0183.Rmc.Position.Latitude.Set ( ppvt->lat*180./PI, _T ( "N" ) );
                          
                          if ( ppvt->lon < 0. )
                              oNMEA0183.Rmc.Position.Longitude.Set ( -ppvt->lon*180./PI, _T ( "W" ) );
                          else
                              oNMEA0183.Rmc.Position.Longitude.Set ( ppvt->lon*180./PI, _T ( "E" ) );
                          
                          /* speed over ground */
                          double sog = sqrt(ppvt->east*ppvt->east + ppvt->north*ppvt->north) * 3.6 / 1.852;
                          oNMEA0183.Rmc.SpeedOverGroundKnots = sog;
                          
                          /* course over ground */
                          double course = atan2(ppvt->east, ppvt->north);
                          if (course < 0)
                              course += 2*PI;
                          double cog = course * 180 / PI;
                          oNMEA0183.Rmc.TrackMadeGoodDegreesTrue = cog;
                          
                          oNMEA0183.Rmc.IsDataValid = NTrue;
                          
                          oNMEA0183.Rmc.Write ( snt );
                          wxString message = snt.Sentence;

                          if( m_pMessageTarget ) {
                            OCPN_DataStreamEvent Nevent(wxEVT_OCPN_DATASTREAM, 0);
                            wxCharBuffer buffer=message.ToUTF8();
                            if(buffer.data()) {
                                Nevent.SetNMEAString( buffer.data() );
                                Nevent.SetStream( m_parent_stream );
                                
                                m_pMessageTarget->AddPendingEvent(Nevent);
                            }
                          }
                          
                   }
                  
            }

      }

thread_prexit:
      m_parent->m_Thread_run_flag = -1;
      return 0;
}


int GARMIN_USB_Thread::gusb_cmd_get(garmin_usb_packet *ibuf, size_t sz)
{
      int rv = 0;
      unsigned char *buf = (unsigned char *) &ibuf->dbuf[0];
      int orig_receive_state;
top:
      orig_receive_state = m_receive_state;
      switch (m_receive_state) {
            case rs_fromintr:
                  rv = gusb_win_get(ibuf, sz);
                  break;
            case rs_frombulk:
                  rv = gusb_win_get_bulk(ibuf, sz);
                  break;
      }

      /* Adjust internal state and retry the read */
      if ((rv > 0) && (ibuf->gusb_pkt.pkt_id[0] == GUSB_REQUEST_BULK)) {
            m_receive_state = rs_frombulk;
            goto top;
      }
      /*
      * If we were reading from the bulk pipe and we just got
      * a zero request, adjust our internal state.
      * It's tempting to retry the read here to hide this "stray"
      * packet from our callers, but that only works when you know
      * there's another packet coming.   That works in every case
      * except the A000 discovery sequence.
      */
      if ((m_receive_state == rs_frombulk) && (rv <= 0)) {
            m_receive_state = rs_fromintr;
      }

      return rv;
}

int GARMIN_USB_Thread::gusb_win_get(garmin_usb_packet *ibuf, size_t sz)
{
    int tsz=0;
#ifdef __WXMSW__    
    DWORD rxed = GARMIN_USB_INTERRUPT_DATA_SIZE;
      unsigned char *buf = (unsigned char *) &ibuf->dbuf[0];

      while (sz)
      {
            /* The driver wrongly (IMO) rejects reads smaller than
            * GARMIN_USB_INTERRUPT_DATA_SIZE
            */
            if(!DeviceIoControl(m_usb_handle, IOCTL_GARMIN_USB_INTERRUPT_IN, NULL, 0,
                buf, GARMIN_USB_INTERRUPT_DATA_SIZE, &rxed, NULL))
            {
//                GPS_Serial_Error("Ioctl");
//                fatal("ioctl\n");
            }

            buf += rxed;
            sz  -= rxed;
            tsz += rxed;
            if (rxed < GARMIN_USB_INTERRUPT_DATA_SIZE)
                  break;
      }
      
#endif      
      return tsz;
}

int GARMIN_USB_Thread::gusb_win_get_bulk(garmin_usb_packet *ibuf, size_t sz)
{
      int n;
      int ret_val = 0;
#ifdef __WXMSW__      
      DWORD rsz;
      unsigned char *buf = (unsigned char *) &ibuf->dbuf[0];

      n = ReadFile(m_usb_handle, buf, sz, &rsz, NULL);
      ret_val = rsz;
#endif
      
      return ret_val;
}
        
        
bool DataStream::SetOutputSocketOptions(wxSocketBase* tsock)
{
    int ret;

    // Disable nagle algorithm on outgoing connection
    // Doing this here rather than after the accept() is
    // pointless  on platforms where TCP_NODELAY is
    // not inherited.  However, none of OpenCPN's currently
    // supported platforms fall into that category.

    int nagleDisable=1;
    ret = tsock->SetOption(IPPROTO_TCP,TCP_NODELAY,&nagleDisable, sizeof(nagleDisable));
                        
    //  Drastically reduce the size of the socket output buffer
    //  so that when client goes away without properly closing, the stream will
    //  quickly fill the output buffer, and thus fail the write() call
    //  within a few seconds.    
    unsigned long outbuf_size = 1024;       // Smallest allowable value on Linux
    return (tsock->SetOption(SOL_SOCKET,SO_SNDBUF,&outbuf_size, sizeof(outbuf_size)) && ret);
}
