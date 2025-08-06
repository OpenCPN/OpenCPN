/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Implement the comm_drv_n0183.h -- Nmea0183 driver base
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
#endif

#include "model/comm_drv_n0183.h"

static bool Is0183ChecksumOk(const std::string& sentence) {
  size_t check_start = sentence.find('*');
  if (check_start == wxString::npos || check_start > sentence.size() - 3)
    return false;  // * not found, or it didn't have 2 characters following it.

  std::string check_str = sentence.substr(check_start + 1, 2);
  unsigned long checksum = strtol(check_str.c_str(), nullptr, 16);
  if (checksum == 0L && check_str != "00") return false;

  unsigned char calculated_checksum = 0;
  for (auto i = sentence.begin() + 1; i != sentence.end() && *i != '*'; ++i)
    calculated_checksum ^= static_cast<unsigned char>(*i);

  return calculated_checksum == checksum;
}

CommDriverN0183::CommDriverN0183() : AbstractCommDriver(NavAddr::Bus::N0183) {}

CommDriverN0183::CommDriverN0183(NavAddr::Bus b, const std::string& s)
    : AbstractCommDriver(NavAddr::Bus::N0183, s) {}

CommDriverN0183::~CommDriverN0183() = default;

void CommDriverN0183::SendToListener(const std::string& payload,
                                     DriverListener& listener,
                                     const ConnectionParams& params) {
  if (payload.empty()) return;

  bool is_garbage = payload.size() < 6;
  is_garbage |= (payload[0] != '$' && payload[0] != '!');
  is_garbage |= !std::all_of(payload.begin(), payload.end(), [](char c) {
    return isprint(c) || c == '\n' || c == '\r';
  });

  NavMsg::State state;
  if (is_garbage)
    state = NavMsg::State::kCannotParse;
  else if (!params.SentencePassesFilter(payload, FILTER_INPUT))
    state = NavMsg::State::kFiltered;
  else if (!Is0183ChecksumOk(payload))
    state = NavMsg::State::kBadChecksum;
  else
    state = NavMsg::State::kOk;

  // We notify based on full message, including the Talker ID
  std::string id =
      state == NavMsg::State::kCannotParse ? "TRASH" : payload.substr(1, 5);
  auto msg =
      std::make_shared<const Nmea0183Msg>(id, payload, GetAddress(), state);
  listener.Notify(std::move(msg));
}
