/***************************************************************************
 * $Id: dashboard_pi.cpp, v1.0 2010/08/05 SethDart Exp $
 *
 * Project:  OpenCPN
 * Purpose:  Dashboard Plugin
 * Author:   Jean-Eudes Onfray
 * expanded: Bernd Cirotzki 2023 (special colour design)
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

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // precompiled headers

#include <assert.h>
#include <cmath>
// xw 2.8
#include <wx/filename.h>
#include <wx/fontdlg.h>

#include <typeinfo>
#include "dashboard_pi.h"
#include "icons.h"
#include "wx/jsonreader.h"
#include "wx/jsonwriter.h"
#include "N2KParser.h"


wxFontData* g_pFontTitle;
wxFontData *g_pFontData;
wxFontData *g_pFontLabel;
wxFontData *g_pFontSmall;

wxFontData g_FontTitle;
wxFontData g_FontData;
wxFontData g_FontLabel;
wxFontData g_FontSmall;

wxFontData *g_pUSFontTitle;
wxFontData *g_pUSFontData;
wxFontData *g_pUSFontLabel;
wxFontData *g_pUSFontSmall;

wxFontData g_USFontTitle;
wxFontData g_USFontData;
wxFontData g_USFontLabel;
wxFontData g_USFontSmall;

int g_iDashSpeedMax;
int g_iDashCOGDamp;
int g_iDashSpeedUnit;
int g_iDashSOGDamp;
int g_iDashDepthUnit;
int g_iDashDistanceUnit;
int g_iDashWindSpeedUnit;
int g_iUTCOffset;
double g_dDashDBTOffset;
bool g_bDBtrueWindGround;
double g_dHDT;
double g_dSOG, g_dCOG;
int g_iDashTempUnit;
int g_dashPrefWidth, g_dashPrefHeight;

PI_ColorScheme aktuellColorScheme;
#if !defined(NAN)
static const long long lNaN = 0xfff8000000000000;
#define NAN (*(double *)&lNaN)
#endif

#ifdef __OCPN__ANDROID__
#include "qdebug.h"
#endif


// the class factories, used to create and destroy instances of the PlugIn

extern "C" DECL_EXP opencpn_plugin *create_pi(void *ppimgr) {
  return (opencpn_plugin *)new dashboard_pi(ppimgr);
}

extern "C" DECL_EXP void destroy_pi(opencpn_plugin *p) { delete p; }

#ifdef __OCPN__ANDROID__

QString qtStyleSheet =
    "QScrollBar:horizontal {\
border: 0px solid grey;\
background-color: rgb(240, 240, 240);\
height: 35px;\
margin: 0px 1px 0 1px;\
}\
QScrollBar::handle:horizontal {\
background-color: rgb(200, 200, 200);\
min-width: 20px;\
border-radius: 10px;\
}\
QScrollBar::add-line:horizontal {\
border: 0px solid grey;\
background: #32CC99;\
width: 0px;\
subcontrol-position: right;\
subcontrol-origin: margin;\
}\
QScrollBar::sub-line:horizontal {\
border: 0px solid grey;\
background: #32CC99;\
width: 0px;\
subcontrol-position: left;\
subcontrol-origin: margin;\
}\
QScrollBar:vertical {\
border: 0px solid grey;\
background-color: rgb(240, 240, 240);\
width: 35px;\
margin: 1px 0px 1px 0px;\
}\
QScrollBar::handle:vertical {\
background-color: rgb(200, 200, 200);\
min-height: 20px;\
border-radius: 10px;\
}\
QScrollBar::add-line:vertical {\
border: 0px solid grey;\
background: #32CC99;\
height: 0px;\
subcontrol-position: top;\
subcontrol-origin: margin;\
}\
QScrollBar::sub-line:vertical {\
border: 0px solid grey;\
background: #32CC99;\
height: 0px;\
subcontrol-position: bottom;\
subcontrol-origin: margin;\
}\
QCheckBox {\
spacing: 25px;\
}\
QCheckBox::indicator {\
width: 30px;\
height: 30px;\
}\
";

#endif

#ifdef __OCPN__ANDROID__
#include <QtWidgets/QScroller>
#endif

//---------------------------------------------------------------------------------------------------------
//
//    Dashboard PlugIn Implementation
//
//---------------------------------------------------------------------------------------------------------
// !!! WARNING !!!
// do not change the order, add new instruments at the end, before
// ID_DBP_LAST_ENTRY! otherwise, for users with an existing opencpn.ini file,
// their instruments are changing !
enum {
  ID_DBP_I_POS,
  ID_DBP_I_SOG,
  ID_DBP_D_SOG,
  ID_DBP_I_COG,
  ID_DBP_D_COG,
  ID_DBP_I_STW,
  ID_DBP_I_HDT,
  ID_DBP_D_AW,
  ID_DBP_D_AWA,
  ID_DBP_I_AWS,
  ID_DBP_D_AWS,
  ID_DBP_D_TW,
  ID_DBP_I_DPT,
  ID_DBP_D_DPT,
  ID_DBP_I_TMP,
  ID_DBP_I_VMG,
  ID_DBP_D_VMG,
  ID_DBP_I_RSA,
  ID_DBP_D_RSA,
  ID_DBP_I_SAT,
  ID_DBP_D_GPS,
  ID_DBP_I_PTR,
  ID_DBP_I_GPSUTC,
  ID_DBP_I_SUN,
  ID_DBP_D_MON,
  ID_DBP_I_ATMP,
  ID_DBP_I_AWA,
  ID_DBP_I_TWA,
  ID_DBP_I_TWD,
  ID_DBP_I_TWS,
  ID_DBP_D_TWD,
  ID_DBP_I_HDM,
  ID_DBP_D_HDT,
  ID_DBP_D_WDH,
  ID_DBP_I_VLW1,
  ID_DBP_I_VLW2,
  ID_DBP_D_MDA,
  ID_DBP_I_MDA,
  ID_DBP_D_BPH,
  ID_DBP_I_FOS,
  ID_DBP_M_COG,
  ID_DBP_I_PITCH,
  ID_DBP_I_HEEL,
  ID_DBP_D_AWA_TWA,
  ID_DBP_I_GPSLCL,
  ID_DBP_I_CPULCL,
  ID_DBP_I_SUNLCL,
  ID_DBP_I_ALTI,
  ID_DBP_D_ALTI,
  ID_DBP_I_VMGW,
  ID_DBP_LAST_ENTRY  // this has a reference in one of the routines; defining a
                     // "LAST_ENTRY" and setting the reference to it, is one
                     // codeline less to change (and find) when adding new
                     // instruments :-)
};

bool IsObsolete(int id) {
  switch (id) {
    case ID_DBP_D_AWA:
      return true;
    default:
      return false;
  }
}

wxString getInstrumentCaption(unsigned int id) {
  switch (id) {
    case ID_DBP_I_POS:
      return _("Position");
    case ID_DBP_I_SOG:
      return _("SOG");
    case ID_DBP_D_SOG:
      return _("Speedometer");
    case ID_DBP_I_COG:
      return _("COG");
    case ID_DBP_M_COG:
      return _("Mag COG");
    case ID_DBP_D_COG:
      return _("GNSS Compass");
    case ID_DBP_D_HDT:
      return _("True Compass");
    case ID_DBP_I_STW:
      return _("STW");
    case ID_DBP_I_HDT:
      return _("True HDG");
    case ID_DBP_I_HDM:
      return _("Mag HDG");
    case ID_DBP_D_AW:
    case ID_DBP_D_AWA:
      return _("App. Wind Angle & Speed");
    case ID_DBP_D_AWA_TWA:
      return _("App & True Wind Angle");
    case ID_DBP_I_AWS:
      return _("App. Wind Speed");
    case ID_DBP_D_AWS:
      return _("App. Wind Speed");
    case ID_DBP_D_TW:
      return _("True Wind Angle & Speed");
    case ID_DBP_I_ALTI:
      return _("Altitude");
    case ID_DBP_D_ALTI:
      return _("Altitude Trace");
    case ID_DBP_I_DPT:
      return _("Depth");
    case ID_DBP_D_DPT:
      return _("Depth");
    case ID_DBP_D_MDA:
      return _("Barometric pressure");
    case ID_DBP_I_MDA:
      return _("Barometric pressure");
    case ID_DBP_I_TMP:
      return _("Water Temp.");
    case ID_DBP_I_ATMP:
      return _("Air Temp.");
    case ID_DBP_I_AWA:
      return _("App. Wind Angle");
    case ID_DBP_I_TWA:
      return _("True Wind Angle");
    case ID_DBP_I_TWD:
      return _("True Wind Direction");
    case ID_DBP_I_TWS:
      return _("True Wind Speed");
    case ID_DBP_D_TWD:
      return _("True Wind Dir. & Speed");
    case ID_DBP_I_VMG:
      return _("VMG");
    case ID_DBP_D_VMG:
      return _("VMG");
    case ID_DBP_I_VMGW:
      return _("VMG Wind");
    case ID_DBP_I_RSA:
      return _("Rudder Angle");
    case ID_DBP_D_RSA:
      return _("Rudder Angle");
    case ID_DBP_I_SAT:
      return _("GNSS in use");
    case ID_DBP_D_GPS:
      return _("GNSS Status");
    case ID_DBP_I_PTR:
      return _("Cursor");
    case ID_DBP_I_GPSUTC:
      return _("GNSS Clock");
    case ID_DBP_I_SUN:
      return _("Sunrise/Sunset");
    case ID_DBP_D_MON:
      return _("Moon phase");
    case ID_DBP_D_WDH:
      return _("Wind history");
    case ID_DBP_D_BPH:
      return _("Barometric history");
    case ID_DBP_I_VLW1:
      return _("Trip Log");
    case ID_DBP_I_VLW2:
      return _("Sum Log");
    case ID_DBP_I_FOS:
      return _("From Ownship");
    case ID_DBP_I_PITCH:
      return _("Pitch");
    case ID_DBP_I_HEEL:
      return _("Heel");
    case ID_DBP_I_GPSLCL:
      return _("Local GNSS Clock");
    case ID_DBP_I_CPULCL:
      return _("Local CPU Clock");
    case ID_DBP_I_SUNLCL:
      return _("Local Sunrise/Sunset");
  }
  return _T("");
}

void getListItemForInstrument(wxListItem &item, unsigned int id) {
  item.SetData(id);
  item.SetText(getInstrumentCaption(id));
  switch (id) {
    case ID_DBP_I_POS:
    case ID_DBP_I_SOG:
    case ID_DBP_I_COG:
    case ID_DBP_M_COG:
    case ID_DBP_I_STW:
    case ID_DBP_I_HDT:
    case ID_DBP_I_HDM:
    case ID_DBP_I_AWS:
    case ID_DBP_I_DPT:
    case ID_DBP_I_MDA:
    case ID_DBP_I_TMP:
    case ID_DBP_I_ATMP:
    case ID_DBP_I_TWA:
    case ID_DBP_I_TWD:
    case ID_DBP_I_TWS:
    case ID_DBP_I_AWA:
    case ID_DBP_I_VMG:
    case ID_DBP_I_VMGW:
    case ID_DBP_I_RSA:
    case ID_DBP_I_SAT:
    case ID_DBP_I_PTR:
    case ID_DBP_I_GPSUTC:
    case ID_DBP_I_GPSLCL:
    case ID_DBP_I_CPULCL:
    case ID_DBP_I_SUN:
    case ID_DBP_I_SUNLCL:
    case ID_DBP_I_VLW1:
    case ID_DBP_I_VLW2:
    case ID_DBP_I_FOS:
    case ID_DBP_I_PITCH:
    case ID_DBP_I_HEEL:
    case ID_DBP_I_ALTI:
      item.SetImage(0);
      break;
    case ID_DBP_D_SOG:
    case ID_DBP_D_COG:
    case ID_DBP_D_AW:
    case ID_DBP_D_AWA:
    case ID_DBP_D_AWS:
    case ID_DBP_D_TW:
    case ID_DBP_D_AWA_TWA:
    case ID_DBP_D_TWD:
    case ID_DBP_D_DPT:
    case ID_DBP_D_MDA:
    case ID_DBP_D_VMG:
    case ID_DBP_D_RSA:
    case ID_DBP_D_GPS:
    case ID_DBP_D_HDT:
    case ID_DBP_D_MON:
    case ID_DBP_D_WDH:
    case ID_DBP_D_BPH:
    case ID_DBP_D_ALTI:
      item.SetImage(1);
      break;
  }
}

/*  These two function were taken from gpxdocument.cpp */
int GetRandomNumber(int range_min, int range_max) {
  long u = (long)wxRound(
      ((double)rand() / ((double)(RAND_MAX) + 1) * (range_max - range_min)) +
      range_min);
  return (int)u;
}

// RFC4122 version 4 compliant random UUIDs generator.
wxString GetUUID(void) {
  wxString str;
  struct {
    int time_low;
    int time_mid;
    int time_hi_and_version;
    int clock_seq_hi_and_rsv;
    int clock_seq_low;
    int node_hi;
    int node_low;
  } uuid;

  uuid.time_low = GetRandomNumber(
      0, 2147483647);  // FIXME: the max should be set to something like
                       // MAXINT32, but it doesn't compile un gcc...
  uuid.time_mid = GetRandomNumber(0, 65535);
  uuid.time_hi_and_version = GetRandomNumber(0, 65535);
  uuid.clock_seq_hi_and_rsv = GetRandomNumber(0, 255);
  uuid.clock_seq_low = GetRandomNumber(0, 255);
  uuid.node_hi = GetRandomNumber(0, 65535);
  uuid.node_low = GetRandomNumber(0, 2147483647);

  /* Set the two most significant bits (bits 6 and 7) of the
   * clock_seq_hi_and_rsv to zero and one, respectively. */
  uuid.clock_seq_hi_and_rsv = (uuid.clock_seq_hi_and_rsv & 0x3F) | 0x80;

  /* Set the four most significant bits (bits 12 through 15) of the
   * time_hi_and_version field to 4 */
  uuid.time_hi_and_version = (uuid.time_hi_and_version & 0x0fff) | 0x4000;

  str.Printf(_T("%08x-%04x-%04x-%02x%02x-%04x%08x"), uuid.time_low,
             uuid.time_mid, uuid.time_hi_and_version, uuid.clock_seq_hi_and_rsv,
             uuid.clock_seq_low, uuid.node_hi, uuid.node_low);

  return str;
}

wxString MakeName() { return _T("DASH_") + GetUUID(); }

//---------------------------------------------------------------------------------------------------------
//
//          PlugIn initialization and de-init
//
//---------------------------------------------------------------------------------------------------------

dashboard_pi::dashboard_pi(void *ppimgr)
    : wxTimer(this), opencpn_plugin_18(ppimgr) {
  // Create the PlugIn icons
  initialize_images();
  mCOGFilter.setType(IIRFILTER_TYPE_DEG);
}

dashboard_pi::~dashboard_pi(void) {
  delete _img_dashboard_pi;
  delete _img_dashboard;
  delete _img_dial;
  delete _img_instrument;
  delete _img_minus;
  delete _img_plus;
}

