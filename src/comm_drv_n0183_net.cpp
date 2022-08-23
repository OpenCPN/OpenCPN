/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Implement comm_drv_n0183_net.h -- network nmea0183 driver
 * Author:   David Register, Alec Leamas
 *
 ***************************************************************************
 *   Copyright (C) 2022 by David Register, Alec Leamas                     *
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

#ifdef __MINGW32__
#undef IPV6STRICT  // mingw FTBS fix:  missing struct ip_mreq
#include <ws2tcpip.h>
#include <windows.h>
#endif

#ifdef __MSVC__
#include "winsock2.h"
#include "wx/msw/winundef.h"
#endif

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // precompiled headers

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

#include "OCPN_DataStreamEvent.h"
#include "OCP_DataStreamInput_Thread.h"
#include "GarminProtocolHandler.h"

#include "comm_drv_n0183_net.h"
#include "comm_navmsg_bus.h"

#define N_DOG_TIMEOUT 5

wxDEFINE_EVENT(wxEVT_COMMDRIVER_N0183_NET, CommDriverN0183NetEvent);

class CommDriverN0183NetEvent;
wxDECLARE_EVENT(wxEVT_COMMDRIVER_N0183_NET, CommDriverN0183NetEvent);

class CommDriverN0183NetEvent : public wxEvent {
public:
  CommDriverN0183NetEvent(wxEventType commandType = wxEVT_NULL, int id = 0)
      : wxEvent(id, commandType){};
  ~CommDriverN0183NetEvent(){};

  // accessors
  void SetPayload(std::shared_ptr<std::vector<unsigned char>> data) {
    m_payload = data;
  }
  std::shared_ptr<std::vector<unsigned char>> GetPayload() { return m_payload; }

  // required for sending with wxPostEvent()
  wxEvent* Clone() const {
    CommDriverN0183NetEvent* newevent = new CommDriverN0183NetEvent(*this);
    newevent->m_payload = this->m_payload;
    return newevent;
  };

private:
  std::shared_ptr<std::vector<unsigned char>> m_payload;
};

//========================================================================
/*    commdriverN0183Net implementation
 * */

BEGIN_EVENT_TABLE(CommDriverN0183Net, wxEvtHandler)
EVT_TIMER(TIMER_SOCKET, CommDriverN0183Net::OnTimerSocket)
EVT_SOCKET(DS_SOCKET_ID, CommDriverN0183Net::OnSocketEvent)
EVT_SOCKET(DS_SERVERSOCKET_ID, CommDriverN0183Net::OnServerSocketEvent)
EVT_TIMER(TIMER_SOCKET + 1, CommDriverN0183Net::OnSocketReadWatchdogTimer)
END_EVENT_TABLE()

// CommDriverN0183Net::CommDriverN0183Net() : CommDriverN0183() {}

CommDriverN0183Net::CommDriverN0183Net(const ConnectionParams* params,
                                       DriverListener& listener)
    : CommDriverN0183(NavAddr::Bus::N0183,
                      ((ConnectionParams*)params)->GetStrippedDSPort()),
      m_params(*params),
      m_listener(listener),
      m_net_port(wxString::Format("%i", params->NetworkPort)),
      m_net_protocol(params->NetProtocol),
      m_sock(NULL),
      m_tsock(NULL),
      m_socket_server(NULL),
      m_is_multicast(false),
      m_txenter(0),
      m_portstring(params->GetDSPort()),
      m_io_select(params->IOSelect),
      m_connection_type(params->Type),
      m_bok(false)

{
  m_addr.Hostname(params->NetworkAddress);
  m_addr.Service(params->NetworkPort);

  m_socket_timer.SetOwner(this, TIMER_SOCKET);
  m_socketread_watchdog_timer.SetOwner(this, TIMER_SOCKET + 1);

  // Prepare the wxEventHandler to accept events from the actual hardware thread
  Bind(wxEVT_COMMDRIVER_N0183_NET, &CommDriverN0183Net::handle_N0183_MSG, this);

  Open();
}

CommDriverN0183Net::~CommDriverN0183Net() {
  Close();
}

