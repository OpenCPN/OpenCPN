/**************************************************************************
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

/**
 *  \file
 *
 *  Implement multiplexer.h
 */

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
#include "model/comm_navmsg_bus.h"
#include "model/nmea_log.h"

Multiplexer *g_pMUX;

/** KeyProvider wrapper for a plain key string. */
class RawKey : public KeyProvider {
public:
  explicit RawKey(const std::string &key) : m_key(key) {}
  [[nodiscard]] std::string GetKey() const override { return m_key; }

private:
  std::string m_key;
};

static bool CheckSumCheck(const std::string &sentence) {
  size_t check_start = sentence.find('*');
  if (check_start == wxString::npos || check_start > sentence.size() - 3)
    return false;  // * not found, or it didn't have 2 characters following it.

  std::string check_str = sentence.substr(check_start + 1, 2);
  unsigned long checksum = strtol(check_str.c_str(), nullptr, 16);
  if (checksum == 0L && check_str != "00") return false;

  unsigned char calculated_checksum = 0;
  for (std::string::const_iterator i = sentence.begin() + 1;
       i != sentence.end() && *i != '*'; ++i)
    calculated_checksum ^= static_cast<unsigned char>(*i);

  return calculated_checksum == checksum;
}

static std::string N2K_LogMessage_Detail(unsigned int pgn) {
  std::string not_used = "Not used by OCPN, maybe by Plugins";

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
      return "DBoard: Environment data";
    // Not used PGNs
    case 126992:
      return "System time. " + not_used;
    case 127233:
      return "Man Overboard Notification. " + not_used;
    case 127237:
      return "Heading/Track control. " + not_used;
    case 127251:
      return "Rate of turn. " + not_used;
    case 127258:
      return "Magnetic variation. " + not_used;
    case 127488:
      return "Engine rapid param. " + not_used;
    case 127489:
      return "Engine parameters dynamic. " + not_used;
    case 127493:
      return "Transmission parameters dynamic. " + not_used;
    case 127497:
      return "Trip Parameters, Engine. " + not_used;
    case 127501:
      return "Binary status report. " + not_used;
    case 127505:
      return "Fluid level. " + not_used;
    case 127506:
      return "DC Detailed Status. " + not_used;
    case 127507:
      return "Charger Status. " + not_used;
    case 127508:
      return "Battery Status. " + not_used;
    case 127513:
      return "Battery Configuration Status. " + not_used;
    case 128000:
      return "Leeway. " + not_used;
    case 128776:
      return "Windlass Control Status. " + not_used;
    case 128777:
      return "Windlass Operating Status. " + not_used;
    case 128778:
      return "Windlass Monitoring Status. " + not_used;
    case 129033:
      return "Date,Time & Local offset. " + not_used;
    case 129539:
      return "GNSS DOP data. " + not_used;
    case 129283:
      return "Cross Track Error. " + not_used;
    case 129284:
      return "Navigation info. " + not_used;
    case 129285:
      return "Waypoint list. " + not_used;
    case 129802:
      return "AIS Safety Related Broadcast Message. " + not_used;
    case 130074:
      return "Waypoint list. " + not_used;
    case 130311:
      return "Environmental parameters. " + not_used;
    case 130312:
      return "Temperature. " + not_used;
    case 130313:
      return "Humidity. " + not_used;
    case 130314:
      return "Actual Pressure. " + not_used;
    case 130315:
      return "Set Pressure. " + not_used;
    case 130316:
      return "Temperature extended range. " + not_used;
    case 130323:
      return "Meteorological Station Data. " + not_used;
    case 130576:
      return "Trim Tab Position. " + not_used;
    case 130577:
      return "Direction Data. " + not_used;
    default:
      return "No description. Not used by OCPN, maybe passed to plugins";
  }
}

Multiplexer::Multiplexer(const MuxLogCallbacks &cb, bool &filter_behaviour)

    : m_log_callbacks(cb),
      m_legacy_input_filter_behaviour(filter_behaviour),
      m_new_msgtype_lstnr(NavMsgBus::GetInstance().new_msg_event,
                          [&](ObservedEvt &) { OnNewMessageType(); }),
      m_n2k_repeat_count(0),
      m_last_pgn_logged(0) {
  if (g_GPS_Ident.IsEmpty()) g_GPS_Ident = "Generic";
}

Multiplexer::~Multiplexer() = default;

void Multiplexer::LogOutputMessage(const std::shared_ptr<const NavMsg> &msg,
                                   NavmsgStatus ns) const {
  if (m_log_callbacks.log_is_active()) {
    ns.direction = NavmsgStatus::Direction::kOutput;
    Logline ll(msg, ns);
    m_log_callbacks.log_message(ll);
  }
}