int dashboard_pi::Init(void) {
  AddLocaleCatalog(_T("opencpn-dashboard_pi"));

  mVar = NAN;
  mPriPosition = 99;
  mPriCOGSOG = 99;
  mPriHeadingT = 99;  // True heading
  mPriHeadingM = 99;  // Magnetic heading
  mPriVar = 99;
  mPriDateTime = 99;
  mPriAWA = 99;  // Relative wind
  mPriTWA = 99;  // True wind
  mPriWDN = 99;  // True hist. wind
  mPriDepth = 99;
  mPriSTW = 99;
  mPriWTP = 99;  // Water temp
  mPriATMP = 99; // Air temp
  mPriSatStatus = 99;
  mPriSatUsed = 99;
  mSatsInView = 0;
  mPriAlt = 99;
  mPriRSA = 99;  //Rudder angle
  mPriPitchRoll = 99; //Pitch and roll
  m_config_version = -1;
  mHDx_Watchdog = 2;
  mHDT_Watchdog = 2;
  mSatsUsed_Wdog = 2;
  mSatStatus_Wdog = 2;
  mVar_Watchdog = 2;
  mMWVA_Watchdog = 2;
  mMWVT_Watchdog = 2;
  mDPT_DBT_Watchdog = 2; // Depth
  mSTW_Watchdog = 2;
  mWTP_Watchdog = 2;
  mRSA_Watchdog = 2;
  mVMG_Watchdog = 2;
  mVMGW_Watchdog = 2;
  mUTC_Watchdog = 2;
  mATMP_Watchdog = 2;
  mWDN_Watchdog = 2;
  mMDA_Watchdog = 2;
  mPITCH_Watchdog = 2;
  mHEEL_Watchdog = 2;
  mALT_Watchdog = 2;
  mLOG_Watchdog = 2;
  mTrLOG_Watchdog = 2;

  g_pFontTitle = new wxFontData();
  g_pFontTitle->SetChosenFont(wxFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_NORMAL));
  
  g_pFontData = new wxFontData();
  g_pFontData->SetChosenFont(wxFont(14, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

  g_pFontLabel = new wxFontData();
  g_pFontLabel->SetChosenFont(wxFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

  g_pFontSmall = new wxFontData();
  g_pFontSmall->SetChosenFont(wxFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

  g_pUSFontTitle = &g_USFontTitle;
  g_pUSFontData = &g_USFontData;
  g_pUSFontLabel = &g_USFontLabel;
  g_pUSFontSmall = &g_USFontSmall;

  m_pauimgr = GetFrameAuiManager();
  m_pauimgr->Connect(wxEVT_AUI_PANE_CLOSE,
                     wxAuiManagerEventHandler(dashboard_pi::OnPaneClose), NULL,
                     this);

  //    Get a pointer to the opencpn configuration object
  m_pconfig = GetOCPNConfigObject();

  //    And load the configuration items
  LoadConfig();

  //    This PlugIn needs a toolbar icon
  //    m_toolbar_item_id = InsertPlugInTool( _T(""), _img_dashboard,
  //    _img_dashboard, wxITEM_CHECK,
  //            _("Dashboard"), _T(""), NULL, DASHBOARD_TOOL_POSITION, 0, this
  //            );

  wxString shareLocn = *GetpSharedDataLocation() + _T("plugins") +
                       wxFileName::GetPathSeparator() + _T("dashboard_pi") +
                       wxFileName::GetPathSeparator() + _T("data") +
                       wxFileName::GetPathSeparator();

  wxString normalIcon = shareLocn + _T("Dashboard.svg");
  wxString toggledIcon = shareLocn + _T("Dashboard_toggled.svg");
  wxString rolloverIcon = shareLocn + _T("Dashboard_rollover.svg");

  //  For journeyman styles, we prefer the built-in raster icons which match the
  //  rest of the toolbar.
  if (GetActiveStyleName().Lower() != _T("traditional")) {
    normalIcon = _T("");
    toggledIcon = _T("");
    rolloverIcon = _T("");
  }

  m_toolbar_item_id = InsertPlugInToolSVG(
      _T(""), normalIcon, rolloverIcon, toggledIcon, wxITEM_CHECK,
      _("Dashboard"), _T(""), NULL, DASHBOARD_TOOL_POSITION, 0, this);

  ApplyConfig();

  //  If we loaded a version 1 config setup, convert now to version 2
  if (m_config_version == 1) {
    SaveConfig();
  }

  // initialize NavMsg listeners
  //-----------------------------

  // Rudder data PGN 127245
  wxDEFINE_EVENT(EVT_N2K_127245, ObservedEvt);
  NMEA2000Id id_127245 = NMEA2000Id(127245);
  listener_127245 = GetListener(id_127245, EVT_N2K_127245, this);
  Bind(EVT_N2K_127245, [&](ObservedEvt ev) {
    HandleN2K_127245(ev);
  });


  // Roll Pitch   PGN 127257
  wxDEFINE_EVENT(EVT_N2K_127257, ObservedEvt);
  NMEA2000Id id_127257 = NMEA2000Id(127257);
  listener_127257 = GetListener(id_127257, EVT_N2K_127257, this);
  Bind(EVT_N2K_127257, [&](ObservedEvt ev) {
    HandleN2K_127257(ev);
  });

  // Speed through water PGN 128259
  wxDEFINE_EVENT(EVT_N2K_128259, ObservedEvt);
  NMEA2000Id id_128259 = NMEA2000Id(128259);
  listener_128259 = GetListener(id_128259, EVT_N2K_128259, this);
  Bind(EVT_N2K_128259, [&](ObservedEvt ev) {
    HandleN2K_128259(ev);
  });

  // Depth Data   PGN 128267
  wxDEFINE_EVENT(EVT_N2K_128267, ObservedEvt);
  NMEA2000Id id_128267 = NMEA2000Id(128267);
  listener_128267 = GetListener(id_128267, EVT_N2K_128267, this);
  Bind(EVT_N2K_128267, [&](ObservedEvt ev) {
    HandleN2K_128267(ev);
  });

  // Distance log
  wxDEFINE_EVENT(EVT_N2K_128275, ObservedEvt);
  NMEA2000Id id_128275 = NMEA2000Id(128275);
  listener_128275 = GetListener(id_128275, EVT_N2K_128275, this);
  Bind(EVT_N2K_128275, [&](ObservedEvt ev) {
    HandleN2K_128275(ev);
  });

  // GNSS Position Data   PGN 129029
  wxDEFINE_EVENT(EVT_N2K_129029, ObservedEvt);
  NMEA2000Id id_129029 = NMEA2000Id(129029);
  listener_129029 = GetListener(id_129029, EVT_N2K_129029, this);
  Bind(EVT_N2K_129029, [&](ObservedEvt ev) {
    HandleN2K_129029(ev);
  });

  // GNSS Satellites in View   PGN 129540
  wxDEFINE_EVENT(EVT_N2K_129540, ObservedEvt);
  NMEA2000Id id_129540 = NMEA2000Id(129540);
  listener_129540 = GetListener(id_129540, EVT_N2K_129540, this);
  Bind(EVT_N2K_129540, [&](ObservedEvt ev) {
    HandleN2K_129540(ev);
  });

  // Wind   PGN 130306
  wxDEFINE_EVENT(EVT_N2K_130306, ObservedEvt);
  NMEA2000Id id_130306 = NMEA2000Id(130306);
  listener_130306 = GetListener(id_130306, EVT_N2K_130306, this);
  Bind(EVT_N2K_130306, [&](ObservedEvt ev) {
    HandleN2K_130306(ev);
  });

  // Envorinment   PGN 130310
  wxDEFINE_EVENT(EVT_N2K_130310, ObservedEvt);
  NMEA2000Id id_130310 = NMEA2000Id(130310);
  listener_130310 = GetListener(id_130310, EVT_N2K_130310, this);
  Bind(EVT_N2K_130310, [&](ObservedEvt ev) {
    HandleN2K_130310(ev);
  });

  Start(1000, wxTIMER_CONTINUOUS);

  return (WANTS_CURSOR_LATLON | WANTS_TOOLBAR_CALLBACK | INSTALLS_TOOLBAR_TOOL |
          WANTS_PREFERENCES | WANTS_CONFIG | WANTS_NMEA_SENTENCES |
          WANTS_NMEA_EVENTS | USES_AUI_MANAGER | WANTS_PLUGIN_MESSAGING);
}

bool dashboard_pi::DeInit(void) {
  SaveConfig();
  if (IsRunning())  // Timer started?
    Stop();         // Stop timer

  for (size_t i = 0; i < m_ArrayOfDashboardWindow.GetCount(); i++) {
    DashboardWindow *dashboard_window =
        m_ArrayOfDashboardWindow.Item(i)->m_pDashboardWindow;
    if (dashboard_window) {
      m_pauimgr->DetachPane(dashboard_window);
      dashboard_window->Close();
      dashboard_window->Destroy();
      m_ArrayOfDashboardWindow.Item(i)->m_pDashboardWindow = NULL;
    }
  }

  for (size_t i = 0; i < m_ArrayOfDashboardWindow.GetCount(); i++) {
    DashboardWindowContainer *pdwc = m_ArrayOfDashboardWindow.Item(i);
    delete pdwc;
  }

//   delete g_pFontTitle;
//   delete g_pFontData;
//   delete g_pFontLabel;
//   delete g_pFontSmall;

  return true;
}

double GetJsonDouble(wxJSONValue &value) {
  double d_ret = 0;
  if (value.IsDouble()) {
    d_ret = value.AsDouble();
    return d_ret;
  } else if (value.IsLong()) {
    int i_ret = value.AsLong();
    d_ret = i_ret;
    return d_ret;
  }
  return nan("");
}



void dashboard_pi::Notify() {
  SendUtcTimeToAllInstruments(mUTCDateTime);
  for (size_t i = 0; i < m_ArrayOfDashboardWindow.GetCount(); i++) {
    DashboardWindow *dashboard_window =
        m_ArrayOfDashboardWindow.Item(i)->m_pDashboardWindow;
    if (dashboard_window) {
      dashboard_window->Refresh();
#ifdef __OCPN__ANDROID__
      wxWindowList list = dashboard_window->GetChildren();
      wxWindowListNode *node = list.GetFirst();
      for (size_t i = 0; i < list.GetCount(); i++) {
        wxWindow *win = node->GetData();
        //                qDebug() << "Refresh Dash child:" << i;
        win->Refresh();
        node = node->GetNext();
      }
#endif
    }
  }
  //  Manage the watchdogs

  mHDx_Watchdog--;
  if (mHDx_Watchdog <= 0) {
    mHdm = NAN;
    mPriHeadingM = 99;
    SendSentenceToAllInstruments(OCPN_DBP_STC_HDM, mHdm, _T("\u00B0"));
    mHDx_Watchdog = gps_watchdog_timeout_ticks;
  }

  mHDT_Watchdog--;
  if (mHDT_Watchdog <= 0) {
    mPriHeadingT = 99;
    SendSentenceToAllInstruments(OCPN_DBP_STC_HDT, NAN, _T("\u00B0T"));
    mHDT_Watchdog = gps_watchdog_timeout_ticks;
  }

  mVar_Watchdog--;
  if (mVar_Watchdog <= 0) {
    mVar = NAN;
    mPriVar = 99;
    SendSentenceToAllInstruments(OCPN_DBP_STC_HMV, NAN, _T("\u00B0T"));
    mVar_Watchdog = gps_watchdog_timeout_ticks;
  }

  mSatsUsed_Wdog--;
  if (mSatsUsed_Wdog <= 0) {
    mPriSatUsed = 99;
    mSatsInUse = 0;
    SendSentenceToAllInstruments(OCPN_DBP_STC_SAT, NAN, _T(""));
    mSatsUsed_Wdog = gps_watchdog_timeout_ticks;
  }

  mSatStatus_Wdog--;
  if (mSatStatus_Wdog <= 0) {
    SAT_INFO sats[4];
    for (int i = 0; i < 4; i++) {
      sats[i].SatNumber = 0;
      sats[i].SignalToNoiseRatio = 0;
    }
    SendSatInfoToAllInstruments(0, 1, wxEmptyString, sats);
    SendSatInfoToAllInstruments(0, 2, wxEmptyString, sats);
    SendSatInfoToAllInstruments(0, 3, wxEmptyString, sats);
    mPriSatStatus = 99;
    mSatStatus_Wdog = gps_watchdog_timeout_ticks;
  }
  // Set Satellite Status data from the same source as OCPN use for position
  // Get the identifiers
  std::vector<std::string> PriorityIDs = GetActivePriorityIdentifiers();
  // Get current satellite priority identifier = item 4
  std::string satID = PriorityIDs[4];
  if (satID.find("nmea0183") != std::string::npos)
    mPriSatStatus = 3; // GSV
  else if (satID.find("SignalK") != std::string::npos)
    mPriSatStatus = 2; // SignalK
  else if (satID.find("nmea2000") != std::string::npos) {
    prioN2kPGNsat = satID;
    mPriSatStatus = 1; // N2k
  }

  mMWVA_Watchdog--;
  if (mMWVA_Watchdog <= 0) {
    SendSentenceToAllInstruments(OCPN_DBP_STC_AWA, NAN, _T("-"));
    SendSentenceToAllInstruments(OCPN_DBP_STC_AWS, NAN, _T("-"));
    mPriAWA = 99;
    mMWVA_Watchdog = gps_watchdog_timeout_ticks;
  }

  mMWVT_Watchdog--;
  if (mMWVT_Watchdog <= 0) {
    SendSentenceToAllInstruments(OCPN_DBP_STC_TWA, NAN, _T("-"));
    SendSentenceToAllInstruments(OCPN_DBP_STC_TWS, NAN, _T("-"));
    SendSentenceToAllInstruments(OCPN_DBP_STC_TWS2, NAN, _T("-"));
    mPriTWA = 99;
    mMWVT_Watchdog = gps_watchdog_timeout_ticks;
  }

  mDPT_DBT_Watchdog--;
  if (mDPT_DBT_Watchdog <= 0) {
    mPriDepth = 99;
    SendSentenceToAllInstruments(OCPN_DBP_STC_DPT, NAN, _T("-"));
    mDPT_DBT_Watchdog = gps_watchdog_timeout_ticks;
  }

  mSTW_Watchdog--;
  if (mSTW_Watchdog <= 0) {
    mPriSTW = 99;
    SendSentenceToAllInstruments(OCPN_DBP_STC_STW, NAN, _T("-"));
    mSTW_Watchdog = gps_watchdog_timeout_ticks;
  }

  mWTP_Watchdog--;
  if (mWTP_Watchdog <= 0) {
    mPriWTP = 99;
    SendSentenceToAllInstruments(OCPN_DBP_STC_TMP, NAN, "-");
    mWTP_Watchdog = no_nav_watchdog_timeout_ticks;
  }
  mRSA_Watchdog--;
  if (mRSA_Watchdog <= 0) {
    mPriRSA = 99;
    SendSentenceToAllInstruments(OCPN_DBP_STC_RSA, NAN, "-");
    mRSA_Watchdog = gps_watchdog_timeout_ticks;
  }
  mVMG_Watchdog--;
  if (mVMG_Watchdog <= 0) {
    SendSentenceToAllInstruments(OCPN_DBP_STC_VMG, NAN, "-");
    mVMG_Watchdog = gps_watchdog_timeout_ticks;
  }
  mVMGW_Watchdog--;
  if (mVMGW_Watchdog <= 0) {
    SendSentenceToAllInstruments(OCPN_DBP_STC_VMGW, NAN, "-");
    mVMGW_Watchdog = gps_watchdog_timeout_ticks;
  }
  mUTC_Watchdog--;
  if (mUTC_Watchdog <= 0) {
    mPriDateTime = 99;
    mUTC_Watchdog = gps_watchdog_timeout_ticks;
  }
  mATMP_Watchdog--;
  if (mATMP_Watchdog <= 0) {
    SendSentenceToAllInstruments(OCPN_DBP_STC_ATMP, NAN, "-");
    mPriATMP = 99;
    mATMP_Watchdog = gps_watchdog_timeout_ticks;
  }
  mWDN_Watchdog--;
  if (mWDN_Watchdog <= 0) {
    SendSentenceToAllInstruments(OCPN_DBP_STC_TWD, NAN, _T("-"));
    mPriWDN = 99;
    mWDN_Watchdog = no_nav_watchdog_timeout_ticks;
  }
  mMDA_Watchdog--;
  if (mMDA_Watchdog <= 0) {
    SendSentenceToAllInstruments(OCPN_DBP_STC_MDA, NAN, _T("-"));
    mMDA_Watchdog = gps_watchdog_timeout_ticks;
  }
  mPITCH_Watchdog--;
  if (mPITCH_Watchdog <= 0) {
    mPriPitchRoll = 99;
    SendSentenceToAllInstruments(OCPN_DBP_STC_PITCH, NAN, _T("-"));
    mPITCH_Watchdog = gps_watchdog_timeout_ticks;
  }
  mHEEL_Watchdog--;
  if (mHEEL_Watchdog <= 0) {
    mPriPitchRoll = 99;
    SendSentenceToAllInstruments(OCPN_DBP_STC_HEEL, NAN, _T("-"));
    mHEEL_Watchdog = gps_watchdog_timeout_ticks;
  }
  mALT_Watchdog--;
  if (mALT_Watchdog <= 0) {
    mPriAlt = 99;
    SendSentenceToAllInstruments(OCPN_DBP_STC_ALTI, NAN, _T("-"));
    mALT_Watchdog = gps_watchdog_timeout_ticks;
  }

  mLOG_Watchdog--;
  if (mLOG_Watchdog <= 0) {
    SendSentenceToAllInstruments(OCPN_DBP_STC_VLW2, NAN, _T("-"));
    mLOG_Watchdog = no_nav_watchdog_timeout_ticks;
  }
  mTrLOG_Watchdog--;
  if (mTrLOG_Watchdog <= 0) {
    SendSentenceToAllInstruments(OCPN_DBP_STC_VLW1, NAN, _T("-"));
    mTrLOG_Watchdog = no_nav_watchdog_timeout_ticks;
  }
}

int dashboard_pi::GetAPIVersionMajor() { return MY_API_VERSION_MAJOR; }

int dashboard_pi::GetAPIVersionMinor() { return MY_API_VERSION_MINOR; }

int dashboard_pi::GetPlugInVersionMajor() { return PLUGIN_VERSION_MAJOR; }

int dashboard_pi::GetPlugInVersionMinor() { return PLUGIN_VERSION_MINOR; }

wxBitmap *dashboard_pi::GetPlugInBitmap() { return _img_dashboard_pi; }

wxString dashboard_pi::GetCommonName() { return _("Dashboard"); }

wxString dashboard_pi::GetShortDescription() {
  return _("Dashboard PlugIn for OpenCPN");
}

wxString dashboard_pi::GetLongDescription() {
  return _(
      "Dashboard PlugIn for OpenCPN\n\
Provides navigation instrument display from NMEA source.");
}

void dashboard_pi::SendSentenceToAllInstruments(DASH_CAP st, double value,
                                                wxString unit) {
  for (size_t i = 0; i < m_ArrayOfDashboardWindow.GetCount(); i++) {
    DashboardWindow *dashboard_window =
        m_ArrayOfDashboardWindow.Item(i)->m_pDashboardWindow;
    if (dashboard_window)
      dashboard_window->SendSentenceToAllInstruments(st, value, unit);
  }
  if (st == OCPN_DBP_STC_HDT) {
    g_dHDT = value;
  }
  if (st == OCPN_DBP_STC_SOG) {
    g_dSOG = value;
  }
  if (st == OCPN_DBP_STC_COG) {
    g_dCOG = value;
  }
}

void dashboard_pi::SendUtcTimeToAllInstruments(wxDateTime value) {
  for (size_t i = 0; i < m_ArrayOfDashboardWindow.GetCount(); i++) {
    DashboardWindow *dashboard_window =
        m_ArrayOfDashboardWindow.Item(i)->m_pDashboardWindow;
    if (dashboard_window) dashboard_window->SendUtcTimeToAllInstruments(value);
  }
}

void dashboard_pi::SendSatInfoToAllInstruments(int cnt, int seq, wxString talk,
                                               SAT_INFO sats[4]) {
  for (size_t i = 0; i < m_ArrayOfDashboardWindow.GetCount(); i++) {
    DashboardWindow *dashboard_window =
        m_ArrayOfDashboardWindow.Item(i)->m_pDashboardWindow;
    if (dashboard_window)
      dashboard_window->SendSatInfoToAllInstruments(cnt, seq, talk, sats);
  }
}

// NMEA 0183 N0183.....
void dashboard_pi::SetNMEASentence(wxString &sentence) {
  m_NMEA0183 << sentence;

  if (m_NMEA0183.PreParse()) {
    if (m_NMEA0183.LastSentenceIDReceived == _T("DBT")) {        
      if (mPriDepth >= 5) {
        if (m_NMEA0183.Parse()) {
          /*
           double m_NMEA0183.Dbt.DepthFeet;
           double m_NMEA0183.Dbt.DepthMeters;
           double m_NMEA0183.Dbt.DepthFathoms;
           */
          double depth = NAN;
          if (!std::isnan(m_NMEA0183.Dbt.DepthMeters))
            depth = m_NMEA0183.Dbt.DepthMeters;
          else if (!std::isnan(m_NMEA0183.Dbt.DepthFeet))
            depth = m_NMEA0183.Dbt.DepthFeet * 0.3048;
          else if (!std::isnan(m_NMEA0183.Dbt.DepthFathoms))
            depth = m_NMEA0183.Dbt.DepthFathoms * 1.82880;
          if (!std::isnan(depth)) depth += g_dDashDBTOffset;
          if (!std::isnan(depth)) {
            SendSentenceToAllInstruments(
                OCPN_DBP_STC_DPT,
                toUsrDistance_Plugin(depth / 1852.0, g_iDashDepthUnit),
                getUsrDistanceUnit_Plugin(g_iDashDepthUnit));
            mPriDepth = 5;
            mDPT_DBT_Watchdog = gps_watchdog_timeout_ticks;
          }
        }        
      }
    }

    else if (m_NMEA0183.LastSentenceIDReceived == _T("DPT")) {
      if (mPriDepth >= 4) {
        if (m_NMEA0183.Parse()) {
          /*
           double m_NMEA0183.Dpt.DepthMeters
           double m_NMEA0183.Dpt.OffsetFromTransducerMeters
           */
          double depth = m_NMEA0183.Dpt.DepthMeters;
          if (!std::isnan(m_NMEA0183.Dpt.OffsetFromTransducerMeters)) {
            depth += m_NMEA0183.Dpt.OffsetFromTransducerMeters;
          }
          depth += g_dDashDBTOffset;
          if (!std::isnan(depth)) {
            SendSentenceToAllInstruments(
                OCPN_DBP_STC_DPT,
                toUsrDistance_Plugin(depth / 1852.0, g_iDashDepthUnit),
                getUsrDistanceUnit_Plugin(g_iDashDepthUnit));
            mPriDepth = 4;
            mDPT_DBT_Watchdog = gps_watchdog_timeout_ticks;
          }
        }
      }
    }
    // TODO: GBS - GPS Satellite fault detection
    else if (m_NMEA0183.LastSentenceIDReceived == _T("GGA")) {
      if (0)  // debug output
        printf("GGA mPriPosition=%d mPriSatUsed=%d \tnSat=%d alt=%3.2f\n",
                mPriPosition, mPriSatUsed,
                m_NMEA0183.Gga.NumberOfSatellitesInUse,
                m_NMEA0183.Gga.AntennaAltitudeMeters);
      if (mPriAlt >= 3 && (mPriPosition >= 1 || mPriSatUsed >= 1)) {
        if (m_NMEA0183.Parse()) {
          if (m_NMEA0183.Gga.GPSQuality > 0 &&
              m_NMEA0183.Gga.NumberOfSatellitesInUse >= 5) {
            // Altimeter, takes altitude from gps GGA message, which is
            // typically less accurate than lon and lat.
            double alt = m_NMEA0183.Gga.AntennaAltitudeMeters;
            SendSentenceToAllInstruments(OCPN_DBP_STC_ALTI, alt, _T("m"));
            mPriAlt = 3;
            mALT_Watchdog = gps_watchdog_timeout_ticks;
          }
        }
      }
      if (mPriPosition >= 4 || mPriSatUsed >= 3) {
        if (m_NMEA0183.Parse()) {
          if (m_NMEA0183.Gga.GPSQuality > 0) {
            if (mPriPosition >= 4) {
              mPriPosition = 4;
              double lat, lon;
              float llt = m_NMEA0183.Gga.Position.Latitude.Latitude;
              int lat_deg_int = (int)(llt / 100);
              float lat_deg = lat_deg_int;
              float lat_min = llt - (lat_deg * 100);
              lat = lat_deg + (lat_min / 60.);
              if (m_NMEA0183.Gga.Position.Latitude.Northing == South)
                lat = -lat;
              SendSentenceToAllInstruments(OCPN_DBP_STC_LAT, lat, _T("SDMM"));

              float lln = m_NMEA0183.Gga.Position.Longitude.Longitude;
              int lon_deg_int = (int)(lln / 100);
              float lon_deg = lon_deg_int;
              float lon_min = lln - (lon_deg * 100);
              lon = lon_deg + (lon_min / 60.);
              if (m_NMEA0183.Gga.Position.Longitude.Easting == West) lon = -lon;
              SendSentenceToAllInstruments(OCPN_DBP_STC_LON, lon, _T("SDMM"));
            }
            if (mPriSatUsed >= 3) {
              mSatsInUse = m_NMEA0183.Gga.NumberOfSatellitesInUse;
              SendSentenceToAllInstruments( OCPN_DBP_STC_SAT, mSatsInUse, _T (""));
              mPriSatUsed = 3;
              mSatsUsed_Wdog = gps_watchdog_timeout_ticks;
            }

            // if( mPriDateTime >= 4 ) {
            //    // Not in use, we need the date too.
            //    //mPriDateTime = 4;
            //    //mUTCDateTime.ParseFormat( m_NMEA0183.Gga.UTCTime.c_str(),
            //    _T("%H%M%S") );
            //}
          }
        }
      }
    }

    else if (m_NMEA0183.LastSentenceIDReceived == _T("GLL")) {
      if (mPriPosition >= 3) {
        if (m_NMEA0183.Parse()) {
          if (m_NMEA0183.Gll.IsDataValid == NTrue) {
            double lat, lon;
            float llt = m_NMEA0183.Gll.Position.Latitude.Latitude;
            int lat_deg_int = (int)(llt / 100);
            float lat_deg = lat_deg_int;
            float lat_min = llt - (lat_deg * 100);
            lat = lat_deg + (lat_min / 60.);
            if (m_NMEA0183.Gll.Position.Latitude.Northing == South) lat = -lat;
            SendSentenceToAllInstruments(OCPN_DBP_STC_LAT, lat, _T("SDMM"));

            float lln = m_NMEA0183.Gll.Position.Longitude.Longitude;
            int lon_deg_int = (int)(lln / 100);
            float lon_deg = lon_deg_int;
            float lon_min = lln - (lon_deg * 100);
            lon = lon_deg + (lon_min / 60.);
            if (m_NMEA0183.Gll.Position.Longitude.Easting == West) lon = -lon;
            SendSentenceToAllInstruments(OCPN_DBP_STC_LON, lon, _T("SDMM"));

            mPriPosition = 3;
          }

          // if( mPriDateTime >= 5 ) {
          //    // Not in use, we need the date too.
          //    //mPriDateTime = 5;
          //    //mUTCDateTime.ParseFormat( m_NMEA0183.Gll.UTCTime.c_str(),
          //    _T("%H%M%S") );
          //}
        }
      }
    }

    else if (m_NMEA0183.LastSentenceIDReceived == _T("GSV")) {
      if (mPriSatStatus >= 3 || mPriSatUsed >= 5) {
        if (m_NMEA0183.Parse()) {
          if (m_NMEA0183.Gsv.MessageNumber == 1) {
            // NMEA0183 recommend to not repeat SatsInView
            // in subsequent messages
            mSatsInView = m_NMEA0183.Gsv.SatsInView;

            if (mPriSatUsed >= 5) {
              SendSentenceToAllInstruments(OCPN_DBP_STC_SAT,
                                           m_NMEA0183.Gsv.SatsInView, _T (""));
              mPriSatUsed = 5;
              mSatsUsed_Wdog = gps_watchdog_timeout_ticks;
            }
          }

          if (mPriSatStatus >= 3) {
            SendSatInfoToAllInstruments(
                mSatsInView, m_NMEA0183.Gsv.MessageNumber,
                m_NMEA0183.TalkerID, m_NMEA0183.Gsv.SatInfo);
            mPriSatStatus = 3;
            mSatStatus_Wdog = gps_watchdog_timeout_ticks;
          }
        }
      }
    }

    else if (m_NMEA0183.LastSentenceIDReceived == _T("HDG")) {
      if (mPriVar >= 3 || mPriHeadingM >= 3 || mPriHeadingT >= 7) {
        if (m_NMEA0183.Parse()) {
          if (mPriVar >= 3) {
            // Any device sending VAR=0.0 can be assumed to not really know
            // what the actual variation is, so in this case we use WMM if
            // available
            if ((!std::isnan(m_NMEA0183.Hdg.MagneticVariationDegrees)) &&
                0.0 != m_NMEA0183.Hdg.MagneticVariationDegrees) {
              mPriVar = 3;
              if (m_NMEA0183.Hdg.MagneticVariationDirection == East)
                mVar = m_NMEA0183.Hdg.MagneticVariationDegrees;
              else if (m_NMEA0183.Hdg.MagneticVariationDirection == West)
                mVar = -m_NMEA0183.Hdg.MagneticVariationDegrees;
              SendSentenceToAllInstruments(OCPN_DBP_STC_HMV, mVar,
                                           _T("\u00B0"));
            }
          }
          if (mPriHeadingM >= 3) {
            if (!std::isnan(m_NMEA0183.Hdg.MagneticSensorHeadingDegrees)) {
              mPriHeadingM = 3;
              mHdm = m_NMEA0183.Hdg.MagneticSensorHeadingDegrees;
              SendSentenceToAllInstruments(OCPN_DBP_STC_HDM, mHdm,
                                           _T("\u00B0"));
            }
          }
          if (!std::isnan(m_NMEA0183.Hdg.MagneticSensorHeadingDegrees))
            mHDx_Watchdog = gps_watchdog_timeout_ticks;

          //      If Variation is available, no higher priority HDT is
          //      available, then calculate and propagate calculated HDT
          if (!std::isnan(m_NMEA0183.Hdg.MagneticSensorHeadingDegrees)) {
            if (!std::isnan(mVar) && (mPriHeadingT >= 7)) {
              mPriHeadingT = 7;
              double heading = mHdm + mVar;
              if (heading < 0)
                heading += 360;
              else if (heading >= 360.0)
                heading -= 360;
              SendSentenceToAllInstruments(OCPN_DBP_STC_HDT, heading,
                                           _T("\u00B0"));
              mHDT_Watchdog = gps_watchdog_timeout_ticks;
            }
          }
        }
      }
    }

    else if (m_NMEA0183.LastSentenceIDReceived == _T("HDM")) {
      if (mPriHeadingM >= 4 || mPriHeadingT >= 5) {
        if (m_NMEA0183.Parse()) {
          if (mPriHeadingM >= 4) {
            if (!std::isnan(m_NMEA0183.Hdm.DegreesMagnetic)) {
              mPriHeadingM = 4;
              mHdm = m_NMEA0183.Hdm.DegreesMagnetic;
              SendSentenceToAllInstruments(OCPN_DBP_STC_HDM, mHdm,
                                           _T("\u00B0M"));
              mHDx_Watchdog = gps_watchdog_timeout_ticks;
            }
          }

          //      If Variation is available, no higher priority HDT is
          //      available, then calculate and propagate calculated HDT
          if (!std::isnan(m_NMEA0183.Hdm.DegreesMagnetic)) {
            if (!std::isnan(mVar) && (mPriHeadingT >= 5)) {
              mPriHeadingT = 5;
              double heading = mHdm + mVar;
              if (heading < 0)
                heading += 360;
              else if (heading >= 360.0)
                heading -= 360;
              SendSentenceToAllInstruments(OCPN_DBP_STC_HDT, heading,
                                           _T("\u00B0"));
              mHDT_Watchdog = gps_watchdog_timeout_ticks;
            }
          }
        }
      }
    }

    else if (m_NMEA0183.LastSentenceIDReceived == _T("HDT")) {
      if (mPriHeadingT >= 3) {
        if (m_NMEA0183.Parse()) {
          if (!std::isnan(m_NMEA0183.Hdt.DegreesTrue)) {
            SendSentenceToAllInstruments(
                OCPN_DBP_STC_HDT, m_NMEA0183.Hdt.DegreesTrue, _T("\u00B0T"));
            mPriHeadingT = 3;
            mHDT_Watchdog = gps_watchdog_timeout_ticks;
          }
        }
      }
    } else if (m_NMEA0183.LastSentenceIDReceived ==
               _T("MTA")) {  // Air temperature
      if (mPriATMP >= 3) {
        if (m_NMEA0183.Parse()) {
          mPriATMP = 3;
          SendSentenceToAllInstruments(
              OCPN_DBP_STC_ATMP,
              toUsrTemp_Plugin(m_NMEA0183.Mta.Temperature, g_iDashTempUnit),
              getUsrTempUnit_Plugin(g_iDashTempUnit));
          mATMP_Watchdog = gps_watchdog_timeout_ticks;
        }
      }
    } else if (m_NMEA0183.LastSentenceIDReceived ==
               _T("MDA")) {  // Barometric pressure
      if (m_NMEA0183.Parse()) {
        // TODO make posibilyti to select between Bar or InchHg
        /*
         double   m_NMEA0183.Mda.Pressure;
         wxString m_NMEA0183.Mda.UnitOfMeasurement;
         */
        if (m_NMEA0183.Mda.Pressure > .8 && m_NMEA0183.Mda.Pressure < 1.1) {
          SendSentenceToAllInstruments(
              OCPN_DBP_STC_MDA, m_NMEA0183.Mda.Pressure * 1000,
              _T("hPa"));  // Convert to hpa befor sending to instruments.
          mMDA_Watchdog = no_nav_watchdog_timeout_ticks;
        }
        if (mPriATMP >= 5) {
          double airtemp = m_NMEA0183.Mda.AirTemp;
          if (airtemp < 999.0) {
            SendSentenceToAllInstruments(
              OCPN_DBP_STC_ATMP,
              toUsrTemp_Plugin(airtemp, g_iDashTempUnit),
              getUsrTempUnit_Plugin(g_iDashTempUnit));
            mATMP_Watchdog = no_nav_watchdog_timeout_ticks;
            mPriATMP = 5;
          }
        }
      }

    } else if (m_NMEA0183.LastSentenceIDReceived == _T("MTW")) {
      if (mPriWTP >= 4) {
        if (m_NMEA0183.Parse()) {
          mPriWTP = 4;
          SendSentenceToAllInstruments(
              OCPN_DBP_STC_TMP,
              toUsrTemp_Plugin(m_NMEA0183.Mtw.Temperature, g_iDashTempUnit),
              getUsrTempUnit_Plugin(g_iDashTempUnit));
          mWTP_Watchdog = no_nav_watchdog_timeout_ticks;
        }
      }

    } else if (m_NMEA0183.LastSentenceIDReceived == _T("VLW")) {
      if (m_NMEA0183.Parse()) {
        /*
         double   m_NMEA0183.Vlw.TotalMileage;
         double   m_NMEA0183.Vlw.TripMileage;
                          */
        SendSentenceToAllInstruments(
            OCPN_DBP_STC_VLW1,
            toUsrDistance_Plugin(m_NMEA0183.Vlw.TripMileage,
                                 g_iDashDistanceUnit),
            getUsrDistanceUnit_Plugin(g_iDashDistanceUnit));
        mTrLOG_Watchdog = no_nav_watchdog_timeout_ticks;

        SendSentenceToAllInstruments(
            OCPN_DBP_STC_VLW2,
            toUsrDistance_Plugin(m_NMEA0183.Vlw.TotalMileage,
                                 g_iDashDistanceUnit),
            getUsrDistanceUnit_Plugin(g_iDashDistanceUnit));
        mLOG_Watchdog = no_nav_watchdog_timeout_ticks;
      }

    }
    // NMEA 0183 standard Wind Direction and Speed, with respect to north.
    else if (m_NMEA0183.LastSentenceIDReceived == _T("MWD")) {
      if (mPriWDN >= 6) {
        if (m_NMEA0183.Parse()) {
          // Option for True vs Magnetic
          wxString windunit;
          mPriWDN = 6;
          if (!std::isnan(
                  m_NMEA0183.Mwd.WindAngleTrue)) {  // if WindAngleTrue is
                                                    // available, use it ...
            SendSentenceToAllInstruments(
                OCPN_DBP_STC_TWD, m_NMEA0183.Mwd.WindAngleTrue, _T("\u00B0"));
            // MWD can be seldom updated by the sensor. Set prolonged watchdog
            mWDN_Watchdog = no_nav_watchdog_timeout_ticks;
          } else if (!std::isnan(
                         m_NMEA0183.Mwd
                             .WindAngleMagnetic)) {  // otherwise try
                                                     // WindAngleMagnetic ...
            SendSentenceToAllInstruments(OCPN_DBP_STC_TWD,
                                         m_NMEA0183.Mwd.WindAngleMagnetic,
                                         _T("\u00B0M"));
            mWDN_Watchdog = no_nav_watchdog_timeout_ticks;
          }

          SendSentenceToAllInstruments(
              OCPN_DBP_STC_TWS,
              toUsrSpeed_Plugin(m_NMEA0183.Mwd.WindSpeedKnots,
                                g_iDashWindSpeedUnit),
              getUsrSpeedUnit_Plugin(g_iDashWindSpeedUnit));
          SendSentenceToAllInstruments(
              OCPN_DBP_STC_TWS2,
              toUsrSpeed_Plugin(m_NMEA0183.Mwd.WindSpeedKnots,
                                g_iDashWindSpeedUnit),
              getUsrSpeedUnit_Plugin(g_iDashWindSpeedUnit));
          mMWVT_Watchdog = gps_watchdog_timeout_ticks;
          // m_NMEA0183.Mwd.WindSpeedms
        }
      }
    }
    // NMEA 0183 standard Wind Speed and Angle, in relation to the vessel's
    // bow/centerline.
    else if (m_NMEA0183.LastSentenceIDReceived == _T("MWV")) {
      if (mPriAWA >= 4 || mPriTWA >= 5 || mPriWDN >= 5) {
        if (m_NMEA0183.Parse()) {
          if (m_NMEA0183.Mwv.IsDataValid == NTrue) {
            // MWV windspeed has different units. Form it to knots to fit
            // "toUsrSpeed_Plugin()"
            double m_wSpeedFactor = 1.0;  // knots ("N")
            if (m_NMEA0183.Mwv.WindSpeedUnits == _T("K"))
              m_wSpeedFactor = 0.53995;  // km/h > knots
            if (m_NMEA0183.Mwv.WindSpeedUnits == _T("M"))
              m_wSpeedFactor = 1.94384;  // m/s > knots

            if (m_NMEA0183.Mwv.Reference ==
                _T("R"))  // Relative (apparent wind)
            {
              if (mPriAWA >= 4) {
                mPriAWA = 4;
                wxString m_awaunit;
                double m_awaangle;
                if (m_NMEA0183.Mwv.WindAngle > 180) {
                  m_awaunit = _T("\u00B0L");
                  m_awaangle = 180.0 - (m_NMEA0183.Mwv.WindAngle - 180.0);
                } else {
                  m_awaunit = _T("\u00B0R");
                  m_awaangle = m_NMEA0183.Mwv.WindAngle;
                }
                SendSentenceToAllInstruments(OCPN_DBP_STC_AWA, m_awaangle,
                                             m_awaunit);
                SendSentenceToAllInstruments(
                    OCPN_DBP_STC_AWS,
                    toUsrSpeed_Plugin(m_NMEA0183.Mwv.WindSpeed * m_wSpeedFactor,
                                      g_iDashWindSpeedUnit),
                    getUsrSpeedUnit_Plugin(g_iDashWindSpeedUnit));
                mMWVA_Watchdog = gps_watchdog_timeout_ticks;
              }

              // If we have true HDT, COG, and SOG
              // then using simple vector math, we can calculate true wind
              // direction and speed. If there is no higher priority source for
              // WDN, then do so here, and update the appropriate instruments.
              if (mPriWDN >= 8) {
                CalculateAndUpdateTWDS(
                    m_NMEA0183.Mwv.WindSpeed * m_wSpeedFactor,
                    m_NMEA0183.Mwv.WindAngle);
                mPriWDN = 8;
                mWDN_Watchdog = no_nav_watchdog_timeout_ticks;
                mMWVT_Watchdog = gps_watchdog_timeout_ticks;
              }
            } else if (m_NMEA0183.Mwv.Reference ==
                       _T("T"))  // Theoretical (aka True)
            {
              if (mPriTWA >= 4) {
                mPriTWA = 4;
                wxString m_twaunit;
                double m_twaangle;
                bool b_R = false;
                if (m_NMEA0183.Mwv.WindAngle > 180) {
                  m_twaunit = _T("\u00B0L");
                  m_twaangle = 180.0 - (m_NMEA0183.Mwv.WindAngle - 180.0);
                } else {
                  m_twaunit = _T("\u00B0R");
                  m_twaangle = m_NMEA0183.Mwv.WindAngle;
                  b_R = true;
                }
                SendSentenceToAllInstruments(OCPN_DBP_STC_TWA, m_twaangle,
                                             m_twaunit);

                if (mPriWDN >= 7) {
                  // MWV has wind angle relative to the bow.
                  // Wind history use angle relative to north.
                  // If no TWD with higher priority is present
                  // and true heading is available calculate it.
                  if (g_dHDT < 361. && g_dHDT >= 0.0) {
                    double g_dCalWdir = (m_NMEA0183.Mwv.WindAngle) + g_dHDT;
                    if (g_dCalWdir > 360.) {
                      g_dCalWdir -= 360;
                    } else if (g_dCalWdir < 0.) {
                      g_dCalWdir += 360;
                    }
                    SendSentenceToAllInstruments(OCPN_DBP_STC_TWD, g_dCalWdir,
                                                 _T("\u00B0"));
                    mPriWDN = 7;
                    mWDN_Watchdog = no_nav_watchdog_timeout_ticks;
                  }
                }

                SendSentenceToAllInstruments(
                    OCPN_DBP_STC_TWS,
                    toUsrSpeed_Plugin(m_NMEA0183.Mwv.WindSpeed * m_wSpeedFactor,
                                      g_iDashWindSpeedUnit),
                    getUsrSpeedUnit_Plugin(g_iDashWindSpeedUnit));
                SendSentenceToAllInstruments(
                    OCPN_DBP_STC_TWS2,
                    toUsrSpeed_Plugin(m_NMEA0183.Mwv.WindSpeed * m_wSpeedFactor,
                                      g_iDashWindSpeedUnit),
                    getUsrSpeedUnit_Plugin(g_iDashWindSpeedUnit));
                mMWVT_Watchdog = gps_watchdog_timeout_ticks;
              }
            }
          }
        }
      }
    }

    else if (m_NMEA0183.LastSentenceIDReceived == _T("RMC")) {
      if (mPriPosition >= 5 || mPriCOGSOG >= 3 || mPriVar >= 4 ||
          mPriDateTime >= 3) {
        if (m_NMEA0183.Parse()) {
          if (m_NMEA0183.Rmc.IsDataValid == NTrue) {
            if (mPriPosition >= 5) {
              mPriPosition = 5;
              double lat, lon;
              float llt = m_NMEA0183.Rmc.Position.Latitude.Latitude;
              int lat_deg_int = (int)(llt / 100);
              float lat_deg = lat_deg_int;
              float lat_min = llt - (lat_deg * 100);
              lat = lat_deg + (lat_min / 60.);
              if (m_NMEA0183.Rmc.Position.Latitude.Northing == South)
                lat = -lat;
              SendSentenceToAllInstruments(OCPN_DBP_STC_LAT, lat, _T("SDMM"));

              float lln = m_NMEA0183.Rmc.Position.Longitude.Longitude;
              int lon_deg_int = (int)(lln / 100);
              float lon_deg = lon_deg_int;
              float lon_min = lln - (lon_deg * 100);
              lon = lon_deg + (lon_min / 60.);
              if (m_NMEA0183.Rmc.Position.Longitude.Easting == West) lon = -lon;
              SendSentenceToAllInstruments(OCPN_DBP_STC_LON, lon, _T("SDMM"));
            }

            if (mPriCOGSOG >= 3) {
              mPriCOGSOG = 3;
              if (!std::isnan(m_NMEA0183.Rmc.SpeedOverGroundKnots)) {
                SendSentenceToAllInstruments(
                    OCPN_DBP_STC_SOG,
                    toUsrSpeed_Plugin(
                        mSOGFilter.filter(m_NMEA0183.Rmc.SpeedOverGroundKnots),
                        g_iDashSpeedUnit),
                    getUsrSpeedUnit_Plugin(g_iDashSpeedUnit));
              }

              if (!std::isnan(m_NMEA0183.Rmc.TrackMadeGoodDegreesTrue)) {
                SendSentenceToAllInstruments(
                    OCPN_DBP_STC_COG,
                    mCOGFilter.filter(m_NMEA0183.Rmc.TrackMadeGoodDegreesTrue),
                    _T("\u00B0"));
              }
              if (!std::isnan(m_NMEA0183.Rmc.TrackMadeGoodDegreesTrue) &&
                  !std::isnan(m_NMEA0183.Rmc.MagneticVariation)) {
                double dMagneticCOG;
                if (m_NMEA0183.Rmc.MagneticVariationDirection == East) {
                  dMagneticCOG =
                      mCOGFilter.get() - m_NMEA0183.Rmc.MagneticVariation;
                  if (dMagneticCOG < 0.0) dMagneticCOG = 360.0 + dMagneticCOG;
                } else {
                  dMagneticCOG =
                      mCOGFilter.get() + m_NMEA0183.Rmc.MagneticVariation;
                  if (dMagneticCOG > 360.0) dMagneticCOG = dMagneticCOG - 360.0;
                }
                SendSentenceToAllInstruments(OCPN_DBP_STC_MCOG, dMagneticCOG,
                                             _T("\u00B0M"));
              }
            }

            if (mPriVar >= 4) {
              // Any device sending VAR=0.0 can be assumed to not really know
              // what the actual variation is, so in this case we use WMM if
              // available
              if ((!std::isnan(m_NMEA0183.Rmc.MagneticVariation)) &&
                  0.0 != m_NMEA0183.Rmc.MagneticVariation) {
                mPriVar = 4;
                if (m_NMEA0183.Rmc.MagneticVariationDirection == East)
                  mVar = m_NMEA0183.Rmc.MagneticVariation;
                else if (m_NMEA0183.Rmc.MagneticVariationDirection == West)
                  mVar = -m_NMEA0183.Rmc.MagneticVariation;
                mVar_Watchdog = gps_watchdog_timeout_ticks;

                SendSentenceToAllInstruments(OCPN_DBP_STC_HMV, mVar,
                                             _T("\u00B0"));
              }
            }

            if (mPriDateTime >= 3) {
              mPriDateTime = 3;
              wxString dt = m_NMEA0183.Rmc.Date + m_NMEA0183.Rmc.UTCTime;
              mUTCDateTime.ParseFormat(dt.c_str(), _T("%d%m%y%H%M%S"));
              mUTC_Watchdog = gps_watchdog_timeout_ticks;
            }
          }
        }
      }
    }

    else if (m_NMEA0183.LastSentenceIDReceived == _T("RSA")) {
      if (mPriRSA >= 3) {
        if (m_NMEA0183.Parse()) {
          if (m_NMEA0183.Rsa.IsStarboardDataValid == NTrue) {
            SendSentenceToAllInstruments(OCPN_DBP_STC_RSA,
                                         m_NMEA0183.Rsa.Starboard, _T("\u00B0"));
          }
          else if (m_NMEA0183.Rsa.IsPortDataValid == NTrue) {
            SendSentenceToAllInstruments(OCPN_DBP_STC_RSA, -m_NMEA0183.Rsa.Port,
                                         _T("\u00B0"));
          }
          mRSA_Watchdog = gps_watchdog_timeout_ticks;
          mPriRSA = 3;
        }
      }
    }

    else if (m_NMEA0183.LastSentenceIDReceived == _T("VHW")) {
      if (mPriHeadingT >= 4 || mPriHeadingM >= 5 || mPriSTW >= 3) {
        if (m_NMEA0183.Parse()) {
          if (mPriHeadingT >= 4) {
            if (!std::isnan(m_NMEA0183.Vhw.DegreesTrue)) {
              mPriHeadingT = 4;
              SendSentenceToAllInstruments(
                  OCPN_DBP_STC_HDT, m_NMEA0183.Vhw.DegreesTrue, _T("\u00B0T"));
              mHDT_Watchdog = gps_watchdog_timeout_ticks;
            }
          }
          if (mPriHeadingM >= 5) {
            if (!std::isnan(m_NMEA0183.Vhw.DegreesMagnetic)) {
              mPriHeadingM = 5;
              SendSentenceToAllInstruments(OCPN_DBP_STC_HDM,
                                           m_NMEA0183.Vhw.DegreesMagnetic,
                                           _T("\u00B0M"));
              mHDx_Watchdog = gps_watchdog_timeout_ticks;
            }
          }
          if (!std::isnan(m_NMEA0183.Vhw.Knots)) {
            if (mPriSTW >= 3) {
              mPriSTW = 3;
              SendSentenceToAllInstruments(
                  OCPN_DBP_STC_STW,
                  toUsrSpeed_Plugin(m_NMEA0183.Vhw.Knots, g_iDashSpeedUnit),
                  getUsrSpeedUnit_Plugin(g_iDashSpeedUnit));
              mSTW_Watchdog = gps_watchdog_timeout_ticks;
            }
          }
        }
      }
    }

    else if (m_NMEA0183.LastSentenceIDReceived == _T("VTG")) {
      if (mPriCOGSOG >= 2) {
        if (m_NMEA0183.Parse()) {
          mPriCOGSOG = 2;
          //    Special check for unintialized values, as opposed to zero values
          if (!std::isnan(m_NMEA0183.Vtg.SpeedKnots)) {
            SendSentenceToAllInstruments(
                OCPN_DBP_STC_SOG,
                toUsrSpeed_Plugin(mSOGFilter.filter(m_NMEA0183.Vtg.SpeedKnots),
                                  g_iDashSpeedUnit),
                getUsrSpeedUnit_Plugin(g_iDashSpeedUnit));
          }
          // Vtg.SpeedKilometersPerHour;
          if (!std::isnan(m_NMEA0183.Vtg.TrackDegreesTrue)) {
            SendSentenceToAllInstruments(
                OCPN_DBP_STC_COG,
                mCOGFilter.filter(m_NMEA0183.Vtg.TrackDegreesTrue),
                _T("\u00B0"));
          }
        }

        /*
         m_NMEA0183.Vtg.TrackDegreesMagnetic;
         */
      }
    }
    /* NMEA 0183 Relative (Apparent) Wind Speed and Angle. Wind angle in
     * relation
     * to the vessel's heading, and wind speed measured relative to the moving
     * vessel. */
    else if (m_NMEA0183.LastSentenceIDReceived == _T("VWR")) {
      if (mPriAWA >= 3) {
        if (m_NMEA0183.Parse()) {
          if (m_NMEA0183.Vwr.WindDirectionMagnitude < 200) {
            mPriAWA = 3;

            wxString awaunit;
            awaunit = m_NMEA0183.Vwr.DirectionOfWind == Left ? _T("\u00B0L")
                                                             : _T("\u00B0R");
            SendSentenceToAllInstruments(OCPN_DBP_STC_AWA,
                                         m_NMEA0183.Vwr.WindDirectionMagnitude,
                                         awaunit);
            SendSentenceToAllInstruments(
                OCPN_DBP_STC_AWS,
                toUsrSpeed_Plugin(m_NMEA0183.Vwr.WindSpeedKnots,
                                  g_iDashWindSpeedUnit),
                getUsrSpeedUnit_Plugin(g_iDashWindSpeedUnit));
            mMWVA_Watchdog = gps_watchdog_timeout_ticks;
            /*
                double m_NMEA0183.Vwr.WindSpeedms;
                double m_NMEA0183.Vwr.WindSpeedKmh;
                */
          }

          // If we have true HDT, COG, and SOG
          // then using simple vector math, we can calculate true wind direction
          // and speed. If there is no higher priority source for WDN, then do
          // so here, and update the appropriate instruments.
          if (mPriWDN >= 9) {
            double awa = m_NMEA0183.Vwr.WindDirectionMagnitude;
            if (m_NMEA0183.Vwr.DirectionOfWind == Left)
              awa = 360. - m_NMEA0183.Vwr.WindDirectionMagnitude;
            CalculateAndUpdateTWDS(m_NMEA0183.Vwr.WindSpeedKnots, awa);
            mPriWDN = 9;
            mMWVT_Watchdog = gps_watchdog_timeout_ticks;
            mWDN_Watchdog = no_nav_watchdog_timeout_ticks;
          }
        }
      }
    }
    /* NMEA 0183 True wind angle in relation to the vessel's heading, and true
     * wind speed referenced to the water. True wind is the vector sum of the
     * Relative (apparent) wind vector and the vessel's velocity vector relative
     * to the water along the heading line of the vessel. It represents the wind
     * at the vessel if it were
     * stationary relative to the water and heading in the same direction. */
    else if (m_NMEA0183.LastSentenceIDReceived == _T("VWT")) {
      if (mPriTWA >= 4) {
        if (m_NMEA0183.Parse()) {
          if (m_NMEA0183.Vwt.WindDirectionMagnitude < 200) {
            mPriTWA = 4;
            wxString vwtunit;
            vwtunit = m_NMEA0183.Vwt.DirectionOfWind == Left ? _T("\u00B0L")
                                                             : _T("\u00B0R");
            SendSentenceToAllInstruments(OCPN_DBP_STC_TWA,
                                         m_NMEA0183.Vwt.WindDirectionMagnitude,
                                         vwtunit);
            SendSentenceToAllInstruments(
                OCPN_DBP_STC_TWS,
                toUsrSpeed_Plugin(m_NMEA0183.Vwt.WindSpeedKnots,
                                  g_iDashWindSpeedUnit),
                getUsrSpeedUnit_Plugin(g_iDashWindSpeedUnit));
            mMWVT_Watchdog = gps_watchdog_timeout_ticks;
            /*
             double           m_NMEA0183.Vwt.WindSpeedms;
             double           m_NMEA0183.Vwt.WindSpeedKmh;
             */
          }
        }
      }
    }

    else if (m_NMEA0183.LastSentenceIDReceived ==
             _T("XDR")) {  // Transducer measurement
      /* XDR Transducer types
       * AngularDisplacementTransducer = 'A',
       * TemperatureTransducer = 'C',
       * LinearDisplacementTransducer = 'D',
       * FrequencyTransducer = 'F',
       * HumidityTransducer = 'H',
       * ForceTransducer = 'N',
       * PressureTransducer = 'P',
       * FlowRateTransducer = 'R',
       * TachometerTransducer = 'T',
       * VolumeTransducer = 'V'
       */

      if (m_NMEA0183.Parse()) {
        wxString xdrunit;
        double xdrdata;
        for (int i = 0; i < m_NMEA0183.Xdr.TransducerCnt; i++) {
          xdrdata = m_NMEA0183.Xdr.TransducerInfo[i].MeasurementData;
          // XDR Airtemp
          if (m_NMEA0183.Xdr.TransducerInfo[i].TransducerType == _T("C") && (
              m_NMEA0183.Xdr.TransducerInfo[i].TransducerName ==
                   _T("Te") ||
              m_NMEA0183.Xdr.TransducerInfo[i].TransducerName ==
                   _T("TempAir") ||
              m_NMEA0183.Xdr.TransducerInfo[i].TransducerName ==
                  _T("AIRTEMP") ||
              m_NMEA0183.Xdr.TransducerInfo[i].TransducerName ==
                  _T("ENV_OUTAIR_T") ||
              m_NMEA0183.Xdr.TransducerInfo[i].TransducerName ==
                  _T("ENV_OUTSIDE_T"))) {
            if (mPriATMP >= 4) {
              mPriATMP = 4;
              SendSentenceToAllInstruments(
                  OCPN_DBP_STC_ATMP, toUsrTemp_Plugin(xdrdata, g_iDashTempUnit),
                  getUsrTempUnit_Plugin(g_iDashTempUnit));
              mATMP_Watchdog = no_nav_watchdog_timeout_ticks;
            }
          }
          // XDR Pressure
          if (m_NMEA0183.Xdr.TransducerInfo[i].TransducerType == _T("P")) {
            if (m_NMEA0183.Xdr.TransducerInfo[i].UnitOfMeasurement == _T("B")) {
              xdrdata *= 1000;
              SendSentenceToAllInstruments(OCPN_DBP_STC_MDA, xdrdata,
                                           _T("hPa"));
              mMDA_Watchdog = no_nav_watchdog_timeout_ticks;
            }
          }
          // XDR Pitch (=Nose up/down) or Heel (stb/port)
          if (m_NMEA0183.Xdr.TransducerInfo[i].TransducerType == _T("A")) {
            if (m_NMEA0183.Xdr.TransducerInfo[i].TransducerName.
                Contains(_T("PTCH")) ||
                m_NMEA0183.Xdr.TransducerInfo[i].TransducerName.
                Contains(_T("PITCH"))) {
              if (mPriPitchRoll >= 3) {
                if (m_NMEA0183.Xdr.TransducerInfo[i].MeasurementData > 0) {
                  xdrunit = _T("\u00B0\u2191") + _("Up");
                }
                else if (m_NMEA0183.Xdr.TransducerInfo[i].MeasurementData < 0) {
                  xdrunit = _T("\u00B0\u2193") + _("Down");
                  xdrdata *= -1;
                }
                else {
                  xdrunit = _T("\u00B0");
                }
                SendSentenceToAllInstruments(OCPN_DBP_STC_PITCH, xdrdata,
                                             xdrunit);
                mPITCH_Watchdog = gps_watchdog_timeout_ticks;
                mPriPitchRoll = 3;
              }
            }
            // XDR Heel
            else if (m_NMEA0183.Xdr.TransducerInfo[i].TransducerName.
                     Contains(_T("ROLL"))) {
              if (mPriPitchRoll >= 3) {
                if (m_NMEA0183.Xdr.TransducerInfo[i].MeasurementData > 0) {
                  xdrunit = _T("\u00B0\u003E") + _("Stbd");
                }
                else if (m_NMEA0183.Xdr.TransducerInfo[i].MeasurementData < 0) {
                  xdrunit = _T("\u00B0\u003C") + _("Port");
                  xdrdata *= -1;
                }
                else {
                  xdrunit = _T("\u00B0");
                }
                SendSentenceToAllInstruments(OCPN_DBP_STC_HEEL, xdrdata, xdrunit);
                mHEEL_Watchdog = gps_watchdog_timeout_ticks;
                mPriPitchRoll = 3;
              }
            }
            // XDR Rudder Angle
            else if (m_NMEA0183.Xdr.TransducerInfo[i].TransducerName ==
                     _T("RUDDER")) {
              if (mPriRSA > 4) {
                SendSentenceToAllInstruments(OCPN_DBP_STC_RSA, xdrdata,
                                             _T("\u00B0"));
                mRSA_Watchdog = gps_watchdog_timeout_ticks;
                mPriRSA = 4;
              }
            }
          }
          // Nasa style water temp
          if (m_NMEA0183.Xdr.TransducerInfo[i].TransducerName ==
              _T("ENV_WATER_T")) {
            if (mPriWTP >= 3) {
              mPriWTP = 3;
              SendSentenceToAllInstruments(
                  OCPN_DBP_STC_TMP,
                  toUsrTemp_Plugin(
                      m_NMEA0183.Xdr.TransducerInfo[i].MeasurementData,
                      g_iDashTempUnit),
                  getUsrTempUnit_Plugin(g_iDashTempUnit));
              mWTP_Watchdog = no_nav_watchdog_timeout_ticks;
            }
          }
        }
      }
    } else if (m_NMEA0183.LastSentenceIDReceived == _T("ZDA")) {
      if (mPriDateTime >= 2) {
        if (m_NMEA0183.Parse()) {
          mPriDateTime = 2;
          /*
           wxString m_NMEA0183.Zda.UTCTime;
           int      m_NMEA0183.Zda.Day;
           int      m_NMEA0183.Zda.Month;
           int      m_NMEA0183.Zda.Year;
           int      m_NMEA0183.Zda.LocalHourDeviation;
           int      m_NMEA0183.Zda.LocalMinutesDeviation;
           */
          wxString dt;
          dt.Printf(_T("%4d%02d%02d"), m_NMEA0183.Zda.Year,
                    m_NMEA0183.Zda.Month, m_NMEA0183.Zda.Day);
          dt.Append(m_NMEA0183.Zda.UTCTime);
          mUTCDateTime.ParseFormat(dt.c_str(), _T("%Y%m%d%H%M%S"));
          mUTC_Watchdog = gps_watchdog_timeout_ticks;
        }
      }
    }
  }
  //      Process an AIVDO message
  else if (sentence.Mid(1, 5).IsSameAs(_T("AIVDO"))) {
    PlugIn_Position_Fix_Ex gpd;
    if (DecodeSingleVDOMessage(sentence, &gpd, &m_VDO_accumulator)) {
      if (!std::isnan(gpd.Lat))
        SendSentenceToAllInstruments(OCPN_DBP_STC_LAT, gpd.Lat, _T("SDMM"));

      if (!std::isnan(gpd.Lon))
        SendSentenceToAllInstruments(OCPN_DBP_STC_LON, gpd.Lon, _T("SDMM"));

      SendSentenceToAllInstruments(
          OCPN_DBP_STC_SOG,
          toUsrSpeed_Plugin(mSOGFilter.filter(gpd.Sog), g_iDashSpeedUnit),
          getUsrSpeedUnit_Plugin(g_iDashSpeedUnit));
      SendSentenceToAllInstruments(OCPN_DBP_STC_COG, mCOGFilter.filter(gpd.Cog),
                                   _T("\u00B0"));
      if (!std::isnan(gpd.Hdt)) {
        SendSentenceToAllInstruments(OCPN_DBP_STC_HDT, gpd.Hdt, _T("\u00B0T"));
        mHDT_Watchdog = gps_watchdog_timeout_ticks;
      }
    }
  }
}

/*      Calculate True Wind speed and direction from AWS and AWA
 *      This algorithm requires HDT, SOG, and COG, which are maintained as
 * globals elsewhere. Also, update all instruments tagged with OCPN_DBP_STC_TWD,
 * OCPN_DBP_STC_TWS, and OCPN_DBP_STC_TWS2
 */
void dashboard_pi::CalculateAndUpdateTWDS(double awsKnots, double awaDegrees) {
  if (!std::isnan(g_dHDT)) {
    // Apparent wind velocity vector, relative to head-up
    double awsx = awsKnots * cos(awaDegrees * PI / 180.);
    double awsy = awsKnots * sin(awaDegrees * PI / 180.);

    // Ownship velocity vector, relative to head-up
    double bsx = 0;
    double bsy = 0;
    if ((!std::isnan(g_dSOG)) && (!std::isnan(g_dCOG))) {
      bsx = g_dSOG * cos((g_dCOG - g_dHDT) * PI / 180.);
      bsy = g_dSOG * sin((g_dCOG - g_dHDT) * PI / 180.);
      ;
    }

    // "True" wind is calculated by vector subtraction
    double twdx = awsx - bsx;
    double twdy = awsy - bsy;

    // Calculate the speed (magnitude of the vector)
    double tws = pow(((twdx * twdx) + (twdy * twdy)), 0.5);

    // calculate the True Wind Angle
    double twd = atan2(twdy, twdx) * 180. / PI;
    if (twd < 0)
      SendSentenceToAllInstruments(OCPN_DBP_STC_TWA, -twd, _T("\u00B0L"));
    else
      SendSentenceToAllInstruments(OCPN_DBP_STC_TWA, twd, _T("\u00B0R"));

    // Calculate the True Wind Direction, by re-orienting to the ownship HDT
    double twdc = twd + g_dHDT;

    // Normalize
    if (twdc < 0) twdc += 360.;
    if (twdc > 360.) twdc -= 360;

    // Update the instruments
    // printf("CALC: %4.0f %4.0f\n", tws, twdc);
    SendSentenceToAllInstruments(OCPN_DBP_STC_TWD, twdc, _T("\u00B0"));

    SendSentenceToAllInstruments(OCPN_DBP_STC_TWS,
                                 toUsrSpeed_Plugin(tws, g_iDashWindSpeedUnit),
                                 getUsrSpeedUnit_Plugin(g_iDashWindSpeedUnit));
    SendSentenceToAllInstruments(OCPN_DBP_STC_TWS2,
                                 toUsrSpeed_Plugin(tws, g_iDashWindSpeedUnit),
                                 getUsrSpeedUnit_Plugin(g_iDashWindSpeedUnit));
  }
}

// NMEA2000, N2K
//...............

// Rudder data PGN 127245
void dashboard_pi::HandleN2K_127245(ObservedEvt ev) {
  NMEA2000Id id_127245(127245);
  std::vector<uint8_t>v = GetN2000Payload(id_127245, ev);

  // Get a uniqe ID to prioritize source(s)
  unsigned char source_id = v.at(7);
  char ss[4];
  sprintf(ss, "%d", source_id);
  std::string ident = std::string(ss);
  std::string source = GetN2000Source(id_127245, ev);
  source += ":" + ident;

  if (mPriRSA >= 1) {
    if (mPriRSA == 1) {
      // We favor first received after last WD
      if (source != prio127245) return;
    }
    else {
      // First time use after WD time out.
      prio127245 = source;
    }

    double RudderPosition, AngleOrder;
    unsigned char Instance;
    tN2kRudderDirectionOrder RudderDirectionOrder;

      // Get rudder position
    if (ParseN2kPGN127245(v, RudderPosition, Instance, RudderDirectionOrder, AngleOrder)) {
      if (!N2kIsNA(RudderPosition)) {
        double m_rudangle = GEODESIC_RAD2DEG(RudderPosition);
        SendSentenceToAllInstruments(OCPN_DBP_STC_RSA, m_rudangle, _T("\u00B0"));
        mRSA_Watchdog = gps_watchdog_timeout_ticks;
        mPriRSA = 1;
      }
    }
  }
}

// Roll Pitch data PGN 127257
void dashboard_pi::HandleN2K_127257(ObservedEvt ev) {
  NMEA2000Id id_127257(127257);
  std::vector<uint8_t>v = GetN2000Payload(id_127257, ev);

  // Get a uniqe ID to prioritize source(s)
  unsigned char source_id = v.at(7);
  char ss[4];
  sprintf(ss, "%d", source_id);
  std::string ident = std::string(ss);
  std::string source = GetN2000Source(id_127257, ev);
  source += ":" + ident;

  if (mPriPitchRoll >= 1) {
    if (mPriPitchRoll == 1) {
      // We favor first received after last WD
      if (source != prio127257) return;
    }
    else {
      // First time use after WD time out.
      prio127257 = source;
    }

    unsigned char SID;
    double Yaw, Pitch, Roll;

    // Get roll and pitch
    if (ParseN2kPGN127257(v, SID, Yaw, Pitch, Roll)) {
      if (!N2kIsNA(Pitch)) {
        double m_pitch = GEODESIC_RAD2DEG(Pitch);
        wxString p_unit = _T("\u00B0\u2191") + _("Up");
        if (m_pitch < 0) {
          p_unit = _T("\u00B0\u2193") + _("Down");
          m_pitch *= -1;
        }
        SendSentenceToAllInstruments(OCPN_DBP_STC_PITCH, m_pitch, p_unit);
        mPITCH_Watchdog = gps_watchdog_timeout_ticks;
        mPriPitchRoll = 1;
      }
      if (!N2kIsNA(Roll)) {
        double m_heel = GEODESIC_RAD2DEG(Roll);
        wxString h_unit = _T("\u00B0\u003E") + _("Stbd");
        if (m_heel < 0) {
          h_unit = _T("\u00B0\u003C") + _("Port");
          m_heel *= -1;
        }
        SendSentenceToAllInstruments(OCPN_DBP_STC_HEEL, m_heel, h_unit);
        mHEEL_Watchdog = gps_watchdog_timeout_ticks;
        mPriPitchRoll = 1;
      }
    }
  }
}

void dashboard_pi::HandleN2K_128267(ObservedEvt ev) {
  NMEA2000Id id_128267(128267);
  std::vector<uint8_t>v = GetN2000Payload(id_128267, ev);

  // Get a uniqe ID to prioritize source(s)
  unsigned char source_id = v.at(7);
  char ss[4];
  sprintf(ss, "%d", source_id);
  std::string ident = std::string(ss);
  std::string source = GetN2000Source(id_128267, ev);
  source += ":" + ident;

  if (mPriDepth >= 1) {
    if (mPriDepth == 1) {
      if (source != prio128267) return;
    }
    else {
      prio128267 = source;
    }

    unsigned char SID;
    double DepthBelowTransducer, Offset, Range;

    // Get water depth
    if (ParseN2kPGN128267(v, SID, DepthBelowTransducer, Offset, Range)) {
      if (!N2kIsNA(DepthBelowTransducer)) {
        double depth = DepthBelowTransducer;
        // Set prio to sensor's offset
        if (!std::isnan(Offset) && !N2kIsNA(Offset)) depth += Offset;
        else (depth += g_dDashDBTOffset);

        SendSentenceToAllInstruments(OCPN_DBP_STC_DPT,
          toUsrDistance_Plugin(depth / 1852.0, g_iDashDepthUnit),
          getUsrDistanceUnit_Plugin(g_iDashDepthUnit));
        mPriDepth = 1;
        mDPT_DBT_Watchdog = gps_watchdog_timeout_ticks;
      }
    }
  }
}


void dashboard_pi::HandleN2K_128275(ObservedEvt ev) {
  NMEA2000Id id_128275(128275);
  std::vector<uint8_t>v = GetN2000Payload(id_128275, ev);
  uint16_t DaysSince1970;
  double SecondsSinceMidnight;
  uint32_t Log, TripLog;

  // Get log & Trip log
  if (ParseN2kPGN128275(v, DaysSince1970, SecondsSinceMidnight, Log, TripLog)) {

    if (!N2kIsNA(Log)) {
      double m_slog = METERS2NM((double)Log);
      SendSentenceToAllInstruments( OCPN_DBP_STC_VLW2,
                              toUsrDistance_Plugin(m_slog, g_iDashDistanceUnit),
                              getUsrDistanceUnit_Plugin(g_iDashDistanceUnit));
      mLOG_Watchdog = no_nav_watchdog_timeout_ticks;
    }
  }
  if (!N2kIsNA(TripLog)) {
    double m_tlog = METERS2NM((double)TripLog);
    SendSentenceToAllInstruments(
      OCPN_DBP_STC_VLW1, toUsrDistance_Plugin(m_tlog, g_iDashDistanceUnit),
      getUsrDistanceUnit_Plugin(g_iDashDistanceUnit));
    mTrLOG_Watchdog = no_nav_watchdog_timeout_ticks;
  }
}

void dashboard_pi::HandleN2K_128259(ObservedEvt ev) {
  NMEA2000Id id_128259(128259);
  std::vector<uint8_t>v = GetN2000Payload(id_128259, ev);

  // Get a uniqe ID to prioritize source(s)
  unsigned char source_id = v.at(7);
  char ss[4];
  sprintf(ss, "%d", source_id);
  std::string ident = std::string(ss);
  std::string source = GetN2000Source(id_128259, ev);
  source += ":" + ident;

  if (mPriSTW >= 1) {
    if (mPriSTW == 1) {
      if (source != prio128259) return;
    }
    else {
      prio128259 = source;
    }

    unsigned char SID;
    double WaterReferenced, GroundReferenced;
    tN2kSpeedWaterReferenceType SWRT;

    // Get speed through water
    if (ParseN2kPGN128259(v, SID, WaterReferenced, GroundReferenced, SWRT)) {

      if (!N2kIsNA(WaterReferenced)) {
        double stw_knots = MS2KNOTS(WaterReferenced);
        SendSentenceToAllInstruments(
          OCPN_DBP_STC_STW, toUsrSpeed_Plugin(stw_knots, g_iDashSpeedUnit),
          getUsrSpeedUnit_Plugin(g_iDashSpeedUnit));
        mPriSTW = 1;
        mSTW_Watchdog = gps_watchdog_timeout_ticks;
      }
    }
  }
}

wxString talker_N2k = wxEmptyString;
void dashboard_pi::HandleN2K_129029(ObservedEvt ev) {
  NMEA2000Id id_129029(129029);
  std::vector<uint8_t>v = GetN2000Payload(id_129029, ev);
  // Get a uniqe ID to prioritize source(s)
  unsigned char source_id = v.at(7);
  char ss[4];
  sprintf(ss, "%d", source_id);
  std::string ident = std::string(ss);
  std::string source = GetN2000Source(id_129029, ev);
  source += ":" + ident;
  //Use the source prioritized by OCPN only
  if (source != prioN2kPGNsat) return;

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

  // Get used satellite system
  if (ParseN2kPGN129029(v, SID, DaysSince1970, SecondsSinceMidnight,
                        Latitude, Longitude, Altitude,
                        GNSStype, GNSSmethod,
                        nSatellites, HDOP, PDOP, GeoidalSeparation,
                        nReferenceStations, ReferenceStationType, ReferenceSationID,
                        AgeOfCorrection)) {
    switch (GNSStype) {
      case 0: talker_N2k = "GP"; break;  //GPS
      case 1: talker_N2k = "GL"; break;  //GLONASS
      case 2: talker_N2k = "GPSGLONAS"; break;
      case 3: talker_N2k = "GP"; break;
      case 4: talker_N2k = "GPSGLONAS"; break;
      case 5: talker_N2k = "Chayka"; break;
      case 8: talker_N2k = "GA"; break;  //Galileo
      default: talker_N2k = wxEmptyString;
    }
    if (!N2kIsNA(Altitude)) {
      if (mPriAlt >= 1) {
        SendSentenceToAllInstruments(OCPN_DBP_STC_ALTI, Altitude, _T("m"));
        mPriAlt = 1;
        mALT_Watchdog = gps_watchdog_timeout_ticks;
      }
    }
  }
}

void dashboard_pi::HandleN2K_129540(ObservedEvt ev) {
  NMEA2000Id id_129540(129540);
  std::vector<uint8_t>v = GetN2000Payload(id_129540, ev);

  // Get a uniqe ID to prioritize source(s)
  unsigned char source_id = v.at(7);
  char ss[4];
  sprintf(ss, "%d", source_id);
  std::string ident = std::string(ss);
  std::string source = GetN2000Source(id_129540, ev);
  source += ":" + ident;
  //Use the source prioritized by OCPN only
  if (source != prioN2kPGNsat) return;

  unsigned char SID;
  tN2kRangeResidualMode Mode;
  uint8_t NumberOfSVs;

  // Get the GNSS status data
  if (ParseN2kPGN129540(v, SID, Mode, NumberOfSVs)) {

    if (!N2kIsNA(NumberOfSVs) && mPriSatStatus == 1) {
      // Step through each satellite, one-by-one
      // Arrange to max three messages with up to 4 sats each like N0183 GSV
      SAT_INFO N2K_SatInfo[4];
      int iPRN = 0;
      int iSNR = 0;
      double dElevRad = 0;
      double dAzimRad = 0;
      int idx = 0;
      uint8_t index = 0;
      for (int iMesNum = 0; iMesNum < 3; iMesNum++) {
        for (idx = 0; idx < 4; idx++) {
          tSatelliteInfo SatelliteInfo;
          index = idx + 4 * iMesNum;
          if (index >= NumberOfSVs -1) break;
          if (ParseN2kPGN129540(v, index, SatelliteInfo)) {
            iPRN = (int)SatelliteInfo.PRN;
            dElevRad = SatelliteInfo.Elevation;
            dAzimRad = SatelliteInfo.Azimuth;
            iSNR = N2kIsNA(SatelliteInfo.SNR) ? 0 : (int)SatelliteInfo.SNR;

            N2K_SatInfo[idx].SatNumber = iPRN;
            N2K_SatInfo[idx].ElevationDegrees = GEODESIC_RAD2DEG(dElevRad);
            N2K_SatInfo[idx].AzimuthDegreesTrue = GEODESIC_RAD2DEG(dAzimRad);
            N2K_SatInfo[idx].SignalToNoiseRatio = iSNR;
          }
        }
        // Send to GPS.cpp
        if (idx > 0) {
          SendSatInfoToAllInstruments(NumberOfSVs, iMesNum + 1, talker_N2k, N2K_SatInfo);
          //mPriSatStatus = 1;
          mSatStatus_Wdog = gps_watchdog_timeout_ticks;
        }
      }
    }
  }
}

// Wind   PGN 130306
void dashboard_pi::HandleN2K_130306(ObservedEvt ev) {
  NMEA2000Id id_130306(130306);
  std::vector<uint8_t>v = GetN2000Payload(id_130306, ev);

  // Get a uniqe ID to prioritize source(s)
  unsigned char source_id = v.at(7);
  char ss[4];
  sprintf(ss, "%d", source_id);
  std::string ident = std::string(ss);
  std::string source = GetN2000Source(id_130306, ev);
  source += ":" + ident;

  if (mPriWDN >= 1) {
    if (mPriWDN == 1) {
      if (source != prio130306) return;
    }
    else {
      prio130306 = source;
    }

    unsigned char SID;
    double WindSpeed, WindAngle;
    tN2kWindReference WindReference;

    // Get wind data
    if (ParseN2kPGN130306(v, SID, WindSpeed, WindAngle, WindReference)) {

      if (!N2kIsNA(WindSpeed) && !N2kIsNA(WindAngle)) {
        double m_twaangle, m_twaspeed_kn;
        bool sendTrueWind = false;

        switch (WindReference) {
        case 0: // N2kWind direction True North
          if (mPriWDN >= 1) {
            double m_twdT = GEODESIC_RAD2DEG(WindAngle);
            SendSentenceToAllInstruments(OCPN_DBP_STC_TWD, m_twdT, _T("\u00B0"));
            mPriWDN = 1;
            mWDN_Watchdog = no_nav_watchdog_timeout_ticks;
          }
          break;
        case 1:  // N2kWind direction Magnetic North
          if (mPriWDN >= 1) {
            double m_twdT = GEODESIC_RAD2DEG(WindAngle);
            // Make it true if variation is available
            if (!std::isnan(mVar)) {
              m_twdT = (m_twdT)+mVar;
              if (m_twdT > 360.) {
                m_twdT -= 360;
              }
              else if (m_twdT < 0.) {
                m_twdT += 360;
              }
            }
            SendSentenceToAllInstruments(OCPN_DBP_STC_TWD, m_twdT, _T("\u00B0"));
            mPriWDN = 1;
            mWDN_Watchdog = no_nav_watchdog_timeout_ticks;
          }
          break;
        case 2: // N2kWind_Apparent_centerline
          if (mPriAWA >= 1) {
            double m_awaangle, m_awaspeed_kn, calc_angle;
            // Angle equals 0-360 degr
            m_awaangle = GEODESIC_RAD2DEG(WindAngle);
            calc_angle = m_awaangle;
            wxString m_awaunit = _T("\u00B0R");
            // Should be unit "L" and 0-180 to port
            if (m_awaangle > 180.0) {
              m_awaangle = 360.0 - m_awaangle;
              m_awaunit = _T("\u00B0L");
            }
            SendSentenceToAllInstruments(OCPN_DBP_STC_AWA, m_awaangle, m_awaunit);
            // Speed
            m_awaspeed_kn = MS2KNOTS(WindSpeed);
            SendSentenceToAllInstruments(OCPN_DBP_STC_AWS,
              toUsrSpeed_Plugin(m_awaspeed_kn, g_iDashWindSpeedUnit),
              getUsrSpeedUnit_Plugin(g_iDashWindSpeedUnit));
            mPriAWA = 1;
            mMWVA_Watchdog = gps_watchdog_timeout_ticks;

            // If not N2K true wind data are recently received calculate it.
            if (mPriTWA != 1) {
              // Wants -+ angle instead of "L"/"R"
              if (calc_angle > 180) calc_angle -= 360.0;
              CalculateAndUpdateTWDS(m_awaspeed_kn, calc_angle);
              mPriTWA = 2;
              mPriWDN = 2;
              mMWVT_Watchdog = gps_watchdog_timeout_ticks;
              mWDN_Watchdog = no_nav_watchdog_timeout_ticks;
            }
          }
          break;
        case 3: // N2kWind_True_centerline_boat(ground)
          if (mPriTWA >= 1 && g_bDBtrueWindGround) {
            m_twaangle = GEODESIC_RAD2DEG(WindAngle);
            m_twaspeed_kn = MS2KNOTS(WindSpeed);
            sendTrueWind = true;
          }
          break;
        case 4: // N2kWind_True_Centerline__water
          if (mPriTWA >= 1 && !g_bDBtrueWindGround) {
            m_twaangle = GEODESIC_RAD2DEG(WindAngle);
            m_twaspeed_kn = MS2KNOTS(WindSpeed);
            sendTrueWind = true;
          }
          break;
        case 6: // N2kWind_Error
          break;
        case 7: // N2kWind_Unavailable
          break;
        default: break;
        }

        if (sendTrueWind) {
          // Wind angle is 0-360 degr
          wxString m_twaunit = _T("\u00B0R");
          // Should be unit "L" and 0-180 to port
          if (m_twaangle > 180.0) {
            m_twaangle = 360.0 - m_twaangle;
            m_twaunit = _T("\u00B0L");
          }
          SendSentenceToAllInstruments(OCPN_DBP_STC_TWA, m_twaangle, m_twaunit);
          // Wind speed
          SendSentenceToAllInstruments(OCPN_DBP_STC_TWS,
            toUsrSpeed_Plugin(m_twaspeed_kn, g_iDashWindSpeedUnit),
            getUsrSpeedUnit_Plugin(g_iDashWindSpeedUnit));
          SendSentenceToAllInstruments(OCPN_DBP_STC_TWS2,
            toUsrSpeed_Plugin(m_twaspeed_kn, g_iDashWindSpeedUnit),
            getUsrSpeedUnit_Plugin(g_iDashWindSpeedUnit));
          mPriTWA = 1;
          mPriWDN = 1; // For source prio
          mMWVT_Watchdog = gps_watchdog_timeout_ticks;
        }
      }
    }
  }
}

void dashboard_pi::HandleN2K_130310(ObservedEvt ev) {
  NMEA2000Id id_130310(130310);
  std::vector<uint8_t>v = GetN2000Payload(id_130310, ev);
  unsigned char SID;
  double WaterTemperature, OutsideAmbientAirTemperature, AtmosphericPressure;

  // Outside Environmental parameters
  if (ParseN2kPGN130310(v, SID, WaterTemperature,
                        OutsideAmbientAirTemperature, AtmosphericPressure)) {
    if (mPriWTP >= 1) {
      if (!N2kIsNA(WaterTemperature)) {
        double m_wtemp KELVIN2C(WaterTemperature);
        SendSentenceToAllInstruments(
          OCPN_DBP_STC_TMP, toUsrTemp_Plugin(m_wtemp, g_iDashTempUnit),
          getUsrTempUnit_Plugin(g_iDashTempUnit));
        mPriWTP =1;
        mWTP_Watchdog = no_nav_watchdog_timeout_ticks;
      }
    }

    if (mPriATMP >= 1) {
      if (!N2kIsNA(OutsideAmbientAirTemperature)) {
        double m_airtemp = KELVIN2C(OutsideAmbientAirTemperature);
        if (m_airtemp > -60 && m_airtemp < 100) {
          SendSentenceToAllInstruments(
            OCPN_DBP_STC_ATMP, toUsrTemp_Plugin(m_airtemp, g_iDashTempUnit),
            getUsrTempUnit_Plugin(g_iDashTempUnit));
          mPriATMP = 1;
          mATMP_Watchdog = no_nav_watchdog_timeout_ticks;
        }
      }
    }

    if (!N2kIsNA(AtmosphericPressure)) {
      double m_press = PA2HPA(AtmosphericPressure);
      SendSentenceToAllInstruments(OCPN_DBP_STC_MDA, m_press, _T("hPa"));
      mMDA_Watchdog = no_nav_watchdog_timeout_ticks;
    }
  }
}


/****** Signal K *******/
void dashboard_pi::ParseSignalK(wxString &msg) {
  wxJSONValue root;
  wxJSONReader jsonReader;

  int errors = jsonReader.Parse(msg, &root);

  // wxString dmsg( _T("Dashboard:SignalK Event received: ") );
  // dmsg.append(msg);
  // wxLogMessage(dmsg);
  // printf("%s\n", dmsg.ToUTF8().data());

  if (root.HasMember("self")) {
    if (root["self"].AsString().StartsWith(_T("vessels.")))
      m_self = (root["self"].AsString());  // for java server, and OpenPlotter
                                           // node.js server 1.20
    else if (root["self"].AsString().Length())
      m_self =
          _T("vessels.") + (root["self"].AsString());  // for Node.js server

  }

  if (root.HasMember("context") && root["context"].IsString()) {
    auto context = root["context"].AsString();
    if (context != m_self) {
      return;
    }
  }

  if (root.HasMember("updates") && root["updates"].IsArray()) {
    wxJSONValue &updates = root["updates"];
    for (int i = 0; i < updates.Size(); ++i) {
      handleSKUpdate(updates[i]);
    }
  }
}

void dashboard_pi::handleSKUpdate(wxJSONValue &update) {
  wxString sfixtime = "";

  if (update.HasMember("timestamp")) {
    sfixtime = update["timestamp"].AsString();
  }
  if (update.HasMember("values") && update["values"].IsArray()) {
    wxString talker = wxEmptyString;
    if (update.HasMember("source")) {
      if (update["source"].HasMember("talker")) {
        if (update["source"]["talker"].IsString()) {
          talker = update["source"]["talker"].AsString();
        }
      }
    }
    for (int j = 0; j < update["values"].Size(); ++j) {
      wxJSONValue &item = update["values"][j];
      updateSKItem(item, talker, sfixtime);
    }
  }
}

void dashboard_pi::updateSKItem(wxJSONValue &item, wxString &talker, wxString &sfixtime) {
  if (item.HasMember("path") && item.HasMember("value")) {
    const wxString &update_path = item["path"].AsString();
    wxJSONValue &value = item["value"];

    // Container for last received sat-system info from SK-N2k
    // TODO Watchdog?
    static wxString talkerID = wxEmptyString;

    if (update_path == _T("navigation.position")) {
      if (mPriPosition >= 2) {
        if (value["latitude"].IsDouble() && value["longitude"].IsDouble()) {
          double lat = value["latitude"].AsDouble();
          double lon = value["longitude"].AsDouble();
          SendSentenceToAllInstruments(OCPN_DBP_STC_LAT, lat, _T("SDMM"));
          SendSentenceToAllInstruments(OCPN_DBP_STC_LON, lon, _T("SDMM"));
          mPriPosition = 2;
        }
      }
    }
    else if (update_path == _T("navigation.speedOverGround") &&
             2 == mPriPosition) {
      double sog_knot = GetJsonDouble(value);
      if (std::isnan(sog_knot)) return;

      SendSentenceToAllInstruments(
        OCPN_DBP_STC_SOG,
        toUsrSpeed_Plugin(mSOGFilter.filter(sog_knot), g_iDashSpeedUnit),
        getUsrSpeedUnit_Plugin(g_iDashSpeedUnit));
    }
    else if (update_path == _T("navigation.courseOverGroundTrue") &&
             2 == mPriPosition) {
      double cog_rad = GetJsonDouble(value);
      if (std::isnan(cog_rad)) return;

      double cog_deg = GEODESIC_RAD2DEG(cog_rad);
      SendSentenceToAllInstruments(OCPN_DBP_STC_COG, mCOGFilter.filter(cog_deg),
                                   _T("\u00B0"));
    }
    else if (update_path == _T("navigation.headingTrue")) {
      if (mPriHeadingT >= 2) {
        double hdt = GetJsonDouble(value);
        if (std::isnan(hdt)) return;

        hdt = GEODESIC_RAD2DEG(hdt);
        SendSentenceToAllInstruments(OCPN_DBP_STC_HDT, hdt, _T("\u00B0T"));
        mPriHeadingT = 2;
        mHDT_Watchdog = gps_watchdog_timeout_ticks;
      }
    }
    else if (update_path == _T("navigation.headingMagnetic")) {
      if (mPriHeadingM >= 2) {
        double hdm = GetJsonDouble(value);
        if (std::isnan(hdm)) return;

        hdm = GEODESIC_RAD2DEG(hdm);
        SendSentenceToAllInstruments(OCPN_DBP_STC_HDM, hdm, _T("\u00B0M"));
        mPriHeadingM = 2;
        mHDx_Watchdog = gps_watchdog_timeout_ticks;

        // If no higher priority HDT, calculate it here.
        if (mPriHeadingT >= 6 && ( !std::isnan(mVar) )) {
          double heading = hdm + mVar;
          if (heading < 0)
            heading += 360;
          else if (heading >= 360.0)
            heading -= 360;
          SendSentenceToAllInstruments(OCPN_DBP_STC_HDT, heading, _T("\u00B0"));
          mPriHeadingT = 6;
          mHDT_Watchdog = gps_watchdog_timeout_ticks;
        }
      }
    }
    else if (update_path == _T("navigation.speedThroughWater")) {
      if (mPriSTW >= 2) {
        double stw_knots = GetJsonDouble(value);
        if (std::isnan(stw_knots)) return;

        stw_knots = MS2KNOTS(stw_knots);
        SendSentenceToAllInstruments(
          OCPN_DBP_STC_STW, toUsrSpeed_Plugin(stw_knots, g_iDashSpeedUnit),
          getUsrSpeedUnit_Plugin(g_iDashSpeedUnit));
        mPriSTW = 2;
        mSTW_Watchdog = gps_watchdog_timeout_ticks;
      }
    }
    else if (update_path == _T("navigation.magneticVariation")) {
      if (mPriVar >= 2) {
        double dvar = GetJsonDouble(value);
        if (std::isnan(dvar)) return;

        dvar = GEODESIC_RAD2DEG(dvar);
        if (0.0 != dvar) {  // Let WMM do the job instead
          SendSentenceToAllInstruments(OCPN_DBP_STC_HMV, dvar, _T("\u00B0"));
          mPriVar = 2;
          mVar_Watchdog = gps_watchdog_timeout_ticks;
        }
      }
    }
    else if (update_path == _T("environment.wind.angleApparent")) {
      if (mPriAWA >= 2) {
        double m_awaangle = GetJsonDouble(value);
        if (std::isnan(m_awaangle)) return;

        m_awaangle = GEODESIC_RAD2DEG(m_awaangle);  // negative to port
        wxString m_awaunit = _T("\u00B0R");
        if (m_awaangle < 0) {
          m_awaunit = _T("\u00B0L");
          m_awaangle *= -1;
        }
        SendSentenceToAllInstruments(OCPN_DBP_STC_AWA, m_awaangle, m_awaunit);
        mPriAWA = 2;  // Set prio only here. No need to catch speed if no angle.
        mMWVA_Watchdog = gps_watchdog_timeout_ticks;
      }
    }
    else if (update_path == _T("environment.wind.speedApparent")) {
      if (mPriAWA >= 2) {
        double m_awaspeed_kn = GetJsonDouble(value);
        if (std::isnan(m_awaspeed_kn)) return;

        m_awaspeed_kn = MS2KNOTS(m_awaspeed_kn);
        SendSentenceToAllInstruments(
          OCPN_DBP_STC_AWS,
          toUsrSpeed_Plugin(m_awaspeed_kn, g_iDashWindSpeedUnit),
          getUsrSpeedUnit_Plugin(g_iDashWindSpeedUnit));
      }
    }
    else if (( update_path == _T("environment.wind.angleTrueWater") &&
              !g_bDBtrueWindGround ) ||
              ( update_path == _T("environment.wind.angleTrueGround") &&
               g_bDBtrueWindGround )) {
      if (mPriTWA >= 3) {
        double m_twaangle = GetJsonDouble(value);
        if (std::isnan(m_twaangle)) return;

        m_twaangle = GEODESIC_RAD2DEG(m_twaangle);
        double m_twaangle_raw = m_twaangle;  // for wind history
        wxString m_twaunit = _T("\u00B0R");
        if (m_twaangle < 0) {
          m_twaunit = _T("\u00B0L");
          m_twaangle *= -1;
        }
        SendSentenceToAllInstruments(OCPN_DBP_STC_TWA, m_twaangle, m_twaunit);
        mPriTWA = 3;  // Set prio only here. No need to catch speed if no angle.
        mMWVT_Watchdog = gps_watchdog_timeout_ticks;

        if (mPriWDN >= 5) {
          // m_twaangle_raw has wind angle relative to the bow.
          // Wind history use angle relative to north.
          // If no TWD with higher priority is present and
          // true heading is available calculate it.
          if (g_dHDT < 361. && g_dHDT >= 0.0) {
            double g_dCalWdir = (m_twaangle_raw)+g_dHDT;
            if (g_dCalWdir > 360.) {
              g_dCalWdir -= 360;
            }
            else if (g_dCalWdir < 0.) {
              g_dCalWdir += 360;
            }
            SendSentenceToAllInstruments(OCPN_DBP_STC_TWD, g_dCalWdir,
                                         _T("\u00B0"));
            mPriWDN = 5;
            mWDN_Watchdog = no_nav_watchdog_timeout_ticks;
          }
        }
      }
    }
    else if (( update_path == _T("environment.wind.speedTrue") &&
              !g_bDBtrueWindGround ) ||
              ( update_path == _T("environment.wind.speedOverGround") &&
               g_bDBtrueWindGround )) {
      if (mPriTWA >= 3) {
        double m_twaspeed_kn = GetJsonDouble(value);
        if (std::isnan(m_twaspeed_kn)) return;

        m_twaspeed_kn = MS2KNOTS(m_twaspeed_kn);
        SendSentenceToAllInstruments(
          OCPN_DBP_STC_TWS,
          toUsrSpeed_Plugin(m_twaspeed_kn, g_iDashWindSpeedUnit),
          getUsrSpeedUnit_Plugin(g_iDashWindSpeedUnit));
        SendSentenceToAllInstruments(
          OCPN_DBP_STC_TWS2,
          toUsrSpeed_Plugin(m_twaspeed_kn, g_iDashWindSpeedUnit),
          getUsrSpeedUnit_Plugin(g_iDashWindSpeedUnit));
      }
    }
    else if (update_path == _T("environment.depth.belowSurface")) {
      if (mPriDepth >= 3) {
        double depth = GetJsonDouble(value);
        if (std::isnan(depth)) return;

        mPriDepth = 3;
        depth += g_dDashDBTOffset;
        depth /= 1852.0;
        SendSentenceToAllInstruments(
          OCPN_DBP_STC_DPT, toUsrDistance_Plugin(depth, g_iDashDepthUnit),
          getUsrDistanceUnit_Plugin(g_iDashDepthUnit));
        mDPT_DBT_Watchdog = gps_watchdog_timeout_ticks;
      }
    }
    else if (update_path == _T("environment.depth.belowTransducer")) {
      if (mPriDepth >= 3) {
        double depth = GetJsonDouble(value);
        if (std::isnan(depth)) return;

        mPriDepth = 3;
        depth += g_dDashDBTOffset;
        depth /= 1852.0;
        SendSentenceToAllInstruments(
          OCPN_DBP_STC_DPT, toUsrDistance_Plugin(depth, g_iDashDepthUnit),
          getUsrDistanceUnit_Plugin(g_iDashDepthUnit));
        mDPT_DBT_Watchdog = gps_watchdog_timeout_ticks;
      }
    }
    else if (update_path == _T("environment.water.temperature")) {
      if (mPriWTP >= 2) {
        double m_wtemp = GetJsonDouble(value);
        if (std::isnan(m_wtemp)) return;

        m_wtemp = KELVIN2C(m_wtemp);
        if (m_wtemp > -60 && m_wtemp < 200 && !std::isnan(m_wtemp)) {
          SendSentenceToAllInstruments(
            OCPN_DBP_STC_TMP, toUsrTemp_Plugin(m_wtemp, g_iDashTempUnit),
            getUsrTempUnit_Plugin(g_iDashTempUnit));
          mPriWTP = 2;
          mWTP_Watchdog = no_nav_watchdog_timeout_ticks;
        }
      }
    }
    else if (update_path ==
             _T("navigation.courseRhumbline.nextPoint.velocityMadeGood")) {
      double m_vmg_kn = GetJsonDouble(value);
      if (std::isnan(m_vmg_kn)) return;

      m_vmg_kn = MS2KNOTS(m_vmg_kn);
      SendSentenceToAllInstruments(
        OCPN_DBP_STC_VMG, toUsrSpeed_Plugin(m_vmg_kn, g_iDashSpeedUnit),
        getUsrSpeedUnit_Plugin(g_iDashSpeedUnit));
      mVMG_Watchdog = gps_watchdog_timeout_ticks;
    }

    else if (update_path ==
             _T("performance.velocityMadeGood")) {
      double m_vmgw_kn = GetJsonDouble(value);
      if (std::isnan(m_vmgw_kn)) return;

      m_vmgw_kn = MS2KNOTS(m_vmgw_kn);
      SendSentenceToAllInstruments(
        OCPN_DBP_STC_VMGW, toUsrSpeed_Plugin(m_vmgw_kn, g_iDashSpeedUnit),
        getUsrSpeedUnit_Plugin(g_iDashSpeedUnit));
      mVMGW_Watchdog = gps_watchdog_timeout_ticks;
    }

    else if (update_path == _T("steering.rudderAngle")) {  // ->port
      if (mPriRSA >= 2) {
        double m_rudangle = GetJsonDouble(value);
        if (std::isnan(m_rudangle)) return;

        m_rudangle = GEODESIC_RAD2DEG(m_rudangle);
        SendSentenceToAllInstruments(OCPN_DBP_STC_RSA, m_rudangle, _T("\u00B0"));
        mRSA_Watchdog = gps_watchdog_timeout_ticks;
        mPriRSA = 2;
      }
    }
    else if (update_path ==
             _T("navigation.gnss.satellites")) {  // GNSS satellites in use
      if (mPriSatUsed >= 2) {
        int usedSats = ( value ).AsInt();
        if (usedSats < 1 ) return;
        SendSentenceToAllInstruments(OCPN_DBP_STC_SAT, usedSats, _T (""));
        mPriSatUsed = 2;
        mSatsUsed_Wdog = gps_watchdog_timeout_ticks;
      }
    }
    else if (update_path == _T("navigation.gnss.type") ) {
      if (value.IsString() && value.AsString() != wxEmptyString) {
        talkerID = (value.AsString()); //Like "Combined GPS/GLONASS"
        talkerID.MakeUpper();
        if (( talkerID.Contains(_T("GPS")) ) && ( talkerID.Contains(_T("GLONASS")) ))
          talkerID = _T("GPSGLONAS");
        else if (talkerID.Contains(_T("GPS")))
          talkerID = _T("GP");
        else if (talkerID.Contains(_T("GLONASS")))
          talkerID = _T("GL");
        else if (talkerID.Contains(_T("GALILEO")))
          talkerID = _T("GA");
        else if (talkerID.Contains(_T("BEIDOU")))
          talkerID = _T("GI");
      }
    }
    else if (update_path ==
               _T("navigation.gnss.satellitesInView")) {  // GNSS satellites in
                                                          // view
      if (mPriSatUsed >= 4 ) {
        if (value.HasMember("count") && value["count"].IsInt()) {
          double m_SK_SatsInView = (value["count"].AsInt());
          SendSentenceToAllInstruments(OCPN_DBP_STC_SAT, m_SK_SatsInView,
                                       _T (""));
          mPriSatUsed = 4;
          mSatsUsed_Wdog = gps_watchdog_timeout_ticks;
        }
      }
      if (mPriSatStatus == 2) {
        if (value.HasMember("satellites") && value["satellites"].IsArray()) {
          // Update satellites data.
          int iNumSats;
          if (value.HasMember("count") && value["count"].IsInt()) {
            iNumSats = ( value["count"].AsInt() );
          }
          else iNumSats = value[_T ("satellites")].Size();

          SAT_INFO SK_SatInfo[4];
          for (int idx = 0; idx < 4; idx++) {
            SK_SatInfo[idx].SatNumber = 0;
            SK_SatInfo[idx].ElevationDegrees = 0;
            SK_SatInfo[idx].AzimuthDegreesTrue = 0;
            SK_SatInfo[idx].SignalToNoiseRatio = 0;
          }

          if (iNumSats) {
            // Arrange SK's array[12] to max three messages like NMEA GSV
            int iID = 0;
            int iSNR = 0;
            double dElevRad = 0;
            double dAzimRad = 0;
            int idx = 0;
            int arr = 0;
            for (int iMesNum = 0; iMesNum < 3; iMesNum++) {
              for (idx = 0; idx < 4; idx++) {
                arr = idx + 4 * iMesNum;
                  if (value["satellites"][arr]["id"].IsInt())
                    iID = value["satellites"][arr]["id"].AsInt();
                  if (value["satellites"][arr]["elevation"].IsDouble())
                    dElevRad = value["satellites"][arr]["elevation"].AsDouble();
                  if (value["satellites"][arr]["azimuth"].IsDouble())
                    dAzimRad = value["satellites"][arr]["azimuth"].AsDouble();
                  if (value["satellites"][arr]["SNR"].IsInt())
                    iSNR = value["satellites"][arr]["SNR"].AsInt();

                if (iID < 1) break;
                SK_SatInfo[idx].SatNumber = iID;
                SK_SatInfo[idx].ElevationDegrees = GEODESIC_RAD2DEG(dElevRad);
                SK_SatInfo[idx].AzimuthDegreesTrue = GEODESIC_RAD2DEG(dAzimRad);
                SK_SatInfo[idx].SignalToNoiseRatio = iSNR;
              }
              if (idx > 0) {
                if (talker != wxEmptyString && (talker.StartsWith(_T("G")) || talker.StartsWith(_T("BD"))) ) {
                  talkerID = talker; //Origin NMEA0183
                }
                SendSatInfoToAllInstruments(iNumSats, iMesNum + 1, talkerID, SK_SatInfo);
                //mPriSatStatus = 2;
                mSatStatus_Wdog = gps_watchdog_timeout_ticks;
              }

              if (iID < 1) break;
            }
          }
        }
      }
    } else if (update_path == _T("navigation.gnss.antennaAltitude")) {
      if (mPriAlt >= 2) {
        double m_alt = GetJsonDouble(value);
        if (std::isnan(m_alt)) return;

        SendSentenceToAllInstruments(OCPN_DBP_STC_ALTI, m_alt, _T("m"));
        mPriAlt = 2;
        mALT_Watchdog = gps_watchdog_timeout_ticks;
      }

    } else if (update_path == _T("navigation.datetime")) {
      if (mPriDateTime >= 1) {
        mPriDateTime = 1;
        wxString s_dt = (value.AsString());  //"2019-12-28T09:26:58.000Z"
        s_dt.Replace('-', wxEmptyString);
        s_dt.Replace(':', wxEmptyString);
        wxString utc_dt = s_dt.BeforeFirst('T');      // Date
        utc_dt.Append(s_dt.AfterFirst('T').Left(6));  // time
        mUTCDateTime.ParseFormat(utc_dt.c_str(), _T("%Y%m%d%H%M%S"));
        mUTC_Watchdog = gps_watchdog_timeout_ticks;
      }
    } else if (update_path == _T("environment.outside.temperature")) {
      if (mPriATMP >= 2) {
        double m_airtemp = GetJsonDouble(value);
        if (std::isnan(m_airtemp)) return;

        m_airtemp = KELVIN2C(m_airtemp);
        if (m_airtemp > -60 && m_airtemp < 100) {
          SendSentenceToAllInstruments(
              OCPN_DBP_STC_ATMP, toUsrTemp_Plugin(m_airtemp, g_iDashTempUnit),
              getUsrTempUnit_Plugin(g_iDashTempUnit));
          mPriATMP = 2;
          mATMP_Watchdog = no_nav_watchdog_timeout_ticks;
        }
      }
    } else if (update_path ==
               _T("environment.wind.directionTrue")) {  // relative true north
      if (mPriWDN >= 3) {
        double m_twdT = GetJsonDouble(value);
        if (std::isnan(m_twdT)) return;

        m_twdT = GEODESIC_RAD2DEG(m_twdT);
        SendSentenceToAllInstruments(OCPN_DBP_STC_TWD, m_twdT, _T("\u00B0"));
        mPriWDN = 3;
        mWDN_Watchdog = no_nav_watchdog_timeout_ticks;
      }
    } else if (update_path == _T("environment.wind.directionMagnetic")) {
      // relative magn north
      if (mPriWDN >= 4) {
        double m_twdM = GetJsonDouble(value);
        if (std::isnan(m_twdM)) return;
        m_twdM = GEODESIC_RAD2DEG(m_twdM);
        // Make it true if variation is available
        if (!std::isnan(mVar)) {
          m_twdM = (m_twdM) + mVar;
          if (m_twdM > 360.) {
            m_twdM -= 360;
          }
          else if (m_twdM < 0.) {
            m_twdM += 360;
          }
        }
        SendSentenceToAllInstruments(OCPN_DBP_STC_TWD, m_twdM, _T("\u00B0M"));
        mPriWDN = 4;
        mWDN_Watchdog = no_nav_watchdog_timeout_ticks;
      }
    } else if (update_path == _T("navigation.trip.log")) {  // m
      double m_tlog = GetJsonDouble(value);
      if (std::isnan(m_tlog)) return;

      m_tlog = METERS2NM(m_tlog);
      SendSentenceToAllInstruments(
          OCPN_DBP_STC_VLW1, toUsrDistance_Plugin(m_tlog, g_iDashDistanceUnit),
          getUsrDistanceUnit_Plugin(g_iDashDistanceUnit));
      mTrLOG_Watchdog = no_nav_watchdog_timeout_ticks;
    } else if (update_path == _T("navigation.log")) {  // m
      double m_slog = GetJsonDouble(value);
      if (std::isnan(m_slog)) return;

      m_slog = METERS2NM(m_slog);
      SendSentenceToAllInstruments(
          OCPN_DBP_STC_VLW2, toUsrDistance_Plugin(m_slog, g_iDashDistanceUnit),
          getUsrDistanceUnit_Plugin(g_iDashDistanceUnit));
      mLOG_Watchdog = no_nav_watchdog_timeout_ticks;
    } else if (update_path == _T("environment.outside.pressure")) {  // Pa
      double m_press = GetJsonDouble(value);
      if (std::isnan(m_press)) return;

      m_press = PA2HPA(m_press);
      SendSentenceToAllInstruments(OCPN_DBP_STC_MDA, m_press, _T("hPa"));
      mMDA_Watchdog = no_nav_watchdog_timeout_ticks;
    } else if (update_path == _T("navigation.attitude")) {  // rad
      if (mPriPitchRoll >= 2) {
        if (value["roll"].AsString() != "0") {
          double m_heel = GEODESIC_RAD2DEG(value["roll"].AsDouble());
          wxString h_unit = _T("\u00B0\u003E") + _("Stbd");
          if (m_heel < 0) {
            h_unit = _T("\u00B0\u003C") + _("Port");
            m_heel *= -1;
          }
          SendSentenceToAllInstruments(OCPN_DBP_STC_HEEL, m_heel, h_unit);
          mHEEL_Watchdog = gps_watchdog_timeout_ticks;
          mPriPitchRoll = 2;
        }
        if (value["pitch"].AsString() != "0") {
          double m_pitch = GEODESIC_RAD2DEG(value["pitch"].AsDouble());
          wxString p_unit = _T("\u00B0\u2191") + _("Up");
          if (m_pitch < 0) {
            p_unit = _T("\u00B0\u2193") + _("Down");
            m_pitch *= -1;
          }
          SendSentenceToAllInstruments(OCPN_DBP_STC_PITCH, m_pitch, p_unit);
          mPITCH_Watchdog = gps_watchdog_timeout_ticks;
          mPriPitchRoll = 2;
        }
      }
    }
  }
}

/*******Nav data from OCPN core *******/
void dashboard_pi::SetPositionFixEx(PlugIn_Position_Fix_Ex &pfix) {

  if (mPriPosition >= 1) {
    mPriPosition = 1;
    SendSentenceToAllInstruments(OCPN_DBP_STC_LAT, pfix.Lat, _T("SDMM"));
    SendSentenceToAllInstruments(OCPN_DBP_STC_LON, pfix.Lon, _T("SDMM"));
  }
  if (mPriCOGSOG >= 1) {
    double dMagneticCOG;
    mPriCOGSOG = 1;
    SendSentenceToAllInstruments(
        OCPN_DBP_STC_SOG,
        toUsrSpeed_Plugin(mSOGFilter.filter(pfix.Sog), g_iDashSpeedUnit),
        getUsrSpeedUnit_Plugin(g_iDashSpeedUnit));
    SendSentenceToAllInstruments(OCPN_DBP_STC_COG, mCOGFilter.filter(pfix.Cog),
                                 _T("\u00B0"));
    dMagneticCOG = mCOGFilter.get() - pfix.Var;
    if (dMagneticCOG < 0.0) dMagneticCOG = 360.0 + dMagneticCOG;
    if (dMagneticCOG > 360.0) dMagneticCOG = dMagneticCOG - 360.0;
    SendSentenceToAllInstruments(OCPN_DBP_STC_MCOG, dMagneticCOG,
                                 _T("\u00B0M"));
  }
  if (mPriVar >= 1) {
    if (!std::isnan(pfix.Var)) {
      mPriVar = 1;
      mVar = pfix.Var;
      mVar_Watchdog = gps_watchdog_timeout_ticks;

      SendSentenceToAllInstruments(OCPN_DBP_STC_HMV, pfix.Var, _T("\u00B0"));
    }
  }
  if (mPriDateTime >= 6) {  // We prefer the GPS datetime
    mUTCDateTime.Set(pfix.FixTime);
    if (mUTCDateTime.IsValid()) {
      mPriDateTime = 6;
      mUTCDateTime = mUTCDateTime.ToUTC();
      mUTC_Watchdog = gps_watchdog_timeout_ticks;
    }
  }
  if (mPriSatUsed >= 1) {
    mSatsInUse = pfix.nSats;
    if (mSatsInUse > 0) {
      SendSentenceToAllInstruments(OCPN_DBP_STC_SAT, mSatsInUse, _T(""));
      mPriSatUsed = 1;
      mSatsUsed_Wdog = gps_watchdog_timeout_ticks;
    }
  }
  if (mPriHeadingT >= 1) {
    double hdt = pfix.Hdt;
    if (std::isnan(hdt)) return;
    SendSentenceToAllInstruments(OCPN_DBP_STC_HDT, hdt, _T("\u00B0T"));
    mPriHeadingT = 1;
    mHDT_Watchdog = gps_watchdog_timeout_ticks;
  }
  if (mPriHeadingM >= 1) {
    double hdm = pfix.Hdm;
    if (std::isnan(hdm) && !std::isnan(pfix.Hdt) && !std::isnan(pfix.Var)) {
      hdm = pfix.Hdt - pfix.Var;
      if (hdm < 0) hdm += 360;
      else if (hdm >= 360.0) hdm -= 360;
    }
    if (std::isnan(hdm)) return;
    SendSentenceToAllInstruments(OCPN_DBP_STC_HDM, hdm, _T("\u00B0M"));
    mPriHeadingM = 1;
    mHDx_Watchdog = gps_watchdog_timeout_ticks;
  }
}

void dashboard_pi::SetCursorLatLon(double lat, double lon) {
  SendSentenceToAllInstruments(OCPN_DBP_STC_PLA, lat, _T("SDMM"));
  SendSentenceToAllInstruments(OCPN_DBP_STC_PLO, lon, _T("SDMM"));
}

void dashboard_pi::SetPluginMessage(wxString &message_id,
                                    wxString &message_body) {
  if (message_id == _T("WMM_VARIATION_BOAT")) {
    // construct the JSON root object
    wxJSONValue root;
    // construct a JSON parser
    wxJSONReader reader;

    // now read the JSON text and store it in the 'root' structure
    // check for errors before retreiving values...
    int numErrors = reader.Parse(message_body, &root);
    if (numErrors > 0) {
      //              const wxArrayString& errors = reader.GetErrors();
      return;
    }

    // get the DECL value from the JSON message
    wxString decl = root[_T("Decl")].AsString();
    double decl_val;
    decl.ToDouble(&decl_val);

    if (mPriVar >= 5) {
      mPriVar = 5;
      mVar = decl_val;
      mVar_Watchdog = gps_watchdog_timeout_ticks;
      SendSentenceToAllInstruments(OCPN_DBP_STC_HMV, mVar, _T("\u00B0"));
    }
  } else if (message_id == _T("OCPN_CORE_SIGNALK")) {
    ParseSignalK(message_body);
  }
}

int dashboard_pi::GetToolbarToolCount(void) { return 1; }

void dashboard_pi::ShowPreferencesDialog(wxWindow *parent) {
  DashboardPreferencesDialog *dialog = new DashboardPreferencesDialog(
      parent, wxID_ANY, m_ArrayOfDashboardWindow);

  dialog->RecalculateSize();

#ifdef __OCPN__ANDROID__
  dialog->GetHandle()->setStyleSheet(qtStyleSheet);
#endif

#ifdef __OCPN__ANDROID__
  wxWindow *ccwin = GetOCPNCanvasWindow();

  if (ccwin) {
    int xmax = ccwin->GetSize().GetWidth();
    int ymax = ccwin->GetParent()
                   ->GetSize()
                   .GetHeight();  // This would be the Frame itself
    dialog->SetSize(xmax, ymax);
    dialog->Layout();

    dialog->Move(0, 0);
  }
#endif

  if (dialog->ShowModal() == wxID_OK) {
    double scaler = 1.0;
    if (OCPN_GetWinDIPScaleFactor() < 1.0)
      scaler = 1.0 + OCPN_GetWinDIPScaleFactor() / 4;
    scaler = wxMax(1.0, scaler);

    g_USFontTitle = *(dialog->m_pFontPickerTitle->GetFontData());
    g_FontTitle = *g_pUSFontTitle;
    g_FontTitle.SetChosenFont(g_pUSFontTitle->GetChosenFont().Scaled(scaler));
    g_FontTitle.SetColour(g_pUSFontTitle->GetColour());
    g_USFontTitle = *g_pUSFontTitle;

    g_USFontData = *(dialog->m_pFontPickerData->GetFontData());
    g_FontData = *g_pUSFontData;
    g_FontData.SetChosenFont(g_pUSFontData->GetChosenFont().Scaled(scaler));
    g_FontData.SetColour(g_pUSFontData->GetColour());
    g_USFontData = *g_pUSFontData;

    g_USFontLabel = *(dialog->m_pFontPickerLabel->GetFontData());
    g_FontLabel = *g_pUSFontLabel;
    g_FontLabel.SetChosenFont(g_pUSFontLabel->GetChosenFont().Scaled(scaler));
    g_FontLabel.SetColour(g_pUSFontLabel->GetColour());
    g_USFontLabel = *g_pUSFontLabel;

    g_USFontSmall = *(dialog->m_pFontPickerSmall->GetFontData());
    g_FontSmall = *g_pUSFontSmall;
    g_FontSmall.SetChosenFont(g_pUSFontSmall->GetChosenFont().Scaled(scaler));
    g_FontSmall.SetColour(g_pUSFontSmall->GetColour());
    g_USFontSmall = *g_pUSFontSmall;

    // OnClose should handle that for us normally but it doesn't seems to do so
    // We must save changes first
    g_dashPrefWidth = dialog->GetSize().x;
    g_dashPrefHeight = dialog->GetSize().y;

    dialog->SaveDashboardConfig();
    m_ArrayOfDashboardWindow.Clear();
    m_ArrayOfDashboardWindow = dialog->m_Config;

    ApplyConfig();
    SaveConfig();
    SetToolbarItemState(m_toolbar_item_id, GetDashboardWindowShownCount() != 0);
  }
  dialog->Destroy();
}

void dashboard_pi::SetColorScheme(PI_ColorScheme cs) {
  aktuellColorScheme = cs;
  for (size_t i = 0; i < m_ArrayOfDashboardWindow.GetCount(); i++) {
    DashboardWindow *dashboard_window =
        m_ArrayOfDashboardWindow.Item(i)->m_pDashboardWindow;
    if (dashboard_window) dashboard_window->SetColorScheme(cs);
  }
}

int dashboard_pi::GetDashboardWindowShownCount() {
  int cnt = 0;

  for (size_t i = 0; i < m_ArrayOfDashboardWindow.GetCount(); i++) {
    DashboardWindow *dashboard_window =
        m_ArrayOfDashboardWindow.Item(i)->m_pDashboardWindow;
    if (dashboard_window) {
      wxAuiPaneInfo &pane = m_pauimgr->GetPane(dashboard_window);
      if (pane.IsOk() && pane.IsShown()) cnt++;
    }
  }
  return cnt;
}

void dashboard_pi::OnPaneClose(wxAuiManagerEvent &event) {
  // if name is unique, we should use it
  DashboardWindow *dashboard_window = (DashboardWindow *)event.pane->window;
  int cnt = 0;
  for (size_t i = 0; i < m_ArrayOfDashboardWindow.GetCount(); i++) {
    DashboardWindowContainer *cont = m_ArrayOfDashboardWindow.Item(i);
    DashboardWindow *d_w = cont->m_pDashboardWindow;
    if (d_w) {
      // we must not count this one because it is being closed
      if (dashboard_window != d_w) {
        wxAuiPaneInfo &pane = m_pauimgr->GetPane(d_w);
        if (pane.IsOk() && pane.IsShown()) cnt++;
      } else {
        cont->m_bIsVisible = false;
      }
    }
  }
  SetToolbarItemState(m_toolbar_item_id, cnt != 0);

  event.Skip();
}

void dashboard_pi::OnToolbarToolCallback(int id) {
  int cnt = GetDashboardWindowShownCount();

  bool b_anyviz = false;
  for (size_t i = 0; i < m_ArrayOfDashboardWindow.GetCount(); i++) {
    DashboardWindowContainer *cont = m_ArrayOfDashboardWindow.Item(i);
    if (cont->m_bIsVisible) {
      b_anyviz = true;
      break;
    }
  }

  for (size_t i = 0; i < m_ArrayOfDashboardWindow.GetCount(); i++) {
    DashboardWindowContainer *cont = m_ArrayOfDashboardWindow.Item(i);
    DashboardWindow *dashboard_window = cont->m_pDashboardWindow;
    if (dashboard_window) {
      wxAuiPaneInfo &pane = m_pauimgr->GetPane(dashboard_window);
      if (pane.IsOk()) {
        bool b_reset_pos = false;

#ifdef __WXMSW__
        //  Support MultiMonitor setups which an allow negative window
        //  positions. If the requested window title bar does not intersect any
        //  installed monitor, then default to simple primary monitor
        //  positioning.
        RECT frame_title_rect;
        frame_title_rect.left = pane.floating_pos.x;
        frame_title_rect.top = pane.floating_pos.y;
        frame_title_rect.right = pane.floating_pos.x + pane.floating_size.x;
        frame_title_rect.bottom = pane.floating_pos.y + 30;

        if (NULL == MonitorFromRect(&frame_title_rect, MONITOR_DEFAULTTONULL))
          b_reset_pos = true;
#else

        //    Make sure drag bar (title bar) of window intersects wxClient Area
        //    of screen, with a little slop...
        wxRect window_title_rect;  // conservative estimate
        window_title_rect.x = pane.floating_pos.x;
        window_title_rect.y = pane.floating_pos.y;
        window_title_rect.width = pane.floating_size.x;
        window_title_rect.height = 30;

        wxRect ClientRect = wxGetClientDisplayRect();
        ClientRect.Deflate(
            60, 60);  // Prevent the new window from being too close to the edge
        if (!ClientRect.Intersects(window_title_rect)) b_reset_pos = true;

#endif

        if (b_reset_pos) pane.FloatingPosition(50, 50);

        if (cnt == 0)
          if (b_anyviz)
            pane.Show(cont->m_bIsVisible);
          else {
            cont->m_bIsVisible = cont->m_bPersVisible;
            pane.Show(cont->m_bIsVisible);
          }
        else
          pane.Show(false);
      }

      //  This patch fixes a bug in wxAUIManager
      //  FS#548
      // Dropping a DashBoard Window right on top on the (supposedly fixed)
      // chart bar window causes a resize of the chart bar, and the Dashboard
      // window assumes some of its properties The Dashboard window is no longer
      // grabbable... Workaround:  detect this case, and force the pane to be on
      // a different Row. so that the display is corrected by toggling the
      // dashboard off and back on.
      if ((pane.dock_direction == wxAUI_DOCK_BOTTOM) && pane.IsDocked())
        pane.Row(2);
    }
  }
  // Toggle is handled by the toolbar but we must keep plugin manager b_toggle
  // updated to actual status to ensure right status upon toolbar rebuild
  SetToolbarItemState(m_toolbar_item_id,
                      GetDashboardWindowShownCount() != 0 /*cnt==0*/);
  m_pauimgr->Update();
}

void dashboard_pi::UpdateAuiStatus(void) {
  //    This method is called after the PlugIn is initialized
  //    and the frame has done its initial layout, possibly from a saved
  //    wxAuiManager "Perspective" It is a chance for the PlugIn to syncronize
  //    itself internally with the state of any Panes that were added to the
  //    frame in the PlugIn ctor.

  for (size_t i = 0; i < m_ArrayOfDashboardWindow.GetCount(); i++) {
    DashboardWindowContainer *cont = m_ArrayOfDashboardWindow.Item(i);
    wxAuiPaneInfo &pane = m_pauimgr->GetPane(cont->m_pDashboardWindow);
    // Initialize visible state as perspective is loaded now
    cont->m_bIsVisible = (pane.IsOk() && pane.IsShown());

#ifdef __WXQT__
    if (pane.IsShown()) {
      pane.Show(false);
      m_pauimgr->Update();
      pane.Show(true);
      m_pauimgr->Update();
    }
#endif
  }
  m_pauimgr->Update();

  //    We use this callback here to keep the context menu selection in sync
  //    with the window state

  SetToolbarItemState(m_toolbar_item_id, GetDashboardWindowShownCount() != 0);
}

bool dashboard_pi::LoadConfig(void) {
  wxFileConfig *pConf = (wxFileConfig *)m_pconfig;

  if (pConf) {
    pConf->SetPath(_T("/PlugIns/Dashboard"));

    wxString version;
    pConf->Read(_T("Version"), &version, wxEmptyString);
    wxString config;

    // Set some sensible defaults
    wxString TitleFont;
    wxString DataFont;
    wxString LabelFont;
    wxString SmallFont;

#ifdef __OCPN__ANDROID__
    TitleFont = _T("Roboto,16,-1,5,50,0,0,0,0,0");
    DataFont = _T("Roboto,16,-1,5,50,0,0,0,0,0");
    LabelFont = _T("Roboto,16,-1,5,50,0,0,0,0,0");
    SmallFont = _T("Roboto,14,-1,5,50,0,0,0,0,0");
#else
    TitleFont = g_pFontTitle->GetChosenFont().GetNativeFontInfoDesc();
    DataFont = g_pFontData->GetChosenFont().GetNativeFontInfoDesc();
    LabelFont = g_pFontLabel->GetChosenFont().GetNativeFontInfoDesc();
    SmallFont = g_pFontSmall->GetChosenFont().GetNativeFontInfoDesc();
#endif

    double scaler = 1.0;
    wxFont DummyFont;
    wxFont* pDF = &DummyFont;

    if (OCPN_GetWinDIPScaleFactor() < 1.0)
      scaler = 1.0 + OCPN_GetWinDIPScaleFactor()/4;
    scaler = wxMax(1.0, scaler);

    g_pFontTitle = &g_FontTitle;
    pConf->Read(_T("FontTitle"), &config, TitleFont);
    LoadFont(&pDF, config);
    pConf->Read(_T("ColorTitle"), &config, "#000000");
    wxColour DummyColor(config);
    g_pUSFontTitle->SetChosenFont(DummyFont);
    g_pUSFontTitle->SetColour(DummyColor);
    g_FontTitle = *g_pUSFontTitle;
    g_FontTitle.SetChosenFont(g_pUSFontTitle->GetChosenFont().Scaled(scaler));
    g_USFontTitle = *g_pUSFontTitle;    

    g_pFontData = &g_FontData;
    pConf->Read(_T("FontData"), &config, DataFont);
    LoadFont(&pDF, config);
    pConf->Read(_T("ColorData"), &config, "#000000");
    DummyColor.Set(config);
    g_pUSFontData->SetChosenFont(DummyFont);
    g_pUSFontData->SetColour(DummyColor);
    g_FontData = *g_pUSFontData;
    g_FontData.SetChosenFont(g_pUSFontData->GetChosenFont().Scaled(scaler));
    g_USFontData = *g_pUSFontData;

    g_pFontLabel = &g_FontLabel;
    pConf->Read(_T("FontLabel"), &config, LabelFont);
    LoadFont(&pDF, config);
    pConf->Read(_T("ColorLabel"), &config, "#000000");
    DummyColor.Set(config);
    g_pUSFontLabel->SetChosenFont(DummyFont);
    g_pUSFontLabel->SetColour(DummyColor);
    g_FontLabel = *g_pUSFontLabel;
    g_FontLabel.SetChosenFont(g_pUSFontLabel->GetChosenFont().Scaled(scaler));
    g_USFontLabel = *g_pUSFontLabel;

    g_pFontSmall = &g_FontSmall;
    pConf->Read(_T("FontSmall"), &config, SmallFont);
    LoadFont(&pDF, config);
    pConf->Read(_T("ColorSmall"), &config, "#000000");
    DummyColor.Set(config);
    g_pUSFontSmall->SetChosenFont(DummyFont);
    g_pUSFontSmall->SetColour(DummyColor);
    g_FontSmall = *g_pUSFontSmall;
    g_FontSmall.SetChosenFont(g_pUSFontSmall->GetChosenFont().Scaled(scaler));
    g_USFontSmall = *g_pUSFontSmall;

    pConf->Read(_T("SpeedometerMax"), &g_iDashSpeedMax, 12);
    pConf->Read(_T("COGDamp"), &g_iDashCOGDamp, 0);
    pConf->Read(_T("SpeedUnit"), &g_iDashSpeedUnit, 0);
    pConf->Read(_T("SOGDamp"), &g_iDashSOGDamp, 0);
    pConf->Read(_T("DepthUnit"), &g_iDashDepthUnit, 3);
    g_iDashDepthUnit = wxMax(g_iDashDepthUnit, 3);

    pConf->Read(_T("DepthOffset"), &g_dDashDBTOffset, 0);

    pConf->Read(_T("DistanceUnit"), &g_iDashDistanceUnit, 0);
    pConf->Read(_T("WindSpeedUnit"), &g_iDashWindSpeedUnit, 0);
    pConf->Read(_T("UseSignKtruewind"), &g_bDBtrueWindGround, 0);
    pConf->Read(_T("TemperatureUnit"), &g_iDashTempUnit, 0);

    pConf->Read(_T("UTCOffset"), &g_iUTCOffset, 0);

    pConf->Read(_T("PrefWidth"), &g_dashPrefWidth, 0);
    pConf->Read(_T("PrefHeight"), &g_dashPrefHeight, 0);

    int d_cnt;
    pConf->Read(_T("DashboardCount"), &d_cnt, -1);
    // TODO: Memory leak? We should destroy everything first
    m_ArrayOfDashboardWindow.Clear();
    if (version.IsEmpty() && d_cnt == -1) {
      m_config_version = 1;
      // Let's load version 1 or default settings.
      int i_cnt;
      pConf->Read(_T("InstrumentCount"), &i_cnt, -1);
      wxArrayInt ar;
      wxArrayOfInstrumentProperties Property;
      if (i_cnt != -1) {
        for (int i = 0; i < i_cnt; i++) {
          int id;
          pConf->Read(wxString::Format(_T("Instrument%d"), i + 1), &id, -1);
          if (id != -1) ar.Add(id);
        }
      } else {
        // This is the default instrument list
#ifndef __OCPN__ANDROID__
        ar.Add(ID_DBP_I_POS);
        ar.Add(ID_DBP_D_COG);
        ar.Add(ID_DBP_D_GPS);
#else
        ar.Add(ID_DBP_I_POS);
        ar.Add(ID_DBP_D_COG);
        ar.Add(ID_DBP_I_SOG);

#endif
      }

      DashboardWindowContainer *cont = new DashboardWindowContainer(
          NULL, MakeName(), _("Dashboard"), _T("V"), ar, Property);
      cont->m_bPersVisible = true;
      m_ArrayOfDashboardWindow.Add(cont);

    } else {
      // Version 2
      m_config_version = 2;
      bool b_onePersisted = false;
      for (int k = 0; k < d_cnt; k++) {
        pConf->SetPath(
            wxString::Format(_T("/PlugIns/Dashboard/Dashboard%d"), k + 1));
        wxString name;
        pConf->Read(_T("Name"), &name, MakeName());
        wxString caption;
        pConf->Read(_T("Caption"), &caption, _("Dashboard"));
        wxString orient;
        pConf->Read(_T("Orientation"), &orient, _T("V"));
        int i_cnt;
        pConf->Read(_T("InstrumentCount"), &i_cnt, -1);
        bool b_persist;
        pConf->Read(_T("Persistence"), &b_persist, 1);

        wxArrayInt ar;
        wxArrayOfInstrumentProperties Property;
        for (int i = 0; i < i_cnt; i++) {
            int id;
            pConf->Read(wxString::Format(_T("Instrument%d"), i + 1), &id, -1);            
            if (id != -1)
            {
                ar.Add(id);
                InstrumentProperties* instp;
                if (pConf->Exists(wxString::Format(_T("InstTitelFont%d"), i + 1)))
                {
                    instp = new InstrumentProperties(id, i);

                    pConf->Read(wxString::Format(_T("InstTitelFont%d"), i + 1), &config, TitleFont);
                    LoadFont(&pDF, config);
                    pConf->Read(wxString::Format(_T("InstTitelColor%d"), i + 1), &config, "#000000");
                    DummyColor.Set(config);
                    instp->m_TitelFont.SetChosenFont(DummyFont);
                    instp->m_TitelFont.SetColour(DummyColor);

                    pConf->Read(wxString::Format(_T("InstDataFont%d"), i + 1), &config, DataFont);
                    LoadFont(&pDF, config);
                    pConf->Read(wxString::Format(_T("InstDataColor%d"), i + 1), &config, "#000000");
                    DummyColor.Set(config);
                    instp->m_DataFont.SetChosenFont(DummyFont);
                    instp->m_DataFont.SetColour(DummyColor);

                    pConf->Read(wxString::Format(_T("InstLabelFont%d"), i + 1), &config, LabelFont);
                    LoadFont(&pDF, config);
                    pConf->Read(wxString::Format(_T("InstLabelColor%d"), i + 1), &config, "#000000");
                    DummyColor.Set(config);
                    instp->m_LabelFont.SetChosenFont(DummyFont);
                    instp->m_LabelFont.SetColour(DummyColor);

                    pConf->Read(wxString::Format(_T("InstSmallFont%d"), i + 1), &config, SmallFont);
                    LoadFont(&pDF, config);
                    pConf->Read(wxString::Format(_T("InstSmallColor%d"), i + 1), &config, "#000000");
                    DummyColor.Set(config);
                    instp->m_SmallFont.SetChosenFont(DummyFont);
                    instp->m_SmallFont.SetColour(DummyColor);

                    pConf->Read(wxString::Format(_T("TitlelBackColor%d"), i + 1), &config, "DASHL");
                    instp->m_TitlelBackgroundColour.Set(config);

                    pConf->Read(wxString::Format(_T("DataBackColor%d"), i + 1), &config, "DASHB");
                    instp->m_DataBackgroundColour.Set(config);

                    pConf->Read(wxString::Format(_T("ArrowFirst%d"), i + 1), &config, "DASHN");
                    instp->m_Arrow_First_Colour.Set(config);

                    pConf->Read(wxString::Format(_T("ArrowSecond%d"), i + 1), &config, "BLUE3");
                    instp->m_Arrow_Second_Colour.Set(config);

                    Property.Add(instp);
                }
            }
        }
        // TODO: Do not add if GetCount == 0

        DashboardWindowContainer *cont =
            new DashboardWindowContainer(NULL, name, caption, orient, ar, Property);
        cont->m_bPersVisible = b_persist;

        if (b_persist) b_onePersisted = true;

        m_ArrayOfDashboardWindow.Add(cont);
      }

      // Make sure at least one dashboard is scheduled to be visible
      if (m_ArrayOfDashboardWindow.Count() && !b_onePersisted) {
        DashboardWindowContainer *cont = m_ArrayOfDashboardWindow.Item(0);
        if (cont) cont->m_bPersVisible = true;
      }
    }

    return true;
  } else
    return false;
}

void dashboard_pi::LoadFont(wxFont **target, wxString native_info) {
  if (!native_info.IsEmpty()) {
#ifdef __OCPN__ANDROID__
    wxFont *nf = new wxFont(native_info);
    *target = nf;
#else
    (*target)->SetNativeFontInfo(native_info);
#endif
  }
}

bool dashboard_pi::SaveConfig(void) {
  wxFileConfig *pConf = (wxFileConfig *)m_pconfig;

  if (pConf) {
    pConf->SetPath(_T("/PlugIns/Dashboard"));
    pConf->Write(_T("Version"), _T("2"));
    pConf->Write(_T("FontTitle"), g_pUSFontTitle->GetChosenFont().GetNativeFontInfoDesc());
    pConf->Write(_T("ColorTitle"), g_pUSFontTitle->GetColour().GetAsString(wxC2S_HTML_SYNTAX));
    pConf->Write(_T("FontData"), g_pUSFontData->GetChosenFont().GetNativeFontInfoDesc());
    pConf->Write(_T("ColorData"), g_pUSFontData->GetColour().GetAsString(wxC2S_HTML_SYNTAX));
    pConf->Write(_T("FontLabel"), g_pUSFontLabel->GetChosenFont().GetNativeFontInfoDesc());
    pConf->Write(_T("ColorLabel"), g_pUSFontLabel->GetColour().GetAsString(wxC2S_HTML_SYNTAX));
    pConf->Write(_T("FontSmall"), g_pUSFontSmall->GetChosenFont().GetNativeFontInfoDesc());
    pConf->Write(_T("ColorSmall"), g_pUSFontSmall->GetColour().GetAsString(wxC2S_HTML_SYNTAX));
    pConf->Write(_T("SpeedometerMax"), g_iDashSpeedMax);
    pConf->Write(_T("COGDamp"), g_iDashCOGDamp);
    pConf->Write(_T("SpeedUnit"), g_iDashSpeedUnit);
    pConf->Write(_T("SOGDamp"), g_iDashSOGDamp);
    pConf->Write(_T("DepthUnit"), g_iDashDepthUnit);
    pConf->Write(_T("DepthOffset"), g_dDashDBTOffset);
    pConf->Write(_T("DistanceUnit"), g_iDashDistanceUnit);
    pConf->Write(_T("WindSpeedUnit"), g_iDashWindSpeedUnit);
    pConf->Write(_T("UTCOffset"), g_iUTCOffset);
    pConf->Write(_T("UseSignKtruewind"), g_bDBtrueWindGround);
    pConf->Write(_T("TemperatureUnit"), g_iDashTempUnit);
    pConf->Write(_T("PrefWidth"), g_dashPrefWidth);
    pConf->Write(_T("PrefHeight"), g_dashPrefHeight);

    pConf->Write(_T("DashboardCount" ),
                 (int)m_ArrayOfDashboardWindow.GetCount());
    // Delete old Dashborads
    for (unsigned int i = m_ArrayOfDashboardWindow.GetCount(); i < 20; i++) {
        if (pConf->Exists(wxString::Format(_T("/PlugIns/Dashboard/Dashboard%d"), i + 1)))
        {
            pConf->DeleteGroup(wxString::Format(_T("/PlugIns/Dashboard/Dashboard%d"), i + 1));
        }
    }
    for (unsigned int i = 0; i < m_ArrayOfDashboardWindow.GetCount(); i++) {
      DashboardWindowContainer *cont = m_ArrayOfDashboardWindow.Item(i);
      pConf->SetPath(
          wxString::Format(_T("/PlugIns/Dashboard/Dashboard%d"), i + 1));
      pConf->Write(_T("Name"), cont->m_sName);
      pConf->Write(_T("Caption"), cont->m_sCaption);
      pConf->Write(_T("Orientation"), cont->m_sOrientation);
      pConf->Write(_T("Persistence"), cont->m_bPersVisible);
      pConf->Write(_T("InstrumentCount"),
                   (int)cont->m_aInstrumentList.GetCount());
      // Delete old Instruments
      for (unsigned int i = cont->m_aInstrumentList.GetCount(); i < 40; i++) {
          if (pConf->Exists(wxString::Format(_T("Instrument%d"), i + 1)))
          {
              pConf->DeleteEntry(wxString::Format(_T("Instrument%d"), i + 1));
              if (pConf->Exists(wxString::Format(_T("InstTitelFont%d"), i + 1)))
              {
                  pConf->DeleteEntry(wxString::Format(_T("InstTitelFont%d"), i + 1));
                  pConf->DeleteEntry(wxString::Format(_T("InstTitelColor%d"), i + 1));
                  pConf->DeleteEntry(wxString::Format(_T("InstDataFont%d"), i + 1));
                  pConf->DeleteEntry(wxString::Format(_T("InstDataColor%d"), i + 1));
                  pConf->DeleteEntry(wxString::Format(_T("InstLabelFont%d"), i + 1));
                  pConf->DeleteEntry(wxString::Format(_T("InstLabelColor%d"), i + 1));
                  pConf->DeleteEntry(wxString::Format(_T("InstSmallFont%d"), i + 1));
                  pConf->DeleteEntry(wxString::Format(_T("InstSmallColor%d"), i + 1));
                  pConf->DeleteEntry(wxString::Format(_T("TitlelBackColor%d"), i + 1));
                  pConf->DeleteEntry(wxString::Format(_T("DataBackColor%d"), i + 1));
                  pConf->DeleteEntry(wxString::Format(_T("ArrowFirst%d"), i + 1));
                  pConf->DeleteEntry(wxString::Format(_T("ArrowSecond%d"), i + 1));
              }
          }
      }
      for (size_t j = 0; j < cont->m_aInstrumentList.GetCount(); j++)
      {
          pConf->Write(wxString::Format(_T("Instrument%d"), j + 1), cont->m_aInstrumentList.Item(j));
          InstrumentProperties* Inst = NULL;
          // First delete
          if (pConf->Exists(wxString::Format(_T("InstTitelFont%d"), j + 1)))
          {
              bool Delete = true;
              for (size_t i = 0; i < cont->m_aInstrumentPropertyList.GetCount(); i++)
              {
                  Inst = cont->m_aInstrumentPropertyList.Item(i);
                  if (Inst->m_Listplace == (int)j)
                  {
                      Delete = false;
                      break;
                  }                  
              }
              if (Delete)
              {
                  pConf->DeleteEntry(wxString::Format(_T("InstTitelFont%d"), j + 1));
                  pConf->DeleteEntry(wxString::Format(_T("InstTitelColor%d"), j + 1));
                  pConf->DeleteEntry(wxString::Format(_T("InstDataFont%d"), j + 1));
                  pConf->DeleteEntry(wxString::Format(_T("InstDataColor%d"), j + 1));
                  pConf->DeleteEntry(wxString::Format(_T("InstLabelFont%d"), j + 1));
                  pConf->DeleteEntry(wxString::Format(_T("InstLabelColor%d"), j + 1));
                  pConf->DeleteEntry(wxString::Format(_T("InstSmallFont%d"), j + 1));
                  pConf->DeleteEntry(wxString::Format(_T("InstSmallColor%d"), j + 1));
                  pConf->DeleteEntry(wxString::Format(_T("TitlelBackColor%d"), i + 1));
                  pConf->DeleteEntry(wxString::Format(_T("DataBackColor%d"), i + 1));
                  pConf->DeleteEntry(wxString::Format(_T("ArrowFirst%d"), i + 1));
                  pConf->DeleteEntry(wxString::Format(_T("ArrowSecond%d"), i + 1));
              }
          }
          Inst = NULL;
          for (size_t i = 0; i < (cont->m_aInstrumentPropertyList.GetCount()); i++)
          {
              Inst = cont->m_aInstrumentPropertyList.Item(i);
              if (Inst->m_Listplace == (int)j)
              {
                  pConf->Write(wxString::Format(_T("InstTitelFont%d"), j + 1), Inst->m_TitelFont.GetChosenFont().GetNativeFontInfoDesc());
                  pConf->Write(wxString::Format(_T("InstTitelColor%d"), j + 1), Inst->m_TitelFont.GetColour().GetAsString(wxC2S_HTML_SYNTAX));
                  pConf->Write(wxString::Format(_T("InstDataFont%d"), j + 1), Inst->m_DataFont.GetChosenFont().GetNativeFontInfoDesc());
                  pConf->Write(wxString::Format(_T("InstDataColor%d"), j + 1), Inst->m_DataFont.GetColour().GetAsString(wxC2S_HTML_SYNTAX));
                  pConf->Write(wxString::Format(_T("InstLabelFont%d"), j + 1), Inst->m_LabelFont.GetChosenFont().GetNativeFontInfoDesc());
                  pConf->Write(wxString::Format(_T("InstLabelColor%d"), j + 1), Inst->m_LabelFont.GetColour().GetAsString(wxC2S_HTML_SYNTAX));
                  pConf->Write(wxString::Format(_T("InstSmallFont%d"), j + 1), Inst->m_SmallFont.GetChosenFont().GetNativeFontInfoDesc());
                  pConf->Write(wxString::Format(_T("InstSmallColor%d"), j + 1), Inst->m_SmallFont.GetColour().GetAsString(wxC2S_HTML_SYNTAX));
                  pConf->Write(wxString::Format(_T("TitlelBackColor%d"), j + 1), Inst->m_TitlelBackgroundColour.GetAsString(wxC2S_HTML_SYNTAX));
                  pConf->Write(wxString::Format(_T("DataBackColor%d"), j + 1), Inst->m_DataBackgroundColour.GetAsString(wxC2S_HTML_SYNTAX));
                  pConf->Write(wxString::Format(_T("ArrowFirst%d"), j + 1), Inst->m_Arrow_First_Colour.GetAsString(wxC2S_HTML_SYNTAX));
                  pConf->Write(wxString::Format(_T("ArrowSecond%d"), j + 1), Inst->m_Arrow_Second_Colour.GetAsString(wxC2S_HTML_SYNTAX));
                  break;
              }
          }
      }
    }
    return true;
  } else
    return false;
}

void dashboard_pi::ApplyConfig(void) {
  // Reverse order to handle deletes
  for (size_t i = m_ArrayOfDashboardWindow.GetCount(); i > 0; i--) {
    DashboardWindowContainer *cont = m_ArrayOfDashboardWindow.Item(i - 1);
    int orient = (cont->m_sOrientation == _T("V") ? wxVERTICAL : wxHORIZONTAL);
    if (cont->m_bIsDeleted) {
      if (cont->m_pDashboardWindow) {
        m_pauimgr->DetachPane(cont->m_pDashboardWindow);
        cont->m_pDashboardWindow->Close();
        cont->m_pDashboardWindow->Destroy();
        cont->m_pDashboardWindow = NULL;
      }
      m_ArrayOfDashboardWindow.Remove(cont);
      delete cont;

    } else if (!cont->m_pDashboardWindow) {
      // A new dashboard is created
      cont->m_pDashboardWindow = new DashboardWindow(
          GetOCPNCanvasWindow(), wxID_ANY, m_pauimgr, this, orient, cont);
      cont->m_pDashboardWindow->SetInstrumentList(cont->m_aInstrumentList, &(cont->m_aInstrumentPropertyList));
      bool vertical = orient == wxVERTICAL;
      wxSize sz = cont->m_pDashboardWindow->GetMinSize();
// Mac has a little trouble with initial Layout() sizing...
#ifdef __WXOSX__
      if (sz.x == 0) sz.IncTo(wxSize(160, 388));
#endif
      wxAuiPaneInfo p = wxAuiPaneInfo()
                            .Name(cont->m_sName)
                            .Caption(cont->m_sCaption)
                            .CaptionVisible(false)
                            .TopDockable(!vertical)
                            .BottomDockable(!vertical)
                            .LeftDockable(vertical)
                            .RightDockable(vertical)
                            .MinSize(sz)
                            .BestSize(sz)
                            .FloatingSize(sz)
                            .FloatingPosition(100, 100)
                            .Float()
                            .Show(cont->m_bIsVisible)
                            .Gripper(false);

      m_pauimgr->AddPane(cont->m_pDashboardWindow, p);
      // wxAuiPaneInfo().Name( cont->m_sName ).Caption( cont->m_sCaption
      // ).CaptionVisible( false ).TopDockable(
      // !vertical ).BottomDockable( !vertical ).LeftDockable( vertical
      // ).RightDockable( vertical ).MinSize( sz ).BestSize( sz ).FloatingSize(
      // sz ).FloatingPosition( 100, 100 ).Float().Show( cont->m_bIsVisible ) );

#ifdef __OCPN__ANDROID__
      wxAuiPaneInfo &pane = m_pauimgr->GetPane(cont->m_pDashboardWindow);
      pane.Dockable(false);

#endif

    } else {
      wxAuiPaneInfo &pane = m_pauimgr->GetPane(cont->m_pDashboardWindow);
      pane.Caption(cont->m_sCaption).Show(cont->m_bIsVisible);
      if (!cont->m_pDashboardWindow->isInstrumentListEqual(
              cont->m_aInstrumentList)) {
        cont->m_pDashboardWindow->SetInstrumentList(cont->m_aInstrumentList, &(cont->m_aInstrumentPropertyList));
        wxSize sz = cont->m_pDashboardWindow->GetMinSize();
        pane.MinSize(sz).BestSize(sz).FloatingSize(sz);
      }
      if (cont->m_pDashboardWindow->GetSizerOrientation() != orient) {
        cont->m_pDashboardWindow->ChangePaneOrientation(orient, false);
      }
    }
  }
  m_pauimgr->Update();

  double sogFC = g_iDashSOGDamp ? 1.0 / (2.0 * g_iDashSOGDamp) : 0.0;
  double cogFC = g_iDashCOGDamp ? 1.0 / (2.0 * g_iDashCOGDamp) : 0.0;

  if (abs(sogFC - mSOGFilter.getFc()) > 1e-6) mSOGFilter.setFC(sogFC);
  if (abs(cogFC - mCOGFilter.getFc()) > 1e-6) mCOGFilter.setFC(cogFC);
}

void dashboard_pi::PopulateContextMenu(wxMenu *menu) {
  for (size_t i = 0; i < m_ArrayOfDashboardWindow.GetCount(); i++) {
    DashboardWindowContainer *cont = m_ArrayOfDashboardWindow.Item(i);
    wxMenuItem *item = menu->AppendCheckItem(i + 1, cont->m_sCaption);
    item->Check(cont->m_bIsVisible);
  }
}

void dashboard_pi::ShowDashboard(size_t id, bool visible) {
  if (id < m_ArrayOfDashboardWindow.GetCount()) {
    DashboardWindowContainer *cont = m_ArrayOfDashboardWindow.Item(id);
    m_pauimgr->GetPane(cont->m_pDashboardWindow).Show(visible);
    cont->m_bIsVisible = visible;
    cont->m_bPersVisible = visible;
    m_pauimgr->Update();
  }
}

/* DashboardPreferencesDialog
 *
 */

DashboardPreferencesDialog::DashboardPreferencesDialog(
    wxWindow *parent, wxWindowID id, wxArrayOfDashboard config)
    : wxDialog(parent, id, _("Dashboard preferences"), wxDefaultPosition,
               wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER) {
#ifdef __WXQT__
  wxFont *pF = OCPNGetFont(_T("Dialog"), 0);
  SetFont(*pF);
#endif

  int display_width, display_height;
  wxDisplaySize(&display_width, &display_height);

  wxString shareLocn = *GetpSharedDataLocation() + _T("plugins") +
                       wxFileName::GetPathSeparator() + _T("dashboard_pi") +
                       wxFileName::GetPathSeparator() + _T("data") +
                       wxFileName::GetPathSeparator();

  Connect(wxEVT_CLOSE_WINDOW,
          wxCloseEventHandler(DashboardPreferencesDialog::OnCloseDialog), NULL,
          this);

  // Copy original config
  m_Config = wxArrayOfDashboard(config);
  //      Build Dashboard Page for Toolbox
  int border_size = 2;

  wxBoxSizer *itemBoxSizerMainPanel = new wxBoxSizer(wxVERTICAL);
  SetSizer(itemBoxSizerMainPanel);

  wxWindow *dparent = this;
#ifndef __ANDROID__
  wxScrolledWindow *scrollWin = new wxScrolledWindow(
      this, wxID_ANY, wxDefaultPosition, wxSize(-1, -1), wxVSCROLL | wxHSCROLL);

  scrollWin->SetScrollRate(1, 1);
  itemBoxSizerMainPanel->Add(scrollWin, 1, wxEXPAND | wxALL, 0);

  dparent = scrollWin;

  wxBoxSizer *itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
  scrollWin->SetSizer(itemBoxSizer2);
#else
  wxBoxSizer *itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
  itemBoxSizerMainPanel->Add(itemBoxSizer2, 1, wxEXPAND);
#endif

  wxStdDialogButtonSizer *DialogButtonSizer =
  CreateStdDialogButtonSizer(wxOK | wxCANCEL);
  itemBoxSizerMainPanel->Add(DialogButtonSizer, 0, wxALIGN_RIGHT | wxALL, 5);


  wxNotebook *itemNotebook = new wxNotebook(dparent, wxID_ANY, wxDefaultPosition,
                                            wxDefaultSize, wxNB_TOP);
  itemBoxSizer2->Add(itemNotebook, 0, wxALL | wxEXPAND, border_size);

  wxPanel *itemPanelNotebook01 =
      new wxPanel(itemNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                  wxTAB_TRAVERSAL);
  wxFlexGridSizer *itemFlexGridSizer01 = new wxFlexGridSizer(2);
  itemFlexGridSizer01->AddGrowableCol(1);
  itemPanelNotebook01->SetSizer(itemFlexGridSizer01);
  itemNotebook->AddPage(itemPanelNotebook01, _("Dashboard"));

  wxBoxSizer *itemBoxSizer01 = new wxBoxSizer(wxVERTICAL);
  itemFlexGridSizer01->Add(itemBoxSizer01, 1, wxEXPAND | wxTOP | wxLEFT,
                           border_size);

  // Scale the images in the dashboard list control
  int imageRefSize = GetCharWidth() * 4;

  wxImageList *imglist1 = new wxImageList(imageRefSize, imageRefSize, true, 1);

  wxBitmap bmDashBoard;
#ifdef ocpnUSE_SVG
  wxString filename = shareLocn + _T("Dashboard.svg");
  bmDashBoard = GetBitmapFromSVGFile(filename, imageRefSize, imageRefSize);
#else
  wxImage dash1 = wxBitmap(*_img_dashboard_pi).ConvertToImage();
  wxImage dash1s =
      dash1.Scale(imageRefSize, imageRefSize, wxIMAGE_QUALITY_HIGH);
  bmDashBoard = wxBitmap(dash1s);
#endif

  imglist1->Add(bmDashBoard);

  m_pListCtrlDashboards =
      new wxListCtrl(itemPanelNotebook01, wxID_ANY, wxDefaultPosition,
                     wxSize(imageRefSize * 3 / 2, 200),
                     wxLC_REPORT | wxLC_NO_HEADER | wxLC_SINGLE_SEL);

  m_pListCtrlDashboards->AssignImageList(imglist1, wxIMAGE_LIST_SMALL);
  m_pListCtrlDashboards->InsertColumn(0, _T(""));
  m_pListCtrlDashboards->Connect(
      wxEVT_COMMAND_LIST_ITEM_SELECTED,
      wxListEventHandler(DashboardPreferencesDialog::OnDashboardSelected), NULL,
      this);
  m_pListCtrlDashboards->Connect(
      wxEVT_COMMAND_LIST_ITEM_DESELECTED,
      wxListEventHandler(DashboardPreferencesDialog::OnDashboardSelected), NULL,
      this);
  itemBoxSizer01->Add(m_pListCtrlDashboards, 1, wxEXPAND, 0);

  wxBoxSizer *itemBoxSizer02 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer01->Add(itemBoxSizer02);

  wxBitmap bmPlus, bmMinus;
  int bmSize = imageRefSize * 100 / 275;

#ifdef __OCPN__ANDROID__
  bmSize = imageRefSize / 2;
#endif

#ifdef ocpnUSE_SVG
  bmPlus = GetBitmapFromSVGFile(shareLocn + _T("plus.svg"), bmSize,
                                bmSize);
  bmMinus = GetBitmapFromSVGFile(shareLocn + _T("minus.svg"), bmSize,
                                 bmSize);
#else
  wxImage plus1 = wxBitmap(*_img_plus).ConvertToImage();
  wxImage plus1s =
      plus1.Scale(bmSize, bmSize, wxIMAGE_QUALITY_HIGH);
  bmPlus = wxBitmap(plus1s);

  wxImage minus1 = wxBitmap(*_img_minus).ConvertToImage();
  wxImage minus1s =
      minus1.Scale(bmSize, bmSize, wxIMAGE_QUALITY_HIGH);
  bmMinus = wxBitmap(minus1s);
#endif

  m_pButtonAddDashboard = new wxBitmapButton(
      itemPanelNotebook01, wxID_ANY, bmPlus, wxDefaultPosition, wxDefaultSize);
  itemBoxSizer02->Add(m_pButtonAddDashboard, 0, wxALIGN_CENTER, 2);
  m_pButtonAddDashboard->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(DashboardPreferencesDialog::OnDashboardAdd), NULL,
      this);

  m_pButtonDeleteDashboard = new wxBitmapButton(
      itemPanelNotebook01, wxID_ANY, bmMinus, wxDefaultPosition, wxDefaultSize);
  itemBoxSizer02->Add(m_pButtonDeleteDashboard, 0, wxALIGN_CENTER, 2);
  m_pButtonDeleteDashboard->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(DashboardPreferencesDialog::OnDashboardDelete),
      NULL, this);

  m_pPanelDashboard =
      new wxPanel(itemPanelNotebook01, wxID_ANY, wxDefaultPosition,
                  wxDefaultSize, wxBORDER_SUNKEN);
  itemFlexGridSizer01->Add(m_pPanelDashboard, 1, wxEXPAND | wxTOP | wxRIGHT,
                           border_size);

  wxBoxSizer *itemBoxSizer03 = new wxBoxSizer(wxVERTICAL);
  m_pPanelDashboard->SetSizer(itemBoxSizer03);

  wxStaticBox *itemStaticBox02 =
      new wxStaticBox(m_pPanelDashboard, wxID_ANY, _("Dashboard"));
  wxStaticBoxSizer *itemStaticBoxSizer02 =
      new wxStaticBoxSizer(itemStaticBox02, wxHORIZONTAL);
  itemBoxSizer03->Add(itemStaticBoxSizer02, 0, wxEXPAND | wxALL, border_size);
  wxFlexGridSizer *itemFlexGridSizer = new wxFlexGridSizer(2);
  itemFlexGridSizer->AddGrowableCol(1);
  itemStaticBoxSizer02->Add(itemFlexGridSizer, 1, wxEXPAND | wxALL, 0);

  m_pCheckBoxIsVisible =
      new wxCheckBox(m_pPanelDashboard, wxID_ANY, _("show this dashboard"),
                     wxDefaultPosition, wxDefaultSize, 0);
  m_pCheckBoxIsVisible->SetMinSize(wxSize(25 * GetCharWidth(), -1));

  itemFlexGridSizer->Add(m_pCheckBoxIsVisible, 0, wxEXPAND | wxALL,
                         border_size);
  wxStaticText *itemDummy01 =
      new wxStaticText(m_pPanelDashboard, wxID_ANY, _T(""));
  itemFlexGridSizer->Add(itemDummy01, 0, wxEXPAND | wxALL, border_size);

  wxStaticText *itemStaticText01 =
      new wxStaticText(m_pPanelDashboard, wxID_ANY, _("Caption:"),
                       wxDefaultPosition, wxDefaultSize, 0);
  itemFlexGridSizer->Add(itemStaticText01, 0, wxEXPAND | wxALL, border_size);
  m_pTextCtrlCaption = new wxTextCtrl(m_pPanelDashboard, wxID_ANY, _T(""),
                                      wxDefaultPosition, wxDefaultSize);
  m_pCheckBoxIsVisible->SetMinSize(wxSize(30 * GetCharWidth(), -1));
  itemFlexGridSizer->Add(m_pTextCtrlCaption, 0, wxALIGN_RIGHT | wxALL,
                         border_size);

  wxStaticText *itemStaticText02 =
      new wxStaticText(m_pPanelDashboard, wxID_ANY, _("Orientation:"),
                       wxDefaultPosition, wxDefaultSize, 0);
  itemFlexGridSizer->Add(itemStaticText02, 0, wxEXPAND | wxALL, border_size);
  m_pChoiceOrientation = new wxChoice(m_pPanelDashboard, wxID_ANY,
                                      wxDefaultPosition, wxDefaultSize);
  m_pChoiceOrientation->SetMinSize(wxSize(15 * GetCharWidth(), -1));
  m_pChoiceOrientation->Append(_("Vertical"));
  m_pChoiceOrientation->Append(_("Horizontal"));
  itemFlexGridSizer->Add(m_pChoiceOrientation, 0, wxALIGN_RIGHT | wxALL,
                         border_size);

  int instImageRefSize = 20 * GetOCPNGUIToolScaleFactor_PlugIn();

  wxImageList *imglist =
      new wxImageList(instImageRefSize, instImageRefSize, true, 2);

  wxBitmap bmDial, bmInst;
#ifdef ocpnUSE_SVG
  bmDial = GetBitmapFromSVGFile(shareLocn + _T("dial.svg"), instImageRefSize,
                                instImageRefSize);
  bmInst = GetBitmapFromSVGFile(shareLocn + _T("instrument.svg"),
                                instImageRefSize, instImageRefSize);
#else
  wxImage dial1 = wxBitmap(*_img_dial).ConvertToImage();
  wxImage dial1s =
      dial1.Scale(instImageRefSize, instImageRefSize, wxIMAGE_QUALITY_HIGH);
  bmDial = wxBitmap(dial1);

  wxImage inst1 = wxBitmap(*_img_instrument).ConvertToImage();
  wxImage inst1s =
      inst1.Scale(instImageRefSize, instImageRefSize, wxIMAGE_QUALITY_HIGH);
  bmInst = wxBitmap(inst1s);
#endif

  imglist->Add(bmInst);
  imglist->Add(bmDial);

  wxStaticBox *itemStaticBox03 =
      new wxStaticBox(m_pPanelDashboard, wxID_ANY, _("Instruments"));
  wxStaticBoxSizer *itemStaticBoxSizer03 =
      new wxStaticBoxSizer(itemStaticBox03, wxHORIZONTAL);
  itemBoxSizer03->Add(itemStaticBoxSizer03, 1, wxEXPAND | wxALL, border_size);

  wxSize dsize = GetOCPNCanvasWindow()->GetClientSize();
  int vsize = dsize.y * 35 / 100;

#ifdef __OCPN__ANDROID__
  vsize = display_height * 50 / 100;
#endif

  m_pListCtrlInstruments = new wxListCtrl(
      m_pPanelDashboard, wxID_ANY, wxDefaultPosition, wxSize(-1, vsize),
      wxLC_REPORT | wxLC_NO_HEADER | wxLC_SINGLE_SEL);

  itemStaticBoxSizer03->Add(m_pListCtrlInstruments, 1, wxEXPAND | wxALL,
                            border_size);
  m_pListCtrlInstruments->AssignImageList(imglist, wxIMAGE_LIST_SMALL);
  m_pListCtrlInstruments->InsertColumn(0, _("Instruments"));
  m_pListCtrlInstruments->Connect(
      wxEVT_COMMAND_LIST_ITEM_SELECTED,
      wxListEventHandler(DashboardPreferencesDialog::OnInstrumentSelected),
      NULL, this);
  m_pListCtrlInstruments->Connect(
      wxEVT_COMMAND_LIST_ITEM_DESELECTED,
      wxListEventHandler(DashboardPreferencesDialog::OnInstrumentSelected),
      NULL, this);

  wxBoxSizer *itemBoxSizer04 = new wxBoxSizer(wxVERTICAL);
  itemStaticBoxSizer03->Add(itemBoxSizer04, 0, wxALIGN_TOP | wxALL,
                            border_size);
  m_pButtonAdd = new wxButton(m_pPanelDashboard, wxID_ANY, _("Add"),
                              wxDefaultPosition, wxSize(-1, -1));
  itemBoxSizer04->Add(m_pButtonAdd, 0, wxEXPAND | wxALL, border_size);
  m_pButtonAdd->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(DashboardPreferencesDialog::OnInstrumentAdd), NULL,
      this);

  // TODO  Instrument Properties ... done by Bernd Cirotzki
     m_pButtonEdit = new wxButton( m_pPanelDashboard, wxID_ANY, _("Edit"),
     wxDefaultPosition, wxDefaultSize ); itemBoxSizer04->Add( m_pButtonEdit, 0,
     wxEXPAND | wxALL, border_size ); m_pButtonEdit->Connect(
     wxEVT_COMMAND_BUTTON_CLICKED,
              wxCommandEventHandler(DashboardPreferencesDialog::OnInstrumentEdit),
     NULL, this );

  //    m_pFontPickerTitle =
  //        new wxFontPickerCtrl(m_pPanelDashboard, wxID_ANY, g_USFontTitle,
  //            wxDefaultPosition, wxSize(-1, -1), 0,
  //            wxDefaultValidator, "Bernd");
  //    itemBoxSizer04->Add(m_pFontPickerTitle, 0, wxALIGN_RIGHT | wxALL, 0);
  //    wxColor Farbe = m_pFontPickerTitle->GetSelectedColour();
  //    m_pFontPickerTitle->SetBackgroundColour(Farbe);

  m_pButtonDelete = new wxButton(m_pPanelDashboard, wxID_ANY, _("Delete"),
                                 wxDefaultPosition, wxSize(-1, -1));
  itemBoxSizer04->Add(m_pButtonDelete, 0, wxEXPAND | wxALL, border_size);
  m_pButtonDelete->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(DashboardPreferencesDialog::OnInstrumentDelete),
      NULL, this);
  itemBoxSizer04->AddSpacer(10);
  m_pButtonUp = new wxButton(m_pPanelDashboard, wxID_ANY, _("Up"),
                             wxDefaultPosition, wxDefaultSize);
  itemBoxSizer04->Add(m_pButtonUp, 0, wxEXPAND | wxALL, border_size);
  m_pButtonUp->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(DashboardPreferencesDialog::OnInstrumentUp), NULL,
      this);
  m_pButtonDown = new wxButton(m_pPanelDashboard, wxID_ANY, _("Down"),
                               wxDefaultPosition, wxDefaultSize);
  itemBoxSizer04->Add(m_pButtonDown, 0, wxEXPAND | wxALL, border_size);
  m_pButtonDown->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(DashboardPreferencesDialog::OnInstrumentDown), NULL,
      this);

  wxPanel *itemPanelNotebook02 =
      new wxPanel(itemNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                  wxTAB_TRAVERSAL);
  wxBoxSizer *itemBoxSizer05 = new wxBoxSizer(wxVERTICAL);
  itemPanelNotebook02->SetSizer(itemBoxSizer05);
  itemNotebook->AddPage(itemPanelNotebook02, _("Appearance"));

  wxStaticBox *itemStaticBox01 =
      new wxStaticBox(itemPanelNotebook02, wxID_ANY, _("Fonts"));
  wxStaticBoxSizer *itemStaticBoxSizer01 =
      new wxStaticBoxSizer(itemStaticBox01, wxHORIZONTAL);
  itemBoxSizer05->Add(itemStaticBoxSizer01, 0, wxEXPAND | wxALL, border_size);
  wxFlexGridSizer *itemFlexGridSizer03 = new wxFlexGridSizer(2);
  itemFlexGridSizer03->AddGrowableCol(1);
  itemStaticBoxSizer01->Add(itemFlexGridSizer03, 1, wxEXPAND | wxALL, 0);

  wxStaticText *itemStaticText04 =
      new wxStaticText(itemPanelNotebook02, wxID_ANY, _("Title:"),
                       wxDefaultPosition, wxDefaultSize, 0);
  itemFlexGridSizer03->Add(itemStaticText04, 0, wxEXPAND | wxALL, border_size);
  m_pFontPickerTitle =
      new wxFontPickerCtrl(itemPanelNotebook02, wxID_ANY, g_USFontTitle,
                           wxDefaultPosition, wxDefaultSize);
  itemFlexGridSizer03->Add(m_pFontPickerTitle, 0, wxALIGN_RIGHT | wxALL, 0);

  wxStaticText *itemStaticText05 =
      new wxStaticText(itemPanelNotebook02, wxID_ANY, _("Data:"),
                       wxDefaultPosition, wxDefaultSize, 0);
  itemFlexGridSizer03->Add(itemStaticText05, 0, wxEXPAND | wxALL, border_size);
  m_pFontPickerData =
      new wxFontPickerCtrl(itemPanelNotebook02, wxID_ANY, g_USFontData,
                           wxDefaultPosition, wxDefaultSize);
  itemFlexGridSizer03->Add(m_pFontPickerData, 0, wxALIGN_RIGHT | wxALL, 0);

  wxStaticText *itemStaticText06 =
      new wxStaticText(itemPanelNotebook02, wxID_ANY, _("Label:"),
                       wxDefaultPosition, wxDefaultSize, 0);
  itemFlexGridSizer03->Add(itemStaticText06, 0, wxEXPAND | wxALL, border_size);
  m_pFontPickerLabel =
      new wxFontPickerCtrl(itemPanelNotebook02, wxID_ANY, g_USFontLabel,
                           wxDefaultPosition, wxDefaultSize);
  itemFlexGridSizer03->Add(m_pFontPickerLabel, 0, wxALIGN_RIGHT | wxALL, 0);

  wxStaticText *itemStaticText07 =
      new wxStaticText(itemPanelNotebook02, wxID_ANY, _("Small:"),
                       wxDefaultPosition, wxDefaultSize, 0);
  itemFlexGridSizer03->Add(itemStaticText07, 0, wxEXPAND | wxALL, border_size);
  m_pFontPickerSmall =
      new wxFontPickerCtrl(itemPanelNotebook02, wxID_ANY, g_USFontSmall,
                           wxDefaultPosition, wxDefaultSize);
  itemFlexGridSizer03->Add(m_pFontPickerSmall, 0, wxALIGN_RIGHT | wxALL, 0);
  //      wxColourPickerCtrl

  wxStaticText* itemStaticText80 =
      new wxStaticText(itemPanelNotebook02, wxID_ANY, _("Reset:"),
          wxDefaultPosition, wxDefaultSize, 0);
  itemFlexGridSizer03->Add(itemStaticText80, 0, wxEXPAND | wxALL, border_size);

  m_pButtondefaultFont = new wxButton(itemPanelNotebook02, wxID_ANY, _("Set dashboard default fonts"),
      wxDefaultPosition, wxSize(-1, -1));
  itemFlexGridSizer03->Add(m_pButtondefaultFont, 0, wxALIGN_RIGHT | wxALL, 0);
  m_pButtondefaultFont->Connect(
      wxEVT_COMMAND_BUTTON_CLICKED,
      wxCommandEventHandler(DashboardPreferencesDialog::OnDashboarddefaultFont), NULL,
      this);

  wxStaticBox *itemStaticBox04 = new wxStaticBox(itemPanelNotebook02, wxID_ANY,
                                                 _("Units, Ranges, Formats"));
  wxStaticBoxSizer *itemStaticBoxSizer04 =
      new wxStaticBoxSizer(itemStaticBox04, wxHORIZONTAL);
  itemBoxSizer05->Add(itemStaticBoxSizer04, 0, wxEXPAND | wxALL, border_size);
  wxFlexGridSizer *itemFlexGridSizer04 = new wxFlexGridSizer(2);
  itemFlexGridSizer04->AddGrowableCol(1);
  itemStaticBoxSizer04->Add(itemFlexGridSizer04, 1, wxEXPAND | wxALL, 0);
  wxStaticText *itemStaticText08 = new wxStaticText(
      itemPanelNotebook02, wxID_ANY, _("Speedometer max value:"),
      wxDefaultPosition, wxDefaultSize, 0);
  itemFlexGridSizer04->Add(itemStaticText08, 0, wxEXPAND | wxALL, border_size);
  m_pSpinSpeedMax = new wxSpinCtrl(itemPanelNotebook02, wxID_ANY, wxEmptyString,
                                   wxDefaultPosition, wxDefaultSize,
                                   wxSP_ARROW_KEYS, 10, 100, g_iDashSpeedMax);
  itemFlexGridSizer04->Add(m_pSpinSpeedMax, 0, wxALIGN_RIGHT | wxALL, 0);

  wxStaticText *itemStaticText10 = new wxStaticText(
      itemPanelNotebook02, wxID_ANY, _("Speed Over Ground Damping Factor:"),
      wxDefaultPosition, wxDefaultSize, 0);
  itemFlexGridSizer04->Add(itemStaticText10, 0, wxEXPAND | wxALL, border_size);
  m_pSpinSOGDamp = new wxSpinCtrl(itemPanelNotebook02, wxID_ANY, wxEmptyString,
                                  wxDefaultPosition, wxDefaultSize,
                                  wxSP_ARROW_KEYS, 0, 100, g_iDashSOGDamp);
  itemFlexGridSizer04->Add(m_pSpinSOGDamp, 0, wxALIGN_RIGHT | wxALL, 0);

  wxStaticText *itemStaticText11 =
      new wxStaticText(itemPanelNotebook02, wxID_ANY, _("COG Damping Factor:"),
                       wxDefaultPosition, wxDefaultSize, 0);
  itemFlexGridSizer04->Add(itemStaticText11, 0, wxEXPAND | wxALL, border_size);
  m_pSpinCOGDamp = new wxSpinCtrl(itemPanelNotebook02, wxID_ANY, wxEmptyString,
                                  wxDefaultPosition, wxDefaultSize,
                                  wxSP_ARROW_KEYS, 0, 100, g_iDashCOGDamp);
  itemFlexGridSizer04->Add(m_pSpinCOGDamp, 0, wxALIGN_RIGHT | wxALL, 0);

  wxStaticText *itemStaticText12 = new wxStaticText(
      itemPanelNotebook02, wxID_ANY, _("Local Time Offset From UTC:"),
      wxDefaultPosition, wxDefaultSize, 0);
  itemFlexGridSizer04->Add(itemStaticText12, 0, wxEXPAND | wxALL, border_size);
  wxString m_UTCOffsetChoices[] = {
      _T( "-12:00" ), _T( "-11:30" ), _T( "-11:00" ), _T( "-10:30" ),
      _T( "-10:00" ), _T( "-09:30" ), _T( "-09:00" ), _T( "-08:30" ),
      _T( "-08:00" ), _T( "-07:30" ), _T( "-07:00" ), _T( "-06:30" ),
      _T( "-06:00" ), _T( "-05:30" ), _T( "-05:00" ), _T( "-04:30" ),
      _T( "-04:00" ), _T( "-03:30" ), _T( "-03:00" ), _T( "-02:30" ),
      _T( "-02:00" ), _T( "-01:30" ), _T( "-01:00" ), _T( "-00:30" ),
      _T( " 00:00" ), _T( " 00:30" ), _T( " 01:00" ), _T( " 01:30" ),
      _T( " 02:00" ), _T( " 02:30" ), _T( " 03:00" ), _T( " 03:30" ),
      _T( " 04:00" ), _T( " 04:30" ), _T( " 05:00" ), _T( " 05:30" ),
      _T( " 06:00" ), _T( " 06:30" ), _T( " 07:00" ), _T( " 07:30" ),
      _T( " 08:00" ), _T( " 08:30" ), _T( " 09:00" ), _T( " 09:30" ),
      _T( " 10:00" ), _T( " 10:30" ), _T( " 11:00" ), _T( " 11:30" ),
      _T( " 12:00" )};
  int m_UTCOffsetNChoices = sizeof(m_UTCOffsetChoices) / sizeof(wxString);
  m_pChoiceUTCOffset =
      new wxChoice(itemPanelNotebook02, wxID_ANY, wxDefaultPosition,
                   wxDefaultSize, m_UTCOffsetNChoices, m_UTCOffsetChoices, 0);
  m_pChoiceUTCOffset->SetSelection(g_iUTCOffset + 24);
  itemFlexGridSizer04->Add(m_pChoiceUTCOffset, 0, wxALIGN_RIGHT | wxALL, 0);

  wxStaticText *itemStaticText09 =
      new wxStaticText(itemPanelNotebook02, wxID_ANY, _("Boat speed units:"),
                       wxDefaultPosition, wxDefaultSize, 0);
  itemFlexGridSizer04->Add(itemStaticText09, 0, wxEXPAND | wxALL, border_size);
  wxString m_SpeedUnitChoices[] = {_("Honor OpenCPN settings"), _("Kts"),
                                   _("mph"), _("km/h"), _("m/s")};
  int m_SpeedUnitNChoices = sizeof(m_SpeedUnitChoices) / sizeof(wxString);
  wxSize szSpeedUnit = wxDefaultSize;
  m_pChoiceSpeedUnit =
      new wxChoice(itemPanelNotebook02, wxID_ANY, wxDefaultPosition,
                   szSpeedUnit, m_SpeedUnitNChoices, m_SpeedUnitChoices, 0);
  for (auto const &iUnit : m_SpeedUnitChoices) {
    szSpeedUnit.IncTo(m_pChoiceSpeedUnit->GetTextExtent(iUnit));
  }
  m_pChoiceSpeedUnit->SetSize(szSpeedUnit);
  m_pChoiceSpeedUnit->SetSelection(g_iDashSpeedUnit + 1);
  itemFlexGridSizer04->Add(m_pChoiceSpeedUnit, 0, wxALIGN_RIGHT | wxALL, 0);

  wxStaticText *itemStaticTextDepthU =
      new wxStaticText(itemPanelNotebook02, wxID_ANY, _("Depth units:"),
                       wxDefaultPosition, wxDefaultSize, 0);
  itemFlexGridSizer04->Add(itemStaticTextDepthU, 0, wxEXPAND | wxALL,
                           border_size);
  wxString m_DepthUnitChoices[] = {_("Meters"), _("Feet"), _("Fathoms"),
                                   _("Inches"), _("Centimeters")};
  int m_DepthUnitNChoices = sizeof(m_DepthUnitChoices) / sizeof(wxString);
  wxSize szDepthUnit = wxDefaultSize;
  m_pChoiceDepthUnit =
      new wxChoice(itemPanelNotebook02, wxID_ANY, wxDefaultPosition,
                   szDepthUnit, m_DepthUnitNChoices, m_DepthUnitChoices, 0);
  for (auto const &iUnit : m_DepthUnitChoices) {
    szDepthUnit.IncTo(m_pChoiceDepthUnit->GetTextExtent(iUnit));
  }
  m_pChoiceDepthUnit->SetSize(szDepthUnit);
  m_pChoiceDepthUnit->SetSelection(g_iDashDepthUnit - 3);
  itemFlexGridSizer04->Add(m_pChoiceDepthUnit, 0, wxALIGN_RIGHT | wxALL, 0);
  wxString dMess = wxString::Format(_("Depth Offset (%s):"),
                                    m_DepthUnitChoices[g_iDashDepthUnit - 3]);
  wxStaticText *itemStaticDepthO =
      new wxStaticText(itemPanelNotebook02, wxID_ANY, dMess, wxDefaultPosition,
                       wxDefaultSize, 0);
  double DepthOffset;
  switch (g_iDashDepthUnit - 3) {
    case 1:
      DepthOffset = g_dDashDBTOffset * 3.2808399;
      break;
    case 2:
      DepthOffset = g_dDashDBTOffset * 0.54680665;
      break;
    case 3:
      DepthOffset = g_dDashDBTOffset * 39.3700787;
      break;
    case 4:
      DepthOffset = g_dDashDBTOffset * 100;
      break;
    default:
      DepthOffset = g_dDashDBTOffset;
  }
  itemFlexGridSizer04->Add(itemStaticDepthO, 0, wxEXPAND | wxALL, border_size);
  m_pSpinDBTOffset = new wxSpinCtrlDouble(
      itemPanelNotebook02, wxID_ANY, wxEmptyString, wxDefaultPosition,
      wxDefaultSize, wxSP_ARROW_KEYS, -100, 100, DepthOffset, 0.1);
  itemFlexGridSizer04->Add(m_pSpinDBTOffset, 0, wxALIGN_RIGHT | wxALL, 0);

  wxStaticText *itemStaticText0b =
      new wxStaticText(itemPanelNotebook02, wxID_ANY, _("Distance units:"),
                       wxDefaultPosition, wxDefaultSize, 0);
  itemFlexGridSizer04->Add(itemStaticText0b, 0, wxEXPAND | wxALL, border_size);
  wxString m_DistanceUnitChoices[] = {_("Honor OpenCPN settings"),
                                      _("Nautical miles"), _("Statute miles"),
                                      _("Kilometers"), _("Meters")};
  wxSize szDistanceUnit = wxDefaultSize;
  int m_DistanceUnitNChoices = sizeof(m_DistanceUnitChoices) / sizeof(wxString);
  m_pChoiceDistanceUnit = new wxChoice(
      itemPanelNotebook02, wxID_ANY, wxDefaultPosition, szDistanceUnit,
      m_DistanceUnitNChoices, m_DistanceUnitChoices, 0);
  for (auto const &iUnit : m_DistanceUnitChoices) {
    szDistanceUnit.IncTo(m_pChoiceDistanceUnit->GetTextExtent(iUnit));
  }
  m_pChoiceDistanceUnit->SetSize(szDistanceUnit);
  m_pChoiceDistanceUnit->SetSelection(g_iDashDistanceUnit + 1);
  itemFlexGridSizer04->Add(m_pChoiceDistanceUnit, 0, wxALIGN_RIGHT | wxALL, 0);

  wxStaticText *itemStaticText0a =
      new wxStaticText(itemPanelNotebook02, wxID_ANY, _("Wind speed units:"),
                       wxDefaultPosition, wxDefaultSize, 0);
  itemFlexGridSizer04->Add(itemStaticText0a, 0, wxEXPAND | wxALL, border_size);
  wxString m_WSpeedUnitChoices[] = {_("Kts"), _("mph"), _("km/h"), _("m/s")};
  int m_WSpeedUnitNChoices = sizeof(m_WSpeedUnitChoices) / sizeof(wxString);
  wxSize szWSpeedUnit = wxDefaultSize;
  m_pChoiceWindSpeedUnit =
      new wxChoice(itemPanelNotebook02, wxID_ANY, wxDefaultPosition,
                   szWSpeedUnit, m_WSpeedUnitNChoices, m_WSpeedUnitChoices, 0);
  for (auto const &iUnit : m_WSpeedUnitChoices) {
    szWSpeedUnit.IncTo(m_pChoiceWindSpeedUnit->GetTextExtent(iUnit));
  }
  m_pChoiceWindSpeedUnit->SetSize(szWSpeedUnit);
  m_pChoiceWindSpeedUnit->SetSelection(g_iDashWindSpeedUnit);
  itemFlexGridSizer04->Add(m_pChoiceWindSpeedUnit, 0, wxALIGN_RIGHT | wxALL, 0);

  wxStaticText *itemStaticText0c =
      new wxStaticText(itemPanelNotebook02, wxID_ANY, _("Temperature units:"),
                       wxDefaultPosition, wxDefaultSize, 0);
  itemFlexGridSizer04->Add(itemStaticText0c, 0, wxEXPAND | wxALL, border_size);
  wxString m_TempUnitChoices[] = {_("Celsius"), _("Fahrenheit"), _("Kelvin")};
  int m_TempUnitNChoices = sizeof(m_TempUnitChoices) / sizeof(wxString);
  wxSize szTempUnit = wxDefaultSize;
  m_pChoiceTempUnit =
      new wxChoice(itemPanelNotebook02, wxID_ANY, wxDefaultPosition, szTempUnit,
                   m_TempUnitNChoices, m_TempUnitChoices, 0);
  for (auto const &iUnit : m_TempUnitChoices) {
    szTempUnit.IncTo(m_pChoiceTempUnit->GetTextExtent(iUnit));
  }
  m_pChoiceTempUnit->SetSize(szTempUnit);
  m_pChoiceTempUnit->SetSelection(g_iDashTempUnit);
  itemFlexGridSizer04->Add(m_pChoiceTempUnit, 0, wxALIGN_RIGHT | wxALL, 0);

  m_pUseTrueWinddata = new wxCheckBox(
      itemPanelNotebook02, wxID_ANY,
      _("Use N2K & SignalK true wind data over ground.\n(Instead of through water)"));
  m_pUseTrueWinddata->SetValue(g_bDBtrueWindGround);
  itemFlexGridSizer04->Add(m_pUseTrueWinddata, 1, wxALIGN_LEFT, border_size);


  curSel = -1;
  for (size_t i = 0; i < m_Config.GetCount(); i++) {
    m_pListCtrlDashboards->InsertItem(i, 0);
    // Using data to store m_Config index for managing deletes
    m_pListCtrlDashboards->SetItemData(i, i);
  }
  m_pListCtrlDashboards->SetColumnWidth(0, wxLIST_AUTOSIZE);

  m_pListCtrlDashboards->SetItemState(0, wxLIST_STATE_SELECTED,
                                      wxLIST_STATE_SELECTED);
  curSel = 0;

  UpdateDashboardButtonsState();
  UpdateButtonsState();

  //SetMinSize(wxSize(400, -1));

  Fit();

  // Constrain size on small displays
  SetMaxSize(wxSize(display_width, display_height));

  wxSize canvas_size = GetOCPNCanvasWindow()->GetSize();
  if(display_height < 600){
    if(g_dashPrefWidth > 0 && g_dashPrefHeight > 0)
      SetSize(wxSize(g_dashPrefWidth, g_dashPrefHeight));
    else
      SetSize(wxSize(canvas_size.x * 8/10, canvas_size.y * 8 / 10));
  }
  else {
    if(g_dashPrefWidth > 0 && g_dashPrefHeight > 0)
      SetSize(wxSize(g_dashPrefWidth, g_dashPrefHeight));
    else
      SetSize(wxSize(canvas_size.x * 3 /4, canvas_size.y * 8 / 10));
  }

  Layout();
  CentreOnScreen();

}

