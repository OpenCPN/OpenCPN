/**************************************************************************
 *   Copyright (C) 2022 by David Register                                  *
 *   Copyright (C) 2022 by Alec Leamas                                     *
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

/** \file n0183_comm_mgr.h N0183 serial communications IO thread */

#ifndef _N0183_PROTOL_MGR__
#define _N0183_PROTOL_MGR__

#include "config.h"

#include <atomic>

#include <wx/string.h>

#include "comm_buffers.h"

#ifndef __ANDROID__
#include "serial/serial.h"
#endif

class CommDriverN0183Serial;

/** Nmea0183 serial IO thread. */
class CommDriverN0183SerialThread {
public:
  CommDriverN0183SerialThread(CommDriverN0183Serial* Launcher,
                              const wxString& PortName,
                              const wxString& strBaudRate);

  ~CommDriverN0183SerialThread(void);
  void* Entry();
  bool SetOutMsg(const wxString& msg);
  void OnExit(void);

  /** Unset thread "keep going" flag i. e., initiate stop sequence. */
  void Stop() { keep_going = 0; }

  /** Return true if/when thread has stopped. */
  bool IsStopped() const { return keep_going < 0; }

private:
#ifndef __ANDROID__
  serial::Serial m_serial;
#endif
  std::atomic_int keep_going;
  void ThreadMessage(const wxString& msg);
  bool OpenComPortPhysical(const wxString& com_name, int baud_rate);
  void CloseComPortPhysical();
  ssize_t WriteComPortPhysical(const char* msg);

  CommDriverN0183Serial* m_pParentDriver;
  wxString m_PortName;
  wxString m_FullPortName;

  int m_baud;

  OutputBuffer m_out_que;
};

#endif  //    _N0183_PROTOL_MGR__
