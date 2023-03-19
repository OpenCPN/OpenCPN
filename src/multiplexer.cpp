/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  NMEA Data Multiplexer Object
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
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

#ifdef __MSVC__
#include "winsock2.h"
#include <wx/msw/winundef.h>
#endif

#include "config.h"

#ifdef HAVE_LIBGEN_H
#include <libgen.h>
#endif

#if defined(HAVE_READLINK) && !defined(HAVE_LIBGEN_H)
#error Using readlink(3) requires libgen.h which cannot be found.
#endif

#include <wx/wx.h>

#include "multiplexer.h"

#include "config_vars.h"
#include "conn_params.h"
#include "comm_drv_registry.h"
#include "comm_drv_n0183_serial.h"
#include "comm_drv_n0183_net.h"
#include "comm_drv_n0183_android_bt.h"
#include "comm_navmsg_bus.h"

#ifdef __linux__
#include "udev_rule_mgr.h"
#endif

wxDEFINE_EVENT(EVT_N0183_MUX, ObservedEvt);

wxDEFINE_EVENT(EVT_N2K_129029, ObservedEvt);
wxDEFINE_EVENT(EVT_N2K_129025, ObservedEvt);
wxDEFINE_EVENT(EVT_N2K_129026, ObservedEvt);
wxDEFINE_EVENT(EVT_N2K_127250, ObservedEvt);
wxDEFINE_EVENT(EVT_N2K_129540, ObservedEvt);
wxDEFINE_EVENT(EVT_N2K_ALL, ObservedEvt);

#ifdef HAVE_READLINK

static std::string do_readlink(const char *link) {
  // Strip possible Serial: or Usb: prefix:
  const char *colon = strchr(link, ':');
  const char *path = colon ? colon + 1 : link;

  char target[PATH_MAX + 1] = {0};
  int r = readlink(path, target, sizeof(target));
  if (r == -1 && (errno == EINVAL || errno == ENOENT)) {
    // Not a a symlink
    return path;
  }
  if (r == -1) {
    wxLogDebug("Error reading device link %s: %s", path, strerror(errno));
    return path;
  }
  if (*target == '/') {
    return target;
  }
  char buff[PATH_MAX + 1];
  memcpy(buff, path, std::min(strlen(path) + 1, (size_t)PATH_MAX));
  return std::string(dirname(buff)) + "/" + target;
}

static bool is_same_device(const char *port1, const char *port2) {
  std::string dev1 = do_readlink(port1);
  std::string dev2 = do_readlink(port2);
  return dev1 == dev2;
}

#else  // HAVE_READLINK

static bool inline is_same_device(const char *port1, const char *port2) {
  return strcmp(port1, port2) == 0;
}

#endif  // HAVE_READLINK

Multiplexer::Multiplexer(MuxLogCallbacks cb, bool& filter_behaviour)
    : m_log_callbacks(cb), m_legacy_input_filter_behaviour(filter_behaviour) {
  auto &msgbus = NavMsgBus::GetInstance();

  m_listener_N0183_all.Listen(Nmea0183Msg::MessageKey("ALL"), this,
                              EVT_N0183_MUX);
  Bind(EVT_N0183_MUX, [&](ObservedEvt ev) {
    auto ptr = ev.GetSharedPtr();
    auto n0183_msg = std::static_pointer_cast<const Nmea0183Msg>(ptr);
    HandleN0183(n0183_msg);
  });

  InitN2KCommListeners();
  n_N2K_repeat = 0;

  if (g_GPS_Ident.IsEmpty()) g_GPS_Ident = wxT("Generic");
}

Multiplexer::~Multiplexer() {}

void Multiplexer::LogOutputMessageColor(const wxString &msg,
                                        const wxString &stream_name,
                                        const wxString &color) {
  if (m_log_callbacks.log_is_active()) {
    wxDateTime now = wxDateTime::Now();
    wxString ss;
#ifndef __WXQT__  //  Date/Time on Qt are broken, at least for android
    ss = now.FormatISOTime();
#endif
    ss.Prepend(_T("--> "));
    ss.Append(_T(" ("));
    ss.Append(stream_name);
    ss.Append(_T(") "));
    ss.Append(msg);
    ss.Prepend(color);

    m_log_callbacks.log_message(ss.ToStdString());
  }
}

