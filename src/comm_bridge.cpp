/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:
 * Author:   David Register, Alec Leamas
 *
 ***************************************************************************
 *   Copyright (C) 2022 by David Register, Alec Leamas                     *
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

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // precompiled headers

#include "comm_bridge.h"
#include "comm_appmsg_bus.h"
#include "idents.h"

// #include "comm_util.h"
// #include "comm_drv_n2k_serial.h"
// #include "comm_drv_n0183_serial.h"
// #include "comm_drv_n0183_net.h"
// #include "comm_navmsg_bus.h"
// #include "comm_drv_registry.h"

//  comm event definitions
wxDEFINE_EVENT(EVT_N2K_129029, wxCommandEvent);
wxDEFINE_EVENT(EVT_N2K_129026, wxCommandEvent);

wxDEFINE_EVENT(EVT_N0183_RMC, wxCommandEvent);
wxDEFINE_EVENT(EVT_N0183_HDT, wxCommandEvent);
wxDEFINE_EVENT(EVT_N0183_HDG, wxCommandEvent);
wxDEFINE_EVENT(EVT_N0183_HDM, wxCommandEvent);
wxDEFINE_EVENT(EVT_N0183_VTG, wxCommandEvent);
wxDEFINE_EVENT(EVT_N0183_GSV, wxCommandEvent);
wxDEFINE_EVENT(EVT_N0183_GGA, wxCommandEvent);
wxDEFINE_EVENT(EVT_N0183_GLL, wxCommandEvent);
wxDEFINE_EVENT(EVT_N0183_AIVDO, wxCommandEvent);

extern double gLat, gLon, gCog, gSog, gHdt, gHdm, gVar;
extern wxString gRmcDate, gRmcTime;
extern int g_nNMEADebug;
extern int g_priSats, g_SatsInView;
extern bool g_bSatValid;
extern bool g_bHDT_Rx, g_bVAR_Rx;
extern double g_UserVar;
extern int gps_watchdog_timeout_ticks;

// CommBridge implementation

BEGIN_EVENT_TABLE(CommBridge, wxEvtHandler)
EVT_TIMER(WATCHDOG_TIMER, CommBridge::OnWatchdogTimer)
END_EVENT_TABLE()

CommBridge::CommBridge() {}

CommBridge::~CommBridge() {}

bool CommBridge::Initialize() {
  // Clear the watchdogs
  PresetWatchdogs();

  m_watchdog_timer.SetOwner(this, WATCHDOG_TIMER);
  m_watchdog_timer.Start(1000, wxTIMER_CONTINUOUS);

  // Initialize the comm listeners
  InitCommListeners();

  return true;
}

void CommBridge::PresetWatchdogs() {
  m_watchdogs.gps_watchdog = 5;
  m_watchdogs.var_watchdog = 5;
  m_watchdogs.hdx_watchdog = 5;
  m_watchdogs.hdt_watchdog = 5;
  m_watchdogs.sat_watchdog = 5;
}

void CommBridge::OnWatchdogTimer(wxTimerEvent& event) {
  //  Update and check watchdog timer for GPS data source
  m_watchdogs.gps_watchdog--;
  if (m_watchdogs.gps_watchdog <= 0) {
    // bGPSValid = false;
    if (m_watchdogs.gps_watchdog == 0) {
      // Send AppMsg telling of watchdog expiry
      auto msg = std::make_shared<GPSWatchdogMsg>(m_watchdogs.gps_watchdog);
      auto& msgbus = AppMsgBus::GetInstance();
      msgbus.Notify(std::move(msg));

      wxString logmsg;
      logmsg.Printf(_T("   ***GPS Watchdog timeout at Lat:%g   Lon: %g"), gLat,
                    gLon);
      wxLogMessage(logmsg);
      // FIXME (dave)
      //  There is no valid fix, we need to invalidate the fix time
      // m_fixtime = -1;
    }
    gSog = NAN;
    gCog = NAN;
    gRmcDate.Empty();
    gRmcTime.Empty();
  }

  //  Update and check watchdog timer for Mag Heading data source
  m_watchdogs.hdx_watchdog--;
  if (m_watchdogs.hdx_watchdog <= 0) {
    gHdm = NAN;
    if (g_nNMEADebug && (m_watchdogs.hdx_watchdog == 0))
      wxLogMessage(_T("   ***HDx Watchdog timeout..."));
  }

  //  Update and check watchdog timer for True Heading data source
  m_watchdogs.hdt_watchdog--;
  if (m_watchdogs.hdt_watchdog <= 0) {
    g_bHDT_Rx = false;
    gHdt = NAN;
    if (g_nNMEADebug && (m_watchdogs.hdt_watchdog == 0))
      wxLogMessage(_T("   ***HDT Watchdog timeout..."));
  }

  //  Update and check watchdog timer for Magnetic Variation data source
  m_watchdogs.var_watchdog--;
  if (m_watchdogs.var_watchdog <= 0) {
    g_bVAR_Rx = false;
    if (g_nNMEADebug && (m_watchdogs.var_watchdog == 0))
      wxLogMessage(_T("   ***VAR Watchdog timeout..."));
  }
  //  Update and check watchdog timer for GSV, GGA and SignalK (Satellite data)
  m_watchdogs.sat_watchdog--;
  if (m_watchdogs.sat_watchdog <= 0) {
    g_bSatValid = false;
    g_SatsInView = 0;
    g_priSats = 99;
    if (g_nNMEADebug && (m_watchdogs.sat_watchdog == 0))
      wxLogMessage(_T("   ***SAT Watchdog timeout..."));
  }
}

