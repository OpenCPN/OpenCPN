/***************************************************************************
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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

/**
 * \file
 *
 * SignalK IP network driver
 */

// Originally by balp on 2018-07-28.

#ifndef SigNaLK_nEt_h_
#define SigNaLK_nEt_h_

#include <atomic>
#include <string>

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/datetime.h>
#include <wx/socket.h>

#include "rapidjson/fwd.h"
#include "model/conn_params.h"
#include "model/comm_drv_signalk.h"
#include "model/thread_ctrl.h"
#include "comm_drv_stats.h"

#define SIGNALK_SOCKET_ID 5011
#define N_DOG_TIMEOUT 5             // seconds
#define N_DOG_TIMEOUT_RECONNECT 10  // seconds

static const double kMsToKnotFactor = 1.9438444924406;

class WebSocketThread;            // Forward in .cpp file
class CommDriverSignalKNetEvent;  // Forward in .cpp file

class CommDriverSignalKNet : public CommDriverSignalK,
                             public wxEvtHandler,
                             public DriverStatsProvider {
public:
  CommDriverSignalKNet(const ConnectionParams* params, DriverListener& l);
  virtual ~CommDriverSignalKNet();

  static void initIXNetSystem();
  static void uninitIXNetSystem();

  static bool DiscoverSKServer(wxString& ip, int& port, int tSec);
  static bool DiscoverSKServer(std::string serviceIdent, wxString& ip,
                               int& port, int tSec);
  DriverStats GetDriverStats() const override;

private:
  ConnectionParams m_params;
  DriverListener& m_listener;
  std::string m_context;
  std::string m_self;
  wxIPV4address m_addr;
  int m_dog_value;
  wxTimer m_socketread_watchdog_timer;
  bool m_use_web_socket;
  bool m_gps_valid_sk;
  std::string m_token;
  WebSocketThread* m_ws_thread;
  StatsTimer m_stats_timer;
  DriverStats m_driver_stats;

  void Open();
  void Close();
  void OpenWebSocket();
  void CloseWebSocket();

  bool SetOutputSocketOptions(wxSocketBase* sock);

  wxIPV4address GetAddr() const { return m_addr; }

  wxTimer* GetSocketThreadWatchdogTimer() {
    return &m_socketread_watchdog_timer;
  }
  void HandleSkSentence(CommDriverSignalKNetEvent& event);

  void HandleUpdate(const rapidjson::Value& update);
  void UpdateItem(const rapidjson::Value& item, wxString& fixtime);

  void ResetWatchdog() { m_dog_value = N_DOG_TIMEOUT; }
  void SetWatchdog(int n) { m_dog_value = n; }
};

#endif  // SigNaLK_nEt_h_
