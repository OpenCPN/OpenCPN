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

// For compilers that support precompilation, includes "wx.h".
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // precompiled headers

#ifdef __MINGW32__
#undef IPV6STRICT  // mingw FTBFS fix:  missing struct ip_mreq
#include <windows.h>
#endif

#include <wx/checklst.h>
#include <wx/combobox.h>
#include <wx/intl.h>
#include <wx/regex.h>
#include <wx/statline.h>
#include <wx/tokenzr.h>

#include "conn_params.h"

#include "ocpn_frame.h"
#include "ocpn_plugin.h"
#include "options.h"

#if !wxUSE_XLOCALE && wxCHECK_VERSION(3, 0, 0)
#define wxAtoi(arg) atoi(arg)
#endif



ConnectionParams::ConnectionParams(const wxString &configStr) {
  m_optionsPanel = NULL;
  Deserialize(configStr);
}

void ConnectionParams::Deserialize(const wxString &configStr) {
  Valid = true;
  wxArrayString prms = wxStringTokenize(configStr, _T(";"));
  if (prms.Count() < 18) {
    Valid = false;
    return;
  }

  Type = (ConnectionType)wxAtoi(prms[0]);
  NetProtocol = (NetworkProtocol)wxAtoi(prms[1]);
  NetworkAddress = prms[2];
  NetworkPort = (ConnectionType)wxAtoi(prms[3]);
  Protocol = (DataProtocol)wxAtoi(prms[4]);
  Port = prms[5];
  Baudrate = wxAtoi(prms[6]);
  ChecksumCheck = wxAtoi(prms[7]);
  int iotval = wxAtoi(prms[8]);
  IOSelect = ((iotval <= 2) ? static_cast<dsPortType>(iotval) : DS_TYPE_INPUT);
  InputSentenceListType = (ListType)wxAtoi(prms[9]);
  InputSentenceList = wxStringTokenize(prms[10], _T(","));
  OutputSentenceListType = (ListType)wxAtoi(prms[11]);
  OutputSentenceList = wxStringTokenize(prms[12], _T(","));
  Priority = wxAtoi(prms[13]);
  Garmin = !!wxAtoi(prms[14]);
  GarminUpload = !!wxAtoi(prms[15]);
  FurunoGP3X = !!wxAtoi(prms[16]);

  bEnabled = true;
  LastNetworkPort = 0;
  b_IsSetup = false;
  if (prms.Count() >= 18) {
    bEnabled = !!wxAtoi(prms[17]);
  }
  if (prms.Count() >= 19) {
    UserComment = prms[18];
  }
  if (prms.Count() >= 20) {
    AutoSKDiscover = !!wxAtoi(prms[19]);
  }
  if (prms.Count() >= 21) {
    socketCAN_port = prms[20];
  }
}

wxString ConnectionParams::Serialize() const {
  wxString istcs;
  for (size_t i = 0; i < InputSentenceList.Count(); i++) {
    if (i > 0) istcs.Append(_T(","));
    istcs.Append(InputSentenceList[i]);
  }
  wxString ostcs;
  for (size_t i = 0; i < OutputSentenceList.Count(); i++) {
    if (i > 0) ostcs.Append(_T(","));
    ostcs.Append(OutputSentenceList[i]);
  }
  wxString ret = wxString::Format(
      _T("%d;%d;%s;%d;%d;%s;%d;%d;%d;%d;%s;%d;%s;%d;%d;%d;%d;%d;%s;%d;%s"), Type,
      NetProtocol, NetworkAddress.c_str(), NetworkPort, Protocol, Port.c_str(),
      Baudrate, ChecksumCheck, IOSelect, InputSentenceListType, istcs.c_str(),
      OutputSentenceListType, ostcs.c_str(), Priority, Garmin, GarminUpload,
      FurunoGP3X, bEnabled, UserComment.c_str(), AutoSKDiscover, socketCAN_port.c_str());

  return ret;
}

