/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Configuration wizard
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010-2024 by David S. Register                          *
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
 ***************************************************************************
 *
 *
 *
 */

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // precompiled headers

#include <regex>
#include <wx/sckaddr.h>
#include <wx/socket.h>
#include "wiz_ui.h"
#include "OCPNPlatform.h"
#include "model/comm_drv_signalk_net.h"
#include "model/conn_params.h"
#include "model/logger.h"
#include "model/mDNS_query.h"
#include "navutil.h"
#include "svg_utils.h"
#ifndef __ANDROID__
#include "serial/serial.h"
#include "dnet.h"
#endif

extern OCPNPlatform* g_Platform;
extern std::vector<ocpn_DNS_record_t> g_sk_servers;

FirstUseWizImpl::FirstUseWizImpl(wxWindow* parent, wxWindowID id,
                                 const wxString& title, const wxBitmap& bitmap,
                                 const wxPoint& pos, long style)
    : FirstUseWiz(parent, id, title, bitmap, pos, style) {
  wxString svgDir = g_Platform->GetSharedDataDir() + _T("uidata") +
                    wxFileName::GetPathSeparator() + "MUI_flat" +
                    wxFileName::GetPathSeparator();
  auto settings_icon = LoadSVG(svgDir + "MUI_settings.svg", 32, 32);

  // Units
  m_rtLangUnitInfo->Clear();
  m_rtLangUnitInfo->WriteText(
      _("Select the units and data formats you would like to use."));
  m_rtLangUnitInfo->Newline();
  m_rtLangUnitInfo->Newline();
  m_rtLangUnitInfo->WriteText(
      _("All the settings can be changed at any time in the configuration "
        "Toolbox accessible by clicking on the "));
  m_rtLangUnitInfo->WriteImage(settings_icon.ConvertToImage());
  m_rtLangUnitInfo->WriteText(_(" icon in the main Toolbar."));
  // Connections

  // Charts
  m_rtChartDirInfo->Clear();
  m_rtChartDirInfo->WriteText(
      _("If you already have charts on your system, you may add them here."));
  m_rtChartDirInfo->Newline();
  m_rtChartDirInfo->WriteText(
      _("Additional charts can be obtained  using the Chart Downloader "
        "integrated in the application."));
  m_rtChartDirInfo->Newline();
  m_rtChartDirInfo->Newline();
  m_rtChartDirInfo->WriteText(
      _("To access the Chart Downloader click on the "));
  m_rtChartDirInfo->WriteImage(settings_icon.ConvertToImage());
  m_rtChartDirInfo->WriteText(
      _(" icon in the main Toolbar and navigate to Charts -> Chart Downloader "
        "tab."));
  // Quick start guide
  m_htmlWinFinish->SetPage(
      _("<html><body><h1>Welcome to OpenCPN!</h1><p>You have successfully "
        "completed the initial configuration. You can now start using the "
        "application.</p></body></html>"));
}

FirstUseWizImpl::~FirstUseWizImpl() = default;

void FirstUseWizImpl::OnWizardFinished(wxWizardEvent& event) {
  auto cfg = g_Platform->GetConfigObject();
  // Units
  cfg->SetPath(_T("/Settings"));
  cfg->Write("SDMMFormat", m_cPosition->GetSelection());
  cfg->Write("DistanceFormat", m_cDistance->GetSelection());
  cfg->Write("SpeedFormat", m_cSpeed->GetSelection());
  // cfg->Write("WindSpeedFormat", m_cSpeed->GetSelection());
  //  True/magnetic
  cfg->Write("ShowTrue", m_cbShowTrue->GetValue());
  cfg->Write("ShowMag", m_cbShowMagnetic->GetValue());
  cfg->SetPath(_T("/Settings/GlobalState"));
  cfg->Write("S52_DEPTH_UNIT_SHOW", m_cDepth->GetSelection());
  // Connections
  bool anychecked = false;
  for (unsigned int i = 0; i < m_clSources->GetCount(); i++) {
    if (m_clSources->IsChecked(i)) {
      anychecked = true;
      break;
    }
  }
  if (anychecked) {
    cfg->DeleteGroup("/Settings/NMEADataSource");
    cfg->SetPath("/Settings/NMEADataSource");
    wxString connectionconfigs;
    bool firstconn = true;
    for (unsigned int i = 0; i < m_detected_connections.size(); i++) {
      if (m_clSources->IsChecked(i)) {
        if (!firstconn) {
          connectionconfigs.Append(_T("|"));
        }
        connectionconfigs.Append(m_detected_connections[i].Serialize());
        firstconn = false;
      }
    }
    cfg->Write("DataConnections", connectionconfigs);
  }
  // Charts
  if (!m_lbChartsDirs->IsEmpty()) {
    cfg->DeleteGroup("/ChartDirectories");
    cfg->SetPath("/ChartDirectories");
    for (unsigned int iDir = 0; iDir < m_lbChartsDirs->GetCount(); iDir++) {
      wxString dirn = m_lbChartsDirs->GetString(iDir);
      dirn.Append("^");
      wxString str_buf;
      str_buf.Printf("ChartDir%d", iDir + 1);
      cfg->Write(str_buf, dirn);
    }
  }
  cfg->Flush();
}

