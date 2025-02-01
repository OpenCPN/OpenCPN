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

#include "model/comm_navmsg.h"
#include "model/nmea_log.h"

class Multiplexer;  // forward

extern Multiplexer* g_pMUX;

struct MuxLogCallbacks {
  std::function<bool()> log_is_active;
  std::function<void(Logline)> log_message;
  MuxLogCallbacks()
      : log_is_active([]() { return false; }), log_message([](Logline) {}) {}
};

class Multiplexer : public wxEvtHandler {
public:
  Multiplexer(MuxLogCallbacks log_callbacks,
              bool& legacy_input_filter_behaviour);
  ~Multiplexer();

  void LogOutputMessage(const std::shared_ptr<const NavMsg>& msg,
                        const std::string& stream_name, NavmsgStatus status);

  void LogOutputMessageColor(const std::shared_ptr<const NavMsg>& msg,
                             const std::string& stream_name,
                             NavmsgStatus status);
  /**
   * Logs an input message with context information.
   *
   * @param msg The message to be logged.
   * @param stream_name The name of the stream from which the message
   * originated.
   * @param b_filter Indicates whether the message was filtered.
   * @param b_error Indicates whether the message has an error such as bad CRC.
   * @param error_msg The error message to be logged.
   */
  void LogInputMessage(const std::shared_ptr<const NavMsg>& msg,
                       const std::string& stream_name, bool is_filtered,
                       bool is_error, const wxString error_msg = "");

  bool IsLogActive() { return m_log_callbacks.log_is_active(); }

private:
  //  comm event listeners
  ObservableListener listener_N2K_All;
  ObservableListener m_listener_N0183_all;

  void InitN2KCommListeners();

  void HandleN0183(std::shared_ptr<const Nmea0183Msg> n0183_msg);
  bool HandleN2K_Log(std::shared_ptr<const Nmea2000Msg> n2k_msg);
  std::string N2K_LogMessage_Detail(unsigned int pgn);

  MuxLogCallbacks m_log_callbacks;
  unsigned int last_pgn_logged;
  int n_N2K_repeat;
  bool& m_legacy_input_filter_behaviour;
};
#endif  // _MULTIPLEXER_H__
