
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

/**
 * \file
 * Implement monitor_filter.h
 */

#include <fstream>

#include "model/base_platform.h"
#include "model/navmsg_filter.h"

#include "wx/jsonreader.h"
#include "wx/jsonwriter.h"
#include "wx/log.h"

#include "std_filesystem.h"

using Direction = NavmsgStatus::Direction;
using State = NavmsgStatus::State;
using Accepted = NavmsgStatus::Accepted;

static const std::unordered_map<Accepted, std::string> StringByAccepted = {
    {Accepted::kOk, "Ok"},
    {Accepted::kFilteredNoOutput, "FilteredNoOutput"},
    {Accepted::kFilteredDropped, "FilteredDropped"},
    {Accepted::kNone, "None"}};

std::string NavmsgStatus::AcceptedToString(Accepted a) {
  try {
    return StringByAccepted.at(a);
  } catch (std::out_of_range& e) {
    std::cout << "Error: " << e.what() << " :" << static_cast<int>(a) << "\n";
    assert(false && "Bad Accepted state");
    return "";  // For the compiler
  }
}

Accepted NavmsgStatus::StringToAccepted(const std::string& s) {
  for (auto& kv : StringByAccepted)
    if (kv.second == s) return kv.first;
  return Accepted::kNone;
}

// clang-format: off
static const std::unordered_map<const char*, Direction> dir_map = {
    {"input", Direction::kInput},
    {"handled", Direction::kHandled},
    {"output", Direction::kOutput},
    {"internal", Direction::kInternal},
    {"none", NavmsgStatus::Direction::kNone}};

static const std::unordered_map<const char*, State> statemap = {
    {"ok", State::kOk},
    {"checksum-err", State::kChecksumError},
    {"malformed", State::kMalformed},
    {"tx-error", State::kTxError}};

static const std::unordered_map<const char*, Accepted> acceptmap = {
    {"Ok", Accepted::kOk},
    {"FilteredDropped", Accepted::kFilteredDropped},
    {"FilteredNoOutput", Accepted::kFilteredNoOutput},
    {"None", Accepted::kNone}};  // clang-format: on

static NavmsgStatus::Direction StringToDirection(const std::string s) {
  // Transition from 5.12 beta which used "received" isf "handled"; to
  // be removed
  if (s == "received") return NavmsgStatus::Direction::kHandled;

  for (auto kv : dir_map)
    if (kv.first == s) return kv.second;
  return NavmsgStatus::Direction::kNone;
}

static std::string DirectionToString(Direction direction) {
  for (auto kv : dir_map)
    if (kv.second == direction) return kv.first;
  assert(false && "Illegal direction enumeration type (!)");
  return "";  // for the compiler
}

static Accepted StringToAccepted(const std::string s) {
  for (auto kv : acceptmap)
    if (kv.first == s) return kv.second;
  return Accepted::kNone;
}

static std::string AcceptedToString(Accepted accept) {
  for (auto kv : acceptmap)
    if (kv.second == accept) return kv.first;
  assert(false && "Illegal direction enumeration type (!)");
  return "";  // for the compiler
}

static State StringToState(const std::string s) {
  for (auto kv : statemap)
    if (kv.first == s) return kv.second;
  return State::kNone;
}

static std::string StateToString(State state) {
  for (auto kv : statemap)
    if (kv.second == state) return kv.first;
  assert(false && "Illegal direction enumeration type (!)");
  return "";  // for the compiler
}

static void ParseBuses(NavmsgFilter& filter, wxJSONValue json_val) {
  for (int i = 0; i < json_val.Size(); i++) {
    auto str = json_val[i].AsString().ToStdString();
    filter.buses.insert(NavAddr::StringToBus(str));
  }
}

static void ParseDirections(NavmsgFilter& filter, wxJSONValue json_val) {
  for (int i = 0; i < json_val.Size(); i++) {
    auto str = json_val[i].AsString().ToStdString();
    filter.directions.insert(StringToDirection(str));
  }
}

