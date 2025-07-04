/**************************************************************************
 *   Copyright (C) 2022 David Register                                     *
 *   Copyright (C) 2022 Alec Leamas                                        *
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

/**
 *  \file
 *  Implement comm_drv_n0183_net.h.
 */

#ifdef __MSVC__
#include "winsock2.h"
#include <wx/msw/winundef.h>
#include <ws2tcpip.h>
#endif

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <ctime>
#include <deque>

#ifndef __WXMSW__
#include <arpa/inet.h>
#include <netinet/tcp.h>
#endif

#include <wx/datetime.h>
#include <wx/socket.h>
#include <wx/log.h>
#include <wx/memory.h>
#include <wx/chartype.h>
#include <wx/sckaddr.h>

#include "model/comm_drv_n0183_net.h"
#include "model/comm_drv_registry.h"
#include "model/comm_navmsg_bus.h"
#include "model/config_vars.h"
#include "model/garmin_protocol_mgr.h"
#include "model/idents.h"
#include "model/logger.h"
#include "model/sys_events.h"

#include "observable.h"

using namespace std::literals::chrono_literals;

#define N_DOG_TIMEOUT 8

/** Return true iff addr has all host bits set to 1. IPv4 only. */
static bool IsBroadcastAddr(unsigned addr, unsigned netmask_bits) {
  assert(netmask_bits <= 32);
  uint32_t netmask = 0xffffffff << (32 - netmask_bits);
  uint32_t host_mask = ~netmask;
  return (addr & host_mask) == host_mask;
}

class MrqContainer {
public:
  struct ip_mreq m_mrq;
  void SetMrqAddr(unsigned int addr) {
    m_mrq.imr_multiaddr.s_addr = addr;
    m_mrq.imr_interface.s_addr = INADDR_ANY;
  }
};