void CommBridge::MakeHDTFromHDM() {
  //    Here is the one place we try to create gHdt from gHdm and gVar,
  //    but only if NMEA HDT sentence is not being received

  if (!g_bHDT_Rx) {
    if (!std::isnan(gHdm)) {
      // Set gVar if needed from manual entry. gVar will be overwritten if
      // WMM plugin is available
      if (std::isnan(gVar) && (g_UserVar != 0.0)) gVar = g_UserVar;
      gHdt = gHdm + gVar;
      if (!std::isnan(gHdt)) {
        if (gHdt < 0)
          gHdt += 360.0;
        else if (gHdt >= 360)
          gHdt -= 360.0;
      }

      m_watchdogs.hdt_watchdog = gps_watchdog_timeout_ticks;
    }
  }
}

void CommBridge::InitCommListeners() {
  // Initialize the comm listeners

  auto& msgbus = NavMsgBus::GetInstance();

  // GNSS Position Data PGN  129029
  //----------------------------------
  Nmea2000Msg n2k_msg_129029(static_cast<uint64_t>(129029));

  listener_N2K_129029 =
      msgbus.GetListener(EVT_N2K_129029, this, n2k_msg_129029);

  Bind(EVT_N2K_129029, [&](wxCommandEvent ev) {
    auto message = get_navmsg_ptr(ev);
    auto n2k_msg = std::dynamic_pointer_cast<const Nmea2000Msg>(message);
    HandleN2K_129029(n2k_msg);
  });

  // COG SOG rapid   PGN 129026
  //-----------------------------
  Nmea2000Msg n2k_msg_129026(static_cast<uint64_t>(129026));
  listener_N2K_129026 =
      msgbus.GetListener(EVT_N2K_129026, this, n2k_msg_129026);
  Bind(EVT_N2K_129026, [&](wxCommandEvent ev) {
    auto message = get_navmsg_ptr(ev);
    auto n2k_msg = std::dynamic_pointer_cast<const Nmea2000Msg>(message);
    HandleN2K_129026(n2k_msg);
  });

  // NMEA0183
  // RMC
  Nmea0183Msg n0183_msg_RMC("RMC");
  listener_N0183_RMC = msgbus.GetListener(EVT_N0183_RMC, this, n0183_msg_RMC);

  Bind(EVT_N0183_RMC, [&](wxCommandEvent ev) {
    auto message = get_navmsg_ptr(ev);
    auto n0183_msg = std::dynamic_pointer_cast<const Nmea0183Msg>(message);
    HandleN0183_RMC(n0183_msg);
  });

  // HDT
  Nmea0183Msg n0183_msg_HDT("HDT");
  listener_N0183_HDT = msgbus.GetListener(EVT_N0183_HDT, this, n0183_msg_HDT);

  Bind(EVT_N0183_HDT, [&](wxCommandEvent ev) {
    auto message = get_navmsg_ptr(ev);
    auto n0183_msg = std::dynamic_pointer_cast<const Nmea0183Msg>(message);
    HandleN0183_HDT(n0183_msg);
  });

  // HDG
  Nmea0183Msg n0183_msg_HDG("HDG");
  listener_N0183_HDG = msgbus.GetListener(EVT_N0183_HDG, this, n0183_msg_HDG);

  Bind(EVT_N0183_HDG, [&](wxCommandEvent ev) {
    auto message = get_navmsg_ptr(ev);
    auto n0183_msg = std::dynamic_pointer_cast<const Nmea0183Msg>(message);
    HandleN0183_HDG(n0183_msg);
  });

  // HDM
  Nmea0183Msg n0183_msg_HDM("HDM");
  listener_N0183_HDM = msgbus.GetListener(EVT_N0183_HDM, this, n0183_msg_HDM);

  Bind(EVT_N0183_HDM, [&](wxCommandEvent ev) {
    auto message = get_navmsg_ptr(ev);
    auto n0183_msg = std::dynamic_pointer_cast<const Nmea0183Msg>(message);
    HandleN0183_HDM(n0183_msg);
  });

  // VTG
  Nmea0183Msg n0183_msg_VTG("VTG");
  listener_N0183_VTG = msgbus.GetListener(EVT_N0183_VTG, this, n0183_msg_VTG);

  Bind(EVT_N0183_VTG, [&](wxCommandEvent ev) {
    auto message = get_navmsg_ptr(ev);
    auto n0183_msg = std::dynamic_pointer_cast<const Nmea0183Msg>(message);
    HandleN0183_VTG(n0183_msg);
  });

  // GSV
  Nmea0183Msg n0183_msg_GSV("GSV");
  listener_N0183_GSV = msgbus.GetListener(EVT_N0183_GSV, this, n0183_msg_GSV);

  Bind(EVT_N0183_GSV, [&](wxCommandEvent ev) {
    auto message = get_navmsg_ptr(ev);
    auto n0183_msg = std::dynamic_pointer_cast<const Nmea0183Msg>(message);
    HandleN0183_GSV(n0183_msg);
  });

  // GGA
  Nmea0183Msg n0183_msg_GGA("GGA");
  listener_N0183_GGA = msgbus.GetListener(EVT_N0183_GGA, this, n0183_msg_GGA);

  Bind(EVT_N0183_GGA, [&](wxCommandEvent ev) {
    auto message = get_navmsg_ptr(ev);
    auto n0183_msg = std::dynamic_pointer_cast<const Nmea0183Msg>(message);
    HandleN0183_GGA(n0183_msg);
  });

  // GLL
  Nmea0183Msg n0183_msg_GLL("GLL");
  listener_N0183_GLL = msgbus.GetListener(EVT_N0183_GLL, this, n0183_msg_GLL);

  Bind(EVT_N0183_GLL, [&](wxCommandEvent ev) {
    auto message = get_navmsg_ptr(ev);
    auto n0183_msg = std::dynamic_pointer_cast<const Nmea0183Msg>(message);
    HandleN0183_GLL(n0183_msg);
  });

  // AIVDO
  Nmea0183Msg n0183_msg_AIVDO("AIVDO");
  listener_N0183_AIVDO =
      msgbus.GetListener(EVT_N0183_AIVDO, this, n0183_msg_AIVDO);

  Bind(EVT_N0183_AIVDO, [&](wxCommandEvent ev) {
    auto message = get_navmsg_ptr(ev);
    auto n0183_msg = std::dynamic_pointer_cast<const Nmea0183Msg>(message);
    HandleN0183_AIVDO(n0183_msg);
  });
}

