/***************************************************************************
 *   Copyright (C) 2013 by David S. Register                               *
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
 * \file
 *
 * Implement conn_params.h -- connection parameters
 */

#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/arrstr.h>
#include <wx/intl.h>
#include <wx/regex.h>
#include <wx/string.h>
#include <wx/tokenzr.h>

#include "model/conn_params.h"

#include "ocpn_plugin.h"

#if !wxUSE_XLOCALE && wxCHECK_VERSION(3, 0, 0)
#define wxAtoi(arg) atoi(arg)
#endif

static std::vector<ConnectionParams*> the_connection_params;

std::vector<ConnectionParams*>& TheConnectionParams() {
  return the_connection_params;
}

// TODO: Make part of NetworkProtocol interface
static wxString NetworkProtocolToString(NetworkProtocol NetProtocol) {
  switch (NetProtocol) {
    case TCP:
      return _("TCP");
    case UDP:
      return _("UDP");
    case GPSD:
      return _("GPSD");
    case SIGNALK:
      return _("Signal K");
    default:
      return _("Undefined");
  }
}

ConnectionParams::ConnectionParams(const wxString& ws) : ConnectionParams() {
  Deserialize(ws);
}

ConnectionParams::ConnectionParams()
    : type(UNKNOWN),
      net_protocol(TCP),
      data_protocol(PROTO_NMEA0183),
      network_port(0),
      direction(PortDirection::kInput),
      last_data_protocol(PROTO_NMEA0183),
      last_net_protocol(TCP),
      auto_sk_discover(false),
      checksum_check(true),
      disable_echo(false),
      FurunoGP3X(false),
      GarminUpload(false),
      is_enabled(true),
      is_garmin(false),
      is_server(false),
      is_setup(false),
      is_valid(true),
      no_data_reconnect(false),
      baudrate(4800),
      last_network_port(0),
      input_sentence_list_type(WHITELIST),
      output_sentence_list_type(WHITELIST),
      input_sentence_list(WHITELIST),
      output_sentence_list(WHITELIST),
      options_panel(nullptr) {}

ConnectionParams::~ConnectionParams() = default;

void ConnectionParams::Deserialize(const wxString& configStr) {
  is_valid = true;
  wxArrayString prms = wxStringTokenize(configStr, ";");
  if (prms.Count() < 18) {
    is_valid = false;
    return;
  }

  type = static_cast<ConnectionType>(wxAtoi(prms[0]));
  net_protocol = static_cast<NetworkProtocol>(wxAtoi(prms[1]));
  network_address = prms[2];
  network_port = static_cast<ConnectionType>(wxAtoi(prms[3]));
  data_protocol = static_cast<DataProtocol>(wxAtoi(prms[4]));
  serial_port = prms[5];
  baudrate = wxAtoi(prms[6]);
  checksum_check = wxAtoi(prms[7]);
  int iotval = wxAtoi(prms[8]);
  direction =
      iotval <= 2 ? static_cast<PortDirection>(iotval) : PortDirection::kInput;
  input_sentence_list_type = static_cast<ListType>(wxAtoi(prms[9]));
  input_sentence_list = wxStringTokenize(prms[10], ",");
  output_sentence_list_type = static_cast<ListType>(wxAtoi(prms[11]));
  output_sentence_list = wxStringTokenize(prms[12], ",");
  is_garmin = !!wxAtoi(prms[14]);
  GarminUpload = !!wxAtoi(prms[15]);
  FurunoGP3X = !!wxAtoi(prms[16]);

  is_enabled = true;
  last_network_port = 0;
  is_setup = false;
  if (prms.Count() >= 18) {
    is_enabled = !!wxAtoi(prms[17]);
  }
  if (prms.Count() >= 19) {
    user_comment = prms[18];
  }
  if (prms.Count() >= 20) {
    auto_sk_discover = !!wxAtoi(prms[19]);
  }
  if (prms.Count() >= 21) {
    socket_can_port = prms[20];
  }
  if (prms.Count() >= 22) {
    no_data_reconnect = wxAtoi(prms[21]);
  }
  if (prms.Count() >= 23) {
    disable_echo = wxAtoi(prms[22]);
  }
  if (prms.Count() >= 24) {
    auth_token = prms[23];
  }
}

wxString ConnectionParams::Serialize() const {
  wxString istcs;
  for (size_t i = 0; i < input_sentence_list.Count(); i++) {
    if (i > 0) istcs.Append(",");
    istcs.Append(input_sentence_list[i]);
  }
  wxString ostcs;
  for (size_t i = 0; i < output_sentence_list.Count(); i++) {
    if (i > 0) ostcs.Append(",");
    ostcs.Append(output_sentence_list[i]);
  }
  wxString ret = wxString::Format(
      "%d;%d;%s;%d;%d;%s;%d;%d;%d;%d;%s;%d;%s;%d;%d;%d;%d;%d;%s;%d;%s;%d;%d;%s",
      type, net_protocol, network_address.c_str(), network_port, data_protocol,
      serial_port.c_str(), baudrate, checksum_check,
      static_cast<int>(direction), input_sentence_list_type, istcs.c_str(),
      output_sentence_list_type, ostcs.c_str(), 0 /* Priority */, is_garmin,
      GarminUpload, FurunoGP3X, is_enabled, user_comment.c_str(),
      auto_sk_discover, socket_can_port.c_str(), no_data_reconnect,
      disable_echo, auth_token.c_str());

  return ret;
}