void DashboardPreferencesDialog::RecalculateSize(void) {
#ifdef __OCPN__ANDROID__
  wxSize esize;
  esize.x = GetCharWidth() * 110;
  esize.y = GetCharHeight() * 40;

  wxSize dsize = GetOCPNCanvasWindow()->GetClientSize();
  esize.y = wxMin(esize.y, dsize.y - (3 * GetCharHeight()));
  esize.x = wxMin(esize.x, dsize.x - (3 * GetCharHeight()));
  SetSize(esize);

  CentreOnScreen();
#endif
}

void DashboardPreferencesDialog::OnCloseDialog(wxCloseEvent &event) {
  g_dashPrefWidth = GetSize().x;
  g_dashPrefHeight = GetSize().y;
  SaveDashboardConfig();
  event.Skip();
}

void DashboardPreferencesDialog::SaveDashboardConfig() {
  g_iDashSpeedMax = m_pSpinSpeedMax->GetValue();
  g_iDashCOGDamp = m_pSpinCOGDamp->GetValue();
  g_iDashSOGDamp = m_pSpinSOGDamp->GetValue();
  g_iUTCOffset = m_pChoiceUTCOffset->GetSelection() - 24;
  g_iDashSpeedUnit = m_pChoiceSpeedUnit->GetSelection() - 1;
  double DashDBTOffset = m_pSpinDBTOffset->GetValue();
  switch (g_iDashDepthUnit - 3) {
    case 1:
      g_dDashDBTOffset = DashDBTOffset / 3.2808399;
      break;
    case 2:
      g_dDashDBTOffset = DashDBTOffset / 0.54680665;
      break;
    case 3:
      g_dDashDBTOffset = DashDBTOffset / 39.3700787;
      break;
    case 4:
      g_dDashDBTOffset = DashDBTOffset / 100;
      break;
    default:
      g_dDashDBTOffset = DashDBTOffset;
  }
  g_iDashDepthUnit = m_pChoiceDepthUnit->GetSelection() + 3;
  g_iDashDistanceUnit = m_pChoiceDistanceUnit->GetSelection() - 1;
  g_iDashWindSpeedUnit = m_pChoiceWindSpeedUnit->GetSelection();
  g_bDBtrueWindGround = m_pUseTrueWinddata->GetValue();
  g_iDashTempUnit = m_pChoiceTempUnit->GetSelection();
  if (curSel != -1) {
    DashboardWindowContainer *cont = m_Config.Item(curSel);
    cont->m_bIsVisible = m_pCheckBoxIsVisible->IsChecked();
    cont->m_sCaption = m_pTextCtrlCaption->GetValue();
    cont->m_sOrientation =
        m_pChoiceOrientation->GetSelection() == 0 ? _T("V") : _T("H");
    cont->m_aInstrumentList.Clear();
    for (int i = 0; i < m_pListCtrlInstruments->GetItemCount(); i++)
      cont->m_aInstrumentList.Add((int)m_pListCtrlInstruments->GetItemData(i));
  }
}