bool CommBridge::HandleN2K_129029(std::shared_ptr<const Nmea2000Msg> n2k_msg) {
#if 0
  printf("   HandleN2K_129029\n");

  std::vector<unsigned char> v = n2k_msg->payload;

  // extract and verify PGN
  uint64_t pgn = 0;
  unsigned char *c = (unsigned char *)&pgn;
  *c++ = v.at(3);
  *c++ = v.at(4);
  *c++ = v.at(5);

  unsigned char SID;
  uint16_t DaysSince1970;
  double SecondsSinceMidnight;
  double Latitude, Longitude, Altitude;
  tN2kGNSStype GNSStype;
  tN2kGNSSmethod GNSSmethod;
  unsigned char nSatellites;
  double HDOP, PDOP, GeoidalSeparation;
  unsigned char nReferenceStations;
  tN2kGNSStype ReferenceStationType;
  uint16_t ReferenceSationID;
  double AgeOfCorrection;

  if (ParseN2kPGN129029(v, SID, DaysSince1970, SecondsSinceMidnight,
                        Latitude, Longitude, Altitude,
                        GNSStype, GNSSmethod,
                        nSatellites, HDOP, PDOP, GeoidalSeparation,
                        nReferenceStations, ReferenceStationType, ReferenceSationID,
                        AgeOfCorrection
                        )) {
    setPosition(Latitude, Longitude);
    PostProcessNMEA(true, false, false, "");
    setSatelitesInView(nSatellites);
  }

#endif
  return true;
}

