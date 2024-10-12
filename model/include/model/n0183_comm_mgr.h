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
#include <memory>
#include <functional>

#include <wx/string.h>

#include "comm_buffers.h"
#include "model/thread_ctrl.h"

#ifndef __ANDROID__
#include "serial/serial.h"
#endif
using SendMsgFunc = std::function<void(const std::vector<unsigned char>&)>;
class CommDriverN0183Serial;

/** Nmea0183 serial IO thread. */
class CommDriverN0183SerialThread : public ThreadCtrl {
public:
  CommDriverN0183SerialThread(SendMsgFunc send_msg_func);

  ~CommDriverN0183SerialThread();

  void SetParams(const wxString& portname, const wxString& str_baudrate);

  /** Thread main function. */
  void* Entry();

  /** Start IO operations, possibly in separate thread. */
  void Start();

  /** Send a message to remote peer. */
  bool SetOutMsg(const wxString& msg);

  void RequestStop() override;

private:
#ifndef __ANDROID__
  serial::Serial m_serial;
#endif
  void ThreadMessage(const wxString& msg);
  bool OpenComPortPhysical(const wxString& com_name, int baud_rate);
  void CloseComPortPhysical();
  ssize_t WriteComPortPhysical(const char* msg);

  wxString m_portname;
  int m_baud;
  OutputBuffer m_out_que;
  SendMsgFunc m_send_msg_func;
};

#endif  //    _N0183_PROTOL_MGR__
