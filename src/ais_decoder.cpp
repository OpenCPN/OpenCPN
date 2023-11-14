/***************************************************************************
 *
 * Project:  OpenCPN
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
 ***************************************************************************
 */

// For compilers that support precompilation, includes "wx.h".
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // precompiled headers

#include <algorithm>
#include <cstdio>
#include <fstream>

#ifdef __MINGW32__
#undef IPV6STRICT  // mingw FTBS fix:  missing struct ip_mreq
#include <windows.h>
#endif

#include <wx/datetime.h>
#include <wx/event.h>
#include <wx/log.h>
#include <wx/string.h>
#include <wx/textfile.h>
#include <wx/timer.h>
#include <wx/tokenzr.h>

#include "ais_decoder.h"
#include "meteo_points.h"
#include "ais_target_data.h"
#include "comm_navmsg_bus.h"
#include "config_vars.h"
#include "geodesic.h"
#include "georef.h"
#include "idents.h"
#include "multiplexer.h"
#include "navutil_base.h"
#include "own_ship.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "route_point.h"
#include "select.h"
#include "SoundFactory.h"
#include "track.h"
#include "N2KParser.h"
#include "AISTargetAlertDialog.h"

#if !defined(NAN)
static const long long lNaN = 0xfff8000000000000;
#define NAN (*(double *)&lNaN)
#endif


extern AISTargetAlertDialog *g_pais_alert_dialog_active;
extern Select *pSelectAIS;
extern Select *pSelect;
extern bool bGPSValid;
extern bool g_bCPAMax;
extern double g_CPAMax_NM;
extern bool g_bCPAWarn;
extern double g_CPAWarn_NM;
extern bool g_bTCPA_Max;
extern double g_TCPA_Max;
extern bool g_bMarkLost;
extern double g_MarkLost_Mins;
extern bool g_bRemoveLost;
extern double g_RemoveLost_Mins;
extern double g_AISShowTracks_Mins;
extern bool g_bHideMoored;
extern double g_ShowMoored_Kts;
extern bool g_bAIS_CPA_Alert_Suppress_Moored;
extern bool g_bAIS_ACK_Timeout;
extern double g_AckTimeout_Mins;
extern bool g_bDrawAISSize;
extern bool g_bAllowShowScaled;
extern bool g_bShowScaled;
extern bool g_bInlandEcdis;
extern int g_WplAction;
extern bool g_bAIS_CPA_Alert;
extern bool g_bAIS_CPA_Alert_Audio;

extern ArrayOfMmsiProperties g_MMSI_Props_Array;
extern Route *pAISMOBRoute;
extern wxString AISTargetNameFileName;
extern wxString g_default_wp_icon;
extern std::vector<Track*> g_TrackList;
extern Multiplexer *g_pMUX;
extern AisDecoder *g_pAIS;

extern wxString g_CmdSoundString;

bool g_benableAISNameCache;
bool g_bUseOnlyConfirmedAISName;
wxString GetShipNameFromFile(int);

wxDEFINE_EVENT(EVT_N0183_VDO, ObservedEvt);
wxDEFINE_EVENT(EVT_N0183_VDM, ObservedEvt);
wxDEFINE_EVENT(EVT_N0183_FRPOS, ObservedEvt);
wxDEFINE_EVENT(EVT_N0183_CDDSC, ObservedEvt);
wxDEFINE_EVENT(EVT_N0183_CDDSE, ObservedEvt);
wxDEFINE_EVENT(EVT_N0183_TLL, ObservedEvt);
wxDEFINE_EVENT(EVT_N0183_TTM, ObservedEvt);
wxDEFINE_EVENT(EVT_N0183_OSD, ObservedEvt);
wxDEFINE_EVENT(EVT_SIGNALK, ObservedEvt);
wxDEFINE_EVENT(EVT_N2K_129038, ObservedEvt);
wxDEFINE_EVENT(EVT_N2K_129039, ObservedEvt);
wxDEFINE_EVENT(EVT_N2K_129041, ObservedEvt);
wxDEFINE_EVENT(EVT_N2K_129794, ObservedEvt);
wxDEFINE_EVENT(EVT_N2K_129809, ObservedEvt);
wxDEFINE_EVENT(EVT_N2K_129810, ObservedEvt);
wxDEFINE_EVENT(EVT_N2K_129793, ObservedEvt);

BEGIN_EVENT_TABLE(AisDecoder, wxEvtHandler)
EVT_TIMER(TIMER_AIS1, AisDecoder::OnTimerAIS)
EVT_TIMER(TIMER_DSC, AisDecoder::OnTimerDSC)
END_EVENT_TABLE()

static const double ms_to_knot_factor = 1.9438444924406;

static int n_msgs;
static int n_msg1;
static int n_msg5;
static int n_msg24;
static bool b_firstrx;
static int first_rx_ticks;
static int rx_ticks;
static double arpa_ref_hdg = NAN;

extern const wxEventType wxEVT_OCPN_DATASTREAM;
extern bool g_bquiting;
extern wxString g_DSC_sound_file;
extern wxString g_SART_sound_file;
extern wxString g_AIS_sound_file;
extern bool g_bAIS_GCPA_Alert_Audio;
extern bool g_bAIS_SART_Alert_Audio;
extern bool g_bAIS_DSC_Alert_Audio;

static inline double GeodesicRadToDeg(double rads) {
  return rads * 180.0 / M_PI;
}

static inline double MS2KNOTS(double ms) {
  return ms * 1.9438444924406;
}

int AisMeteoNewMmsi(int, int, int, double, double);

void AISshipNameCache(AisTargetData *pTargetData,
                      AIS_Target_Name_Hash *AISTargetNamesC,
                      AIS_Target_Name_Hash *AISTargetNamesNC, long mmsi);

AisDecoder::AisDecoder(AisDecoderCallbacks callbacks)
    : m_signalk_selfid(""), m_callbacks(callbacks) {
  // Load cached AIS target names from a file
  AISTargetNamesC = new AIS_Target_Name_Hash;
  AISTargetNamesNC = new AIS_Target_Name_Hash;

  if (g_benableAISNameCache) {
    wxTextFile infile;
    if (infile.Open(AISTargetNameFileName)) {
      AIS_Target_Name_Hash *HashFile = AISTargetNamesNC;
      wxString line = infile.GetFirstLine();
      while (!infile.Eof()) {
        if (line.IsSameAs(wxT("+++==Confirmed Entry's==+++")))
          HashFile = AISTargetNamesC;
        else {
          if (line.IsSameAs(wxT("+++==Non Confirmed Entry's==+++")))
            HashFile = AISTargetNamesNC;
          else {
            wxStringTokenizer tokenizer(line, _T(","));
            int mmsi = wxAtoi(tokenizer.GetNextToken());
            wxString name = tokenizer.GetNextToken().Trim();
            (*HashFile)[mmsi] = name;
          }
        }
        line = infile.GetNextLine();
      }
    }
    infile.Close();
  }

  BuildERIShipTypeHash();

  g_pais_alert_dialog_active = NULL;
  m_bAIS_Audio_Alert_On = false;

  m_n_targets = 0;

  m_bAIS_AlertPlaying = false;

  TimerAIS.SetOwner(this, TIMER_AIS1);
  TimerAIS.Start(TIMER_AIS_MSEC, wxTIMER_CONTINUOUS);

  m_ptentative_dsctarget = NULL;
  m_dsc_timer.SetOwner(this, TIMER_DSC);

  //  Create/connect a dynamic event handler slot for wxEVT_OCPN_DATASTREAM(s)
  //FIXME delete Connect(wxEVT_OCPN_DATASTREAM,
  //        (wxObjectEventFunction)(wxEventFunction)&AisDecoder::OnEvtAIS);
//   Connect(EVT_OCPN_SIGNALKSTREAM,
//           (wxObjectEventFunction)(wxEventFunction)&AisDecoder::OnEvtSignalK);
  InitCommListeners();
}

AisDecoder::~AisDecoder(void) {
//   for (const auto &it : GetTargetList()) {
//     AisTargetData *td = it.second;
//
//     delete td;
//   }

  // Write mmsi-shipsname to file in a safe way
  wxTempFile outfile;
  if (outfile.Open(AISTargetNameFileName)) {
    wxString content;
    content = wxT("+++==Confirmed Entry's==+++");
    AIS_Target_Name_Hash::iterator it;
    for (it = AISTargetNamesC->begin(); it != AISTargetNamesC->end(); ++it) {
      content.append(_T("\r\n"));
      content.append(wxString::Format(wxT("%i"), it->first));
      content.append(_T(",")).append(it->second);
    }
    content.append(_T("\r\n"));
    content.append(_T("+++==Non Confirmed Entry's==+++"));
    for (it = AISTargetNamesNC->begin(); it != AISTargetNamesNC->end(); ++it) {
      content.append(_T("\r\n"));
      content.append(wxString::Format(wxT("%i"), it->first));
      content.append(_T(",")).append(it->second);
    }
    outfile.Write(content);
    outfile.Commit();
  }

  AISTargetNamesC->clear();
  delete AISTargetNamesC;
  AISTargetNamesNC->clear();
  delete AISTargetNamesNC;

  clear_hash_ERI();

  m_dsc_timer.Stop();
  m_AIS_Audio_Alert_Timer.Stop();
  TimerAIS.Stop();

#ifdef AIS_DEBUG
  printf(
      "First message[1, 2] ticks: %d  Last Message [1,2]ticks %d  Difference:  "
      "%d\n",
      first_rx_ticks, rx_ticks, rx_ticks - first_rx_ticks);
#endif
}

void AisDecoder::InitCommListeners(void) {
  // Initialize the comm listeners

  auto& msgbus = NavMsgBus::GetInstance();

  //NMEA0183
  //VDM
  Nmea0183Msg n0183_msg_VDM("VDM");
  listener_N0183_VDM.Listen(n0183_msg_VDM, this, EVT_N0183_VDM);
  Bind(EVT_N0183_VDM, [&](ObservedEvt ev) {
        auto ptr = ev.GetSharedPtr();
        auto n0183_msg = std::static_pointer_cast<const Nmea0183Msg>(ptr);
        HandleN0183_AIS( n0183_msg );
      });

  //FRPOS
  Nmea0183Msg n0183_msg_FRPOS("FRPOS");
  listener_N0183_FRPOS.Listen(n0183_msg_FRPOS, this, EVT_N0183_FRPOS);

  Bind(EVT_N0183_FRPOS, [&](ObservedEvt ev) {
        auto ptr = ev.GetSharedPtr();
        auto n0183_msg = std::static_pointer_cast<const Nmea0183Msg>(ptr);
        HandleN0183_AIS( n0183_msg );
      });

  //CDDSC
  Nmea0183Msg n0183_msg_CDDSC("CDDSC");
  listener_N0183_CDDSC.Listen(n0183_msg_CDDSC, this, EVT_N0183_CDDSC);
  Bind(EVT_N0183_CDDSC, [&](ObservedEvt ev) {
        auto ptr = ev.GetSharedPtr();
        auto n0183_msg = std::static_pointer_cast<const Nmea0183Msg>(ptr);
        HandleN0183_AIS( n0183_msg );
      });

  //CDDSE
  Nmea0183Msg n0183_msg_CDDSE("CDDSE");
  listener_N0183_CDDSE.Listen(n0183_msg_CDDSE, this, EVT_N0183_CDDSE);
  Bind(EVT_N0183_CDDSE, [&](ObservedEvt ev) {
        auto ptr = ev.GetSharedPtr();
        auto n0183_msg = std::static_pointer_cast<const Nmea0183Msg>(ptr);
        HandleN0183_AIS( n0183_msg );
      });

  //TLL
  Nmea0183Msg n0183_msg_TLL("TLL");
  listener_N0183_TLL.Listen(n0183_msg_TLL, this, EVT_N0183_TLL);

  Bind(EVT_N0183_TLL, [&](ObservedEvt ev) {
        auto ptr = ev.GetSharedPtr();
        auto n0183_msg = std::static_pointer_cast<const Nmea0183Msg>(ptr);
        HandleN0183_AIS( n0183_msg );
      });

  //TTM
  Nmea0183Msg n0183_msg_ttm("TTM");
  listener_N0183_TTM.Listen(n0183_msg_ttm, this, EVT_N0183_TTM);
  Bind(EVT_N0183_TTM, [&](ObservedEvt ev) {
        auto ptr = ev.GetSharedPtr();
        auto n0183_msg = std::static_pointer_cast<const Nmea0183Msg>(ptr);
        HandleN0183_AIS(n0183_msg);
      });

  //OSD
  Nmea0183Msg n0183_msg_OSD("OSD");
  listener_N0183_OSD.Listen(n0183_msg_OSD, this, EVT_N0183_OSD);
  Bind(EVT_N0183_OSD, [&](ObservedEvt ev) {
        auto ptr = ev.GetSharedPtr();
        auto n0183_msg = std::static_pointer_cast<const Nmea0183Msg>(ptr);
        HandleN0183_AIS( n0183_msg );
      });

  //SignalK
  SignalkMsg sk_msg;
  listener_SignalK.Listen(sk_msg, this, EVT_SIGNALK);
  Bind(EVT_SIGNALK, [&](ObservedEvt ev) {
    HandleSignalK(UnpackEvtPointer<SignalkMsg>(ev));
  });

    // AIS Class A   PGN 129038
  //-----------------------------
  Nmea2000Msg n2k_msg_129038(static_cast<uint64_t>(129038));
  listener_N2K_129038.Listen(n2k_msg_129038, this, EVT_N2K_129038);
  Bind(EVT_N2K_129038, [&](ObservedEvt ev) {
    HandleN2K_129038(UnpackEvtPointer<Nmea2000Msg>(ev));
  });

    // AIS Class B   PGN 129039
  //-----------------------------
  Nmea2000Msg n2k_msg_129039(static_cast<uint64_t>(129039));
  listener_N2K_129039.Listen(n2k_msg_129039, this, EVT_N2K_129039);
  Bind(EVT_N2K_129039, [&](ObservedEvt ev) {
    HandleN2K_129039(UnpackEvtPointer<Nmea2000Msg>(ev));
  });

    // AIS ATON   PGN 129041
  //-----------------------------
  Nmea2000Msg n2k_msg_129041(static_cast<uint64_t>(129041));
  listener_N2K_129041.Listen(n2k_msg_129041, this, EVT_N2K_129041);
  Bind(EVT_N2K_129041, [&](ObservedEvt ev) {
    HandleN2K_129041(UnpackEvtPointer<Nmea2000Msg>(ev));
  });

      // AIS static data class A PGN 129794
  //-----------------------------
  Nmea2000Msg n2k_msg_129794(static_cast<uint64_t>(129794));
  listener_N2K_129794.Listen(n2k_msg_129794, this, EVT_N2K_129794);
  Bind(EVT_N2K_129794, [&](ObservedEvt ev) {
    HandleN2K_129794(UnpackEvtPointer<Nmea2000Msg>(ev));
  });

      // AIS static data class B part A PGN 129809
  //-----------------------------
  Nmea2000Msg n2k_msg_129809(static_cast<uint64_t>(129809));
  listener_N2K_129809.Listen(n2k_msg_129809, this, EVT_N2K_129809);
  Bind(EVT_N2K_129809, [&](ObservedEvt ev) {
    HandleN2K_129809(UnpackEvtPointer<Nmea2000Msg>(ev));
  });

  // AIS static data class B part B PGN 129810
  //-----------------------------
  Nmea2000Msg n2k_msg_129810(static_cast<uint64_t>(129810));
  listener_N2K_129810.Listen(n2k_msg_129810, this, EVT_N2K_129810);
  Bind(EVT_N2K_129810, [&](ObservedEvt ev) {
    HandleN2K_129810(UnpackEvtPointer<Nmea2000Msg>(ev));
  });

  // AIS Base Station report PGN 129793
  //-----------------------------
  Nmea2000Msg n2k_msg_129793(static_cast<uint64_t>(129793));
  listener_N2K_129793.Listen(n2k_msg_129793, this, EVT_N2K_129793);
  Bind(EVT_N2K_129793, [&](ObservedEvt ev) {
    HandleN2K_129793(UnpackEvtPointer<Nmea2000Msg>(ev));
  });

}


bool AisDecoder::HandleN0183_AIS( std::shared_ptr <const Nmea0183Msg> n0183_msg ){
  std::string str = n0183_msg->payload;
  wxString sentence(str.c_str());
  DecodeN0183(sentence);
  touch_state.Notify();
  return true;
}

bool AisDecoder::HandleN2K_129038( std::shared_ptr<const Nmea2000Msg> n2k_msg ){
  std::vector<unsigned char> v = n2k_msg->payload;

  uint8_t MessageID;
  tN2kAISRepeat Repeat;
  uint32_t UserID;
  double Latitude;
  double Longitude;
  bool Accuracy;
  bool RAIM;
  uint8_t Seconds;
  double COG;
  double SOG;
  double Heading;
  double ROT;
  tN2kAISNavStatus NavStat = N2kaisns_Under_Way_Motoring;
  tN2kAISTransceiverInformation AISTransceiverInformation;


  if (ParseN2kPGN129038(v, MessageID, Repeat, UserID,
                        Latitude, Longitude, Accuracy, RAIM, Seconds,
                        COG, SOG, Heading, ROT, NavStat, AISTransceiverInformation)) {

    // Is this target already in the global target list?
    //  Search the current AISTargetList for an MMSI match
    int mmsi = UserID;
    long mmsi_long = mmsi;
    std::shared_ptr<AisTargetData>pTargetData = 0;
    bool bnewtarget = false;

    auto it = AISTargetList.find(mmsi);
    if (it == AISTargetList.end())  // not found
    {
      pTargetData = AisTargetDataMaker::GetInstance().GetTargetData();
      bnewtarget = true;
      m_n_targets++;
    } else {
      pTargetData = it->second;    // find current entry
    }

    wxDateTime now = wxDateTime::Now();
    now.MakeUTC();

    //Populate the target_data
    pTargetData->MMSI = mmsi;
    pTargetData->MID = MessageID;
    pTargetData->MMSI = mmsi;
    pTargetData->Class = AIS_CLASS_A;
    //    Check for SART and friends by looking at first two digits of MMSI
    if( 97 == pTargetData->MMSI / 10000000) {
      pTargetData->Class = AIS_SART;
      // won't get a static report, so fake it here
      pTargetData->StaticReportTicks = now.GetTicks();
    }
    pTargetData->NavStatus = (ais_nav_status)NavStat;
    if (!N2kIsNA(SOG)) pTargetData->SOG = MS2KNOTS(SOG);
    if (!N2kIsNA(COG)) pTargetData->COG = GeodesicRadToDeg(COG);
    if (!N2kIsNA(Heading)) pTargetData->HDG = GeodesicRadToDeg(Heading);
    if (!N2kIsNA(Longitude)) pTargetData->Lon = Longitude;
    if (!N2kIsNA(Latitude)) pTargetData->Lat = Latitude;

    pTargetData->ROTAIS = ROT;

    double rot_dir = 1.0;

    //FIXME (dave)
//     if (ROT == 128)
//       pTargetData->ROTAIS = -128;  // not available codes as -128
//     else if ((ROT & 0x80) == 0x80) {
//       pTargetData->ROTAIS = ROT - 256;  // convert to twos complement
//       rot_dir = -1.0;
//     }
//
//     pTargetData->ROTIND = round(rot_dir * pow((ROT / 4.733), 2));

    pTargetData->b_OwnShip =
        AISTransceiverInformation == tN2kAISTransceiverInformation::N2kaisown_information_not_broadcast;
    pTargetData->b_active = true;
    pTargetData->b_lost = false;
    pTargetData->b_positionOnceValid = true;
    pTargetData->PositionReportTicks = now.GetTicks();

    pSelectAIS->DeleteSelectablePoint((void *)(long)mmsi, SELTYPE_AISTARGET);
    CommitAISTarget(pTargetData, "", true, bnewtarget);

    touch_state.Notify();
    return true;
  }
  else
    return false;
}

  // AIS position reports for Class B