bool CommBridge::HandleN2K_129026(std::shared_ptr<const Nmea2000Msg> n2k_msg) {
#if 0
 // std::cout << "HandleN2K_129026\n" ;

  std::vector<unsigned char> v = n2k_msg->payload;

  // extract and verify PGN
  uint64_t pgn = 0;
  unsigned char *c = (unsigned char *)&pgn;
  *c++ = v.at(3);
  *c++ = v.at(4);
  *c++ = v.at(5);

  unsigned char SID;
  tN2kHeadingReference ref;
  double COG, SOG;

  if (ParseN2kPGN129026(v, SID, ref, COG, SOG)) {
    setCourseOverGround(COG);
    setSpeedOverGround(SOG);
    PostProcessNMEA(false, true, true, "");
  }
#endif
  return true;
}

bool CommBridge::HandleN0183_RMC(std::shared_ptr<const Nmea0183Msg> n0183_msg) {
  printf("HandleN0183_RMC \n");

  std::string str = n0183_msg->payload;
  if (!m_decoder.DecodeRMC(str, m_watchdogs)) return false;

  // Populate a comm_appmsg with current global values
  auto msg = std::make_shared<BasicNavDataMsg>(
      gLat, gLon, gSog, gCog, gVar, gHdt, wxDateTime::Now().GetTicks());

  // Notify the AppMsgBus of new data available
  auto& msgbus = AppMsgBus::GetInstance();
  msgbus.Notify(std::move(msg));

  return true;
}

bool CommBridge::HandleN0183_HDT(std::shared_ptr<const Nmea0183Msg> n0183_msg) {
  std::string str = n0183_msg->payload;
  if (!m_decoder.DecodeHDT(str, m_watchdogs)) return false;

  // Populate a comm_appmsg with current global values
  auto msg = std::make_shared<BasicNavDataMsg>(
      gLat, gLon, gSog, gCog, gVar, gHdt, wxDateTime::Now().GetTicks());

  // Notify the AppMsgBus of new data available
  auto& msgbus = AppMsgBus::GetInstance();
  msgbus.Notify(std::move(msg));

  return true;
}

bool CommBridge::HandleN0183_HDG(std::shared_ptr<const Nmea0183Msg> n0183_msg) {
  std::string str = n0183_msg->payload;
  if (!m_decoder.DecodeHDG(str, m_watchdogs)) return false;

  // Populate a comm_appmsg with current global values
  auto msg = std::make_shared<BasicNavDataMsg>(
      gLat, gLon, gSog, gCog, gVar, gHdt, wxDateTime::Now().GetTicks());

  // Notify the AppMsgBus of new data available
  auto& msgbus = AppMsgBus::GetInstance();
  msgbus.Notify(std::move(msg));

  return true;
}

bool CommBridge::HandleN0183_HDM(std::shared_ptr<const Nmea0183Msg> n0183_msg) {
  std::string str = n0183_msg->payload;
  if (!m_decoder.DecodeHDM(str, m_watchdogs)) return false;

  MakeHDTFromHDM();

  // Populate a comm_appmsg with current global values
  auto msg = std::make_shared<BasicNavDataMsg>(
      gLat, gLon, gSog, gCog, gVar, gHdt, wxDateTime::Now().GetTicks());

  // Notify the AppMsgBus of new data available
  auto& msgbus = AppMsgBus::GetInstance();
  msgbus.Notify(std::move(msg));

  return true;
}

bool CommBridge::HandleN0183_VTG(std::shared_ptr<const Nmea0183Msg> n0183_msg) {
  std::string str = n0183_msg->payload;
  if (!m_decoder.DecodeVTG(str, m_watchdogs)) return false;

  // Populate a comm_appmsg with current global values
  auto msg = std::make_shared<BasicNavDataMsg>(
      gLat, gLon, gSog, gCog, gVar, gHdt, wxDateTime::Now().GetTicks());

  // Notify the AppMsgBus of new data available
  auto& msgbus = AppMsgBus::GetInstance();
  msgbus.Notify(std::move(msg));

  return true;
}

bool CommBridge::HandleN0183_GSV(std::shared_ptr<const Nmea0183Msg> n0183_msg) {
  std::string str = n0183_msg->payload;
  if (!m_decoder.DecodeGSV(str, m_watchdogs)) return false;

  // Populate a comm_appmsg with current global values
  auto msg = std::make_shared<BasicNavDataMsg>(
      gLat, gLon, gSog, gCog, gVar, gHdt, wxDateTime::Now().GetTicks());

  // Notify the AppMsgBus of new data available
  auto& msgbus = AppMsgBus::GetInstance();
  msgbus.Notify(std::move(msg));

  return true;
}

