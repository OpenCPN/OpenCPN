/***************************************************************************
 *   Copyright (C) 2022 David Register                                     *
 *   Copyright (C) 2022 Alec Leamas                                        *
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

/** \file comm_drv_n0183_serial.cpp  Implement comm_drv_n0183_serial.h */

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

#include "config.h"
#include "model/comm_buffers.h"
#include "model/comm_drv_n0183_serial.h"
#include "model/comm_navmsg_bus.h"
#include "model/comm_drv_registry.h"
#include "model/logger.h"
#include "model/sys_events.h"
#include "model/wait_continue.h"
#include "observable.h"

#ifdef __ANDROID__
#include "androidUTIL.h"
#else
#include "serial/serial.h"
#include "model/n0183_comm_mgr.h"
#endif

using namespace std::literals::chrono_literals;

typedef enum DS_ENUM_BUFFER_STATE {
  DS_RX_BUFFER_EMPTY,
  DS_RX_BUFFER_FULL
} _DS_ENUM_BUFFER_STATE;

CommDriverN0183Serial::CommDriverN0183Serial(const ConnectionParams* params,
                                             DriverListener& listener)
    : CommDriverN0183(NavAddr::Bus::N0183,
                      ((ConnectionParams*)params)->GetStrippedDSPort()),
      m_portstring(params->GetDSPort()),
      m_secondary_thread(
          [&](const std::vector<unsigned char>& v) { SendMessage(v); }),
      m_params(*params),
      m_listener(listener) {
  m_baudrate = wxString::Format("%i", params->Baudrate);
  m_garmin_handler = NULL;
  this->attributes["commPort"] = params->Port.ToStdString();
  this->attributes["userComment"] = params->UserComment.ToStdString();
  dsPortType iosel = params->IOSelect;
  std::string s_iosel = std::string("IN");
  if (iosel == DS_TYPE_INPUT_OUTPUT) {
    s_iosel = "OUT";
  } else if (iosel == DS_TYPE_INPUT_OUTPUT) {
    s_iosel = "IN/OUT";
  }
  this->attributes["ioDirection"] = s_iosel;
  Open();
}

CommDriverN0183Serial::~CommDriverN0183Serial() { Close(); }

bool CommDriverN0183Serial::Open() {
  wxString comx;
  comx = m_params.GetDSPort().AfterFirst(':');  // strip "Serial:"
  if (comx.IsEmpty()) return false;

  wxString port_uc = m_params.GetDSPort().Upper();

  auto send_func = [&](const std::vector<unsigned char>& v) { SendMessage(v); };
  if ((wxNOT_FOUND != port_uc.Find("USB")) &&
      (wxNOT_FOUND != port_uc.Find("GARMIN"))) {
    m_garmin_handler = new GarminProtocolHandler(comx, send_func, true);
  } else if (m_params.Garmin) {
    m_garmin_handler = new GarminProtocolHandler(comx, send_func, false);
  } else {
    // strip off any description provided by Windows
    comx = comx.BeforeFirst(' ');

#ifndef __ANDROID__
    //    Kick off the  RX thread
    m_secondary_thread.SetParams(comx, m_baudrate);
    std::thread t(&CommDriverN0183SerialThread::Entry, &m_secondary_thread);
    t.detach();
#else
    androidStartUSBSerial(
        comx, m_baudrate,
        [&](const std::vector<unsigned char>& v) { SendMessage(v); });
#endif
  }

  return true;
}

void CommDriverN0183Serial::Close() {
  wxLogMessage(
      wxString::Format("Closing NMEA Driver %s", m_portstring.c_str()));

  // FIXME (dave)
  // If port is opened, and then closed immediately,
  // the secondary thread may not stop quickly enough.
  // It can then crash trying to send an event to its "parent".

#ifndef __ANDROID__
  //    Kill off the Secondary RX Thread if alive
  if (m_secondary_thread.IsRunning()) {
    using namespace std::chrono;
    wxLogMessage("Stopping Secondary Thread");
    m_secondary_thread.RequestStop();
    std::chrono::milliseconds elapsed;
    if (m_secondary_thread.WaitUntilStopped(10s, elapsed)) {
      MESSAGE_LOG << "Stopped in " << elapsed.count() << " msec.";
    } else {
      MESSAGE_LOG << "Not stopped after 10 sec.";
    }
  }

  //  Kill off the Garmin handler, if alive
  if (m_garmin_handler) {
    m_garmin_handler->Close();
    delete m_garmin_handler;
    m_garmin_handler = NULL;
  }

#else
  wxString comx;
  comx = m_params.GetDSPort().AfterFirst(':');  // strip "Serial:"
  androidStopUSBSerial(comx);
#endif
}

bool CommDriverN0183Serial::IsGarminThreadActive() {
  if (m_garmin_handler) {
    // TODO expand for serial
#ifdef __WXMSW__
    if (m_garmin_handler->m_usb_handle != INVALID_HANDLE_VALUE)
      return true;
    else
      return false;
#endif
  }

  return false;
}

void CommDriverN0183Serial::StopGarminUSBIOThread(bool b_pause) {
  if (m_garmin_handler) {
    m_garmin_handler->StopIOThread(b_pause);
  }
}

void CommDriverN0183Serial::Activate() {
  CommDriverRegistry::GetInstance().Activate(shared_from_this());
  // TODO: Read input data.
}

bool CommDriverN0183Serial::SendMessage(std::shared_ptr<const NavMsg> msg,
                                        std::shared_ptr<const NavAddr> addr) {
  auto msg_0183 = std::dynamic_pointer_cast<const Nmea0183Msg>(msg);
  wxString sentence(msg_0183->payload.c_str());

#ifdef __ANDROID__
  wxString payload = sentence;
  if (!sentence.EndsWith("\r\n")) payload += "\r\n";

  wxString port = m_params.GetStrippedDSPort();  // GetPort().AfterFirst(':');
  androidWriteSerial(port, payload);
  return true;
#else
  if (m_secondary_thread.IsRunning()) {
    for (int retries = 0; retries < 10; retries += 1) {
      if (m_secondary_thread.SetOutMsg(sentence)) {
        return true;
      }
    }
    return false;  // could not send after several tries....
  } else {
    return false;
  }
#endif
}

void CommDriverN0183Serial::SendMessage(const std::vector<unsigned char>& msg) {
  // Is this an output-only port?
  // Commonly used for "Send to GPS" function
  if (m_params.IOSelect == DS_TYPE_OUTPUT) return;

  // sanity checks
  if (msg.size() < 6) return;
  if (msg[0] != '$' && msg[0] != '!') return;

  // We use the full src + type to discriminate messages,  like GPGGA
  std::string identifier(msg.begin() + 1, msg.begin() + 6);

  // notify msg listener and also "ALL" N0183 messages, to support plugin
  // API using original talker id
  std::string payload(msg.begin(), msg.end());
  auto message =
      std::make_shared<const Nmea0183Msg>(identifier, payload, GetAddress());
  auto message_all = std::make_shared<const Nmea0183Msg>(*message, "ALL");

  if (m_params.SentencePassesFilter(payload, FILTER_INPUT))
    m_listener.Notify(std::move(message));
  m_listener.Notify(std::move(message_all));
}
