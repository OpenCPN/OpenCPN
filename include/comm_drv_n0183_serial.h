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

#ifndef _COMMDRIVERN0183SERIAL_H
#define _COMMDRIVERN0183SERIAL_H

#include <string>

#include <wx/event.h>

#include "comm_drv_n0183.h"
#include "conn_params.h"

class CommDriverN0183SerialThread;  // Internal

class CommDriverN0183SerialEvent;  // Internal

class CommDriverN0183Serial : public CommDriverN0183, public wxEvtHandler {
public:
  CommDriverN0183Serial(const ConnectionParams* params, DriverListener& l);

  virtual ~CommDriverN0183Serial();

  /** Register driver and possibly do other post-ctor steps. */
  void Activate() override;

  bool Open();
  void Close();

  //    Secondary thread life toggle
  //    Used to inform launching object (this) to determine if the thread can
  //    be safely called or polled, e.g. wxThread->Destroy();
  void SetSecThreadActive(void) { m_bsec_thread_active = true; }
  void SetSecThreadInActive(void) { m_bsec_thread_active = false; }
  bool IsSecThreadActive() const { return m_bsec_thread_active; }

  void SetSecondaryThread(CommDriverN0183SerialThread* secondary_Thread) {
    m_pSecondary_Thread = secondary_Thread;
  }
  CommDriverN0183SerialThread* GetSecondaryThread() {
    return m_pSecondary_Thread;
  }
  void SetThreadRunFlag(int run) { m_Thread_run_flag = run; }

  int m_Thread_run_flag;

  ConnectionParams GetParams() const { return m_params; }

  void SendMessage(std::shared_ptr<const NavMsg> msg,
                   std::shared_ptr<const NavAddr> addr) override;

private:
  bool m_bok;
  std::string m_portstring;
  std::string m_BaudRate;
  int m_handshake;

  CommDriverN0183SerialThread* m_pSecondary_Thread;
  bool m_bsec_thread_active;

  ConnectionParams m_params;
  DriverListener& m_listener;
  void handle_N0183_MSG(CommDriverN0183SerialEvent& event);
};

#endif  // guard