bool CommBridge::HandleN0183_GGA(std::shared_ptr<const Nmea0183Msg> n0183_msg) {
  std::string str = n0183_msg->payload;
  if (!m_decoder.DecodeGGA(str, m_watchdogs)) return false;

  // Populate a comm_appmsg with current global values
  auto msg = std::make_shared<BasicNavDataMsg>(
      gLat, gLon, gSog, gCog, gVar, gHdt, wxDateTime::Now().GetTicks());

  // Notify the AppMsgBus of new data available
  auto& msgbus = AppMsgBus::GetInstance();
  msgbus.Notify(std::move(msg));

  return true;
}

bool CommBridge::HandleN0183_GLL(std::shared_ptr<const Nmea0183Msg> n0183_msg) {
  std::string str = n0183_msg->payload;
  if (!m_decoder.DecodeGLL(str, m_watchdogs)) return false;

  // Populate a comm_appmsg with current global values
  auto msg = std::make_shared<BasicNavDataMsg>(
      gLat, gLon, gSog, gCog, gVar, gHdt, wxDateTime::Now().GetTicks());

  // Notify the AppMsgBus of new data available
  auto& msgbus = AppMsgBus::GetInstance();
  msgbus.Notify(std::move(msg));

  return true;
}

bool CommBridge::HandleN0183_AIVDO(
    std::shared_ptr<const Nmea0183Msg> n0183_msg) {
#if 0
  GenericPosDatEx gpd;
  wxString sfixtime;
  bool pos_valid = false;
  bool cog_valid = false;
  bool sog_valid = false;

  std::string str = n0183_msg->payload;
  wxString sentence(str.c_str());

  AIS_Error nerr = AIS_GENERIC_ERROR;
  if (g_pAIS)
    nerr = g_pAIS->DecodeSingleVDO(sentence, &gpd, &m_VDO_accumulator);

  if (nerr == AIS_NoError) {
    if (!std::isnan(gpd.kLat)) gLat = gpd.kLat;
    if (!std::isnan(gpd.kLon)) gLon = gpd.kLon;

    if (!g_own_ship_sog_cog_calc) {
      gCog = gpd.kCog;
      gSog = gpd.kSog;
    } else {
      UpdatePositionCalculatedSogCog();
    }
    cog_valid = true;
    sog_valid = true;

    if (!std::isnan(gpd.kHdt)) {
      gHdt = gpd.kHdt;
      g_bHDT_Rx = true;
//      gHDT_Watchdog = gps_watchdog_timeout_ticks;
    }

    if (!std::isnan(gpd.kLat) && !std::isnan(gpd.kLon)) {
//      gGPS_Watchdog = gps_watchdog_timeout_ticks;
      wxDateTime now = wxDateTime::Now();
      //m_fixtime = now.GetTicks();

      pos_valid = true;
    }

    PostProcessNMEA(pos_valid, sog_valid, cog_valid, sfixtime);
  }
#endif
  return true;
}


//   std::string mnemonic = str.substr(1,5);
//   std::string key = "N0183-RMC-";
//   key += mnemonic;
//
//   EvalPriorityPos(key, n0183_msg);

bool CommBridge::EvalPriorityPos(std::string priority_key, std::shared_ptr <const NavMsg> msg) {

  // Fetch the established priority for the message
  int this_priority;
  auto it = pos_priority_map.find(priority_key);
  if (it == pos_priority_map.end()) {
    // Not found, so make it default highest priority
    pos_priority_map[priority_key] = 0;
  }

  this_priority = pos_priority_map[priority_key];




  //Incoming message priority lower than currently active priority?
  //  If so, drop the message
  if ( this_priority > position_priority.active_priority)
    return false;

/*
  // Do we see two sources with the same priority?
  if (msg.Source != pos_prio.ActiveSource){
    // activate GUI to select
    // or auto adjust the priority of the this message down
  }

  //  For N0183 message, has the Mnemonic changed?
  //  Example:  RMC and AIVDO from same source.
  if (msg.isN0183){
    if (msg.Mnemonnic  == pos_prio.ActiveMnemonic){
      return true;
    }
    else {
      //auto adjust the priority of the this message down
      //and drop it
      return false;
    }
  }
  else{
    if (msg.PGN == pos_prio.ActivePGN){
      return true;
    }
    else {
      //auto adjust the priority of the this message down
      //and drop it
      return false;
    }
  }
*/
  return false;
}
