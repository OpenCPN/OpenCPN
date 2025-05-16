/**************************************************************************
 *   Copyright (C) 2022 by David Register                                  *
 *   Copyright (C) 2022-2024  Alec Leamas                                  *
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
 * SerialIo asynchronous implementation based on the serial/serial.h header.
 * Used on all platforms besides Android.
 */

#include <memory>
#include <string>

// For compilers that support precompilation, includes "wx.h".
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <mutex>  // std::mutex
#include <queue>  // std::queue
#include <thread>
#include <vector>

#include <wx/log.h>
#include <wx/string.h>
#include <wx/utils.h>

#include "model/comm_buffers.h"
#include "model/comm_drv_registry.h"
#include "model/logger.h"
#include "model/serial_io.h"
#include "serial/serial.h"

/** SerialIo implementation based on serial/serial.h.*/
class StdSerialIo : public SerialIo {
public:
  StdSerialIo(SendMsgFunc send_func, const std::string& port, unsigned baud)
      : SerialIo(send_func, port, baud) {}

  bool SetOutMsg(const wxString& msg) override;
  void Start() override;
  DriverStats GetStats() const override;

private:
  serial::Serial m_serial;
  OutputBuffer m_out_que;
  void* Entry();
  void Reconnect();

  bool OpenComPortPhysical(const wxString& com_name, unsigned baud_rate);
  void CloseComPortPhysical();
  ssize_t WriteComPortPhysical(const char* msg);
  void RequestStop() override { ThreadCtrl::RequestStop(); }
};

std::unique_ptr<SerialIo> SerialIo::Create(SendMsgFunc send_msg_func,
                                           const std::string& port,
                                           unsigned baud) {
  return std::make_unique<StdSerialIo>(send_msg_func, port, baud);
}

void* StdSerialIo::Entry() {
  LineBuffer line_buf;

  //    Request the com port from the comm manager
  if (!OpenComPortPhysical(m_portname, m_baud)) {
    std::string msg(_("NMEA input device open failed: "));
    msg += m_portname;
    CommDriverRegistry::GetInstance().evt_driver_msg.Notify(msg);
  }

  //    The main loop
  unsigned retries = 0;
  m_stats.driver_bus = NavAddr::Bus::N0183;
  m_stats.driver_iface = m_portname.ToStdString();
  while (KeepGoing()) {
    unsigned newdata = 0;
    uint8_t rdata[2000];

    if (m_serial.isOpen()) {
      try {
        newdata = m_serial.read(rdata, sizeof(rdata));
      } catch (std::exception& e) {
        DEBUG_LOG << "Serial read exception: " << e.what();
        if (10 < retries++) {
          // We timed out waiting for the next character 10 times, let's close
          // the port so that the reconnection logic kicks in and tries to fix
          // our connection.
          CloseComPortPhysical();
          retries = 0;
        }
      }
    } else {
      // Reconnection logic. Let's try to reopen the port while waiting longer
      // every time (until we simply keep trying every 2.5 seconds)
      // std::cerr << "Serial port seems closed." << std::endl;
      wxMilliSleep(250 * retries);
      CloseComPortPhysical();
      if (OpenComPortPhysical(m_portname, m_baud))
        retries = 0;
      else if (retries < 10)
        retries++;
    }

    // Handle received data
    for (unsigned i = 0; i < newdata; i++) line_buf.Put(rdata[i]);
    while (KeepGoing() && line_buf.HasLine()) {
      auto line = line_buf.GetLine();
      {
        std::lock_guard lock(m_stats_mutex);
        m_stats.rx_count += line.size();
      }
      m_send_msg_func(line);
    }

    //  Handle pending output messages
    std::string qmsg;
    while (KeepGoing() && m_out_que.Get(qmsg)) {
      qmsg += "\r\n";
      bool failed_write = WriteComPortPhysical(qmsg.c_str()) == -1;
      if (!failed_write) {
        std::lock_guard lock(m_stats_mutex);
        m_stats.tx_count += qmsg.size();
      }
      if (failed_write && 10 < retries++) {
        // We failed to write the port 10 times, let's close the port so that
        // the reconnection logic kicks in and tries to fix our connection.
        retries = 0;
        CloseComPortPhysical();
      }
    }
  }
  CloseComPortPhysical();
  SignalExit();
  return nullptr;
}

void StdSerialIo::Start() {
  std::thread t([&] { Entry(); });
  t.detach();
}

bool StdSerialIo::SetOutMsg(const wxString& msg) {
  if (msg.size() < 6 || (msg[0] != '$' && msg[0] != '!')) return false;
  m_out_que.Put(msg.ToStdString());
  return true;
}

DriverStats StdSerialIo::GetStats() const {
  std::lock_guard lock(m_stats_mutex);
  return m_stats;
}

bool StdSerialIo::OpenComPortPhysical(const wxString& com_name,
                                      unsigned baud_rate) {
  try {
    m_serial.setPort(com_name.ToStdString());
    m_serial.setBaudrate(baud_rate);
    m_serial.open();
    m_serial.setTimeout(250, 250, 0, 250, 0);
  } catch (std::exception& e) {
    auto msg = std::string("Unhandled Exception while opening serial port: ");
    m_open_log_filter.Log(msg + e.what());
  }
  {
    std::lock_guard lock(m_stats_mutex);
    m_stats.available = m_serial.isOpen();
  }
  return m_serial.isOpen();
}

void StdSerialIo::CloseComPortPhysical() {
  try {
    m_serial.close();
  } catch (std::exception& e) {
    MESSAGE_LOG << "Unhandled Exception while closing serial port: "
                << e.what();
  }
}

ssize_t StdSerialIo::WriteComPortPhysical(const char* msg) {
  if (m_serial.isOpen()) {
    try {
      return static_cast<ssize_t>(m_serial.write((uint8_t*)msg, strlen(msg)));
    } catch (std::exception& e) {
      DEBUG_LOG << "Unhandled Exception while writing to serial port: "
                << e.what();
      return -1;
    }
  } else {
    return -1;
  }
}

void SerialIo::RequestStop() { ThreadCtrl::RequestStop(); }
