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

/** \file serial_io.h Abstract N0183 serial communications interface */

#ifndef _N0183_PROTOL_MGR__
#define _N0183_PROTOL_MGR__

#include "config.h"

#include <atomic>
#include <memory>
#include <functional>

#include <wx/string.h>

#include "comm_buffers.h"
#include "model/thread_ctrl.h"
#include "model/ocpn_utils.h"

/** Remove possible Serial: prefix. */
static std::string NormalizePort(const std::string& port) {
  return port.find("Serial:") == 0 ? port.substr(strlen("Serial:")) : port;
}

/** Forwards data from driver to local receivers (main code, plugins, etc). */
using SendMsgFunc = std::function<void(const std::vector<unsigned char>&)>;

/** Nmea0183 serial communications wrapper, possibly running a thread */
class SerialIo : public ThreadCtrl {
public:
  /** Factory */
  static std::unique_ptr<SerialIo> Create(SendMsgFunc send_msg_func,
                                          const std::string& port,
                                          unsigned baud);

  virtual ~SerialIo() = default;

  /** Start IO operations including input, possibly in separate thread. */
  virtual void Start() = 0;

  /** Send a message to remote peer. */
  virtual bool SetOutMsg(const wxString& msg) = 0;

  virtual void RequestStop() override = 0;

protected:
  const wxString m_portname;
  const unsigned m_baud;
  const SendMsgFunc m_send_msg_func;

  SerialIo(SendMsgFunc send_msg_func, const std::string& port, unsigned baud)
      : m_portname(NormalizePort(port)),
        m_baud(baud),
        m_send_msg_func(send_msg_func) {}
};

#ifdef __clang__
// As of 12.0.5, clang does not emit the SerialIo vtable and typeinfo linker
// addresses without this hack (gcc does). Net result without hack is
// undefined symbols like "typeinfo for SerialIo". Possible clang bug?

__attribute__((used)) static SerialIo* force_clang_to_emit_typeinfo;
#endif

#endif  //    _N0183_PROTOL_MGR__