static void ParseAccepted(NavmsgFilter& filter, wxJSONValue json_val) {
  for (int i = 0; i < json_val.Size(); i++) {
    auto str = json_val[i].AsString().ToStdString();
    filter.accepted.insert(StringToAccepted(str));
  }
}

static void ParseStatus(NavmsgFilter& filter, wxJSONValue json_val) {
  for (int i = 0; i < json_val.Size(); i++) {
    auto str = json_val[i].AsString().ToStdString();
    filter.status.insert(StringToState(str));
  }
}

static void ParseMsgFilter(NavmsgFilter& filter, wxJSONValue json_val) {
  if (json_val.HasMember("blockedMsg")) {
    auto val = json_val["blockedMsg"];
    for (int i = 0; i < val.Size(); i++)
      filter.exclude_msg.insert(val[i].AsString().ToStdString());
  } else if (json_val.HasMember("allowedMsg")) {
    auto val = json_val["allowedMsg"];
    for (int i = 0; i < val.Size(); i++)
      filter.include_msg.insert(val[i].AsString().ToStdString());
  }
}

static void ParseInterfaces(NavmsgFilter& filter, wxJSONValue json_val) {
  for (int i = 0; i < json_val.Size(); i++)
    filter.interfaces.insert(json_val[i].AsString().ToStdString());
}

static void ParsePgn(NavmsgFilter& filter, wxJSONValue json_val) {
  try {
    for (int i = 0; i < json_val.Size(); i++)
      filter.pgns.insert(std::stoi(json_val[i].AsString().ToStdString()));
  } catch (...) {
    return;
  }
}

static void ParseSource(NavmsgFilter& filter, wxJSONValue json_val) {
  try {
    for (int i = 0; i < json_val.Size(); i++) {
      filter.src_pgns.insert(std::stoi(json_val[i].AsString().ToStdString()));
    }
  } catch (...) {
    return;
  }
}

std::vector<NavmsgFilter> NavmsgFilter::GetFilters(const fs::path& dir) {
  std::vector<NavmsgFilter> filters;
  try {
    for (auto& entry : fs::directory_iterator(dir)) {
      auto filter = NavmsgFilter::Parse(entry);
      if (filter.m_is_valid)
        filters.push_back(filter);
      else
        wxLogWarning("Illegal system filter: %s",
                     entry.path().string().c_str());
    }
  } catch (...) {
    wxLogWarning("Bad system filter path: %s", dir.string().c_str());
  }
  return filters;
}

std::vector<NavmsgFilter> NavmsgFilter::GetSystemFilters() {
  fs::path dirpath(g_BasePlatform->GetSharedDataDir().ToStdString());
  return NavmsgFilter::GetFilters(dirpath / "filters");
}

std::vector<NavmsgFilter> GetUserFilters() {
  fs::path dirpath(g_BasePlatform->GetPrivateDataDir().ToStdString());
  return NavmsgFilter::GetFilters(dirpath / "filters");
}

std::vector<NavmsgFilter> NavmsgFilter::GetAllFilters() {
  std::vector<NavmsgFilter> filters = GetSystemFilters();
  std::vector user_filters = GetUserFilters();
  filters.insert(filters.end(), user_filters.begin(), user_filters.end());
  return filters;
}

