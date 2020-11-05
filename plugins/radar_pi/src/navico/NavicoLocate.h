/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Radar Plugin
 * Author:   David Register
 *           Dave Cowell
 *           Kees Verruijt
 *           Douwe Fokkema
 *           Sean D'Epagnier
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register              bdbcat@yahoo.com *
 *   Copyright (C) 2012-2013 by Dave Cowell                                *
 *   Copyright (C) 2012-2016 by Kees Verruijt         canboat@verruijt.net *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************
 */

#ifndef _NAVICOLOCATE_H_
#define _NAVICOLOCATE_H_

#include <map>

#include "NavicoCommon.h"
#include "radar_pi.h"
#include "socketutil.h"

PLUGIN_BEGIN_NAMESPACE

//
// Listens for (possibly unknown) Navico radars and known ones.
// A single instance of this class will exist, and run a thread, if one or more
// Navico radars of 4G or newer is selected.
//
// It will fill a map that given a radar IP address will give its listening ports.
// The individual radars will then listen to multicast data on those ports.
//

class NavicoLocate : public wxThread {
#define MAX_REPORT 10
 public:
  NavicoLocate(radar_pi *pi) : wxThread(wxTHREAD_JOINABLE) {
    Create(64 * 1024);  // Stack size
    m_pi = pi;          // This allows you to access the main plugin stuff
    m_shutdown = false;
    m_is_shutdown = true;

    m_interface_addr = 0;
    m_socket = 0;
    m_interface_count = 0;
    m_report_count = 0;

    LOG_INFO(wxT("radar_pi: NavicoLocate thread created, prio= %i"), GetPriority());
  }

  /*
   * Shutdown
   *
   * Called when the thread should stop.
   * It should stop running.
   */
  void Shutdown(void) { m_shutdown = true; }

  ~NavicoLocate() {
    while (!m_is_shutdown) {
      wxMilliSleep(50);
    }
  }

  volatile bool m_is_shutdown;

 protected:
  void *Entry(void);

 private:
  bool ProcessReport(const NetworkAddress &radar_address, const NetworkAddress &interface_address, const uint8_t *data, size_t len);
  bool DetectedRadar(const NetworkAddress &radar_address);
  void WakeRadar();

  void UpdateEthernetCards();
  void CleanupCards();

  radar_pi *m_pi;
  volatile bool m_shutdown;

  // Three arrays, all created on each call to UpdateEthernetCards.
  // One entry for each ethernet card.
  NetworkAddress *m_interface_addr;
  SOCKET *m_socket;
  size_t m_interface_count;
  size_t m_report_count;

  wxCriticalSection m_exclusive;
};

PLUGIN_END_NAMESPACE

#endif /* _NAVICORECEIVE_H_ */
