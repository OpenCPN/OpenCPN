/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:
 * Author:   David Register, Alec Leamas
 *
 ***************************************************************************
 *   Copyright (C) 2023 by David Register, Alec Leamas                     *
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

#ifndef _COMMDRIVERN0183ANDROIDBT_H
#define _COMMDRIVERN0183ANDROIDBT_H

#include <atomic>
#include <string>

#include <wx/event.h>

#include "comm_drv_n0183.h"
#include "conn_params.h"

class CommDriverN0183AndroidBTEvent : public wxEvent {
public:
  CommDriverN0183AndroidBTEvent( wxEventType commandType, int id);
  ~CommDriverN0183AndroidBTEvent();

  // accessors
  void SetPayload(std::shared_ptr<std::vector<unsigned char>> data);
  std::shared_ptr<std::vector<unsigned char>> GetPayload();

  // required for sending with wxPostEvent()
  wxEvent* Clone() const;
private:
  std::shared_ptr<std::vector<unsigned char>> m_payload;
};

wxDECLARE_EVENT(wxEVT_COMMDRIVER_N0183_ANDROID_BT, CommDriverN0183AndroidBTEvent);


class CommDriverN0183AndroidBT : public CommDriverN0183, public wxEvtHandler {
public:
  CommDriverN0183AndroidBT(const ConnectionParams* params, DriverListener& l);

  virtual ~CommDriverN0183AndroidBT();

  /** Register driver and possibly do other post-ctor steps. */
  void Activate() override;

  bool Open();
  void Close();

  ConnectionParams GetParams() const { return m_params; }

  bool SendMessage(std::shared_ptr<const NavMsg> msg,
                   std::shared_ptr<const NavAddr> addr) override;

private:
  bool m_bok;
  std::string m_portstring;
  std::string m_BaudRate;
  int m_handshake;


  ConnectionParams m_params;
  DriverListener& m_listener;
  void handle_N0183_MSG(CommDriverN0183AndroidBTEvent& event);
};

#endif  // guard
