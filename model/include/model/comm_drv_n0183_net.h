/**************************************************************************
 *   Copyright (C) 2022  David Register                                    *
 *   Copyright (C) 2022  Alec Leamas                                       *
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
 * \file
 * NMEA0183 over IP driver
 */

#ifndef COMMDRIVERN0183NET_H_
#define COMMDRIVERN0183NET_H_

#include <chrono>
#include <memory>
#include <string>

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/string.h>
#include <wx/timer.h>

#ifdef __WXGTK__
// newer versions of glib define its own GSocket, but we unfortunately use this
// name in our own (semi-)public header and so can't change it -- rename glib
// one instead
// #include <gtk/gtk.h>
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

#include "model/comm_buffers.h"
#include "model/comm_drv_n0183.h"
#include "model/comm_drv_stats.h"
#include "model/conn_params.h"
#include "model/ocpn_utils.h"
#include "observable.h"

class MrqContainer;

class CommDriverN0183Net : public CommDriverN0183,
                           public wxEvtHandler,
                           public DriverStatsProvider {
public:
  CommDriverN0183Net(const ConnectionParams* params, DriverListener& listener);

  ~CommDriverN0183Net() override;

  const ConnectionParams& GetParams() const override { return m_params; }

  const wxSocketBase* GetSock() const { return m_sock; }

  bool SendMessage(std::shared_ptr<const NavMsg> msg,
                   std::shared_ptr<const NavAddr> addr) override;

  DriverStats GetDriverStats() const override { return m_driver_stats; }

private:
  class SocketTimer : public wxTimer {
  public:
    SocketTimer(CommDriverN0183Net& owner) : wxTimer(), m_owner(owner) {}
    void Notify() override { m_owner.OnTimerSocket(); }

  private:
    CommDriverN0183Net& m_owner;
  };

  class SocketReadWatchdogTimer : public wxTimer {
  public:
    SocketReadWatchdogTimer(CommDriverN0183Net& owner)
        : wxTimer(), m_owner(owner) {}
    void Notify() override { m_owner.OnSocketReadWatchdogTimer(); }

  private:
    CommDriverN0183Net& m_owner;
  };

  void Open();
  void Close();
  void HandleResume();
  void OnServerSocketEvent(wxSocketEvent& event);  // The listener
  void OnTimerSocket();
  void OnSocketEvent(wxSocketEvent& event);
  void OnSocketReadWatchdogTimer();
  void OpenNetworkGpsd();
  void OpenNetworkTcp(unsigned int addr);
  void OpenNetworkUdp(unsigned int addr);
  void HandleN0183Msg(const std::string& sentence);
  bool SendSentenceNetwork(const wxString& payload);

  const ConnectionParams m_params;
  DriverListener& m_listener;
  N0183Buffer n0183_buffer;
  wxIPV4address m_addr;
  wxSocketBase* m_sock;
  wxSocketBase* m_tsock;
  wxSocketServer* m_socket_server;
  bool m_is_multicast;
  std::unique_ptr<MrqContainer> m_mrq_container;
  StatsTimer m_stats_timer;
  DriverStats m_driver_stats;

  int m_txenter;
  int m_dog_value;
  std::chrono::time_point<std::chrono::steady_clock> m_connect_time;
  bool m_rx_connect_event;

  SocketTimer m_socket_timer;
  SocketReadWatchdogTimer m_socketread_watchdog_timer;

  bool m_ok;
  bool m_is_conn_err_reported;

  ObsListener resume_listener;
};

#endif  // COMMDRIVERN0183NET_H_