void DashboardPreferencesDialog::OnDashboardSelected(wxListEvent &event) {
  // save changes
  SaveDashboardConfig();
  UpdateDashboardButtonsState();
}

void DashboardPreferencesDialog::UpdateDashboardButtonsState() {
  long item = -1;
  item = m_pListCtrlDashboards->GetNextItem(item, wxLIST_NEXT_ALL,
                                            wxLIST_STATE_SELECTED);
  bool enable = (item != -1);

  //  Disable the Dashboard Delete button if the parent(Dashboard) of this
  //  dialog is selected.
  bool delete_enable = enable;
  if (item != -1) {
    int sel = m_pListCtrlDashboards->GetItemData(item);
    DashboardWindowContainer *cont = m_Config.Item(sel);
    DashboardWindow *dash_sel = cont->m_pDashboardWindow;
    if (dash_sel == GetParent()) delete_enable = false;
  }
  m_pButtonDeleteDashboard->Enable(delete_enable);

  m_pPanelDashboard->Enable(enable);

  if (item != -1) {
    curSel = m_pListCtrlDashboards->GetItemData(item);
    DashboardWindowContainer *cont = m_Config.Item(curSel);
    m_pCheckBoxIsVisible->SetValue(cont->m_bIsVisible);
    m_pTextCtrlCaption->SetValue(cont->m_sCaption);
    m_pChoiceOrientation->SetSelection(cont->m_sOrientation == _T("V") ? 0 : 1);
    m_pListCtrlInstruments->DeleteAllItems();
    for (size_t i = 0; i < cont->m_aInstrumentList.GetCount(); i++) {
      wxListItem item;
      getListItemForInstrument(item, cont->m_aInstrumentList.Item(i));
      item.SetId(m_pListCtrlInstruments->GetItemCount());
      m_pListCtrlInstruments->InsertItem(item);
    }

    m_pListCtrlInstruments->SetColumnWidth(0, wxLIST_AUTOSIZE);
  } else {
    curSel = -1;
    m_pCheckBoxIsVisible->SetValue(false);
    m_pTextCtrlCaption->SetValue(_T(""));
    m_pChoiceOrientation->SetSelection(0);
    m_pListCtrlInstruments->DeleteAllItems();
  }
  //      UpdateButtonsState();
}

