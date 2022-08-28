/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2022 by David S. Register                               *
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

#ifdef __MINGW32__
#undef IPV6STRICT  // mingw FTBS fix:  missing struct ip_mreq
#include <windows.h>
#endif

#include <wx/tokenzr.h>
#include <wx/intl.h>

#include <wx/statline.h>
#include "ConnectionParams.h"
#include "connection_panel.h"

#include "ocpn_plugin.h"
#include "options.h"
#include "connections_dialog.h"
#include "ocpn_frame.h"

#if !wxUSE_XLOCALE && wxCHECK_VERSION(3, 0, 0)
#define wxAtoi(arg) atoi(arg)
#endif


/** A wxStaticText bold label with correct width, see #2538 */
class ConnBoldLabel: public wxStaticText {
public:
  ConnBoldLabel(wxWindow* parent, const wxString& label)
      : wxStaticText(parent, wxID_ANY, "") {
    font = parent->GetFont();
    font.MakeBold();
    SetFont(font);
    SetLabel(label);
    Connect(wxEVT_LEFT_DOWN,
            wxMouseEventHandler(ConnectionParamsPanel::OnSelected), NULL,
            parent);
  }

  void SetLabel(const wxString& label) {
    wxStaticText::SetLabel(label);
    dc.SetFont(font);
    auto size = dc.GetTextExtent(label).Scale(1.1, 1.1);
    SetMinSize(size);
  }

private:
  wxScreenDC dc;
  wxFont font;
};


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
      FurunoGP3X, bEnabled, UserComment.c_str(), AutoSKDiscover,
      socketCAN_port.c_str());

  return ret;
}

ConnectionParams::ConnectionParams() {
  Type = SERIAL;
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

wxString ConnectionParams::GetLastDSPort() const {
  if (Type == SERIAL)
    return wxString::Format(_T("Serial:%s"), Port.c_str());
  else {
    wxString proto = NetworkProtocolToString(LastNetProtocol);
    return wxString::Format(_T("%s:%s:%d"), proto.c_str(),
                            LastNetworkAddress.c_str(), LastNetworkPort);
  }
}


