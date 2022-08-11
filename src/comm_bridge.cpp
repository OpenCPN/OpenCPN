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

// CommBridge implementation

CommBridge::CommBridge() {

}

CommBridge::~CommBridge() {}

bool CommBridge::Initialize() {

  // Initialize the comm listeners
  InitCommListeners();

  return true;
}


void CommBridge::InitCommListeners() {

  // Initialize the comm listeners

  auto& msgbus = NavMsgBus::getInstance();

  // GNSS Position Data PGN  129029
  //----------------------------------
  Nmea2000Msg n2k_msg_129029(static_cast<uint64_t>(129029));

  listener_N2K_129029 = msgbus.get_listener(EVT_N2K_129029, this, n2k_msg_129029.key());

  Bind(EVT_N2K_129029, [&](wxCommandEvent ev) {
        auto message = get_navmsg_ptr(ev);
        auto n2k_msg = std::dynamic_pointer_cast<const Nmea2000Msg>(message);
        HandleN2K_129029( n2k_msg );
      });

  // COG SOG rapid   PGN 129026
  //-----------------------------
  Nmea2000Msg n2k_msg_129026(static_cast<uint64_t>(129026));
  listener_N2K_129026 = msgbus.get_listener(EVT_N2K_129026, this, n2k_msg_129026.key());
   Bind(EVT_N2K_129026, [&](wxCommandEvent ev) {
         auto message = get_navmsg_ptr(ev);
         auto n2k_msg = std::dynamic_pointer_cast<const Nmea2000Msg>(message);
         HandleN2K_129026( n2k_msg );
       });


  //NMEA0183
  //RMC
  Nmea0183Msg n0183_msg_RMC("RMC", "");
  listener_N0183_RMC = msgbus.get_listener(EVT_N0183_RMC, this, n0183_msg_RMC.key());

  Bind(EVT_N0183_RMC, [&](wxCommandEvent ev) {
        auto message = get_navmsg_ptr(ev);
        auto n0183_msg = std::dynamic_pointer_cast<const Nmea0183Msg>(message);
        HandleN0183_RMC( n0183_msg );
      });

  //HDT
  Nmea0183Msg n0183_msg_HDT("HDT", "");
  listener_N0183_HDT = msgbus.get_listener(EVT_N0183_HDT, this, n0183_msg_HDT.key());

  Bind(EVT_N0183_HDT, [&](wxCommandEvent ev) {
        auto message = get_navmsg_ptr(ev);
        auto n0183_msg = std::dynamic_pointer_cast<const Nmea0183Msg>(message);
        HandleN0183_HDT( n0183_msg );
      });

  //HDG
  Nmea0183Msg n0183_msg_HDG("HDG", "");
  listener_N0183_HDG = msgbus.get_listener(EVT_N0183_HDG, this, n0183_msg_HDG.key());

  Bind(EVT_N0183_HDG, [&](wxCommandEvent ev) {
        auto message = get_navmsg_ptr(ev);
        auto n0183_msg = std::dynamic_pointer_cast<const Nmea0183Msg>(message);
        HandleN0183_HDG( n0183_msg );
      });

  //HDM
  Nmea0183Msg n0183_msg_HDM("HDM", "");
  listener_N0183_HDM = msgbus.get_listener(EVT_N0183_HDM, this, n0183_msg_HDM.key());

  Bind(EVT_N0183_HDM, [&](wxCommandEvent ev) {
        auto message = get_navmsg_ptr(ev);
        auto n0183_msg = std::dynamic_pointer_cast<const Nmea0183Msg>(message);
        HandleN0183_HDM( n0183_msg );
      });

  //VTG
  Nmea0183Msg n0183_msg_VTG("VTG", "");
  listener_N0183_VTG = msgbus.get_listener(EVT_N0183_VTG, this, n0183_msg_VTG.key());

  Bind(EVT_N0183_VTG, [&](wxCommandEvent ev) {
        auto message = get_navmsg_ptr(ev);
        auto n0183_msg = std::dynamic_pointer_cast<const Nmea0183Msg>(message);
        HandleN0183_VTG( n0183_msg );
      });

  //GSV
  Nmea0183Msg n0183_msg_GSV("GSV", "");
  listener_N0183_GSV = msgbus.get_listener(EVT_N0183_GSV, this, n0183_msg_GSV.key());

  Bind(EVT_N0183_GSV, [&](wxCommandEvent ev) {
        auto message = get_navmsg_ptr(ev);
        auto n0183_msg = std::dynamic_pointer_cast<const Nmea0183Msg>(message);
        HandleN0183_GSV( n0183_msg );
      });

  //GGA
  Nmea0183Msg n0183_msg_GGA("GGA", "");
  listener_N0183_GGA = msgbus.get_listener(EVT_N0183_GGA, this, n0183_msg_GGA.key());

  Bind(EVT_N0183_GGA, [&](wxCommandEvent ev) {
        auto message = get_navmsg_ptr(ev);
        auto n0183_msg = std::dynamic_pointer_cast<const Nmea0183Msg>(message);
        HandleN0183_GGA( n0183_msg );
      });

  //GLL
  Nmea0183Msg n0183_msg_GLL("GLL", "");
  listener_N0183_GLL = msgbus.get_listener(EVT_N0183_GLL, this, n0183_msg_GLL.key());

  Bind(EVT_N0183_GLL, [&](wxCommandEvent ev) {
        auto message = get_navmsg_ptr(ev);
        auto n0183_msg = std::dynamic_pointer_cast<const Nmea0183Msg>(message);
        HandleN0183_GLL( n0183_msg );
      });

  //AIVDO
  Nmea0183Msg n0183_msg_AIVDO("AIVDO", "");
  listener_N0183_AIVDO = msgbus.get_listener(EVT_N0183_AIVDO, this, n0183_msg_AIVDO.key());

  Bind(EVT_N0183_AIVDO, [&](wxCommandEvent ev) {
        auto message = get_navmsg_ptr(ev);
        auto n0183_msg = std::dynamic_pointer_cast<const Nmea0183Msg>(message);
        HandleN0183_AIVDO( n0183_msg );
      });

}