void DashboardPreferencesDialog::OnDashboarddefaultFont(wxCommandEvent& event){

    m_pFontPickerTitle->SetSelectedFont(wxFont(10, wxFONTFAMILY_SWISS, wxFONTSTYLE_ITALIC, wxFONTWEIGHT_NORMAL));
    m_pFontPickerTitle->SetSelectedColour(wxColour(0, 0, 0));
    m_pFontPickerData->SetSelectedFont(wxFont(14, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    m_pFontPickerData->SetSelectedColour(wxColour(0, 0, 0));
    m_pFontPickerLabel->SetSelectedFont(wxFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    m_pFontPickerLabel->SetSelectedColour(wxColour(0, 0, 0));
    m_pFontPickerSmall->SetSelectedFont(wxFont(8, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    m_pFontPickerSmall->SetSelectedColour(wxColour(0, 0, 0));    
    double scaler = 1.0;
    if (OCPN_GetWinDIPScaleFactor() < 1.0)
        scaler = 1.0 + OCPN_GetWinDIPScaleFactor() / 4;
    scaler = wxMax(1.0, scaler);

    g_USFontTitle = *(m_pFontPickerTitle->GetFontData());
    g_FontTitle = *g_pUSFontTitle;
    g_FontTitle.SetChosenFont(g_pUSFontTitle->GetChosenFont().Scaled(scaler));
    g_FontTitle.SetColour(g_pUSFontTitle->GetColour());
    g_USFontTitle = *g_pUSFontTitle;

    g_USFontData = *(m_pFontPickerData->GetFontData());
    g_FontData = *g_pUSFontData;
    g_FontData.SetChosenFont(g_pUSFontData->GetChosenFont().Scaled(scaler));
    g_FontData.SetColour(g_pUSFontData->GetColour());
    g_USFontData = *g_pUSFontData;

    g_USFontLabel = *(m_pFontPickerLabel->GetFontData());
    g_FontLabel = *g_pUSFontLabel;
    g_FontLabel.SetChosenFont(g_pUSFontLabel->GetChosenFont().Scaled(scaler));
    g_FontLabel.SetColour(g_pUSFontLabel->GetColour());
    g_USFontLabel = *g_pUSFontLabel;

    g_USFontSmall = *(m_pFontPickerSmall->GetFontData());
    g_FontSmall = *g_pUSFontSmall;
    g_FontSmall.SetChosenFont(g_pUSFontSmall->GetChosenFont().Scaled(scaler));
    g_FontSmall.SetColour(g_pUSFontSmall->GetColour());
    g_USFontSmall = *g_pUSFontSmall;
}

void DashboardPreferencesDialog::OnDashboardAdd(wxCommandEvent &event) {
  int idx = m_pListCtrlDashboards->GetItemCount();
  m_pListCtrlDashboards->InsertItem(idx, 0);
  // Data is index in m_Config
  m_pListCtrlDashboards->SetItemData(idx, m_Config.GetCount());
  wxArrayInt ar;
  wxArrayOfInstrumentProperties Property;
  DashboardWindowContainer *dwc = new DashboardWindowContainer(
      NULL, MakeName(), _("Dashboard"), _T("V"), ar, Property);
  dwc->m_bIsVisible = true;
  m_Config.Add(dwc);
}

void DashboardPreferencesDialog::OnDashboardDelete(wxCommandEvent &event) {
  long itemID = -1;
  itemID = m_pListCtrlDashboards->GetNextItem(itemID, wxLIST_NEXT_ALL,
                                              wxLIST_STATE_SELECTED);

  int idx = m_pListCtrlDashboards->GetItemData(itemID);
  m_pListCtrlDashboards->DeleteItem(itemID);
  m_Config.Item(idx)->m_bIsDeleted = true;
  UpdateDashboardButtonsState();
}

void DashboardPreferencesDialog::OnInstrumentSelected(wxListEvent &event) {
  UpdateButtonsState();
}

void DashboardPreferencesDialog::UpdateButtonsState() {
  long item = -1;
  item = m_pListCtrlInstruments->GetNextItem(item, wxLIST_NEXT_ALL,
                                             wxLIST_STATE_SELECTED);
  bool enable = (item != -1);

  m_pButtonDelete->Enable(enable);
  m_pButtonEdit->Enable(enable); // TODO: Properties ... done Bernd Cirotzki
  m_pButtonUp->Enable(item > 0);
  m_pButtonDown->Enable(item != -1 &&
                        item < m_pListCtrlInstruments->GetItemCount() - 1);
}

void DashboardPreferencesDialog::OnInstrumentAdd(wxCommandEvent &event) {
  AddInstrumentDlg pdlg((wxWindow *)event.GetEventObject(), wxID_ANY);

#ifdef __OCPN__ANDROID__
  wxFont *pF = OCPNGetFont(_T("Dialog"), 0);
  pdlg.SetFont(*pF);

  wxSize esize;
  esize.x = GetCharWidth() * 110;
  esize.y = GetCharHeight() * 40;

  wxSize dsize = GetOCPNCanvasWindow()->GetClientSize();
  esize.y = wxMin(esize.y, dsize.y - (3 * GetCharHeight()));
  esize.x = wxMin(esize.x, dsize.x - (3 * GetCharHeight()));
  pdlg.SetSize(esize);

  pdlg.CentreOnScreen();
#endif
  pdlg.ShowModal();
  if (pdlg.GetReturnCode() == wxID_OK) {
    wxListItem item;
    getListItemForInstrument(item, pdlg.GetInstrumentAdded());
    item.SetId(m_pListCtrlInstruments->GetItemCount());
    m_pListCtrlInstruments->InsertItem(item);
    m_pListCtrlInstruments->SetColumnWidth(0, wxLIST_AUTOSIZE);
    UpdateButtonsState();
  }
}

void DashboardPreferencesDialog::OnInstrumentDelete(wxCommandEvent &event) {
  long itemIDWindow = -1;
  itemIDWindow = m_pListCtrlDashboards->GetNextItem(itemIDWindow, wxLIST_NEXT_ALL,
                                                    wxLIST_STATE_SELECTED);
  long itemID = -1;
  itemID = m_pListCtrlInstruments->GetNextItem(itemID, wxLIST_NEXT_ALL,
                                               wxLIST_STATE_SELECTED);
  DashboardWindowContainer* cont = m_Config.Item(m_pListCtrlDashboards->GetItemData(itemIDWindow));
  InstrumentProperties *InstDel = NULL;
  if (cont)
  {
      InstrumentProperties *Inst = NULL;
      for (unsigned int i = 0; i < (cont->m_aInstrumentPropertyList.GetCount()); i++)
      {
          Inst = cont->m_aInstrumentPropertyList.Item(i);
          if (Inst->m_aInstrument == (int)m_pListCtrlInstruments->GetItemData(itemID) &&
              Inst->m_Listplace == itemID)
          {
              cont->m_aInstrumentPropertyList.Remove(Inst);
              InstDel = Inst;
              break;
          }
          else
          {
              if (Inst->m_Listplace > itemID)
                  Inst->m_Listplace--;
          }
      }
  }
  m_pListCtrlInstruments->DeleteItem(itemID);
  if (InstDel)
  {
      cont->m_pDashboardWindow->SetInstrumentList(cont->m_aInstrumentList, &(cont->m_aInstrumentPropertyList));      
      delete InstDel;
  }
  UpdateButtonsState();
}

void DashboardPreferencesDialog::OnInstrumentEdit(wxCommandEvent &event) {
  // TODO: Instument options
  //  m_Config = Arrayofdashboardwindows.
    long itemIDWindow = -1;
    itemIDWindow = m_pListCtrlDashboards->GetNextItem(itemIDWindow, wxLIST_NEXT_ALL,
        wxLIST_STATE_SELECTED);
    long itemID = -1;
    itemID = m_pListCtrlInstruments->GetNextItem(itemID, wxLIST_NEXT_ALL,
        wxLIST_STATE_SELECTED);
    //m_Config.
    // curSel = m_pListCtrlDashboards->GetItemData(itemWindow);
    // DashboardWindowContainer *cont = m_Config.Item(curSel);
    // if (cont) ....
    DashboardWindowContainer* cont = m_Config.Item(m_pListCtrlDashboards->GetItemData(itemIDWindow));
    if (!cont) return;
    InstrumentProperties* Inst = NULL;
    for (unsigned int i = 0; i < (cont->m_aInstrumentPropertyList.GetCount()); i++)
    {
        Inst = cont->m_aInstrumentPropertyList.Item(i); // m_pListCtrlInstruments->GetItemData(itemID)
        if (Inst->m_aInstrument == (int)m_pListCtrlInstruments->GetItemData(itemID)) // Is for right Instrumenttype.
        {
            if (Inst->m_Listplace == itemID)
                break;
        }
        Inst = NULL;
    }
    if (!Inst)
    {
        Inst = new InstrumentProperties(m_pListCtrlInstruments->GetItemData(itemID), itemID);
        cont->m_aInstrumentPropertyList.Add(Inst);
    }
    EditDialog *Edit = new EditDialog(this, *Inst, wxID_ANY);
    Edit->Fit();
    bool DefaultFont = false;
    if (Edit->ShowModal() == wxID_OK) {
        DefaultFont = true;
        double scaler = 1.0;
        if (OCPN_GetWinDIPScaleFactor() < 1.0)
            scaler = 1.0 + OCPN_GetWinDIPScaleFactor() / 4;
        scaler = wxMax(1.0, scaler);
        if (Edit->m_fontPicker2->GetFont().Scaled(scaler) != g_FontTitle.GetChosenFont() || Edit->m_fontPicker2->GetSelectedColour() != g_FontTitle.GetColour())
            DefaultFont = false;
        if (Edit->m_fontPicker4->GetFont().Scaled(scaler) != g_FontData.GetChosenFont() || Edit->m_fontPicker4->GetSelectedColour() != g_FontData.GetColour())
            DefaultFont = false;
        if (Edit->m_fontPicker5->GetFont().Scaled(scaler) != g_FontLabel.GetChosenFont() || Edit->m_fontPicker5->GetSelectedColour() != g_FontLabel.GetColour())
            DefaultFont = false;
        if (Edit->m_fontPicker6->GetFont().Scaled(scaler) != g_FontSmall.GetChosenFont() || Edit->m_fontPicker6->GetSelectedColour() != g_FontSmall.GetColour())
            DefaultFont = false;
        wxColour dummy;
        GetGlobalColor(_T("DASHL"), &dummy);
        if (Edit->m_colourPicker1->GetColour() != dummy) DefaultFont = false;
        GetGlobalColor(_T("DASHB"), &dummy);
        if (Edit->m_colourPicker2->GetColour() != dummy) DefaultFont = false;
        GetGlobalColor(_T("DASHN"), &dummy);
        if (Edit->m_colourPicker3->GetColour() != dummy) DefaultFont = false;
        GetGlobalColor(_T("BLUE3"), &dummy);
        if (Edit->m_colourPicker4->GetColour() != dummy) DefaultFont = false;
        if (DefaultFont)
            cont->m_aInstrumentPropertyList.Remove(Inst);
        else
        {                      
            Inst->m_TitelFont = *(Edit->m_fontPicker2->GetFontData());
            Inst->m_TitelFont.SetChosenFont(Inst->m_TitelFont.GetChosenFont().Scaled(scaler));
            Inst->m_DataFont = *(Edit->m_fontPicker4->GetFontData());
            Inst->m_DataFont.SetChosenFont(Inst->m_DataFont.GetChosenFont().Scaled(scaler));
            Inst->m_LabelFont = *(Edit->m_fontPicker5->GetFontData());
            Inst->m_LabelFont.SetChosenFont(Inst->m_LabelFont.GetChosenFont().Scaled(scaler));
            Inst->m_SmallFont = *(Edit->m_fontPicker6->GetFontData());
            Inst->m_SmallFont.SetChosenFont(Inst->m_SmallFont.GetChosenFont().Scaled(scaler));
            Inst->m_DataBackgroundColour = Edit->m_colourPicker2->GetColour();
            Inst->m_TitlelBackgroundColour = Edit->m_colourPicker1->GetColour();
            Inst->m_Arrow_First_Colour = Edit->m_colourPicker3->GetColour();
            Inst->m_Arrow_Second_Colour = Edit->m_colourPicker4->GetColour();
        }
    }
    delete Edit;    
    cont->m_pDashboardWindow->SetInstrumentList(cont->m_aInstrumentList, &(cont->m_aInstrumentPropertyList));
    if (DefaultFont) delete Inst;
}

void DashboardPreferencesDialog::OnInstrumentUp(wxCommandEvent &event) {
  long itemIDWindow = -1;
  itemIDWindow = m_pListCtrlDashboards->GetNextItem(itemIDWindow, wxLIST_NEXT_ALL,
                                                    wxLIST_STATE_SELECTED);
  long itemID = -1;
  itemID = m_pListCtrlInstruments->GetNextItem(itemID, wxLIST_NEXT_ALL,
                                               wxLIST_STATE_SELECTED);
  wxListItem item;
  item.SetId(itemID);
  item.SetMask(wxLIST_MASK_TEXT | wxLIST_MASK_IMAGE | wxLIST_MASK_DATA);
  m_pListCtrlInstruments->GetItem(item);
  item.SetId(itemID - 1);
  // item.SetImage(0);           // image 0, by default  
  // Now see if the Old itemId has an own Fontdata
  DashboardWindowContainer* cont = m_Config.Item(m_pListCtrlDashboards->GetItemData(itemIDWindow));
  if (cont)
  {
      InstrumentProperties* Inst = NULL;
      for (unsigned int i = 0; i < (cont->m_aInstrumentPropertyList.GetCount()); i++)
      {
          Inst = cont->m_aInstrumentPropertyList.Item(i);
          if (Inst->m_Listplace  == (itemID - 1))
              Inst->m_Listplace = itemID;
          if (Inst->m_aInstrument == (int)m_pListCtrlInstruments->GetItemData(itemID) &&
              Inst->m_Listplace == itemID)
          {
              cont->m_aInstrumentPropertyList.Item(i)->m_Listplace = itemID - 1;
          }          
      }
  }
  m_pListCtrlInstruments->DeleteItem(itemID);
  m_pListCtrlInstruments->InsertItem(item);
  for (int i = 0; i < m_pListCtrlInstruments->GetItemCount(); i++)
    m_pListCtrlInstruments->SetItemState(i, 0, wxLIST_STATE_SELECTED);

  m_pListCtrlInstruments->SetItemState(itemID - 1, wxLIST_STATE_SELECTED,
                                       wxLIST_STATE_SELECTED);

  UpdateButtonsState();
}

void DashboardPreferencesDialog::OnInstrumentDown(wxCommandEvent &event) {
  long itemIDWindow = -1;
  itemIDWindow = m_pListCtrlDashboards->GetNextItem(itemIDWindow, wxLIST_NEXT_ALL,
                                                    wxLIST_STATE_SELECTED);
  long itemID = -1;
  itemID = m_pListCtrlInstruments->GetNextItem(itemID, wxLIST_NEXT_ALL,
                                               wxLIST_STATE_SELECTED);

  wxListItem item;
  item.SetId(itemID);
  item.SetMask(wxLIST_MASK_TEXT | wxLIST_MASK_IMAGE | wxLIST_MASK_DATA);
  m_pListCtrlInstruments->GetItem(item);
  item.SetId(itemID + 1);
  // item.SetImage(0);           // image 0, by default  
  // Now see if the Old itemId has an own Fontdata
  DashboardWindowContainer* cont = m_Config.Item(m_pListCtrlDashboards->GetItemData(itemIDWindow));
  if (cont)
  {
      InstrumentProperties* Inst = NULL;
      for (unsigned int i = 0; i < (cont->m_aInstrumentPropertyList.GetCount()); i++)
      {
          Inst = cont->m_aInstrumentPropertyList.Item(i);
          if (Inst->m_Listplace == (itemID + 1) && Inst->m_aInstrument != (int)m_pListCtrlInstruments->GetItemData(itemID))
              Inst->m_Listplace = itemID;
          if (Inst->m_aInstrument == (int)m_pListCtrlInstruments->GetItemData(itemID) &&
              Inst->m_Listplace == itemID)
          {
              cont->m_aInstrumentPropertyList.Item(i)->m_Listplace = itemID + 1;
              break;
          }
      }
  }
  m_pListCtrlInstruments->DeleteItem(itemID);
  m_pListCtrlInstruments->InsertItem(item);
  for (int i = 0; i < m_pListCtrlInstruments->GetItemCount(); i++)
    m_pListCtrlInstruments->SetItemState(i, 0, wxLIST_STATE_SELECTED);

  m_pListCtrlInstruments->SetItemState(itemID + 1, wxLIST_STATE_SELECTED,
                                       wxLIST_STATE_SELECTED);

  UpdateButtonsState();
}

//----------------------------------------------------------------
//
//    Add Instrument Dialog Implementation
//
//----------------------------------------------------------------

AddInstrumentDlg::AddInstrumentDlg(wxWindow *pparent, wxWindowID id)
    : wxDialog(pparent, id, _("Add instrument"), wxDefaultPosition,
               wxDefaultSize, wxDEFAULT_DIALOG_STYLE) {
  wxBoxSizer *itemBoxSizer01 = new wxBoxSizer(wxVERTICAL);
  SetSizer(itemBoxSizer01);
  wxStaticText *itemStaticText01 =
      new wxStaticText(this, wxID_ANY, _("Select instrument to add:"),
                       wxDefaultPosition, wxDefaultSize, 0);
  itemBoxSizer01->Add(itemStaticText01, 0, wxEXPAND | wxALL, 5);

  int instImageRefSize = 20 * GetOCPNGUIToolScaleFactor_PlugIn();

  wxImageList *imglist =
      new wxImageList(instImageRefSize, instImageRefSize, true, 2);

  wxImage inst1 = wxBitmap(*_img_instrument).ConvertToImage();
  wxImage inst1s =
      inst1.Scale(instImageRefSize, instImageRefSize, wxIMAGE_QUALITY_HIGH);
  imglist->Add(wxBitmap(inst1s));

  wxImage dial1 = wxBitmap(*_img_dial).ConvertToImage();
  wxImage dial1s =
      dial1.Scale(instImageRefSize, instImageRefSize, wxIMAGE_QUALITY_HIGH);
  imglist->Add(wxBitmap(dial1s));

  wxSize dsize = GetOCPNCanvasWindow()->GetClientSize();
  int vsize = dsize.y * 50 / 100;

#ifdef __OCPN__ANDROID__
  int dw, dh;
  wxDisplaySize(&dw, &dh);
  vsize = dh * 50 / 100;
#endif

  m_pListCtrlInstruments = new wxListCtrl(
      this, wxID_ANY, wxDefaultPosition, wxSize(-1, vsize /*250, 180 */),
      wxLC_REPORT | wxLC_NO_HEADER | wxLC_SINGLE_SEL | wxLC_SORT_ASCENDING);
  itemBoxSizer01->Add(m_pListCtrlInstruments, 0, wxEXPAND | wxALL, 5);
  m_pListCtrlInstruments->AssignImageList(imglist, wxIMAGE_LIST_SMALL);
  m_pListCtrlInstruments->InsertColumn(0, _("Instruments"));

  wxFont *pF = OCPNGetFont(_T("Dialog"), 0);
  m_pListCtrlInstruments->SetFont(*pF);

#ifdef __OCPN__ANDROID__
  m_pListCtrlInstruments->GetHandle()->setStyleSheet(qtStyleSheet);
/// QScroller::ungrabGesture(m_pListCtrlInstruments->GetHandle());
#endif

  wxStdDialogButtonSizer *DialogButtonSizer =
      CreateStdDialogButtonSizer(wxOK | wxCANCEL);
  itemBoxSizer01->Add(DialogButtonSizer, 0, wxALIGN_RIGHT | wxALL, 5);

  long ident = 0;
  for (unsigned int i = ID_DBP_I_POS; i < ID_DBP_LAST_ENTRY;
       i++) {  // do not reference an instrument, but the last dummy entry in
               // the list
    wxListItem item;
    if (IsObsolete(i)) continue;
    getListItemForInstrument(item, i);
    item.SetId(ident);
    m_pListCtrlInstruments->InsertItem(item);
    id++;
  }

  m_pListCtrlInstruments->SetColumnWidth(0, wxLIST_AUTOSIZE);
  m_pListCtrlInstruments->SetItemState(0, wxLIST_STATE_SELECTED,
                                       wxLIST_STATE_SELECTED);
  Fit();
}

unsigned int AddInstrumentDlg::GetInstrumentAdded() {
  long itemID = -1;
  itemID = m_pListCtrlInstruments->GetNextItem(itemID, wxLIST_NEXT_ALL,
                                               wxLIST_STATE_SELECTED);

  return (int)m_pListCtrlInstruments->GetItemData(itemID);
}

//----------------------------------------------------------------
//
//    Dashboard Window Implementation
//
//----------------------------------------------------------------

// wxWS_EX_VALIDATE_RECURSIVELY required to push events to parents
DashboardWindow::DashboardWindow(wxWindow *pparent, wxWindowID id,
                                 wxAuiManager *auimgr, dashboard_pi *plugin,
                                 int orient, DashboardWindowContainer *mycont)
    : wxWindow(pparent, id, wxDefaultPosition, wxDefaultSize, 0) {
  // wxDialog::Create(pparent, id, _("tileMine"), wxDefaultPosition,
  // wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("Dashboard"));

  m_pauimgr = auimgr;
  m_plugin = plugin;
  m_Container = mycont;
  
  // wx2.9      itemBoxSizer = new wxWrapSizer( orient );
  itemBoxSizer = new wxBoxSizer(orient);
  SetSizer(itemBoxSizer);
  Connect(wxEVT_SIZE, wxSizeEventHandler(DashboardWindow::OnSize), NULL, this);
  Connect(wxEVT_CONTEXT_MENU,
          wxContextMenuEventHandler(DashboardWindow::OnContextMenu), NULL,
          this);
  Connect(wxEVT_COMMAND_MENU_SELECTED,
          wxCommandEventHandler(DashboardWindow::OnContextMenuSelect), NULL,
          this);

#ifdef __OCPN__ANDROID__
  Connect(wxEVT_LEFT_DOWN, wxMouseEventHandler(DashboardWindow::OnMouseEvent));
  Connect(wxEVT_LEFT_UP, wxMouseEventHandler(DashboardWindow::OnMouseEvent));
  Connect(wxEVT_MOTION, wxMouseEventHandler(DashboardWindow::OnMouseEvent));

  GetHandle()->setAttribute(Qt::WA_AcceptTouchEvents);
  GetHandle()->grabGesture(Qt::PinchGesture);
  GetHandle()->grabGesture(Qt::PanGesture);

  Connect(wxEVT_QT_PINCHGESTURE,
          (wxObjectEventFunction)(wxEventFunction)&DashboardWindow::
              OnEvtPinchGesture,
          NULL, this);
  Connect(
      wxEVT_QT_PANGESTURE,
      (wxObjectEventFunction)(wxEventFunction)&DashboardWindow::OnEvtPanGesture,
      NULL, this);
#endif

  Hide();

  m_binResize = false;
  m_binPinch = false;
}

DashboardWindow::~DashboardWindow() {
  for (size_t i = 0; i < m_ArrayOfInstrument.GetCount(); i++) {
    DashboardInstrumentContainer *pdic = m_ArrayOfInstrument.Item(i);
    delete pdic;
  }
}

#ifdef __OCPN__ANDROID__
void DashboardWindow::OnEvtPinchGesture(wxQT_PinchGestureEvent &event) {
  float zoom_gain = 0.3;
  float zoom_val;
  float total_zoom_val;

  if (event.GetScaleFactor() > 1)
    zoom_val = ((event.GetScaleFactor() - 1.0) * zoom_gain) + 1.0;
  else
    zoom_val = 1.0 - ((1.0 - event.GetScaleFactor()) * zoom_gain);

  if (event.GetTotalScaleFactor() > 1)
    total_zoom_val = ((event.GetTotalScaleFactor() - 1.0) * zoom_gain) + 1.0;
  else
    total_zoom_val = 1.0 - ((1.0 - event.GetTotalScaleFactor()) * zoom_gain);

  wxAuiPaneInfo &pane = m_pauimgr->GetPane(this);

  wxSize currentSize = wxSize(pane.floating_size.x, pane.floating_size.y);
  double aRatio = (double)currentSize.y / (double)currentSize.x;

  wxSize par_size = GetOCPNCanvasWindow()->GetClientSize();
  wxPoint par_pos = wxPoint(pane.floating_pos.x, pane.floating_pos.y);

  switch (event.GetState()) {
    case GestureStarted:
      m_binPinch = true;
      break;

    case GestureUpdated:
      currentSize.y *= zoom_val;
      currentSize.x *= zoom_val;

      if ((par_pos.y + currentSize.y) > par_size.y)
        currentSize.y = par_size.y - par_pos.y;

      if ((par_pos.x + currentSize.x) > par_size.x)
        currentSize.x = par_size.x - par_pos.x;

      /// vertical
      currentSize.x = currentSize.y / aRatio;

      currentSize.x = wxMax(currentSize.x, 150);
      currentSize.y = wxMax(currentSize.y, 150);

      pane.FloatingSize(currentSize);
      m_pauimgr->Update();

      break;

    case GestureFinished: {
      if (itemBoxSizer->GetOrientation() == wxVERTICAL) {
        currentSize.y *= total_zoom_val;
        currentSize.x = currentSize.y / aRatio;
      } else {
        currentSize.x *= total_zoom_val;
        currentSize.y = currentSize.x * aRatio;
      }

      //  Bound the resulting size
      if ((par_pos.y + currentSize.y) > par_size.y)
        currentSize.y = par_size.y - par_pos.y;

      if ((par_pos.x + currentSize.x) > par_size.x)
        currentSize.x = par_size.x - par_pos.x;

      // not too small
      currentSize.x = wxMax(currentSize.x, 150);
      currentSize.y = wxMax(currentSize.y, 150);

      //  Try a manual layout of the window, to estimate a good primary size..

      // vertical
      if (itemBoxSizer->GetOrientation() == wxVERTICAL) {
        int total_y = 0;
        for (unsigned int i = 0; i < m_ArrayOfInstrument.size(); i++) {
          DashboardInstrument *inst =
              m_ArrayOfInstrument.Item(i)->m_pInstrument;
          wxSize is =
              inst->GetSize(itemBoxSizer->GetOrientation(), currentSize);
          total_y += is.y;
        }

        currentSize.y = total_y;
      }

      pane.FloatingSize(currentSize);

      // Reshow the window
      for (unsigned int i = 0; i < m_ArrayOfInstrument.size(); i++) {
        DashboardInstrument *inst = m_ArrayOfInstrument.Item(i)->m_pInstrument;
        inst->Show();
      }

      m_pauimgr->Update();

      m_binPinch = false;
      m_binResize = false;

      break;
    }

    case GestureCanceled:
      m_binPinch = false;
      m_binResize = false;
      break;

    default:
      break;
  }
}

void DashboardWindow::OnEvtPanGesture(wxQT_PanGestureEvent &event) {
  if (m_binPinch) return;

  if (m_binResize) return;

  int x = event.GetOffset().x;
  int y = event.GetOffset().y;

  int lx = event.GetLastOffset().x;
  int ly = event.GetLastOffset().y;

  int dx = x - lx;
  int dy = y - ly;

  switch (event.GetState()) {
    case GestureStarted:
      if (m_binPan) break;

      m_binPan = true;
      break;

    case GestureUpdated:
      if (m_binPan) {
        wxSize par_size = GetOCPNCanvasWindow()->GetClientSize();
        wxPoint par_pos_old = ClientToScreen(wxPoint(0, 0));  // GetPosition();

        wxPoint par_pos = par_pos_old;
        par_pos.x += dx;
        par_pos.y += dy;

        par_pos.x = wxMax(par_pos.x, 0);
        par_pos.y = wxMax(par_pos.y, 0);

        wxSize mySize = GetSize();

        if ((par_pos.y + mySize.y) > par_size.y)
          par_pos.y = par_size.y - mySize.y;

        if ((par_pos.x + mySize.x) > par_size.x)
          par_pos.x = par_size.x - mySize.x;

        wxAuiPaneInfo &pane = m_pauimgr->GetPane(this);
        pane.FloatingPosition(par_pos).Float();
        m_pauimgr->Update();
      }
      break;

    case GestureFinished:
      if (m_binPan) {
      }
      m_binPan = false;

      break;

    case GestureCanceled:
      m_binPan = false;
      break;

    default:
      break;
  }
}

void DashboardWindow::OnMouseEvent(wxMouseEvent &event) {
  if (m_binPinch) return;

  if (m_binResize) {
    wxAuiPaneInfo &pane = m_pauimgr->GetPane(this);
    wxSize currentSize = wxSize(pane.floating_size.x, pane.floating_size.y);
    double aRatio = (double)currentSize.y / (double)currentSize.x;

    wxSize par_size = GetOCPNCanvasWindow()->GetClientSize();
    wxPoint par_pos = wxPoint(pane.floating_pos.x, pane.floating_pos.y);

    if (event.LeftDown()) {
      m_resizeStartPoint = event.GetPosition();
      m_resizeStartSize = currentSize;
      m_binResize2 = true;
    }

    if (m_binResize2) {
      if (event.Dragging()) {
        wxPoint p = event.GetPosition();

        wxSize dragSize = m_resizeStartSize;

        dragSize.y += p.y - m_resizeStartPoint.y;
        dragSize.x += p.x - m_resizeStartPoint.x;
        ;

        if ((par_pos.y + dragSize.y) > par_size.y)
          dragSize.y = par_size.y - par_pos.y;

        if ((par_pos.x + dragSize.x) > par_size.x)
          dragSize.x = par_size.x - par_pos.x;

        /// vertical
        // dragSize.x = dragSize.y / aRatio;

        // not too small
        dragSize.x = wxMax(dragSize.x, 150);
        dragSize.y = wxMax(dragSize.y, 150);

        pane.FloatingSize(dragSize);
        m_pauimgr->Update();
      }

      if (event.LeftUp()) {
        wxPoint p = event.GetPosition();

        wxSize dragSize = m_resizeStartSize;

        dragSize.y += p.y - m_resizeStartPoint.y;
        dragSize.x += p.x - m_resizeStartPoint.x;
        ;

        if ((par_pos.y + dragSize.y) > par_size.y)
          dragSize.y = par_size.y - par_pos.y;

        if ((par_pos.x + dragSize.x) > par_size.x)
          dragSize.x = par_size.x - par_pos.x;

        // not too small
        dragSize.x = wxMax(dragSize.x, 150);
        dragSize.y = wxMax(dragSize.y, 150);
        /*
                        for( unsigned int i=0; i<m_ArrayOfInstrument.size(); i++
           ) { DashboardInstrument* inst =
           m_ArrayOfInstrument.Item(i)->m_pInstrument; inst->Show();
                        }
        */
        pane.FloatingSize(dragSize);
        m_pauimgr->Update();

        m_binResize = false;
        m_binResize2 = false;
      }
    }
  }
}
#endif

void DashboardWindow::OnSize(wxSizeEvent &event) {
  event.Skip();
  for (unsigned int i = 0; i < m_ArrayOfInstrument.size(); i++) {
    DashboardInstrument *inst = m_ArrayOfInstrument.Item(i)->m_pInstrument;
    inst->SetMinSize(
        inst->GetSize(itemBoxSizer->GetOrientation(), GetClientSize()));
  }
  Layout();
  Refresh();
}

void DashboardWindow::OnContextMenu(wxContextMenuEvent &event) {
  wxMenu *contextMenu = new wxMenu();

#ifdef __WXQT__
  wxFont *pf = OCPNGetFont(_T("Menu"), 0);

  // add stuff
  wxMenuItem *item1 =
      new wxMenuItem(contextMenu, ID_DASH_PREFS, _("Preferences..."));
  item1->SetFont(*pf);
  contextMenu->Append(item1);

  wxMenuItem *item2 =
      new wxMenuItem(contextMenu, ID_DASH_RESIZE, _("Resize..."));
  item2->SetFont(*pf);
  contextMenu->Append(item2);

#else

  wxAuiPaneInfo &pane = m_pauimgr->GetPane(this);
  if (pane.IsOk() && pane.IsDocked()) {
    contextMenu->Append(ID_DASH_UNDOCK, _("Undock"));
  }
  wxMenuItem *btnVertical =
      contextMenu->AppendRadioItem(ID_DASH_VERTICAL, _("Vertical"));
  btnVertical->Check(itemBoxSizer->GetOrientation() == wxVERTICAL);
  wxMenuItem *btnHorizontal =
      contextMenu->AppendRadioItem(ID_DASH_HORIZONTAL, _("Horizontal"));
  btnHorizontal->Check(itemBoxSizer->GetOrientation() == wxHORIZONTAL);
  contextMenu->AppendSeparator();

  m_plugin->PopulateContextMenu(contextMenu);

  contextMenu->AppendSeparator();
  contextMenu->Append(ID_DASH_PREFS, _("Preferences..."));

#endif

  PopupMenu(contextMenu);
  delete contextMenu;
}

void DashboardWindow::OnContextMenuSelect(wxCommandEvent &event) {
  if (event.GetId() < ID_DASH_PREFS) {  // Toggle dashboard visibility
    m_plugin->ShowDashboard(event.GetId() - 1, event.IsChecked());
    SetToolbarItemState(m_plugin->GetToolbarItemId(),
                        m_plugin->GetDashboardWindowShownCount() != 0);
  }

  switch (event.GetId()) {
    case ID_DASH_PREFS: {
      m_plugin->ShowPreferencesDialog(this);
      return;  // Does it's own save.
    }
    case ID_DASH_RESIZE: {
      /*
                  for( unsigned int i=0; i<m_ArrayOfInstrument.size(); i++ ) {
                      DashboardInstrument* inst =
         m_ArrayOfInstrument.Item(i)->m_pInstrument; inst->Hide();
                  }
      */
      m_binResize = true;

      return;
    }
    case ID_DASH_VERTICAL: {
      ChangePaneOrientation(wxVERTICAL, true);
      m_Container->m_sOrientation = _T("V");
      break;
    }
    case ID_DASH_HORIZONTAL: {
      ChangePaneOrientation(wxHORIZONTAL, true);
      m_Container->m_sOrientation = _T("H");
      break;
    }
    case ID_DASH_UNDOCK: {
      ChangePaneOrientation(GetSizerOrientation(), true);
      return;  // Nothing changed so nothing need be saved
    }
  }
  m_plugin->SaveConfig();
}

void DashboardWindow::SetColorScheme(PI_ColorScheme cs) {
  DimeWindow(this);

  //  Improve appearance, especially in DUSK or NIGHT palette
  wxColour col;
  GetGlobalColor(_T("DASHL"), &col);
  SetBackgroundColour(col);

  Refresh(false);
}

void DashboardWindow::ChangePaneOrientation(int orient, bool updateAUImgr) {
  m_pauimgr->DetachPane(this);
  SetSizerOrientation(orient);
  bool vertical = orient == wxVERTICAL;
  // wxSize sz = GetSize( orient, wxDefaultSize );
  wxSize sz = GetMinSize();
  // We must change Name to reset AUI perpective
  m_Container->m_sName = MakeName();
  m_pauimgr->AddPane(this, wxAuiPaneInfo()
                               .Name(m_Container->m_sName)
                               .Caption(m_Container->m_sCaption)
                               .CaptionVisible(true)
                               .TopDockable(!vertical)
                               .BottomDockable(!vertical)
                               .LeftDockable(vertical)
                               .RightDockable(vertical)
                               .MinSize(sz)
                               .BestSize(sz)
                               .FloatingSize(sz)
                               .FloatingPosition(100, 100)
                               .Float()
                               .Show(m_Container->m_bIsVisible));

#ifdef __OCPN__ANDROID__
  wxAuiPaneInfo &pane = m_pauimgr->GetPane(this);
  pane.Dockable(false);
#endif

  if (updateAUImgr) m_pauimgr->Update();
}

void DashboardWindow::SetSizerOrientation(int orient) {
  itemBoxSizer->SetOrientation(orient);
  /* We must reset all MinSize to ensure we start with new default */
  wxWindowListNode *node = GetChildren().GetFirst();
  while (node) {
    node->GetData()->SetMinSize(wxDefaultSize);
    node = node->GetNext();
  }
  SetMinSize(wxDefaultSize);
  Fit();
  SetMinSize(itemBoxSizer->GetMinSize());
}

int DashboardWindow::GetSizerOrientation() {
  return itemBoxSizer->GetOrientation();
}

bool isArrayIntEqual(const wxArrayInt &l1, const wxArrayOfInstrument &l2) {
  if (l1.GetCount() != l2.GetCount()) return false;

  for (size_t i = 0; i < l1.GetCount(); i++)
    if (l1.Item(i) != l2.Item(i)->m_ID) return false;

  return true;
}

bool DashboardWindow::isInstrumentListEqual(const wxArrayInt &list) {
  return isArrayIntEqual(list, m_ArrayOfInstrument);
}

void DashboardWindow::SetInstrumentList(wxArrayInt list, wxArrayOfInstrumentProperties* InstrumentPropertyList) {
  /* options
   ID_DBP_D_SOG: config max value, show STW optional
   ID_DBP_D_COG:  +SOG +HDG? +BRG?
   ID_DBP_D_AWS: config max value. Two arrows for AWS+TWS?
   ID_DBP_D_VMG: config max value
   ID_DBP_I_DPT: config unit (meter, feet, fathoms)
   ID_DBP_D_DPT: show temp optional
   // compass: use COG or HDG
   // velocity range
   // rudder range

   */
  InstrumentProperties *Properties;
  m_ArrayOfInstrument.Clear();
  itemBoxSizer->Clear(true);
  for (size_t i = 0; i < list.GetCount(); i++) {
    int id = list.Item(i);
    Properties = NULL;
    for (size_t j = 0; j < InstrumentPropertyList->GetCount(); j++)
    {
        if (InstrumentPropertyList->Item(j)->m_aInstrument == id && InstrumentPropertyList->Item(j)->m_Listplace == (int)i)
        {
            Properties = InstrumentPropertyList->Item(j);
            break;
        }
    }
    DashboardInstrument *instrument = NULL;
    switch (id) {
      case ID_DBP_I_POS:
        instrument = new DashboardInstrument_Position(this, wxID_ANY,
                                                      getInstrumentCaption(id), Properties);
        break;
      case ID_DBP_I_SOG:
        instrument = new DashboardInstrument_Single(
            this, wxID_ANY, getInstrumentCaption(id), Properties, OCPN_DBP_STC_SOG,
            _T("%5.1f"));
        break;
      case ID_DBP_D_SOG:
        instrument = new DashboardInstrument_Speedometer(
            this, wxID_ANY, getInstrumentCaption(id), Properties, OCPN_DBP_STC_SOG, 0,
            g_iDashSpeedMax);
        ((DashboardInstrument_Dial *)instrument)
            ->SetOptionLabel(g_iDashSpeedMax / 20 + 1, DIAL_LABEL_HORIZONTAL);
        //(DashboardInstrument_Dial *)instrument->SetOptionMarker(0.1,
        //DIAL_MARKER_SIMPLE, 5);
        ((DashboardInstrument_Dial *)instrument)
            ->SetOptionMarker(0.5, DIAL_MARKER_SIMPLE, 2);
        ((DashboardInstrument_Dial *)instrument)
            ->SetOptionExtraValue(OCPN_DBP_STC_STW, _T("STW\n%.2f"),
                                  DIAL_POSITION_BOTTOMLEFT);
        break;
      case ID_DBP_I_COG:
        instrument = new DashboardInstrument_Single(
            this, wxID_ANY, getInstrumentCaption(id), Properties, OCPN_DBP_STC_COG,
            _T("%03.0f"));
        break;
      case ID_DBP_M_COG:
        instrument = new DashboardInstrument_Single(
            this, wxID_ANY, getInstrumentCaption(id), Properties, OCPN_DBP_STC_MCOG,
            _T("%03.0f"));
        break;
      case ID_DBP_D_COG:
        instrument = new DashboardInstrument_Compass(
            this, wxID_ANY, getInstrumentCaption(id), Properties, OCPN_DBP_STC_COG);
        ((DashboardInstrument_Dial *)instrument)
            ->SetOptionMarker(5, DIAL_MARKER_SIMPLE, 2);
        ((DashboardInstrument_Dial *)instrument)
            ->SetOptionLabel(30, DIAL_LABEL_ROTATED);
        ((DashboardInstrument_Dial *)instrument)
            ->SetOptionExtraValue(OCPN_DBP_STC_SOG, _T("SOG\n%.2f"),
                                  DIAL_POSITION_BOTTOMLEFT);
        break;
      case ID_DBP_D_HDT:
        instrument = new DashboardInstrument_Compass(
            this, wxID_ANY, getInstrumentCaption(id), Properties, OCPN_DBP_STC_HDT);
        ((DashboardInstrument_Dial *)instrument)
            ->SetOptionMarker(5, DIAL_MARKER_SIMPLE, 2);
        ((DashboardInstrument_Dial *)instrument)
            ->SetOptionLabel(30, DIAL_LABEL_ROTATED);
        ((DashboardInstrument_Dial *)instrument)
            ->SetOptionExtraValue(OCPN_DBP_STC_STW, _T("STW\n%.1f"),
                                  DIAL_POSITION_BOTTOMLEFT);
        break;
      case ID_DBP_I_STW:
        instrument = new DashboardInstrument_Single(
            this, wxID_ANY, getInstrumentCaption(id), Properties, OCPN_DBP_STC_STW,
            _T("%.1f"));
        break;
      case ID_DBP_I_HDT:  // true heading
        // TODO: Option True or Magnetic
        instrument = new DashboardInstrument_Single(
            this, wxID_ANY, getInstrumentCaption(id), Properties, OCPN_DBP_STC_HDT,
            _T("%03.0f"));
        break;
      case ID_DBP_I_HDM:  // magnetic heading
        instrument = new DashboardInstrument_Single(
            this, wxID_ANY, getInstrumentCaption(id), Properties, OCPN_DBP_STC_HDM,
            _T("%03.0f"));
        break;
      case ID_DBP_D_AW:
      case ID_DBP_D_AWA:
        instrument = new DashboardInstrument_Wind(
            this, wxID_ANY, getInstrumentCaption(id), Properties, OCPN_DBP_STC_AWA);
        ((DashboardInstrument_Dial *)instrument)
            ->SetOptionMainValue(_T("%.0f"), DIAL_POSITION_BOTTOMLEFT);
        ((DashboardInstrument_Dial *)instrument)
            ->SetOptionExtraValue(OCPN_DBP_STC_AWS, _T("%.1f"),
                                  DIAL_POSITION_INSIDE);
        break;
      case ID_DBP_I_AWS:
        instrument = new DashboardInstrument_Single(
            this, wxID_ANY, getInstrumentCaption(id), Properties, OCPN_DBP_STC_AWS,
            _T("%.1f"));
        break;
      case ID_DBP_D_AWS:
        instrument = new DashboardInstrument_Speedometer(
            this, wxID_ANY, getInstrumentCaption(id), Properties, OCPN_DBP_STC_AWS, 0, 45);
        ((DashboardInstrument_Dial *)instrument)
            ->SetOptionLabel(5, DIAL_LABEL_HORIZONTAL);
        ((DashboardInstrument_Dial *)instrument)
            ->SetOptionMarker(1, DIAL_MARKER_SIMPLE, 5);
        ((DashboardInstrument_Dial *)instrument)
            ->SetOptionMainValue(_T("A %.1f"), DIAL_POSITION_BOTTOMLEFT);
        ((DashboardInstrument_Dial *)instrument)
            ->SetOptionExtraValue(OCPN_DBP_STC_TWS, _T("T %.1f"),
                                  DIAL_POSITION_BOTTOMRIGHT);
        break;
      case ID_DBP_D_TW:  // True Wind angle +-180 degr on boat axis
        instrument = new DashboardInstrument_TrueWindAngle(
            this, wxID_ANY, getInstrumentCaption(id), Properties, OCPN_DBP_STC_TWA);
        ((DashboardInstrument_Dial *)instrument)
            ->SetOptionMainValue(_T("%.0f"), DIAL_POSITION_BOTTOMLEFT);
        ((DashboardInstrument_Dial *)instrument)
            ->SetOptionExtraValue(OCPN_DBP_STC_TWS, _T("%.1f"),
                                  DIAL_POSITION_INSIDE);
        break;
      case ID_DBP_D_AWA_TWA:  // App/True Wind angle +-180 degr on boat axis
        instrument = new DashboardInstrument_AppTrueWindAngle(
            this, wxID_ANY, getInstrumentCaption(id), Properties, OCPN_DBP_STC_AWA);
        ((DashboardInstrument_Dial *)instrument)->SetCapFlag(OCPN_DBP_STC_TWA);
        ((DashboardInstrument_Dial *)instrument)
            ->SetOptionMainValue(_T("%.0f"), DIAL_POSITION_NONE);
        ((DashboardInstrument_Dial *)instrument)
            ->SetOptionExtraValue(OCPN_DBP_STC_TWS, _T("%.1f"),
                                  DIAL_POSITION_NONE);
        ((DashboardInstrument_Dial *)instrument)
            ->SetOptionExtraValue(OCPN_DBP_STC_AWS, _T("%.1f"),
                                  DIAL_POSITION_NONE);
        break;
      case ID_DBP_D_TWD:  // True Wind direction
        instrument = new DashboardInstrument_WindCompass(
            this, wxID_ANY, getInstrumentCaption(id), Properties, OCPN_DBP_STC_TWD);
        ((DashboardInstrument_Dial *)instrument)
            ->SetOptionMainValue(_T("%.0f"), DIAL_POSITION_BOTTOMLEFT);
        ((DashboardInstrument_Dial *)instrument)
            ->SetOptionExtraValue(OCPN_DBP_STC_TWS2, _T("%.1f"),
                                  DIAL_POSITION_INSIDE);
        break;
      case ID_DBP_I_ALTI:
        instrument = new DashboardInstrument_Single(
            this, wxID_ANY, getInstrumentCaption(id), Properties, OCPN_DBP_STC_ALTI,
            _T("%6.1f"));
        break;
      case ID_DBP_D_ALTI:
        instrument = new DashboardInstrument_Altitude(this, wxID_ANY,
                                                   getInstrumentCaption(id), Properties);
        break;
      case ID_DBP_I_DPT:
        instrument = new DashboardInstrument_Single(
            this, wxID_ANY, getInstrumentCaption(id), Properties, OCPN_DBP_STC_DPT,
            _T("%5.2f"));
        break;
      case ID_DBP_D_DPT:
        instrument = new DashboardInstrument_Depth(this, wxID_ANY,
                                                   getInstrumentCaption(id), Properties);
        break;
      case ID_DBP_I_TMP:  // water temperature
        instrument = new DashboardInstrument_Single(
            this, wxID_ANY, getInstrumentCaption(id), Properties, OCPN_DBP_STC_TMP,
            _T("%2.1f"));
        break;
      case ID_DBP_I_MDA:  // barometric pressure
        instrument = new DashboardInstrument_Single(
            this, wxID_ANY, getInstrumentCaption(id), Properties, OCPN_DBP_STC_MDA,
            _T("%5.1f"));
        break;
      case ID_DBP_D_MDA:  // barometric pressure
        instrument = new DashboardInstrument_Speedometer(
            this, wxID_ANY, getInstrumentCaption(id), Properties, OCPN_DBP_STC_MDA, 938,
            1088);
        ((DashboardInstrument_Dial *)instrument)
            ->SetOptionLabel(15, DIAL_LABEL_HORIZONTAL);
        ((DashboardInstrument_Dial *)instrument)
            ->SetOptionMarker(7.5, DIAL_MARKER_SIMPLE, 1);
        ((DashboardInstrument_Dial *)instrument)
            ->SetOptionMainValue(_T("%5.1f"), DIAL_POSITION_INSIDE);
        break;
      case ID_DBP_I_ATMP:  // air temperature
        instrument = new DashboardInstrument_Single(
            this, wxID_ANY, getInstrumentCaption(id), Properties, OCPN_DBP_STC_ATMP,
            _T("%2.1f"));
        break;
      case ID_DBP_I_VLW1:  // Trip Log
        instrument = new DashboardInstrument_Single(
            this, wxID_ANY, getInstrumentCaption(id), Properties, OCPN_DBP_STC_VLW1,
            _T("%2.1f"));
        break;

      case ID_DBP_I_VLW2:  // Sum Log
        instrument = new DashboardInstrument_Single(
            this, wxID_ANY, getInstrumentCaption(id), Properties, OCPN_DBP_STC_VLW2,
            _T("%2.1f"));
        break;

      case ID_DBP_I_TWA:  // true wind angle
        instrument = new DashboardInstrument_Single(
            this, wxID_ANY, getInstrumentCaption(id), Properties, OCPN_DBP_STC_TWA,
            _T("%5.0f"));
        break;
      case ID_DBP_I_TWD:  // true wind direction
        instrument = new DashboardInstrument_Single(
            this, wxID_ANY, getInstrumentCaption(id), Properties, OCPN_DBP_STC_TWD,
            _T("%3.0f"));
        break;
      case ID_DBP_I_TWS:  // true wind speed
        instrument = new DashboardInstrument_Single(
            this, wxID_ANY, getInstrumentCaption(id), Properties, OCPN_DBP_STC_TWS,
            _T("%2.1f"));
        break;
      case ID_DBP_I_AWA:  // apparent wind angle
        instrument = new DashboardInstrument_Single(
            this, wxID_ANY, getInstrumentCaption(id), Properties, OCPN_DBP_STC_AWA,
            _T("%3.0f"));
        break;
      case ID_DBP_I_VMGW:   // VMG based on wind and STW
        instrument = new DashboardInstrument_Single(
            this, wxID_ANY, getInstrumentCaption(id), Properties, OCPN_DBP_STC_VMGW,
            _T("%2.1f"));
        break;
      case ID_DBP_I_VMG:
        instrument = new DashboardInstrument_Single(
            this, wxID_ANY, getInstrumentCaption(id), Properties, OCPN_DBP_STC_VMG,
            _T("%5.1f"));
        break;
      case ID_DBP_D_VMG:
        instrument = new DashboardInstrument_Speedometer(
            this, wxID_ANY, getInstrumentCaption(id), Properties, OCPN_DBP_STC_VMG, 0,
            g_iDashSpeedMax);
        ((DashboardInstrument_Dial *)instrument)
            ->SetOptionLabel(1, DIAL_LABEL_HORIZONTAL);
        ((DashboardInstrument_Dial *)instrument)
            ->SetOptionMarker(0.5, DIAL_MARKER_SIMPLE, 2);
        ((DashboardInstrument_Dial *)instrument)
            ->SetOptionExtraValue(OCPN_DBP_STC_SOG, _T("SOG\n%.1f"),
                                  DIAL_POSITION_BOTTOMLEFT);
        break;
      case ID_DBP_I_RSA:
        instrument = new DashboardInstrument_Single(
            this, wxID_ANY, getInstrumentCaption(id), Properties, OCPN_DBP_STC_RSA,
            _T("%5.0f"));
        break;
      case ID_DBP_D_RSA:
        instrument = new DashboardInstrument_RudderAngle(
            this, wxID_ANY, getInstrumentCaption(id), Properties);
        break;
      case ID_DBP_I_SAT:
        instrument = new DashboardInstrument_Single(
            this, wxID_ANY, getInstrumentCaption(id), Properties, OCPN_DBP_STC_SAT,
            _T("%5.0f"));
        break;
      case ID_DBP_D_GPS:
        instrument = new DashboardInstrument_GPS(this, wxID_ANY,
                                                 getInstrumentCaption(id), Properties);
        break;
      case ID_DBP_I_PTR:
        instrument = new DashboardInstrument_Position(
            this, wxID_ANY, getInstrumentCaption(id), Properties, OCPN_DBP_STC_PLA,
            OCPN_DBP_STC_PLO);
        break;
      case ID_DBP_I_GPSUTC:
        instrument = new DashboardInstrument_Clock(this, wxID_ANY,
                                                   getInstrumentCaption(id), Properties);
        break;
      case ID_DBP_I_SUN:
        instrument = new DashboardInstrument_Sun(this, wxID_ANY,
                                                 getInstrumentCaption(id), Properties);
        break;
      case ID_DBP_D_MON:
        instrument = new DashboardInstrument_Moon(this, wxID_ANY,
                                                  getInstrumentCaption(id), Properties);
        break;
      case ID_DBP_D_WDH:
        instrument = new DashboardInstrument_WindDirHistory(
            this, wxID_ANY, getInstrumentCaption(id), Properties);
        break;
      case ID_DBP_D_BPH:
        instrument = new DashboardInstrument_BaroHistory(
            this, wxID_ANY, getInstrumentCaption(id), Properties);
        break;
      case ID_DBP_I_FOS:
        instrument = new DashboardInstrument_FromOwnship(
            this, wxID_ANY, getInstrumentCaption(id), Properties);
        break;
      case ID_DBP_I_PITCH:
        instrument = new DashboardInstrument_Single(
            this, wxID_ANY, getInstrumentCaption(id), Properties, OCPN_DBP_STC_PITCH,
            _T("%2.1f"));
        break;
      case ID_DBP_I_HEEL:
        instrument = new DashboardInstrument_Single(
            this, wxID_ANY, getInstrumentCaption(id), Properties, OCPN_DBP_STC_HEEL,
            _T("%2.1f"));
        break;
        // any clock display with "LCL" in the format string is converted from
        // UTC to local TZ
      case ID_DBP_I_SUNLCL:
        instrument = new DashboardInstrument_Sun(this, wxID_ANY,
                                                 getInstrumentCaption(id), Properties,
                                                 _T( "%02i:%02i:%02i LCL" ));
        break;
      case ID_DBP_I_GPSLCL:
        instrument = new DashboardInstrument_Clock(
            this, wxID_ANY, getInstrumentCaption(id), Properties, OCPN_DBP_STC_CLK,
            _T( "%02i:%02i:%02i LCL" ));
        break;
      case ID_DBP_I_CPULCL:
        instrument = new DashboardInstrument_CPUClock(
            this, wxID_ANY, getInstrumentCaption(id), Properties,
            _T( "%02i:%02i:%02i LCL" ));
    }
    if (instrument) {
      instrument->instrumentTypeId = id;
      m_ArrayOfInstrument.Add(new DashboardInstrumentContainer(
          id, instrument, instrument->GetCapacity()));
      itemBoxSizer->Add(instrument, 0, wxEXPAND, 0);
      if (itemBoxSizer->GetOrientation() == wxHORIZONTAL) {
        itemBoxSizer->AddSpacer(5);
      }
    }
  }

  //  In the absense of any other hints, build the default instrument sizes by
  //  taking the calculated with of the first (and succeeding) instruments as
  //  hints for the next. So, best in default loads to start with an instrument
  //  that accurately calculates its minimum width. e.g.
  //  DashboardInstrument_Position

  wxSize Hint = wxSize(DefaultWidth, DefaultWidth);
  for (unsigned int i = 0; i < m_ArrayOfInstrument.size(); i++) {
    DashboardInstrument *inst = m_ArrayOfInstrument.Item(i)->m_pInstrument;
    inst->SetMinSize(inst->GetSize(itemBoxSizer->GetOrientation(), Hint));
    Hint = inst->GetMinSize();
  }

  Fit();
  Layout();
  SetMinSize(itemBoxSizer->GetMinSize());
}

void DashboardWindow::SendSentenceToAllInstruments(DASH_CAP st, double value,
                                                   wxString unit) {
  for (size_t i = 0; i < m_ArrayOfInstrument.GetCount(); i++) {
    if (m_ArrayOfInstrument.Item(i)->m_cap_flag.test(st))
      m_ArrayOfInstrument.Item(i)->m_pInstrument->SetData(st, value, unit);
  }
}

void DashboardWindow::SendSatInfoToAllInstruments(int cnt, int seq,
                                                  wxString talk,
                                                  SAT_INFO sats[4]) {
  for (size_t i = 0; i < m_ArrayOfInstrument.GetCount(); i++) {
    if ((m_ArrayOfInstrument.Item(i)->m_cap_flag.test(OCPN_DBP_STC_GPS)) &&
        m_ArrayOfInstrument.Item(i)->m_pInstrument->IsKindOf(
            CLASSINFO(DashboardInstrument_GPS)))
      ((DashboardInstrument_GPS *)m_ArrayOfInstrument.Item(i)->m_pInstrument)
          ->SetSatInfo(cnt, seq, talk, sats);
  }
}

void DashboardWindow::SendUtcTimeToAllInstruments(wxDateTime value) {
  for (size_t i = 0; i < m_ArrayOfInstrument.GetCount(); i++) {
    if ((m_ArrayOfInstrument.Item(i)->m_cap_flag.test(OCPN_DBP_STC_CLK)) &&
        m_ArrayOfInstrument.Item(i)->m_pInstrument->IsKindOf(
            CLASSINFO(DashboardInstrument_Clock)))
      //                  || m_ArrayOfInstrument.Item( i
      //                  )->m_pInstrument->IsKindOf( CLASSINFO(
      //                  DashboardInstrument_Sun ) )
      //                  || m_ArrayOfInstrument.Item( i
      //                  )->m_pInstrument->IsKindOf( CLASSINFO(
      //                  DashboardInstrument_Moon ) ) ) )
      ((DashboardInstrument_Clock *)m_ArrayOfInstrument.Item(i)->m_pInstrument)
          ->SetUtcTime(value);
  }
}

//#include "wx/fontpicker.h"

//#include "wx/fontdlg.h"

// ============================================================================
// implementation
// ============================================================================

IMPLEMENT_DYNAMIC_CLASS(OCPNFontButton, wxButton)

// ----------------------------------------------------------------------------
// OCPNFontButton
// ----------------------------------------------------------------------------

bool OCPNFontButton::Create(wxWindow *parent, wxWindowID id,
                            const wxFontData &initial, const wxPoint &pos,
                            const wxSize &size, long style,
                            const wxValidator &validator,
                            const wxString &name) {
  wxString label = (style & wxFNTP_FONTDESC_AS_LABEL)
                       ? wxString()
                       :  // label will be updated by UpdateFont
                       _("Choose font");
  label = name;
  // create this button
  if (!wxButton::Create(parent, id, label, pos, size, style, validator, name)) {
    wxFAIL_MSG(wxT("OCPNFontButton creation failed"));
    return false;
  }

  // and handle user clicks on it
  Connect(GetId(), wxEVT_BUTTON,
          wxCommandEventHandler(OCPNFontButton::OnButtonClick), NULL, this);

  m_data = initial;
  m_selectedFont = initial.GetChosenFont().IsOk() ? initial.GetChosenFont() : *wxNORMAL_FONT;
  UpdateFont();

  return true;
}

void OCPNFontButton::OnButtonClick(wxCommandEvent &WXUNUSED(ev)) {
  // update the wxFontData to be shown in the dialog
  m_data.SetInitialFont(m_selectedFont);

  // create the font dialog and display it
  wxFontDialog dlg(this, m_data);

  wxFont *pF = OCPNGetFont(_T("Dialog"), 0);
  dlg.SetFont(*pF);

#ifdef __WXQT__
  // Make sure that font dialog will fit on the screen without scrolling
  // We do this by setting the dialog font size "small enough" to show "n" lines
  wxSize proposed_size = GetParent()->GetSize();
  float n_lines = 30;
  float font_size = pF->GetPointSize();

  if ((proposed_size.y / font_size) < n_lines) {
    float new_font_size = proposed_size.y / n_lines;
    wxFont *smallFont = new wxFont(*pF);
    smallFont->SetPointSize(new_font_size);
    dlg.SetFont(*smallFont);
  }

  dlg.SetSize(GetParent()->GetSize());
  dlg.Centre();
#endif

  if (dlg.ShowModal() == wxID_OK) {
    m_data = dlg.GetFontData();
    m_selectedFont = m_data.GetChosenFont();

    // fire an event
    wxFontPickerEvent event(this, GetId(), m_selectedFont);
    GetEventHandler()->ProcessEvent(event);

    UpdateFont();
  }
}

void OCPNFontButton::UpdateFont() {
  if (!m_selectedFont.IsOk()) return;

  //  Leave black, until Instruments are modified to accept color fonts
  // SetForegroundColour(m_data.GetColour());

  if (HasFlag(wxFNTP_USEFONT_FOR_LABEL)) {
    // use currently selected font for the label...
    wxButton::SetFont(m_selectedFont);
    wxButton::SetForegroundColour(GetSelectedColour());
  }

  wxString label =
      wxString::Format(wxT("%s, %d"), m_selectedFont.GetFaceName().c_str(),
                       m_selectedFont.GetPointSize());

  if (HasFlag(wxFNTP_FONTDESC_AS_LABEL)) {
    SetLabel(label);
  }

  auto minsize = GetTextExtent(label);
  SetSize(minsize);  

  GetParent()->Layout();
  GetParent()->Fit();
}

// Edit Dialog

EditDialog::EditDialog(wxWindow* parent, InstrumentProperties& Properties, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style) : wxDialog(parent, id, title, pos, size, style)
{
    this->SetSizeHints(wxDefaultSize, wxDefaultSize);

    wxBoxSizer* bSizer5;
    bSizer5 = new wxBoxSizer(wxVERTICAL);

    wxFlexGridSizer* fgSizer2;
    fgSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
    fgSizer2->SetFlexibleDirection(wxBOTH);
    fgSizer2->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

    m_staticText1 = new wxStaticText(this, wxID_ANY, _("Title:"), wxDefaultPosition, wxDefaultSize, 0);
    m_staticText1->Wrap(-1);
    fgSizer2->Add(m_staticText1, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    m_fontPicker2 = new wxFontPickerCtrl(this, wxID_ANY, Properties.m_TitelFont, wxDefaultPosition, wxDefaultSize);
    fgSizer2->Add(m_fontPicker2, 0, wxALL, 5);

    m_staticText5 = new wxStaticText(this, wxID_ANY, _("Titlebackgroundcolor:"), wxDefaultPosition, wxDefaultSize, 0);
    m_staticText5->Wrap(-1);
    fgSizer2->Add(m_staticText5, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    m_colourPicker1 = new wxColourPickerCtrl(this, wxID_ANY, Properties.m_TitlelBackgroundColour, wxDefaultPosition, wxDefaultSize, wxCLRP_DEFAULT_STYLE);
    fgSizer2->Add(m_colourPicker1, 0, wxALL, 5);

    m_staticText2 = new wxStaticText(this, wxID_ANY, _("Data:"), wxDefaultPosition, wxDefaultSize, 0);
    m_staticText2->Wrap(-1);
    fgSizer2->Add(m_staticText2, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    m_fontPicker4 = new wxFontPickerCtrl(this, wxID_ANY, Properties.m_DataFont, wxDefaultPosition, wxDefaultSize);
    fgSizer2->Add(m_fontPicker4, 0, wxALL, 5);

    m_staticText6 = new wxStaticText(this, wxID_ANY, _("Databackgroundcolor:"), wxDefaultPosition, wxDefaultSize, 0);
    m_staticText6->Wrap(-1);
    fgSizer2->Add(m_staticText6, 0, wxALL, 5);

    m_colourPicker2 = new wxColourPickerCtrl(this, wxID_ANY, Properties.m_DataBackgroundColour, wxDefaultPosition, wxDefaultSize, wxCLRP_DEFAULT_STYLE);
    fgSizer2->Add(m_colourPicker2, 0, wxALL, 5);

    m_staticText3 = new wxStaticText(this, wxID_ANY, _("Label:"), wxDefaultPosition, wxDefaultSize, 0);
    m_staticText3->Wrap(-1);
    fgSizer2->Add(m_staticText3, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    m_fontPicker5 = new wxFontPickerCtrl(this, wxID_ANY, Properties.m_LabelFont, wxDefaultPosition, wxDefaultSize);
    fgSizer2->Add(m_fontPicker5, 0, wxALL, 5);

    m_staticText4 = new wxStaticText(this, wxID_ANY, _("Small:"), wxDefaultPosition, wxDefaultSize, 0);
    m_staticText4->Wrap(-1);
    fgSizer2->Add(m_staticText4, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    m_fontPicker6 = new wxFontPickerCtrl(this, wxID_ANY, Properties.m_SmallFont, wxDefaultPosition, wxDefaultSize);
    fgSizer2->Add(m_fontPicker6, 0, wxALL, 5);

    m_staticText9 = new wxStaticText(this, wxID_ANY, _("Arrow 1 Colour :"), wxDefaultPosition, wxDefaultSize, 0);
    m_staticText9->Wrap(-1);
    fgSizer2->Add(m_staticText9, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    m_colourPicker3 = new wxColourPickerCtrl(this, wxID_ANY, Properties.m_Arrow_First_Colour, wxDefaultPosition, wxDefaultSize, wxCLRP_DEFAULT_STYLE);
    fgSizer2->Add(m_colourPicker3, 0, wxALL, 5);

    m_staticText10 = new wxStaticText(this, wxID_ANY, _("Arrow 2 Colour :"), wxDefaultPosition, wxDefaultSize, 0);
    m_staticText10->Wrap(-1);
    fgSizer2->Add(m_staticText10, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    m_colourPicker4 = new wxColourPickerCtrl(this, wxID_ANY, Properties.m_Arrow_Second_Colour, wxDefaultPosition, wxDefaultSize, wxCLRP_DEFAULT_STYLE);
    fgSizer2->Add(m_colourPicker4, 0, wxALL, 5);

    m_staticline1 = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
    fgSizer2->Add(m_staticline1, 0, wxEXPAND | wxALL, 5);

    m_staticline2 = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
    fgSizer2->Add(m_staticline2, 0, wxEXPAND | wxALL, 5);

    fgSizer2->Add(0, 5, 1, wxEXPAND, 5);

    fgSizer2->Add(0, 0, 1, wxEXPAND, 5);

    m_staticText7 = new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
    m_staticText7->Wrap(-1);
    fgSizer2->Add(m_staticText7, 0, wxALL, 5);

    m_button1 = new wxButton(this, wxID_ANY, _("Set default"), wxDefaultPosition, wxDefaultSize, 0);
    fgSizer2->Add(m_button1, 0, wxALL, 5);

    fgSizer2->Add(0, 5, 1, wxEXPAND, 5);

    fgSizer2->Add(5, 0, 1, wxEXPAND, 5);

    bSizer5->Add(fgSizer2, 1, wxALL | wxEXPAND, 5);

    m_sdbSizer3 = new wxStdDialogButtonSizer();
    m_sdbSizer3OK = new wxButton(this, wxID_OK);
    m_sdbSizer3->AddButton(m_sdbSizer3OK);
    m_sdbSizer3Cancel = new wxButton(this, wxID_CANCEL);
    m_sdbSizer3->AddButton(m_sdbSizer3Cancel);
    m_sdbSizer3->Realize();

    bSizer5->Add(m_sdbSizer3, 0, 0, 1);


    bSizer5->Add(0, 10, 0, wxEXPAND, 5);


    this->SetSizer(bSizer5);
    this->Layout();
    bSizer5->Fit(this);

    this->Centre(wxBOTH);

    // Connect Events
    m_button1->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(EditDialog::OnSetdefault), NULL, this);
}

EditDialog::~EditDialog()
{
    // Disconnect Events
    m_button1->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(EditDialog::OnSetdefault), NULL, this);
}

void EditDialog::OnSetdefault(wxCommandEvent& event)
{
    m_fontPicker2->SetSelectedFont(g_FontTitle.GetChosenFont());
    m_fontPicker2->SetSelectedColour(g_FontTitle.GetColour());
    m_fontPicker4->SetSelectedFont(g_FontData.GetChosenFont());
    m_fontPicker4->SetSelectedColour(g_FontData.GetColour());
    m_fontPicker5->SetSelectedFont(g_FontLabel.GetChosenFont());
    m_fontPicker5->SetSelectedColour(g_FontLabel.GetColour());
    m_fontPicker6->SetSelectedFont(g_FontSmall.GetChosenFont());
    m_fontPicker6->SetSelectedColour(g_FontSmall.GetColour());
    wxColour dummy;
    GetGlobalColor(_T("DASHL"), &dummy);
    m_colourPicker1->SetColour(dummy);
    GetGlobalColor(_T("DASHB"), &dummy);
    m_colourPicker2->SetColour(dummy);
    GetGlobalColor(_T("DASHN"), &dummy);
    m_colourPicker3->SetColour(dummy);
    GetGlobalColor(_T("BLUE3"), &dummy);
    m_colourPicker4->SetColour(dummy);
    Update();
}