void Multiplexer::LogOutputMessage(const wxString &msg, wxString stream_name,
                                   bool b_filter) {
  if (b_filter)
    LogOutputMessageColor(msg, stream_name, _T("<CORAL>"));
  else
    LogOutputMessageColor(msg, stream_name, _T("<BLUE>"));
}

void Multiplexer::LogInputMessage(const wxString &msg,
                                  const wxString &stream_name, bool b_filter,
                                  bool b_error) {
  if (m_log_callbacks.log_is_active()) {
    wxDateTime now = wxDateTime::Now();
    wxString ss;
#ifndef __WXQT__  //  Date/Time on Qt are broken, at least for android
    ss = now.FormatISOTime();
#endif
    ss.Append(_T(" ("));
    ss.Append(stream_name);
    ss.Append(_T(") "));
    ss.Append(msg);
    if (b_error) {
      ss.Prepend(_T("<RED>"));
    } else {
      if (b_filter)
        if (m_legacy_input_filter_behaviour)
          ss.Prepend(_T("<CORAL>"));
        else
          ss.Prepend(_T("<MAROON>"));
      else
        ss.Prepend(_T("<GREEN>"));
    }
    m_log_callbacks.log_message(ss.ToStdString());
  }
}

void Multiplexer::HandleN0183(std::shared_ptr<const Nmea0183Msg> n0183_msg) {
  // Find the driver that originated this message

  const auto& drivers = CommDriverRegistry::GetInstance().GetDrivers();
  auto source_driver = FindDriver(drivers, n0183_msg->source->iface);

  wxString fmsg;
  bool bpass_input_filter = true;

  // Send to the Debug Window, if open
  //  Special formatting for non-printable characters helps debugging NMEA
  //  problems
  if (m_log_callbacks.log_is_active()) {
    std::string str = n0183_msg->payload;

    // Get the params for the driver sending this message
      ConnectionParams params;
      auto drv_serial =
          std::dynamic_pointer_cast<CommDriverN0183Serial>(source_driver);
      if (drv_serial) {
        params = drv_serial->GetParams();
      } else {
        auto drv_net = std::dynamic_pointer_cast<CommDriverN0183Net>(source_driver);
        if (drv_net) {
          params = drv_net->GetParams();
        }
#ifdef __ANDROID__
        else {
          auto drv_bluetooth = std::dynamic_pointer_cast<CommDriverN0183AndroidBT>(source_driver);
          if (drv_bluetooth) {
            params = drv_bluetooth->GetParams();
          }
        }
#endif
      }

    // Check to see if the message passes the source's input filter
    bpass_input_filter = params.SentencePassesFilter(n0183_msg->payload.c_str(),
                                        FILTER_INPUT);

    bool b_error = false;
    for (std::string::iterator it = str.begin(); it != str.end(); ++it) {
      if (isprint(*it))
        fmsg += *it;
      else {
        wxString bin_print;
        bin_print.Printf(_T("<0x%02X>"), *it);
        fmsg += bin_print;
        if ((*it != 0x0a) && (*it != 0x0d)) b_error = true;
      }
    }

    // FIXME (dave)  Flag checksum errors, but fix and process the sentence anyway
    //std::string goodMessage(message);
    //bool checksumOK = CheckSumCheck(event.GetNMEAString());
    //if (!checksumOK) {
    //goodMessage = stream->FixChecksum(goodMessage);
    //goodEvent->SetNMEAString(goodMessage);
    //}


    wxString port(n0183_msg->source->iface);
    LogInputMessage(fmsg, port, !bpass_input_filter, b_error);
  }

  // Detect virtual driver, message comes from plugin API
  // Set such source iface to "" for later test
  std::string source_iface;
  if (source_driver)        // NULL for virtual driver
    source_iface = source_driver->iface;


  // Perform multiplexer output functions
  for (auto& driver : drivers) {

    if (driver->bus == NavAddr::Bus::N0183) {
      ConnectionParams params;
      auto drv_serial =
          std::dynamic_pointer_cast<CommDriverN0183Serial>(driver);
      if (drv_serial) {
        params = drv_serial->GetParams();
      } else {
        auto drv_net = std::dynamic_pointer_cast<CommDriverN0183Net>(driver);
        if (drv_net) {
          params = drv_net->GetParams();
        }
#ifdef __ANDROID__
        else {
          auto drv_bluetooth = std::dynamic_pointer_cast<CommDriverN0183AndroidBT>(driver);
          if (drv_bluetooth) {
            params = drv_bluetooth->GetParams();
          }
        }
#endif
      }

      if ((m_legacy_input_filter_behaviour && !bpass_input_filter) ||
           bpass_input_filter) {

      //  Allow re-transmit on same port (if type is SERIAL),
      //  or any any other NMEA0183 port supporting output
      //  But, do not echo to the source network interface.  This will likely recurse...
        if (params.Type == SERIAL || driver->iface != source_iface) {
          if (params.IOSelect == DS_TYPE_INPUT_OUTPUT ||
              params.IOSelect == DS_TYPE_OUTPUT)
          {
            bool bout_filter = true;
            bool bxmit_ok = true;
            if (params.SentencePassesFilter(n0183_msg->payload.c_str(),
                                          FILTER_OUTPUT)) {
              bxmit_ok = driver->SendMessage(n0183_msg,
                                std::make_shared<NavAddr0183>(driver->iface));
              bout_filter = false;
            }

            // Send to the Debug Window, if open
            if (!bout_filter) {
              if (bxmit_ok)
                LogOutputMessageColor(fmsg, driver->iface, _T("<BLUE>"));
              else
                LogOutputMessageColor(fmsg, driver->iface, _T("<RED>"));
            } else
              LogOutputMessageColor(fmsg, driver->iface, _T("<CORAL>"));
          }
        }
      }
    }
  }
}

