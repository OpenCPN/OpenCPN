/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2018 by David S. Register                               *
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

#include "config.h"
#include "model/config_vars.h"
#include "ConfigMgr.h"

#include <wx/filename.h>
#include <wx/fileconf.h>
#include <wx/statline.h>

#include "dychart.h"

#include <stdlib.h>
//#include <math.h>
#include <time.h>
#include <locale>
#include <list>

#include <wx/listimpl.cpp>
#include <wx/progdlg.h>

#include "navutil.h"
#include "chcanv.h"
#include "model/georef.h"
#include "model/cutil.h"
#include "styles.h"
#include "model/routeman.h"
#include "s52utils.h"
#include "chartbase.h"
#include "ocpndc.h"
#include "model/geodesic.h"
#include "model/multiplexer.h"
#include "nmea0183.h"
#include "ais.h"
#include "model/route.h"
#include "model/select.h"
#include "FontMgr.h"
#include "Layer.h"
#include "model/nav_object_database.h"
#include "NMEALogWindow.h"
#include "model/ais_decoder.h"
#include "OCPNPlatform.h"
#include "model/track.h"
#include "chartdb.h"
#include "CanvasConfig.h"
#include "ocpn_frame.h"

#include "s52plib.h"
#include "cm93.h"

#ifdef ocpnUSE_GL
#include "glChartCanvas.h"
#endif

// Global statics
//    Statics

extern OCPNPlatform *g_Platform;
extern MyFrame *gFrame;

extern int g_restore_stackindex;
extern int g_restore_dbindex;
extern LayerList *pLayerList;
extern MyConfig *pConfig;
extern int g_nbrightness;
extern bool g_bShowTrue, g_bShowMag;
extern bool g_bShowStatusBar;
extern bool g_bUIexpert;
extern bool g_bFullscreen;

extern wxString g_SENCPrefix;
extern wxString g_UserPresLibData;

extern wxString *pInit_Chart_Dir;
extern wxString gWorldMapLocation;
extern wxString  g_TalkerIdText;

extern bool s_bSetSystemTime;
extern bool g_bDisplayGrid;  // Flag indicating if grid is to be displayed
extern bool g_bPlayShipsBells;
extern int g_iSoundDeviceIndex;
extern bool g_bFullscreenToolbar;
extern bool g_bShowLayers;
extern bool g_bTransparentToolbar;
extern bool g_bPermanentMOBIcon;

extern bool g_bShowDepthUnits;
extern bool g_bAutoAnchorMark;
extern bool g_bskew_comp;
extern bool g_bopengl;
extern bool g_bSoftwareGL;
extern bool g_bShowFPS;
extern bool g_bsmoothpanzoom;

extern bool g_bShowOutlines;
extern bool g_bShowActiveRouteHighway;
extern bool g_bShowRouteTotal;
extern int g_nAWDefault;
extern int g_nAWMax;
extern int g_nTrackPrecision;

extern int g_nframewin_x;
extern int g_nframewin_y;
extern int g_nframewin_posx;
extern int g_nframewin_posy;
extern bool g_bframemax;
extern int g_route_prop_x, g_route_prop_y;
extern int g_route_prop_sx, g_route_prop_sy;

extern double g_PlanSpeed;
extern wxString g_VisibleLayers;
extern wxString g_InvisibleLayers;

// LIVE ETA OPTION
extern bool g_bShowLiveETA;
extern double g_defaultBoatSpeed;

//    AIS Global configuration
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
extern bool g_bShowCOG;
extern bool g_bSyncCogPredictors;
extern double g_ShowCOG_Mins;
extern bool g_bAISShowTracks;
extern bool g_bTrackCarryOver;
extern bool g_bTrackDaily;
extern int g_track_rotate_time;
extern int g_track_rotate_time_type;
extern double g_AISShowTracks_Mins;
extern double g_AISShowTracks_Limit;
extern bool g_bHideMoored;
extern double g_ShowMoored_Kts;
extern bool g_bAllowShowScaled;
extern bool g_bShowScaled;
extern int g_ShowScaled_Num;
extern bool g_bAIS_CPA_Alert;
extern bool g_bAIS_CPA_Alert_Audio;
extern int g_ais_alert_dialog_x, g_ais_alert_dialog_y;
extern int g_ais_alert_dialog_sx, g_ais_alert_dialog_sy;
extern int g_ais_query_dialog_x, g_ais_query_dialog_y;
extern wxString g_sAIS_Alert_Sound_File;
extern bool g_bAIS_CPA_Alert_Suppress_Moored;
extern bool g_bAIS_ACK_Timeout;
extern double g_AckTimeout_Mins;
extern wxString g_AisTargetList_perspective;
extern int g_AisTargetList_range;
extern int g_AisTargetList_sortColumn;
extern bool g_bAisTargetList_sortReverse;
extern wxString g_AisTargetList_column_spec;
extern wxString g_AisTargetList_column_order;
extern bool g_bShowAreaNotices;
extern bool g_bDrawAISSize;
extern bool g_bDrawAISRealtime;
extern double g_AIS_RealtPred_Kts;
extern bool g_bShowAISName;
extern int g_Show_Target_Name_Scale;
extern int g_WplAction;
extern bool g_benableAISNameCache;
extern int g_ScaledNumWeightSOG;
extern int g_ScaledNumWeightCPA;
extern int g_ScaledNumWeightTCPA;
extern int g_ScaledNumWeightRange;
extern int g_ScaledNumWeightSizeOfT;
extern int g_ScaledSizeMinimal;

extern int g_S57_dialog_sx, g_S57_dialog_sy;

extern int g_iNavAidRadarRingsNumberVisible;
extern float g_fNavAidRadarRingsStep;
extern int g_pNavAidRadarRingsStepUnits;
extern int g_iWaypointRangeRingsNumber;
extern float g_fWaypointRangeRingsStep;
extern int g_iWaypointRangeRingsStepUnits;
extern wxColour g_colourWaypointRangeRingsColour;
extern bool g_bWayPointPreventDragging;
extern bool g_bConfirmObjectDelete;
extern wxColour g_colourOwnshipRangeRingsColour;

extern bool g_bEnableZoomToCursor;
extern wxString g_toolbarConfig;
extern double g_TrackIntervalSeconds;
extern double g_TrackDeltaDistance;

extern bool g_bGDAL_Debug;
extern bool g_bDebugCM93;
extern bool g_bDebugS57;

extern double g_ownship_predictor_minutes;
extern double g_ownship_HDTpredictor_miles;

extern bool g_own_ship_sog_cog_calc;
extern int g_own_ship_sog_cog_calc_damp_sec;

extern s52plib *ps52plib;

extern int g_cm93_zoom_factor;
extern bool g_b_legacy_input_filter_behaviour;
extern bool g_bShowDetailSlider;
extern int g_detailslider_dialog_x, g_detailslider_dialog_y;

extern int g_OwnShipIconType;
extern double g_n_ownship_length_meters;
extern double g_n_ownship_beam_meters;
extern double g_n_gps_antenna_offset_y;
extern double g_n_gps_antenna_offset_x;
extern int g_n_ownship_min_mm;
extern double g_n_arrival_circle_radius;

extern bool g_bPreserveScaleOnX;
extern bool g_bsimplifiedScalebar;

extern bool g_bUseGLL;

extern wxString g_locale;
extern wxString g_localeOverride;

extern bool g_bCourseUp;
extern bool g_bLookAhead;
extern int g_COGAvgSec;
extern bool g_bMagneticAPB;
extern bool g_bShowChartBar;

extern int g_MemFootMB;

extern wxString g_AW1GUID;
extern wxString g_AW2GUID;
extern int g_BSBImgDebug;

extern wxString g_config_version_string;
extern wxString g_config_version_string;

extern bool g_bAISRolloverShowClass;
extern bool g_bAISRolloverShowCOG;
extern bool g_bAISRolloverShowCPA;

extern bool g_bDebugGPSD;

extern bool g_bfilter_cogsog;
extern int g_COGFilterSec;

extern int g_navobjbackups;

extern bool g_bQuiltEnable;
extern bool g_bFullScreenQuilt;
extern bool g_bQuiltStart;

extern int g_SkewCompUpdatePeriod;

extern int g_maintoolbar_x;
extern int g_maintoolbar_y;
extern long g_maintoolbar_orient;

extern int g_lastClientRectx;
extern int g_lastClientRecty;
extern int g_lastClientRectw;
extern int g_lastClientRecth;

extern bool g_bHighliteTracks;
extern int g_cog_predictor_width;
extern int g_ais_cog_predictor_width;

extern int g_route_line_width;
extern int g_track_line_width;
extern wxColour g_colourTrackLineColour;
extern wxString g_default_wp_icon;

extern ChartGroupArray *g_pGroupArray;

extern bool g_bDebugOGL;
extern wxString g_uploadConnection;

extern std::vector<std::string> TideCurrentDataSet;
extern wxString g_TCData_Dir;

extern bool g_btouch;
extern bool g_bresponsive;

extern bool g_bGLexpert;

extern int g_SENC_LOD_pixels;
extern ArrayOfMmsiProperties g_MMSI_Props_Array;

extern int g_chart_zoom_modifier_raster;
extern int g_chart_zoom_modifier_vector;

extern int g_NMEAAPBPrecision;

extern bool g_bAdvanceRouteWaypointOnArrivalOnly;
extern double g_display_size_mm;
extern double g_config_display_size_mm;
extern bool g_config_display_size_manual;

extern bool g_benable_rotate;
extern bool g_bEmailCrashReport;

extern int g_default_font_size;

extern bool g_bAutoHideToolbar;
extern int g_nAutoHideToolbar;
extern int g_GUIScaleFactor;
extern int g_ChartScaleFactor;
extern int g_ShipScaleFactor;

extern bool g_bInlandEcdis;
extern int g_iENCToolbarPosX;
extern int g_iENCToolbarPosY;

