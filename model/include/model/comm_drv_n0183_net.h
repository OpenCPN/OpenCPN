/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:
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

#ifndef _COMMDRIVERN0183NET_H
#define _COMMDRIVERN0183NET_H

#include <memory>
#include <string>

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // precompiled header

#include <wx/datetime.h>
#include <wx/string.h>
#include <wx/timer.h>

#ifdef __WXGTK__
// newer versions of glib define its own GSocket but we unfortunately use this
// name in our own (semi-)public header and so can't change it -- rename glib
// one instead
//#include <gtk/gtk.h>
#define GSocket GlibGSocket
#include <wx/socket.h>
#undef GSocket
#else
#include <wx/socket.h>
#endif

#ifndef __WXMSW__
#include <sys/socket.h>  // needed for (some) Mac builds
#include <netinet/in.h>
#endif

#include "model/comm_drv_n0183.h"
#include "model/comm_out_queue.h"
#include "model/conn_params.h"
#include "observable.h"

class CommDriverN0183NetEvent;  // Internal
class MrqContainer;

class CommDriverN0183Net : public CommDriverN0183, public wxEvtHandler {
public:
  CommDriverN0183Net();
  CommDriverN0183Net(const ConnectionParams* params, DriverListener& listener);

  virtual ~CommDriverN0183Net();

  void Open();
  void Close();

  void DumpStats() const override;

  ConnectionParams GetParams() const { return m_params; }

  bool SetOutputSocketOptions(wxSocketBase* tsock);
  bool SendSentenceNetwork(const wxString& payload);
  void OnServerSocketEvent(wxSocketEvent& event);  // The listener
  void OnTimerSocket(wxTimerEvent& event) { OnTimerSocket(); }
  void OnTimerSocket();
  void OnSocketEvent(wxSocketEvent& event);
  void OpenNetworkGPSD();
  void OpenNetworkTCP(unsigned int addr);
  void OpenNetworkUDP(unsigned int addr);
  void OnSocketReadWatchdogTimer(wxTimerEvent& event);
  void HandleResume();

  bool SendMessage(std::shared_ptr<const NavMsg> msg,
                   std::shared_ptr<const NavAddr> addr) override;
  wxSocketBase* GetSock() const { return m_sock; }

private:
  ConnectionParams m_params;
  DriverListener& m_listener;

  void handle_N0183_MSG(CommDriverN0183NetEvent& event);
  wxString GetNetPort() const { return m_net_port; }
  wxIPV4address GetAddr() const { return m_addr; }
  wxTimer* GetSocketThreadWatchdogTimer() {
    return &m_socketread_watchdog_timer;
  }
  wxTimer* GetSocketTimer() { return &m_socket_timer; }
  void SetSock(wxSocketBase* sock) { m_sock = sock; }
  void SetTSock(wxSocketBase* sock) { m_tsock = sock; }
  wxSocketBase* GetTSock() const { return m_tsock; }
  void SetSockServer(wxSocketServer* sock) { m_socket_server = sock; }
  wxSocketServer* GetSockServer() const { return m_socket_server; }
  void SetMulticast(bool multicast) { m_is_multicast = multicast; }
  bool GetMulticast() const { return m_is_multicast; }

  NetworkProtocol GetProtocol() { return m_net_protocol; }
  void SetBrxConnectEvent(bool event) { m_brx_connect_event = event; }
  bool GetBrxConnectEvent() { return m_brx_connect_event; }

  void SetConnectTime(wxDateTime time) { m_connect_time = time; }
  wxDateTime GetConnectTime() { return m_connect_time; }

  dsPortType GetPortType() const { return m_io_select; }
  wxString GetPort() const { return m_portstring; }

  ConnectionType GetConnectionType() const { return m_connection_type; }

  bool ChecksumOK(const std::string& sentence);
  void SetOk(bool ok) { m_bok = ok; };

  wxString m_net_port;
  NetworkProtocol m_net_protocol;
  wxIPV4address m_addr;
  wxSocketBase* m_sock;
  wxSocketBase* m_tsock;
  wxSocketServer* m_socket_server;
  bool m_is_multicast;
  MrqContainer  *m_mrq_container;

  int m_txenter;
  int m_dog_value;
  std::string m_sock_buffer;
  wxString m_portstring;
  dsPortType m_io_select;
  wxDateTime m_connect_time;
  bool m_brx_connect_event;
  bool m_bchecksumCheck;
  ConnectionType m_connection_type;

  wxTimer m_socket_timer;
  wxTimer m_socketread_watchdog_timer;

  bool m_bok;

  ObsListener resume_listener;
  const std::unique_ptr<CommOutQueue> m_out_queue;

  DECLARE_EVENT_TABLE()
};

#endif  // guard