bool CommBridge::HandleN2K_129029( std::shared_ptr <const Nmea2000Msg> n2k_msg ) {
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

bool CommBridge::HandleN2K_129026( std::shared_ptr <const Nmea2000Msg> n2k_msg ) {
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

bool CommBridge::HandleN0183_RMC( std::shared_ptr <const Nmea0183Msg> n0183_msg ) {

  printf("HandleN0183_RMC \n");

  // Populate a comm_appmsg with current global values
  auto msg = std::make_shared<BasicNavDataMsg>(gLat, gLon,
            gSog, gCog, gVar, gHdt, wxDateTime::Now().GetTicks());


  std::string str = n0183_msg->payload;
  m_decoder.DecodeRMC(str, msg);

  // Notify the AppMsgBus of new data available
  auto msgbus = AppMsgBus::getInstance();
  msgbus->notify(std::move(msg));

  return true;
}

bool CommBridge::HandleN0183_HDT( std::shared_ptr <const Nmea0183Msg> n0183_msg ){
#if 0
  std::string str = n0183_msg->payload;

  wxString sentence(str.c_str());
  wxString sentence3 = ProcessNMEA4Tags(sentence);

  //FIXME Evaluate priority here?
  m_NMEA0183 << sentence3;

  if (!m_NMEA0183.PreParse())
    return false;
  if (!m_NMEA0183.Parse())
    return false;

  gHdt = m_NMEA0183.Hdt.DegreesTrue;
  if (!std::isnan(m_NMEA0183.Hdt.DegreesTrue)) {
    g_bHDT_Rx = true;
    gHDT_Watchdog = gps_watchdog_timeout_ticks;
  }
  PostProcessNMEA(false, false, false, "");
#endif
  return true;
}

bool CommBridge::HandleN0183_HDG( std::shared_ptr <const Nmea0183Msg> n0183_msg ){
#if 0
  std::string str = n0183_msg->payload;

  wxString sentence(str.c_str());
  wxString sentence3 = ProcessNMEA4Tags(sentence);

  //FIXME Evaluate priority here?
  m_NMEA0183 << sentence3;

  if (!m_NMEA0183.PreParse())
    return false;
  if (!m_NMEA0183.Parse())
    return false;

  gHdm = m_NMEA0183.Hdg.MagneticSensorHeadingDegrees;
  if (!std::isnan(m_NMEA0183.Hdg.MagneticSensorHeadingDegrees))
    gHDx_Watchdog = gps_watchdog_timeout_ticks;

  // Any device sending VAR=0.0 can be assumed to not really know
  // what the actual variation is, so in this case we use WMM if
  // available
  if ((!std::isnan(m_NMEA0183.Hdg.MagneticVariationDegrees)) &&
              0.0 != m_NMEA0183.Hdg.MagneticVariationDegrees) {
    if (m_NMEA0183.Hdg.MagneticVariationDirection == East)
      gVar = m_NMEA0183.Hdg.MagneticVariationDegrees;
    else if (m_NMEA0183.Hdg.MagneticVariationDirection == West)
      gVar = -m_NMEA0183.Hdg.MagneticVariationDegrees;

    g_bVAR_Rx = true;
    gVAR_Watchdog = gps_watchdog_timeout_ticks;
  }
  PostProcessNMEA(false, false, false, "");
#endif
  return true;
}

bool CommBridge::HandleN0183_HDM( std::shared_ptr <const Nmea0183Msg> n0183_msg ){
#if 0
  printf("HandleN0183_HDM \n");
    std::string str = n0183_msg->payload;

  wxString sentence(str.c_str());
  wxString sentence3 = ProcessNMEA4Tags(sentence);

  //FIXME Evaluate priority here?
  m_NMEA0183 << sentence3;

  if (!m_NMEA0183.PreParse())
    return false;
  if (!m_NMEA0183.Parse())
    return false;

  gHdm = m_NMEA0183.Hdm.DegreesMagnetic;
  if (!std::isnan(m_NMEA0183.Hdm.DegreesMagnetic))
    gHDx_Watchdog = gps_watchdog_timeout_ticks;
  PostProcessNMEA(false, false, false, "");
#endif

  return true;
}

bool CommBridge::HandleN0183_VTG( std::shared_ptr <const Nmea0183Msg> n0183_msg ){
#if 0
  bool bsog_valid = false;
  bool bcog_valid = false;

  std::string str = n0183_msg->payload;

  wxString sentence(str.c_str());
  wxString sentence3 = ProcessNMEA4Tags(sentence);

  //FIXME Evaluate priority here?
  m_NMEA0183 << sentence3;

  if (!m_NMEA0183.PreParse())
    return false;
  if (!m_NMEA0183.Parse())
    return false;

  // should we allow either Sog or Cog but not both to be valid?
  if (!g_own_ship_sog_cog_calc &&
      !std::isnan(m_NMEA0183.Vtg.SpeedKnots)){
    gSog = m_NMEA0183.Vtg.SpeedKnots;
    bsog_valid = true;
  }

  if (!g_own_ship_sog_cog_calc &&
      !std::isnan(m_NMEA0183.Vtg.TrackDegreesTrue)){
    gCog = m_NMEA0183.Vtg.TrackDegreesTrue;
    bcog_valid = true;
  }

  if (!g_own_ship_sog_cog_calc &&
      !std::isnan(m_NMEA0183.Vtg.SpeedKnots) &&
      !std::isnan(m_NMEA0183.Vtg.TrackDegreesTrue)) {
    gCog = m_NMEA0183.Vtg.TrackDegreesTrue;
    bcog_valid = true;
  }
  PostProcessNMEA(false, bsog_valid, bcog_valid, "");
#endif
  return true;
}

bool CommBridge::HandleN0183_GSV( std::shared_ptr <const Nmea0183Msg> n0183_msg ){
#if 0
  std::string str = n0183_msg->payload;

  wxString sentence(str.c_str());
  wxString sentence3 = ProcessNMEA4Tags(sentence);

  //FIXME Evaluate priority here?
  m_NMEA0183 << sentence3;

  if (!m_NMEA0183.PreParse())
    return false;
  if (!m_NMEA0183.Parse())
    return false;

  if (g_priSats >= 4) {
    if (m_NMEA0183.Gsv.MessageNumber == 1) {
      // Some GNSS print SatsInView in message #1 only
      setSatelitesInView(m_NMEA0183.Gsv.SatsInView);
      g_priSats = 4;
    }
  }
  PostProcessNMEA(false, false, false, "");
#endif
  return true;
}

bool CommBridge::HandleN0183_GGA( std::shared_ptr <const Nmea0183Msg> n0183_msg ){
#if 0
  std::string str = n0183_msg->payload;

  wxString sentence(str.c_str());
  wxString sentence3 = ProcessNMEA4Tags(sentence);

  //FIXME Evaluate priority here?
  m_NMEA0183 << sentence3;

  if (!m_NMEA0183.PreParse())
    return false;
  if (!m_NMEA0183.Parse())
    return false;

  bool pos_valid = false;
  wxString sfixtime;
  if (m_NMEA0183.Gga.GPSQuality > 0) {
    pos_valid = ParsePosition(m_NMEA0183.Gga.Position);
    sfixtime = m_NMEA0183.Gga.UTCTime;
    if (g_priSats >= 1) {
      setSatelitesInView(m_NMEA0183.Gga.NumberOfSatellitesInUse);
      g_priSats = 1;
    }
  }
  PostProcessNMEA(pos_valid, false, false, sfixtime);
#endif
  return true;
}

bool CommBridge::HandleN0183_GLL( std::shared_ptr <const Nmea0183Msg> n0183_msg ){
#if 0
  std::string str = n0183_msg->payload;

  wxString sentence(str.c_str());
  wxString sentence3 = sentence; //ProcessNMEA4Tags(sentence);

  //FIXME Evaluate priority here?
  m_NMEA0183 << sentence3;

  if (!m_NMEA0183.PreParse())
    return false;
  if (!m_NMEA0183.Parse())
    return false;

  bool pos_valid = false;
  wxString sfixtime;

  if (m_NMEA0183.Gll.IsDataValid == NTrue) {
    pos_valid = ParsePosition(m_NMEA0183.Gll.Position);
    sfixtime = m_NMEA0183.Gll.UTCTime;
  }
  PostProcessNMEA(pos_valid, false, false, sfixtime);
#endif
  return true;
}

bool CommBridge::HandleN0183_AIVDO( std::shared_ptr <const Nmea0183Msg> n0183_msg ){
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