extern bool g_bSpaceDropMark;

extern bool g_benableUDPNullHeader;
extern bool g_bShowMenuBar;
extern bool g_bShowCompassWin;

extern wxString g_uiStyle;
extern bool g_useMUI;
extern wxString g_gpx_path;

extern unsigned int g_canvasConfig;

#ifdef ocpnUSE_GL
extern ocpnGLOptions g_GLOptions;
#endif

#if !defined(NAN)
static const long long lNaN = 0xfff8000000000000;
#define NAN (*(double *)&lNaN)
#endif

ConfigMgr *ConfigMgr::instance = NULL;

//--------------------------------------------------------------------------
//
//      Utility functions
//
//--------------------------------------------------------------------------
int GetRandomNumber(int range_min, int range_max) {
  long u = (long)wxRound(
      ((double)rand() / ((double)(RAND_MAX) + 1) * (range_max - range_min)) +
      range_min);
  return (int)u;
}

// Helper conditional file name dir slash
void appendOSDirSlash(wxString *pString);

class OCPNConfigObject {
public:
  OCPNConfigObject();
  ~OCPNConfigObject();

  OCPNConfigObject(int canvas_config);

  void Init();
  wxPanel *GetSettingsPanel();

  int m_canvasConfig;
  wxString m_GUID;
  wxString templateFileName;
  wxString m_title;
  wxString m_description;
};

OCPNConfigObject::OCPNConfigObject() { Init(); }

OCPNConfigObject::OCPNConfigObject(int canvas_config) {
  Init();
  m_canvasConfig = canvas_config;
}

OCPNConfigObject::~OCPNConfigObject() {}

void OCPNConfigObject::Init() { m_canvasConfig = 0; }

#include <wx/listimpl.cpp>
WX_DEFINE_LIST(ConfigObjectList);

//--------------------------------------------------------------------
//   Private ( XML encoded ) catalog of available configurations
//--------------------------------------------------------------------

class OCPNConfigCatalog : public pugi::xml_document {
public:
  OCPNConfigCatalog();
  ~OCPNConfigCatalog();

  bool AddConfig(OCPNConfigObject *config, unsigned int flags);
  bool RemoveConfig(wxString GUID);

  void SetRootConfigNode(void);
  bool IsOpenCPN();
  bool SaveFile(const wxString filename);
  bool LoadFile(const wxString filename);

  pugi::xml_node m_config_root;
};

OCPNConfigCatalog::OCPNConfigCatalog() : pugi::xml_document() {}

OCPNConfigCatalog::~OCPNConfigCatalog() {}

void OCPNConfigCatalog::SetRootConfigNode(void) {
  if (!strlen(m_config_root.name())) {
    m_config_root = append_child("configs");
    m_config_root.append_attribute("version") = "1.0";
    m_config_root.append_attribute("creator") = "OpenCPN";
    m_config_root.append_attribute("xmlns:xsi") =
        "http://www.w3.org/2001/XMLSchema-instance";
    m_config_root.append_attribute("xmlns") =
        "http://www.topografix.com/GPX/1/1";
    m_config_root.append_attribute("xmlns:gpxx") =
        "http://www.garmin.com/xmlschemas/GpxExtensions/v3";
    m_config_root.append_attribute("xsi:schemaLocation") =
        "http://www.topografix.com/GPX/1/1 "
        "http://www.topografix.com/GPX/1/1/gpx.xsd";
    m_config_root.append_attribute("xmlns:opencpn") = "http://www.opencpn.org";
  }
}

bool OCPNConfigCatalog::IsOpenCPN() {
  for (pugi::xml_attribute attr = root().first_child().first_attribute(); attr;
       attr = attr.next_attribute())
    if (!strcmp(attr.name(), "creator") && !strcmp(attr.value(), "OpenCPN"))
      return true;
  return false;
}

bool OCPNConfigCatalog::SaveFile(const wxString filename) {
  save_file(filename.fn_str(), "  ");
  return true;
}

bool OCPNConfigCatalog::LoadFile(const wxString filename) {
  load_file(filename.fn_str());
  m_config_root = this->child("configs");
  return true;
}

bool OCPNConfigCatalog::AddConfig(OCPNConfigObject *config,
                                  unsigned int flags) {
  pugi::xml_node node = m_config_root.append_child("config");

  node.append_attribute("GUID") = config->m_GUID.mb_str();

  //  Handle non-ASCII characters as UTF8
  wxCharBuffer abuf = config->m_title.ToUTF8();
  if (abuf.data())
    node.append_attribute("title") = abuf.data();
  else
    node.append_attribute("title") = _T("Substitute Title");

  abuf = config->m_description.ToUTF8();
  if (abuf.data())
    node.append_attribute("description") = abuf.data();
  else
    node.append_attribute("description") = _T("Substitute Description");

  node.append_attribute("templateFile") = config->templateFileName.mb_str();

  return true;
}

bool OCPNConfigCatalog::RemoveConfig(wxString GUID) {
  for (pugi::xml_node child = m_config_root.first_child(); child;) {
    pugi::xml_node next = child.next_sibling();
    const char *guid = child.attribute("GUID").value();

    if (!strcmp(guid, GUID.mb_str())) {
      child.parent().remove_child(child);
      return true;
    }

    child = next;
  }

  return false;
}

//--------------------------------------------------------------------
//   ConfigPanel implementation
//--------------------------------------------------------------------

ConfigPanel::ConfigPanel(OCPNConfigObject *config, wxWindow *parent,
                         wxWindowID id, const wxPoint &pos, const wxSize &size)
    : wxPanel(parent, id, pos, size, wxSIMPLE_BORDER)

{
  m_config = config;
  wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
  SetSizer(mainSizer);

  mainSizer->Add(new wxStaticText(this, wxID_ANY, _("Title")));
  mainSizer->Add(new wxStaticText(this, wxID_ANY, config->m_title));

  mainSizer->Add(new wxStaticLine(this, wxID_ANY), 0, wxEXPAND | wxALL, 1);

  mainSizer->Add(new wxStaticText(this, wxID_ANY, _("Description")));
  mainSizer->Add(new wxStaticText(this, wxID_ANY, config->m_description));

  SetMinSize(wxSize(-1, 6 * GetCharHeight()));

  wxColour colour;
  GetGlobalColor(_T("COMP1"), &colour);
  SetBackgroundColour(colour);
  // Connect(wxEVT_LEFT_DOWN,
  // wxMouseEventHandler(ConfigPanel::OnConfigPanelMouseSelected), NULL, this);
}

ConfigPanel::~ConfigPanel() {}

void ConfigPanel::OnConfigPanelMouseSelected(wxMouseEvent &event) {
  //     SetBackgroundColour(*wxRED);
  //     event.Skip();
}

wxString ConfigPanel::GetConfigGUID() { return m_config->m_GUID; }

//--------------------------------------------------------------------
//   ConfigMgr implementation
//   Singleton Pattern
//--------------------------------------------------------------------

ConfigMgr &ConfigMgr::Get() {
  if (!instance) instance = new ConfigMgr;
  return *instance;
}

void ConfigMgr::Shutdown() {
  if (instance) {
    delete instance;
    instance = NULL;
  }
}

ConfigMgr::ConfigMgr() {
  Init();

  // Load any existing configs from the catalog
  LoadCatalog();
}

ConfigMgr::~ConfigMgr() { delete configList; }

void ConfigMgr::Init() {
  m_configDir = g_Platform->GetPrivateDataDir();
  appendOSDirSlash(&m_configDir);
  m_configDir.append(_T("Configs"));
  appendOSDirSlash(&m_configDir);
  if (!wxFileName::DirExists(m_configDir)) {
    wxFileName::Mkdir(m_configDir);
  }

  m_configCatalogName = g_Platform->GetPrivateDataDir();
  appendOSDirSlash(&m_configCatalogName);
  m_configCatalogName.append(_T("Configs"));
  appendOSDirSlash(&m_configCatalogName);
  m_configCatalogName.append(_T("configs.xml"));

  // Create the catalog, if necessary
  if (!wxFileExists(m_configCatalogName)) {
    wxLogMessage(_T("Creating new Configs catalog: ") + m_configCatalogName);

    OCPNConfigCatalog *cat = new OCPNConfigCatalog();
    cat->SetRootConfigNode();
    cat->SaveFile(m_configCatalogName);
    delete cat;
  }

  m_configCatalog = new OCPNConfigCatalog();

  configList = new ConfigObjectList;

  //  Add the default "Recovery" template
  wxString t_title = _("Recovery Template");
  wxString t_desc =
      _("Apply this template to return to a known safe configuration");
  CreateNamedConfig(t_title, t_desc,
                    _T("11111111-1111-1111-1111-111111111111"));
}

bool ConfigMgr::LoadCatalog() {
  wxLogMessage(_T("Loading Configs catalog: ") + m_configCatalogName);
  m_configCatalog->LoadFile(m_configCatalogName);

  // Parse the config catalog
  pugi::xml_node objects = m_configCatalog->child("configs");

  // pugi::xml_node node = m_config_root.append_child("config");

  // node.append_attribute("GUID") = config->m_GUID.mb_str();
  // node.append_attribute("title") = config->m_title.mb_str();
  // node.append_attribute("description") = config->m_description.mb_str();
  // node.append_attribute("templateFile") = config->templateFileName.mb_str();

  for (pugi::xml_node object = objects.first_child(); object;
       object = object.next_sibling()) {
    if (!strcmp(object.name(), "config")) {
      // Check the GUID for duplicates
      wxString testGUID =
          wxString::FromUTF8(object.attribute("GUID").as_string());

      bool bFound = false;
      for (ConfigObjectList::Node *node = configList->GetFirst(); node;
           node = node->GetNext()) {
        OCPNConfigObject *look = node->GetData();
        if (look->m_GUID == testGUID) {
          bFound = true;
          break;
        }
      }

      if (!bFound) {
        OCPNConfigObject *newConfig = new OCPNConfigObject;

        newConfig->m_GUID =
            wxString::FromUTF8(object.attribute("GUID").as_string());
        newConfig->m_title =
            wxString::FromUTF8(object.attribute("title").as_string());
        newConfig->m_description =
            wxString::FromUTF8(object.attribute("description").as_string());
        newConfig->templateFileName =
            wxString::FromUTF8(object.attribute("templateFile").as_string());

        // Add to the class list of configs
        configList->Append(newConfig);
      }
    }
  }

  return true;
}