bool AisDecoder::HandleN2K_129039( std::shared_ptr<const Nmea2000Msg> n2k_msg ){
  std::vector<unsigned char> v = n2k_msg->payload;

// Input:
//  - N2kMsg                NMEA2000 message to decode
// bool ParseN2kPGN129039(std::vector<unsigned char> &v, uint8_t &MessageID, tN2kAISRepeat &Repeat, uint32_t &UserID,
//                         double &Latitude, double &Longitude, bool &Accuracy, bool &RAIM, uint8_t &Seconds, double &COG,
//                         double &SOG, tN2kAISTransceiverInformation &AISTransceiverInformation, double &Heading,
//                         tN2kAISUnit &Unit, bool &Display, bool &DSC, bool &Band, bool &Msg22, tN2kAISMode &Mode, bool &State);

  uint8_t MessageID;
  tN2kAISRepeat Repeat;
  uint32_t UserID;
  double Latitude;
  double Longitude;
  bool Accuracy;
  bool RAIM;
  uint8_t Seconds;
  double COG;
  double SOG;
  double Heading;
  tN2kAISNavStatus NavStat = N2kaisns_Under_Way_Motoring;
  tN2kAISTransceiverInformation AISTransceiverInformation;
  tN2kAISUnit Unit;
  bool DSC, Band, Msg22, State, Display;
  tN2kAISMode Mode;

  if (ParseN2kPGN129039(v, MessageID, Repeat, UserID,
                         Latitude, Longitude, Accuracy, RAIM, Seconds, COG,
                         SOG, AISTransceiverInformation, Heading,
                         Unit, Display, DSC, Band, Msg22, Mode, State)) {

    // Is this target already in the global target list?
    //  Search the current AISTargetList for an MMSI match
    int mmsi = UserID;
    long mmsi_long = mmsi;
    std::shared_ptr<AisTargetData> pTargetData = 0;
    bool bnewtarget = false;

    auto it = AISTargetList.find(mmsi);
    if (it == AISTargetList.end())  // not found
    {
      pTargetData = AisTargetDataMaker::GetInstance().GetTargetData();
      bnewtarget = true;
      m_n_targets++;
    } else {
      pTargetData = it->second;    // find current entry
    }

    wxDateTime now = wxDateTime::Now();
    now.MakeUTC();

    //Populate the target_data
    pTargetData->MMSI = mmsi;
    pTargetData->MID = MessageID;
    pTargetData->MMSI = mmsi;
    pTargetData->Class = AIS_CLASS_B;
    pTargetData->NavStatus = (ais_nav_status)NavStat;
    if (!N2kIsNA(SOG)) pTargetData->SOG = MS2KNOTS(SOG);
    if (!N2kIsNA(COG)) pTargetData->COG = GeodesicRadToDeg(COG);
    if (!N2kIsNA(Heading)) pTargetData->HDG = GeodesicRadToDeg(Heading);
    if(!N2kIsNA(Longitude)) pTargetData->Lon = Longitude;
    if (!N2kIsNA(Latitude)) pTargetData->Lat = Latitude;

    pTargetData->b_positionOnceValid = true;
    pTargetData->b_active = true;
    pTargetData->b_lost = false;
    pTargetData->PositionReportTicks = now.GetTicks();
    pTargetData->b_OwnShip =
        AISTransceiverInformation == tN2kAISTransceiverInformation::N2kaisown_information_not_broadcast;

    pSelectAIS->DeleteSelectablePoint((void *)(long)mmsi, SELTYPE_AISTARGET);
    CommitAISTarget(pTargetData, "", true, bnewtarget);

    touch_state.Notify();
    return true;
  }
  else
    return false;


  return true;
}