void Multiplexer::InitN2KCommListeners() {
  // Initialize the comm listeners
  auto& msgbus = NavMsgBus::GetInstance();

  // Create a series of N2K listeners
  // to allow minimal N2K Debug window logging

  // All N2K
  //----------------------------------
  Nmea2000Msg n2k_msg_All(static_cast<uint64_t>(1));
  listener_N2K_All.Listen(n2k_msg_All, this, EVT_N2K_ALL);
  Bind(EVT_N2K_ALL, [&](ObservedEvt ev) {
    HandleN2K_Log(UnpackEvtPointer<Nmea2000Msg>(ev));
  });
}

bool Multiplexer::HandleN2K_Log(std::shared_ptr<const Nmea2000Msg> n2k_msg) {
  if (!m_log_callbacks.log_is_active())
    return false;

  // extract PGN
  unsigned int pgn = 0;
  pgn += n2k_msg.get()->payload.at(3);
  pgn += n2k_msg.get()->payload.at(4) << 8;
  pgn += n2k_msg.get()->payload.at(5) << 16;

  // extract data source
  std::string source = n2k_msg.get()->source->to_string();

  // extract source ID
  unsigned char source_id = n2k_msg.get()->payload.at(7);
  char ss[4];
  sprintf(ss, "%d", source_id);
  std::string ident = std::string(ss);

  if (pgn == last_pgn_logged) {
    n_N2K_repeat++;
    return false;
  }
  else {
    if(n_N2K_repeat) {
      wxString repeat_log_msg;
      repeat_log_msg.Printf("...Repeated %d times\n", n_N2K_repeat);
      LogInputMessage(repeat_log_msg, "N2000", false, false);
      n_N2K_repeat = 0;
    }
  }

  wxString log_msg;
  log_msg.Printf("PGN: %d Source: %s ID: %s  Desc: %s\n", pgn, source,
                 ident,N2K_LogMessage_Detail(pgn, n2k_msg).c_str());

  LogInputMessage(log_msg, "N2000", false, false);

  last_pgn_logged = pgn;
  return true;
}