NMEA0183Flavor FirstUseWizImpl::SeemsN0183(std::string& data) {
  std::stringstream ss(data);
  std::string to;

  if (!data.empty()) {
    std::regex nmea_regex(".*[\\$!]([a-zA-Z]{5,6})(,.*)");
    std::regex nmea_crc_regex(".*[\\$!]([a-zA-Z]{5,6})(,.*)(\\*[0-9A-Z]{2})");
    while (std::getline(ss, to, '\n')) {
      if (std::regex_search(to, nmea_regex)) {
        DEBUG_LOG << "Looks like NMEA0183: " << to;
        if (std::regex_search(to, nmea_crc_regex)) {
          DEBUG_LOG << "Has CRC: " << to;
          return NMEA0183Flavor::CRC;
        }
        return NMEA0183Flavor::NO_CRC;
      } else {
        DEBUG_LOG << "Not NMEA0183: " << to;
      }
    }
  }
  return NMEA0183Flavor::INVALID;
}

bool FirstUseWizImpl::SeemsN2000(std::string& data) {
  std::stringstream ss(data);
  std::string to;

  if (!data.empty()) {
    std::regex n2k_regex(
        "[0-9]{2}:[0-9]{2}:[0-9]{2}\\.[0-9]{3} [RT] [0-9A-F]{8}("
        "[0-9A-F]{2})*");  // YD RAW format
                           // (https://www.yachtd.com/downloads/ydnr02.pdf
                           // appendix E)
    // TODO: Other formats of NMEA2000 data
    while (std::getline(ss, to, '\n')) {
      if (std::regex_search(to, n2k_regex)) {
        DEBUG_LOG << "Looks like NMEA2000: " << to;
        return true;
      } else {
        DEBUG_LOG << "Not NMEA2000: " << to;
      }
    }
  }
  return false;
}

void FirstUseWizImpl::EnumerateUSB() {
#ifndef __ANDROID__
  for (const auto& port : serial::list_ports()) {
    bool known = false;
    DEBUG_LOG << "Found port: " << port.port << ", " << port.description << ", "
              << port.hardware_id;
    for (const auto& device : known_usb_devices) {
      std::stringstream stream_vid;
      std::stringstream stream_pid;
      stream_vid << std::uppercase << std::hex << device.vid;
      stream_pid << std::uppercase << std::hex << device.pid;
      std::string hwid = port.hardware_id;
      for (auto c = hwid.begin(); c != hwid.end(); ++c) {
        *c = ::toupper(*c);
      }
      if (hwid.find(stream_vid.str()) != std::string::npos &&
          hwid.find(stream_pid.str()) != std::string::npos) {
        DEBUG_LOG << "Known device: " << port.port << " " << port.description
                  << " " << port.hardware_id;
        ConnectionParams params;
        params.Type = ConnectionType::SERIAL;
        params.NetProtocol = NetworkProtocol::PROTO_UNDEFINED;
        params.Protocol = device.protocol;
        params.LastDataProtocol = device.protocol;
        params.Port = port.port;
        params.UserComment = port.description;
        params.Baudrate = device.baudrate;
        m_detected_connections.push_back(params);
        known = true;
      }
    }
    if (!known) {
      for (auto sp : Speeds) {
        try {
          DEBUG_LOG << "Trying " << port.port << " at " << sp;
          serial::Serial serial;
          serial.setPort(port.port);
          serial.setBaudrate(sp);
          serial.open();
          serial.setTimeout(500, 500, 0, 500, 0);
          std::string data;
          for (auto i = 0; i < 4; i++) {
            try {
              data.append(serial.read(64));
            } catch (std::exception& e) {
              DEBUG_LOG << "Serial read exception: " << e.what();
            }
          }
          DEBUG_LOG << "Read: " << data;
          if (!data.empty()) {
            auto flavor = SeemsN0183(data);
            if (flavor != NMEA0183Flavor::INVALID) {
              ConnectionParams params;
              params.Type = ConnectionType::SERIAL;
              params.NetProtocol = NetworkProtocol::PROTO_UNDEFINED;
              params.Protocol = DataProtocol::PROTO_NMEA0183;
              params.LastDataProtocol = DataProtocol::PROTO_NMEA0183;
              if (flavor == NMEA0183Flavor::CRC) {
                params.ChecksumCheck = true;
              } else {
                params.ChecksumCheck = false;
              }
              params.Port = port.port;
              params.UserComment = wxString::Format(
                  "NMEA0183: %s (%s)", port.description, port.port);
              params.Baudrate = sp;
              m_detected_connections.push_back(params);
              break;
            } else if (SeemsN2000(data)) {
              ConnectionParams params;
              params.Type = ConnectionType::SERIAL;
              params.NetProtocol = NetworkProtocol::PROTO_UNDEFINED;
              params.Protocol = DataProtocol::PROTO_NMEA2000;
              params.LastDataProtocol = DataProtocol::PROTO_NMEA2000;
              params.Port = port.port;
              params.UserComment = wxString::Format(
                  "NMEA2000: %s (%s)", port.description, port.port);
              params.Baudrate = sp;
              m_detected_connections.push_back(params);
              break;
            }
          }
          serial.close();
        } catch (std::invalid_argument& e) {
          DEBUG_LOG << "Invalid argument " << port.port << " at " << sp << " "
                    << e.what();
          continue;
        } catch (serial::SerialException& e) {
          DEBUG_LOG << "SerialException " << port.port << " at " << sp << " "
                    << e.what();
          continue;
        } catch (serial::IOException& e) {
          DEBUG_LOG << "IOException " << port.port << " at " << sp << " "
                    << e.what() << "\n";
          continue;
        } catch (std::exception& e) {
          DEBUG_LOG << "Exception " << port.port << " at " << sp << " "
                    << e.what();
          continue;
        }
      }
    }
  }
#endif
}