std::string ConnectionParams::GetKey() const {
  std::stringstream ss;
  ss << type << net_protocol << network_address << network_port << data_protocol
     << serial_port << baudrate << checksum_check << static_cast<int>(direction)
     << input_sentence_list_type << output_sentence_list_type << is_garmin
     << user_comment << auto_sk_discover << socket_can_port << no_data_reconnect
     << disable_echo << auth_token;
  for (const auto& sentence : output_sentence_list) ss << sentence;
  for (const auto& sentence : input_sentence_list) ss << sentence;
  return ss.str();
}

wxString ConnectionParams::GetPortDirectionValueStr() const {
  static const std::unordered_map<PortDirection, wxString> StringByDirection = {
      {PortDirection::kInput, _("Input")},
      {PortDirection::kOutput, _("Output")},
      {PortDirection::kInOut, _("InOut")},
      {PortDirection::kUpload, _("Upload")}};
  if (static_cast<size_t>(direction) >= StringByDirection.size()) return "???";
  return StringByDirection.at(direction);
}

wxString ConnectionParams::GetDSPort() const {
  if (type == SERIAL)
    return wxString::Format("Serial:%s", serial_port.c_str());
  else if (type == NETWORK) {
    wxString proto = NetworkProtocolToString(net_protocol);
    return wxString::Format("%s:%s:%d", proto.c_str(), network_address.c_str(),
                            network_port);
  } else if (type == INTERNAL_BT) {
    return serial_port;  // mac
  } else
    return "";
}

bool ConnectionParams::IsPortValid() const {
  if (type == SERIAL && serial_port.empty()) return false;
  if (type == NETWORK && (network_address.empty() || !network_port))
    return false;
  if (type == INTERNAL_BT && serial_port.empty()) return false;
  return true;
}

std::string ConnectionParams::GetStrippedDSPort() const {
  if (type == SERIAL) {
    wxString t = wxString::Format("Serial:%s", serial_port.c_str());
    wxString comx = t.AfterFirst(':').BeforeFirst(' ');
    return comx.ToStdString();
  }
  if (type == NETWORK) {
    wxString proto = NetworkProtocolToString(net_protocol);
    wxString t = wxString::Format("%s:%s:%d", proto.c_str(),
                                  network_address.c_str(), network_port);
    return t.ToStdString();
  }
  if (type == SOCKETCAN) {
    std::string rv;
    if (!socket_can_port.ToStdString().empty())
      rv += "socketCAN-" + socket_can_port.ToStdString();
    return rv;
  }
  if (type == INTERNAL_BT) {
    return serial_port.ToStdString();
  }
  if (type == INTERNAL_GPS) {
    return serial_port.ToStdString();
  }
  return "";
}

std::string ConnectionParams::GetLastDSPort() const {
  if (type == SERIAL) {
    wxString sp = wxString::Format("Serial:%s", serial_port.c_str());
    return sp.ToStdString();
  } else {
    wxString proto = NetworkProtocolToString(last_net_protocol);
    wxString sp =
        wxString::Format("%s:%s:%d", proto.c_str(),
                         last_network_address.c_str(), last_network_port);
    return sp.ToStdString();
  }
}

bool ConnectionParams::SentencePassesFilter(const wxString& sentence,
                                            FilterDirection direction) const {
  wxArrayString filter;
  bool listype = false;

  if (direction == FILTER_INPUT) {
    filter = input_sentence_list;
    if (input_sentence_list_type == WHITELIST) listype = true;
  } else {
    filter = output_sentence_list;
    if (output_sentence_list_type == WHITELIST) listype = true;
  }
  if (filter.Count() == 0)  // Empty list means everything passes
    return true;

  for (size_t i = 0; i < filter.Count(); i++) {
    const wxString& fs = filter[i];
    switch (fs.Length()) {
      case 2:
        if (fs == sentence.Mid(1, 2)) return listype;
        break;
      case 3:
        if (fs == sentence.Mid(3, 3)) return listype;
        break;
      case 5:
        if (fs == sentence.Mid(1, 5)) return listype;
        break;
      default:
        // TODO: regex patterns like ".GPZ.." or 6-character patterns
        //       are rejected in the connection settings dialogue currently
        //       experts simply edit .opencpn/opencpn.config
        wxRegEx re(fs);
        if (re.Matches(sentence.Mid(0, 8))) {
          return listype;
        }
        break;
    }
  }
  return !listype;
}

NavAddr::Bus ConnectionParams::GetCommProtocol() const {
  if (type == NETWORK) {
    if (net_protocol == SIGNALK)
      return NavAddr::Bus::Signalk;
    else if (net_protocol == GPSD)
      return NavAddr::Bus::N0183;
  }
  switch (data_protocol) {
    case PROTO_NMEA0183:
      return NavAddr::Bus::N0183;
    case PROTO_NMEA2000:
      return NavAddr::Bus::N2000;
    default:
      return NavAddr::Bus::Undef;
  }
}

NavAddr::Bus ConnectionParams::GetLastCommProtocol() const {
  if (type == NETWORK) {
    if (last_net_protocol == SIGNALK)
      return NavAddr::Bus::Signalk;
    else if (last_net_protocol == GPSD)
      return NavAddr::Bus::N0183;
  }
  switch (last_data_protocol) {
    case PROTO_NMEA0183:
      return NavAddr::Bus::N0183;
    case PROTO_NMEA2000:
      return NavAddr::Bus::N2000;
    default:
      return NavAddr::Bus::Undef;
  }
}
