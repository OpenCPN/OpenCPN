/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  IO thread for serial N0183 connections
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

// For compilers that support precompilation, includes "wx.h".
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // precompiled headers

#include <mutex>  // std::mutex
#include <queue>  // std::queue
#include <thread>
#include <vector>

#include <wx/event.h>
#include <wx/log.h>
#include <wx/string.h>
#include <wx/utils.h>

#include "model/comm_buffers.h"
#include "model/comm_drv_n0183_serial.h"
#include "model/comm_drv_registry.h"
#include "model/comm_navmsg_bus.h"
#include "model/logger.h"
#include "model/n0183_comm_mgr.h"
#include "serial/serial.h"

CommDriverN0183SerialThread::CommDriverN0183SerialThread(
    CommDriverN0183Serial* Launcher, const wxString& PortName,
    const wxString& strBaudRate) {
  m_pParentDriver = Launcher;  // This thread's immediate "parent"

  m_PortName = PortName;
  m_FullPortName = "Serial:" + PortName;

  m_baud = 4800;  // default
  long lbaud;
  if (strBaudRate.ToLong(&lbaud)) m_baud = (int)lbaud;
}

CommDriverN0183SerialThread::~CommDriverN0183SerialThread(void) {}

void CommDriverN0183SerialThread::OnExit(void) {}

bool CommDriverN0183SerialThread::OpenComPortPhysical(const wxString& com_name,
                                                      int baud_rate) {
  try {
    m_serial.setPort(com_name.ToStdString());
    m_serial.setBaudrate(baud_rate);
    m_serial.open();
    m_serial.setTimeout(250, 250, 0, 250, 0);
  } catch (std::exception& e) {
    //      std::cerr << "Unhandled Exception while opening serial port: " <<
    //      e.what() << std::endl;
  }
  return m_serial.isOpen();
}

void CommDriverN0183SerialThread::CloseComPortPhysical() {
  try {
    m_serial.close();
  } catch (std::exception& e) {
    //      std::cerr << "Unhandled Exception while closing serial port: " <<
    //      e.what() << std::endl;
  }
}

bool CommDriverN0183SerialThread::SetOutMsg(const wxString& msg) {
  m_out_que.Put(msg.ToStdString());
  return true;
}

void CommDriverN0183SerialThread::ThreadMessage(const wxString& msg) {
  //    Signal the main program thread
  //   OCPN_ThreadMessageEvent event(wxEVT_OCPN_THREADMSG, 0);
  //   event.SetSString(std::string(msg.mb_str()));
  //   if (gFrame) gFrame->GetEventHandler()->AddPendingEvent(event);
}

ssize_t CommDriverN0183SerialThread::WriteComPortPhysical(const char* msg) {
  if (m_serial.isOpen()) {
    ssize_t status;
    try {
      status = m_serial.write((uint8_t*)msg, strlen(msg));
    } catch (std::exception& e) {
      DEBUG_LOG << "Unhandled Exception while writing to serial port: "
                << e.what();
      return -1;
    }
    return status;
  } else {
    return -1;
  }
}

void* CommDriverN0183SerialThread::Entry() {
  m_pParentDriver->SetSecThreadActive();  // I am alive
  LineBuffer line_buf;

  //    Request the com port from the comm manager
  if (!OpenComPortPhysical(m_PortName, m_baud)) {
    wxString msg("NMEA input device open failed: ");
    msg.Append(m_PortName);
    ThreadMessage(msg);
    // goto thread_exit; // This means we will not be trying to connect = The
    // device must be connected when the thread is created. Does not seem to be
    // needed/what we want as the reconnection logic is able to pick it up
    // whenever it actually appears (Of course given it appears with the
    // expected device name).
  }

  //    The main loop
  unsigned retries = 0;
  while (m_pParentDriver->m_Thread_run_flag > 0) {
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
      if (OpenComPortPhysical(m_PortName, m_baud))
        retries = 0;
      else if (retries < 10)
        retries++;
    }

    // Handle received data
    for (unsigned i = 0; i < newdata; i++) line_buf.Put(rdata[i]);
    while (line_buf.HasLine()) {
      auto payload = std::make_shared<std::vector<uint8_t>>(line_buf.GetLine());
      CommDriverN0183SerialEvent Nevent(wxEVT_COMMDRIVER_N0183_SERIAL, 0);
      Nevent.SetPayload(payload);
      m_pParentDriver->AddPendingEvent(Nevent);
    }

    //  Handle pending output messages
    std::string qmsg;
    while (m_out_que.Get(qmsg)) {
      qmsg += "\r\n";
      if (-1 == WriteComPortPhysical(qmsg.c_str()) && 10 < retries++) {
        // We failed to write the port 10 times, let's close the port so that
        // the reconnection logic kicks in and tries to fix our connection.
        retries = 0;
        CloseComPortPhysical();
      }
    }
  }

  CloseComPortPhysical();
  m_pParentDriver->SetSecThreadInActive();  // I am dead
  m_pParentDriver->m_Thread_run_flag = -1;

  return 0;
}