bool ConfigMgr::SaveCatalog() {
  m_configCatalog->SaveFile(m_configCatalogName);

  return true;
}

wxString ConfigMgr::CreateNamedConfig(const wxString &title,
                                      const wxString &description,
                                      wxString UUID) {
  wxString GUID;

  // Must have title
  if (title.IsEmpty()) return GUID;

  OCPNConfigObject *pConfig = new OCPNConfigObject;

  // If no UUID is passed, then create a new GUID for this config
  if (UUID.IsEmpty())
    GUID = GetUUID();
  else
    GUID = UUID;

  pConfig->m_GUID = GUID;
  pConfig->m_title = title;
  pConfig->m_description = description;

  if (UUID.IsEmpty()) {
    // create template file name
    pConfig->templateFileName = _T("OCPNTemplate-") + GUID + _T(".conf");

    //  Save the template contents
    wxString templateFullFileName = GetConfigDir() + pConfig->templateFileName;
    if (!SaveTemplate(templateFullFileName)) {
      wxLogMessage(_T("Unable to save template titled: ") + title +
                   _T(" as file: ") + templateFullFileName);
      delete pConfig;
      return _T("");
    }
  }

  // Add this config to the catalog
  if (!m_configCatalog->AddConfig(pConfig, 0)) {
    wxLogMessage(_T("Unable to add config to catalog...Title: ") + title);
    delete pConfig;
    return _T("");
  }

  // Add to the class list of configs
  configList->Append(pConfig);

  if (UUID.IsEmpty()) SaveCatalog();

  return GUID;
}

bool ConfigMgr::DeleteConfig(wxString GUID) {
  OCPNConfigObject *cfg = GetConfig(GUID);
  if (!cfg) return false;

  // Find and delete the template file
  wxString templateFullFileName = GetConfigDir() + cfg->templateFileName;
  if (wxFileExists(templateFullFileName)) wxRemoveFile(templateFullFileName);

  // Remove the config from the catalog
  bool rv = m_configCatalog->RemoveConfig(GUID);

  if (rv) SaveCatalog();

  //  Remove the config from the member list
  bool bDel = configList->DeleteObject(cfg);
  if (bDel) delete cfg;

  return rv;
}

wxPanel *ConfigMgr::GetConfigPanel(wxWindow *parent, wxString GUID) {
  wxPanel *retPanel = NULL;

  // Find the GUID-matching config in the member list
  OCPNConfigObject *config = GetConfig(GUID);

  //  Found it?
  if (config) {
    retPanel = new ConfigPanel(config, parent);
  }

  return retPanel;
}

OCPNConfigObject *ConfigMgr::GetConfig(wxString GUID) {
  // Find the GUID-matching config in the member list
  for (ConfigObjectList::Node *node = configList->GetFirst(); node;
       node = node->GetNext()) {
    OCPNConfigObject *look = node->GetData();
    if (look->m_GUID == GUID) {
      return look;
      break;
    }
  }

  return NULL;
}

wxString ConfigMgr::GetTemplateTitle(wxString GUID) {
  for (ConfigObjectList::Node *node = configList->GetFirst(); node;
       node = node->GetNext()) {
    OCPNConfigObject *look = node->GetData();
    if (look->m_GUID == GUID) {
      return look->m_title;
      break;
    }
  }

  return wxEmptyString;
}

wxArrayString ConfigMgr::GetConfigGUIDArray() {
  wxArrayString ret_val;

  for (ConfigObjectList::Node *node = configList->GetFirst(); node;
       node = node->GetNext()) {
    OCPNConfigObject *look = node->GetData();
    ret_val.Add(look->m_GUID);
  }

  return ret_val;
}

bool ConfigMgr::ApplyConfigGUID(wxString GUID) {
  // Find the GUID-matching config in the member list
  OCPNConfigObject *config = GetConfig(GUID);

  //  Found it?
  if (config) {
    wxString thisConfig = GetConfigDir() + config->templateFileName;

    // Special case for Recovery template
    if (GUID.StartsWith(_T("11111111"))) {
      thisConfig =
          *GetpSharedDataLocation() + _T("configs/OCPNTemplate-Recovery.conf");
    }

    MyConfig fconf(thisConfig);

    //  Load the template contents, without resetting defaults
    fconf.LoadMyConfigRaw(true);

    //  Load Canvas configs, applying only the "templateable" items
    fconf.LoadCanvasConfigs(true);

    if (ps52plib && ps52plib->m_bOK) fconf.LoadS57Config();

    return true;
  }

  return false;
}

