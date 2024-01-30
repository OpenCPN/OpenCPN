/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  PlugIn Manager Object
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2022 by David S. Register                               *
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

#ifndef _SIGNALK_NET_H
#define _SIGNALK_NET_H

#include <atomic>
#include <string>

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // precompiled header

#include <wx/datetime.h>
#include <wx/socket.h>

#include "rapidjson/fwd.h"
#include "conn_params.h"
#include "comm_drv_signalk.h"

#define SIGNALK_SOCKET_ID 5011
#define N_DOG_TIMEOUT 5             // seconds
#define N_DOG_TIMEOUT_RECONNECT 10  // seconds

static const double ms_to_knot_factor = 1.9438444924406;

class WebSocketThread;
class OCPN_WebSocketMessageHandler;
class CommDriverSignalKNetEvent;

class CommDriverSignalKNet : public CommDriverSignalK, public wxEvtHandler {
public:
  CommDriverSignalKNet(const ConnectionParams *params, DriverListener& l);
  virtual ~CommDriverSignalKNet();

  void Open();
  void Close();
  static bool DiscoverSKServer(wxString &ip, int &port, int tSec);
  static bool DiscoverSKServer(std::string serviceIdent, wxString &ip,
                               int &port, int tSec);


  void SetThreadRunning(bool active) { m_threadActive = active; }
  void SetThreadRunFlag(int run) { m_Thread_run_flag = run; }
  void ResetWatchdog() { m_dog_value = N_DOG_TIMEOUT; }
  void SetWatchdog(int n) { m_dog_value = n; }

/** Register driver and possibly do other post-ctor steps. */
  void Activate() override;

  void handle_SK_sentence(CommDriverSignalKNetEvent& event);
  void handleUpdate(const rapidjson::Value &update);
  void updateItem(const rapidjson::Value &item, wxString &sfixtime);

  void OpenWebSocket();
  void CloseWebSocket();
  bool IsThreadRunning() { return m_threadActive == 1; }

  std::string m_self;
  std::string m_context;

  std::atomic_int m_Thread_run_flag;
  std::atomic_int m_threadActive;

  ConnectionParams m_params;
  DriverListener& m_listener;

  static void initIXNetSystem();

  static void uninitIXNetSystem();

private:
  wxIPV4address m_addr;
  wxIPV4address GetAddr() const { return m_addr; }

   int m_dog_value;

   wxTimer m_socketread_watchdog_timer;
   wxTimer *GetSocketThreadWatchdogTimer() {
     return &m_socketread_watchdog_timer;
   }

  OCPN_WebSocketMessageHandler *m_eventHandler;
  bool m_useWebSocket;

  bool m_bGPSValid_SK;

  bool SetOutputSocketOptions(wxSocketBase *sock);

  std::string m_token;

  WebSocketThread *m_wsThread;

};

#endif  // _SIGNALK_NET_H
