/**************************************************************************
 *   Copyright (C) 2025 Alec Leamas                                        *
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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

/**
 *  \file
 *
 *  Implement comm_drv_loopback.h -- Accept messages, treat them as received
 */

// For compilers that support precompilation, includes "wx.h".
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <algorithm>
#include <cctype>
#include <memory>
#include <string>
#include <vector>

#include <wx/jsonreader.h>
#include <wx/log.h>
#include <wx/string.h>

#include "model/comm_drv_loopback.h"
#include "model/comm_navmsg.h"
#include "model/ocpn_utils.h"

using namespace std;

using NavMsgPtr = shared_ptr<const NavMsg>;

static string NextWord(std::string& line) {
  if (line.empty()) return "";
  size_t space_pos = line.find(" ");
  if (space_pos == string::npos) space_pos = line.size();
  string word(line.substr(0, space_pos));
  line = ocpn::ltrim(line.substr(space_pos));
  return word;
}

static unsigned HexNibbleValue(char c) {
  c = tolower(c);
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return 10 + c - 'a';
  return -1;
}

static unsigned char HexByteValue(const string& text) {
  unsigned value;
  if (text.size() == 1) {
    value = HexNibbleValue(text[0]);
  } else {
    value = 16 * HexNibbleValue(text[0]) + HexNibbleValue(text[1]);
  }
  return static_cast<unsigned char>(value);
}

static NavMsgPtr Parse2000(const std::string& iface, const string& type,
                           const std::string& msg) {
  vector<unsigned char> payload;
  auto hexbytes = ocpn::split(msg, " ");
  for (const auto& byte : hexbytes) payload.push_back(HexByteValue(byte));
  uint64_t pgn;
  try {
    pgn = stoi(type);
  } catch (invalid_argument&) {
    return {nullptr};
  }
  return make_shared<Nmea2000Msg>(
      pgn, payload, make_shared<NavAddr2000>(iface, N2kName(pgn)));
}

static NavMsgPtr Parse0183(const string& iface, const string& type,
                           const string& msg) {
  return make_shared<Nmea0183Msg>(type, msg, make_shared<NavAddr0183>(iface));
}

static NavMsgPtr ParseSignalk(const string& iface, const string& type,
                              const string& msg) {
  wxJSONValue root;
  wxJSONReader reader;
  int err_count = reader.Parse(msg, &root);
  std::string context;
  if (err_count == 0) {
    if (root.HasMember("context")) context = root["context"].AsString();
  }
  return make_shared<SignalkMsg>(type, context, msg, iface);
}

NavMsgPtr LoopbackDriver::ParsePluginMessage(const string& msg) {
  int space_count = count(msg.begin(), msg.end(), ' ');
  if (space_count < 3) {
    return {nullptr};
  }
  std::string msg_(msg);
  auto protocol = ocpn::tolower(NextWord(msg_));
  auto iface = NextWord(msg_);
  auto type = NextWord(msg_);

  if (protocol == "nmea2000") return Parse2000(iface, type, msg_);
  if (protocol == "nmea0183") return Parse0183(iface, type, msg_);
  if (protocol == "signalk") return ParseSignalk(iface, type, msg_);
  return {nullptr};
}