std::string Multiplexer::N2K_LogMessage_Detail(unsigned int pgn, std::shared_ptr<const Nmea2000Msg> n2k_msg) {
  std::string notused = "Not used by OCPN, maybe by Plugins";

  switch (pgn){
    case 129029:
      return "GNSS Position & DBoard: SAT System";
      break;
    case 129025:
      return "Position rapid";
      break;
    case 129026:
      return "COG/SOG rapid";
      break;
    case 129038:
      return "AIS Class A position report";
      break;
    case 129039:
      return "AIS Class B position report";
      break;
    case 129041:
      return "AIS Aids to Navigation (AtoN) Report";
      break;
    case 129793:
      return "AIS Base Station report";
      break;
    case 129794:
      return "AIS static data class A";
      break;
    case 129809:
      return "AIS static data class B part A";
      break;
    case 129810:
      return "AIS static data class B part B";
      break;
    case 127250:
      return "Heading rapid";
      break;
    case 129540:
      return "GNSS Sats & DBoard: SAT Status";
      break;
    //>> Dashboard
    case 127245:
      return "DBoard: Rudder data";
      break;
    case 127257:
      return "DBoard: Roll Pitch";
      break;
    case 128259:
      return "DBoard: Speed through water";
      break;
    case 128267:
      return "DBoard: Depth Data";
      break;
    case 128275:
      return "DBoard: Distance log";
      break;
    case 130306:
      return "DBoard: Wind data";
      break;
    case 130310:
      return "DBoard: Envorinment data";
      break;
    // Not used PGNs
    case 126992:
      return "System time. " + notused;
      break;
    case 127233:
      return "Man Overboard Notification. " + notused;
      break;
    case 127237:
      return "Heading/Track control. " + notused;
      break;
    case 127251:
      return "Rate of turn. " + notused;
      break;
    case 127258:
      return "Magnetic variation. " + notused;
      break;
    case 127488:
      return "Engine rapid param. " + notused;
      break;
    case 127489:
      return "Engine parameters dynamic. " + notused;
      break;
    case 127493:
      return "Transmission parameters dynamic. " + notused;
      break;
    case 127497:
      return "Trip Parameters, Engine. " + notused;
      break;
    case 127501:
      return "Binary status report. " + notused;
      break;
    case 127505:
      return "Fluid level. " + notused;
      break;
    case 127506:
      return "DC Detailed Status. " + notused;
      break;
    case 127507:
      return "Charger Status. " + notused;
      break;
    case 127508:
      return "Battery Status. " + notused;
      break;
    case 127513:
      return "Battery Configuration Status. " + notused;
      break;
    case 128000:
      return "Leeway. " + notused;
      break;
    case 128776:
      return "Windlass Control Status. " + notused;
      break;
    case 128777:
      return "Windlass Operating Status. " + notused;
      break;
    case 128778:
      return "Windlass Monitoring Status. " + notused;
      break;
    case 129033:
      return "Date,Time & Local offset. " + notused;
      break;
    case 129539:
      return "GNSS DOP data. " + notused;
      break;
    case 129283:
      return "Cross Track Error. " + notused;
      break;
    case 129284:
      return "Navigation info. " + notused;
      break;
    case 129285:
      return "Waypoint list. " + notused;
      break;
    case 129802:
      return "AIS Safety Related Broadcast Message. " + notused;
      break;
    case 130074:
      return "Waypoint list. " + notused;
      break;
    case 130311:
      return "Environmental parameters. " + notused;
      break;
    case 130312:
      return "Temperature. " + notused;
      break;
    case 130313:
      return "Humidity. " + notused;
      break;
    case 130314:
      return "Actual Pressure. " + notused;
      break;
    case 130315:
      return "Set Pressure. " + notused;
      break;
    case 130316:
      return "Temperature extended range. " + notused;
      break;
    case 130323:
      return "Meteorological Station Data. " + notused;
      break;
    case 130576:
      return "Trim Tab Position. " + notused;
      break;
    case 130577:
      return "Direction Data. " + notused;
      break;
    default:
      return "No description. Not used by OCPN, maybe passed to plugins";
  }
}