// RFC4122 version 4 compliant random UUIDs generator.
wxString ConfigMgr::GetUUID(void) {
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

bool ConfigMgr::SaveTemplate(wxString fileName) {
  //  Assuming the file exists, and is empty....

  //  Create a private wxFileConfig object
  MyConfig *conf = new MyConfig(fileName);

//  Write out all the elements of a config template....

//  Temporarily suppress logging of trivial non-fatal wxLogSysError() messages
//  provoked by Android security...
#ifdef __OCPN__ANDROID__
  wxLogNull logNo;
#endif

  //    Global options and settings
  conf->SetPath(_T ( "/Settings" ));

  conf->Write(_T ( "InlandEcdis" ), g_bInlandEcdis);
  conf->Write(_T ( "UIexpert" ), g_bUIexpert);
  conf->Write(_T ( "SpaceDropMark" ), g_bSpaceDropMark);

  conf->Write(_T ( "ShowStatusBar" ), g_bShowStatusBar);
#ifndef __WXOSX__
  conf->Write(_T ( "ShowMenuBar" ), g_bShowMenuBar);
#endif
  conf->Write(_T ( "DefaultFontSize" ), g_default_font_size);

  conf->Write(_T ( "Fullscreen" ), g_bFullscreen);
  conf->Write(_T ( "ShowCompassWindow" ), g_bShowCompassWin);
  conf->Write(_T ( "SetSystemTime" ), s_bSetSystemTime);
  conf->Write(_T ( "ShowGrid" ), g_bDisplayGrid);
  conf->Write(_T ( "PlayShipsBells" ), g_bPlayShipsBells);
  conf->Write(_T ( "SoundDeviceIndex" ), g_iSoundDeviceIndex);
  conf->Write(_T ( "FullscreenToolbar" ), g_bFullscreenToolbar);
  // conf->Write( _T ( "TransparentToolbar" ), g_bTransparentToolbar );
  conf->Write(_T ( "PermanentMOBIcon" ), g_bPermanentMOBIcon);
  conf->Write(_T ( "ShowLayers" ), g_bShowLayers);
  conf->Write(_T ( "AutoAnchorDrop" ), g_bAutoAnchorMark);
  conf->Write(_T ( "ShowChartOutlines" ), g_bShowOutlines);
  conf->Write(_T ( "ShowActiveRouteTotal" ), g_bShowRouteTotal);
  conf->Write(_T ( "ShowActiveRouteHighway" ), g_bShowActiveRouteHighway);
  conf->Write(_T ( "SDMMFormat" ), g_iSDMMFormat);
  conf->Write(_T ( "ShowChartBar" ), g_bShowChartBar);

  conf->Write(_T ( "GUIScaleFactor" ), g_GUIScaleFactor);
  conf->Write(_T ( "ChartObjectScaleFactor" ), g_ChartScaleFactor);
  conf->Write(_T ( "ShipScaleFactor" ), g_ShipScaleFactor);

  conf->Write(_T ( "ShowTrue" ), g_bShowTrue);
  conf->Write(_T ( "ShowMag" ), g_bShowMag);
  conf->Write(_T ( "UserMagVariation" ),
              wxString::Format(_T("%.2f"), g_UserVar));

  conf->Write(_T ( "CM93DetailFactor" ), g_cm93_zoom_factor);
  conf->Write(_T ( "CM93DetailZoomPosX" ), g_detailslider_dialog_x);
  conf->Write(_T ( "CM93DetailZoomPosY" ), g_detailslider_dialog_y);
  conf->Write(_T ( "ShowCM93DetailSlider" ), g_bShowDetailSlider);

  conf->Write(_T ( "SkewToNorthUp" ), g_bskew_comp);

  conf->Write(_T ( "ZoomDetailFactor" ), g_chart_zoom_modifier_raster);
  conf->Write(_T ( "ZoomDetailFactorVector" ), g_chart_zoom_modifier_vector);

  conf->Write(_T ( "SmoothPanZoom" ), g_bsmoothpanzoom);

  conf->Write(_T ( "CourseUpMode" ), g_bCourseUp);
  if (!g_bInlandEcdis) conf->Write(_T ( "LookAheadMode" ), g_bLookAhead);
  conf->Write(_T ( "COGUPAvgSeconds" ), g_COGAvgSec);
  conf->Write(_T ( "UseMagAPB" ), g_bMagneticAPB);

  conf->Write(_T ( "OwnshipCOGPredictorMinutes" ), g_ownship_predictor_minutes);
  conf->Write(_T ( "OwnshipCOGPredictorWidth" ), g_cog_predictor_width);
  conf->Write(_T ( "OwnshipHDTPredictorMiles" ), g_ownship_HDTpredictor_miles);
  conf->Write(_T ( "OwnShipIconType" ), g_OwnShipIconType);
  conf->Write(_T ( "OwnShipLength" ), g_n_ownship_length_meters);
  conf->Write(_T ( "OwnShipWidth" ), g_n_ownship_beam_meters);
  conf->Write(_T ( "OwnShipGPSOffsetX" ), g_n_gps_antenna_offset_x);
  conf->Write(_T ( "OwnShipGPSOffsetY" ), g_n_gps_antenna_offset_y);
  conf->Write(_T ( "OwnShipMinSize" ), g_n_ownship_min_mm);
  conf->Write(_T ( "OwnShipSogCogCalc" ), g_own_ship_sog_cog_calc);
  conf->Write(_T ( "OwnShipSogCogCalcDampSec"),
              g_own_ship_sog_cog_calc_damp_sec);

  conf->Write(_T ( "RouteArrivalCircleRadius" ),
              wxString::Format(_T("%.3f"), g_n_arrival_circle_radius));
  conf->Write(_T ( "ChartQuilting" ), g_bQuiltEnable);

  conf->Write(_T ( "StartWithTrackActive" ), g_bTrackCarryOver);
  conf->Write(_T ( "AutomaticDailyTracks" ), g_bTrackDaily);
  conf->Write(_T ( "TrackRotateAt" ), g_track_rotate_time);
  conf->Write(_T ( "TrackRotateTimeType" ), g_track_rotate_time_type);
  conf->Write(_T ( "HighlightTracks" ), g_bHighliteTracks);

  conf->Write(_T ( "InitialStackIndex" ), g_restore_stackindex);
  conf->Write(_T ( "InitialdBIndex" ), g_restore_dbindex);

  conf->Write(_T ( "AnchorWatch1GUID" ), g_AW1GUID);
  conf->Write(_T ( "AnchorWatch2GUID" ), g_AW2GUID);

  conf->Write(_T ( "ToolbarX" ), g_maintoolbar_x);
  conf->Write(_T ( "ToolbarY" ), g_maintoolbar_y);
  conf->Write(_T ( "ToolbarOrient" ), g_maintoolbar_orient);

  conf->Write(_T ( "iENCToolbarX" ), g_iENCToolbarPosX);
  conf->Write(_T ( "iENCToolbarY" ), g_iENCToolbarPosY);

  if (!g_bInlandEcdis) {
    conf->Write(_T ( "GlobalToolbarConfig" ), g_toolbarConfig);
    conf->Write(_T ( "DistanceFormat" ), g_iDistanceFormat);
    conf->Write(_T ( "SpeedFormat" ), g_iSpeedFormat);
    conf->Write(_T ( "WindSpeedFormat" ), g_iWindSpeedFormat);
    conf->Write(_T ( "ShowDepthUnits" ), g_bShowDepthUnits);
  }

  conf->Write(_T ( "MobileTouch" ), g_btouch);
  conf->Write(_T ( "ResponsiveGraphics" ), g_bresponsive);

  conf->Write(_T ( "AutoHideToolbar" ), g_bAutoHideToolbar);
  conf->Write(_T ( "AutoHideToolbarSecs" ), g_nAutoHideToolbar);

  conf->Write(_T ( "DisplaySizeMM" ), g_config_display_size_mm);
  conf->Write(_T ( "DisplaySizeManual" ), g_config_display_size_manual);

  conf->Write(_T ( "PlanSpeed" ), wxString::Format(_T("%.2f"), g_PlanSpeed));

#if 0
    wxString vis, invis;
    LayerList::iterator it;
    int index = 0;
    for( it = ( *pLayerList ).begin(); it != ( *pLayerList ).end(); ++it, ++index ) {
        Layer *lay = (Layer *) ( *it );
        if( lay->IsVisibleOnChart() ) vis += ( lay->m_LayerName ) + _T(";");
        else
            invis += ( lay->m_LayerName ) + _T(";");
    }
    conf->Write( _T ( "VisibleLayers" ), vis );
    conf->Write( _T ( "InvisibleLayers" ), invis );
#endif

  conf->Write(_T ( "Locale" ), g_locale);
  conf->Write(_T ( "LocaleOverride" ), g_localeOverride);

  // LIVE ETA OPTION
  conf->Write(_T( "LiveETA" ), g_bShowLiveETA);
  conf->Write(_T( "DefaultBoatSpeed" ), g_defaultBoatSpeed);

  //    S57 Object Filter Settings
  conf->SetPath(_T ( "/Settings/ObjectFilter" ));

  if (ps52plib) {
    for (unsigned int iPtr = 0; iPtr < ps52plib->pOBJLArray->GetCount();
         iPtr++) {
      OBJLElement *pOLE = (OBJLElement *)(ps52plib->pOBJLArray->Item(iPtr));

      wxString st1(_T ( "viz" ));
      char name[7];
      strncpy(name, pOLE->OBJLName, 6);
      name[6] = 0;
      st1.Append(wxString(name, wxConvUTF8));
      conf->Write(st1, pOLE->nViz);
    }
  }

  //    Global State

  conf->SetPath(_T ( "/Settings/GlobalState" ));

  //    Various Options
  if (!g_bInlandEcdis)
    conf->Write(_T ( "nColorScheme" ), (int)gFrame->GetColorScheme());

  //    AIS
  conf->SetPath(_T ( "/Settings/AIS" ));

  conf->Write(_T ( "bNoCPAMax" ), g_bCPAMax);
  conf->Write(_T ( "NoCPAMaxNMi" ), g_CPAMax_NM);
  conf->Write(_T ( "bCPAWarn" ), g_bCPAWarn);
  conf->Write(_T ( "CPAWarnNMi" ), g_CPAWarn_NM);
  conf->Write(_T ( "bTCPAMax" ), g_bTCPA_Max);
  conf->Write(_T ( "TCPAMaxMinutes" ), g_TCPA_Max);
  conf->Write(_T ( "bMarkLostTargets" ), g_bMarkLost);
  conf->Write(_T ( "MarkLost_Minutes" ), g_MarkLost_Mins);
  conf->Write(_T ( "bRemoveLostTargets" ), g_bRemoveLost);
  conf->Write(_T ( "RemoveLost_Minutes" ), g_RemoveLost_Mins);
  conf->Write(_T ( "bShowCOGArrows" ), g_bShowCOG);
  conf->Write(_T ( "CogArrowMinutes" ), g_ShowCOG_Mins);
  conf->Write(_T ( "bShowTargetTracks" ), g_bAISShowTracks);
  conf->Write(_T ( "TargetTracksMinutes" ), g_AISShowTracks_Mins);

  conf->Write(_T ( "bHideMooredTargets" ), g_bHideMoored);
  conf->Write(_T ( "MooredTargetMaxSpeedKnots" ), g_ShowMoored_Kts);

  conf->Write(_T ( "bAISAlertDialog" ), g_bAIS_CPA_Alert);
  conf->Write(_T ( "bAISAlertAudio" ), g_bAIS_CPA_Alert_Audio);
  conf->Write(_T ( "AISAlertAudioFile" ), g_sAIS_Alert_Sound_File);
  conf->Write(_T ( "bAISAlertSuppressMoored" ),
              g_bAIS_CPA_Alert_Suppress_Moored);
  conf->Write(_T ( "bShowAreaNotices" ), g_bShowAreaNotices);
  conf->Write(_T ( "bDrawAISSize" ), g_bDrawAISSize);
  conf->Write(_T ( "bDrawAISRealtime" ), g_bDrawAISRealtime);
  conf->Write(_T ( "AISRealtimeMinSpeedKnots" ), g_AIS_RealtPred_Kts);
  conf->Write(_T ( "bShowAISName" ), g_bShowAISName);
  conf->Write(_T ( "ShowAISTargetNameScale" ), g_Show_Target_Name_Scale);
  conf->Write(_T ( "bWplIsAprsPositionReport" ), g_bWplUsePosition);
  conf->Write(_T ( "WplSelAction" ), g_WplAction);
  conf->Write(_T ( "AISCOGPredictorWidth" ), g_ais_cog_predictor_width);
  conf->Write(_T ( "bShowScaledTargets" ), g_bAllowShowScaled);
  conf->Write(_T ( "AISScaledNumber" ), g_ShowScaled_Num);
  conf->Write(_T ( "AISScaledNumberWeightSOG" ), g_ScaledNumWeightSOG);
  conf->Write(_T ( "AISScaledNumberWeightCPA" ), g_ScaledNumWeightCPA);
  conf->Write(_T ( "AISScaledNumberWeightTCPA" ), g_ScaledNumWeightTCPA);
  conf->Write(_T ( "AISScaledNumberWeightRange" ), g_ScaledNumWeightRange);
  conf->Write(_T ( "AISScaledNumberWeightSizeOfTarget" ),
              g_ScaledNumWeightSizeOfT);
  conf->Write(_T ( "AISScaledSizeMinimal" ), g_ScaledSizeMinimal);
  conf->Write(_T ( "AISShowScaled"), g_bShowScaled);

  conf->Write(_T ( "AlertDialogSizeX" ), g_ais_alert_dialog_sx);
  conf->Write(_T ( "AlertDialogSizeY" ), g_ais_alert_dialog_sy);
  conf->Write(_T ( "AlertDialogPosX" ), g_ais_alert_dialog_x);
  conf->Write(_T ( "AlertDialogPosY" ), g_ais_alert_dialog_y);
  conf->Write(_T ( "QueryDialogPosX" ), g_ais_query_dialog_x);
  conf->Write(_T ( "QueryDialogPosY" ), g_ais_query_dialog_y);
  conf->Write(_T ( "AISTargetListPerspective" ), g_AisTargetList_perspective);
  conf->Write(_T ( "AISTargetListRange" ), g_AisTargetList_range);
  conf->Write(_T ( "AISTargetListSortColumn" ), g_AisTargetList_sortColumn);
  conf->Write(_T ( "bAISTargetListSortReverse" ), g_bAisTargetList_sortReverse);
  conf->Write(_T ( "AISTargetListColumnSpec" ), g_AisTargetList_column_spec);
  conf->Write(_T ( "AISTargetListColumnOrder" ), g_AisTargetList_column_order);
  conf->Write(_T ( "S57QueryDialogSizeX" ), g_S57_dialog_sx);
  conf->Write(_T ( "S57QueryDialogSizeY" ), g_S57_dialog_sy);
  conf->Write(_T ( "bAISRolloverShowClass" ), g_bAISRolloverShowClass);
  conf->Write(_T ( "bAISRolloverShowCOG" ), g_bAISRolloverShowCOG);
  conf->Write(_T ( "bAISRolloverShowCPA" ), g_bAISRolloverShowCPA);
  conf->Write(_T ( "bAISAlertAckTimeout" ), g_bAIS_ACK_Timeout);
  conf->Write(_T ( "AlertAckTimeoutMinutes" ), g_AckTimeout_Mins);

  conf->SetPath(_T ( "/Settings/GlobalState" ));
  if (ps52plib) {
    conf->Write(_T ( "bShowS57Text" ), ps52plib->GetShowS57Text());
    conf->Write(_T ( "bShowS57ImportantTextOnly" ),
                ps52plib->GetShowS57ImportantTextOnly());
    if (!g_bInlandEcdis)
      conf->Write(_T ( "nDisplayCategory" ),
                  (long)ps52plib->GetDisplayCategory());
    conf->Write(_T ( "nSymbolStyle" ), (int)ps52plib->m_nSymbolStyle);
    conf->Write(_T ( "nBoundaryStyle" ), (int)ps52plib->m_nBoundaryStyle);

    conf->Write(_T ( "bShowSoundg" ), ps52plib->m_bShowSoundg);
    conf->Write(_T ( "bShowMeta" ), ps52plib->m_bShowMeta);
    conf->Write(_T ( "bUseSCAMIN" ), ps52plib->m_bUseSCAMIN);
    conf->Write(_T ( "bUseSUPER_SCAMIN" ), ps52plib->m_bUseSUPER_SCAMIN);
    conf->Write(_T ( "bShowAtonText" ), ps52plib->m_bShowAtonText);
    conf->Write(_T ( "bShowLightDescription" ), ps52plib->m_bShowLdisText);
    conf->Write(_T ( "bExtendLightSectors" ), ps52plib->m_bExtendLightSectors);
    conf->Write(_T ( "bDeClutterText" ), ps52plib->m_bDeClutterText);
    conf->Write(_T ( "bShowNationalText" ), ps52plib->m_bShowNationalTexts);

    conf->Write(_T ( "S52_MAR_SAFETY_CONTOUR" ),
                S52_getMarinerParam(S52_MAR_SAFETY_CONTOUR));
    conf->Write(_T ( "S52_MAR_SHALLOW_CONTOUR" ),
                S52_getMarinerParam(S52_MAR_SHALLOW_CONTOUR));
    conf->Write(_T ( "S52_MAR_DEEP_CONTOUR" ),
                S52_getMarinerParam(S52_MAR_DEEP_CONTOUR));
    conf->Write(_T ( "S52_MAR_TWO_SHADES" ),
                S52_getMarinerParam(S52_MAR_TWO_SHADES));
    conf->Write(_T ( "S52_DEPTH_UNIT_SHOW" ), ps52plib->m_nDepthUnitDisplay);
  }

  conf->SetPath(_T ( "/Settings/Others" ));

  // Radar rings
  conf->Write(_T ( "ShowRadarRings" ), (bool)(g_iNavAidRadarRingsNumberVisible >
                                              0));  // 3.0.0 config support
  conf->Write(_T ( "RadarRingsNumberVisible" ),
              g_iNavAidRadarRingsNumberVisible);
  conf->Write(_T ( "RadarRingsStep" ), g_fNavAidRadarRingsStep);
  conf->Write(_T ( "RadarRingsStepUnits" ), g_pNavAidRadarRingsStepUnits);
  conf->Write(_T ( "RadarRingsColour" ),
              g_colourOwnshipRangeRingsColour.GetAsString(wxC2S_HTML_SYNTAX));

  // Waypoint Radar rings
  conf->Write(_T ( "WaypointRangeRingsNumber" ), g_iWaypointRangeRingsNumber);
  conf->Write(_T ( "WaypointRangeRingsStep" ), g_fWaypointRangeRingsStep);
  conf->Write(_T ( "WaypointRangeRingsStepUnits" ),
              g_iWaypointRangeRingsStepUnits);
  conf->Write(_T ( "WaypointRangeRingsColour" ),
              g_colourWaypointRangeRingsColour.GetAsString(wxC2S_HTML_SYNTAX));

  conf->Write(_T ( "ConfirmObjectDeletion" ), g_bConfirmObjectDelete);

  // Waypoint dragging with mouse
  conf->Write(_T ( "WaypointPreventDragging" ), g_bWayPointPreventDragging);

  conf->Write(_T ( "EnableZoomToCursor" ), g_bEnableZoomToCursor);

  conf->Write(_T ( "TrackIntervalSeconds" ), g_TrackIntervalSeconds);
  conf->Write(_T ( "TrackDeltaDistance" ), g_TrackDeltaDistance);
  conf->Write(_T ( "TrackPrecision" ), g_nTrackPrecision);

  conf->Write(_T ( "RouteLineWidth" ), g_route_line_width);
  conf->Write(_T ( "TrackLineWidth" ), g_track_line_width);
  conf->Write(_T ( "TrackLineColour" ),
              g_colourTrackLineColour.GetAsString(wxC2S_HTML_SYNTAX));
  conf->Write(_T ( "DefaultWPIcon" ), g_default_wp_icon);

  //    Fonts

  //  Store the persistent Auxiliary Font descriptor Keys
  conf->SetPath(_T ( "/Settings/AuxFontKeys" ));

  wxArrayString keyArray = FontMgr::Get().GetAuxKeyArray();
  for (unsigned int i = 0; i < keyArray.GetCount(); i++) {
    wxString key;
    key.Printf(_T("Key%i"), i);
    wxString keyval = keyArray[i];
    conf->Write(key, keyval);
  }

  wxString font_path;
#ifdef __WXX11__
  font_path = (_T ( "/Settings/X11Fonts" ));
#endif

#ifdef __WXGTK__
  font_path = (_T ( "/Settings/GTKFonts" ));
#endif

#ifdef __WXMSW__
  font_path = (_T ( "/Settings/MSWFonts" ));
#endif

#ifdef __WXMAC__
  font_path = (_T ( "/Settings/MacFonts" ));
#endif

#ifdef __WXQT__
  font_path = (_T ( "/Settings/QTFonts" ));
#endif

  conf->DeleteGroup(font_path);

  conf->SetPath(font_path);

  int nFonts = FontMgr::Get().GetNumFonts();

  for (int i = 0; i < nFonts; i++) {
    wxString cfstring(FontMgr::Get().GetConfigString(i));
    wxString valstring = FontMgr::Get().GetFullConfigDesc(i);
    conf->Write(cfstring, valstring);
  }

  //  Save the per-canvas config options
  conf->SaveCanvasConfigs();

  conf->Flush();

  delete conf;

  return true;
}

bool ConfigMgr::CheckTemplateGUID(wxString GUID) {
  bool rv = false;

  OCPNConfigObject *config = GetConfig(GUID);
  if (config) {
    rv = CheckTemplate(GetConfigDir() + config->templateFileName);
  }

  return rv;
}

#define CHECK_INT(s, t)                           \
  read_int = *t;                                  \
  if (!conf->Read(s, &read_int)) wxLogMessage(s); \
  if ((int)*t != read_int) return false;

#define CHECK_STR(s, t) \
  val = t;              \
  conf->Read(s, &val);  \
  if (!t.IsSameAs(val)) return false;

#define CHECK_STRP(s, t) \
  conf->Read(s, &val);   \
  if (!t->IsSameAs(val)) return false;

#define CHECK_FLT(s, t, eps) \
  conf->Read(s, &val);       \
  val.ToDouble(&dval);       \
  if (fabs(dval - *t) > eps) return false;

bool ConfigMgr::CheckTemplate(wxString fileName) {
  bool rv = true;

  int read_int;
  wxString val;
  double dval;

  MyConfig *conf = new MyConfig(fileName);

  //    Global options and settings
  conf->SetPath(_T ( "/Settings" ));

  CHECK_INT(_T ( "UIexpert" ), &g_bUIexpert);

  /// CHECK_STR( _T ( "UIStyle" ), g_uiStyle  );

  CHECK_INT(_T ( "InlandEcdis" ), &g_bInlandEcdis);

  CHECK_INT(_T( "SpaceDropMark" ), &g_bSpaceDropMark);

  /// CHECK_INT( _T ( "UseModernUI5" ), &g_useMUI );

#if 0
    CHECK_INT( _T ( "DebugGDAL" ), &g_bGDAL_Debug );
    CHECK_INT( _T ( "DebugNMEA" ), &g_nNMEADebug );
    CHECK_INT( _T ( "DebugOpenGL" ), &g_bDebugOGL );
    ///CHECK_INT( _T ( "AnchorWatchDefault" ), &g_nAWDefault );
    ///CHECK_INT( _T ( "AnchorWatchMax" ), &g_nAWMax );
    ///CHECK_INT( _T ( "GPSDogTimeout" ), &gps_watchdog_timeout_ticks );
    CHECK_INT( _T ( "DebugCM93" ), &g_bDebugCM93 );
    CHECK_INT( _T ( "DebugS57" ), &g_bDebugS57 );         // Show LUP and Feature info in object query
    CHECK_INT( _T ( "DebugBSBImg" ), &g_BSBImgDebug );
    CHECK_INT( _T ( "DebugGPSD" ), &g_bDebugGPSD );
#endif

  CHECK_INT(_T ( "DefaultFontSize"), &g_default_font_size);

  //    Read( _T ( "GPSIdent" ), &g_GPS_Ident );
  CHECK_INT(_T ( "UseGarminHostUpload" ), &g_bGarminHostUpload);

  CHECK_INT(_T ( "UseNMEA_GLL" ), &g_bUseGLL);

  CHECK_INT(_T ( "AutoHideToolbar" ), &g_bAutoHideToolbar);
  CHECK_INT(_T ( "AutoHideToolbarSecs" ), &g_nAutoHideToolbar);

  CHECK_INT(_T ( "UseSimplifiedScalebar" ), &g_bsimplifiedScalebar);

  CHECK_INT(_T ( "DisplaySizeMM" ), &g_display_size_mm);
  CHECK_INT(_T ( "DisplaySizeManual" ), &g_config_display_size_manual);

  CHECK_INT(_T ( "GUIScaleFactor" ), &g_GUIScaleFactor);

  CHECK_INT(_T ( "ChartObjectScaleFactor" ), &g_ChartScaleFactor);
  CHECK_INT(_T ( "ShipScaleFactor" ), &g_ShipScaleFactor);

  CHECK_INT(_T ( "FilterNMEA_Avg" ), &g_bfilter_cogsog);
  CHECK_INT(_T ( "FilterNMEA_Sec" ), &g_COGFilterSec);

  CHECK_INT(_T ( "ShowTrue" ), &g_bShowTrue);
  CHECK_INT(_T ( "ShowMag" ), &g_bShowMag);

  CHECK_FLT(_T ( "UserMagVariation" ), &g_UserVar, 0.1)

  CHECK_INT(_T ( "UseMagAPB" ), &g_bMagneticAPB);

  CHECK_INT(_T ( "ScreenBrightness" ), &g_nbrightness);

  CHECK_INT(_T ( "MemFootprintTargetMB" ), &g_MemFootMB);

  CHECK_INT(_T ( "WindowsComPortMax" ), &g_nCOMPortCheck);

  CHECK_INT(_T ( "ChartQuilting" ), &g_bQuiltEnable);
  CHECK_INT(_T ( "ChartQuiltingInitial" ), &g_bQuiltStart);

  CHECK_INT(_T ( "CourseUpMode" ), &g_bCourseUp);
  CHECK_INT(_T ( "COGUPAvgSeconds" ), &g_COGAvgSec);
  // CHECK_INT( _T ( "LookAheadMode" ), &g_bLookAhead );
  // CHECK_INT( _T ( "SkewToNorthUp" ), &g_bskew_comp );
  CHECK_INT(_T ( "OpenGL" ), &g_bopengl);
  CHECK_INT(_T ( "SoftwareGL" ), &g_bSoftwareGL);

  CHECK_INT(_T ( "ShowFPS" ), &g_bShowFPS);

  CHECK_INT(_T( "NMEAAPBPrecision" ), &g_NMEAAPBPrecision);

  CHECK_STR(_T( "TalkerIdText" ), g_TalkerIdText);
  CHECK_INT(_T( "MaxWaypointNameLength" ), &g_maxWPNameLength);

  /* opengl options */
#ifdef ocpnUSE_GL
  CHECK_INT(_T ( "OpenGLExpert" ), &g_bGLexpert);
  CHECK_INT(_T ( "UseAcceleratedPanning" ),
            &g_GLOptions.m_bUseAcceleratedPanning);
  CHECK_INT(_T ( "GPUTextureCompression" ), &g_GLOptions.m_bTextureCompression);
  CHECK_INT(_T ( "GPUTextureCompressionCaching" ),
            &g_GLOptions.m_bTextureCompressionCaching);
  CHECK_INT(_T ( "PolygonSmoothing" ), &g_GLOptions.m_GLPolygonSmoothing);
  CHECK_INT(_T ( "LineSmoothing" ), &g_GLOptions.m_GLLineSmoothing);
  CHECK_INT(_T ( "GPUTextureDimension" ), &g_GLOptions.m_iTextureDimension);
  CHECK_INT(_T ( "GPUTextureMemSize" ), &g_GLOptions.m_iTextureMemorySize);

#endif
  CHECK_INT(_T ( "SmoothPanZoom" ), &g_bsmoothpanzoom);

  CHECK_INT(_T ( "ToolbarX"), &g_maintoolbar_x);
  CHECK_INT(_T ( "ToolbarY" ), &g_maintoolbar_y);
  CHECK_INT(_T ( "ToolbarOrient" ), &g_maintoolbar_orient);
  CHECK_STR(_T ( "GlobalToolbarConfig" ), g_toolbarConfig);

  CHECK_INT(_T ( "iENCToolbarX"), &g_iENCToolbarPosX);
  CHECK_INT(_T ( "iENCToolbarY"), &g_iENCToolbarPosY);

  CHECK_STR(_T ( "AnchorWatch1GUID" ), g_AW1GUID);
  CHECK_STR(_T ( "AnchorWatch2GUID" ), g_AW2GUID);

  CHECK_INT(_T ( "MobileTouch" ), &g_btouch);
  CHECK_INT(_T ( "ResponsiveGraphics" ), &g_bresponsive);

  CHECK_INT(_T ( "ZoomDetailFactor" ), &g_chart_zoom_modifier_raster);
  CHECK_INT(_T ( "ZoomDetailFactorVector" ), &g_chart_zoom_modifier_vector);

  CHECK_INT(_T ( "CM93DetailFactor" ), &g_cm93_zoom_factor);
  CHECK_INT(_T ( "CM93DetailZoomPosX" ), &g_detailslider_dialog_x);
  CHECK_INT(_T ( "CM93DetailZoomPosY" ), &g_detailslider_dialog_y);
  CHECK_INT(_T ( "ShowCM93DetailSlider" ), &g_bShowDetailSlider);

  CHECK_INT(_T ( "SENC_LOD_Pixels" ), &g_SENC_LOD_pixels);

  CHECK_INT(_T ( "SkewCompUpdatePeriod" ), &g_SkewCompUpdatePeriod);

  CHECK_INT(_T ( "ShowStatusBar" ), &g_bShowStatusBar);
#ifndef __WXOSX__
  CHECK_INT(_T ( "ShowMenuBar" ), &g_bShowMenuBar);
#endif
  CHECK_INT(_T ( "Fullscreen" ), &g_bFullscreen);
  CHECK_INT(_T ( "ShowCompassWindow" ), &g_bShowCompassWin);
  CHECK_INT(_T ( "PlayShipsBells" ), &g_bPlayShipsBells);
  CHECK_INT(_T ( "SoundDeviceIndex" ), &g_iSoundDeviceIndex);
  CHECK_INT(_T ( "FullscreenToolbar" ), &g_bFullscreenToolbar);
  // CHECK_INT( _T ( "TransparentToolbar" ), &g_bTransparentToolbar );
  CHECK_INT(_T ( "PermanentMOBIcon" ), &g_bPermanentMOBIcon);
  CHECK_INT(_T ( "ShowLayers" ), &g_bShowLayers);
  CHECK_INT(_T ( "ShowDepthUnits" ), &g_bShowDepthUnits);
  CHECK_INT(_T ( "AutoAnchorDrop" ), &g_bAutoAnchorMark);
  CHECK_INT(_T ( "ShowActiveRouteHighway" ), &g_bShowActiveRouteHighway);
  CHECK_INT(_T ( "ShowActiveRouteTotal" ), &g_bShowRouteTotal);
  CHECK_STR(_T ( "MostRecentGPSUploadConnection" ), g_uploadConnection);
  CHECK_INT(_T ( "ShowChartBar" ), &g_bShowChartBar);
  CHECK_INT(_T ( "SDMMFormat" ),
            &g_iSDMMFormat);  // 0 = "Degrees, Decimal minutes"), 1 = "Decimal
                              // degrees", 2 = "Degrees,Minutes, Seconds"

  CHECK_INT(_T ( "DistanceFormat" ),
            &g_iDistanceFormat);  // 0 = "Nautical miles"), 1 = "Statute miles",
                                  // 2 = "Kilometers", 3 = "Meters"
  CHECK_INT(_T ( "SpeedFormat" ),
            &g_iSpeedFormat);  // 0 = "kts"), 1 = "mph", 2 = "km/h", 3 = "m/s"
  CHECK_INT(_T ( "WindSpeedFormat" ),
            &g_iWindSpeedFormat);  // 0 = "knots"), 1 = "m/s", 2 = "Mph", 3 = "km/h"

  // LIVE ETA OPTION
  CHECK_INT(_T ( "LiveETA" ), &g_bShowLiveETA);
  CHECK_INT(_T ( "DefaultBoatSpeed" ), &g_defaultBoatSpeed);

  CHECK_INT(_T ( "OwnshipCOGPredictorMinutes" ), &g_ownship_predictor_minutes);
  CHECK_INT(_T ( "OwnshipCOGPredictorWidth" ), &g_cog_predictor_width);
  CHECK_INT(_T ( "OwnshipHDTPredictorMiles" ), &g_ownship_HDTpredictor_miles);

  CHECK_INT(_T ( "OwnShipIconType" ), &g_OwnShipIconType);
  CHECK_FLT(_T ( "OwnShipLength" ), &g_n_ownship_length_meters, 0.1);
  CHECK_FLT(_T ( "OwnShipWidth" ), &g_n_ownship_beam_meters, 0.1);
  CHECK_FLT(_T ( "OwnShipGPSOffsetX" ), &g_n_gps_antenna_offset_x, 0.1);
  CHECK_FLT(_T ( "OwnShipGPSOffsetY" ), &g_n_gps_antenna_offset_y, 0.1);
  CHECK_INT(_T ( "OwnShipMinSize" ), &g_n_ownship_min_mm);
  CHECK_INT(_T ( "OwnShipSogCogCalc" ), &g_own_ship_sog_cog_calc);
  CHECK_INT(_T ( "OwnShipSogCogCalcDampSec"),
            &g_own_ship_sog_cog_calc_damp_sec);

  CHECK_FLT(_T ( "RouteArrivalCircleRadius" ), &g_n_arrival_circle_radius, .01);

  CHECK_INT(_T ( "FullScreenQuilt" ), &g_bFullScreenQuilt);

  CHECK_INT(_T ( "StartWithTrackActive" ), &g_bTrackCarryOver);
  CHECK_INT(_T ( "AutomaticDailyTracks" ), &g_bTrackDaily);
  CHECK_INT(_T ( "TrackRotateAt" ), &g_track_rotate_time);
  CHECK_INT(_T ( "TrackRotateTimeType" ), &g_track_rotate_time_type);
  CHECK_INT(_T ( "HighlightTracks" ), &g_bHighliteTracks);

  CHECK_FLT(_T ( "PlanSpeed" ), &g_PlanSpeed, 0.1)

  /// CHECK_STR( _T ( "VisibleLayers" ), g_VisibleLayers );
  /// CHECK_STR( _T ( "InvisibleLayers" ), g_InvisibleLayers );

  CHECK_INT(_T ( "PreserveScaleOnX" ), &g_bPreserveScaleOnX);

  CHECK_STR(_T ( "Locale" ), g_locale);
  CHECK_STR(_T ( "LocaleOverride" ), g_localeOverride);

  // We allow 0-99 backups ov navobj.xml
  CHECK_INT(_T ( "KeepNavobjBackups" ), &g_navobjbackups);

  //     NMEALogWindow::Get().SetSize(Read(_T("NMEALogWindowSizeX"), 600L),
  //     Read(_T("NMEALogWindowSizeY"), 400L));
  //     NMEALogWindow::Get().SetPos(Read(_T("NMEALogWindowPosX"), 10L),
  //     Read(_T("NMEALogWindowPosY"), 10L));
  //     NMEALogWindow::Get().CheckPos(display_width, display_height);

  // Boolean to cater for legacy Input COM Port filer behaviour, i.e. show msg
  // filtered but put msg on bus.
  CHECK_INT(_T ( "LegacyInputCOMPortFilterBehaviour" ),
            &g_b_legacy_input_filter_behaviour);

  CHECK_INT(_T( "AdvanceRouteWaypointOnArrivalOnly" ),
            &g_bAdvanceRouteWaypointOnArrivalOnly);

  CHECK_INT(_T ( "EnableRotateKeys" ), &g_benable_rotate);
  CHECK_INT(_T ( "EmailCrashReport" ), &g_bEmailCrashReport);

  CHECK_INT(_T ( "EnableAISNameCache" ), &g_benableAISNameCache);

  CHECK_INT(_T ( "EnableUDPNullHeader" ), &g_benableUDPNullHeader);

  conf->SetPath(_T ( "/Settings/GlobalState" ));

  CHECK_INT(_T ( "FrameWinX" ), &g_nframewin_x);
  CHECK_INT(_T ( "FrameWinY" ), &g_nframewin_y);
  CHECK_INT(_T ( "FrameWinPosX" ), &g_nframewin_posx);
  CHECK_INT(_T ( "FrameWinPosY" ), &g_nframewin_posy);
  CHECK_INT(_T ( "FrameMax" ), &g_bframemax);

  CHECK_INT(_T ( "ClientPosX" ), &g_lastClientRectx);
  CHECK_INT(_T ( "ClientPosY" ), &g_lastClientRecty);
  CHECK_INT(_T ( "ClientSzX" ), &g_lastClientRectw);
  CHECK_INT(_T ( "ClientSzY" ), &g_lastClientRecth);

  CHECK_INT(_T( "RoutePropSizeX" ), &g_route_prop_sx);
  CHECK_INT(_T( "RoutePropSizeY" ), &g_route_prop_sy);
  CHECK_INT(_T( "RoutePropPosX" ), &g_route_prop_x);
  CHECK_INT(_T( "RoutePropPosY" ), &g_route_prop_y);

  CHECK_INT(_T ( "S52_DEPTH_UNIT_SHOW" ),
            &g_nDepthUnitDisplay);  // default is metres

  //    AIS
  conf->SetPath(_T ( "/Settings/AIS" ));
  CHECK_INT(_T ( "bNoCPAMax" ), &g_bCPAMax);
  CHECK_FLT(_T ( "NoCPAMaxNMi" ), &g_CPAMax_NM, .01)
  CHECK_INT(_T ( "bCPAWarn" ), &g_bCPAWarn);
  CHECK_FLT(_T ( "CPAWarnNMi" ), &g_CPAWarn_NM, .01)
  CHECK_INT(_T ( "bTCPAMax" ), &g_bTCPA_Max);
  CHECK_FLT(_T ( "TCPAMaxMinutes" ), &g_TCPA_Max, 1)
  CHECK_INT(_T ( "bMarkLostTargets" ), &g_bMarkLost);
  CHECK_FLT(_T ( "MarkLost_Minutes" ), &g_MarkLost_Mins, 1)
  CHECK_INT(_T ( "bRemoveLostTargets" ), &g_bRemoveLost);
  CHECK_FLT(_T ( "RemoveLost_Minutes" ), &g_RemoveLost_Mins, 1)
  CHECK_INT(_T ( "bShowCOGArrows" ), &g_bShowCOG);
  CHECK_INT(_T ( "bSyncCogPredictors" ), &g_bSyncCogPredictors);
  CHECK_FLT(_T ( "CogArrowMinutes" ), &g_ShowCOG_Mins, 1);
  CHECK_INT(_T ( "bShowTargetTracks" ), &g_bAISShowTracks);
  CHECK_FLT(_T ( "TargetTracksMinutes" ), &g_AISShowTracks_Mins, 1)
  CHECK_FLT(_T ( "TargetTracksLimit" ), &g_AISShowTracks_Limit, 300)
  CHECK_INT(_T ( "bHideMooredTargets" ), &g_bHideMoored)
  CHECK_FLT(_T ( "MooredTargetMaxSpeedKnots" ), &g_ShowMoored_Kts, .1)
  CHECK_INT(_T ( "bShowScaledTargets"), &g_bAllowShowScaled);
  CHECK_INT(_T ( "AISScaledNumber" ), &g_ShowScaled_Num);
  CHECK_INT(_T ( "AISScaledNumberWeightSOG" ), &g_ScaledNumWeightSOG);
  CHECK_INT(_T ( "AISScaledNumberWeightCPA" ), &g_ScaledNumWeightCPA);
  CHECK_INT(_T ( "AISScaledNumberWeightTCPA" ), &g_ScaledNumWeightTCPA);
  CHECK_INT(_T ( "AISScaledNumberWeightRange" ), &g_ScaledNumWeightRange);
  CHECK_INT(_T ( "AISScaledNumberWeightSizeOfTarget" ),
            &g_ScaledNumWeightSizeOfT);
  CHECK_INT(_T ( "AISScaledSizeMinimal" ), &g_ScaledSizeMinimal);
  CHECK_INT(_T(  "AISShowScaled"), &g_bShowScaled);
  CHECK_INT(_T ( "bShowAreaNotices" ), &g_bShowAreaNotices);
  CHECK_INT(_T ( "bDrawAISSize" ), &g_bDrawAISSize);
  CHECK_INT(_T ( "bDrawAISRealtime" ), &g_bDrawAISRealtime);
  CHECK_FLT(_T ( "AISRealtimeMinSpeedKnots" ), &g_AIS_RealtPred_Kts, .1);
  CHECK_INT(_T ( "bShowAISName" ), &g_bShowAISName);
  CHECK_INT(_T ( "bAISAlertDialog" ), &g_bAIS_CPA_Alert);
  CHECK_INT(_T ( "ShowAISTargetNameScale" ), &g_Show_Target_Name_Scale);
  CHECK_INT(_T ( "bWplIsAprsPositionReport" ), &g_bWplUsePosition);
  CHECK_INT(_T ( "WplSelAction" ), &g_WplAction);
  CHECK_INT(_T ( "AISCOGPredictorWidth" ), &g_ais_cog_predictor_width);
  CHECK_INT(_T ( "bAISAlertAudio" ), &g_bAIS_CPA_Alert_Audio);
  CHECK_STR(_T ( "AISAlertAudioFile" ), g_sAIS_Alert_Sound_File);
  CHECK_INT(_T ( "bAISAlertSuppressMoored" ),
            &g_bAIS_CPA_Alert_Suppress_Moored);
  CHECK_INT(_T ( "bAISAlertAckTimeout" ), &g_bAIS_ACK_Timeout);
  CHECK_FLT(_T ( "AlertAckTimeoutMinutes" ), &g_AckTimeout_Mins, 1)
  CHECK_STR(_T ( "AISTargetListPerspective" ), g_AisTargetList_perspective);
  CHECK_INT(_T ( "AISTargetListRange" ), &g_AisTargetList_range);
  CHECK_INT(_T ( "AISTargetListSortColumn" ), &g_AisTargetList_sortColumn);
  CHECK_INT(_T ( "bAISTargetListSortReverse" ), &g_bAisTargetList_sortReverse);
  CHECK_STR(_T ( "AISTargetListColumnSpec" ), g_AisTargetList_column_spec);
  CHECK_STR(_T ("AISTargetListColumnOrder"), g_AisTargetList_column_order);
  CHECK_INT(_T ( "bAISRolloverShowClass" ), &g_bAISRolloverShowClass);
  CHECK_INT(_T ( "bAISRolloverShowCOG" ), &g_bAISRolloverShowCOG);
  CHECK_INT(_T ( "bAISRolloverShowCPA" ), &g_bAISRolloverShowCPA);

  CHECK_INT(_T ( "S57QueryDialogSizeX" ), &g_S57_dialog_sx);
  CHECK_INT(_T ( "S57QueryDialogSizeY" ), &g_S57_dialog_sy);
  CHECK_INT(_T ( "AlertDialogSizeX" ), &g_ais_alert_dialog_sx);
  CHECK_INT(_T ( "AlertDialogSizeY" ), &g_ais_alert_dialog_sy);
  CHECK_INT(_T ( "AlertDialogPosX" ), &g_ais_alert_dialog_x);
  CHECK_INT(_T ( "AlertDialogPosY" ), &g_ais_alert_dialog_y);
  CHECK_INT(_T ( "QueryDialogPosX" ), &g_ais_query_dialog_x);
  CHECK_INT(_T ( "QueryDialogPosY" ), &g_ais_query_dialog_y);

  conf->SetPath(_T ( "/Directories" ));
  CHECK_STR(_T ( "PresentationLibraryData" ), g_UserPresLibData)
  /// CHECK_STRP( _T ( "InitChartDir" ), pInit_Chart_Dir)

  CHECK_STR(_T ( "SENCFileLocation" ), g_SENCPrefix)

  CHECK_STR(_T ( "GPXIODir" ), g_gpx_path);     // Get the Directory name
  CHECK_STR(_T ( "TCDataDir" ), g_TCData_Dir);  // Get the Directory name
  CHECK_STR(_T ( "BasemapDir"), gWorldMapLocation);

  //    Fonts

#if 0
    //  Load the persistent Auxiliary Font descriptor Keys
    conf->SetPath ( _T ( "/Settings/AuxFontKeys" ) );

    wxString strk;
    long dummyk;
    wxString kval;
    bool bContk = conf->GetFirstEntry( strk, dummyk );
    bool bNewKey = false;
    while( bContk ) {
        Read( strk, &kval );
        bNewKey = FontMgr::Get().AddAuxKey(kval);
        if(!bNewKey) {
            DeleteEntry( strk );
            dummyk--;
        }
        bContk = GetNextEntry( strk, dummyk );
    }
#endif

#ifdef __WXX11__
  conf->SetPath(_T ( "/Settings/X11Fonts" ));
#endif

#ifdef __WXGTK__
  conf->SetPath(_T ( "/Settings/GTKFonts" ));
#endif

#ifdef __WXMSW__
  conf->SetPath(_T ( "/Settings/MSWFonts" ));
#endif

#ifdef __WXMAC__
  conf->SetPath(_T ( "/Settings/MacFonts" ));
#endif

#ifdef __WXQT__
  conf->SetPath(_T ( "/Settings/QTFonts" ));
#endif

  conf->SetPath(_T ( "/Settings/Others" ));

  // Radar rings
  CHECK_INT(_T ( "RadarRingsNumberVisible" ), &g_iNavAidRadarRingsNumberVisible)
  CHECK_INT(_T ( "RadarRingsStep" ), &g_fNavAidRadarRingsStep)

  CHECK_INT(_T ( "RadarRingsStepUnits" ), &g_pNavAidRadarRingsStepUnits);

  //    wxString l_wxsOwnshipRangeRingsColour;
  //    CHECK_STR( _T ( "RadarRingsColour" ), &l_wxsOwnshipRangeRingsColour );
  //    if(l_wxsOwnshipRangeRingsColour.Length())
  //    g_colourOwnshipRangeRingsColour.Set( l_wxsOwnshipRangeRingsColour );

  // Waypoint Radar rings
  CHECK_INT(_T ( "WaypointRangeRingsNumber" ), &g_iWaypointRangeRingsNumber)

  CHECK_FLT(_T ( "WaypointRangeRingsStep" ), &g_fWaypointRangeRingsStep, .1)

  CHECK_INT(_T ( "WaypointRangeRingsStepUnits" ),
            &g_iWaypointRangeRingsStepUnits);

  //    wxString l_wxsWaypointRangeRingsColour;
  //    CHECK_STR( _T( "WaypointRangeRingsColour" ),
  //    &l_wxsWaypointRangeRingsColour ); g_colourWaypointRangeRingsColour.Set(
  //    l_wxsWaypointRangeRingsColour );

  CHECK_INT(_T ( "ConfirmObjectDeletion" ), &g_bConfirmObjectDelete);

  // Waypoint dragging with mouse
  CHECK_INT(_T ( "WaypointPreventDragging" ), &g_bWayPointPreventDragging);

  CHECK_INT(_T ( "EnableZoomToCursor" ), &g_bEnableZoomToCursor);

  CHECK_FLT(_T ( "TrackIntervalSeconds" ), &g_TrackIntervalSeconds, 1)

  CHECK_FLT(_T ( "TrackDeltaDistance" ), &g_TrackDeltaDistance, .1)

  CHECK_INT(_T ( "TrackPrecision" ), &g_nTrackPrecision);

  // CHECK_STR( _T ( "NavObjectFileName" ), m_sNavObjSetFile );

  CHECK_INT(_T ( "RouteLineWidth" ), &g_route_line_width);
  CHECK_INT(_T ( "TrackLineWidth" ), &g_track_line_width);

  //     wxString l_wxsTrackLineColour;
  //     CHECK_STR( _T( "TrackLineColour" ), l_wxsTrackLineColour )
  //         g_colourTrackLineColour.Set( l_wxsTrackLineColour );

  CHECK_STR(_T ( "DefaultWPIcon" ), g_default_wp_icon)

  // S57 template items

#define CHECK_BFN(s, t)     \
  conf->Read(s, &read_int); \
  bval = t;                 \
  bval0 = read_int != 0;    \
  if (bval != bval0) return false;

#define CHECK_IFN(s, t)     \
  conf->Read(s, &read_int); \
  if (read_int != t) return false;

#define CHECK_FFN(s, t) \
  conf->Read(s, &dval); \
  if (fabs(dval - t) > 0.1) return false;

  if (ps52plib) {
    int read_int;
    double dval;
    bool bval, bval0;

    conf->SetPath(_T ( "/Settings/GlobalState" ));

    CHECK_BFN(_T ( "bShowS57Text" ), ps52plib->GetShowS57Text());

    CHECK_BFN(_T ( "bShowS57ImportantTextOnly" ),
              ps52plib->GetShowS57ImportantTextOnly());
    CHECK_BFN(_T ( "bShowLightDescription" ), ps52plib->m_bShowLdisText);
    CHECK_BFN(_T ( "bExtendLightSectors" ), ps52plib->m_bExtendLightSectors);
    CHECK_BFN(_T ( "bShowSoundg" ), ps52plib->m_bShowSoundg);
    CHECK_BFN(_T ( "bShowMeta" ), ps52plib->m_bShowMeta);
    CHECK_BFN(_T ( "bUseSCAMIN" ), ps52plib->m_bUseSCAMIN);
    CHECK_BFN(_T ( "bUseSUPERSCAMIN" ), ps52plib->m_bUseSUPER_SCAMIN);
    CHECK_BFN(_T ( "bShowAtonText" ), ps52plib->m_bShowAtonText);
    CHECK_BFN(_T ( "bDeClutterText" ), ps52plib->m_bDeClutterText);
    CHECK_BFN(_T ( "bShowNationalText" ), ps52plib->m_bShowNationalTexts);
    CHECK_IFN(_T ( "nDisplayCategory" ), ps52plib->GetDisplayCategory());
    CHECK_IFN(_T ( "nSymbolStyle" ), ps52plib->m_nSymbolStyle);
    CHECK_IFN(_T ( "nBoundaryStyle" ), ps52plib->m_nBoundaryStyle);
    CHECK_FFN(_T ( "S52_MAR_SAFETY_CONTOUR" ),
              S52_getMarinerParam(S52_MAR_SAFETY_CONTOUR));
    CHECK_FFN(_T ( "S52_MAR_SHALLOW_CONTOUR" ),
              S52_getMarinerParam(S52_MAR_SHALLOW_CONTOUR));
    CHECK_FFN(_T ( "S52_MAR_DEEP_CONTOUR" ),
              S52_getMarinerParam(S52_MAR_DEEP_CONTOUR));
    CHECK_FFN(_T ( "S52_MAR_TWO_SHADES" ),
              S52_getMarinerParam(S52_MAR_TWO_SHADES));
    CHECK_INT(_T ( "S52_DEPTH_UNIT_SHOW" ), &g_nDepthUnitDisplay);

    //    S57 Object Class Visibility

    OBJLElement *pOLE;

    conf->SetPath(_T ( "/Settings/ObjectFilter" ));

    unsigned int iOBJMax = conf->GetNumberOfEntries();

    if (iOBJMax != ps52plib->pOBJLArray->GetCount()) return false;

    if (iOBJMax) {
      wxString str, sObj;
      long val;
      long dummy;

      bool bCont = conf->GetFirstEntry(str, dummy);
      while (bCont) {
        conf->Read(str, &val);  // Get an Object Viz

        // scan for the same key in the global list
        bool bfound = false;
        if (str.StartsWith(_T ( "viz" ), &sObj)) {
          for (unsigned int iPtr = 0; iPtr < ps52plib->pOBJLArray->GetCount();
               iPtr++) {
            pOLE = (OBJLElement *)(ps52plib->pOBJLArray->Item(iPtr));
            if (!strncmp(pOLE->OBJLName, sObj.mb_str(), 6)) {
              bfound = true;
              if (pOLE->nViz != val) {
                return false;
              }
            }
          }

          if (!bfound) return false;
        }
        bCont = conf->GetNextEntry(str, dummy);
      }
    }
  }

  conf->SetPath(_T ( "/MmsiProperties" ));
  int iPMax = conf->GetNumberOfEntries();
  if (iPMax) {
    wxString str, val;
    long dummy;

    bool bCont = conf->GetFirstEntry(str, dummy);
    while (bCont) {
      conf->Read(str, &val);  // Get an entry

      bool bfound = false;
      for (unsigned int j = 0; j < g_MMSI_Props_Array.GetCount(); j++) {
        MmsiProperties *pProps = g_MMSI_Props_Array.Item(j);
        if (pProps->Serialize().IsSameAs(val)) {
          bfound = true;
          break;
        }
      }
      if (!bfound) return false;

      bCont = conf->GetNextEntry(str, dummy);
    }
  }

  return rv;
}