void CommDriverN0183Net::handle_N0183_MSG(CommDriverN0183NetEvent& event) {
  auto p = event.GetPayload();
  std::vector<unsigned char>* payload = p.get();

  // Extract the NMEA0183 sentence
  std::string full_sentence = std::string(payload->begin(), payload->end());

  // FIXME
  //     if (stream) bpass = stream->SentencePassesFilter(message,
  //     FILTER_INPUT);
  //       if ((g_b_legacy_input_filter_behaviour && !bpass) || bpass) {

  if ((full_sentence[0] == '$') || (full_sentence[0] == '!')) {  // Sanity check
    std::string identifier;
    // We notify based on Mnemonic only, ignoring the Talker ID
    identifier = full_sentence.substr(3, 3);

    // Notify listener for msg and also "all" N0183 messages to support plugin API
    auto msg = std::make_shared<const Nmea0183Msg>(identifier, full_sentence, GetAddress());
    auto msg_all = std::make_shared<const Nmea0183Msg>("", full_sentence, GetAddress());

    m_listener.Notify(std::move(msg));
    m_listener.Notify(std::move(msg_all));
  }
}

void CommDriverN0183Net::Open(void) {
#ifdef __UNIX__
#if wxCHECK_VERSION(3, 0, 0)
  in_addr_t addr =
      ((struct sockaddr_in*)GetAddr().GetAddressData())->sin_addr.s_addr;
#else
  in_addr_t addr =
      ((struct sockaddr_in*)GetAddr().GetAddress()->m_addr)->sin_addr.s_addr;
#endif
#else
  unsigned int addr = inet_addr(GetAddr().IPAddress().mb_str());
#endif
  // Create the socket
  switch (m_net_protocol) {
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

void CommDriverN0183Net::OpenNetworkUDP(unsigned int addr) {
  if (GetPortType() != DS_TYPE_OUTPUT) {
    //  We need a local (bindable) address to create the Datagram receive socket
    // Set up the receive socket
    wxIPV4address conn_addr;
    conn_addr.Service(GetNetPort());
    conn_addr.AnyAddress();
    SetSock(
        new wxDatagramSocket(conn_addr, wxSOCKET_NOWAIT | wxSOCKET_REUSEADDR));

    // Test if address is IPv4 multicast
    if ((ntohl(addr) & 0xf0000000) == 0xe0000000) {
      SetMulticast(true);
      SetMrqAddr(addr);
      GetSock()->SetOption(IPPROTO_IP, IP_ADD_MEMBERSHIP, &GetMrq(),
                           sizeof(GetMrq()));
    }

    GetSock()->SetEventHandler(*this, DS_SOCKET_ID);

    GetSock()->SetNotify(wxSOCKET_CONNECTION_FLAG | wxSOCKET_INPUT_FLAG |
                         wxSOCKET_LOST_FLAG);
    GetSock()->Notify(TRUE);
    GetSock()->SetTimeout(1);  // Short timeout
  }

  // Set up another socket for transmit
  if (GetPortType() != DS_TYPE_INPUT) {
    wxIPV4address tconn_addr;
    tconn_addr.Service(0);  // use ephemeral out port
    tconn_addr.AnyAddress();
    SetTSock(
        new wxDatagramSocket(tconn_addr, wxSOCKET_NOWAIT | wxSOCKET_REUSEADDR));
    // Here would be the place to disable multicast loopback
    // but for consistency with broadcast behaviour, we will
    // instead rely on setting priority levels to ignore
    // sentences read back that have just been transmitted
    if ((!GetMulticast()) && (GetAddr().IPAddress().EndsWith(_T("255")))) {
      int broadcastEnable = 1;
      bool bam = GetTSock()->SetOption(
          SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));
    }
  }

  // In case the connection is lost before acquired....
  SetConnectTime(wxDateTime::Now());
}

void CommDriverN0183Net::OpenNetworkTCP(unsigned int addr) {
  int isServer = ((addr == INADDR_ANY) ? 1 : 0);
  wxLogMessage(wxString::Format(_T("Opening TCP Server %d"), isServer));

  if (isServer) {
    SetSockServer(new wxSocketServer(GetAddr(), wxSOCKET_REUSEADDR));
  } else {
    SetSock(new wxSocketClient());
  }

  if (isServer) {
    GetSockServer()->SetEventHandler(*this, DS_SERVERSOCKET_ID);
    GetSockServer()->SetNotify(wxSOCKET_CONNECTION_FLAG);
    GetSockServer()->Notify(TRUE);
    GetSockServer()->SetTimeout(1);  // Short timeout
  } else {
    GetSock()->SetEventHandler(*this, DS_SOCKET_ID);
    int notify_flags = (wxSOCKET_CONNECTION_FLAG | wxSOCKET_LOST_FLAG);
    if (GetPortType() != DS_TYPE_INPUT) notify_flags |= wxSOCKET_OUTPUT_FLAG;
    if (GetPortType() != DS_TYPE_OUTPUT) notify_flags |= wxSOCKET_INPUT_FLAG;
    GetSock()->SetNotify(notify_flags);
    GetSock()->Notify(TRUE);
    GetSock()->SetTimeout(1);  // Short timeout

    SetBrxConnectEvent(false);
    GetSocketTimer()->Start(100, wxTIMER_ONE_SHOT);  // schedule a connection
  }

  // In case the connection is lost before acquired....
  SetConnectTime(wxDateTime::Now());
}

void CommDriverN0183Net::OpenNetworkGPSD() {
  SetSock(new wxSocketClient());
  GetSock()->SetEventHandler(*this, DS_SOCKET_ID);
  GetSock()->SetNotify(wxSOCKET_CONNECTION_FLAG | wxSOCKET_INPUT_FLAG |
                       wxSOCKET_LOST_FLAG);
  GetSock()->Notify(TRUE);
  GetSock()->SetTimeout(1);  // Short timeout

  wxSocketClient* tcp_socket = static_cast<wxSocketClient*>(GetSock());
  tcp_socket->Connect(GetAddr(), FALSE);
  SetBrxConnectEvent(false);
}

void CommDriverN0183Net::OnSocketReadWatchdogTimer(wxTimerEvent& event) {
  m_dog_value--;
  if (m_dog_value <= 0) {  // No receive in n seconds, assume connection lost
    wxLogMessage(
        wxString::Format(_T("    TCP NetworkDataStream watchdog timeout: %s"),
                         GetPort().c_str()));

    if (GetProtocol() == TCP) {
      wxSocketClient* tcp_socket = dynamic_cast<wxSocketClient*>(GetSock());
      if (tcp_socket) {
        tcp_socket->Close();
      }
      GetSocketTimer()->Start(5000, wxTIMER_ONE_SHOT);  // schedule a reconnect
      GetSocketThreadWatchdogTimer()->Stop();
    }
  }
}

void CommDriverN0183Net::OnTimerSocket(wxTimerEvent& event) {
  //  Attempt a connection
  wxSocketClient* tcp_socket = dynamic_cast<wxSocketClient*>(GetSock());
  if (tcp_socket) {
    if (tcp_socket->IsDisconnected()) {
      SetBrxConnectEvent(false);
      tcp_socket->Connect(GetAddr(), FALSE);
      GetSocketTimer()->Start(5000,
                              wxTIMER_ONE_SHOT);  // schedule another attempt
    }
  }
}

void CommDriverN0183Net::OnSocketEvent(wxSocketEvent& event) {
  //#define RD_BUF_SIZE    200
#define RD_BUF_SIZE \
  4096  // Allows handling of high volume data streams, such as a National AIS
        // stream with 100s of msgs a second.

  switch (event.GetSocketEvent()) {
    case wxSOCKET_INPUT:  // from gpsd Daemon
    {
      // TODO determine if the follwing SetFlags needs to be done at every
      // socket event or only once when socket is created, it it needs to be
      // done at all!
      // m_sock->SetFlags(wxSOCKET_WAITALL | wxSOCKET_BLOCK);      // was
      // (wxSOCKET_NOWAIT);

      // We use wxSOCKET_BLOCK to avoid Yield() reentrancy problems
      // if a long ProgressDialog is active, as in S57 SENC creation.

      //    Disable input event notifications to preclude re-entrancy on
      //    non-blocking socket
      //           m_sock->SetNotify(wxSOCKET_LOST_FLAG);

      std::vector<char> data(RD_BUF_SIZE + 1);
      event.GetSocket()->Read(&data.front(), RD_BUF_SIZE);
      if (!event.GetSocket()->Error()) {
        size_t count = event.GetSocket()->LastCount();
        if (count) {
          if (1 /*FIXME !g_benableUDPNullHeader*/) {
            data[count] = 0;
            m_sock_buffer += (&data.front());
          } else {
            // XXX FIXME: is it reliable?
            // copy all received bytes
            // there's 0 in furuno UDP tags before NMEA sentences.
            m_sock_buffer.append(&data.front(), count);
          }
        }
      }

      bool done = false;

      while (!done) {
        int nmea_tail = 2;
        size_t nmea_end = m_sock_buffer.find_first_of(
            "*\r\n");  // detect the potential end of a NMEA string by finding
                       // the checkum marker or EOL

        if (nmea_end ==
            wxString::npos)  // No termination characters: continue reading
          break;

        if (m_sock_buffer[nmea_end] != '*') nmea_tail = -1;

        if (nmea_end < m_sock_buffer.size() - nmea_tail) {
          nmea_end +=
              nmea_tail +
              1;  // move to the char after the 2 checksum digits, if present
          if (nmea_end == 0)  // The first character in the buffer is a
                              // terminator, skip it to avoid infinite loop
            nmea_end = 1;
          std::string nmea_line = m_sock_buffer.substr(0, nmea_end);

          //  If, due to some logic error, the {nmea_end} parameter is larger
          //  than the length of the socket buffer, then std::string::substr()
          //  will throw an exception. We don't want that, so test for it. If
          //  found, the simple solution is to clear the socket buffer, and
          //  carry on This has been seen on high volume TCP feeds, Windows
          //  only. Hard to catch.....
          if (nmea_end > m_sock_buffer.size())
            m_sock_buffer.clear();
          else
            m_sock_buffer = m_sock_buffer.substr(nmea_end);

          size_t nmea_start = nmea_line.find_last_of(
              "$!");  // detect the potential start of a NMEA string, skipping
                      // preceding chars that may look like the start of a
                      // string.
          if (nmea_start != wxString::npos) {
            nmea_line = nmea_line.substr(nmea_start);
            nmea_line += "\r\n";  // Add cr/lf, possibly superfluous
            if (ChecksumOK(nmea_line)) {
              CommDriverN0183NetEvent Nevent(wxEVT_COMMDRIVER_N0183_NET, 0);
              if (nmea_line.size()) {
                //    Copy the message into a vector for tranmittal upstream
                auto buffer = std::make_shared<std::vector<unsigned char>>();
                std::vector<unsigned char>* vec = buffer.get();
                std::copy(nmea_line.begin(), nmea_line.end(),
                          std::back_inserter(*vec));

                Nevent.SetPayload(buffer);
                AddPendingEvent(Nevent);
              }
            }
          }
        } else
          done = true;
      }

      // Prevent non-nmea junk from consuming to much memory by limiting
      // carry-over buffer size.
      if (m_sock_buffer.size() > RD_BUF_SIZE)
        m_sock_buffer =
            m_sock_buffer.substr(m_sock_buffer.size() - RD_BUF_SIZE);

      m_dog_value = N_DOG_TIMEOUT;  // feed the dog
      break;
    }

    case wxSOCKET_LOST: {
      if (GetProtocol() == TCP || GetProtocol() == GPSD) {
        if (GetBrxConnectEvent())
          wxLogMessage(wxString::Format(
              _T("NetworkDataStream connection lost: %s"), GetPort().c_str()));
        if (GetSockServer()) {
          GetSock()->Destroy();
          SetSock(NULL);
          break;
        }
        wxDateTime now = wxDateTime::Now();
        wxTimeSpan since_connect(
            0, 0, 10);  // ten secs assumed, if connect time is uninitialized
        if (GetConnectTime().IsValid()) since_connect = now - GetConnectTime();

        int retry_time = 5000;  // default

        //  If the socket has never connected, and it is a short interval since
        //  the connect request then stretch the time a bit.  This happens on
        //  Windows if there is no dafault IP on any interface

        if (!GetBrxConnectEvent() && (since_connect.GetSeconds() < 5))
          retry_time = 10000;  // 10 secs

        GetSocketThreadWatchdogTimer()->Stop();
        GetSocketTimer()->Start(
            retry_time, wxTIMER_ONE_SHOT);  // Schedule a re-connect attempt
      }
      break;
    }

    case wxSOCKET_CONNECTION: {
      if (GetProtocol() == GPSD) {
        //      Sign up for watcher mode, Cooked NMEA
        //      Note that SIRF devices will be converted by gpsd into
        //      pseudo-NMEA
        char cmd[] = "?WATCH={\"class\":\"WATCH\", \"nmea\":true}";
        GetSock()->Write(cmd, strlen(cmd));
      } else if (GetProtocol() == TCP) {
        wxLogMessage(wxString::Format(
            _T("TCP NetworkDataStream connection established: %s"),
            GetPort().c_str()));
        m_dog_value = N_DOG_TIMEOUT;  // feed the dog
        if (GetPortType() != DS_TYPE_OUTPUT)
          GetSocketThreadWatchdogTimer()->Start(1000);
        if (GetPortType() != DS_TYPE_INPUT && GetSock()->IsOk())
          (void)SetOutputSocketOptions(GetSock());
        GetSocketTimer()->Stop();
        SetBrxConnectEvent(true);
      }

      SetConnectTime(wxDateTime::Now());
      break;
    }

    default:
      break;
  }
}

void CommDriverN0183Net::OnServerSocketEvent(wxSocketEvent& event) {
  switch (event.GetSocketEvent()) {
    case wxSOCKET_CONNECTION: {
      SetSock(GetSockServer()->Accept(false));

      if (GetSock()) {
        GetSock()->SetTimeout(2);
        //        GetSock()->SetFlags(wxSOCKET_BLOCK);
        GetSock()->SetEventHandler(*this, DS_SOCKET_ID);
        int notify_flags = (wxSOCKET_CONNECTION_FLAG | wxSOCKET_LOST_FLAG);
        if (GetPortType() != DS_TYPE_INPUT) {
          notify_flags |= wxSOCKET_OUTPUT_FLAG;
          (void)SetOutputSocketOptions(GetSock());
        }
        if (GetPortType() != DS_TYPE_OUTPUT)
          notify_flags |= wxSOCKET_INPUT_FLAG;
        GetSock()->SetNotify(notify_flags);
        GetSock()->Notify(true);
      }

      break;
    }

    default:
      break;
  }
}

bool CommDriverN0183Net::SendSentenceNetwork(const wxString& payload) {
  if (m_txenter)
    return false;  // do not allow recursion, could happen with non-blocking
                   // sockets
  m_txenter++;

  bool ret = true;
  wxDatagramSocket* udp_socket;
  switch (GetProtocol()) {
    case TCP:
      if (GetSock() && GetSock()->IsOk()) {
        GetSock()->Write(payload.mb_str(), strlen(payload.mb_str()));
        if (GetSock()->Error()) {
          if (GetSockServer()) {
            GetSock()->Destroy();
            SetSock(NULL);
          } else {
            wxSocketClient* tcp_socket =
                dynamic_cast<wxSocketClient*>(GetSock());
            if (tcp_socket) tcp_socket->Close();
            if (!GetSocketTimer()->IsRunning())
              GetSocketTimer()->Start(
                  5000, wxTIMER_ONE_SHOT);  // schedule a reconnect
            GetSocketThreadWatchdogTimer()->Stop();
          }
          ret = false;
        }

      } else
        ret = false;
      break;
    case UDP:
      udp_socket = dynamic_cast<wxDatagramSocket*>(GetTSock());
      if (udp_socket && udp_socket->IsOk()) {
        udp_socket->SendTo(GetAddr(), payload.mb_str(), payload.size());
        if (udp_socket->Error()) ret = false;
      } else
        ret = false;
      break;

    case GPSD:
    default:
      ret = false;
      break;
  }
  m_txenter--;
  return ret;
}

void CommDriverN0183Net::Close() {
  wxLogMessage(wxString::Format(_T("Closing NMEA NetworkDataStream %s"),
                                GetNetPort().c_str()));
  //    Kill off the TCP Socket if alive
  if (m_sock) {
    if (m_is_multicast)
      m_sock->SetOption(IPPROTO_IP, IP_DROP_MEMBERSHIP, &m_mrq, sizeof(m_mrq));
    m_sock->Notify(FALSE);
    m_sock->Destroy();
  }

  if (m_tsock) {
    m_tsock->Notify(FALSE);
    m_tsock->Destroy();
  }

  if (m_socket_server) {
    m_socket_server->Notify(FALSE);
    m_socket_server->Destroy();
  }

  m_socket_timer.Stop();
  m_socketread_watchdog_timer.Stop();
}

bool CommDriverN0183Net::SetOutputSocketOptions(wxSocketBase* tsock) {
  int ret;

  // Disable nagle algorithm on outgoing connection
  // Doing this here rather than after the accept() is
  // pointless  on platforms where TCP_NODELAY is
  // not inherited.  However, none of OpenCPN's currently
  // supported platforms fall into that category.

  int nagleDisable = 1;
  ret = tsock->SetOption(IPPROTO_TCP, TCP_NODELAY, &nagleDisable,
                         sizeof(nagleDisable));

  //  Drastically reduce the size of the socket output buffer
  //  so that when client goes away without properly closing, the stream will
  //  quickly fill the output buffer, and thus fail the write() call
  //  within a few seconds.
  unsigned long outbuf_size = 1024;  // Smallest allowable value on Linux
  return (tsock->SetOption(SOL_SOCKET, SO_SNDBUF, &outbuf_size,
                           sizeof(outbuf_size)) &&
          ret);
}

bool CommDriverN0183Net::ChecksumOK(const std::string& sentence) {
  if (!m_bchecksumCheck) return true;

  return CheckSumCheck(sentence);
}
