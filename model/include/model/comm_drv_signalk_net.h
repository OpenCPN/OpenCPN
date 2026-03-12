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

#include <string>

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/socket.h>

#include "ixwebsocket/IXWebSocket.h"

#include "model/conn_params.h"
#include "model/comm_drv_signalk.h"
#include "model/thread_ctrl.h"
#include "comm_drv_stats.h"

constexpr int kSignalkSocketId = 5011;
constexpr int kDogTimeoutSeconds = 5;
constexpr int kDogTimeoutReconnectSeconds = 10;

constexpr double kMsToKnotFactor = 1.9438444924406;

class CommDrvSignalkNetEvt;  // Forward in .cpp file

class WebSocketThread : public ThreadCtrl {
public:
  WebSocketThread(const std::string& iface, const wxIPV4address& address,
                  wxEvtHandler* consumer, const std::string& token);

  ~WebSocketThread() override = default;

  void* Run();

  DriverStats GetStats() const;

private:
  wxIPV4address m_address;
  wxEvtHandler* m_consumer;
  const std::string m_iface;
  std::string m_token;
  ix::WebSocket m_ws;
  ObsListener m_resume_listener;
  DriverStats m_driver_stats;
  mutable std::mutex m_stats_mutex;
};

class CommDriverSignalKNet : public CommDriverSignalK,
                             public wxEvtHandler,
                             public DriverStatsProvider {
public:
  CommDriverSignalKNet(const ConnectionParams* params, DriverListener& l);
  ~CommDriverSignalKNet() override;

  static void initIXNetSystem();
  static void uninitIXNetSystem();

  static bool DiscoverSKServer(const std::string& serviceIdent, wxString& ip,
                               int& port, int tSec);

  DriverStats GetDriverStats() const override;

private:
  ConnectionParams m_params;
  DriverListener& m_listener;
  std::string m_context;
  std::string m_self;
  int m_dog_value;
  wxTimer m_socketread_watchdog_timer;
  std::string m_token;
  std::thread m_std_thread;
  WebSocketThread m_ws_thread;
  StatsTimer m_stats_timer;
  DriverStats m_driver_stats;

  void Open();
  void Close();
  void OpenWebSocket();
  void CloseWebSocket();

  wxTimer* GetSocketThreadWatchdogTimer() {
    return &m_socketread_watchdog_timer;
  }
  void HandleSkSentence(const CommDrvSignalkNetEvt& event);

  void ResetWatchdog() { m_dog_value = kDogTimeoutSeconds; }
  void SetWatchdog(int n) { m_dog_value = n; }
};

#endif  // SigNaLK_nEt_h_