static bool SetOutputSocketOptions(wxSocketBase* tsock) {
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

//========================================================================
/*
 * CommDriverN0183Net implementation
 */
CommDriverN0183Net::CommDriverN0183Net(const ConnectionParams* params,
                                       DriverListener& listener)
    : CommDriverN0183(NavAddr::Bus::N0183, params->GetStrippedDSPort()),
      m_params(*params),
      m_listener(listener),
      m_sock(nullptr),
      m_tsock(nullptr),
      m_socket_server(nullptr),
      m_is_multicast(false),
      m_stats_timer(*this, 2s),
      m_txenter(0),
      m_dog_value(0),
      m_rx_connect_event(false),
      m_socket_timer(*this),
      m_socketread_watchdog_timer(*this),
      m_ok(false),
      m_is_conn_err_reported(false) {
  m_addr.Hostname(params->NetworkAddress);
  m_addr.Service(params->NetworkPort);
  this->attributes["netAddress"] = params->NetworkAddress.ToStdString();
  this->attributes["netPort"] = std::to_string(params->NetworkPort);
  this->attributes["userComment"] = params->UserComment.ToStdString();
  this->attributes["ioDirection"] = DsPortTypeToString(params->IOSelect);
  m_driver_stats.driver_bus = NavAddr::Bus::N0183;
  m_driver_stats.driver_iface = params->GetStrippedDSPort();

  m_mrq_container = std::make_unique<MrqContainer>();

  // Establish event listeners
  resume_listener.Init(SystemEvents::GetInstance().evt_resume,
                       [&](ObservedEvt&) { HandleResume(); });
  Bind(wxEVT_SOCKET, &CommDriverN0183Net::OnSocketEvent, this, DS_SOCKET_ID);
  Bind(wxEVT_SOCKET, &CommDriverN0183Net::OnServerSocketEvent, this,
       DS_SERVERSOCKET_ID);

  Open();
}

CommDriverN0183Net::~CommDriverN0183Net() { Close(); }

void CommDriverN0183Net::HandleN0183Msg(const std::string& sentence) {
  // Sanity check
  if ((sentence[0] == '$' || sentence[0] == '!') && sentence.size() > 5) {
    m_driver_stats.rx_count += sentence.size();
    std::string identifier;
    // We notify based on full message, including the Talker ID
    identifier = sentence.substr(1, 5);

    // notify message listener and also "ALL" N0183 messages, to support plugin
    // API using original talker id
    auto msg =
        std::make_shared<const Nmea0183Msg>(identifier, sentence, GetAddress());
    auto msg_all = std::make_shared<const Nmea0183Msg>(*msg, "ALL");

    if (m_params.SentencePassesFilter(sentence, FILTER_INPUT))
      m_listener.Notify(std::move(msg));
    m_listener.Notify(std::move(msg_all));
  }
}

void CommDriverN0183Net::Open() {
#ifdef __UNIX__
  in_addr_t addr =
      ((struct sockaddr_in*)m_addr.GetAddressData())->sin_addr.s_addr;
#else
  unsigned int addr = inet_addr(m_addr.IPAddress().mb_str());
#endif
  // Create the socket
  switch (m_params.NetProtocol) {
    case GPSD: {
      OpenNetworkGpsd();
      break;
    }
    case TCP: {
      OpenNetworkTcp(addr);
      break;
    }
    case UDP: {
      OpenNetworkUdp(addr);
      break;
    }
    default:
      break;
  }
  m_ok = true;
}

void CommDriverN0183Net::OpenNetworkUdp(unsigned int addr) {
  if (m_params.IOSelect != DS_TYPE_OUTPUT) {
    // We need a local (bindable) address to create the Datagram receive socket
    // Set up the reception socket
    wxIPV4address conn_addr;
    conn_addr.Service(std::to_string(m_params.NetworkPort));
    conn_addr.AnyAddress();
    conn_addr.AnyAddress();
    m_sock =
        new wxDatagramSocket(conn_addr, wxSOCKET_NOWAIT | wxSOCKET_REUSEADDR);

    // Test if address is IPv4 multicast
    if ((ntohl(addr) & 0xf0000000) == 0xe0000000) {
      m_is_multicast = true;
      m_mrq_container->SetMrqAddr(addr);
      m_sock->SetOption(IPPROTO_IP, IP_ADD_MEMBERSHIP, &m_mrq_container->m_mrq,
                        sizeof(m_mrq_container->m_mrq));
    }

    m_sock->SetEventHandler(*this, DS_SOCKET_ID);

    m_sock->SetNotify(wxSOCKET_CONNECTION_FLAG | wxSOCKET_INPUT_FLAG |
                      wxSOCKET_LOST_FLAG);
    m_sock->Notify(TRUE);
    m_sock->SetTimeout(1);  // Short timeout
    m_driver_stats.available = true;
  }

  // Set up another socket for transmit
  if (m_params.IOSelect != DS_TYPE_INPUT) {
    wxIPV4address tconn_addr;
    tconn_addr.Service(0);  // use ephemeral out port
    tconn_addr.AnyAddress();
    m_tsock =
        new wxDatagramSocket(tconn_addr, wxSOCKET_NOWAIT | wxSOCKET_REUSEADDR);
    // Here would be the place to disable multicast loopback
    // but for consistency with broadcast behaviour, we will
    // instead rely on setting priority levels to ignore
    // sentences read back that have just been transmitted
    if (!m_is_multicast && IsBroadcastAddr(addr, g_netmask_bits)) {
      int broadcastEnable = 1;
      m_tsock->SetOption(SOL_SOCKET, SO_BROADCAST, &broadcastEnable,
                         sizeof(broadcastEnable));
      m_driver_stats.available = true;
    }
  }

  // In case the connection is lost before acquired....
  m_connect_time = std::chrono::steady_clock::now();
}

void CommDriverN0183Net::OpenNetworkTcp(unsigned int addr) {
  if (addr == INADDR_ANY) {
    MESSAGE_LOG << "Listening for TCP connections on " << INADDR_ANY;
    m_socket_server = new wxSocketServer(m_addr, wxSOCKET_REUSEADDR);
    m_socket_server->SetEventHandler(*this, DS_SERVERSOCKET_ID);
    m_socket_server->SetNotify(wxSOCKET_CONNECTION_FLAG);
    m_socket_server->Notify(TRUE);
    m_socket_server->SetTimeout(1);  // Short timeout
  } else {
    MESSAGE_LOG << "Opening TCP connection to " << m_params.NetworkAddress
                << ":" << m_params.NetworkPort;
    m_sock = new wxSocketClient();
    m_sock->SetEventHandler(*this, DS_SOCKET_ID);
    int notify_flags = (wxSOCKET_CONNECTION_FLAG | wxSOCKET_LOST_FLAG);
    if (m_params.IOSelect != DS_TYPE_INPUT)
      notify_flags |= wxSOCKET_OUTPUT_FLAG;
    if (m_params.IOSelect != DS_TYPE_OUTPUT)
      notify_flags |= wxSOCKET_INPUT_FLAG;
    m_sock->SetNotify(notify_flags);
    m_sock->Notify(true);
    m_sock->SetTimeout(1);  // Short timeout

    m_rx_connect_event = false;
    m_socket_timer.Start(100, wxTIMER_ONE_SHOT);  // schedule a connection
  }

  // In case the connection is lost before acquired....
  m_connect_time = std::chrono::steady_clock::now();
}

void CommDriverN0183Net::OpenNetworkGpsd() {
  m_sock = new wxSocketClient();
  m_sock->SetEventHandler(*this, DS_SOCKET_ID);
  m_sock->SetNotify(wxSOCKET_CONNECTION_FLAG | wxSOCKET_INPUT_FLAG |
                    wxSOCKET_LOST_FLAG);
  m_sock->Notify(TRUE);
  m_sock->SetTimeout(1);  // Short timeout

  auto* tcp_socket = dynamic_cast<wxSocketClient*>(m_sock);
  tcp_socket->Connect(m_addr, false);
  m_rx_connect_event = false;
}

void CommDriverN0183Net::OnSocketReadWatchdogTimer() {
  m_dog_value--;

  if (m_dog_value <= 0) {  // No receive in n seconds
    if (GetParams().NoDataReconnect) {
      // Reconnect on NO DATA is true, so try to reconnect now.
      if (m_params.NetProtocol == TCP) {
        auto* tcp_socket = dynamic_cast<wxSocketClient*>(m_sock);
        if (tcp_socket) tcp_socket->Close();

        int n_reconnect_delay = wxMax(N_DOG_TIMEOUT - 2, 2);
        wxLogMessage("Reconnection scheduled in %d seconds.",
                     n_reconnect_delay);
        m_socket_timer.Start(n_reconnect_delay * 1000, wxTIMER_ONE_SHOT);

        //  Stop DATA watchdog, will be restarted on successful connection.
        m_socketread_watchdog_timer.Stop();
      }
    }
  }
}

void CommDriverN0183Net::OnTimerSocket() {
  //  Attempt a connection
  using namespace std::chrono;
  auto* tcp_socket = dynamic_cast<wxSocketClient*>(m_sock);
  if (tcp_socket) {
    if (tcp_socket->IsDisconnected()) {
      m_driver_stats.available = false;
      wxLogDebug("Attempting reconnection...");
      m_rx_connect_event = false;
      //  Stop DATA watchdog, may be restarted on successful connection.
      m_socketread_watchdog_timer.Stop();
      tcp_socket->Connect(m_addr, false);

      // schedule another connection attempt, in case this one fails
      int n_reconnect_delay = N_DOG_TIMEOUT;
      m_socket_timer.Start(n_reconnect_delay * 1000, wxTIMER_ONE_SHOT);

      // Possibly report connect error to GUI.
      if (m_connect_time == time_point<steady_clock>()) return;
      auto since_connect = steady_clock::now() - m_connect_time;
      if (since_connect > 10s && !m_is_conn_err_reported) {
        std::stringstream ss;
        ss << _("Cannot connect to remote server ") << m_params.NetworkAddress
           << ":" << m_params.NetworkPort;
        CommDriverRegistry::GetInstance().evt_driver_msg.Notify(ss.str());
        m_is_conn_err_reported = true;
        m_driver_stats.error_count++;
      }
    }
  }
}

void CommDriverN0183Net::HandleResume() {
  //  Attempt a stop and restart of connection
  auto* tcp_socket = dynamic_cast<wxSocketClient*>(m_sock);
  if (tcp_socket) {
    m_socketread_watchdog_timer.Stop();

    tcp_socket->Close();

    // schedule reconnect attempt
    int n_reconnect_delay = wxMax(N_DOG_TIMEOUT - 2, 2);
    wxLogMessage("Reconnection scheduled in %d seconds.", n_reconnect_delay);

    m_socket_timer.Start(n_reconnect_delay * 1000, wxTIMER_ONE_SHOT);
  }
}

bool CommDriverN0183Net::SendMessage(std::shared_ptr<const NavMsg> msg,
                                     std::shared_ptr<const NavAddr> addr) {
  auto msg_0183 = std::dynamic_pointer_cast<const Nmea0183Msg>(msg);
  std::string payload(msg_0183->payload);
  if (!ocpn::endswith(payload, "\r\n")) payload += "\r\n";
  m_driver_stats.tx_count += payload.size();
  return SendSentenceNetwork(payload.c_str());
}

void CommDriverN0183Net::OnSocketEvent(wxSocketEvent& event) {
#define RD_BUF_SIZE 4096
  // Allows handling of high volume data streams, such as a National AIS
  // stream with 100s of msgs a second.

  switch (event.GetSocketEvent()) {
    case wxSOCKET_INPUT:  // from gpsd Daemon
    {
      // TODO determine if the following SetFlags needs to be done at every
      // socket event or only once when socket is created, it it needs to be
      // done at all!
      // m_sock->SetFlags(wxSOCKET_WAITALL | wxSOCKET_BLOCK);      // was
      // (wxSOCKET_NOWAIT);

      // We use wxSOCKET_BLOCK to avoid Yield() reentrancy problems
      // if a long ProgressDialog is active, as in S57 SENC creation.

      //    Disable input event notifications to preclude re-entrancy on
      //    non-blocking socket
      //           m_sock->SetNotify(wxSOCKET_LOST_FLAG);
      uint8_t buff[RD_BUF_SIZE + 1];
      event.GetSocket()->Read(buff, RD_BUF_SIZE);
      if (!event.GetSocket()->Error()) {
        unsigned count = event.GetSocket()->LastCount();
        for (unsigned i = 0; i < count; i += 1) n0183_buffer.Put(buff[i]);
        while (n0183_buffer.HasSentence()) {
          HandleN0183Msg(n0183_buffer.GetSentence() + "\r\n");
        }
      }
      m_dog_value = N_DOG_TIMEOUT;  // feed the dog
      break;
    }

    case wxSOCKET_LOST: {
      m_driver_stats.available = false;
      using namespace std::chrono;
      if (m_params.NetProtocol == TCP || m_params.NetProtocol == GPSD) {
        if (m_rx_connect_event) {
          MESSAGE_LOG << "NetworkDataStream connection lost: "
                      << m_params.GetDSPort();
        }
        if (m_socket_server) {
          m_sock->Destroy();
          m_sock = nullptr;
          break;
        }
        auto since_connect = 10s;
        // ten secs assumed, if connect time is uninitialized
        auto now = steady_clock::now();
        if (m_connect_time != time_point<steady_clock>())
          since_connect = duration_cast<seconds>(now - m_connect_time);

        auto retry_time = 5s;  // default
        //  If the socket has never connected, and it is a short interval since
        //  the connect request then stretch the time a bit.  This happens on
        //  Windows if there is no default IP on any interface
        if (!m_rx_connect_event && (since_connect < 5s)) retry_time = 10s;

        m_socketread_watchdog_timer.Stop();

        // Schedule a re-connect attempt
        m_socket_timer.Start(duration_cast<milliseconds>(retry_time).count(),
                             wxTIMER_ONE_SHOT);
      }
      break;
    }

    case wxSOCKET_CONNECTION: {
      if (m_params.NetProtocol == GPSD) {
        //      Sign up for watcher mode, Cooked NMEA
        //      Note that SIRF devices will be converted by gpsd into
        //      pseudo-NMEA

        char cmd[] = R"--(?WATCH={"class":"WATCH", "nmea":true})--";
        m_sock->Write(cmd, strlen(cmd));
      } else if (m_params.NetProtocol == TCP) {
        MESSAGE_LOG << "TCP NetworkDataStream connection established: "
                    << m_params.GetDSPort();

        m_dog_value = N_DOG_TIMEOUT;  // feed the dog
        if (m_params.IOSelect != DS_TYPE_OUTPUT) {
          // start the DATA watchdog only if NODATA Reconnect is desired
          if (GetParams().NoDataReconnect)
            m_socketread_watchdog_timer.Start(1000);
        }

        if (m_params.IOSelect != DS_TYPE_INPUT && GetSock()->IsOk())
          (void)SetOutputSocketOptions(m_sock);
        m_socket_timer.Stop();
        m_rx_connect_event = true;
      }

      m_driver_stats.available = true;
      m_connect_time = std::chrono::steady_clock::now();
      break;
    }

    default:
      break;
  }
}

void CommDriverN0183Net::OnServerSocketEvent(wxSocketEvent& event) {
  switch (event.GetSocketEvent()) {
    case wxSOCKET_CONNECTION: {
      m_sock = m_socket_server->Accept(false);

      if (GetSock()) {
        m_sock->SetTimeout(2);
        //        GetSock()->SetFlags(wxSOCKET_BLOCK);
        m_sock->SetEventHandler(*this, DS_SOCKET_ID);
        int notify_flags = (wxSOCKET_CONNECTION_FLAG | wxSOCKET_LOST_FLAG);
        if (m_params.IOSelect != DS_TYPE_INPUT) {
          notify_flags |= wxSOCKET_OUTPUT_FLAG;
          (void)SetOutputSocketOptions(m_sock);
        }
        if (m_params.IOSelect != DS_TYPE_OUTPUT)
          notify_flags |= wxSOCKET_INPUT_FLAG;
        m_sock->SetNotify(notify_flags);
        m_sock->Notify(true);
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
  switch (m_params.NetProtocol) {
    case TCP:
      if (GetSock() && GetSock()->IsOk()) {
        m_sock->Write(payload.mb_str(), strlen(payload.mb_str()));
        if (GetSock()->Error()) {
          if (m_socket_server) {
            m_sock->Destroy();
            m_sock = nullptr;
          } else {
            auto* tcp_socket = dynamic_cast<wxSocketClient*>(m_sock);
            if (tcp_socket) tcp_socket->Close();
            if (!m_socket_timer.IsRunning())
              m_socket_timer.Start(5000, wxTIMER_ONE_SHOT);
            // schedule a reconnect
            m_socketread_watchdog_timer.Stop();
          }
          ret = false;
        }

      } else
        ret = false;
      m_driver_stats.available = ret;

      break;
    case UDP:
      udp_socket = dynamic_cast<wxDatagramSocket*>(m_tsock);
      if (udp_socket && udp_socket->IsOk()) {
        udp_socket->SendTo(m_addr, payload.mb_str(), payload.size());
        if (udp_socket->Error()) ret = false;
      } else {
        ret = false;
      }
      m_driver_stats.available = ret;
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
  MESSAGE_LOG << "Closing NMEA NetworkDataStream " << m_params.NetworkPort;
  m_stats_timer.Stop();
  //    Kill off the TCP Socket if alive
  if (m_sock) {
    if (m_is_multicast)
      m_sock->SetOption(IPPROTO_IP, IP_DROP_MEMBERSHIP, &m_mrq_container->m_mrq,
                        sizeof(m_mrq_container->m_mrq));
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
  m_driver_stats.available = false;
}
