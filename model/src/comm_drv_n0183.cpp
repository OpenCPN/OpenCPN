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

/** Return true if checksum in 0183 sentence is correct. */
static bool Is0183ChecksumOk(const std::string& sentence) {
  const size_t cs_start = sentence.find('*');
  if (cs_start == std::string::npos || cs_start > sentence.size() - 3)
    return false;  // Not found, or didn't have 2 characters following it.

  const std::string cs_str = sentence.substr(cs_start + 1, 2);
  const unsigned long checksum = strtol(cs_str.c_str(), nullptr, 16);
  if (checksum == 0L && cs_str != "00") return false;

  unsigned char calculated_checksum = 0;
  for (const char c : sentence.substr(1, cs_start - 1))
    calculated_checksum ^= static_cast<unsigned char>(c);
  return calculated_checksum == checksum;
}

/**
 * Return part of string starting with '$' or '!' and check length.
 * In particular, strip v4 tag prefixes.
 * @return part starting with '$' or '!' guaranteed to be at six least chars
 * if found in input, else "".
 */
static std::string GetPayloadSentence(const std::string& sentence) {
  size_t start_pos = sentence.find('$');
  if (start_pos == std::string::npos) start_pos = sentence.find('!');
  if (start_pos == std::string::npos) return "";
  if (sentence.size() < start_pos + 6) return "";
  return sentence.substr(start_pos);
}

CommDriverN0183::CommDriverN0183() : AbstractCommDriver(NavAddr::Bus::N0183) {}

CommDriverN0183::CommDriverN0183(NavAddr::Bus, const std::string& s)
    : AbstractCommDriver(NavAddr::Bus::N0183, s) {}

CommDriverN0183::~CommDriverN0183() = default;

void CommDriverN0183::SendToListener(const std::string& payload,
                                     DriverListener& listener,
                                     const ConnectionParams& params) {
  const std::string sentence = GetPayloadSentence(payload);
  if (sentence.empty()) return;
  assert(sentence[0] == '$' || sentence[0] == '!');
  assert(sentence.size() >= 6);

  const bool is_garbage =
      std::any_of(sentence.begin(), sentence.end(),
                  [](char c) { return !isprint(c) && c != '\n' && c != '\r'; });
  const bool has_checksum =
      sentence.find('*', sentence.size() - 6) != std::string::npos;

  NavMsg::State state;
  if (is_garbage)
    state = NavMsg::State::kCannotParse;
  else if (!params.SentencePassesFilter(sentence, FILTER_INPUT))
    state = NavMsg::State::kFiltered;
  else if (has_checksum && !Is0183ChecksumOk(sentence))
    state = NavMsg::State::kBadChecksum;
  else
    state = NavMsg::State::kOk;

  // We notify based on full message, including the Talker ID
  std::string id =
      state == NavMsg::State::kCannotParse ? "TRASH" : sentence.substr(1, 5);
  auto msg =
      std::make_shared<const Nmea0183Msg>(id, sentence, GetAddress(), state);
  listener.Notify(std::move(msg));
}
