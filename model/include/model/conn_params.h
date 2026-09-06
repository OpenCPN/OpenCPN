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
 * Connection parameters
 */

#ifndef CONNECTIONPARAMS_H_
#define CONNECTIONPARAMS_H_

#ifndef WX_PRECOMP
#include <wx/arrstr.h>
#include <wx/string.h>
#endif

#include "model/comm_navmsg.h"

#ifndef __DSPORTTYPE_H__
#include "model/ds_porttype.h"
#endif

#define CONN_ENABLE_ID 47621

class ConnectionParams;   // forward
class ConnectionsDialog;  // Indirectly unused FIXME  (leamas) remove

typedef enum {
  SERIAL = 0,
  NETWORK = 1,
  INTERNAL_GPS = 2,
  INTERNAL_BT = 3,
  SOCKETCAN = 4,
  UNKNOWN = 5
} ConnectionType;

typedef enum {
  TCP = 0,
  UDP = 1,
  GPSD = 2,
  SIGNALK = 3,
  PROTO_UNDEFINED = 4
} NetworkProtocol;

typedef enum { WHITELIST = 0, BLACKLIST = 1 } ListType;

typedef enum { FILTER_INPUT = 0, FILTER_OUTPUT = 1 } FilterDirection;

typedef enum {
  PROTO_NMEA0183 = 0,
  PROTO_NMEA2000 = 1,
  PROTO_SIGNALK = 2
} DataProtocol;

class ConnectionParamsPanel;

/**
 * Connection data container close to a POD struct
 */
class ConnectionParams {
public:
  ConnectionParams();
  explicit ConnectionParams(const wxString &config_str);
  ~ConnectionParams();

  ConnectionType type;
  NetworkProtocol net_protocol;
  DataProtocol data_protocol;
  wxString network_address;
  int network_port;
  wxString serial_port;
  wxString socket_can_port;
  PortDirection direction;
  DataProtocol last_data_protocol;
  NetworkProtocol last_net_protocol;

  bool auto_sk_discover;
  bool checksum_check;
  bool disable_echo;
  bool FurunoGP3X;    // Unused placeholder?
  bool GarminUpload;  // Unused placeholder?
  bool is_enabled;
  bool is_garmin;
  bool is_server;
  bool is_setup;
  bool is_valid;
  bool no_data_reconnect;
  int baudrate;
  int last_network_port;
  ListType input_sentence_list_type;
  ListType output_sentence_list_type;
  wxArrayString input_sentence_list;
  wxArrayString output_sentence_list;
  wxString auth_token;
  wxString last_network_address;
  wxString user_comment;

  ConnectionParamsPanel *options_panel;

  /** Return string unique for each instance. */
  [[nodiscard]] std::string GetKey() const;

  [[nodiscard]] wxString Serialize() const;
  void Deserialize(const wxString &config_str);

  /** Return translated name for direction, like _("Input"). */
  [[nodiscard]] wxString GetPortDirectionValueStr() const;

  /**
   * Return port description including for example serial port or
   * network address/port, possibly empty.
   */
  [[nodiscard]] wxString GetDSPort() const;

  /** Return true if port data like com port or network address are sane. */
  [[nodiscard]] bool IsPortValid() const;

  /**
   * Return Last known network "address:protocol:port" for network connections,
   *     else "Serial: <port>"
   */
  [[nodiscard]] std::string GetLastDSPort() const;

  /**
   * Return NavAddr::Bus corresponding to network address/port or
   * serial parameters.
   */
  [[nodiscard]] NavAddr::Bus GetCommProtocol() const;

  /**
   * Return NavAddr::Bus corresponding to last known network address/port or
   *  serial parameters.
   */
  [[nodiscard]] NavAddr::Bus GetLastCommProtocol() const;

  /**
   * Return port string with possible windows extra data removed, in some
   * cases empty.
   */
  [[nodiscard]] std::string GetStrippedDSPort() const;

  /** Return true if given sentence and direction passes connection filters. */
  [[nodiscard]] bool SentencePassesFilter(const wxString &sentence,
                                          FilterDirection direction) const;
};

/** Return global list of connections. */
std::vector<ConnectionParams *> &TheConnectionParams();

#endif
