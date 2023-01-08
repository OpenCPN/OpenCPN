/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 **************************************************************************/

#ifndef _CONNECTIONPARAMS_H__
#define _CONNECTIONPARAMS_H__

#include <wx/wxprec.h>

#ifndef __DSPORTTYPE_H__
#include <dsPortType.h>
#endif


#ifndef WX_PRECOMP
#include <wx/arrstr.h>
#include <wx/dynarray.h>
#include <wx/string.h>
#endif  // precompiled headers

#include "comm_navmsg.h"

class ConnectionParams;
class ConnectionsDialog;

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

#define CONN_ENABLE_ID 47621

class ConnectionParamsPanel;

class ConnectionParams {
public:
  ConnectionParams();
  ~ConnectionParams();
  ConnectionParams(const wxString &configStr);

  ConnectionType Type;
  NetworkProtocol NetProtocol;
  wxString NetworkAddress;
  int NetworkPort;

  wxString LastNetworkAddress;
  int LastNetworkPort;
  NetworkProtocol LastNetProtocol;
  DataProtocol LastDataProtocol;

  DataProtocol Protocol;
  wxString Port;
  wxString socketCAN_port;
  int Baudrate;
  bool ChecksumCheck;
  bool Garmin;
  bool GarminUpload;
  bool FurunoGP3X;
  bool AutoSKDiscover;
  dsPortType IOSelect;
  ListType InputSentenceListType;
  wxArrayString InputSentenceList;
  ListType OutputSentenceListType;
  wxArrayString OutputSentenceList;
  int Priority;
  bool bEnabled;
  wxString UserComment;

  wxString Serialize() const;
  void Deserialize(const wxString &configStr);

  wxString GetSourceTypeStr() const;
  wxString GetAddressStr() const;
  wxString GetParametersStr() const;
  wxString GetIOTypeValueStr() const;
  wxString GetFiltersStr() const;
  wxString GetDSPort() const;
  std::string GetLastDSPort() const;
  NavAddr::Bus GetLastCommProtocol();
  wxString GetPortStr() const { return Port; }
  void SetPortStr(wxString str) { Port = str; }
  std::string GetStrippedDSPort();
  NavAddr::Bus GetCommProtocol();

  bool SentencePassesFilter(const wxString& sentence, FilterDirection direction);

  bool Valid;
  bool b_IsSetup;
  ConnectionParamsPanel *m_optionsPanel;

private:
  wxString FilterTypeToStr(ListType type, FilterDirection dir) const;
};

WX_DEFINE_ARRAY(ConnectionParams *, wxArrayOfConnPrm);

#endif
