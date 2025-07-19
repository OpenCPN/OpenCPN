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

#include "model/multiplexer.h"

#include "model/config_vars.h"
#include "model/conn_params.h"
#include "model/comm_drv_registry.h"
#include "model/comm_drv_n0183_serial.h"
#include "model/comm_drv_n0183_net.h"
#include "model/comm_drv_n0183_android_bt.h"
#include "model/comm_navmsg_bus.h"
#include "model/nmea_log.h"

wxDEFINE_EVENT(EVT_N0183_MUX, ObservedEvt);

wxDEFINE_EVENT(EVT_N2K_129029, ObservedEvt);
wxDEFINE_EVENT(EVT_N2K_129025, ObservedEvt);
wxDEFINE_EVENT(EVT_N2K_129026, ObservedEvt);
wxDEFINE_EVENT(EVT_N2K_127250, ObservedEvt);
wxDEFINE_EVENT(EVT_N2K_129540, ObservedEvt);
wxDEFINE_EVENT(EVT_N2K_ALL, ObservedEvt);

Multiplexer *g_pMUX;

bool CheckSumCheck(const std::string &sentence) {
  size_t check_start = sentence.find('*');
  if (check_start == wxString::npos || check_start > sentence.size() - 3)
    return false;  // * not found, or it didn't have 2 characters following it.

  std::string check_str = sentence.substr(check_start + 1, 2);
  unsigned long checksum = strtol(check_str.c_str(), 0, 16);
  if (checksum == 0L && check_str != "00") return false;

  unsigned char calculated_checksum = 0;
  for (std::string::const_iterator i = sentence.begin() + 1;
       i != sentence.end() && *i != '*'; ++i)
    calculated_checksum ^= static_cast<unsigned char>(*i);

  return calculated_checksum == checksum;
}