bool AisDecoder::HandleN2K_129041( std::shared_ptr<const Nmea2000Msg> n2k_msg ){
  std::vector<unsigned char> v = n2k_msg->payload;

  tN2kAISAtoNReportData data;

#if 0
  struct tN2kAISAtoNReportData {
  uint8_t MessageID;
  tN2kAISRepeat Repeat;
  uint32_t UserID;
  double Longitude;
  double Latitude;
  bool Accuracy;
  bool RAIM;
  uint8_t Seconds;
  double Length;
  double Beam;
  double PositionReferenceStarboard ;
  double PositionReferenceTrueNorth;
  tN2kAISAtoNType AtoNType;
  bool OffPositionIndicator;
  bool VirtualAtoNFlag;
  bool AssignedModeFlag;
  tN2kGNSStype GNSSType;
  uint8_t AtoNStatus;
  tN2kAISTransceiverInformation AISTransceiverInformation;
  char AtoNName[34 + 1];
#endif

  if (ParseN2kPGN129041(v, data)){
    // Is this target already in the global target list?
    //  Search the current AISTargetList for an MMSI match
    int mmsi = data.UserID;
    long mmsi_long = mmsi;
    std::shared_ptr<AisTargetData> pTargetData = 0;
    bool bnewtarget = false;

    auto it = AISTargetList.find(mmsi);
    if (it == AISTargetList.end())  // not found
    {
      pTargetData = AisTargetDataMaker::GetInstance().GetTargetData();
      bnewtarget = true;
      m_n_targets++;
    } else {
      pTargetData = it->second;    // find current entry
    }

    //Populate the target_data
    pTargetData->MMSI = mmsi;

    wxDateTime now = wxDateTime::Now();
    now.MakeUTC();

    int offpos = data.OffPositionIndicator;  // off position flag
    int virt = data.VirtualAtoNFlag;    // virtual flag

    if (virt)
      pTargetData->NavStatus = ATON_VIRTUAL;
    else
      pTargetData->NavStatus = ATON_REAL;

    pTargetData->m_utc_sec = data.Seconds;

    if (pTargetData->m_utc_sec <= 59 ){
        pTargetData->NavStatus += 1;
        if (offpos) pTargetData->NavStatus += 1;
    }

    data.AtoNName[34] = 0;
    strncpy(pTargetData->ShipName, data.AtoNName, SHIP_NAME_LEN - 1);
    pTargetData->b_nameValid = true;
    pTargetData->MID = 124;  // Indicates a name from n2k
    pTargetData->Class = AIS_ATON;

    if (!N2kIsNA(data.Longitude)) pTargetData->Lon = data.Longitude;
    if (!N2kIsNA(data.Latitude)) pTargetData->Lat = data.Latitude;
    pTargetData->b_positionDoubtful = false;
    pTargetData->b_positionOnceValid = true;  // Got the position at least once
    pTargetData->PositionReportTicks = now.GetTicks();

    //FIXME (dave) Populate more fiddly static data

    pSelectAIS->DeleteSelectablePoint((void *)(long)mmsi, SELTYPE_AISTARGET);
    CommitAISTarget(pTargetData, "", true, bnewtarget);

    touch_state.Notify();
    return true;
  }
  else
    return false;
}

//AIS static data class A
bool AisDecoder::HandleN2K_129794( std::shared_ptr<const Nmea2000Msg> n2k_msg ){
  std::vector<unsigned char> v = n2k_msg->payload;

  uint8_t MessageID;
  tN2kAISRepeat Repeat;
  uint32_t UserID;
  uint32_t IMOnumber;
  char Callsign[21];
  char Name[SHIP_NAME_LEN];
  uint8_t VesselType;
  double Length;
  double Beam;
  double PosRefStbd;
  double PosRefBow;
  uint16_t ETAdate;
  double ETAtime;
  double Draught;
  char Destination[DESTINATION_LEN];
  tN2kAISVersion AISversion;
  tN2kGNSStype GNSStype;
  tN2kAISDTE DTE;
  tN2kAISTranceiverInfo AISinfo;


  if (ParseN2kPGN129794(v, MessageID, Repeat, UserID,
                        IMOnumber, Callsign, Name, VesselType, Length,
                        Beam, PosRefStbd, PosRefBow, ETAdate, ETAtime,
                        Draught, Destination, AISversion, GNSStype,
                        DTE, AISinfo) )
    {
    // Is this target already in the global target list?
    //  Search the current AISTargetList for an MMSI match
    int mmsi = UserID;
    long mmsi_long = mmsi;
    std::shared_ptr<AisTargetData> pTargetData = 0;
    bool bnewtarget = false;

    auto it = AISTargetList.find(mmsi);
    if (it == AISTargetList.end())  // not found
    {
      pTargetData = AisTargetDataMaker::GetInstance().GetTargetData();
      bnewtarget = true;
      m_n_targets++;
    } else {
      pTargetData = it->second;    // find current entry
    }

    //Populate the target_data
    pTargetData->MMSI = mmsi;
    Name[sizeof(Name) - 1] = 0;
    strncpy(pTargetData->ShipName, Name, SHIP_NAME_LEN - 1);
    pTargetData->b_nameValid = true;
    pTargetData->MID = 124;  // Indicates a name from n2k

    pTargetData->b_OwnShip =
      AISinfo == tN2kAISTranceiverInfo::N2kaisti_Own_information_not_broadcast;

    pTargetData->DimA = PosRefBow;
    pTargetData->DimB = Length - PosRefBow;
    pTargetData->DimC = Beam - PosRefStbd;
    pTargetData->DimD = PosRefStbd;
    pTargetData->Draft = Draught;
    pTargetData->IMO = IMOnumber;
    strncpy(pTargetData->CallSign, Callsign, CALL_SIGN_LEN - 1);
    pTargetData->ShipType = (unsigned char)VesselType;
    Destination[sizeof(Destination) - 1] = 0;
    strncpy(pTargetData->Destination, Destination, DESTINATION_LEN - 1);

    if (!N2kIsNA(ETAdate) && !N2kIsNA(ETAtime)) {
      long secs = (ETAdate * 24 * 3600) + wxRound(ETAtime);
      wxDateTime t((time_t)secs);
      if (t.IsValid()) {
        wxDateTime tz = t.ToUTC();
        pTargetData->ETA_Mo = tz.GetMonth() + 1;
        pTargetData->ETA_Day = tz.GetDay();
        pTargetData->ETA_Hr = tz.GetHour();
        pTargetData->ETA_Min = tz.GetMinute();
      }
    }

    pSelectAIS->DeleteSelectablePoint((void *)(long)mmsi, SELTYPE_AISTARGET);
    CommitAISTarget(pTargetData, "", true, bnewtarget);

    touch_state.Notify();
    return true;
  }
  else
    return false;
}
// AIS static data class B part A
bool AisDecoder::HandleN2K_129809( std::shared_ptr<const Nmea2000Msg> n2k_msg ){
  std::vector<unsigned char> v = n2k_msg->payload;

  uint8_t MessageID;
  tN2kAISRepeat Repeat;
  uint32_t UserID;
  char Name[21];

  if (ParseN2kPGN129809(v, MessageID, Repeat, UserID, Name))
  {
    // Is this target already in the global target list?
    //  Search the current AISTargetList for an MMSI match
    int mmsi = UserID;
    long mmsi_long = mmsi;
    std::shared_ptr<AisTargetData> pTargetData = 0;
    bool bnewtarget = false;

    auto it = AISTargetList.find(mmsi);
    if (it == AISTargetList.end())  // not found
    {
      pTargetData = AisTargetDataMaker::GetInstance().GetTargetData();
      bnewtarget = true;
      m_n_targets++;
    } else {
      pTargetData = it->second;    // find current entry
    }

    //Populate the target_data
    pTargetData->MMSI = mmsi;
    Name[sizeof(Name) - 1] = 0;
    strncpy(pTargetData->ShipName, Name, SHIP_NAME_LEN - 1);
    pTargetData->b_nameValid = true;
    pTargetData->MID = 124;  // Indicates a name from n2k

    pSelectAIS->DeleteSelectablePoint((void *)(long)mmsi, SELTYPE_AISTARGET);
    CommitAISTarget(pTargetData, "", true, bnewtarget);

    touch_state.Notify();
    return true;

  }
  else
    return false;
}


// AIS static data class B part B
bool AisDecoder::HandleN2K_129810( std::shared_ptr<const Nmea2000Msg> n2k_msg ){
  std::vector<unsigned char> v = n2k_msg->payload;

  uint8_t MessageID;
  tN2kAISRepeat Repeat;
  uint32_t UserID;
  uint8_t VesselType;
  char Vendor[20];
  char Callsign[20];
  double Length;
  double Beam;
  double PosRefStbd;
  double PosRefBow;
  uint32_t MothershipID;

  if (ParseN2kPGN129810(v, MessageID, Repeat, UserID,
                      VesselType, Vendor, Callsign, Length, Beam,
                      PosRefStbd, PosRefBow, MothershipID))
  {
    // Is this target already in the global target list?
    //  Search the current AISTargetList for an MMSI match
    int mmsi = UserID;
    long mmsi_long = mmsi;
    std::shared_ptr<AisTargetData> pTargetData = 0;
    bool bnewtarget = false;

    auto it = AISTargetList.find(mmsi);
    if (it == AISTargetList.end())  // not found
    {
      pTargetData = AisTargetDataMaker::GetInstance().GetTargetData();
      bnewtarget = true;
      m_n_targets++;
    } else {
      pTargetData = it->second;    // find current entry
    }

    //Populate the target_data
    pTargetData->MMSI = mmsi;
    pTargetData->DimA = PosRefBow;
    pTargetData->DimB = Length - PosRefBow;
    pTargetData->DimC = Beam - PosRefStbd;
    pTargetData->DimD = PosRefStbd;
    strncpy(pTargetData->CallSign, Callsign, CALL_SIGN_LEN - 1);
    pTargetData->ShipType = (unsigned char)VesselType;

    pSelectAIS->DeleteSelectablePoint((void *)(long)mmsi, SELTYPE_AISTARGET);
    CommitAISTarget(pTargetData, "", true, bnewtarget);

    touch_state.Notify();
    return true;
  }
  else
    return false;
}

// AIS Base Station Report
bool AisDecoder::HandleN2K_129793( std::shared_ptr<const Nmea2000Msg> n2k_msg ){
  std::vector<unsigned char> v = n2k_msg->payload;

  uint8_t MessageID;
  tN2kAISRepeat Repeat;
  uint32_t UserID;
  double Longitude;
  double Latitude;
  unsigned int SecondsSinceMidnight;
  unsigned int DaysSinceEpoch;

  if (ParseN2kPGN129793(v, MessageID, Repeat, UserID,
                        Longitude, Latitude,
                        SecondsSinceMidnight, DaysSinceEpoch))
  {
    wxDateTime now = wxDateTime::Now();
    now.MakeUTC();

    // Is this target already in the global target list?
    //  Search the current AISTargetList for an MMSI match
    int mmsi = UserID;
    long mmsi_long = mmsi;
    std::shared_ptr<AisTargetData> pTargetData = 0;
    bool bnewtarget = false;

    auto it = AISTargetList.find(mmsi);
    if (it == AISTargetList.end())  // not found
    {
      pTargetData = AisTargetDataMaker::GetInstance().GetTargetData();
      bnewtarget = true;
      m_n_targets++;
    } else {
      pTargetData = it->second;    // find current entry
    }

    //Populate the target_data
    pTargetData->MMSI = mmsi;
    pTargetData->Class = AIS_BASE;

    if (!N2kIsNA(Longitude)) pTargetData->Lon = Longitude;
    if (!N2kIsNA(Latitude)) pTargetData->Lat = Latitude;
    pTargetData->b_positionDoubtful = false;
    pTargetData->b_positionOnceValid = true;  // Got the position at least once
    pTargetData->PositionReportTicks = now.GetTicks();


      //FIXME (dave) Populate more fiddly static data

    pSelectAIS->DeleteSelectablePoint((void *)(long)mmsi, SELTYPE_AISTARGET);
    CommitAISTarget(pTargetData, "", true, bnewtarget);

    touch_state.Notify();
    return true;
  }
  else
    return false;
}

// void AisDecoder::HandleSignalK(std::shared_ptr<const SignalkMsg> sK_msg){
//   std::string msgTerminated = sK_msg->raw_message;;
//   wxString sentence(msgTerminated.c_str());
//   Decode(sentence);
//   touch_state.notify();
//   return true;
// }

void AisDecoder::BuildERIShipTypeHash(void) {
  make_hash_ERI(8000, _("Vessel, type unknown"));
  make_hash_ERI(8150, _("Freightbarge"));
  make_hash_ERI(8160, _("Tankbarge"));
  make_hash_ERI(8163, _("Tankbarge, dry cargo as if liquid (e.g. cement)"));
  make_hash_ERI(8450, _("Service vessel, police patrol, port service"));
  make_hash_ERI(8430, _("Pushboat, single"));
  make_hash_ERI(8510, _("Object, not otherwise specified"));
  make_hash_ERI(8470, _("Object, towed, not otherwise specified"));
  make_hash_ERI(8490, _("Bunkership"));
  make_hash_ERI(8010, _("Motor freighter"));
  make_hash_ERI(8020, _("Motor tanker"));
  make_hash_ERI(8021, _("Motor tanker, liquid cargo, type N"));
  make_hash_ERI(8022, _("Motor tanker, liquid cargo, type C"));
  make_hash_ERI(8023, _("Motor tanker, dry cargo as if liquid (e.g. cement)"));
  make_hash_ERI(8030, _("Container vessel"));
  make_hash_ERI(8040, _("Gas tanker"));
  make_hash_ERI(8050, _("Motor freighter, tug"));
  make_hash_ERI(8060, _("Motor tanker, tug"));
  make_hash_ERI(8070, _("Motor freighter with one or more ships alongside"));
  make_hash_ERI(8080, _("Motor freighter with tanker"));
  make_hash_ERI(8090, _("Motor freighter pushing one or more freighters"));
  make_hash_ERI(8100, _("Motor freighter pushing at least one tank-ship"));
  make_hash_ERI(8110, _("Tug, freighter"));
  make_hash_ERI(8120, _("Tug, tanker"));
  make_hash_ERI(8130, _("Tug freighter, coupled"));
  make_hash_ERI(8140, _("Tug, freighter/tanker, coupled"));
  make_hash_ERI(8161, _("Tankbarge, liquid cargo, type N"));
  make_hash_ERI(8162, _("Tankbarge, liquid cargo, type C"));
  make_hash_ERI(8170, _("Freightbarge with containers"));
  make_hash_ERI(8180, _("Tankbarge, gas"));
  make_hash_ERI(8210, _("Pushtow, one cargo barge"));
  make_hash_ERI(8220, _("Pushtow, two cargo barges"));
  make_hash_ERI(8230, _("Pushtow, three cargo barges"));
  make_hash_ERI(8240, _("Pushtow, four cargo barges"));
  make_hash_ERI(8250, _("Pushtow, five cargo barges"));
  make_hash_ERI(8260, _("Pushtow, six cargo barges"));
  make_hash_ERI(8270, _("Pushtow, seven cargo barges"));
  make_hash_ERI(8280, _("Pushtow, eight cargo barges"));
  make_hash_ERI(8290, _("Pushtow, nine or more barges"));
  make_hash_ERI(8310, _("Pushtow, one tank/gas barge"));
  make_hash_ERI(8320,
                _("Pushtow, two barges at least one tanker or gas barge"));
  make_hash_ERI(8330,
                _("Pushtow, three barges at least one tanker or gas barge"));
  make_hash_ERI(8340,
                _("Pushtow, four barges at least one tanker or gas barge"));
  make_hash_ERI(8350,
                _("Pushtow, five barges at least one tanker or gas barge"));
  make_hash_ERI(8360,
                _("Pushtow, six barges at least one tanker or gas barge"));
  make_hash_ERI(8370,
                _("Pushtow, seven barges at least one tanker or gas barge"));
  make_hash_ERI(8380,
                _("Pushtow, eight barges at least one tanker or gas barge"));
  make_hash_ERI(
      8390, _("Pushtow, nine or more barges at least one tanker or gas barge"));
  make_hash_ERI(8400, _("Tug, single"));
  make_hash_ERI(8410, _("Tug, one or more tows"));
  make_hash_ERI(8420, _("Tug, assisting a vessel or linked combination"));
  make_hash_ERI(8430, _("Pushboat, single"));
  make_hash_ERI(8440, _("Passenger ship, ferry, cruise ship, red cross ship"));
  make_hash_ERI(8441, _("Ferry"));
  make_hash_ERI(8442, _("Red cross ship"));
  make_hash_ERI(8443, _("Cruise ship"));
  make_hash_ERI(8444, _("Passenger ship without accommodation"));
  make_hash_ERI(8460, _("Vessel, work maintenance craft, floating derrick, "
                        "cable-ship, buoy-ship, dredge"));
  make_hash_ERI(8480, _("Fishing boat"));
  make_hash_ERI(8500, _("Barge, tanker, chemical"));
  make_hash_ERI(1500, _("General cargo Vessel maritime"));
  make_hash_ERI(1510, _("Unit carrier maritime"));
  make_hash_ERI(1520, _("Bulk carrier maritime"));
  make_hash_ERI(1530, _("Tanker"));
  make_hash_ERI(1540, _("Liquified gas tanker"));
  make_hash_ERI(1850, _("Pleasure craft, longer than 20 metres"));
  make_hash_ERI(1900, _("Fast ship"));
  make_hash_ERI(1910, _("Hydrofoil"));
}

#if 0
//FIXME delete
//----------------------------------------------------------------------------------
//     Handle events from AIS DataStream
//----------------------------------------------------------------------------------
void AisDecoder::OnEvtAIS(OCPN_DataStreamEvent &event) {
  wxString message = event.ProcessNMEA4Tags();

  int nr = 0;
  if (!message.IsEmpty()) {
    if (message.Mid(3, 3).IsSameAs(_T("VDM")) ||
        message.Mid(3, 3).IsSameAs(_T("VDO")) ||
        message.Mid(1, 5).IsSameAs(_T("FRPOS")) ||
        message.Mid(1, 2).IsSameAs(_T("CD")) ||
        message.Mid(3, 3).IsSameAs(_T("TLL")) ||
        message.Mid(3, 3).IsSameAs(_T("TTM")) ||
        message.Mid(3, 3).IsSameAs(_T("OSD")) ||
        (g_bWplUsePosition && message.Mid(3, 3).IsSameAs(_T("WPL")))) {
      nr = Decode(message);
      if (nr == AIS_NoError) {
        g_pi_manager->SendAISSentenceToAllPlugIns(message);
      }
      gFrame->TouchAISActive();
    }
  }
}
#endif

//----------------------------------------------------------------------------------
//     Handle events from SignalK
//----------------------------------------------------------------------------------
void AisDecoder::HandleSignalK(std::shared_ptr<const SignalkMsg> sK_msg){
  rapidjson::Document root;

  root.Parse(sK_msg->raw_message);

  if (root.HasParseError()) return;

  if (root.HasMember("self")) {
    // m_signalk_selfid = _T("vessels.") + (root["self"].AsString());
    m_signalk_selfid =
        (root["self"]
             .GetString());  // Verified for OpenPlotter node.js server 1.20
  }
  if (m_signalk_selfid.IsEmpty()) {
    return;  // Don't handle any messages (with out self) until we know how we
             // are
  }
  long mmsi = 0;
  if (root.HasMember("context") && root["context"].IsString()) {
    wxString context = root["context"].GetString();
    if (context == m_signalk_selfid) {
#if 0
            wxLogMessage(_T("** Ignore context own ship.."));
#endif
      return;
    }
    wxString mmsi_string;
    if (context.StartsWith(_T("vessels.urn:mrn:imo:mmsi:"), &mmsi_string) ||
        context.StartsWith(_T("atons.urn:mrn:imo:mmsi:"), &mmsi_string) ||
        context.StartsWith(_T("aircraft.urn:mrn:imo:mmsi:"), &mmsi_string)) {
      //    wxLogMessage(wxString::Format(_T("Context: %s, %s"),
      //    context.c_str(), mmsi_string));
      if (mmsi_string.ToLong(&mmsi)) {
        // wxLogMessage(_T("Got MMSI from context."));
      } else {
        mmsi = 0;
      }
    }
  }
  if (mmsi == 0) {
    return;  // Only handle ships with MMSI for now
  }
  // Stop here if the target shall be ignored
  for (unsigned int i = 0; i < g_MMSI_Props_Array.GetCount(); i++) {
    if (mmsi == g_MMSI_Props_Array[i]->MMSI) {
      MmsiProperties * props = g_MMSI_Props_Array[i];
      if (props->m_bignore) {
        return;
      }
    }
  }
#if 0
    wxString dbg;
    wxJSONWriter writer;
    writer.Write(root, dbg);

    wxString msg( _T("AisDecoder::OnEvtSignalK: ") );
    msg.append(dbg);
    wxLogMessage(msg);
#endif
  std::shared_ptr<AisTargetData> pTargetData = 0;
  std::shared_ptr<AisTargetData> pStaleTarget = NULL;
  bool bnewtarget = false;
  int last_report_ticks;
  wxDateTime now;
  getAISTarget(mmsi, pTargetData, pStaleTarget, bnewtarget, last_report_ticks,
               now);
  if (pTargetData) {
    getMmsiProperties(pTargetData);
    if (root.HasMember("updates") && root["updates"].IsArray()) {
      for (rapidjson::Value::ConstValueIterator itr = root["updates"].Begin(); itr != root["updates"].End(); ++itr) {
        handleUpdate(pTargetData, bnewtarget, *itr);
      }
    }
    pTargetData->MMSI = mmsi;
    // A SART can send wo any values first transmits. Detect class already here.
    if (97 == mmsi / 10000000) {
      pTargetData->Class = AIS_SART;
    }
    pTargetData->b_OwnShip = false;
    AISTargetList[pTargetData->MMSI] = pTargetData;
  }
}

void AisDecoder::handleUpdate(std::shared_ptr<AisTargetData> pTargetData, bool bnewtarget,
                               const rapidjson::Value &update) {
  wxString sfixtime = "";

  if (update.HasMember("timestamp")) {
    sfixtime = update["timestamp"].GetString();
  }
  if (update.HasMember("values") && update["values"].IsArray()) {
    for (rapidjson::Value::ConstValueIterator itr = update["values"].Begin(); itr != update["values"].End(); ++itr) {
      updateItem(pTargetData, bnewtarget, *itr, sfixtime);
    }
  }
  wxDateTime now = wxDateTime::Now();
  pTargetData->m_utc_hour = now.ToUTC().GetHour();
  pTargetData->m_utc_min = now.ToUTC().GetMinute();
  pTargetData->m_utc_sec = now.ToUTC().GetSecond();
  // pTargetData->NavStatus = 15; // undefined
  pTargetData->b_active = true;
  pTargetData->b_lost = false;

  if (pTargetData->b_positionOnceValid) {
    long mmsi_long = pTargetData->MMSI;
    SelectItem *pSel =
        pSelectAIS->AddSelectablePoint(pTargetData->Lat, pTargetData->Lon,
                                       (void *)mmsi_long, SELTYPE_AISTARGET);
    pSel->SetUserData(pTargetData->MMSI);
  }
  UpdateOneCPA(pTargetData.get());
  if (pTargetData->b_show_track) UpdateOneTrack(pTargetData.get());
}

void AisDecoder::updateItem(std::shared_ptr<AisTargetData> pTargetData, bool bnewtarget,
                             const rapidjson::Value &item, wxString &sfixtime) const {
  if (item.HasMember("path") && item.HasMember("value")) {
    const wxString &update_path = item["path"].GetString();
    if (update_path == _T("navigation.position")) {
      if (item["value"].HasMember("latitude") && item["value"].HasMember("longitude")) {
        wxDateTime now = wxDateTime::Now();
        now.MakeUTC();
        double lat = item["value"]["latitude"].GetDouble();
        double lon = item["value"]["longitude"].GetDouble();
        pTargetData->PositionReportTicks = now.GetTicks();
        pTargetData->StaticReportTicks = now.GetTicks();
        pTargetData->Lat = lat;
        pTargetData->Lon = lon;
        pTargetData->b_positionOnceValid = true;
        pTargetData->b_positionDoubtful = false;
      }

      if (item["value"].HasMember("altitude")) {
        pTargetData->altitude = item["value"]["altitude "].GetInt();
      }
    } else if (update_path == _T("navigation.speedOverGround") && item["value"].IsNumber()) {
      pTargetData->SOG = item["value"].GetDouble() * ms_to_knot_factor;
    } else if (update_path == _T("navigation.courseOverGroundTrue") && item["value"].IsNumber()) {
      pTargetData->COG = GEODESIC_RAD2DEG(item["value"].GetDouble());
    } else if (update_path == _T("navigation.headingTrue") && item["value"].IsNumber()) {
      pTargetData->HDG = GEODESIC_RAD2DEG(item["value"].GetDouble());
    } else if (update_path == _T("navigation.rateOfTurn") && item["value"].IsNumber()) {
      pTargetData->ROTAIS = 4.733 * sqrt(item["value"].GetDouble());
    } else if (update_path == _T("design.aisShipType")) {
      if (item["value"].HasMember("id")) {
        if (!pTargetData->b_isDSCtarget) {
          pTargetData->ShipType = item["value"]["id"].GetUint();
        }
      }
    } else if (update_path == _T("atonType")) {
      if (item["value"].HasMember("id")) {
        pTargetData->ShipType = item["value"]["id"].GetUint();
      }
    } else if (update_path == _T("virtual")) {
      if (item["value"].GetBool()) {
        pTargetData->NavStatus = ATON_VIRTUAL;
      } else {
        pTargetData->NavStatus = ATON_REAL;
      }
    } else if (update_path == _T("offPosition")) {
      if (item["value"].GetBool()) {
        if (ATON_REAL == pTargetData->NavStatus) {
          pTargetData->NavStatus = ATON_REAL_OFFPOSITION;
        } else if (ATON_VIRTUAL == pTargetData->NavStatus) {
          pTargetData->NavStatus = ATON_VIRTUAL_OFFPOSITION;
        }
      }
    } else if (update_path == _T("design.draft")) {
      if (item["value"].HasMember("maximum") && item["value"].IsNumber()) {
        pTargetData->Draft = item["value"]["maximum"].GetDouble();
        pTargetData->Euro_Draft = item["value"]["maximum"].GetDouble();
      }
      if (item["value"].HasMember("current") && item["value"].IsNumber()) {
        double draft = item["value"]["current"].GetDouble();
        if (draft > 0) {
          pTargetData->Draft = draft;
          pTargetData->Euro_Draft = draft;
        }
      }
    } else if (update_path == _T("design.length")) {
      if (pTargetData->DimB == 0) {
        if (item["value"].HasMember("overall")) {
          if (item["value"]["overall"].IsNumber()) {
            pTargetData->Euro_Length = item["value"]["overall"].GetDouble();
            pTargetData->DimA = item["value"]["overall"].GetDouble();
          }
          pTargetData->DimB = 0;
        }
      }
    } else if (update_path == _T("sensors.ais.class")) {
      wxString aisclass = item["value"].GetString();
      if (aisclass == _T("A")) {
        if(!pTargetData->b_isDSCtarget)
          pTargetData->Class = AIS_CLASS_A;
      } else if (aisclass == _T("B")) {
        if (!pTargetData->b_isDSCtarget)
          pTargetData->Class = AIS_CLASS_B;
        pTargetData->NavStatus =
            UNDEFINED;  // Class B targets have no status.  Enforce this...
      } else if (aisclass == _T("BASE")) {
        pTargetData->Class = AIS_BASE;
      } else if (aisclass == _T("ATON")) {
        pTargetData->Class = AIS_ATON;
      }
    } else if (update_path == _T("sensors.ais.fromBow")) {
      if (pTargetData->DimB == 0 && pTargetData->DimA != 0) {
        int length = pTargetData->DimA;
        if (item["value"].IsNumber()) {
          pTargetData->DimA = item["value"].GetDouble();
          pTargetData->DimB = length - item["value"].GetDouble();
        }
      }
    } else if (update_path == _T("design.beam")) {
      if (pTargetData->DimD == 0) {
        if (item["value"].IsNumber()) {
          pTargetData->Euro_Beam = item["value"].GetDouble();
          pTargetData->DimC = item["value"].GetDouble();
        }
        pTargetData->DimD = 0;
      }
    } else if (update_path == _T("sensors.ais.fromCenter")) {
      if (pTargetData->DimD == 0 && pTargetData->DimC != 0) {
        int beam = pTargetData->DimC;
        int center = beam / 2;
        if (item["value"].IsNumber()) {
          //FIXME (nohal): Dim* are int, but we have seen data streams with doubles in them...
          pTargetData->DimC = center + item["value"].GetDouble();
          pTargetData->DimD = beam - pTargetData->DimC;
        }
      }
    } else if (update_path == _T("navigation.state")) {
      wxString state = item["value"].GetString();
      if (state == _T("motoring")) {
        pTargetData->NavStatus = UNDERWAY_USING_ENGINE;
      } else if (state == _T("anchored")) {
        pTargetData->NavStatus = AT_ANCHOR;
      } else if (state == _T("not under command")) {
        pTargetData->NavStatus = NOT_UNDER_COMMAND;
      } else if (state == _T("restricted manouverability")) {
        pTargetData->NavStatus = RESTRICTED_MANOEUVRABILITY;
      } else if (state == _T("constrained by draft")) {
        pTargetData->NavStatus = CONSTRAINED_BY_DRAFT;
      } else if (state == _T("moored")) {
        pTargetData->NavStatus = MOORED;
      } else if (state == _T("aground")) {
        pTargetData->NavStatus = AGROUND;
      } else if (state == _T("fishing")) {
        pTargetData->NavStatus = FISHING;
      } else if (state == _T("sailing")) {
        pTargetData->NavStatus = UNDERWAY_SAILING;
      } else if (state == _T("hazardous material high speed")) {
        pTargetData->NavStatus = HSC;
      } else if (state == _T("hazardous material wing in ground")) {
        pTargetData->NavStatus = WIG;
      } else if (state == _T("ais-sart")) {
        pTargetData->NavStatus = RESERVED_14;
      } else {
        pTargetData->NavStatus = UNDEFINED;
      }
    } else if (update_path == _T("navigation.destination.commonName")) {
      const wxString &destination = item["value"].GetString();
      pTargetData->Destination[0] = '\0';
      strncpy(pTargetData->Destination, destination.c_str(),
              DESTINATION_LEN - 1);
    } else if (update_path == _T("navigation.specialManeuver")) {
      if (strcmp("not available", item["value"].GetString()) != 0 && pTargetData->IMO < 1) {
        const wxString &bluesign = item["value"].GetString();
        if (_T("not engaged") == bluesign) {
          pTargetData->blue_paddle = 1;
        }
        if (_T("engaged") == bluesign) {
          pTargetData->blue_paddle = 2;
        }
        pTargetData->b_blue_paddle =
            pTargetData->blue_paddle == 2 ? true : false;
      }
    } else if (update_path == _T("sensors.ais.designatedAreaCode")) {
      if (item["value"].GetInt() == 200) {
        pTargetData->b_hasInlandDac = true;
      }  // European inland
    } else if (update_path == _T("sensors.ais.functionalId")) {
      if (item["value"].GetInt() ==
              10 &&  // "Inland ship static and voyage related data"
          pTargetData->b_hasInlandDac) {
        pTargetData->b_isEuroInland = true;
      }
    } else if (update_path == _T("")) {
      if (item["value"].HasMember("name")) {
        const wxString &name = item["value"]["name"].GetString();
        strncpy(pTargetData->ShipName, name.c_str(), SHIP_NAME_LEN - 1);
        pTargetData->b_nameValid = true;
        pTargetData->MID = 123;  // Indicates a name from SignalK
      } else if (item["value"].HasMember("registrations")) {
        const wxString &imo = item["value"]["registrations"]["imo"].GetString();
        pTargetData->IMO = wxAtoi(imo.Right(7));
      } else if (item["value"].HasMember("communication")) {
        const wxString &callsign =
            item["value"]["communication"]["callsignVhf"].GetString();
        strncpy(pTargetData->CallSign, callsign.c_str(), 7);
      }
      if (item["value"].HasMember("mmsi")) {
        long mmsi;
        wxString tmp = item["value"]["mmsi"].GetString();
        if (tmp.ToLong(&mmsi)) {
          pTargetData->MMSI = mmsi;

          if (97 == mmsi / 10000000) {
            pTargetData->Class = AIS_SART;
          }
          if (111 == mmsi / 1000000) {
            pTargetData->b_SarAircraftPosnReport = true;
          }

          AISshipNameCache(pTargetData.get(), AISTargetNamesC, AISTargetNamesNC,
                           mmsi);
        }
      }
    } else {
      wxLogMessage(wxString::Format(
          _T("** AisDecoder::updateItem: unhandled path %s"), update_path));
#if 1
      rapidjson::StringBuffer buffer;
      rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
      item.Accept(writer);
      wxString msg(_T("update: "));
      msg.append(buffer.GetString());
      wxLogMessage(msg);
#endif
    }
  }
}

//----------------------------------------------------------------------------------
//      Decode a single AIVDO sentence to a Generic Position Report
//----------------------------------------------------------------------------------
AisError AisDecoder::DecodeSingleVDO(const wxString &str,
                                       GenericPosDatEx *pos,
                                       wxString *accumulator) {
  //  Make some simple tests for validity
  if (str.Len() > 100) return AIS_NMEAVDX_TOO_LONG;

  if (!NMEACheckSumOK(str)) return AIS_NMEAVDX_CHECKSUM_BAD;

  if (!pos) return AIS_GENERIC_ERROR;

  if (!accumulator) return AIS_GENERIC_ERROR;

  //  We only process AIVDO messages
  if (!str.Mid(1, 5).IsSameAs(_T("AIVDO"))) return AIS_GENERIC_ERROR;

  //  Use a tokenizer to pull out the first 4 fields
  wxStringTokenizer tkz(str, _T(","));

  wxString token;
  token = tkz.GetNextToken();  // !xxVDx

  token = tkz.GetNextToken();
  int nsentences = atoi(token.mb_str());

  token = tkz.GetNextToken();
  int isentence = atoi(token.mb_str());

  token = tkz.GetNextToken();  // skip 2 fields
  token = tkz.GetNextToken();

  wxString string_to_parse;
  string_to_parse.Clear();

  //  Fill the output structure with all NANs
  pos->kLat = NAN;
  pos->kLon = NAN;
  pos->kCog = NAN;
  pos->kSog = NAN;
  pos->kHdt = NAN;
  pos->kVar = NAN;
  pos->kHdm = NAN;

  //  Simple case first
  //  First and only part of a one-part sentence
  if ((1 == nsentences) && (1 == isentence)) {
    string_to_parse = tkz.GetNextToken();  // the encapsulated data
  }

  else if (nsentences > 1) {
    if (1 == isentence) {
      *accumulator = tkz.GetNextToken();  // the encapsulated data
    }

    else {
      accumulator->Append(tkz.GetNextToken());
    }

    if (isentence == nsentences) {  // ready to parse
      string_to_parse = *accumulator;
    }
  }

  if (string_to_parse.IsEmpty() &&
      (nsentences > 1)) {             // not ready, so return with NAN
    return AIS_INCOMPLETE_MULTIPART;  // and non-zero return
  }

  //  Create the bit accessible string
  AisBitstring strbit(string_to_parse.mb_str());

//   auto TargetData = std::make_unique<AisTargetData>(
//           *AisTargetDataMaker::GetInstance().GetTargetData());

  auto TargetData = AisTargetDataMaker::GetInstance().GetTargetData();

  bool bdecode_result = Parse_VDXBitstring(&strbit, TargetData);

  if (bdecode_result) {
    switch (TargetData->MID) {
      case 1:
      case 2:
      case 3:
      case 18: {
        if (!TargetData->b_positionDoubtful) {
          pos->kLat = TargetData->Lat;
          pos->kLon = TargetData->Lon;
        } else {
          pos->kLat = NAN;
          pos->kLon = NAN;
        }

        if (TargetData->COG == 360.0)
          pos->kCog = NAN;
        else
          pos->kCog = TargetData->COG;

        if (TargetData->SOG > 102.2)
          pos->kSog = NAN;
        else
          pos->kSog = TargetData->SOG;

        if ((int)TargetData->HDG == 511)
          pos->kHdt = NAN;
        else
          pos->kHdt = TargetData->HDG;

        //  VDO messages do not contain variation or magnetic heading
        pos->kVar = NAN;
        pos->kHdm = NAN;
        break;
      }
      default:
        return AIS_GENERIC_ERROR;  // unrecognised sentence
    }

    return AIS_NoError;
  } else
    return AIS_GENERIC_ERROR;
}

//----------------------------------------------------------------------------------------
//      Decode NMEA VDM/VDO/FRPOS/DSCDSE/TTM/TLL/OSD/RSD/TLB/WPL sentence to AIS
//      Target(s)
//----------------------------------------------------------------------------------------

AisError AisDecoder::DecodeN0183(const wxString &str) {
  AisError ret = AIS_GENERIC_ERROR;
  wxString string_to_parse;

  double gpsg_lat, gpsg_lon, gpsg_mins, gpsg_degs;
  double gpsg_cog, gpsg_sog, gpsg_utc_time;
  int gpsg_utc_hour = 0;
  int gpsg_utc_min = 0;
  int gpsg_utc_sec = 0;
  char gpsg_name_str[21];
  wxString gpsg_date;

  bool bdecode_result = false;

  int gpsg_mmsi = 0;
  int arpa_mmsi = 0;
  int aprs_mmsi = 0;
  int mmsi = 0;

  long arpa_tgt_num = 0;
  double arpa_sog = 0.;
  double arpa_cog = 0.;
  double arpa_lat = 0.;
  double arpa_lon = 0.;
  double arpa_dist = 0.;
  double arpa_brg = 0.;
  wxString arpa_brgunit;
  wxString arpa_status;
  wxString arpa_distunit;
  wxString arpa_cogunit;
  wxString arpa_reftarget;
  double arpa_mins, arpa_degs;
  double arpa_utc_time;
  int arpa_utc_hour = 0;
  int arpa_utc_min = 0;
  int arpa_utc_sec = 0;
  char arpa_name_str[21];
  bool arpa_lost = true;
  bool arpa_nottracked = false;

  double aprs_lat = 0.;
  double aprs_lon = 0.;
  char aprs_name_str[21];
  double aprs_mins, aprs_degs;

  std::shared_ptr<AisTargetData> pTargetData = 0;
  std::shared_ptr<AisTargetData> pStaleTarget = NULL;
  bool bnewtarget = false;
  int last_report_ticks;

  //  Make some simple tests for validity

  if (str.Len() > 100) return AIS_NMEAVDX_TOO_LONG;

  if (!NMEACheckSumOK(str)) {
    return AIS_NMEAVDX_CHECKSUM_BAD;
  }
  if (str.Mid(1, 2).IsSameAs(_T("CD"))) {
    ProcessDSx(str);
    return AIS_NoError;
  } else if (str.Mid(3, 3).IsSameAs(_T("TTM"))) {
    //$--TTM,xx,x.x,x.x,a,x.x,x.x,a,x.x,x.x,a,c--c,a,a*hh <CR><LF>
    // or
    //$--TTM,xx,x.x,x.x,a,x.x,x.x,a,x.x,x.x,a,c--c,a,a,hhmmss.ss,a*hh<CR><LF>
    wxString string(str);
    wxStringTokenizer tkz(string, _T(",*"));

    wxString token;
    token = tkz.GetNextToken();  // Sentence (xxTTM)
    token = tkz.GetNextToken();  // 1) Target Number
    token.ToLong(&arpa_tgt_num);
    token = tkz.GetNextToken();  // 2)Target Distance
    token.ToDouble(&arpa_dist);
    token = tkz.GetNextToken();  // 3) Bearing from own ship
    token.ToDouble(&arpa_brg);
    arpa_brgunit = tkz.GetNextToken();  // 4) Bearing Units
    if (arpa_brgunit == _T("R")) {
      if (std::isnan(arpa_ref_hdg)) {
        if (!std::isnan(gHdt))
          arpa_brg += gHdt;
        else
          arpa_brg += gCog;
      } else
        arpa_brg += arpa_ref_hdg;
      if (arpa_brg >= 360.) arpa_brg -= 360.;
    }
    token = tkz.GetNextToken();  // 5) Target speed
    token.ToDouble(&arpa_sog);
    token = tkz.GetNextToken();  // 6) Target Course
    token.ToDouble(&arpa_cog);
    arpa_cogunit = tkz.GetNextToken();  // 7) Course Units
    if (arpa_cogunit == _T("R")) {
      if (std::isnan(arpa_ref_hdg)) {
        if (!std::isnan(gHdt))
          arpa_cog += gHdt;
        else
          arpa_cog += gCog;
      } else
        arpa_cog += arpa_ref_hdg;
      if (arpa_cog >= 360.) arpa_cog -= 360.;
    }
    token = tkz.GetNextToken();  // 8) Distance of closest-point-of-approach
    token = tkz.GetNextToken();  // 9) Time until closest-point-of-approach "-"
                                 // means increasing
    arpa_distunit = tkz.GetNextToken();  // 10)Speed/ dist unit
    token = tkz.GetNextToken();          // 11) Target name
    if (token == wxEmptyString)
      token = wxString::Format(_T("ARPA %ld"), arpa_tgt_num);
    int len = wxMin(token.Length(), 20);
    strncpy(arpa_name_str, token.mb_str(), len);
    arpa_name_str[len] = 0;
    arpa_status = tkz.GetNextToken();  // 12) Target Status
    if (arpa_status != _T( "L" )) {
      arpa_lost = false;
    } else if (arpa_status != wxEmptyString)
      arpa_nottracked = true;
    arpa_reftarget = tkz.GetNextToken();  // 13) Reference Target
    if (tkz.HasMoreTokens()) {
      token = tkz.GetNextToken();
      token.ToDouble(&arpa_utc_time);
      arpa_utc_hour = (int)(arpa_utc_time / 10000.0);
      arpa_utc_min = (int)(arpa_utc_time / 100.0) - arpa_utc_hour * 100;
      arpa_utc_sec =
          (int)arpa_utc_time - arpa_utc_hour * 10000 - arpa_utc_min * 100;
    } else {
      arpa_utc_hour = wxDateTime::Now().ToUTC().GetHour();
      arpa_utc_min = wxDateTime::Now().ToUTC().GetMinute();
      arpa_utc_sec = wxDateTime::Now().ToUTC().GetSecond();
    }

    if (arpa_distunit == _T("K")) {
      arpa_dist = fromUsrDistance(arpa_dist, DISTANCE_KM, g_iDistanceFormat);
      arpa_sog = fromUsrSpeed(arpa_sog, SPEED_KMH, g_iSpeedFormat);
    } else if (arpa_distunit == _T("S")) {
      arpa_dist = fromUsrDistance(arpa_dist, DISTANCE_MI, g_iDistanceFormat);
      arpa_sog = fromUsrSpeed(arpa_sog, SPEED_MPH, g_iSpeedFormat);
    }

    mmsi = arpa_mmsi = 199200000 + arpa_tgt_num;
                       // 199 is INMARSAT-A MID, should not occur ever in AIS
                       // stream + we make sure we are out of the hashes for
                       // GPSGate buddies by being above 1992*
  } else if (str.Mid(3, 3).IsSameAs(_T("TLL"))) {
    //$--TLL,xx,llll.lll,a,yyyyy.yyy,a,c--c,hhmmss.ss,a,a*hh<CR><LF>
    //"$RATLL,01,5603.370,N,01859.976,E,ALPHA,015200.36,T,*75\r\n"
    wxString aprs_tll_str;
    wxString string(str);
    wxStringTokenizer tkz(string, _T(",*"));

    wxString token;
    aprs_tll_str = tkz.GetNextToken();  // Sentence (xxTLL)
    token = tkz.GetNextToken();         // 1) Target number 00 - 99
    token.ToLong(&arpa_tgt_num);
    token = tkz.GetNextToken();  // 2) Latitude, N/S
    token.ToDouble(&arpa_lat);
    arpa_degs = (int)(arpa_lat / 100.0);
    arpa_mins = arpa_lat - arpa_degs * 100.0;
    arpa_lat = arpa_degs + arpa_mins / 60.0;
    token = tkz.GetNextToken();  // hemisphere N or S
    if (token.Mid(0, 1).Contains(_T("S")) == true ||
        token.Mid(0, 1).Contains(_T("s")) == true)
      arpa_lat = 0. - arpa_lat;
    token = tkz.GetNextToken();  // 3) Longitude, E/W
    token.ToDouble(&arpa_lon);
    arpa_degs = (int)(arpa_lon / 100.0);
    arpa_mins = arpa_lon - arpa_degs * 100.0;
    arpa_lon = arpa_degs + arpa_mins / 60.0;
    token = tkz.GetNextToken();  // hemisphere E or W
    if (token.Mid(0, 1).Contains(_T("W")) == true ||
        token.Mid(0, 1).Contains(_T("w")) == true)
      arpa_lon = 0. - arpa_lon;
    token = tkz.GetNextToken();  // 4) Target name
    if (token == wxEmptyString)
      token = wxString::Format(_T("ARPA %d"), arpa_tgt_num);
    int len = wxMin(token.Length(), 20);
    strncpy(arpa_name_str, token.mb_str(), len);
    arpa_name_str[len] = 0;
    token = tkz.GetNextToken();  // 5) UTC of data
    token.ToDouble(&arpa_utc_time);
    arpa_utc_hour = (int)(arpa_utc_time / 10000.0);
    arpa_utc_min = (int)(arpa_utc_time / 100.0) - arpa_utc_hour * 100;
    arpa_utc_sec =
        (int)arpa_utc_time - arpa_utc_hour * 10000 - arpa_utc_min * 100;
    arpa_status = tkz.GetNextToken();  // 6) Target status: L = lost,tracked
                                       // target has beenlost Q = query,target in
                                       // the process of acquisition T = tracking
    if (arpa_status != _T("L"))
      arpa_lost = false;
    else if (arpa_status != wxEmptyString)
      arpa_nottracked = true;
    arpa_reftarget = tkz.GetNextToken();  // 7) Reference target=R,null
                                          // otherwise
    mmsi = arpa_mmsi =
        199200000 +
        arpa_tgt_num;  // 199 is INMARSAT-A MID, should not occur ever in AIS
                       // stream + we make sure we are out of the hashes for
                       // GPSGate buddies by being above 1992*
  } else if (str.Mid(3, 3).IsSameAs(_T("OSD"))) {
    //$--OSD,x.x,A,x.x,a,x.x,a,x.x,x.x,a*hh <CR><LF>
    wxString string(str);
    wxStringTokenizer tkz(string, _T(",*"));

    wxString token;
    token = tkz.GetNextToken();  // Sentence (xxOSD)
    token = tkz.GetNextToken();  // 1) Heading (true)
    token.ToDouble(&arpa_ref_hdg);
    // 2) speed
    // 3) Vessel Course, degrees True
    // 4) Course Reference, B/M/W/R/P (see note)
    // 5) Vessel Speed
    // 6) Speed Reference, B/M/W/R/P (see note)
    // 7) Vessel Set, degrees True - Manually entered
    // 8) Vessel drift (speed) - Manually entered
    // 9) Speed Units K = km/h; N = Knots; S = statute miles/h

  } else if (str.Mid(3, 3).IsSameAs(_T("WPL"))) {
    //** $--WPL,llll.ll,a,yyyyy.yy,a,c--c*hh<CR><LF>
    wxString string(str);
    wxStringTokenizer tkz(string, _T(",*"));

    wxString token;
    token = tkz.GetNextToken();  // Sentence (xxWPL)
    token = tkz.GetNextToken();  // 1) Latitude, N/S
    token.ToDouble(&aprs_lat);
    aprs_degs = (int)(aprs_lat / 100.0);
    aprs_mins = aprs_lat - aprs_degs * 100.0;
    aprs_lat = aprs_degs + aprs_mins / 60.0;
    token = tkz.GetNextToken();  // 2) hemisphere N or S
    if (token.Mid(0, 1).Contains(_T("S")) == true ||
        token.Mid(0, 1).Contains(_T("s")) == true)
      aprs_lat = 0. - aprs_lat;
    token = tkz.GetNextToken();  // 3) Longitude, E/W
    token.ToDouble(&aprs_lon);
    aprs_degs = (int)(aprs_lon / 100.0);
    aprs_mins = aprs_lon - aprs_degs * 100.0;
    aprs_lon = aprs_degs + aprs_mins / 60.0;
    token = tkz.GetNextToken();  // 4) hemisphere E or W
    if (token.Mid(0, 1).Contains(_T("W")) == true ||
        token.Mid(0, 1).Contains(_T("w")) == true)
      aprs_lon = 0. - aprs_lon;
    token = tkz.GetNextToken();  // 5) Target name
    int len = wxMin(token.Length(), 20);
    strncpy(aprs_name_str, token.mb_str(), len + 1);
    if (0 == g_WplAction) {  // APRS position reports
      int i, hash = 0;
      aprs_name_str[len] = 0;
      for (i = 0; i < len; i++) {
        hash = hash * 10;
        hash += (int)(aprs_name_str[i]);
        while (hash >= 100000) hash = hash / 100000;
      }
      mmsi = aprs_mmsi = 199300000 + hash;
                 // 199 is INMARSAT-A MID, should not occur ever in AIS stream +
                 // we make sure we are out of the hashes for GPSGate buddies
                 // and ARPA by being above 1993*
    } else if (1 == g_WplAction) {  // Create mark

      //FIXME (dave) This is a GUI thing...

//       RoutePoint *pWP = new RoutePoint(aprs_lat, aprs_lon, g_default_wp_icon,
//                                        aprs_name_str, wxEmptyString);
//       pWP->m_bIsolatedMark = true;  // This is an isolated mark
//       pSelect->AddSelectableRoutePoint(aprs_lat, aprs_lon, pWP);
//       new_ais_wp.notify(pWP);
    }
  } else if (str.Mid(1, 5).IsSameAs(_T("FRPOS"))) {
    // parse a GpsGate Position message            $FRPOS,.....

    //  Use a tokenizer to pull out the first 9 fields
    wxString string(str);
    wxStringTokenizer tkz(string, _T(",*"));

    wxString token;
    token = tkz.GetNextToken();  // !$FRPOS

    token = tkz.GetNextToken();  //    latitude DDMM.MMMM
    token.ToDouble(&gpsg_lat);
    gpsg_degs = (int)(gpsg_lat / 100.0);
    gpsg_mins = gpsg_lat - gpsg_degs * 100.0;
    gpsg_lat = gpsg_degs + gpsg_mins / 60.0;

    token = tkz.GetNextToken();  //  hemisphere N or S
    if (token.Mid(0, 1).Contains(_T("S")) == true ||
        token.Mid(0, 1).Contains(_T("s")) == true)
      gpsg_lat = 0. - gpsg_lat;

    token = tkz.GetNextToken();  // longitude DDDMM.MMMM
    token.ToDouble(&gpsg_lon);
    gpsg_degs = (int)(gpsg_lon / 100.0);
    gpsg_mins = gpsg_lon - gpsg_degs * 100.0;
    gpsg_lon = gpsg_degs + gpsg_mins / 60.0;

    token = tkz.GetNextToken();  // hemisphere E or W
    if (token.Mid(0, 1).Contains(_T("W")) == true ||
        token.Mid(0, 1).Contains(_T("w")) == true)
      gpsg_lon = 0. - gpsg_lon;

    token = tkz.GetNextToken();  //    altitude AA.a
    //    token.toDouble(&gpsg_alt);

    token = tkz.GetNextToken();  //  speed over ground SSS.SS knots
    token.ToDouble(&gpsg_sog);

    token = tkz.GetNextToken();  //  heading over ground HHH.hh degrees
    token.ToDouble(&gpsg_cog);

    token = tkz.GetNextToken();  // date DDMMYY
    gpsg_date = token;

    token = tkz.GetNextToken();  // time UTC hhmmss.dd
    token.ToDouble(&gpsg_utc_time);
    gpsg_utc_hour = (int)(gpsg_utc_time / 10000.0);
    gpsg_utc_min = (int)(gpsg_utc_time / 100.0) - gpsg_utc_hour * 100;
    gpsg_utc_sec =
        (int)gpsg_utc_time - gpsg_utc_hour * 10000 - gpsg_utc_min * 100;

    // now comes the name, followed by in * and NMEA checksum

    token = tkz.GetNextToken();
    int i, len, hash = 0;
    len = wxMin(wxStrlen(token), 20);
    strncpy(gpsg_name_str, token.mb_str(), len);
    gpsg_name_str[len] = 0;
    for (i = 0; i < len; i++) {
      hash = hash * 10;
      hash += (int)(token[i]);
      while (hash >= 100000) hash = hash / 100000;
    }
    // 199 is INMARSAT-A MID, should not occur ever in AIS stream
    gpsg_mmsi = 199000000 + hash;
    mmsi = gpsg_mmsi;
  } else if (!str.Mid(3, 2).IsSameAs(_T("VD"))) {
    return AIS_NMEAVDX_BAD;
  }

  //  OK, looks like the sentence is OK

  //  Use a tokenizer to pull out the first 4 fields
  wxString string(str);
  wxStringTokenizer tkz(string, _T(","));

  wxString token;
  token = tkz.GetNextToken();  // !xxVDx

  token = tkz.GetNextToken();
  nsentences = atoi(token.mb_str());

  token = tkz.GetNextToken();
  isentence = atoi(token.mb_str());

  token = tkz.GetNextToken();
  long lsequence_id = 0;
  token.ToLong(&lsequence_id);

  token = tkz.GetNextToken();
  long lchannel;
  token.ToLong(&lchannel);
  //  Now, some decisions

  string_to_parse.Clear();

  //  Simple case first
  //  First and only part of a one-part sentence
  if ((1 == nsentences) && (1 == isentence)) {
    string_to_parse = tkz.GetNextToken();  // the encapsulated data
  }

  else if (nsentences > 1) {
    if (1 == isentence) {
      sentence_accumulator = tkz.GetNextToken();  // the encapsulated data
    }

    else {
      sentence_accumulator += tkz.GetNextToken();
    }

    if (isentence == nsentences) {
      string_to_parse = sentence_accumulator;
    }
  }

  if (mmsi || (!string_to_parse.IsEmpty() &&
               (string_to_parse.Len() < AIS_MAX_MESSAGE_LEN))) {
    //  Create the bit accessible string
    wxCharBuffer abuf = string_to_parse.ToUTF8();
    if (!abuf.data())  // badly formed sentence?
      return AIS_GENERIC_ERROR;

    AisBitstring strbit(abuf.data());

    //  Extract the MMSI
    if (!mmsi) mmsi = strbit.GetInt(9, 30);
    long mmsi_long = mmsi;
    //  Search the current AISTargetList for an MMSI match
    auto it = AISTargetList.find(mmsi);
    if (it == AISTargetList.end())  // not found
    {
      pTargetData = AisTargetDataMaker::GetInstance().GetTargetData();
      bnewtarget = true;
      m_n_targets++;
    } else {
      pTargetData = it->second;    // find current entry

      //Ais8_001_31 (class AIS_METEO) test for a possible new mmsi ID
      int messID = strbit.GetInt(1, 6);
      int dac = strbit.GetInt(41, 10);
      int fi = strbit.GetInt(51, 6);
      if (messID == 8 && dac == 001 && fi == 31) { //Ais8_001_31
        int met_lon = strbit.GetInt(57, 25);
        int met_lat = strbit.GetInt(82, 24);
        //Compare message position with already present in TargetData
        int new_mmsi = AisMeteoNewMmsi(mmsi, met_lat, met_lon, pTargetData->Lat,
                                       pTargetData->Lon);
        if (mmsi != new_mmsi) {
          int origin_mmsi = mmsi;
          mmsi = new_mmsi;
          mmsi_long = new_mmsi;
          pTargetData.reset();
          it = AISTargetList.find(mmsi);
          if (it == AISTargetList.end()) {  // not found
            pTargetData = AisTargetDataMaker::GetInstance().GetTargetData();
            bnewtarget = true;
            m_n_targets++;
            pTargetData->MMSI = mmsi;
            pTargetData->met_data.original_mmsi = origin_mmsi;
          } else {
            pTargetData = it->second;
          }
        }
      }

      if(!bnewtarget) pStaleTarget = pTargetData;  // save a pointer to stale data
    }
    for (unsigned int i = 0; i < g_MMSI_Props_Array.GetCount(); i++) {
      MmsiProperties *props = g_MMSI_Props_Array[i];
      if (mmsi == props->MMSI) {
        // Check if this target has a dedicated tracktype
        if (TRACKTYPE_NEVER == props->TrackType) {
          pTargetData->b_show_track = false;
        }
        else if (TRACKTYPE_ALWAYS == props->TrackType) {
          pTargetData->b_show_track = true;
        }

        // Check to see if this MMSI has been configured to be ignored
        // completely...
        if (props->m_bignore) return AIS_NoError;
        // Check to see if this MMSI wants VDM translated to VDO or whether we
        // want to persist it's track...
        else if (props->m_bVDM) {
          // Only single line VDM messages to be translated
          if (str.Mid(3, 9).IsSameAs(wxT("VDM,1,1,,"))) {
            int message_ID = strbit.GetInt(1, 6);  // Parse on message ID
            // Only translate the dynamic positionreport messages (1, 2, 3 or
            // 18)
            if ((message_ID <= 3) || (message_ID == 18)) {
              // set OwnShip to prevent target from being drawn
              pTargetData->b_OwnShip = true;
              // Rename nmea sentence to AIVDO and calc a new checksum
              wxString aivdostr = str;
              aivdostr.replace(1, 5, "AIVDO");
              unsigned char calculated_checksum = 0;
              wxString::iterator i;
              for (i = aivdostr.begin() + 1; i != aivdostr.end() && *i != '*';
                   ++i)
                calculated_checksum ^= static_cast<unsigned char>(*i);
              // if i is not at least 3 positons befoere end, there is no
              // checksum added so also no need to add one now.
              if (i <= aivdostr.end() - 3)
                aivdostr.replace(
                    i + 1, i + 3,
                    wxString::Format(_("%02X"), calculated_checksum));

              gps_watchdog_timeout_ticks =
                  60;  // increase watchdog time up to 1 minute
              // add the changed sentence into nmea message system
              std::string full_sentence = aivdostr.ToStdString();
              std::string identifier("AIVDO");
              // We notify based on full message, including the Talker ID
              // notify message listener and also "ALL" N0183 messages, to support plugin
              // API using original talker id
              auto address = std::make_shared<NavAddr0183>("virtual");
              auto msg =
              std::make_shared<const Nmea0183Msg>(identifier, full_sentence, address);
              auto msg_all = std::make_shared<const Nmea0183Msg>(*msg, "ALL");

              auto &msgbus = NavMsgBus::GetInstance();

              msgbus.Notify(std::move(msg));
              msgbus.Notify(std::move(msg_all));
            }
          }
          return AIS_NoError;
        } else
          break;
      }
    }

    //  Grab the stale targets's last report time
    wxDateTime now = wxDateTime::Now();
    now.MakeGMT();

    if (pStaleTarget)
      last_report_ticks = pStaleTarget->PositionReportTicks;
    else
      last_report_ticks = now.GetTicks();

    // Delete the stale AIS Target selectable point
    if (pStaleTarget)
      pSelectAIS->DeleteSelectablePoint((void *)mmsi_long, SELTYPE_AISTARGET);

     if (pTargetData) {
      if (gpsg_mmsi) {
        pTargetData->PositionReportTicks = now.GetTicks();
        pTargetData->StaticReportTicks = now.GetTicks();
        pTargetData->m_utc_hour = gpsg_utc_hour;
        pTargetData->m_utc_min = gpsg_utc_min;
        pTargetData->m_utc_sec = gpsg_utc_sec;
        pTargetData->m_date_string = gpsg_date;
        pTargetData->MMSI = gpsg_mmsi;
        pTargetData->NavStatus = 0;  // underway
        pTargetData->Lat = gpsg_lat;
        pTargetData->Lon = gpsg_lon;
        pTargetData->b_positionOnceValid = true;
        pTargetData->COG = gpsg_cog;
        pTargetData->SOG = gpsg_sog;
        pTargetData->ShipType = 52;  // buddy
        pTargetData->Class = AIS_GPSG_BUDDY;
        memcpy(pTargetData->ShipName, gpsg_name_str, sizeof(gpsg_name_str));
        pTargetData->b_nameValid = true;
        pTargetData->b_active = true;
        pTargetData->b_lost = false;

        bdecode_result = true;
      } else if (arpa_mmsi) {
        pTargetData->m_utc_hour = arpa_utc_hour;
        pTargetData->m_utc_min = arpa_utc_min;
        pTargetData->m_utc_sec = arpa_utc_sec;
        pTargetData->MMSI = arpa_mmsi;
        pTargetData->NavStatus = 15;  // undefined
        if (str.Mid(3, 3).IsSameAs(_T("TLL"))) {
          if (!bnewtarget) {
            int age_of_last =
                (now.GetTicks() - pTargetData->PositionReportTicks);
            if (age_of_last > 0) {
              ll_gc_ll_reverse(pTargetData->Lat, pTargetData->Lon, arpa_lat,
                               arpa_lon, &pTargetData->COG, &pTargetData->SOG);
              pTargetData->SOG = pTargetData->SOG * 3600 / age_of_last;
            }
          }
          pTargetData->Lat = arpa_lat;
          pTargetData->Lon = arpa_lon;
        } else if (str.Mid(3, 3).IsSameAs(_T("TTM"))) {
          if (arpa_dist != 0.)  // Not a new or turned off target
            ll_gc_ll(gLat, gLon, arpa_brg, arpa_dist, &pTargetData->Lat,
                     &pTargetData->Lon);
          else
            arpa_lost = true;
          pTargetData->COG = arpa_cog;
          pTargetData->SOG = arpa_sog;
        }
        pTargetData->PositionReportTicks = now.GetTicks();
        pTargetData->StaticReportTicks = now.GetTicks();
        pTargetData->b_positionOnceValid = true;
        pTargetData->ShipType = 55;  // arpa
        pTargetData->Class = AIS_ARPA;

        memcpy(pTargetData->ShipName, arpa_name_str, sizeof(arpa_name_str));
        if (arpa_status != _T("Q"))
          pTargetData->b_nameValid = true;
        else
          pTargetData->b_nameValid = false;
        pTargetData->b_active = !arpa_lost;
        pTargetData->b_lost = arpa_nottracked;

        bdecode_result = true;
      } else if (aprs_mmsi) {
        pTargetData->m_utc_hour = now.GetHour();
        pTargetData->m_utc_min = now.GetMinute();
        pTargetData->m_utc_sec = now.GetSecond();
        pTargetData->MMSI = aprs_mmsi;
        pTargetData->NavStatus = 15;  // undefined
        if (!bnewtarget) {
          int age_of_last = (now.GetTicks() - pTargetData->PositionReportTicks);
          if (age_of_last > 0) {
            ll_gc_ll_reverse(pTargetData->Lat, pTargetData->Lon, aprs_lat,
                             aprs_lon, &pTargetData->COG, &pTargetData->SOG);
            pTargetData->SOG = pTargetData->SOG * 3600 / age_of_last;
          }
        }
        pTargetData->PositionReportTicks = now.GetTicks();
        pTargetData->StaticReportTicks = now.GetTicks();
        pTargetData->Lat = aprs_lat;
        pTargetData->Lon = aprs_lon;
        pTargetData->b_positionOnceValid = true;
        pTargetData->ShipType = 56;  // aprs
        pTargetData->Class = AIS_APRS;
        memcpy(pTargetData->ShipName, aprs_name_str, sizeof(aprs_name_str));
        pTargetData->b_nameValid = true;
        pTargetData->b_active = true;
        pTargetData->b_lost = false;

        bdecode_result = true;
      } else {
        // The normal Plain-Old AIS target code path....
        bdecode_result =
            Parse_VDXBitstring(&strbit, pTargetData);  // Parse the new data
      }

      // Catch mmsi properties like track, persistent track, follower.
      getMmsiProperties(pTargetData);

      //     Update the most recent report period
      pTargetData->RecentPeriod =
          pTargetData->PositionReportTicks - last_report_ticks;
    }
    ret = AIS_NoError;
  } else {
    ret = AIS_Partial;  // accumulating parts of a multi-sentence message
    pTargetData = 0;
  }

  if (pTargetData) {
    //  pTargetData is valid, either new or existing. Commit to GUI
    CommitAISTarget(pTargetData, str, bdecode_result, bnewtarget);
  }

  n_msgs++;
#ifdef AIS_DEBUG
  if ((n_msgs % 10000) == 0)
    printf("n_msgs %10d m_n_targets: %6d  n_msg1: %10d  n_msg5+24: %10d \n",
           n_msgs, m_n_targets, n_msg1, n_msg5 + n_msg24);
#endif

  return ret;
}

void AisDecoder::CommitAISTarget(std::shared_ptr<AisTargetData> pTargetData,
                                  const wxString &str,
                                  bool message_valid,
                                  bool new_target){

    m_pLatestTargetData = pTargetData;

    if (!str.IsEmpty()) {       // NMEA0183 message
      if (str.Mid(3, 3).IsSameAs(_T("VDO")))
        pTargetData->b_OwnShip = true;
      else
        pTargetData->b_OwnShip = false;
    }

    if (!pTargetData->b_OwnShip) {
      // set  mmsi-props to default values
      if (0 == m_persistent_tracks.count(pTargetData->MMSI)) {
        // Normal target
        pTargetData->b_PersistTrack = false;
        // Or first decode for this target
        for (unsigned int i = 0; i < g_MMSI_Props_Array.GetCount(); i++) {
          if (pTargetData->MMSI == g_MMSI_Props_Array[i]->MMSI) {
            MmsiProperties *props = g_MMSI_Props_Array[i];
            pTargetData->b_mPropPersistTrack = props->m_bPersistentTrack;
            break;
          }
        }
      } else {
        // The track persistency enabled in the query window or mmsi-props
      }
      pTargetData->b_NoTrack = false;
    }

    //  If the message was decoded correctly
    //  Update the AIS Target information
    if (message_valid) {
      // Print to name cache only if not mmsi = 0
      if (pTargetData->MMSI) {
        AISshipNameCache(pTargetData.get(), AISTargetNamesC, AISTargetNamesNC, pTargetData->MMSI);
      }
      AISTargetList[pTargetData->MMSI] =
          pTargetData;  // update the hash table entry

      if (!pTargetData->area_notices.empty()) {
        auto it = AIS_AreaNotice_Sources.find(pTargetData->MMSI);
        if (it == AIS_AreaNotice_Sources.end())
          AIS_AreaNotice_Sources[pTargetData->MMSI] = pTargetData;
      }

      //  If this is not an ownship message, update the AIS Target in the
      //  Selectable list, and update the CPA info
      if (!pTargetData->b_OwnShip) {
        if (pTargetData->b_positionOnceValid) {
          long mmsi_long = pTargetData->MMSI;
          SelectItem *pSel = pSelectAIS->AddSelectablePoint(
              pTargetData->Lat, pTargetData->Lon, (void *)mmsi_long,
              SELTYPE_AISTARGET);
          pSel->SetUserData(pTargetData->MMSI);
        }

        //    Calculate CPA info for this target immediately
        UpdateOneCPA(pTargetData.get());

        //    Update this target's track
        if (pTargetData->b_show_track) UpdateOneTrack(pTargetData.get());
      }
      // TODO add ais message call
      plugin_msg.Notify(std::make_shared<AisTargetData>(*pTargetData), "");
    } else {
      //             printf("Unrecognised AIS message ID: %d\n",
      //             pTargetData->MID);
      if (new_target) {
        //delete pTargetData;  // this target is not going to be used
        m_n_targets--;
      } else {
        //  If this is not an ownship message, update the AIS Target in the
        //  Selectable list even if the message type was not recognized
        if (!pTargetData->b_OwnShip) {
          if (pTargetData->b_positionOnceValid) {
            long mmsi_long = pTargetData->MMSI;
            SelectItem *pSel = pSelectAIS->AddSelectablePoint(
                pTargetData->Lat, pTargetData->Lon, (void *)mmsi_long,
                SELTYPE_AISTARGET);
            pSel->SetUserData(pTargetData->MMSI);
          }
        }
      }
    }

}



void AisDecoder::getAISTarget(long mmsi, std::shared_ptr<AisTargetData> &pTargetData,
                               std::shared_ptr<AisTargetData> &pStaleTarget, bool &bnewtarget,
                               int &last_report_ticks, wxDateTime &now) {
  now = wxDateTime::Now();
  auto it = AISTargetList.find(mmsi);
  if (it == AISTargetList.end())  // not found
  {
    pTargetData = AisTargetDataMaker::GetInstance().GetTargetData();
    bnewtarget = true;
    m_n_targets++;
  } else {
    pTargetData = it->second;    // find current entry
    pStaleTarget = pTargetData;  // save a pointer to stale data
  }

  //  Grab the stale targets's last report time
  now.MakeGMT();

  if (pStaleTarget)
    last_report_ticks = pStaleTarget->PositionReportTicks;
  else
    last_report_ticks = now.GetTicks();

  // Delete the stale AIS Target selectable point
  if (pStaleTarget)
    pSelectAIS->DeleteSelectablePoint((void *)mmsi, SELTYPE_AISTARGET);
}

void AisDecoder::getMmsiProperties(std::shared_ptr<AisTargetData> &pTargetData) {
  for (unsigned int i = 0; i < g_MMSI_Props_Array.GetCount(); i++) {
    if (pTargetData->MMSI == g_MMSI_Props_Array[i]->MMSI) {
      MmsiProperties * props = g_MMSI_Props_Array[i];
      pTargetData->b_isFollower = props->m_bFollower;
      pTargetData->b_mPropPersistTrack = props->m_bPersistentTrack;
      if (TRACKTYPE_NEVER == props->TrackType) {
        pTargetData->b_show_track = false;
      }
      else if (TRACKTYPE_ALWAYS == props->TrackType) {
        pTargetData->b_show_track = true;
      }
      break;
    }
  }
}

std::shared_ptr<AisTargetData> AisDecoder::ProcessDSx(const wxString &str, bool b_take_dsc) {
  double dsc_lat = 0.;
  double dsc_lon = 0.;
  double dsc_mins, dsc_degs, dsc_tmp, dsc_addr;
  double dse_tmp;
  double dse_lat = 0.;
  double dse_lon = 0.;
  long dsc_fmt, dsc_quadrant,dsc_cat, dsc_nature;

  int dsc_mmsi = 0;
  int dsc_tx_mmsi = 0;
  int dse_mmsi = 0;
  double dse_cog = 0.;
  double dse_sog = 0.;
  wxString dse_shipName = wxEmptyString;
  wxString dseSymbol;

  int mmsi = 0;

  std::shared_ptr<AisTargetData>pTargetData = NULL;

  // parse a DSC Position message            $CDDSx,.....
  //  Use a tokenizer to pull out the first 9 fields
  wxString string(str);
  wxStringTokenizer tkz(string, _T(",*"));

  wxString token;
  token = tkz.GetNextToken();  // !$CDDS

  if (str.Mid(3, 3).IsSameAs(_T("DSC"))) {
    m_dsc_last_string = str;

    token = tkz.GetNextToken();  // format specifier
    token.ToLong(&dsc_fmt);      // (02-area,12-distress,16-allships,20-individual,...)

    token = tkz.GetNextToken();  // address i.e. mmsi*10 for received msg, or
                                 // area spec or sender mmsi for (12) and (16)
    if (dsc_fmt == 12 || dsc_fmt == 16) {
      dsc_mmsi = wxAtoi(token.Mid(0, 9));
    }
    else {
      token.ToDouble(&dsc_addr);
      dsc_mmsi = 0 - (int)( dsc_addr / 10 );  // as per NMEA 0183 3.01
    }

    token = tkz.GetNextToken();  // category
    token.ToLong(&dsc_cat);      // 12 - Distress (relayed)

    token = tkz.GetNextToken();  // nature of distress or telecommand1
    if (!token.IsSameAs(wxEmptyString)) { // 00-12 = nature of distress
      token.ToLong(&dsc_nature);
    }
    else dsc_nature = 99;

    token = tkz.GetNextToken();  // comm type or telecommand2

    token = tkz.GetNextToken();  // position or channel/freq
    token.ToDouble(&dsc_tmp);

    token = tkz.GetNextToken();  // time or tel. no.
    token = tkz.GetNextToken();  // mmsi of ship in distress, relay
    if (dsc_fmt == 16 && dsc_cat == 12 && !token.IsSameAs(wxEmptyString) ) {
      //wxString dmmsi = token.Mid(0,9);
      dsc_tx_mmsi = dsc_mmsi;    // mmsi of relay issuer
      dsc_mmsi = wxAtoi(token.Mid(0, 9));
    }
    token = tkz.GetNextToken();  // nature of distress, relay
    if (dsc_fmt == 16 && dsc_cat == 12) {
      if (!token.IsSameAs(wxEmptyString)) { // 00-12 = nature of distress
        token.ToLong(&dsc_nature);
      }
      else dsc_nature = 99;
    }
    token = tkz.GetNextToken();  // acknowledgement
    token = tkz.GetNextToken();  // expansion indicator

    dsc_quadrant = (int)(dsc_tmp / 1000000000.0);

    if (dsc_quadrant > 3)  // Position is "Unspecified", or 9999999999
      return NULL;

    dsc_lat = (int)(dsc_tmp / 100000.0);
    dsc_lon = dsc_tmp - dsc_lat * 100000.0;
    dsc_lat = dsc_lat - dsc_quadrant * 10000;
    dsc_degs = (int)(dsc_lat / 100.0);
    dsc_mins = dsc_lat - dsc_degs * 100.0;
    dsc_lat = dsc_degs + dsc_mins / 60.0;

    dsc_degs = (int)(dsc_lon / 100.0);
    dsc_mins = dsc_lon - dsc_degs * 100.0;
    dsc_lon = dsc_degs + dsc_mins / 60.0;
    switch (dsc_quadrant) {
      case 0:
        break;  // NE
      case 1:
        dsc_lon = -dsc_lon;
        break;  // NW
      case 2:
        dsc_lat = -dsc_lat;
        break;  // SE
      case 3:
        dsc_lon = -dsc_lon;
        dsc_lat = -dsc_lat;
        break;  // SW
      default:
        break;
    }
    if (dsc_fmt != 02) mmsi = (int)dsc_mmsi;

  } else if (str.Mid(3, 3).IsSameAs(_T("DSE"))) {
    token = tkz.GetNextToken();  // total number of sentences
    token = tkz.GetNextToken();  // sentence number
    token = tkz.GetNextToken();  // query/rely flag
    token = tkz.GetNextToken();  // vessel MMSI
    dse_mmsi = wxAtoi(token.Mid(0, 9)); // ITU-R M.493-10 �5.2
    //token.ToDouble(&dse_addr);
    //0 - (int)(dse_addr / 10);  // as per NMEA 0183 3.01

# if 0
    token = tkz.GetNextToken();  // code field
    token =
        tkz.GetNextToken();  // data field - position - 2*4 digits latlon .mins
    token.ToDouble(&dse_tmp);
    dse_lat = (int)(dse_tmp / 10000.0);
    dse_lon = (int)(dse_tmp - dse_lat * 10000.0);
    dse_lat = dse_lat / 600000.0;
    dse_lon = dse_lon / 600000.0;
#endif
     // DSE Sentence may contain multiple dse expansion data items
   while (tkz.HasMoreTokens())	{
       dseSymbol = tkz.GetNextToken(); //dse expansion data symbol
       token = tkz.GetNextToken(); // dse expansion data
       if (dseSymbol.IsSameAs(_T("00"))) { // Position
           token.ToDouble(&dse_tmp);
           dse_lat = (int)(dse_tmp / 10000.0);
           dse_lon = (int)(dse_tmp - dse_lat * 10000.0);
           dse_lat = dse_lat / 600000.0;
           dse_lon = dse_lon / 600000.0;
       }
       else if (dseSymbol.IsSameAs(_T("01"))) { // Source & Datum
       }
       else if (dseSymbol.IsSameAs(_T("02"))) { // SOG
           token.ToDouble(&dse_tmp);
           dse_sog = dse_tmp / 10.0;
       }
       else if (dseSymbol.IsSameAs(_T("03"))) { // COG
           token.ToDouble(&dse_tmp);
           dse_cog = dse_tmp / 10.0;
       }
       else if (dseSymbol.IsSameAs(_T("04"))) { // Station Information
           dse_shipName = DecodeDSEExpansionCharacters(token);
       }
       else if (dseSymbol.IsSameAs(_T("05"))) { // Geographic Information
       }
       else if (dseSymbol.IsSameAs(_T("06"))) { // Persons On Board
       }
   }
    mmsi = abs((int)dse_mmsi);
  }

  //  Get the last report time for this target, if it exists
  wxDateTime now = wxDateTime::Now();
  now.MakeGMT();
  int last_report_ticks = now.GetTicks();

  //  Search the current AISTargetList for an MMSI match
  auto it = AISTargetList.find(mmsi);
  std::shared_ptr<AisTargetData> pStaleTarget = NULL;
  if (it == AISTargetList.end()) {  // not found
  } else {
    pStaleTarget = it->second;  // find current entry
    last_report_ticks = pStaleTarget->PositionReportTicks;
  }

  if (dsc_mmsi) {
    //      Create a tentative target, but do not post it pending receipt of
    //      extended data
    m_ptentative_dsctarget = AisTargetDataMaker::GetInstance().GetTargetData();

    m_ptentative_dsctarget->PositionReportTicks = now.GetTicks();
    m_ptentative_dsctarget->StaticReportTicks = now.GetTicks();

    m_ptentative_dsctarget->MMSI = mmsi;
    m_ptentative_dsctarget->NavStatus = 99; //Undefind. "-" in the AIS target list
    m_ptentative_dsctarget->Lat = dsc_lat;
    m_ptentative_dsctarget->Lon = dsc_lon;
    m_ptentative_dsctarget->b_positionOnceValid = true;
    m_ptentative_dsctarget->COG = 0;
    m_ptentative_dsctarget->SOG = 0;
    m_ptentative_dsctarget->ShipType = dsc_fmt;
    m_ptentative_dsctarget->Class = AIS_DSC;
    m_ptentative_dsctarget->b_isDSCtarget = true;
    m_ptentative_dsctarget->b_nameValid = true;
    if (dsc_fmt == 12 || (dsc_fmt == 16 && dsc_cat == 12)) {
      snprintf(m_ptentative_dsctarget->ShipName, SHIP_NAME_LEN, "DISTRESS %d",
               std::abs(mmsi));
      m_ptentative_dsctarget->m_dscNature = int(dsc_nature);
      m_ptentative_dsctarget->m_dscTXmmsi = dsc_tx_mmsi;
    } else {
      snprintf(m_ptentative_dsctarget->ShipName, SHIP_NAME_LEN, "POSITION %d",
               std::abs(mmsi));
    }

    m_ptentative_dsctarget->b_active = true;
    m_ptentative_dsctarget->b_lost = false;
    m_ptentative_dsctarget->RecentPeriod =
        m_ptentative_dsctarget->PositionReportTicks - last_report_ticks;

    //      Start a timer, looking for an expected DSE extension message
    if (!b_take_dsc) m_dsc_timer.Start(1000, wxTIMER_ONE_SHOT);
  }

  //    Got an extension message, or the timer expired and no extension is
  //    expected
  if (dse_mmsi || b_take_dsc) {
    if (m_ptentative_dsctarget) {
      //  stop the timer for sure
      m_dsc_timer.Stop();

      //  Update the extended information
      if (dse_mmsi) {
        m_ptentative_dsctarget->Lat =
            m_ptentative_dsctarget->Lat +
            ((m_ptentative_dsctarget->Lat) >= 0 ? dse_lat : -dse_lat);
        m_ptentative_dsctarget->Lon =
            m_ptentative_dsctarget->Lon +
            ((m_ptentative_dsctarget->Lon) >= 0 ? dse_lon : -dse_lon);
        if (dse_shipName.length() > 0) {
            memset(m_ptentative_dsctarget->ShipName,'\0',SHIP_NAME_LEN);
            snprintf(m_ptentative_dsctarget->ShipName, dse_shipName.length(),
                    "%s", dse_shipName.ToAscii().data());
        }
        m_ptentative_dsctarget->COG = dse_cog;
        m_ptentative_dsctarget->SOG = dse_sog;
      }

      //     Update the most recent report period
      m_ptentative_dsctarget->RecentPeriod =
          m_ptentative_dsctarget->PositionReportTicks - last_report_ticks;

      //  And post the target

      //  Search the current AISTargetList for an MMSI match
      auto it = AISTargetList.find(mmsi);
      if (it == AISTargetList.end()) {  // not found
        pTargetData = m_ptentative_dsctarget;
      } else {
        pTargetData = it->second;  // find current entry
        std::vector<AISTargetTrackPoint> ptrack =
            std::move(pTargetData->m_ptrack);
        pTargetData->CloneFrom(
            m_ptentative_dsctarget.get());  // this will make an empty track list

        pTargetData->m_ptrack =
            std::move(ptrack);  // and substitute the old track list

        //delete m_ptentative_dsctarget;
      }

      //  Reset for next time
      m_ptentative_dsctarget = NULL;

      m_pLatestTargetData = pTargetData;

      AISTargetList[pTargetData->MMSI] =
          pTargetData;  // update the hash table entry

      long mmsi_long = pTargetData->MMSI;

      // Delete any stale Target selectable point
      if (pStaleTarget)
        pSelectAIS->DeleteSelectablePoint((void *)mmsi_long, SELTYPE_AISTARGET);
      //  And add the updated target
      SelectItem *pSel =
          pSelectAIS->AddSelectablePoint(pTargetData->Lat, pTargetData->Lon,
                                         (void *)mmsi_long, SELTYPE_AISTARGET);
      pSel->SetUserData(pTargetData->MMSI);

      //    Calculate CPA info for this target immediately
      UpdateOneCPA(pTargetData.get());

      //    Update this target's track
      if (pTargetData->b_show_track) UpdateOneTrack(pTargetData.get());
    }
  }

  return pTargetData;
}

// DSE Expansion characters, decode table from ITU-R M.825
wxString AisDecoder::DecodeDSEExpansionCharacters(wxString dseData) {
    wxString result;
    char lookupTable[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ' ',
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',
        'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
        'Y', 'Z', '.', ',', '-', '/', ' ' };

    for (size_t i = 0; i < dseData.length(); i += 2) {
        result.append(1, lookupTable[strtol(dseData.Mid(i, 2).data(), NULL, 10)]);
    }
    return result;
}

//----------------------------------------------------------------------------
//      Parse a NMEA VDM/VDO Bitstring
//----------------------------------------------------------------------------
bool AisDecoder::Parse_VDXBitstring(AisBitstring *bstr,
                                     std::shared_ptr<AisTargetData> ptd) {
  bool parse_result = false;
  bool b_posn_report = false;

  wxDateTime now = wxDateTime::Now();
  now.MakeGMT();
  int message_ID = bstr->GetInt(1, 6);  // Parse on message ID
  ptd->MID = message_ID;

    // Save for Ais8_001_31 (class AIS_METEO)
  int met_mmsi = ptd->MMSI;

    // MMSI is always in the same spot in the bitstream
  ptd->MMSI = bstr->GetInt(9, 30);

  switch (message_ID) {
    case 1:  // Position Report
    case 2:
    case 3: {
      n_msg1++;

      ptd->NavStatus = bstr->GetInt(39, 4);
      ptd->SOG = 0.1 * (bstr->GetInt(51, 10));

      int lon = bstr->GetInt(62, 28);
      if (lon & 0x08000000)  // negative?
        lon |= 0xf0000000;
      double lon_tentative = lon / 600000.;

      int lat = bstr->GetInt(90, 27);
      if (lat & 0x04000000)  // negative?
        lat |= 0xf8000000;
      double lat_tentative = lat / 600000.;

      if ((lon_tentative <= 180.) &&
          (lat_tentative <=
           90.))  // Ship does not report Lat or Lon "unavailable"
      {
        ptd->Lon = lon_tentative;
        ptd->Lat = lat_tentative;
        ptd->b_positionDoubtful = false;
        ptd->b_positionOnceValid = true;  // Got the position at least once
        ptd->PositionReportTicks = now.GetTicks();
      } else
        ptd->b_positionDoubtful = true;

      //    decode balance of message....
      ptd->COG = 0.1 * (bstr->GetInt(117, 12));
      ptd->HDG = 1.0 * (bstr->GetInt(129, 9));

      ptd->ROTAIS = bstr->GetInt(43, 8);
      double rot_dir = 1.0;

      if (ptd->ROTAIS == 128)
        ptd->ROTAIS = -128;  // not available codes as -128
      else if ((ptd->ROTAIS & 0x80) == 0x80) {
        ptd->ROTAIS = ptd->ROTAIS - 256;  // convert to twos complement
        rot_dir = -1.0;
      }

      ptd->ROTIND = wxRound(rot_dir * pow((((double)ptd->ROTAIS) / 4.733),
                                          2));  // Convert to indicated ROT

      ptd->m_utc_sec = bstr->GetInt(138, 6);

      if ((1 == message_ID) ||
          (2 == message_ID))  // decode SOTDMA per 7.6.7.2.2
      {
        ptd->SyncState = bstr->GetInt(151, 2);
        ptd->SlotTO = bstr->GetInt(153, 2);
        if ((ptd->SlotTO == 1) && (ptd->SyncState == 0))  // UTCDirect follows
        {
          ptd->m_utc_hour = bstr->GetInt(155, 5);

          ptd->m_utc_min = bstr->GetInt(160, 7);

          if ((ptd->m_utc_hour < 24) && (ptd->m_utc_min < 60) &&
              (ptd->m_utc_sec < 60)) {
            wxDateTime rx_time(ptd->m_utc_hour, ptd->m_utc_min, ptd->m_utc_sec);
            rx_ticks = rx_time.GetTicks();
            if (!b_firstrx) {
              first_rx_ticks = rx_ticks;
              b_firstrx = true;
            }
          }
        }
      }

      //    Capture Euro Inland special passing arrangement signal ("stbd-stbd")
      ptd->blue_paddle = bstr->GetInt(144, 2);
      ptd->b_blue_paddle = (ptd->blue_paddle == 2);  // paddle is set

      if (!ptd->b_isDSCtarget)
        ptd->Class = AIS_CLASS_A;

      //    Check for SART and friends by looking at first two digits of MMSI
      int mmsi_start = ptd->MMSI / 10000000;

      if (mmsi_start == 97) {
        ptd->Class = AIS_SART;
        ptd->StaticReportTicks =
            now.GetTicks();  // won't get a static report, so fake it here

        //    On receipt of Msg 3, force any existing SART target out of
        //    acknowledge mode by adjusting its ack_time to yesterday This will
        //    cause any previously "Acknowledged" SART to re-alert.

        //    On reflection, re-alerting seems a little excessive in real life
        //    use. After all, the target is on-screen, and in the AIS target
        //    list. So lets just honor the programmed ACK timout value for SART
        //    targets as well
        // ptd->m_ack_time = wxDateTime::Now() - wxTimeSpan::Day();
      }

      parse_result = true;  // so far so good
      b_posn_report = true;

      break;
    }

    case 18: {
      ptd->NavStatus =
          UNDEFINED;  // Class B targets have no status.  Enforce this...

      ptd->SOG = 0.1 * (bstr->GetInt(47, 10));

      int lon = bstr->GetInt(58, 28);
      if (lon & 0x08000000)  // negative?
        lon |= 0xf0000000;
      double lon_tentative = lon / 600000.;

      int lat = bstr->GetInt(86, 27);
      if (lat & 0x04000000)  // negative?
        lat |= 0xf8000000;
      double lat_tentative = lat / 600000.;

      if ((lon_tentative <= 180.) &&
          (lat_tentative <=
           90.))  // Ship does not report Lat or Lon "unavailable"
      {
        ptd->Lon = lon_tentative;
        ptd->Lat = lat_tentative;
        ptd->b_positionDoubtful = false;
        ptd->b_positionOnceValid = true;  // Got the position at least once
        ptd->PositionReportTicks = now.GetTicks();
      } else
        ptd->b_positionDoubtful = true;

      ptd->COG = 0.1 * (bstr->GetInt(113, 12));
      ptd->HDG = 1.0 * (bstr->GetInt(125, 9));

      ptd->m_utc_sec = bstr->GetInt(134, 6);

      if (!ptd->b_isDSCtarget)
        ptd->Class = AIS_CLASS_B;

      parse_result = true;  // so far so good
      b_posn_report = true;

      break;
    }

    case 19: {  // Class B mes_ID 19 Is same as mes_ID 18 until bit 139
      ptd->NavStatus =
          UNDEFINED;  // Class B targets have no status.  Enforce this...
      ptd->SOG = 0.1 * (bstr->GetInt(47, 10));
      int lon = bstr->GetInt(58, 28);
      if (lon & 0x08000000)  // negative?
        lon |= 0xf0000000;
      double lon_tentative = lon / 600000.;

      int lat = bstr->GetInt(86, 27);
      if (lat & 0x04000000)  // negative?
        lat |= 0xf8000000;
      double lat_tentative = lat / 600000.;

      if ((lon_tentative <= 180.) &&
          (lat_tentative <=
           90.))  // Ship does not report Lat or Lon "unavailable"
      {
        ptd->Lon = lon_tentative;
        ptd->Lat = lat_tentative;
        ptd->b_positionDoubtful = false;
        ptd->b_positionOnceValid = true;  // Got the position at least once
        ptd->PositionReportTicks = now.GetTicks();
      } else
        ptd->b_positionDoubtful = true;

      ptd->COG = 0.1 * (bstr->GetInt(113, 12));
      ptd->HDG = 1.0 * (bstr->GetInt(125, 9));
      ptd->m_utc_sec = bstr->GetInt(134, 6);
      // From bit 140 and forward data as of mes 5
      bstr->GetStr(144, 120, &ptd->ShipName[0], SHIP_NAME_LEN);
      ptd->b_nameValid = true;
      if (!ptd->b_isDSCtarget) {
        ptd->ShipType = (unsigned char)bstr->GetInt(264, 8);
      }
      ptd->DimA = bstr->GetInt(272, 9);
      ptd->DimB = bstr->GetInt(281, 9);
      ptd->DimC = bstr->GetInt(290, 6);
      ptd->DimD = bstr->GetInt(296, 6);

      if (!ptd->b_isDSCtarget)
        ptd->Class = AIS_CLASS_B;
      parse_result = true;  // so far so good
      b_posn_report = true;

      break;
    }

    case 27: {
      // Long-range automatic identification system broadcast message
      // This message is used for long-range detection of AIS Class A and Class
      // B vessels (typically by satellite).

      // Define the constant to do the covertion from the internal encoded
      // position in message 27. The position is less accuate :  1/10 minute
      // position resolution.
      int bitCorrection = 10;
      int resolution = 10;

      // Default aout of bounce values.
      double lon_tentative = 181.;
      double lat_tentative = 91.;

#ifdef AIS_DEBUG
      printf("AIS Message 27 - received:\r\n");
      printf("MMSI      : %i\r\n", ptd->MMSI);
#endif

      // It can be both a CLASS A and a CLASS B vessel - We have decided for
      // CLASS A
      // TODO: Lookup to see if we have seen it as a CLASS B, and adjust.
      if (!ptd->b_isDSCtarget)
        ptd->Class = AIS_CLASS_A;

      ptd->NavStatus = bstr->GetInt(39, 4);

      int lon = bstr->GetInt(45, 18);
      int lat = bstr->GetInt(63, 17);

      lat_tentative = lat;
      lon_tentative = lon;

      // Negative latitude?
      if (lat >= (0x4000000 >> bitCorrection)) {
        lat_tentative = (0x8000000 >> bitCorrection) - lat;
        lat_tentative *= -1;
      }

      // Negative longitude?
      if (lon >= (0x8000000 >> bitCorrection)) {
        lon_tentative = (0x10000000 >> bitCorrection) - lon;
        lon_tentative *= -1;
      }

      // Decode the internal position format.
      lat_tentative = lat_tentative / resolution / 60.0;
      lon_tentative = lon_tentative / resolution / 60.0;

#ifdef AIS_DEBUG
      printf("Latitude  : %f\r\n", lat_tentative);
      printf("Longitude : %f\r\n", lon_tentative);
#endif

      // Get the latency of the position report.
      int positionLatency = bstr->GetInt(95, 1);

      if ((lon_tentative <= 180.) &&
          (lat_tentative <=
           90.))  // Ship does not report Lat or Lon "unavailable"
      {
        ptd->Lon = lon_tentative;
        ptd->Lat = lat_tentative;
        ptd->b_positionDoubtful = false;
        ptd->b_positionOnceValid = true;  // Got the position at least once
        if (positionLatency == 0) {
// The position is less than 5 seconds old.
#ifdef AIS_DEBUG
          printf("Low latency position report.\r\n");
#endif
          ptd->PositionReportTicks = now.GetTicks();
        }
      } else
        ptd->b_positionDoubtful = true;

      ptd->SOG = 1.0 * (bstr->GetInt(80, 6));
      ptd->COG = 1.0 * (bstr->GetInt(85, 9));

      b_posn_report = true;
      parse_result = true;
      break;
    }

    case 5: {
      n_msg5++;
      if (!ptd->b_isDSCtarget)
        ptd->Class = AIS_CLASS_A;

      //          Get the AIS Version indicator
      //          0 = station compliant with Recommendation ITU-R M.1371-1
      //          1 = station compliant with Recommendation ITU-R M.1371-3
      //          2-3 = station compliant with future editions
      int AIS_version_indicator = bstr->GetInt(39, 2);
      if (AIS_version_indicator < 4) {
        ptd->IMO = bstr->GetInt(41, 30);

        bstr->GetStr(71, 42, &ptd->CallSign[0], 7);
        bstr->GetStr(113, 120, &ptd->ShipName[0], SHIP_NAME_LEN);
        ptd->b_nameValid = true;
        if (!ptd->b_isDSCtarget) {
          ptd->ShipType = (unsigned char)bstr->GetInt(233, 8);
        }

        ptd->DimA = bstr->GetInt(241, 9);
        ptd->DimB = bstr->GetInt(250, 9);
        ptd->DimC = bstr->GetInt(259, 6);
        ptd->DimD = bstr->GetInt(265, 6);

        ptd->ETA_Mo = bstr->GetInt(275, 4);
        ptd->ETA_Day = bstr->GetInt(279, 5);
        ptd->ETA_Hr = bstr->GetInt(284, 5);
        ptd->ETA_Min = bstr->GetInt(289, 6);

        ptd->Draft = (double)(bstr->GetInt(295, 8)) / 10.0;

        bstr->GetStr(303, 120, &ptd->Destination[0], DESTINATION_LEN - 1);

        ptd->StaticReportTicks = now.GetTicks();

        parse_result = true;
      }

      break;
    }

    case 24: {  // Static data report
      int part_number = bstr->GetInt(39, 2);
      if (0 == part_number) {
        bstr->GetStr(41, 120, &ptd->ShipName[0], SHIP_NAME_LEN);
        ptd->b_nameValid = true;
        parse_result = true;
        n_msg24++;
      } else if (1 == part_number) {
        if (!ptd->b_isDSCtarget) {
          ptd->ShipType = (unsigned char)bstr->GetInt(41, 8);
        }
        bstr->GetStr(91, 42, &ptd->CallSign[0], 7);

        ptd->DimA = bstr->GetInt(133, 9);
        ptd->DimB = bstr->GetInt(142, 9);
        ptd->DimC = bstr->GetInt(151, 6);
        ptd->DimD = bstr->GetInt(157, 6);
        parse_result = true;
      }
      break;
    }
    case 4:  // base station
    {
      ptd->Class = AIS_BASE;

      ptd->m_utc_hour = bstr->GetInt(62, 5);
      ptd->m_utc_min = bstr->GetInt(67, 6);
      ptd->m_utc_sec = bstr->GetInt(73, 6);
      //                              (79,  1);
      int lon = bstr->GetInt(80, 28);
      if (lon & 0x08000000)  // negative?
        lon |= 0xf0000000;
      double lon_tentative = lon / 600000.;

      int lat = bstr->GetInt(108, 27);
      if (lat & 0x04000000)  // negative?
        lat |= 0xf8000000;
      double lat_tentative = lat / 600000.;

      if ((lon_tentative <= 180.) &&
          (lat_tentative <=
           90.))  // Ship does not report Lat or Lon "unavailable"
      {
        ptd->Lon = lon_tentative;
        ptd->Lat = lat_tentative;
        ptd->b_positionDoubtful = false;
        ptd->b_positionOnceValid = true;  // Got the position at least once
        ptd->PositionReportTicks = now.GetTicks();
      } else
        ptd->b_positionDoubtful = true;

      ptd->COG = -1.;
      ptd->HDG = 511;
      ptd->SOG = -1.;

      parse_result = true;
      b_posn_report = true;

      break;
    }
    case 9:  // Special Position Report (Standard SAR Aircraft Position Report)
    {
      ptd->SOG = bstr->GetInt(51, 10);

      int lon = bstr->GetInt(62, 28);
      if (lon & 0x08000000)  // negative?
        lon |= 0xf0000000;
      double lon_tentative = lon / 600000.;

      int lat = bstr->GetInt(90, 27);
      if (lat & 0x04000000)  // negative?
        lat |= 0xf8000000;
      double lat_tentative = lat / 600000.;

      if ((lon_tentative <= 180.) &&
          (lat_tentative <=
           90.))  // Ship does not report Lat or Lon "unavailable"
      {
        ptd->Lon = lon_tentative;
        ptd->Lat = lat_tentative;
        ptd->b_positionDoubtful = false;
        ptd->b_positionOnceValid = true;  // Got the position at least once
        ptd->PositionReportTicks = now.GetTicks();
      } else
        ptd->b_positionDoubtful = true;

      //    decode balance of message....
      ptd->COG = 0.1 * (bstr->GetInt(117, 12));

      int alt_tent = bstr->GetInt(39, 12);
      ptd->altitude = alt_tent;

      ptd->b_SarAircraftPosnReport = true;

      parse_result = true;
      b_posn_report = true;

      break;
    }
    case 21:  // Test Message (Aid to Navigation)
    {
      ptd->ShipType = (unsigned char)bstr->GetInt(39, 5);
      ptd->IMO = 0;
      ptd->SOG = 0;
      ptd->HDG = 0;
      ptd->COG = 0;
      ptd->ROTAIS = -128;  // i.e. not available
      ptd->DimA = bstr->GetInt(220, 9);
      ptd->DimB = bstr->GetInt(229, 9);
      ptd->DimC = bstr->GetInt(238, 6);
      ptd->DimD = bstr->GetInt(244, 6);
      ptd->Draft = 0;

      ptd->m_utc_sec = bstr->GetInt(254, 6);

      int offpos = bstr->GetInt(260, 1);  // off position flag
      int virt = bstr->GetInt(270, 1);    // virtual flag

      if (virt)
        ptd->NavStatus = ATON_VIRTUAL;
      else
        ptd->NavStatus = ATON_REAL;
      if (ptd->m_utc_sec <= 59 /*&& !virt*/) {
        ptd->NavStatus += 1;
        if (offpos) ptd->NavStatus += 1;
      }

      bstr->GetStr( 44, 120, &ptd->ShipName[0], SHIP_NAME_LEN);
      // short name only, extension wont fit in Ship structure

      if (bstr->GetBitCount() > 276) {
        int nx = ((bstr->GetBitCount() - 272) / 6) * 6;
        bstr->GetStr(273, nx, &ptd->ShipNameExtension[0], 14);
        ptd->ShipNameExtension[14] = 0;
      } else {
        ptd->ShipNameExtension[0] = 0;
      }

      ptd->b_nameValid = true;

      parse_result = true;  // so far so good

      ptd->Class = AIS_ATON;

      int lon = bstr->GetInt(165, 28);

      if (lon & 0x08000000)  // negative?
        lon |= 0xf0000000;
      double lon_tentative = lon / 600000.;

      int lat = bstr->GetInt(193, 27);

      if (lat & 0x04000000)  // negative?
        lat |= 0xf8000000;
      double lat_tentative = lat / 600000.;

      if ((lon_tentative <= 180.) &&
          (lat_tentative <=
           90.))  // Ship does not report Lat or Lon "unavailable"
      {
        ptd->Lon = lon_tentative;
        ptd->Lat = lat_tentative;
        ptd->b_positionDoubtful = false;
        ptd->b_positionOnceValid = true;  // Got the position at least once
        ptd->PositionReportTicks = now.GetTicks();
      } else
        ptd->b_positionDoubtful = true;

      b_posn_report = true;
      break;
    }
    case 8:  // Binary Broadcast
    {
      int dac = bstr->GetInt(41, 10);
      int fi = bstr->GetInt(51, 6);
      if (dac == 200)  // European inland
      {
        if (fi == 10)  // "Inland ship static and voyage related data"
        {
          ptd->b_isEuroInland = true;

          bstr->GetStr(57, 48, &ptd->Euro_VIN[0], 8);
          ptd->Euro_Length = ((double)bstr->GetInt(105, 13)) / 10.0;
          ptd->Euro_Beam = ((double)bstr->GetInt(118, 10)) / 10.0;
          ptd->UN_shiptype = bstr->GetInt(128, 14);
          ptd->Euro_Draft = ((double)bstr->GetInt(145, 11)) / 100.0;
          parse_result = true;
        }
      }
      if (dac == 1)  // IMO
      {
        if (fi == 22)  // Area Notice
        {
          if (bstr->GetBitCount() >= 111) {
            Ais8_001_22 an;
            an.link_id = bstr->GetInt(57, 10);
            an.notice_type = bstr->GetInt(67, 7);
            an.month = bstr->GetInt(74, 4);
            an.day = bstr->GetInt(78, 5);
            an.hour = bstr->GetInt(83, 5);
            an.minute = bstr->GetInt(88, 6);
            an.duration_minutes = bstr->GetInt(94, 18);

            wxDateTime now = wxDateTime::Now();
            now.MakeGMT();

            an.start_time.Set(an.day, wxDateTime::Month(an.month - 1),
                              now.GetYear(), an.hour, an.minute);

            // msg is not supposed to be transmitted more than a day before it
            // comes into effect, so a start_time less than a day or two away
            // might indicate a month rollover
            if (an.start_time > now + wxTimeSpan::Hours(48))
              an.start_time.Set(an.day, wxDateTime::Month(an.month - 1),
                                now.GetYear() - 1, an.hour, an.minute);

            an.expiry_time =
                an.start_time + wxTimeSpan::Minutes(an.duration_minutes);

            // msg is not supposed to be transmitted beyond expiration, so
            // taking into account a fudge factor for clock issues, assume an
            // expiry date in the past indicates incorrect year
            if (an.expiry_time < now - wxTimeSpan::Hours(24)) {
              an.start_time.Set(an.day, wxDateTime::Month(an.month - 1),
                                now.GetYear() + 1, an.hour, an.minute);
              an.expiry_time =
                  an.start_time + wxTimeSpan::Minutes(an.duration_minutes);
            }

            int subarea_count = (bstr->GetBitCount() - 111) / 87;
            for (int i = 0; i < subarea_count; ++i) {
              int base = 111 + i * 87;
              Ais8_001_22_SubArea sa;
              sa.shape = bstr->GetInt(base + 1, 3);
              int scale_factor = 1;
              if (sa.shape == AIS8_001_22_SHAPE_TEXT) {
                char t[15];
                t[14] = 0;
                bstr->GetStr(base + 4, 84, t, 14);
                sa.text = wxString(t, wxConvUTF8);
              } else {
                int scale_multipliers[4] = {1, 10, 100, 1000};
                scale_factor = scale_multipliers[bstr->GetInt(base + 4, 2)];
                switch (sa.shape) {
                  case AIS8_001_22_SHAPE_CIRCLE:
                  case AIS8_001_22_SHAPE_SECTOR:
                    sa.radius_m = bstr->GetInt(base + 58, 12) * scale_factor;
                    // FALL THROUGH
                  case AIS8_001_22_SHAPE_RECT:
                    sa.longitude = bstr->GetInt(base + 6, 25, true) / 60000.0;
                    sa.latitude = bstr->GetInt(base + 31, 24, true) / 60000.0;
                    break;
                  case AIS8_001_22_SHAPE_POLYLINE:
                  case AIS8_001_22_SHAPE_POLYGON:
                    for (int i = 0; i < 4; ++i) {
                      sa.angles[i] = bstr->GetInt(base + 6 + i * 20, 10) * 0.5;
                      sa.dists_m[i] =
                          bstr->GetInt(base + 16 + i * 20, 10) * scale_factor;
                    }
                }
                if (sa.shape == AIS8_001_22_SHAPE_RECT) {
                  sa.e_dim_m = bstr->GetInt(base + 58, 8) * scale_factor;
                  sa.n_dim_m = bstr->GetInt(base + 66, 8) * scale_factor;
                  sa.orient_deg = bstr->GetInt(base + 74, 9);
                }
                if (sa.shape == AIS8_001_22_SHAPE_SECTOR) {
                  sa.left_bound_deg = bstr->GetInt(70, 9);
                  sa.right_bound_deg = bstr->GetInt(79, 9);
                }
              }
              an.sub_areas.push_back(sa);
            }
            ptd->area_notices[an.link_id] = an;
            parse_result = true;
          }
        }

          // Meteorological and Hydrographic data ref: IMO SN.1/Circ.289
        if (fi == 31) {
          if (bstr->GetBitCount() >= 360) {
            //Ais8_001_31  mmsi can have been changed.
            if (met_mmsi != 666) ptd->MMSI = met_mmsi;

            // Default out of bounce values.
            double lon_tentative = 181.;
            double lat_tentative = 91.;

            int lon = bstr->GetInt(57, 25);
            int lat = bstr->GetInt(82, 24);

            if (lon & 0x01000000)  // negative?
               lon |= 0xFE000000;
            lon_tentative = lon / 60000.;

            if (lat & 0x00800000)  //negative?
               lat |= 0xFF000000;
            lat_tentative = lat / 60000.;

            ptd->Lon = lon_tentative;
            ptd->Lat = lat_tentative;

              // Try to make unique name for each station based on position
            wxString x = ptd->ShipName;
            if (x.Find("METEO") == wxNOT_FOUND) {
            double id1, id2;
            wxString slat = wxString::Format("%0.3f", lat_tentative);
            wxString slon = wxString::Format("%0.3f", lon_tentative);
            slat.ToDouble(&id1);
            slon.ToDouble(&id2);
            wxString nameID = "METEO ";
            nameID << wxString::Format("%0.3f", abs(id1) + abs(id2)).Right(3);
            strncpy(ptd->ShipName, nameID, SHIP_NAME_LEN - 1);
            }

            ptd->met_data.pos_acc = bstr->GetInt(106, 1);
            ptd->met_data.day = bstr->GetInt(107, 5);
            ptd->met_data.hour = bstr->GetInt(112, 5);
            ptd->met_data.minute = bstr->GetInt(117, 6);
            ptd->met_data.wind_kn = bstr->GetInt(123, 7);
            ptd->met_data.wind_gust_kn = bstr->GetInt(130, 7);
            ptd->met_data.wind_dir = bstr->GetInt(137, 9);
            ptd->met_data.wind_gust_dir = bstr->GetInt(146, 9);

            int tmp = bstr->GetInt(155, 11);
            if (tmp & 0x00000400)  // negative?
              tmp |= 0xFFFFF800;
            ptd->met_data.air_temp = tmp / 10.;
            ptd->met_data.rel_humid = bstr->GetInt(166, 7);
            int dew = bstr->GetInt(173, 10);
            if (dew & 0x00000200)  // negative? (bit 9 = 1)
              dew |= 0xFFFFFC00;
            ptd->met_data.dew_point = dew / 10.;

            /*Air pressure, defined as pressure reduced to sea level,
              in 1 hPa steps.0 = pressure 799 hPa or less
              1 - 401 = 800 - 1200 hPa*/
            ptd->met_data.airpress = bstr->GetInt(183, 9) + 799;
            ptd->met_data.airpress_tend = bstr->GetInt(192, 2);
            ptd->met_data.hor_vis = bstr->GetInt(194, 8) / 10.;
            //int MSB = bstr->GetInt(194, 8) < 0;

            ptd->met_data.water_level = (bstr->GetInt(202, 12) / 100.) - 10.;
            ptd->met_data.water_lev_trend = bstr->GetInt(214, 2);
            ptd->met_data.current = bstr->GetInt(216, 8) / 10.;
            ptd->met_data.curr_dir = bstr->GetInt(224, 9);
            ptd->met_data.wave_hight = bstr->GetInt(277, 8) / 10.;
            ptd->met_data.wave_period = bstr->GetInt(285, 6);
            ptd->met_data.wave_dir = bstr->GetInt(291, 9);
            ptd->met_data.swell_hight = bstr->GetInt(300, 8) / 10;
            ptd->met_data.swell_per = bstr->GetInt(308, 6);
            ptd->met_data.swell_dir = bstr->GetInt(314, 9);
            ptd->met_data.seastate = bstr->GetInt(323, 4);

            int wt = bstr->GetInt(327, 10);
            if (wt & 0x00000200)  // negative? (bit 9 = 1)
              wt |= 0xFFFFFC00;
            ptd->met_data.water_temp = wt / 10.;

            ptd->met_data.precipitation = bstr->GetInt(337, 3);
            ptd->met_data.salinity = bstr->GetInt(340, 9) / 10.;
            ptd->met_data.ice = bstr->GetInt(349, 2);

            ptd->Class = AIS_METEO;
            ptd->b_NoTrack = true;
            ptd->b_show_track = false;
            ptd->b_positionDoubtful = false;
            ptd->b_positionOnceValid = true;
            b_posn_report = true;
            ptd->PositionReportTicks = now.GetTicks();
            ptd->b_nameValid = true;
            ptd->b_show_AIS_CPA = false;
            ptd->bCPA_Valid = false;

            parse_result = true;
          }
        }
      }
      break;
    }
    case 14:  // Safety Related Broadcast
    {
      //  Always capture the MSG_14 text
      char msg_14_text[968];
      if (bstr->GetBitCount() > 40) {
        int nx = ((bstr->GetBitCount() - 40) / 6) * 6;
        int nd = bstr->GetStr(41, nx, msg_14_text, 968);
        nd = wxMax(0, nd);
        nd = wxMin(nd, 967);
        msg_14_text[nd] = 0;
        ptd->MSG_14_text = wxString(msg_14_text, wxConvUTF8);
      }
      parse_result = true;  // so far so good

      break;
    }

    case 6:  // Addressed Binary Message
    {
      break;
    }
    case 7:  // Binary Ack
    {
      break;
    }
    default: {
      break;
    }
  }

  if (b_posn_report) ptd->b_lost = false;

  if (true == parse_result) {
    //      Revalidate the target under some conditions
    if (!ptd->b_active && !ptd->b_positionDoubtful && b_posn_report)
      ptd->b_active = true;
  }

  return parse_result;
}

bool AisDecoder::NMEACheckSumOK(const wxString &str_in) {
  unsigned char checksum_value = 0;
  int sentence_hex_sum;

  wxCharBuffer buf = str_in.ToUTF8();
  if (!buf.data()) return false;  // cannot decode string

  char str_ascii[AIS_MAX_MESSAGE_LEN + 1];
  strncpy(str_ascii, buf.data(), AIS_MAX_MESSAGE_LEN);
  str_ascii[AIS_MAX_MESSAGE_LEN] = '\0';

  int string_length = strlen(str_ascii);

  int payload_length = 0;
  while ((payload_length < string_length) &&
         (str_ascii[payload_length] != '*'))  // look for '*'
    payload_length++;

  if (payload_length == string_length)
    return false;  // '*' not found at all, no checksum

  int index = 1;  // Skip over the $ at the begining of the sentence

  while (index < payload_length) {
    checksum_value ^= str_ascii[index];
    index++;
  }

  if (string_length > 4) {
    char scanstr[3];
    scanstr[0] = str_ascii[payload_length + 1];
    scanstr[1] = str_ascii[payload_length + 2];
    scanstr[2] = 0;
    sscanf(scanstr, "%2x", &sentence_hex_sum);

    if (sentence_hex_sum == checksum_value) return true;
  }

  return false;
}

void AisDecoder::UpdateAllCPA(void) {
  //    Iterate thru all the targets
  for (const auto &it : GetTargetList()) {
    std::shared_ptr<AisTargetData> td = it.second;

    if (NULL != td) UpdateOneCPA(td.get());
  }
}

void AisDecoder::UpdateAllTracks(void) {
  //    Iterate thru all the targets
  for (const auto &it : GetTargetList()) {
    std::shared_ptr<AisTargetData> td = it.second;

    if (NULL != td) UpdateOneTrack(td.get());
  }
}

void AisDecoder::UpdateOneTrack(AisTargetData *ptarget) {
  if (!ptarget->b_positionOnceValid) return;
  // Reject for unbelievable jumps (corrupted/bad data)
  if (ptarget->m_ptrack.size() > 0) {
    const AISTargetTrackPoint &LastTrackpoint = ptarget->m_ptrack.back();
    if (fabs(LastTrackpoint.m_lat - ptarget->Lat) > .1 ||
        fabs(LastTrackpoint.m_lon - ptarget->Lon) > .1) {
      // after an unlikely jump in pos, the last trackpoint might also be wrong
      // just to be sure we do delete this one as well.
      ptarget->m_ptrack.pop_back();
      ptarget->b_positionDoubtful = true;
      return;
    }
  }

  //    Add the newest point
  AISTargetTrackPoint ptrackpoint;
  ptrackpoint.m_lat = ptarget->Lat;
  ptrackpoint.m_lon = ptarget->Lon;
  ptrackpoint.m_time = wxDateTime::Now().GetTicks();

  ptarget->m_ptrack.push_back(ptrackpoint);

  if (ptarget->b_PersistTrack || ptarget->b_mPropPersistTrack) {
    Track *t;
    if (0 == m_persistent_tracks.count(ptarget->MMSI)) {
      t = new Track();
      t->SetName(wxString::Format(_T("AIS %s (%u) %s %s"),
                                  ptarget->GetFullName().c_str(), ptarget->MMSI,
                                  wxDateTime::Now().FormatISODate().c_str(),
                                  wxDateTime::Now().FormatISOTime().c_str()));
      g_TrackList.push_back(t);
      new_track.Notify(t);
      m_persistent_tracks[ptarget->MMSI] = t;
    } else {
      t = m_persistent_tracks[ptarget->MMSI];
    }
    TrackPoint *tp = t->GetLastPoint();
    vector2D point(ptrackpoint.m_lon, ptrackpoint.m_lat);
    TrackPoint *tp1 =
        t->AddNewPoint(point, wxDateTime(ptrackpoint.m_time).ToUTC());
    if (tp) {
      //Fixme (dave)  This is GUI related.
//       pSelect->AddSelectableTrackSegment(tp->m_lat, tp->m_lon, tp1->m_lat,
//                                          tp1->m_lon, tp, tp1, t);
    }

    // We do not want dependency on the GUI here, do we?
    //        if( pRouteManagerDialog && pRouteManagerDialog->IsShown() )
    //                pRouteManagerDialog->UpdateTrkListCtrl();
  }
  else {

    //    Walk the list, removing any track points that are older than the
    //    stipulated time

    time_t test_time =
      wxDateTime::Now().GetTicks() - (time_t)( g_AISShowTracks_Mins * 60 );

    ptarget->m_ptrack.erase(
      std::remove_if(ptarget->m_ptrack.begin(), ptarget->m_ptrack.end(),
                     [=](const AISTargetTrackPoint &track) {
      return track.m_time < test_time;
    }),
      ptarget->m_ptrack.end());
  }
}

void AisDecoder::DeletePersistentTrack(Track *track) {
  for (std::map<int, Track *>::iterator iterator = m_persistent_tracks.begin();
       iterator != m_persistent_tracks.end(); iterator++) {
    if (iterator->second == track) {
      int mmsi = iterator->first;
      m_persistent_tracks.erase(iterator);
      //Last tracks for this target?
      if (0 == m_persistent_tracks.count(mmsi)) {
        for (unsigned int i = 0; i < g_MMSI_Props_Array.GetCount(); i++) {
          if (mmsi == g_MMSI_Props_Array[i]->MMSI) {
            MmsiProperties *props = g_MMSI_Props_Array[i];
            if (props->m_bPersistentTrack) {
              // Ask if mmsi props should be changed.
              // Avoid creation of a new track while messaging

              std::shared_ptr<AisTargetData> td = Get_Target_Data_From_MMSI(mmsi);
              if (td) {
                props->m_bPersistentTrack = false;
                td->b_mPropPersistTrack = false;
              }
              if (!m_callbacks.confirm_stop_track()) {
                props->m_bPersistentTrack = true;
              }
            }
            break;
          }
        }
      }
      break;
    }
  }
}

void AisDecoder::UpdateAllAlarms(void) {
  m_bGeneralAlert = false;  // no alerts yet

  //    Iterate thru all the targets
  for (const auto &it : GetTargetList()) {
    std::shared_ptr <AisTargetData> td = it.second;

    if (NULL != td) {
      //  Maintain General Alert
      if (!m_bGeneralAlert) {
        //    Quick check on basic condition
        if ((td->CPA < g_CPAWarn_NM) && (td->TCPA > 0) &&
            (td->Class != AIS_ATON) && (td->Class != AIS_BASE))
          m_bGeneralAlert = true;

        //    Some options can suppress general alerts
        if (g_bAIS_CPA_Alert_Suppress_Moored && (td->SOG <= g_ShowMoored_Kts))
          m_bGeneralAlert = false;

        //    Skip distant targets if requested
        if ((g_bCPAMax) && (td->Range_NM > g_CPAMax_NM))
          m_bGeneralAlert = false;

        //    Skip if TCPA is too long
        if ((g_bTCPA_Max) && (td->TCPA > g_TCPA_Max)) m_bGeneralAlert = false;

        //  SART targets always alert if "Active"
        if (td->Class == AIS_SART && td->NavStatus == 14)
          m_bGeneralAlert = true;

        //  DSC Distress targets always alert
        if (( td->Class == AIS_DSC ) && ( ( td->ShipType == 12 ) || ( td->ShipType == 16 ) ))
          m_bGeneralAlert = true;
      }

      ais_alert_type this_alarm = AIS_NO_ALERT;

      //  SART targets always alert if "Active"
      if (td->Class == AIS_SART && td->NavStatus == 14)
        this_alarm = AIS_ALERT_SET;

      //  DSC Distress targets always alert
      if ((td->Class == AIS_DSC) && (( td->ShipType == 12 ) || ( td->ShipType == 16 )) )
        this_alarm = AIS_ALERT_SET;

      if (g_bCPAWarn && td->b_active && td->b_positionOnceValid &&
          (td->Class != AIS_SART) && (td->Class != AIS_DSC)) {
        //      Skip anchored/moored(interpreted as low speed) targets if
        //      requested
        if ((g_bHideMoored) && (td->SOG <= g_ShowMoored_Kts)) {  // dsr
          td->n_alert_state = AIS_NO_ALERT;
          continue;
        }

        //    No Alert on moored(interpreted as low speed) targets if so
        //    requested
        if (g_bAIS_CPA_Alert_Suppress_Moored &&
            (td->SOG <= g_ShowMoored_Kts)) {  // dsr
          td->n_alert_state = AIS_NO_ALERT;
          continue;
        }

        //    Skip distant targets if requested
        if (g_bCPAMax) {
          if (td->Range_NM > g_CPAMax_NM) {
            td->n_alert_state = AIS_NO_ALERT;
            continue;
          }
        }

        if ((td->CPA < g_CPAWarn_NM) && (td->TCPA > 0) &&
            (td->Class != AIS_ATON) && (td->Class != AIS_BASE)) {
          if (g_bTCPA_Max) {
            if (td->TCPA < g_TCPA_Max) {
              if (td->b_isFollower)
                this_alarm = AIS_ALERT_NO_DIALOG_SET;
              else
                this_alarm = AIS_ALERT_SET;
            }
          } else {
            if (td->b_isFollower)
              this_alarm = AIS_ALERT_NO_DIALOG_SET;
            else
              this_alarm = AIS_ALERT_SET;
          }
        }
      }

      //    Maintain the timer for in_ack flag
      //  SART and DSC targets always maintain ack timeout

      if (g_bAIS_ACK_Timeout || (td->Class == AIS_SART) ||
          ((td->Class == AIS_DSC) && ((td->ShipType == 12) || (td->ShipType == 16)) )) {
        if (td->b_in_ack_timeout) {
          wxTimeSpan delta = wxDateTime::Now() - td->m_ack_time;
          if (delta.GetMinutes() > g_AckTimeout_Mins)
            td->b_in_ack_timeout = false;
        }
      } else {
        //  Not using ack timeouts.
        //  If a target has been acknowledged, leave it ack'ed until it goes out
        //  of AIS_ALARM_SET state
        if (td->b_in_ack_timeout) {
          if (this_alarm == AIS_NO_ALERT) td->b_in_ack_timeout = false;
        }
      }

      td->n_alert_state = this_alarm;
    }
  }
}

void AisDecoder::UpdateOneCPA(AisTargetData *ptarget) {
  ptarget->Range_NM = -1.;  // Defaults
  ptarget->Brg = -1.;

  //    Compute the current Range/Brg to the target
  //    This should always be possible even if GPS data is not valid
  //    because O must always have a position for own-ship. Plugins need
  //    AIS target range and bearing from own-ship position even if GPS is not
  //    valid.
  double brg, dist;
  DistanceBearingMercator(ptarget->Lat, ptarget->Lon, gLat, gLon, &brg, &dist);
  ptarget->Range_NM = dist;
  ptarget->Brg = brg;

  if (dist <= 1e-5) ptarget->Brg = -1.0;  // Brg is undefined if Range == 0.

  if (!ptarget->b_positionOnceValid || !bGPSValid) {
    ptarget->bCPA_Valid = false;
    return;
  }

  //    There can be no collision between ownship and itself....
  //    This can happen if AIVDO messages are received, and there is another
  //    source of ownship position, like NMEA GLL The two positions are always
  //    temporally out of sync, and one will always be exactly in front of the
  //    other one.
  if (ptarget->b_OwnShip) {
    ptarget->CPA = 100;
    ptarget->TCPA = -100;
    ptarget->bCPA_Valid = false;
    return;
  }

  double cpa_calc_ownship_cog = gCog;
  double cpa_calc_target_cog = ptarget->COG;

  //    Ownship is not reporting valid SOG, so no way to calculate CPA
  if (std::isnan(gSog) || (gSog > 102.2)) {
    ptarget->bCPA_Valid = false;
    return;
  }

  //    Ownship is maybe anchored and not reporting COG
  if (std::isnan(gCog) || gCog == 360.0) {
    if (gSog < .01)
      cpa_calc_ownship_cog =
          0.;  // substitute value
               // for the case where SOG ~= 0, and COG is unknown.
    else {
      ptarget->bCPA_Valid = false;
      return;
    }
  }

  //    Target is maybe anchored and not reporting COG
  if (ptarget->COG == 360.0) {
    if (ptarget->SOG > 102.2) {
      ptarget->bCPA_Valid = false;
      return;
    } else if (ptarget->SOG < .01)
      cpa_calc_target_cog =
          0.;  // substitute value
               // for the case where SOG ~= 0, and COG is unknown.
    else {
      ptarget->bCPA_Valid = false;
      return;
    }
  }

  //    Express the SOGs as meters per hour
  double v0 = gSog * 1852.;
  double v1 = ptarget->SOG * 1852.;

  if ((v0 < 1e-6) && (v1 < 1e-6)) {
    ptarget->TCPA = 0.;
    ptarget->CPA = 0.;

    ptarget->bCPA_Valid = false;
  } else {
    //    Calculate the TCPA first

    //    Working on a Reduced Lat/Lon orthogonal plotting sheet....
    //    Get easting/northing to target,  in meters

    double east1 = (ptarget->Lon - gLon) * 60 * 1852;
    double north1 = (ptarget->Lat - gLat) * 60 * 1852;

    double east = east1 * (cos(gLat * PI / 180.));

    double north = north1;

    //    Convert COGs trigonometry to standard unit circle
    double cosa = cos((90. - cpa_calc_ownship_cog) * PI / 180.);
    double sina = sin((90. - cpa_calc_ownship_cog) * PI / 180.);
    double cosb = cos((90. - cpa_calc_target_cog) * PI / 180.);
    double sinb = sin((90. - cpa_calc_target_cog) * PI / 180.);

    //    These will be useful
    double fc = (v0 * cosa) - (v1 * cosb);
    double fs = (v0 * sina) - (v1 * sinb);

    double d = (fc * fc) + (fs * fs);
    double tcpa;

    // the tracks are almost parallel
    if (fabs(d) < 1e-6)
      tcpa = 0.;
    else
      //    Here is the equation for t, which will be in hours
      tcpa = ((fc * east) + (fs * north)) / d;

    //    Convert to minutes
    ptarget->TCPA = tcpa * 60.;

    //    Calculate CPA
    //    Using TCPA, predict ownship and target positions

    double OwnshipLatCPA, OwnshipLonCPA, TargetLatCPA, TargetLonCPA;

    ll_gc_ll(gLat, gLon, cpa_calc_ownship_cog, gSog * tcpa, &OwnshipLatCPA,
             &OwnshipLonCPA);
    ll_gc_ll(ptarget->Lat, ptarget->Lon, cpa_calc_target_cog,
             ptarget->SOG * tcpa, &TargetLatCPA, &TargetLonCPA);

    //   And compute the distance
    ptarget->CPA = DistGreatCircle(OwnshipLatCPA, OwnshipLonCPA, TargetLatCPA,
                                   TargetLonCPA);

    ptarget->bCPA_Valid = true;

    if (ptarget->TCPA < 0) ptarget->bCPA_Valid = false;
  }
}

void AisDecoder::OnTimerDSC(wxTimerEvent &event) {
  //  Timer expired, no CDDSE message was received, so accept the latest CDDSC
  //  message
  if (m_ptentative_dsctarget) {
    ProcessDSx(m_dsc_last_string, true);
  }
}

void AisDecoder::OnTimerAIS(wxTimerEvent &event) {
  TimerAIS.Stop();
  //    Scrub the target hash list
  //    removing any targets older than stipulated age

  wxDateTime now = wxDateTime::Now();
  now.MakeGMT();

  std::unordered_map<int, std::shared_ptr<AisTargetData>> &current_targets = GetTargetList();

  auto it = current_targets.begin();
  std::vector<int> remove_array;  // collector for MMSI of targets to be removed

  while (it != current_targets.end()) {
    if (it->second == NULL)  // This should never happen, but I saw it once....
    {
      current_targets.erase(it);
      break;  // leave the loop
    }
    //std::shared_ptr<AisTargetData> xtd(std::make_shared<AisTargetData>(*it->second));
    auto xtd = it->second;

    int target_posn_age = now.GetTicks() - xtd->PositionReportTicks;
    int target_static_age = now.GetTicks() - xtd->StaticReportTicks;

    //        Global variables controlling lost target handling
    // g_bMarkLost
    // g_MarkLost_Mins       // Minutes until black "cross out
    // g_bRemoveLost
    // g_RemoveLost_Mins);   // minutes until target is removed from screen and
    // internal lists

    // g_bInlandEcdis

    //      Mark lost targets if specified
    double removelost_Mins = fmax(g_RemoveLost_Mins, g_MarkLost_Mins);

    if (g_bInlandEcdis && (xtd->Class != AIS_ARPA)) {
      double iECD_LostTimeOut = 0.0;
      // special rules apply for europe inland ecdis timeout settings. overrule
      // option settings Won't apply for ARPA targets where the radar has all
      // control
      if (xtd->Class == AIS_CLASS_B) {
        if ((xtd->NavStatus == MOORED) || (xtd->NavStatus == AT_ANCHOR))
          iECD_LostTimeOut = 18 * 60;
        else
          iECD_LostTimeOut = 180;
      }
      if (xtd->Class == AIS_CLASS_A) {
        if ((xtd->NavStatus == MOORED) || (xtd->NavStatus == AT_ANCHOR)) {
          if (xtd->SOG < 3.)
            iECD_LostTimeOut = 18 * 60;
          else
            iECD_LostTimeOut = 60;
        } else
          iECD_LostTimeOut = 60;
      }

      if ((target_posn_age > iECD_LostTimeOut) && (xtd->Class != AIS_GPSG_BUDDY))
        xtd->b_active = false;

      removelost_Mins = (2 * iECD_LostTimeOut) / 60.;
    } else if (g_bMarkLost) {
      if ((target_posn_age > g_MarkLost_Mins * 60) &&
          (xtd->Class != AIS_GPSG_BUDDY))
        xtd->b_active = false;
    }

    if (xtd->Class == AIS_SART || xtd->Class == AIS_METEO)
      removelost_Mins = 18.0;

    //      Remove lost targets if specified

    if (g_bRemoveLost || g_bInlandEcdis) {
      bool b_arpalost =
          (xtd->Class == AIS_ARPA &&
           xtd->b_lost);  // A lost ARPA target would be deleted at once
      if (((target_posn_age > removelost_Mins * 60) &&
           (xtd->Class != AIS_GPSG_BUDDY)) ||
          b_arpalost) {
        //      So mark the target as lost, with unknown position, and make it
        //      not selectable
        xtd->b_lost = true;
        xtd->b_positionOnceValid = false;
        xtd->COG = 360.0;
        xtd->SOG = 103.0;
        xtd->HDG = 511.0;
        xtd->ROTAIS = -128;

        plugin_msg.Notify(xtd, "");

        long mmsi_long = xtd->MMSI;
        pSelectAIS->DeleteSelectablePoint((void *)mmsi_long, SELTYPE_AISTARGET);

        //      If we have not seen a static report in 3 times the removal spec,
        //      then remove the target from all lists
        //      or a lost ARPA target.
        if (target_static_age > removelost_Mins * 60 * 3 || b_arpalost) {
          xtd->b_removed = true;
          plugin_msg.Notify(xtd, "");
          remove_array.push_back(xtd->MMSI);  // Add this target to removal list
        }
      }
    }

    // Remove any targets specified as to be "ignored", so that they won't
    // trigger phantom alerts (e.g. SARTs)
    for (unsigned int i = 0; i < g_MMSI_Props_Array.GetCount(); i++) {
      MmsiProperties *props = g_MMSI_Props_Array[i];
      if (xtd->MMSI == props->MMSI) {
        if (props->m_bignore) {
          remove_array.push_back(xtd->MMSI);  // Add this target to removal list
          xtd->b_removed = true;
          plugin_msg.Notify(xtd, "");
        }
        break;
      }
    }

    ++it;
  }

  //  Remove all the targets collected in remove_array in one pass
  for (unsigned int i = 0; i < remove_array.size(); i++) {
    auto itd = current_targets.find(remove_array[i]);
    if (itd != current_targets.end()) {
      auto td = itd->second;
      current_targets.erase(itd);
      //delete td;
    }
  }

  UpdateAllCPA();
  UpdateAllAlarms();

  //    Update the general suppression flag
  m_bSuppressed = false;
  if (g_bAIS_CPA_Alert_Suppress_Moored || g_bHideMoored ||
      (g_bShowScaled && g_bAllowShowScaled))
    m_bSuppressed = true;

  m_bAIS_Audio_Alert_On = false;  // default, may be set on

  //    Process any Alarms

  //    If the AIS Alert Dialog is not currently shown....

  //    Scan all targets, looking for SART, DSC Distress, and CPA incursions
  //    In the case of multiple targets of the same type, select the shortest
  //    range or shortest TCPA
  std::shared_ptr<AisTargetData> palert_target = NULL;
  int audioType = AISAUDIO_NONE;

  if (NULL == g_pais_alert_dialog_active) {
    pAISMOBRoute = NULL;    // Reset the AISMOB auto route.
    double tcpa_min = 1e6;  // really long
    double sart_range = 1e6;
    std::shared_ptr<AisTargetData> palert_target_cpa = NULL;
    std::shared_ptr<AisTargetData> palert_target_sart = NULL;
    std::shared_ptr<AisTargetData> palert_target_dsc = NULL;

    for (it = current_targets.begin(); it != current_targets.end(); ++it) {
      auto td = it->second;
      if (td) {
        if ((td->Class != AIS_SART) && (td->Class != AIS_DSC)) {
          if (g_bAIS_CPA_Alert && td->b_active) {
            if ((AIS_ALERT_SET == td->n_alert_state) && !td->b_in_ack_timeout) {
              if (td->TCPA < tcpa_min) {
                tcpa_min = td->TCPA;
                palert_target_cpa = td;
              }
            }
          }
        } else if ((td->Class == AIS_DSC) && ((td->ShipType == 12) || (td->ShipType == 16)) ) {
          if (td->b_active) {
            if ((AIS_ALERT_SET == td->n_alert_state) && !td->b_in_ack_timeout) {
              palert_target_dsc = td;
            }
            else {  // Reset DCS flag to open for a real AIS for the same target
              td->b_isDSCtarget = false;
            }
          }
        }

        else if (td->Class == AIS_SART) {
          if (td->b_active) {
            if ((AIS_ALERT_SET == td->n_alert_state) && !td->b_in_ack_timeout) {
              if (td->Range_NM < sart_range) {
                tcpa_min = sart_range;
                palert_target_sart = td;
              }
            }
          }
        }
      }
    }

    //    Which of multiple targets?
    //    Give priority to SART targets, then DSC Distress, then CPA incursion
    palert_target = palert_target_cpa;
    if (palert_target) audioType = AISAUDIO_CPA;

    if (palert_target_sart) {
      palert_target = palert_target_sart;
      audioType = AISAUDIO_SART;
    }

    if (palert_target_dsc) {
      palert_target = palert_target_dsc;
      audioType = AISAUDIO_DSC;
    }
  }
  else {                // Alert is currently shown
    palert_target = Get_Target_Data_From_MMSI(
        g_pais_alert_dialog_active->Get_Dialog_MMSI());
  }
    // Show or update the alert
  if (palert_target)
    info_update.Notify(palert_target, "");

  TimerAIS.Start(TIMER_AIS_MSEC, wxTIMER_CONTINUOUS);
}

std::shared_ptr<AisTargetData> AisDecoder::Get_Target_Data_From_MMSI(int mmsi) {
  if (AISTargetList.find(mmsi) == AISTargetList.end())
    return NULL;
  else
    return AISTargetList[mmsi];
}

ArrayOfMmsiProperties g_MMSI_Props_Array;

//      MmsiProperties Implementation

MmsiProperties::MmsiProperties(wxString &spec) {
  Init();
  wxStringTokenizer tkz(spec, _T(";"));
  wxString s;

  s = tkz.GetNextToken();
  long mmsil;
  s.ToLong(&mmsil);
  MMSI = (int)mmsil;

  s = tkz.GetNextToken();
  if (s.Len()) {
    if (s.Upper() == _T("ALWAYS"))
      TrackType = TRACKTYPE_ALWAYS;
    else if (s.Upper() == _T("NEVER"))
      TrackType = TRACKTYPE_NEVER;
  }

  s = tkz.GetNextToken();
  if (s.Len()) {
    if (s.Upper() == _T("IGNORE")) m_bignore = true;
  }

  s = tkz.GetNextToken();
  if (s.Len()) {
    if (s.Upper() == _T("MOB")) m_bMOB = true;
  }

  s = tkz.GetNextToken();
  if (s.Len()) {
    if (s.Upper() == _T("VDM")) m_bVDM = true;
  }

  s = tkz.GetNextToken();
  if (s.Len()) {
    if (s.Upper() == _T("FOLLOWER")) m_bFollower = true;
  }

  s = tkz.GetNextToken();
  if (s.Len()) {
    if (s.Upper() == _T("PERSIST")) m_bPersistentTrack = true;
  }

  s = tkz.GetNextToken();
  if (s.Len()) {
    m_ShipName = s.Upper();
  }
}

MmsiProperties::~MmsiProperties() {}

void MmsiProperties::Init(void) {
  MMSI = -1;
  TrackType = TRACKTYPE_DEFAULT;
  m_bignore = false;
  m_bMOB = false;
  m_bVDM = false;
  m_bFollower = false;
  m_bPersistentTrack = false;
  m_ShipName = wxEmptyString;
}

wxString MmsiProperties::Serialize(void) {
  wxString sMMSI;
  wxString s;

  sMMSI.Printf(_T("%d"), MMSI);
  sMMSI << _T(";");

  if (TrackType) {
    if (TRACKTYPE_ALWAYS == TrackType)
      sMMSI << _T("always");
    else if (TRACKTYPE_NEVER == TrackType)
      sMMSI << _T("never");
  }
  sMMSI << _T(";");

  if (m_bignore) {
    sMMSI << _T("ignore");
  }
  sMMSI << _T(";");

  if (m_bMOB) {
    sMMSI << _T("MOB");
  }
  sMMSI << _T(";");

  if (m_bVDM) {
    sMMSI << _T("VDM");
  }
  sMMSI << _T(";");

  if (m_bFollower) {
    sMMSI << _T("Follower");
  }
  sMMSI << _T(";");

  if (m_bPersistentTrack) {
    sMMSI << _T("PERSIST");
  }
  sMMSI << _T(";");

  if (m_ShipName == wxEmptyString) {
    m_ShipName = GetShipNameFromFile(MMSI);
  }
  sMMSI << m_ShipName;
  return sMMSI;
}

void AISshipNameCache(AisTargetData *pTargetData,
                      AIS_Target_Name_Hash *AISTargetNamesC,
                      AIS_Target_Name_Hash *AISTargetNamesNC, long mmsi) {
  if (g_benableAISNameCache) {
    wxString ship_name = wxEmptyString;

    // Check for valid name data
    if (!pTargetData->b_nameValid) {
      AIS_Target_Name_Hash::iterator it = AISTargetNamesC->find(mmsi);
      if (it != AISTargetNamesC->end()) {
        ship_name = (*AISTargetNamesC)[mmsi].Left(20);
        strncpy(pTargetData->ShipName, ship_name.mb_str(),
                ship_name.length() + 1);
        pTargetData->b_nameValid = true;
        pTargetData->b_nameFromCache = true;
      } else if (!g_bUseOnlyConfirmedAISName) {
        it = AISTargetNamesNC->find(mmsi);
        if (it != AISTargetNamesNC->end()) {
          ship_name = (*AISTargetNamesNC)[mmsi].Left(20);
          strncpy(pTargetData->ShipName, ship_name.mb_str(),
                  ship_name.length() + 1);
          pTargetData->b_nameValid = true;
          pTargetData->b_nameFromCache = true;
        }
      }
    }
    // else there IS a valid name, lets check if it is in one of the hash lists.
    else if ((pTargetData->MID == 5) || (pTargetData->MID == 24) ||
             (pTargetData->MID == 19) ||
             (pTargetData->MID == 123) ||   // 123: Has got a name from SignalK
             (pTargetData->MID == 124) ) {  // 124: Has got a name from n2k
      //  This message contains ship static data, so has a name field
      pTargetData->b_nameFromCache = false;
      ship_name = trimAISField(pTargetData->ShipName);
      AIS_Target_Name_Hash::iterator itC = AISTargetNamesC->find(mmsi);
      AIS_Target_Name_Hash::iterator itNC = AISTargetNamesNC->find(mmsi);
      if (itC !=
          AISTargetNamesC->end()) {  // There is a confirmed entry for this mmsi
        if ((*AISTargetNamesC)[mmsi] ==
            ship_name) {  // Received name is same as confirmed name
          if (itNC != AISTargetNamesNC->end()) {  // there is also an entry in
                                                  // the NC list, delete it
            AISTargetNamesNC->erase(itNC);
          }
        } else {  // There is a confirmed name but that one is different
          if (itNC != AISTargetNamesNC->end()) {  // there is an entry in the NC
                                                  // list check if name is same
            if ((*AISTargetNamesNC)[mmsi] ==
                ship_name) {  // Same name is already in NC list so promote till
                              // confirmed list
              (*AISTargetNamesC)[mmsi] = ship_name;
              // And delete from NC list
              AISTargetNamesNC->erase(itNC);
            } else {  // A different name is in the NC list, update with
                      // received one
              (*AISTargetNamesNC)[mmsi] = ship_name;
            }
            if (g_bUseOnlyConfirmedAISName)
              strncpy(pTargetData->ShipName, (*AISTargetNamesC)[mmsi].mb_str(),
                      (*AISTargetNamesC)[mmsi].Left(20).Length() + 1);
          } else {  // The C list name is different but no NC entry. Add it.
            (*AISTargetNamesNC)[mmsi] = ship_name;
          }
        }
      } else {  // No confirmed entry available
        if (itNC !=
            AISTargetNamesNC->end()) {  // there is  an entry in the NC list,
          if ((*AISTargetNamesNC)[mmsi] ==
              ship_name) {  // Received name same as already in NC list, promote
                            // to confirmen
            (*AISTargetNamesC)[mmsi] = ship_name;
            // And delete from NC list
            AISTargetNamesNC->erase(itNC);
          } else {  // entry in NC list is not same as received one
            (*AISTargetNamesNC)[mmsi] = ship_name;
          }
        } else {  // No entry in NC list so add it
          (*AISTargetNamesNC)[mmsi] = ship_name;
        }
        if (g_bUseOnlyConfirmedAISName) {  // copy back previous name
          pTargetData->ShipName[SHIP_NAME_LEN - 1] ='\0';
          strncpy(pTargetData->ShipName, "Unknown             ", SHIP_NAME_LEN - 1);
        }
      }
    }
  }
}

wxString GetShipNameFromFile(int nmmsi) {
  wxString name = wxEmptyString;
  if (g_benableAISNameCache) {
    std::ifstream infile(AISTargetNameFileName.mb_str());
    if (infile) {
      std::string line;
      while (getline(infile, line)) {
        wxStringTokenizer tokenizer(wxString::FromUTF8(line.c_str()), _T(","));
        if (nmmsi == wxAtoi(tokenizer.GetNextToken())) {
          name = tokenizer.GetNextToken().Trim();
          break;
        } else
          tokenizer.GetNextToken();
      }
    }
    infile.close();
  }
  return name;
}

int AisMeteoNewMmsi(int m_mmsi, int m_lat, int m_lon, double pt_Lat,
                    double pt_Lon) {
  if (m_lon & 0x01000000)  // negative?
    m_lon |= 0xFE000000;
  double lon_tentative = m_lon / 60000.;

  if (m_lat & 0x00800000)  // negative?
    m_lat |= 0xFF000000;
  double lat_tentative = m_lat / 60000.;

    // Since buoys can move we set position to separate ~50 m
    // to be able to compare previous message.
  wxString sLON = wxString::Format("%0.3f", pt_Lon);
  wxString slon = wxString::Format("%0.3f", lon_tentative);
  wxString sLAT = wxString::Format("%0.3f", pt_Lat);
  wxString slat = wxString::Format("%0.3f", lat_tentative);

  if (sLON.IsSameAs(slon) && sLAT.IsSameAs(slat)) {  // Same position - continue
    return m_mmsi;
  } else {  // Change mmsi number
    // Some countries use one mmsi for all meteo stations.
    // Create our own fake mmsi to separate them.
    // 199 is INMARSAT-A MID, should not occur ever in AIS stream.
    // 1992 to 1993 are already used so here we use 1994+
    static int nextMeteommsi = 199400000;
    bool found = false;
    int new_mmsi = 0;

    auto& points = AisMeteoPoints::GetInstance().GetPoints();

    if (points.size()) {
      wxString t_lat, t_lon;
      for (const auto& point: points) {
        // Does this station position exist
        if (slat.IsSameAs(point.lat) &&
            slon.IsSameAs(point.lon)) {
          // Created before. Continue
          new_mmsi = point.mmsi;
          found = true;
          break;
        }
      }
    }
    if (!found) {
      // Create a new post
      nextMeteommsi++;
      points.push_back(AisMeteoPoint(nextMeteommsi, slat, slon));
      new_mmsi = nextMeteommsi;
    }
    return new_mmsi;
  }
}