void FirstUseWizImpl::EnumerateUDP() {
  for (auto port : UDPPorts) {
    wxIPV4address conn_addr;
    conn_addr.Service(port);
    conn_addr.AnyAddress();
    auto sock =
        new wxDatagramSocket(conn_addr, wxSOCKET_NOWAIT | wxSOCKET_REUSEADDR);
    if (!sock->Ok()) {
      DEBUG_LOG << "Failed to open UDP port " << port;
      delete sock;
      continue;
    }
    DEBUG_LOG << "Trying UDP port " << port;
    char buffer[256];
    memset(buffer, 0, 256);
    sock->SetTimeout(1);
    sock->WaitForRead(1, 0);
    sock->Read(&buffer, 256);
    std::string data(buffer);
    DEBUG_LOG << "Read: " << data;
    if (auto flavor = SeemsN0183(data); flavor != NMEA0183Flavor::INVALID) {
      ConnectionParams params;
      params.Type = ConnectionType::NETWORK;
      params.NetProtocol = NetworkProtocol::UDP;
      params.Protocol = DataProtocol::PROTO_NMEA0183;
      params.LastDataProtocol = DataProtocol::PROTO_NMEA0183;
      if (flavor == NMEA0183Flavor::CRC) {
        params.ChecksumCheck = true;
      } else {
        params.ChecksumCheck = false;
      }
      params.NetworkAddress = "0.0.0.0";
      params.NetworkPort = port;
      params.UserComment = wxString::Format(_("NMEA0183: UDP port %d"), port);
      m_detected_connections.push_back(params);
      continue;
    } else if (SeemsN2000(data)) {
      ConnectionParams params;
      params.Type = ConnectionType::NETWORK;
      params.NetProtocol = NetworkProtocol::UDP;
      params.Protocol = DataProtocol::PROTO_NMEA2000;
      params.LastDataProtocol = DataProtocol::PROTO_NMEA2000;
      params.NetworkAddress = "0.0.0.0";
      params.NetworkPort = port;
      params.UserComment = wxString::Format(_("NMEA2000: UDP port %d"), port);
      m_detected_connections.push_back(params);
      continue;
    }
    sock->Close();
    delete sock;
  }
}

#ifndef __ANDROID__
static int print_route(const struct route_entry* entry, void* arg) {
  if (entry->route_gw.__addr_u.__ip != 0) {
    DEBUG_LOG << "Found Route with GW:" << addr_ntoa(&entry->route_dst)
              << addr_ntoa(&entry->route_gw);
    auto ips = (std::vector<std::string>*)arg;
    if (std::find(ips->begin(), ips->end(), addr_ntoa(&entry->route_gw)) ==
        ips->end()) {
      ips->push_back(addr_ntoa(&entry->route_gw));
    }
  }
  return (0);
}