Multiplexer::Multiplexer(MuxLogCallbacks cb, bool &filter_behaviour)
    : m_log_callbacks(cb), m_legacy_input_filter_behaviour(filter_behaviour) {
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

void Multiplexer::LogOutputMessage(const std::shared_ptr<const NavMsg> &msg,
                                   NavmsgStatus ns) {
  if (m_log_callbacks.log_is_active()) {
    ns.direction = NavmsgStatus::Direction::kOutput;
    Logline ll(msg, ns);
    m_log_callbacks.log_message(ll);
  }
}

void Multiplexer::LogInputMessage(const std::shared_ptr<const NavMsg> &msg,
                                  bool is_filtered, bool is_error,
                                  const wxString error_msg) {
  if (m_log_callbacks.log_is_active()) {
    NavmsgStatus ns;
    ns.direction = NavmsgStatus::Direction::kHandled;
    if (is_error) {
      ns.status = NavmsgStatus::State::kChecksumError;
    } else {
      if (is_filtered) {
        if (m_legacy_input_filter_behaviour) {
          ns.accepted = NavmsgStatus::Accepted::kFilteredNoOutput;
        } else {
          ns.accepted = NavmsgStatus::Accepted::kFilteredDropped;
        }
      } else {
        ns.accepted = NavmsgStatus::Accepted::kOk;
      }
    }
    Logline ll(msg, ns);
    ll.error_msg = error_msg;
    m_log_callbacks.log_message(ll);
  }
}

void Multiplexer::HandleN0183(std::shared_ptr<const Nmea0183Msg> n0183_msg) {
  // Find the driver that originated this message

  const auto &drivers = CommDriverRegistry::GetInstance().GetDrivers();
  auto &source_driver = FindDriver(drivers, n0183_msg->source->iface);
  if (!source_driver) return;

  wxString fmsg;
  bool bpass_input_filter = true;

  // Send to the Debug Window, if open
  //  Special formatting for non-printable characters helps debugging NMEA
  //  problems
  std::string str = n0183_msg->payload;

  // Get the params for the driver sending this message
  ConnectionParams params;
  auto drv_serial = dynamic_cast<CommDriverN0183Serial *>(source_driver.get());
  if (drv_serial) {
    params = drv_serial->GetParams();
  } else {
    auto drv_net = dynamic_cast<CommDriverN0183Net *>(source_driver.get());
    if (drv_net) {
      params = drv_net->GetParams();
    }
#ifdef __ANDROID__
    else {
      auto drv_bluetooth =
          dynamic_cast<CommDriverN0183AndroidBT *>(source_driver.get());

      if (drv_bluetooth) {
        params = drv_bluetooth->GetParams();
      }
    }
#endif
  }

  // Check to see if the message passes the source's input filter
  bpass_input_filter =
      params.SentencePassesFilter(n0183_msg->payload.c_str(), FILTER_INPUT);

  bool b_error = false;
  wxString error_msg;
  for (std::string::iterator it = str.begin(); it != str.end(); ++it) {
    if (isprint(*it))
      fmsg += *it;
    else {
      wxString bin_print;
      bin_print.Printf(_T("<0x%02X>"), *it);
      fmsg += bin_print;
      if ((*it != 0x0a) && (*it != 0x0d)) {
        b_error = true;
        error_msg = _("Non-printable character in NMEA0183 message");
      }
    }
  }

  // Flag checksum errors
  bool checksumOK = CheckSumCheck(n0183_msg->payload);
  if (!checksumOK) {
    b_error = true;
    error_msg = _("NMEA0183 checksum error");
  }

  wxString port(n0183_msg->source->iface);
  LogInputMessage(n0183_msg, !bpass_input_filter, b_error, error_msg);

  // Detect virtual driver, message comes from plugin API
  // Set such source iface to "" for later test
  std::string source_iface;
  if (source_driver)  // NULL for virtual driver
    source_iface = source_driver->iface;

  // Perform multiplexer output functions
  for (auto &driver : drivers) {
    if (!driver) continue;
    if (driver->bus == NavAddr::Bus::N0183) {
      ConnectionParams params;
      auto drv_serial = dynamic_cast<CommDriverN0183Serial *>(driver.get());
      if (drv_serial) {
        params = drv_serial->GetParams();
      } else {
        auto drv_net = dynamic_cast<CommDriverN0183Net *>(driver.get());
        if (drv_net) {
          params = drv_net->GetParams();
        }
#ifdef __ANDROID__
        else {
          auto drv_bluetooth =
              dynamic_cast<CommDriverN0183AndroidBT *>(driver.get());
          if (drv_bluetooth) {
            params = drv_bluetooth->GetParams();
          }
        }
#endif
      }

      std::shared_ptr<const Nmea0183Msg> msg = n0183_msg;
      if ((m_legacy_input_filter_behaviour && !bpass_input_filter) ||
          bpass_input_filter) {
        //  Allow re-transmit on same port (if type is SERIAL),
        //  or any other NMEA0183 port supporting output
        //  But, do not echo to the source network interface.  This will likely
        //  recurse...
        if ((!params.DisableEcho && params.Type == SERIAL) ||
            driver->iface != source_iface) {
          if (params.IOSelect == DS_TYPE_INPUT_OUTPUT ||
              params.IOSelect == DS_TYPE_OUTPUT) {
            bool bout_filter = true;
            bool bxmit_ok = true;
            std::string id("XXXXX");
            size_t comma_pos = n0183_msg->payload.find(",");
            if (comma_pos != std::string::npos && comma_pos > 5)
              id = n0183_msg->payload.substr(1, comma_pos - 1);
            if (params.SentencePassesFilter(n0183_msg->payload.c_str(),
                                            FILTER_OUTPUT)) {
              // Reset source address. It's const, so make a modified copy

              auto null_addr = std::make_shared<NavAddr>();
              msg = std::make_shared<Nmea0183Msg>(id, n0183_msg->payload,
                                                  null_addr);
              bxmit_ok = driver->SendMessage(msg, null_addr);
              bout_filter = false;
            }

            // Send to the Debug Window, if open
            if (m_log_callbacks.log_is_active()) {
              NavmsgStatus ns;
              ns.direction = NavmsgStatus::Direction::kOutput;
              if (bout_filter) {
                ns.accepted = NavmsgStatus::Accepted::kFilteredDropped;
              } else {
                if (!bxmit_ok) ns.status = NavmsgStatus::State::kTxError;
              }
              auto logaddr = std::make_shared<NavAddr0183>(driver->iface);
              auto logmsg = std::make_shared<Nmea0183Msg>(
                  id, n0183_msg->payload, logaddr);
              LogOutputMessage(logmsg, ns);
            }
          }
        }
      }
    }
  }
}

void Multiplexer::InitN2KCommListeners() {
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
  if (!m_log_callbacks.log_is_active()) return false;

  auto payload = n2k_msg.get()->payload;
  // extract PGN
  unsigned int pgn = 0;
  pgn += n2k_msg.get()->payload.at(3);
  pgn += n2k_msg.get()->payload.at(4) << 8;
  pgn += n2k_msg.get()->payload.at(5) << 16;

#if 0
  printf(" %d: payload\n", pgn);
  for(size_t i=0; i< payload.size(); i++){
    printf("%02X ", payload.at(i));
  }
  printf("\n");
  std::string pretty = n2k_msg->to_string();
  printf("%s\n\n", pretty.c_str());
#endif

  //  Input, or output?
  if (payload.at(0) == 0x94) {  // output
    NavmsgStatus ns;
    ns.direction = NavmsgStatus::Direction::kOutput;
    LogOutputMessage(n2k_msg, ns);
  } else {  // input
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
    } else {
      if (n_N2K_repeat) {
        wxString repeat_log_msg;
        repeat_log_msg.Printf("...Repeated %d times\n", n_N2K_repeat);
        // LogInputMessage(repeat_log_msg, "N2000", false, false); FIXME(leamas)
        n_N2K_repeat = 0;
      }
    }

    wxString log_msg;
    log_msg.Printf("PGN: %d Source: %s ID: %s  Desc: %s\n", pgn, source, ident,
                   N2K_LogMessage_Detail(pgn).c_str());

    LogInputMessage(n2k_msg, false, false);

    last_pgn_logged = pgn;
  }
  return true;
}

