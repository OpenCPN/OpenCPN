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

/**
 *  \file
 *  Implement comm_drv_n0183_serial.h
 */

// For compilers that support precompilation, includes "wx.h".
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <mutex>  // std::mutex
#include <queue>  // std::queue
#include <vector>

#include <wx/log.h>
#include <wx/string.h>

#include "config.h"
#include "model/comm_buffers.h"
#include "model/comm_drv_n0183_serial.h"
#include "model/comm_drv_registry.h"
#include "model/logger.h"
#include "model/wait_continue.h"

#include "observable.h"

using namespace std::literals::chrono_literals;

CommDriverN0183Serial::CommDriverN0183Serial(const ConnectionParams* params,
                                             DriverListener& listener)
    : CommDriverN0183(NavAddr::Bus::N0183, params->GetStrippedDSPort()),
      m_portstring(params->GetDSPort()),
      m_baudrate(params->Baudrate),
      m_serial_io(SerialIo::Create(
          [&](const std::vector<unsigned char>& v) { SendMessage(v); },
          m_portstring, m_baudrate)),
      m_params(*params),
      m_listener(listener) {
  m_garmin_handler = nullptr;
  this->attributes["commPort"] = params->Port.ToStdString();
  this->attributes["userComment"] = params->UserComment.ToStdString();
  this->attributes["ioDirection"] = DsPortTypeToString(params->IOSelect);

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

    //    Kick off the  RX thread
    m_serial_io->Start();
  }

  return true;
}

void CommDriverN0183Serial::Close() {
  wxLogMessage(
      wxString::Format("Closing NMEA Driver %s", m_portstring.c_str()));

  //    Kill off the secondary RX IO if alive
  if (m_serial_io->IsRunning()) {
    wxLogMessage("Stopping Secondary Thread");
    m_serial_io->RequestStop();
    std::chrono::milliseconds elapsed;
    if (m_serial_io->WaitUntilStopped(10s, elapsed)) {
      MESSAGE_LOG << "Stopped in " << elapsed.count() << " msec.";
    } else {
      MESSAGE_LOG << "Not stopped after 10 sec.";
    }
  }

  //  Kill off the Garmin handler, if alive
  if (m_garmin_handler) {
    m_garmin_handler->Close();
    delete m_garmin_handler;
    m_garmin_handler = nullptr;
  }
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

bool CommDriverN0183Serial::SendMessage(std::shared_ptr<const NavMsg> msg,
                                        std::shared_ptr<const NavAddr> addr) {
  auto msg_0183 = std::dynamic_pointer_cast<const Nmea0183Msg>(msg);
  wxString sentence(msg_0183->payload.c_str());

  if (m_serial_io->IsRunning()) {
    for (int retries = 0; retries < 10; retries += 1) {
      if (m_serial_io->SetOutMsg(sentence)) {
        return true;
      }
    }
    return false;  // could not send after several tries....
  } else {
    return false;
  }
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