ConnectionParams::ConnectionParams() {
  Type = UNKNOWN;
  NetProtocol = TCP;
  NetworkAddress = wxEmptyString;
  NetworkPort = 0;
  Protocol = PROTO_NMEA0183;
  Port = wxEmptyString;
  Baudrate = 4800;
  ChecksumCheck = true;
  Garmin = false;
  FurunoGP3X = false;
  IOSelect = DS_TYPE_INPUT;
  InputSentenceListType = WHITELIST;
  OutputSentenceListType = WHITELIST;
  Priority = 0;
  Valid = true;
  bEnabled = true;
  b_IsSetup = false;
  m_optionsPanel = NULL;
  AutoSKDiscover = false;
}

ConnectionParams::~ConnectionParams() {
  // delete m_optionsPanel;
}

wxString ConnectionParams::GetSourceTypeStr() const {
  switch (Type) {
    case SERIAL:
      return _("Serial");
    case NETWORK:
      return _("Network");
    case INTERNAL_GPS:
      return _("GPS");
    case INTERNAL_BT:
      return _("BT");
    default:
      return _T("");
  }
}

wxString ConnectionParams::GetAddressStr() const {
  if (Type == SERIAL)
    return wxString::Format(_T("%s"), Port.c_str());
  else if (Type == NETWORK)
    return wxString::Format(_T("%s:%d"), NetworkAddress.c_str(), NetworkPort);
  else if (Type == INTERNAL_GPS)
    return _("Internal");
  else if (Type == INTERNAL_BT)
    return NetworkAddress;
  else
    return _T("");
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

wxString ConnectionParams::GetParametersStr() const {
  switch (Type) {
    case SERIAL:
      return wxString::Format(_T("%d"), Baudrate);
    case NETWORK:
      return NetworkProtocolToString(NetProtocol);
    case INTERNAL_GPS:
      return _T("GPS");
    case INTERNAL_BT:
      return Port;
    default:
      return _T("");
  }
}

wxString ConnectionParams::GetIOTypeValueStr() const {
  if (IOSelect == DS_TYPE_INPUT)
    return _("Input");
  else if (IOSelect == DS_TYPE_OUTPUT)
    return _("Output");
  else
    return _("In/Out");
}

wxString ConnectionParams::FilterTypeToStr(ListType type,
                                           FilterDirection dir) const {
  if (dir == FILTER_INPUT) {
    if (type == BLACKLIST)
      return _("Reject");
    else
      return _("Accept");
  } else {
    if (type == BLACKLIST)
      return _("Drop");
    else
      return _("Send");
  }
}

wxString ConnectionParams::GetFiltersStr() const {
  wxString istcs;
  for (size_t i = 0; i < InputSentenceList.Count(); i++) {
    if (i > 0) istcs.Append(_T(","));
    istcs.Append(InputSentenceList[i]);
  }
  wxString ostcs;
  for (size_t i = 0; i < OutputSentenceList.Count(); i++) {
    if (i > 0) ostcs.Append(_T(","));
    ostcs.Append(OutputSentenceList[i]);
  }
  wxString ret = wxEmptyString;
  if (istcs.Len() > 0) {
    ret.Append(_("In"));
    ret.Append(wxString::Format(
        _T(": %s %s"),
        FilterTypeToStr(InputSentenceListType, FILTER_INPUT).c_str(),
        istcs.c_str()));
  } else
    ret.Append(_("In: None"));

  if (ostcs.Len() > 0) {
    ret.Append(_T(", "));
    ret.Append(_("Out"));
    ret.Append(wxString::Format(
        _T(": %s %s"),
        FilterTypeToStr(OutputSentenceListType, FILTER_OUTPUT).c_str(),
        ostcs.c_str()));
  } else
    ret.Append(_(", Out: None"));
  return ret;
}

wxString ConnectionParams::GetDSPort() const {
  if (Type == SERIAL)
    return wxString::Format(_T("Serial:%s"), Port.c_str());
  else if (Type == NETWORK) {
    wxString proto = NetworkProtocolToString(NetProtocol);
    return wxString::Format(_T("%s:%s:%d"), proto.c_str(),
                            NetworkAddress.c_str(), NetworkPort);
  } else if (Type == INTERNAL_BT) {
    return Port;  // mac
  } else
    return _T("");
}

std::string ConnectionParams::GetStrippedDSPort() {
  if (Type == SERIAL){
    wxString t = wxString::Format(_T("Serial:%s"), Port.c_str());
    wxString comx = t.AfterFirst(':').BeforeFirst(' ');
    return comx.ToStdString();
  }
  else if (Type == NETWORK) {
    wxString proto = NetworkProtocolToString(NetProtocol);
    wxString t = wxString::Format(_T("%s:%s:%d"), proto.c_str(),
                            NetworkAddress.c_str(), NetworkPort);
    return t.ToStdString();

  } else if (Type == SOCKETCAN) {
    return "socketCAN";
    //FIXME (dave)
    //wxString proto = NetworkProtocolToString(NetProtocol);
    //wxString t = wxString::Format(_T("%s:%s:%d"), proto.c_str(),
    //                        NetworkAddress.c_str(), NetworkPort);
    //return t.ToStdString();

  } else if (Type == INTERNAL_BT) {
    return Port.ToStdString();
  } else
    return "";
}

std::string ConnectionParams::GetLastDSPort() const {
  if (Type == SERIAL){
    wxString sp = wxString::Format(_T("Serial:%s"), Port.c_str());
    return sp.ToStdString();
  }
  else {
    wxString proto = NetworkProtocolToString(LastNetProtocol);
    wxString sp = wxString::Format(_T("%s:%s:%d"), proto.c_str(),
                            LastNetworkAddress.c_str(), LastNetworkPort);
    return sp.ToStdString();
  }
}

bool ConnectionParams::SentencePassesFilter(const wxString& sentence, FilterDirection direction)
{
    wxArrayString filter;
    bool listype = false;

    if (direction == FILTER_INPUT)
    {
        filter = InputSentenceList;
        if (InputSentenceListType == WHITELIST)
            listype = true;
    }
    else
    {
        filter = OutputSentenceList;
        if (OutputSentenceListType == WHITELIST)
            listype = true;
    }
    if (filter.Count() == 0) //Empty list means everything passes
        return true;

    wxString fs;
    for (size_t i = 0; i < filter.Count(); i++)
    {
        fs = filter[i];
        switch (fs.Length())
        {
            case 2:
                if (fs == sentence.Mid(1, 2))
                    return listype;
                break;
            case 3:
                if (fs == sentence.Mid(3, 3))
                    return listype;
                break;
            case 5:
                if (fs == sentence.Mid(1, 5))
                    return listype;
                break;
            default:
	        // TODO: regex patterns like ".GPZ.." or 6-character patterns
		//       are rejected in the connection settings dialogue currently
		//       experts simply edit .opencpn/opncpn.config
                wxRegEx  re(fs);
                if (re.Matches(sentence.Mid(0, 8)))
                {
                    return listype;
                }
                break;
        }
    }
    return !listype;
}

NavAddr::Bus ConnectionParams::GetCommProtocol(){
  if (Type == NETWORK){
    if (NetProtocol == SIGNALK)
      return NavAddr::Bus::Signalk;
    else if (NetProtocol == UDP)
      return NavAddr::Bus::N0183;
    else if (NetProtocol == TCP)
      return NavAddr::Bus::N0183;
    else if (NetProtocol == GPSD)
      return NavAddr::Bus::N0183;
  }

  switch (Protocol){
    case PROTO_NMEA0183:
      return NavAddr::Bus::N0183;
    case PROTO_NMEA2000:
      return NavAddr::Bus::N2000;
    default:
      return NavAddr::Bus::Undef;
  }
}

NavAddr::Bus ConnectionParams::GetLastCommProtocol(){
   if (Type == NETWORK){
    if (LastNetProtocol == SIGNALK)
      return NavAddr::Bus::Signalk;
    else if (LastNetProtocol == UDP)
      return NavAddr::Bus::N0183;
    else if (LastNetProtocol == TCP)
      return NavAddr::Bus::N0183;
    else if (LastNetProtocol == GPSD)
      return NavAddr::Bus::N0183;
  }

  switch (LastDataProtocol){
    case PROTO_NMEA0183:
      return NavAddr::Bus::N0183;
    case PROTO_NMEA2000:
      return NavAddr::Bus::N2000;
    default:
      return NavAddr::Bus::Undef;
  }
}