static int print_arp(const struct arp_entry* entry, void* arg) {
  DEBUG_LOG << "ARP entry: " << addr_ntoa(&entry->arp_pa) << " "
            << addr_ntoa(&entry->arp_ha);
  auto ips = (std::vector<std::string>*)arg;
  if (std::find(ips->begin(), ips->end(), addr_ntoa(&entry->arp_pa)) ==
      ips->end()) {
    ips->push_back(addr_ntoa(&entry->arp_pa));
  }
  return (0);
}
#endif

void FirstUseWizImpl::EnumerateTCP() {
#ifndef __ANDROID__
  route_t* r;
  std::vector<std::string> ips;
  ips.emplace_back("127.0.0.1");

  if ((r = route_open()) == nullptr) {
    DEBUG_LOG << "route_open failed";
  } else {
    if (route_loop(r, print_route, &ips) < 0) {
      DEBUG_LOG << "route_loop failed";
    }
  }

  arp_t* arp;
  if ((arp = arp_open()) == nullptr) {
    DEBUG_LOG << "arp_open failed";
  } else {
    if (arp_loop(arp, print_arp, &ips) < 0) {
      DEBUG_LOG << "arp_loop failed";
    }
  }

  for (const auto& ip : ips) {
    for (auto port : TCPPorts) {
      DEBUG_LOG << "Trying TCP port " << port << " on " << ip;
      wxIPV4address conn_addr;
      conn_addr.Service(port);
      conn_addr.Hostname(ip);
      auto client = new wxSocketClient();
      client->SetTimeout(1);
      if (client->Connect(conn_addr, true)) {
        DEBUG_LOG << "Connected to " << ip << ":" << port;
        char buffer[256];
        memset(buffer, 0, 256);
        client->WaitForRead(1, 0);
        client->Read(&buffer, 256);
        std::string data(buffer);
        DEBUG_LOG << "Read: " << data;
        if (auto flavor = SeemsN0183(data); flavor != NMEA0183Flavor::INVALID) {
          ConnectionParams params;
          params.Type = ConnectionType::NETWORK;
          params.NetProtocol = NetworkProtocol::TCP;
          params.Protocol = DataProtocol::PROTO_NMEA0183;
          params.LastDataProtocol = DataProtocol::PROTO_NMEA0183;
          if (flavor == NMEA0183Flavor::CRC) {
            params.ChecksumCheck = true;
          } else {
            params.ChecksumCheck = false;
          }
          params.NetworkAddress = ip;
          params.NetworkPort = port;
          params.UserComment =
              wxString::Format(_("NMEA0183: %s TCP port %d"), ip.c_str(), port);
          m_detected_connections.push_back(params);
          continue;
        } else if (SeemsN2000(data)) {
          ConnectionParams params;
          params.Type = ConnectionType::NETWORK;
          params.NetProtocol = NetworkProtocol::TCP;
          params.Protocol = DataProtocol::PROTO_NMEA2000;
          params.LastDataProtocol = DataProtocol::PROTO_NMEA2000;
          params.NetworkAddress = ip;
          params.NetworkPort = port;
          params.UserComment =
              wxString::Format(_("NMEA2000: %s TCP port %d"), ip.c_str(), port);
          m_detected_connections.push_back(params);
          continue;
        }
      }
      client->Close();
      delete client;
    }
  }
  route_close(r);
  arp_close(arp);
#endif
}

void FirstUseWizImpl::EnumerateSignalK() { FindAllSignalKServers(1); }

void FirstUseWizImpl::EnumerateCAN() {
  // TODO look at the canX interfaces if we are on Linux
}

void FirstUseWizImpl::EnumerateGPSD() {
#ifndef __ANDROID__
  // TODO: Is anybody using GPSD over network?
  wxIPV4address conn_addr;
  conn_addr.Service(2947);
  conn_addr.Hostname("127.0.0.1");
  auto client = new wxSocketClient();
  client->SetTimeout(1);
  if (client->Connect(conn_addr, true)) {
    ConnectionParams params;
    params.Type = ConnectionType::NETWORK;
    params.NetProtocol = NetworkProtocol::GPSD;
    params.Protocol = DataProtocol::PROTO_NMEA0183;
    params.LastDataProtocol = DataProtocol::PROTO_NMEA0183;
    params.NetworkAddress = "127.0.0.1";
    params.NetworkPort = 2947;
    params.UserComment =
        wxString::Format(_("GPSd: %s TCP port %d"), "127.0.0.1", 2947);
    m_detected_connections.push_back(params);
  }
  client->Close();
  delete client;
#endif
}

