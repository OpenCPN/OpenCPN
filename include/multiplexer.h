/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  NMEA Data Multiplexer Object
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
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
#ifndef _MULTIPLEXER_H__
#define _MULTIPLEXER_H__

#include <functional>

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // precompiled headers

#include "comm_navmsg.h"

struct MuxLogCallbacks {
  std::function<bool()> log_is_active;
  std::function<void(const std::string&)> log_message;
  MuxLogCallbacks()
    : log_is_active([]() { return false; }),
      log_message([](const std::string& s) { }) { }

};


class Multiplexer : public wxEvtHandler {
public:
  Multiplexer(MuxLogCallbacks log_callbacks,
              bool& legacy_input_filter_behaviour );
  ~Multiplexer();

  void LogOutputMessage(const wxString &msg, wxString stream_name,
                        bool b_filter);
  void LogOutputMessageColor(const wxString &msg, const wxString &stream_name,
                             const wxString &color);
  void LogInputMessage(const wxString &msg, const wxString &stream_name,
                       bool b_filter, bool b_error = false);

private:
  //  comm event listeners
  ObservableListener listener_N2K_All;
  ObservableListener m_listener_N0183_all;

  void InitN2KCommListeners();

  void HandleN0183(std::shared_ptr<const Nmea0183Msg> n0183_msg);
  bool HandleN2K_Log(std::shared_ptr<const Nmea2000Msg> n2k_msg);
  std::string N2K_LogMessage_Detail(unsigned int pgn,
                                    std::shared_ptr<const Nmea2000Msg> n2k_msg);

  MuxLogCallbacks m_log_callbacks;
  unsigned int last_pgn_logged;
  int n_N2K_repeat;
  bool&  m_legacy_input_filter_behaviour;
};
#endif  // _MULTIPLEXER_H__
