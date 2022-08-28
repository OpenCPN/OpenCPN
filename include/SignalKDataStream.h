/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  PlugIn Manager Object
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
// Originally by balp on 2018-07-28.

#ifndef OPENCPN_SIGNALKDATASTREAM_H
#define OPENCPN_SIGNALKDATASTREAM_H

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // precompiled header

#include <wx/datetime.h>

#ifdef __WXGTK__
// newer versions of glib define its own GSocket but we unfortunately use this
// name in our own (semi-)public header and so can't change it -- rename glib
// one instead
//#include <gtk/gtk.h>
#define GSocket GlibGSocket
#include "wx/socket.h"
#undef GSocket
#else
#include "wx/socket.h"
#endif

#ifndef __WXMSW__
#include <sys/socket.h>  // needed for (some) Mac builds
#include <netinet/in.h>
#endif

#ifdef __WXMSW__
#include <windows.h>
#include <dbt.h>
#include <initguid.h>
#endif
#include <string>
#include "conn_params.h"
#include "dsPortType.h"
#include "datastream.h"

#define SIGNALK_SOCKET_ID 5011
#define N_DOG_TIMEOUT 5             // seconds
#define N_DOG_TIMEOUT_RECONNECT 10  // seconds

class WebSocketThread;
class OCPN_WebSocketMessageHandler;

class SignalKDataStream : public DataStream {
public:
  SignalKDataStream(wxEvtHandler *input_consumer,
                    const ConnectionParams *params);
  virtual ~SignalKDataStream();

  void Close();
  static bool DiscoverSKServer(wxString &ip, int &port, int tSec);
  static bool DiscoverSKServer(std::string serviceIdent, wxString &ip,
                               int &port, int tSec);

  void SetThreadRunning(bool active) { m_threadActive = active; }
  void ResetWatchdog() { m_dog_value = N_DOG_TIMEOUT; }
  void SetWatchdog(int n) { m_dog_value = n; }

private:
  void Open();
  void OpenTCPSocket();
  void OpenWebSocket();
  void CloseWebSocket();
  bool IsThreadRunning() { return m_threadActive; }

  const ConnectionParams *m_params;
  wxSocketBase *m_sock;
  void SetSock(wxSocketBase *sock) { m_sock = sock; }
  wxSocketBase *GetSock() const { return m_sock; }

  wxIPV4address m_addr;
  wxIPV4address GetAddr() const { return m_addr; }

  bool m_brx_connect_event;
  void SetBrxConnectEvent(bool event) { m_brx_connect_event = event; }
  bool GetBrxConnectEvent() { return m_brx_connect_event; }

  int m_dog_value;
  wxTimer m_socket_timer;
  wxTimer *GetSocketTimer() { return &m_socket_timer; }

  wxTimer m_socketread_watchdog_timer;
  wxTimer *GetSocketThreadWatchdogTimer() {
    return &m_socketread_watchdog_timer;
  }

  OCPN_WebSocketMessageHandler *m_eventHandler;
  bool m_useWebSocket;
  bool m_threadActive;

  NetworkProtocol GetProtocol() { return m_params->NetProtocol; }
  std::string m_sock_buffer;

  void OnTimerSocket(wxTimerEvent &event);
  void OnSocketEvent(wxSocketEvent &event);
  void OnSocketReadWatchdogTimer(wxTimerEvent &event);

  bool SetOutputSocketOptions(wxSocketBase *sock);

  WebSocketThread *m_wsThread;
  DECLARE_EVENT_TABLE()
};

#endif  // OPENCPN_SIGNALKDATASTREAM_H
