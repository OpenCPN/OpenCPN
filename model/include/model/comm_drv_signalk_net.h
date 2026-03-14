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

#include <memory>
#include <string>
#include <thread>

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/event.h>
#include <wx/string.h>
#include <wx/timer.h>

#include "model/conn_params.h"
#include "model/comm_drv_signalk.h"
#include "comm_drv_stats.h"

constexpr int kDogTimeoutSeconds = 5;

class CommDriverSignalKNet : public CommDriverSignalK,
                             public wxEvtHandler,
                             public DriverStatsProvider {
public:
  CommDriverSignalKNet(const ConnectionParams* params, DriverListener& l);
  ~CommDriverSignalKNet() override;

  /** \internal */
  class InputEvt;

  DriverStats GetDriverStats() const override;

  /** ix::initIXNetSystem wrapper */
  static void initIXNetSystem();

  /** ix::uninitIXNetSystem wrapper */
  static void uninitIXNetSystem();

  /**
   * Scan for a SignalK server on local network using mDNS.
   *
   * @param service_ident   mDNS service to scan for
   * @param ip On successful return found server IP address
   * @param port On successful return found server IP port.
   * @param tSec Scan timeout (seconds).
   * @return true if a server is found, else false.
   */
  static bool DiscoverSkServer(const std::string& service_ident, wxString& ip,
                               int& port, int tSec);

private:
  class IoThread;

  ConnectionParams m_params;
  DriverListener& m_listener;
  int m_dog_value;
  wxTimer m_socketread_watchdog_timer;
  std::thread m_std_thread;
  std::unique_ptr<IoThread> m_io_thread;
  StatsTimer m_stats_timer;
  DriverStats m_driver_stats;
  std::string m_context;
  std::string m_self;

  void Open();
  void Close();
  void OpenWebSocket();
  void CloseWebSocket();

  void HandleSkSentence(const InputEvt& event);

  void ResetWatchdog() { m_dog_value = kDogTimeoutSeconds; }
  void SetWatchdog(int n) { m_dog_value = n; }
};

#endif  // SigNaLK_nEt_h_
