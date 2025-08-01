/**************************************************************************
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

/**
 * \file
 *
 * Multiplexer class and helpers
 */

#ifndef MULTIPLEXER_H_
#define MULTIPLEXER_H_

#include <functional>

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "model/comm_navmsg.h"
#include "model/nmea_log.h"

class Multiplexer;  // forward

extern Multiplexer* g_pMUX;

struct MuxLogCallbacks {
  std::function<bool()> log_is_active;
  std::function<void(const Logline&)> log_message;
  MuxLogCallbacks()
      : log_is_active([]() { return false; }),
        log_message([](const Logline&) {}) {}
};

/**
 * Handle logging and forwarding of incoming n0183/n2k  messages.
 *
 * Listen to all n0183 and n2k messages, whether they are known or not
 * i.e., anything generating an input event in the Data Monitor parlance.
 */
class Multiplexer : public wxEvtHandler {
public:
  Multiplexer(const MuxLogCallbacks& log_callbacks,
              bool& legacy_input_filter_behaviour);

  ~Multiplexer() override;

  void LogOutputMessage(const std::shared_ptr<const NavMsg>& msg,
                        NavmsgStatus status) const;

  /**
   * Logs an input message with context information.
   *
   * @param msg The message to be logged.
   * @param is_filtered Indicates whether the message was filtered.
   * @param is_error Indicates whether the message has an error such as bad CRC.
   * @param error_msg The error message to be logged.
   */
  void LogInputMessage(const std::shared_ptr<const NavMsg>& msg,
                       bool is_filtered, bool is_error,
                       const wxString& error_msg = "") const;

  [[nodiscard]] bool IsLogActive() const {
    return m_log_callbacks.log_is_active();
  }

private:
  MuxLogCallbacks m_log_callbacks;
  bool& m_legacy_input_filter_behaviour;
  std::unordered_map<std::string, ObsListener> m_listeners;
  ObsListener m_new_msgtype_lstnr;
  int m_n2k_repeat_count;
  unsigned int m_last_pgn_logged;

  void OnNewMessageType();

  void HandleN0183(const std::shared_ptr<const Nmea0183Msg>& n0183_msg) const;

  bool HandleN2kLog(const std::shared_ptr<const Nmea2000Msg>& n2k_msg);
};
#endif  // MULTIPLEXER_H_