void FirstUseWizImpl::EnumerateDatasources() {
  wxTheApp->ProcessPendingEvents();
  g_Platform->ShowBusySpinner();
  m_clSources->Clear();
  m_detected_connections.clear();
  m_rtConnectionInfo->Clear();
  wxTheApp->ProcessPendingEvents();
  m_rtConnectionInfo->WriteText(_("Looking for Signal K servers..."));
  m_rtConnectionInfo->Newline();
  EnumerateSignalK();
  wxTheApp->ProcessPendingEvents();
  m_rtConnectionInfo->WriteText(_("Scanning USB devices..."));
  m_rtConnectionInfo->Newline();
  EnumerateUSB();
  wxTheApp->ProcessPendingEvents();
  m_rtConnectionInfo->WriteText(_("Looking for UDP data feeds..."));
  m_rtConnectionInfo->Newline();
  EnumerateUDP();
  wxTheApp->ProcessPendingEvents();
  m_rtConnectionInfo->WriteText(_("Looking for TCP servers..."));
  m_rtConnectionInfo->Newline();
  EnumerateTCP();
  wxTheApp->ProcessPendingEvents();
  m_rtConnectionInfo->WriteText(_("Looking for CAN interfaces..."));
  m_rtConnectionInfo->Newline();
  EnumerateCAN();
  wxTheApp->ProcessPendingEvents();
  m_rtConnectionInfo->WriteText(_("Looking for GPSD servers..."));
  m_rtConnectionInfo->Newline();
  EnumerateGPSD();
  wxTheApp->ProcessPendingEvents();
  for (const auto& sks : g_sk_servers) {
    ConnectionParams params;
    params.Type = ConnectionType::NETWORK;
    params.NetProtocol = NetworkProtocol::SIGNALK;
    params.Protocol = DataProtocol::PROTO_SIGNALK;
    params.LastDataProtocol = DataProtocol::PROTO_SIGNALK;
    params.NetworkAddress = sks.ip;
    params.NetworkPort = std::stoi(sks.port);
    params.UserComment =
        wxString::Format(_("SignalK: %s (%s port %d)"), sks.hostname,
                         params.NetworkAddress, params.NetworkPort);
    m_detected_connections.push_back(params);
  }
  for (const auto& conn : m_detected_connections) {
    m_clSources->Append(conn.UserComment);
    m_clSources->Check(m_clSources->GetCount() - 1, true);
  }
  wxString svgDir = g_Platform->GetSharedDataDir() + _T("uidata") +
                    wxFileName::GetPathSeparator() + "MUI_flat" +
                    wxFileName::GetPathSeparator();
  auto settings_icon = LoadSVG(svgDir + "MUI_settings.svg", 32, 32);
  m_rtConnectionInfo->Clear();
  m_rtConnectionInfo->WriteText(
      _("The system has been scanned for sources of navigation data."));
  m_rtConnectionInfo->Newline();
  m_rtConnectionInfo->WriteText(
      _("The connections to the discovered sources of data will be configured "
        "automatically. You may uncheck the ones you want to ignore."));
  m_rtConnectionInfo->Newline();
  m_rtConnectionInfo->WriteText(
      _("You may now connect additional USB devices or connect to a different "
        "network and press the Rescan button to update the list."));
  m_rtConnectionInfo->Newline();
  m_rtConnectionInfo->Newline();
  m_rtConnectionInfo->WriteText(
      _("The connection settings can be changed at any time in the "
        "configuration Toolbox accessible by clicking on the "));
  m_rtConnectionInfo->WriteImage(settings_icon.ConvertToImage());
  m_rtConnectionInfo->WriteText(
      _(" icon in the main Toolbar. In the Toolbox navigate to the Connections "
        "tab."));

  g_Platform->HideBusySpinner();
}

void FirstUseWizImpl::m_btnAddChartDirOnButtonClick(wxCommandEvent& event) {
  wxDirDialog dlg(this, _("Select a directory containing charts"),
                  wxEmptyString, wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
  if (dlg.ShowModal() == wxID_OK) {
    m_lbChartsDirs->Append(dlg.GetPath());
  }
}

void FirstUseWizImpl::OnWizardPageShown(wxWizardEvent& event) {
  if (event.GetPage() == m_pages[0]) {
    // Units
  } else if (event.GetPage() == m_pages[1]) {
    // Connections
    if (m_clSources->IsEmpty()) {
      EnumerateDatasources();
    }
  } else if (event.GetPage() == m_pages[2]) {
    // Charts
    // TODO: Maybe look somewhere for charts proactively, but it will be slow...
  }
}
