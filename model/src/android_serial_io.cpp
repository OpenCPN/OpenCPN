/**************************************************************************
 *   Copyright (C) 2024  Alec Leamas                                       *
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
 * Android SerialIo synchronous implementation based on the native
 * Android serial interface.
 */

#include <string>

// For compilers that support precompilation, includes "wx.h".
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/string.h>

#include "androidUTIL.h"
#include "model/serial_io.h"
#include "model/comm_drv_stats.h"

/**
 * Android SerialIo synchronous implementation based on the native Android
 * serial interface as exposed in androidUTIL.h
 */
class AndroidSerialIo : public SerialIo {
public:
  AndroidSerialIo(SendMsgFunc send_func, const std::string& port, unsigned baud)
      : SerialIo(send_func, port, baud) {
    m_driver_stats.driver_bus = NavAddr::Bus::N0183;
    m_driver_stats.driver_iface = "android";
  }

  DriverStats GetStats() const { return m_driver_stats; }
  // FIXME (leamas) rx...

  bool SetOutMsg(const wxString& msg) override {
    if (msg.size() < 6 || (msg[0] != '$' && msg[0] != '!')) return false;
    wxString payload = msg;
    if (!msg.EndsWith("\r\n")) payload += "\r\n";
    wxString port(m_portname);  // Horrors due to missing const in library
    androidWriteSerial(port, payload);
    m_driver_stats.tx_count += m_portname.size();
    return true;
  }

  void Start() override {
    if (m_portname.empty()) return;
    wxString port(m_portname);  // Horrors due to missing const in library
    androidStartUSBSerial(port, std::to_string(m_baud), m_send_msg_func);
    m_driver_stats.available = true;
  }

  void RequestStop() override {
    wxString port(m_portname);  // Horrors due to missing const in library
    androidStopUSBSerial(port);
    ThreadCtrl::RequestStop();
    ThreadCtrl::SignalExit();  // No need to wait for exiting thread doing this
    m_driver_stats.available = false;
  }

private:
  DriverStats m_driver_stats;
};

std::unique_ptr<SerialIo> SerialIo::Create(SendMsgFunc send_msg_func,
                                           const std::string& port,
                                           unsigned baud) {
  return std::make_unique<AndroidSerialIo>(send_msg_func, port, baud);
}