bool NavmsgFilter::Pass(NavmsgStatus msg_status,
                        const std::shared_ptr<const NavMsg>& msg) {
  if (directions.size() > 0) {
    if (directions.find(msg_status.direction) == directions.end()) return false;
  }
  if (status.size() > 0) {
    if (status.find(msg_status.status) == status.end()) return false;
  }
  if (buses.size() > 0) {
    if (buses.find(msg->bus) == buses.end()) return false;
  }
  if (include_msg.size() > 0) {
    if (include_msg.find(msg->key()) == include_msg.end()) return false;
  }
  if (exclude_msg.size() > 0) {
    if (exclude_msg.find(msg->key()) != exclude_msg.end()) return false;
  }
  if (interfaces.size() > 0) {
    if (interfaces.find(msg->source->iface) == interfaces.end()) return false;
  }
  if (accepted.size() > 0) {
    if (accepted.find(msg_status.accepted) == accepted.end()) return false;
  }
  auto n2k_msg = std::dynamic_pointer_cast<const Nmea2000Msg>(msg);
  if (n2k_msg) {
    if (pgns.size() > 0) {
      if (pgns.find(n2k_msg->PGN) == pgns.end()) return false;
    }
    if (src_pgns.size() > 0) {
      auto src = std::static_pointer_cast<const NavAddr2000>(msg->source);
      if (src && src_pgns.find(src->name) == src_pgns.end()) return false;
    }
  }
  return true;
}

NavmsgFilter NavmsgFilter::Parse(const fs::path& path) {
  std::ifstream stream(path);
  if (!stream.is_open()) return NavmsgFilter(false);
  std::stringstream ss;
  ss << stream.rdbuf();
  return NavmsgFilter::Parse(ss.str());
}

NavmsgFilter NavmsgFilter::Parse(const std::string& string) {
  wxJSONValue root;
  wxJSONReader reader;
  int err_count = reader.Parse(string, &root);
  if (err_count > 0) {
    wxLogWarning("Error parsing filter XML");
    for (auto& e : reader.GetErrors())
      wxLogWarning("Parse error: %s", e.c_str());
    return NavmsgFilter(false);
  }
  NavmsgFilter filter;
  filter.m_name = root["filter"]["name"].AsString();
  filter.m_description = root["filter"]["description"].AsString();
  if (root["filter"].HasMember("buses"))
    ParseBuses(filter, root["filter"]["buses"]);
  if (root["filter"].HasMember("accepted"))
    ParseAccepted(filter, root["filter"]["accepted"]);
  if (root["filter"].HasMember("status"))
    ParseStatus(filter, root["filter"]["status"]);
  if (root["filter"].HasMember("directions"))
    ParseDirections(filter, root["filter"]["directions"]);
  if (root["filter"].HasMember("msgFilter"))
    ParseMsgFilter(filter, root["filter"]["msgFilter"]);
  if (root["filter"].HasMember("interfaces"))
    ParseInterfaces(filter, root["filter"]["interfaces"]);
  if (root["filter"].HasMember("pgns"))
    ParsePgn(filter, root["filter"]["pgns"]);
  if (root["filter"].HasMember("src_pgns"))
    ParseSource(filter, root["filter"]["src_pgns"]);
  return filter;
}

std::string NavmsgFilter::to_string() const {
  wxJSONValue root;
  root["filter"]["name"] = m_name;
  root["filter"]["description"] = m_description;
  wxJSONValue& filter = root["filter"];

  if (!buses.empty()) {
    for (auto b : buses) filter["buses"].Append(NavAddr::BusToString(b));
  };
  if (!directions.empty()) {
    for (auto d : directions) filter["directions"].Append(DirectionToString(d));
  };
  if (!accepted.empty()) {
    for (auto a : accepted) filter["accepted"].Append(AcceptedToString(a));
  };
  if (!status.empty()) {
    for (auto s : status) filter["status"].Append(StateToString(s));
  };
  if (!include_msg.empty()) {
    for (auto m : include_msg) filter["msgFilter"]["allowedMsg"].Append(m);
  } else if (!exclude_msg.empty()) {
    for (auto m : exclude_msg) filter["msgFilter"]["blockedMsg"].Append(m);
  }
  if (!interfaces.empty()) {
    for (auto i : interfaces) filter["interfaces"].Append(i);
  }
  if (!pgns.empty()) {
    for (auto p : pgns) filter["pgns"].Append(p.to_string());
  }
  if (!src_pgns.empty()) {
    for (auto p : src_pgns) filter["src_pgns"].Append(p.to_string());
  }
  wxJSONWriter writer;
  wxString ws;
  writer.Write(root, ws);
  return ws.ToStdString();
}