void Multiplexer::LogInputMessage(const std::shared_ptr<const NavMsg> &msg,
                                  bool is_filtered, bool is_error,
                                  const wxString &error_msg) const {
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

void Multiplexer::HandleN0183(
    const std::shared_ptr<const Nmea0183Msg> &n0183_msg) const {
  // Find the driver that originated this message
  const auto &drivers = CommDriverRegistry::GetInstance().GetDrivers();
  auto &source_driver = FindDriver(drivers, n0183_msg->source->iface);
  if (!source_driver) {
    // might be a message from a "virtual" plugin.
    if ((n0183_msg->source->iface != "virtual")) {
      return;
    }
  }

  std::string str = n0183_msg->payload;
  std::string error_msg;

  bool is_bad = !std::all_of(str.begin(), str.end(), [](char c) {
    return isprint(c) || c == '\n' || c == '\r';
  });
  if (is_bad)
    error_msg = _("Non-printable character in NMEA0183 message").ToStdString();

  if (!CheckSumCheck(n0183_msg->payload)) {
    is_bad = true;
    error_msg = _("NMEA0183 checksum error");
  }
  if (source_driver) {
    // Get the params for the driver sending this message
    ConnectionParams params;
    auto drv_n0183 = dynamic_cast<CommDriverN0183 *>(source_driver.get());
    assert(drv_n0183);
    params = drv_n0183->GetParams();

    // Check to see if the message passes the source's input filter
    bool bpass_input_filter =
        params.SentencePassesFilter(n0183_msg->payload.c_str(), FILTER_INPUT);

    wxString port(n0183_msg->source->iface);
    LogInputMessage(n0183_msg, !bpass_input_filter, is_bad, error_msg);
  } else {
    // A "virtual" source i.e., an internal message from for example a plugin
    LogInputMessage(n0183_msg, false, is_bad, error_msg);
  }

  // Detect virtual driver, message comes from plugin API
  // Set such source iface to "" for later test
  std::string source_iface;
  if (source_driver)  // NULL for virtual driver
    source_iface = source_driver->iface;

  // Perform multiplexer output functions
  for (auto &driver : drivers) {
    if (!driver) continue;
    if (driver->bus == NavAddr::Bus::N0183) {
      ConnectionParams params_;
      auto *drv_n0183 = dynamic_cast<CommDriverN0183 *>(driver.get());
      ConnectionParams params = drv_n0183->GetParams();

      // Check to see if the message passes the source's input filter
      bool bpass_input_filter =
          params.SentencePassesFilter(n0183_msg->payload.c_str(), FILTER_INPUT);

      params_ = drv_n0183->GetParams();
      std::shared_ptr<const Nmea0183Msg> msg = n0183_msg;
      if ((m_legacy_input_filter_behaviour && !bpass_input_filter) ||
          bpass_input_filter) {
        //  Allow re-transmit on same port (if type is SERIAL),
        //  or any other NMEA0183 port supporting output
        //  But, do not echo to the source network interface.  This will likely
        //  recurse...
        if ((!params_.DisableEcho && params_.Type == SERIAL) ||
            driver->iface != source_iface) {
          if (params_.IOSelect == DS_TYPE_INPUT_OUTPUT ||
              params_.IOSelect == DS_TYPE_OUTPUT) {
            bool bout_filter = true;
            bool bxmit_ok = true;
            std::string id("XXXXX");
            size_t comma_pos = n0183_msg->payload.find(',');
            if (comma_pos != std::string::npos && comma_pos > 5)
              id = n0183_msg->payload.substr(1, comma_pos - 1);
            if (params_.SentencePassesFilter(n0183_msg->payload.c_str(),
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

bool Multiplexer::HandleN2kLog(
    const std::shared_ptr<const Nmea2000Msg> &n2k_msg) {
  if (!m_log_callbacks.log_is_active()) return false;

  auto payload = n2k_msg->payload;
  // extract PGN
  unsigned int pgn = 0;
  pgn += n2k_msg->payload.at(3);
  pgn += n2k_msg->payload.at(4) << 8;
  pgn += n2k_msg->payload.at(5) << 16;

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
    std::string source = n2k_msg->source->to_string();

    // extract source ID
    unsigned char source_id = n2k_msg->payload.at(7);
    char ss[4];
    sprintf(ss, "%d", source_id);
    std::string ident = std::string(ss);

    if (pgn == m_last_pgn_logged) {
      m_n2k_repeat_count++;
      return false;
    }
    if (m_n2k_repeat_count) {
      wxString repeat_log_msg;
      repeat_log_msg.Printf("...Repeated %d times\n", m_n2k_repeat_count);
      // LogInputMessage(repeat_log_msg, "N2000", false, false); FIXME(leamas)
      m_n2k_repeat_count = 0;
    }

    wxString log_msg;
    log_msg.Printf("PGN: %d Source: %s ID: %s  Desc: %s\n", pgn, source, ident,
                   N2K_LogMessage_Detail(pgn).c_str());

    LogInputMessage(n2k_msg, false, false);

    m_last_pgn_logged = pgn;
  }
  return true;
}

void Multiplexer::OnNewMessageType() {
  for (auto msg_key : NavMsgBus::GetInstance().GetActiveMessages()) {
    if (m_listeners.find(msg_key) != m_listeners.end()) continue;
    if (msg_key.find("::") == std::string::npos) continue;
    auto key_parts = ocpn::split(msg_key, "::");
    if (key_parts.size() < 2) continue;
    ObsListener ol;
    switch (NavMsg::GetBusByKey(msg_key)) {
      case NavAddr::Bus::N0183: {
        ol = ObsListener(RawKey(key_parts[1]), [&](ObservedEvt &ev) {
          HandleN0183(UnpackEvtPointer<Nmea0183Msg>(ev));
        });
      } break;

      case NavAddr::Bus::N2000:
        ol = ObsListener(RawKey(key_parts[1]), [&](ObservedEvt &ev) {
          HandleN2kLog(UnpackEvtPointer<Nmea2000Msg>(ev));
        });
        break;

      default:
        break;
    }
    m_listeners[msg_key] = std::move(ol);
  }
}
