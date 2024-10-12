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
 *  \file android_ser_thread.cpp Android serial IO synchronous implementaion.
 */

#include <string>

// For compilers that support precompilation, includes "wx.h".
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // precompiled headers

#include <wx/log.h>
#include <wx/string.h>
#include <wx/utils.h>

#include "androidUTIL.h"
#include "model/n0183_comm_mgr.h"

CommDriverN0183SerialThread::CommDriverN0183SerialThread(SendMsgFunc send_func)
    : m_portname("undefined"), m_baud(4800), m_send_msg_func(send_func) {}

CommDriverN0183SerialThread::~CommDriverN0183SerialThread() = default;

void CommDriverN0183SerialThread::SetParams(const wxString& portname,
                                            const wxString& str_baudrate) {
  m_portname = portname;
  long lbaud;
  if (str_baudrate.ToLong(&lbaud)) m_baud = static_cast<int>(lbaud);
}

void CommDriverN0183SerialThread::Start() {
  if (m_portname.empty()) return;
  androidStartUSBSerial(m_portname, std::to_string(m_baud), m_send_msg_func);
}

void* CommDriverN0183SerialThread::Entry() {
  assert(false && "Android IO must be started using Start()");
  return nullptr;  // for the compiler
}

bool CommDriverN0183SerialThread::SetOutMsg(const wxString& msg) {
  wxString payload = msg;
  if (!msg.EndsWith("\r\n")) payload += "\r\n";
  androidWriteSerial(m_portname, payload);
  return true;
}

void CommDriverN0183SerialThread::RequestStop() {
  androidStopUSBSerial(m_portname);
  ThreadCtrl::RequestStop();
  ThreadCtrl::SignalExit();  // No need for exiting thread doing this.
}
