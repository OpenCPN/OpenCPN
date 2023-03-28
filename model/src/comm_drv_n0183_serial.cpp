/**************************************************************************
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

typedef enum DS_ENUM_BUFFER_STATE {
  DS_RX_BUFFER_EMPTY,
  DS_RX_BUFFER_FULL
} _DS_ENUM_BUFFER_STATE;

class CommDriverN0183Serial;  // fwd

wxDEFINE_EVENT(wxEVT_COMMDRIVER_N0183_SERIAL, CommDriverN0183SerialEvent);

CommDriverN0183SerialEvent::CommDriverN0183SerialEvent(wxEventType commandType,
                                                       int id = 0)
    : wxEvent(id, commandType) {};

CommDriverN0183SerialEvent::~CommDriverN0183SerialEvent() {};

void CommDriverN0183SerialEvent::SetPayload(
    std::shared_ptr<std::vector<unsigned char>> data) {
  m_payload = data;
}
std::shared_ptr<std::vector<unsigned char>>
CommDriverN0183SerialEvent::GetPayload() {
  return m_payload;
}

// required for sending with wxPostEvent()
wxEvent* CommDriverN0183SerialEvent::Clone() const {
  CommDriverN0183SerialEvent* newevent = new CommDriverN0183SerialEvent(*this);
  newevent->m_payload = this->m_payload;
  return newevent;
};

CommDriverN0183Serial::CommDriverN0183Serial(const ConnectionParams* params,
                                             DriverListener& listener)
    : CommDriverN0183(NavAddr::Bus::N0183,
                      ((ConnectionParams*)params)->GetStrippedDSPort()),
      m_Thread_run_flag(-1),
      m_ok(false),
      m_portstring(params->GetDSPort()),
      m_secondary_thread(NULL),
      m_params(*params),
      m_listener(listener),
      m_out_queue(std::unique_ptr<CommOutQueue>(new MeasuredCommOutQueue(12))) {
  m_baudrate = wxString::Format("%i", params->Baudrate);
  SetSecThreadInActive();
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

  // Prepare the wxEventHandler to accept events from the actual hardware thread
  Bind(wxEVT_COMMDRIVER_N0183_SERIAL, &CommDriverN0183Serial::handle_N0183_MSG,
       this);

  Open();
}

CommDriverN0183Serial::~CommDriverN0183Serial() { Close(); }

bool CommDriverN0183Serial::Open() {
  wxString comx;
  comx = m_params.GetDSPort().AfterFirst(':');  // strip "Serial:"
  if (comx.IsEmpty()) return false;

  wxString port_uc = m_params.GetDSPort().Upper();

  if ((wxNOT_FOUND != port_uc.Find("USB")) &&
      (wxNOT_FOUND != port_uc.Find("GARMIN"))) {
    m_garmin_handler = new GarminProtocolHandler(comx, this, true);
  } else if (m_params.Garmin) {
    m_garmin_handler = new GarminProtocolHandler(comx, this, false);
  } else {
    // strip off any description provided by Windows
    comx = comx.BeforeFirst(' ');

#ifndef __ANDROID__
    //    Kick off the  RX thread
    SetSecondaryThread(new CommDriverN0183SerialThread(this, comx, m_baudrate));
    SetThreadRunFlag(1);
    std::thread t(&CommDriverN0183SerialThread::Entry, GetSecondaryThread());
    t.detach();
#else
    androidStartUSBSerial(comx, m_baudrate, this);
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

  Unbind(wxEVT_COMMDRIVER_N0183_SERIAL,
         &CommDriverN0183Serial::handle_N0183_MSG, this);

#ifndef __ANDROID__
  //    Kill off the Secondary RX Thread if alive
  if (m_secondary_thread) {
    if (m_sec_thread_active)  // Try to be sure thread object is still alive
    {
      wxLogMessage("Stopping Secondary Thread");

      m_Thread_run_flag = 0;

      int tsec = 10;
      while ((m_Thread_run_flag >= 0) && (tsec--)) wxSleep(1);

      wxString msg;
      if (m_Thread_run_flag < 0)
        msg.Printf("Stopped in %d sec.", 10 - tsec);
      else
        msg.Printf("Not Stopped after 10 sec.");
      wxLogMessage(msg);
    }

    delete m_secondary_thread;
    m_secondary_thread = NULL;
    m_sec_thread_active = false;
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
  if (GetSecondaryThread()) {
    if (IsSecThreadActive()) {
      int retry = 10;
      while (retry) {
        if (GetSecondaryThread()->SetOutMsg(sentence))
          return true;
        else
          retry--;
      }
      return false;  // could not send after several tries....
    } else
      return false;
  }
  return true;
#endif
}

void CommDriverN0183Serial::handle_N0183_MSG(
    CommDriverN0183SerialEvent& event) {
  // Is this an output-only port?
  // Commonly used for "Send to GPS" function
  if (m_params.IOSelect == DS_TYPE_OUTPUT) return;

  auto p = event.GetPayload();
  std::vector<unsigned char>* payload = p.get();

  // Extract the NMEA0183 sentence
  std::string full_sentence = std::string(payload->begin(), payload->end());

  if ((full_sentence[0] == '$') || (full_sentence[0] == '!')) {  // Sanity check
    std::string identifier;
    // We notify based on full message, including the Talker ID
    identifier = full_sentence.substr(1, 5);

    // notify message listener and also "ALL" N0183 messages, to support plugin
    // API using original talker id
    auto msg = std::make_shared<const Nmea0183Msg>(identifier, full_sentence,
                                                   GetAddress());
    auto msg_all = std::make_shared<const Nmea0183Msg>(*msg, "ALL");

    if (m_params.SentencePassesFilter(full_sentence, FILTER_INPUT))
      m_listener.Notify(std::move(msg));

    m_listener.Notify(std::move(msg_all));
  }
}
