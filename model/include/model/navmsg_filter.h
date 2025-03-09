
/***************************************************************************
 *   Copyright (C) 2025  Alec Leamas                                       *
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

#ifndef MONITOR_FILTER_H_
#define MONITOR_FILTER_H_

/**
 * \file
 * Data monitor filter definitions.
 *
 * Filter       ::= "filter:" <name> [description] [buses] [directions] [status]
 *                    [include-msg | exclude-msg] [interfaces] [pgn] [address]
 *                      "/filter;"
 * name         ::= "name:" <word> ";"
 * description  ::= "description:" [free-text]  "/description;"
 * buses        ::= "buses:"  <bus-type {bus-type} | "Any">  ";"
 * bus-type     ::= <"nmea0183" | "nmea2000" | "signalk" |"internal" | "onenet">
 * directions   ::= "directions:"
 *                      <"input" | "received" | "output" | "internal"> ";"
 * accepted     ::= "accepted:"
 *                      <"yes" | "filtered-no-output" | "filtered-dropped">
 * status       ::= "status:" <"ok" | "checksum_error" | "malformed" >  ";"
 * include-msg  ::= "include-msg:" <nmea-type  {nmea-type}> ";"
 * exclude-msg  ::= "exclude-msg:" <nmea-type  {nmea-type}> ";"
 * nmea-type    ::= word
 * interfaces   ::= "interfaces:" <word  {word}> ";"
 * pgn          ::= "pgn:" <number> ";"
 * address      ::= "src-address:" <word> ";"
 *
 */

#include <cassert>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>

#include "model/comm_navmsg.h"

#include "std_filesystem.h"

/**
 * Packed representation of message status as determined by the
 * multiplexer
 */
class NavmsgStatus {
public:
  enum class Direction { kInput, kReceived, kOutput, kInternal, kNone };
  enum class Accepted { kOk, kFilteredNoOutput, kFilteredDropped, kNone };
  enum class State { kOk, kChecksumError, kMalformed, kTxError, kNone };

  Direction direction : 3;
  State status : 3;
  Accepted accepted : 2;

  NavmsgStatus()
      : direction(Direction::kInput),
        status(State::kOk),
        accepted(Accepted::kOk) {}
};

class NavmsgFilter {
public:
  NavmsgFilter() : m_is_valid(true) {}

  NavmsgFilter(bool is_valid) : m_is_valid(is_valid) {}

  /** Return list fo pre-defined filters shipped with application. */
  static std::vector<NavmsgFilter> GetSystemFilters();

  /** Return list fo pre-defined filters shipped with app, test hook. */
  static std::vector<NavmsgFilter> GetSystemFilters(const fs::path& path);

  /** Parse text as created by to_string(). */
  static NavmsgFilter Parse(const std::string& s);

  /** Parse text as created by to_string() stored in a file */
  static NavmsgFilter Parse(const fs::path& path);

  /** Output parsable JSON string representation. */
  std::string to_string();

  /** Return true if message is not matched by filter. */
  bool Pass(NavmsgStatus status, const std::shared_ptr<const NavMsg>& msg);

  /** Return human-readable name. */
  std::string GetName();

  /** Set human-readable name. */
  void SetName(const std::string& new_name);

  bool m_is_valid;
  std::string m_name;
  std::string m_description;
  std::set<NavmsgStatus::Direction> directions;
  std::set<NavmsgStatus::State> status;
  std::set<NavmsgStatus::Accepted> accepted;
  std::set<NavAddr::Bus> buses;
  std::set<std::string> include_msg;  // Set of message ids included
  std::set<std::string> exclude_msg;  // Set of message ids excluded
  std::set<std::string> interfaces;
  std::set<N2kPGN> pgns;       // Nmea200 only
  std::set<N2kName> src_pgns;  // Nmea200 only
};

#endif  // MONITOR_FILTER