std::string Multiplexer::N2K_LogMessage_Detail(unsigned int pgn) {
  std::string notused = "Not used by OCPN, maybe by Plugins";

  switch (pgn) {
    case 129029:
      return "GNSS Position & DBoard: SAT System";
    case 129025:
      return "Position rapid";
    case 129026:
      return "COG/SOG rapid";
    case 129038:
      return "AIS Class A position report";
    case 129039:
      return "AIS Class B position report";
    case 129041:
      return "AIS Aids to Navigation (AtoN) Report";
    case 129793:
      return "AIS Base Station report";
    case 129794:
      return "AIS static data class A";
      ;
    case 129809:
      return "AIS static data class B part A";
    case 129810:
      return "AIS static data class B part B";
    case 127250:
      return "Heading rapid";
    case 129540:
      return "GNSS Sats & DBoard: SAT Status";
    //>> Dashboard
    case 127245:
      return "DBoard: Rudder data";
    case 127257:
      return "DBoard: Roll Pitch";
    case 128259:
      return "DBoard: Speed through water";
      ;
    case 128267:
      return "DBoard: Depth Data";
    case 128275:
      return "DBoard: Distance log";
    case 130306:
      return "DBoard: Wind data";
    case 130310:
      return "DBoard: Envorinment data";
    // Not used PGNs
    case 126992:
      return "System time. " + notused;
    case 127233:
      return "Man Overboard Notification. " + notused;
    case 127237:
      return "Heading/Track control. " + notused;
    case 127251:
      return "Rate of turn. " + notused;
    case 127258:
      return "Magnetic variation. " + notused;
    case 127488:
      return "Engine rapid param. " + notused;
    case 127489:
      return "Engine parameters dynamic. " + notused;
    case 127493:
      return "Transmission parameters dynamic. " + notused;
    case 127497:
      return "Trip Parameters, Engine. " + notused;
    case 127501:
      return "Binary status report. " + notused;
    case 127505:
      return "Fluid level. " + notused;
    case 127506:
      return "DC Detailed Status. " + notused;
    case 127507:
      return "Charger Status. " + notused;
    case 127508:
      return "Battery Status. " + notused;
    case 127513:
      return "Battery Configuration Status. " + notused;
    case 128000:
      return "Leeway. " + notused;
    case 128776:
      return "Windlass Control Status. " + notused;
    case 128777:
      return "Windlass Operating Status. " + notused;
    case 128778:
      return "Windlass Monitoring Status. " + notused;
    case 129033:
      return "Date,Time & Local offset. " + notused;
    case 129539:
      return "GNSS DOP data. " + notused;
    case 129283:
      return "Cross Track Error. " + notused;
    case 129284:
      return "Navigation info. " + notused;
    case 129285:
      return "Waypoint list. " + notused;
    case 129802:
      return "AIS Safety Related Broadcast Message. " + notused;
    case 130074:
      return "Waypoint list. " + notused;
    case 130311:
      return "Environmental parameters. " + notused;
    case 130312:
      return "Temperature. " + notused;
    case 130313:
      return "Humidity. " + notused;
    case 130314:
      return "Actual Pressure. " + notused;
    case 130315:
      return "Set Pressure. " + notused;
    case 130316:
      return "Temperature extended range. " + notused;
    case 130323:
      return "Meteorological Station Data. " + notused;
    case 130576:
      return "Trim Tab Position. " + notused;
    case 130577:
      return "Direction Data. " + notused;
    default:
      return "No description. Not used by OCPN, maybe passed to plugins";
  }
}
