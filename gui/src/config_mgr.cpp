/**************************************************************************
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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

/**
 * \file
 *
 * Implement config_mgr.h -- config file user configuration interface
 */

#include "gl_headers.h"  // Must be included before anything using GL stuff

#ifdef __MINGW32__
#undef IPV6STRICT  // mingw FTBS fix:  missing struct ip_mreq
#include <windows.h>
#endif

#include <algorithm>
#include <list>
#include <locale>
#include <stdlib.h>

#include <time.h>

#include <wx/colour.h>
#include <wx/fileconf.h>
#include <wx/filename.h>
#include <wx/log.h>
#include <wx/statline.h>
#include <wx/string.h>

#include "model/ais_decoder.h"
#include "model/ais_state_vars.h"
#include "model/config_vars.h"
#include "model/gui_vars.h"
#include "model/georef.h"
#include "model/multiplexer.h"
#include "model/route_point.h"

#include "ais.h"
#include "chartdb.h"
#include "chcanv.h"
#include "config_mgr.h"
#include "font_mgr.h"
#include "layer.h"
#include "navutil.h"
#include "ocpn_frame.h"
#include "ocpn_gl_options.h"
#include "ocpn_platform.h"
#include "ocpn_plugin.h"
#include "route_prop_dlg_impl.h"
#include "s52plib.h"
#include "s52utils.h"

extern s52plib *ps52plib;  // In a library...

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

//--------------------------------------------------------------------
//   Private ( XML encoded ) catalog of available configurations
//--------------------------------------------------------------------

class OCPNConfigCatalog : public pugi::xml_document {
public:
  OCPNConfigCatalog();
  ~OCPNConfigCatalog();

  bool AddConfig(OCPNConfigObject *config, unsigned int flags);
  bool RemoveConfig(wxString GUID);

  void SetRootConfigNode();
  bool IsOpenCPN();
  bool SaveFile(const wxString filename);
  bool LoadFile(const wxString filename);

  pugi::xml_node m_config_root;
};

OCPNConfigCatalog::OCPNConfigCatalog() : pugi::xml_document() {}

OCPNConfigCatalog::~OCPNConfigCatalog() {}

void OCPNConfigCatalog::SetRootConfigNode() {
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
    node.append_attribute("title") = "Substitute Title";

  abuf = config->m_description.ToUTF8();
  if (abuf.data())
    node.append_attribute("description") = abuf.data();
  else
    node.append_attribute("description") = "Substitute Description";

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

  SetBackgroundColour(
      wxSystemSettings::GetColour(wxSystemColour::wxSYS_COLOUR_WINDOW));
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

ConfigMgr::~ConfigMgr() {
  configList->clear();
  delete configList;
}

void ConfigMgr::Init() {
  m_configDir = g_Platform->GetPrivateDataDir();
  appendOSDirSlash(&m_configDir);
  m_configDir.append("Configs");
  appendOSDirSlash(&m_configDir);
  if (!wxFileName::DirExists(m_configDir)) {
    wxFileName::Mkdir(m_configDir);
  }

  m_configCatalogName = g_Platform->GetPrivateDataDir();
  appendOSDirSlash(&m_configCatalogName);
  m_configCatalogName.append("Configs");
  appendOSDirSlash(&m_configCatalogName);
  m_configCatalogName.append("configs.xml");

  // Create the catalog, if necessary
  if (!wxFileExists(m_configCatalogName)) {
    wxLogMessage("Creating new Configs catalog: " + m_configCatalogName);

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
  CreateNamedConfig(t_title, t_desc, "11111111-1111-1111-1111-111111111111");
}

bool ConfigMgr::LoadCatalog() {
  wxLogMessage("Loading Configs catalog: " + m_configCatalogName);
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
      for (auto it = configList->begin(); it != configList->end(); ++it) {
        OCPNConfigObject *look = *it;
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
        configList->push_back(newConfig);
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
    pConfig->templateFileName = "OCPNTemplate-" + GUID + ".conf";

    //  Save the template contents
    wxString templateFullFileName = GetConfigDir() + pConfig->templateFileName;
    if (!SaveTemplate(templateFullFileName)) {
      wxLogMessage("Unable to save template titled: " + title +
                   " as file: " + templateFullFileName);
      delete pConfig;
      return "";
    }
  }

  // Add this config to the catalog
  if (!m_configCatalog->AddConfig(pConfig, 0)) {
    wxLogMessage("Unable to add config to catalog...Title: " + title);
    delete pConfig;
    return "";
  }

  // Add to the class list of configs
  configList->push_back(pConfig);

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

  //  Remove the config from the member list without deleting it
  auto found = std::find(configList->begin(), configList->end(), cfg);
  if (found != configList->end()) configList->erase(found);

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
  for (auto it = configList->begin(); it != configList->end(); ++it) {
    OCPNConfigObject *look = *it;
    if (look->m_GUID == GUID) {
      return look;
      break;
    }
  }
  return NULL;
}

wxString ConfigMgr::GetTemplateTitle(wxString GUID) {
  for (auto it = configList->begin(); it != configList->end(); ++it) {
    OCPNConfigObject *look = *it;
    if (look->m_GUID == GUID) {
      return look->m_title;
      break;
    }
  }
  return wxEmptyString;
}

wxArrayString ConfigMgr::GetConfigGUIDArray() {
  wxArrayString ret_val;

  for (auto it = configList->begin(); it != configList->end(); ++it) {
    OCPNConfigObject *look = *it;
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
    if (GUID.StartsWith("11111111")) {
      thisConfig =
          *GetpSharedDataLocation() + "configs/OCPNTemplate-Recovery.conf";
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
wxString ConfigMgr::GetUUID() {
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

  str.Printf("%08x-%04x-%04x-%02x%02x-%04x%08x", uuid.time_low, uuid.time_mid,
             uuid.time_hi_and_version, uuid.clock_seq_hi_and_rsv,
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
#ifdef __ANDROID__
  wxLogNull logNo;
#endif

  //    Global options and settings
  conf->SetPath("/Settings");

  conf->Write("InlandEcdis", g_bInlandEcdis);
  conf->Write("UIexpert", g_bUIexpert);
  conf->Write("SpaceDropMark", g_bSpaceDropMark);

  conf->Write("ShowStatusBar", g_bShowStatusBar);
#ifndef __WXOSX__
  conf->Write("ShowMenuBar", g_bShowMenuBar);
#endif
  conf->Write("DefaultFontSize", g_default_font_size);

  conf->Write("Fullscreen", g_bFullscreen);
  conf->Write("ShowCompassWindow", g_bShowCompassWin);
  conf->Write("SetSystemTime", s_bSetSystemTime);
  conf->Write("ShowGrid", g_bDisplayGrid);
  conf->Write("PlayShipsBells", g_bPlayShipsBells);
  conf->Write("SoundDeviceIndex", g_iSoundDeviceIndex);
  conf->Write("FullscreenToolbar", g_bFullscreenToolbar);
  // conf->Write( "TransparentToolbar", g_bTransparentToolbar );
  conf->Write("PermanentMOBIcon", g_bPermanentMOBIcon);
  conf->Write("ShowLayers", g_bShowLayers);
  conf->Write("AutoAnchorDrop", g_bAutoAnchorMark);
  conf->Write("ShowChartOutlines", g_bShowOutlines);
  conf->Write("ShowActiveRouteTotal", g_bShowRouteTotal);
  conf->Write("ShowActiveRouteHighway", g_bShowActiveRouteHighway);
  conf->Write("SDMMFormat", g_iSDMMFormat);
  conf->Write("ShowChartBar", g_bShowChartBar);

  conf->Write("GUIScaleFactor", g_GUIScaleFactor);
  conf->Write("ChartObjectScaleFactor", g_ChartScaleFactor);
  conf->Write("ShipScaleFactor", g_ShipScaleFactor);

  conf->Write("ShowTrue", g_bShowTrue);
  conf->Write("ShowMag", g_bShowMag);
  conf->Write("UserMagVariation", wxString::Format("%.2f", g_UserVar));

  conf->Write("CM93DetailFactor", g_cm93_zoom_factor);
  conf->Write("CM93DetailZoomPosX", g_detailslider_dialog_x);
  conf->Write("CM93DetailZoomPosY", g_detailslider_dialog_y);
  conf->Write("ShowCM93DetailSlider", g_bShowDetailSlider);

  conf->Write("SkewToNorthUp", g_bskew_comp);

  conf->Write("ZoomDetailFactor", g_chart_zoom_modifier_raster);
  conf->Write("ZoomDetailFactorVector", g_chart_zoom_modifier_vector);

  conf->Write("SmoothPanZoom", g_bsmoothpanzoom);

  conf->Write("CourseUpMode", g_bCourseUp);
  if (!g_bInlandEcdis) conf->Write("LookAheadMode", g_bLookAhead);
  conf->Write("TenHzUpdate", g_btenhertz);

  conf->Write("COGUPAvgSeconds", g_COGAvgSec);
  conf->Write("UseMagAPB", g_bMagneticAPB);

  conf->Write("OwnshipCOGPredictorMinutes", g_ownship_predictor_minutes);
  conf->Write("OwnshipCOGPredictorWidth", g_cog_predictor_width);
  conf->Write("OwnshipHDTPredictorMiles", g_ownship_HDTpredictor_miles);
  conf->Write("OwnShipIconType", g_OwnShipIconType);
  conf->Write("OwnShipLength", g_n_ownship_length_meters);
  conf->Write("OwnShipWidth", g_n_ownship_beam_meters);
  conf->Write("OwnShipGPSOffsetX", g_n_gps_antenna_offset_x);
  conf->Write("OwnShipGPSOffsetY", g_n_gps_antenna_offset_y);
  conf->Write("OwnShipMinSize", g_n_ownship_min_mm);
  conf->Write("OwnShipSogCogCalc", g_own_ship_sog_cog_calc);
  conf->Write("OwnShipSogCogCalcDampSec", g_own_ship_sog_cog_calc_damp_sec);

  conf->Write("RouteArrivalCircleRadius",
              wxString::Format("%.3f", g_n_arrival_circle_radius));
  conf->Write("ChartQuilting", g_bQuiltEnable);

  conf->Write("StartWithTrackActive", g_bTrackCarryOver);
  conf->Write("AutomaticDailyTracks", g_bTrackDaily);
  conf->Write("TrackRotateAt", g_track_rotate_time);
  conf->Write("TrackRotateTimeType", g_track_rotate_time_type);
  conf->Write("HighlightTracks", g_bHighliteTracks);

  conf->Write("DateTimeFormat", g_datetime_format);

  conf->Write("InitialStackIndex", g_restore_stackindex);
  conf->Write("InitialdBIndex", g_restore_dbindex);

  conf->Write("AnchorWatch1GUID", g_AW1GUID);
  conf->Write("AnchorWatch2GUID", g_AW2GUID);

  conf->Write("ToolbarX", g_maintoolbar_x);
  conf->Write("ToolbarY", g_maintoolbar_y);
  conf->Write("ToolbarOrient", g_maintoolbar_orient);

  conf->Write("iENCToolbarX", g_iENCToolbarPosX);
  conf->Write("iENCToolbarY", g_iENCToolbarPosY);

  if (!g_bInlandEcdis) {
    conf->Write("GlobalToolbarConfig", g_toolbarConfig);
    conf->Write("DistanceFormat", g_iDistanceFormat);
    conf->Write("SpeedFormat", g_iSpeedFormat);
    conf->Write("WindSpeedFormat", g_iWindSpeedFormat);
    conf->Write("ShowDepthUnits", g_bShowDepthUnits);
  }

  conf->Write("MobileTouch", g_btouch);
  conf->Write("ResponsiveGraphics", g_bresponsive);

  conf->Write("AutoHideToolbar", g_bAutoHideToolbar);
  conf->Write("AutoHideToolbarSecs", g_nAutoHideToolbar);

  wxString st0;
  for (const auto &mm : g_config_display_size_mm) {
    st0.Append(wxString::Format("%zu,", mm));
  }
  st0.RemoveLast();  // Strip last comma
  conf->Write("DisplaySizeMM", st0);
  conf->Write("DisplaySizeManual", g_config_display_size_manual);

  conf->Write("PlanSpeed", wxString::Format("%.2f", g_PlanSpeed));

#if 0
    wxString vis, invis;
    LayerList::iterator it;
    int index = 0;
    for( it = ( *pLayerList ).begin(); it != ( *pLayerList ).end(); ++it, ++index ) {
        Layer *lay = (Layer *) ( *it );
        if( lay->IsVisibleOnChart() ) vis += ( lay->m_LayerName ) + ";";
        else
            invis += ( lay->m_LayerName ) + ";";
    }
    conf->Write( "VisibleLayers", vis );
    conf->Write( "InvisibleLayers", invis );
#endif

  conf->Write("Locale", g_locale);
  conf->Write("LocaleOverride", g_localeOverride);

  // LIVE ETA OPTION
  conf->Write("LiveETA", g_bShowLiveETA);
  conf->Write("DefaultBoatSpeed", g_defaultBoatSpeed);

  //    S57 Object Filter Settings
  conf->SetPath("/Settings/ObjectFilter");

  if (ps52plib) {
    for (unsigned int iPtr = 0; iPtr < ps52plib->pOBJLArray->GetCount();
         iPtr++) {
      OBJLElement *pOLE = (OBJLElement *)(ps52plib->pOBJLArray->Item(iPtr));

      wxString st1("viz");
      char name[7];
      strncpy(name, pOLE->OBJLName, 6);
      name[6] = 0;
      st1.Append(wxString(name, wxConvUTF8));
      conf->Write(st1, pOLE->nViz);
    }
  }

  //    Global State

  conf->SetPath("/Settings/GlobalState");

  //    Various Options
  if (!g_bInlandEcdis)
    conf->Write("nColorScheme", (int)gFrame->GetColorScheme());

  //    AIS
  conf->SetPath("/Settings/AIS");

  conf->Write("bNoCPAMax", g_bCPAMax);
  conf->Write("NoCPAMaxNMi", g_CPAMax_NM);
  conf->Write("bCPAWarn", g_bCPAWarn);
  conf->Write("CPAWarnNMi", g_CPAWarn_NM);
  conf->Write("bTCPAMax", g_bTCPA_Max);
  conf->Write("TCPAMaxMinutes", g_TCPA_Max);
  conf->Write("bMarkLostTargets", g_bMarkLost);
  conf->Write("MarkLost_Minutes", g_MarkLost_Mins);
  conf->Write("bRemoveLostTargets", g_bRemoveLost);
  conf->Write("RemoveLost_Minutes", g_RemoveLost_Mins);
  conf->Write("bShowCOGArrows", g_bShowCOG);
  conf->Write("CogArrowMinutes", g_ShowCOG_Mins);
  conf->Write("bShowTargetTracks", g_bAISShowTracks);
  conf->Write("TargetTracksMinutes", g_AISShowTracks_Mins);

  conf->Write("bHideMooredTargets", g_bHideMoored);
  conf->Write("MooredTargetMaxSpeedKnots", g_ShowMoored_Kts);

  conf->Write("bAISAlertDialog", g_bAIS_CPA_Alert);
  conf->Write("bAISAlertAudio", g_bAIS_CPA_Alert_Audio);
  conf->Write("AISAlertAudioFile", g_sAIS_Alert_Sound_File);
  conf->Write("bAISAlertSuppressMoored", g_bAIS_CPA_Alert_Suppress_Moored);
  conf->Write("bShowAreaNotices", g_bShowAreaNotices);
  conf->Write("bDrawAISSize", g_bDrawAISSize);
  conf->Write("bDrawAISRealtime", g_bDrawAISRealtime);
  conf->Write("AISRealtimeMinSpeedKnots", g_AIS_RealtPred_Kts);
  conf->Write("bShowAISName", g_bShowAISName);
  conf->Write("ShowAISTargetNameScale", g_Show_Target_Name_Scale);
  conf->Write("bWplIsAprsPositionReport", g_bWplUsePosition);
  conf->Write("WplSelAction", g_WplAction);
  conf->Write("AISCOGPredictorWidth", g_ais_cog_predictor_width);
  conf->Write("bShowScaledTargets", g_bAllowShowScaled);
  conf->Write("AISScaledNumber", g_ShowScaled_Num);
  conf->Write("AISScaledNumberWeightSOG", g_ScaledNumWeightSOG);
  conf->Write("AISScaledNumberWeightCPA", g_ScaledNumWeightCPA);
  conf->Write("AISScaledNumberWeightTCPA", g_ScaledNumWeightTCPA);
  conf->Write("AISScaledNumberWeightRange", g_ScaledNumWeightRange);
  conf->Write("AISScaledNumberWeightSizeOfTarget", g_ScaledNumWeightSizeOfT);
  conf->Write("AISScaledSizeMinimal", g_ScaledSizeMinimal);
  conf->Write("AISShowScaled", g_bShowScaled);

  conf->Write("AlertDialogSizeX", g_ais_alert_dialog_sx);
  conf->Write("AlertDialogSizeY", g_ais_alert_dialog_sy);
  conf->Write("AlertDialogPosX", g_ais_alert_dialog_x);
  conf->Write("AlertDialogPosY", g_ais_alert_dialog_y);
  conf->Write("QueryDialogPosX", g_ais_query_dialog_x);
  conf->Write("QueryDialogPosY", g_ais_query_dialog_y);
  conf->Write("AISTargetListPerspective", g_AisTargetList_perspective);
  conf->Write("AISTargetListRange", g_AisTargetList_range);
  conf->Write("AISTargetListSortColumn", g_AisTargetList_sortColumn);
  conf->Write("bAISTargetListSortReverse", g_bAisTargetList_sortReverse);
  conf->Write("AISTargetListColumnSpec", g_AisTargetList_column_spec);
  conf->Write("AISTargetListColumnOrder", g_AisTargetList_column_order);
  conf->Write("S57QueryDialogSizeX", g_S57_dialog_sx);
  conf->Write("S57QueryDialogSizeY", g_S57_dialog_sy);
  conf->Write("bAISRolloverShowClass", g_bAISRolloverShowClass);
  conf->Write("bAISRolloverShowCOG", g_bAISRolloverShowCOG);
  conf->Write("bAISRolloverShowCPA", g_bAISRolloverShowCPA);
  conf->Write("bAISAlertAckTimeout", g_bAIS_ACK_Timeout);
  conf->Write("AlertAckTimeoutMinutes", g_AckTimeout_Mins);

  conf->SetPath("/Settings/GlobalState");
  if (ps52plib) {
    conf->Write("bShowS57Text", ps52plib->GetShowS57Text());
    conf->Write("bShowS57ImportantTextOnly",
                ps52plib->GetShowS57ImportantTextOnly());
    if (!g_bInlandEcdis)
      conf->Write("nDisplayCategory", (long)ps52plib->GetDisplayCategory());
    conf->Write("nSymbolStyle", (int)ps52plib->m_nSymbolStyle);
    conf->Write("nBoundaryStyle", (int)ps52plib->m_nBoundaryStyle);

    conf->Write("bShowSoundg", ps52plib->m_bShowSoundg);
    conf->Write("bShowMeta", ps52plib->m_bShowMeta);
    conf->Write("bUseSCAMIN", ps52plib->m_bUseSCAMIN);
    conf->Write("bUseSUPER_SCAMIN", ps52plib->m_bUseSUPER_SCAMIN);
    conf->Write("bShowAtonText", ps52plib->m_bShowAtonText);
    conf->Write("bShowLightDescription", ps52plib->m_bShowLdisText);
    conf->Write("bExtendLightSectors", ps52plib->m_bExtendLightSectors);
    conf->Write("bDeClutterText", ps52plib->m_bDeClutterText);
    conf->Write("bShowNationalText", ps52plib->m_bShowNationalTexts);

    conf->Write("S52_MAR_SAFETY_CONTOUR",
                S52_getMarinerParam(S52_MAR_SAFETY_CONTOUR));
    conf->Write("S52_MAR_SHALLOW_CONTOUR",
                S52_getMarinerParam(S52_MAR_SHALLOW_CONTOUR));
    conf->Write("S52_MAR_DEEP_CONTOUR",
                S52_getMarinerParam(S52_MAR_DEEP_CONTOUR));
    conf->Write("S52_MAR_TWO_SHADES", S52_getMarinerParam(S52_MAR_TWO_SHADES));
    conf->Write("S52_DEPTH_UNIT_SHOW", ps52plib->m_nDepthUnitDisplay);
  }

  conf->SetPath("/Settings/Others");

  // Radar rings
  conf->Write("ShowRadarRings", (bool)(g_iNavAidRadarRingsNumberVisible >
                                       0));  // 3.0.0 config support
  conf->Write("RadarRingsNumberVisible", g_iNavAidRadarRingsNumberVisible);
  g_bNavAidRadarRingsShown = g_iNavAidRadarRingsNumberVisible > 0;
  conf->Write("RadarRingsStep", g_fNavAidRadarRingsStep);
  conf->Write("RadarRingsStepUnits", g_pNavAidRadarRingsStepUnits);
  conf->Write("RadarRingsColour",
              g_colourOwnshipRangeRingsColour.GetAsString(wxC2S_HTML_SYNTAX));

  // Waypoint Radar rings
  conf->Write("WaypointRangeRingsNumber", g_iWaypointRangeRingsNumber);
  conf->Write("WaypointRangeRingsStep", g_fWaypointRangeRingsStep);
  conf->Write("WaypointRangeRingsStepUnits", g_iWaypointRangeRingsStepUnits);
  conf->Write("WaypointRangeRingsColour",
              g_colourWaypointRangeRingsColour.GetAsString(wxC2S_HTML_SYNTAX));

  conf->Write("ConfirmObjectDeletion", g_bConfirmObjectDelete);

  // Waypoint dragging with mouse
  conf->Write("WaypointPreventDragging", g_bWayPointPreventDragging);

  conf->Write("EnableZoomToCursor", g_bEnableZoomToCursor);

  conf->Write("TrackIntervalSeconds", g_TrackIntervalSeconds);
  conf->Write("TrackDeltaDistance", g_TrackDeltaDistance);
  conf->Write("TrackPrecision", g_nTrackPrecision);

  conf->Write("RouteLineWidth", g_route_line_width);
  conf->Write("TrackLineWidth", g_track_line_width);
  conf->Write("TrackLineColour",
              g_colourTrackLineColour.GetAsString(wxC2S_HTML_SYNTAX));
  conf->Write("DefaultWPIcon", g_default_wp_icon);

  //    Fonts

  //  Store the persistent Auxiliary Font descriptor Keys
  conf->SetPath("/Settings/AuxFontKeys");

  wxArrayString keyArray = FontMgr::Get().GetAuxKeyArray();
  for (unsigned int i = 0; i < keyArray.GetCount(); i++) {
    wxString key;
    key.Printf("Key%i", i);
    wxString keyval = keyArray[i];
    conf->Write(key, keyval);
  }

  wxString font_path;
#ifdef __WXX11__
  font_path = ("/Settings/X11Fonts");
#endif

#ifdef __WXGTK__
  font_path = ("/Settings/GTKFonts");
#endif

#ifdef __WXMSW__
  font_path = ("/Settings/MSWFonts");
#endif

#ifdef __WXMAC__
  font_path = ("/Settings/MacFonts");
#endif

#ifdef __WXQT__
  font_path = ("/Settings/QTFonts");
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

#define CHECK_INT(s, t)                          \
  read_int = *t;                                 \
  if (!conf.Read(s, &read_int)) wxLogMessage(s); \
  if ((int)*t != read_int) return false;

#define CHECK_STR(s, t) \
  val = t;              \
  conf.Read(s, &val);   \
  if (!t.IsSameAs(val)) return false;

#define CHECK_STRP(s, t) \
  conf.Read(s, &val);    \
  if (!t->IsSameAs(val)) return false;

#define CHECK_FLT(s, t, eps) \
  conf.Read(s, &val);        \
  val.ToDouble(&dval);       \
  if (fabs(dval - *t) > eps) return false;

bool ConfigMgr::CheckTemplate(wxString fileName) {
  bool rv = true;

  int read_int;
  wxString val;
  double dval;

  MyConfig conf(fileName);

  //    Global options and settings
  conf.SetPath("/Settings");

  CHECK_INT("UIexpert", &g_bUIexpert);

  /// CHECK_STR( "UIStyle", g_uiStyle  );

  CHECK_INT("InlandEcdis", &g_bInlandEcdis);

  CHECK_INT("SpaceDropMark", &g_bSpaceDropMark);

  /// CHECK_INT( "UseModernUI5", &g_useMUI );

#if 0
    CHECK_INT( "DebugGDAL", &g_bGDAL_Debug );
    CHECK_INT( "DebugNMEA", &g_nNMEADebug );
    CHECK_INT( "DebugOpenGL", &g_bDebugOGL );
    ///CHECK_INT( "AnchorWatchDefault", &g_nAWDefault );
    ///CHECK_INT( "AnchorWatchMax", &g_nAWMax );
    ///CHECK_INT( "GPSDogTimeout", &gps_watchdog_timeout_ticks );
    CHECK_INT( "DebugCM93", &g_bDebugCM93 );
    CHECK_INT( "DebugS57", &g_bDebugS57 );         // Show LUP and Feature info in object query
    CHECK_INT( "DebugBSBImg", &g_BSBImgDebug );
    CHECK_INT( "DebugGPSD", &g_bDebugGPSD );
#endif

  CHECK_INT("DefaultFontSize", &g_default_font_size);

  //    Read( "GPSIdent", &g_GPS_Ident );
  CHECK_INT("UseGarminHostUpload", &g_bGarminHostUpload);

  CHECK_INT("UseNMEA_GLL", &g_bUseGLL);

  CHECK_INT("AutoHideToolbar", &g_bAutoHideToolbar);
  CHECK_INT("AutoHideToolbarSecs", &g_nAutoHideToolbar);

  CHECK_INT("UseSimplifiedScalebar", &g_bsimplifiedScalebar);

  CHECK_INT("DisplaySizeMM", &g_display_size_mm);
  CHECK_INT("DisplaySizeManual", &g_config_display_size_manual);

  CHECK_INT("GUIScaleFactor", &g_GUIScaleFactor);

  CHECK_INT("ChartObjectScaleFactor", &g_ChartScaleFactor);
  CHECK_INT("ShipScaleFactor", &g_ShipScaleFactor);

  CHECK_INT("FilterNMEA_Avg", &g_bfilter_cogsog);
  CHECK_INT("FilterNMEA_Sec", &g_COGFilterSec);

  CHECK_INT("ShowTrue", &g_bShowTrue);
  CHECK_INT("ShowMag", &g_bShowMag);

  CHECK_FLT("UserMagVariation", &g_UserVar, 0.1)

  CHECK_INT("UseMagAPB", &g_bMagneticAPB);

  CHECK_INT("ScreenBrightness", &g_nbrightness);

  CHECK_INT("MemFootprintTargetMB", &g_MemFootMB);

  CHECK_INT("WindowsComPortMax", &g_nCOMPortCheck);

  CHECK_INT("ChartQuilting", &g_bQuiltEnable);
  CHECK_INT("ChartQuiltingInitial", &g_bQuiltStart);

  CHECK_INT("CourseUpMode", &g_bCourseUp);
  CHECK_INT("COGUPAvgSeconds", &g_COGAvgSec);
  // CHECK_INT( "LookAheadMode", &g_bLookAhead );
  // CHECK_INT( "SkewToNorthUp", &g_bskew_comp );
  CHECK_INT("OpenGL", &g_bopengl);
  CHECK_INT("SoftwareGL", &g_bSoftwareGL);

  CHECK_INT("NMEAAPBPrecision", &g_NMEAAPBPrecision);

  CHECK_STR("TalkerIdText", g_TalkerIdText);
  CHECK_INT("MaxWaypointNameLength", &g_maxWPNameLength);
  CHECK_INT("MbtilesMaxLayers", &g_mbtilesMaxLayers);

  /* opengl options */
#ifdef ocpnUSE_GL
  CHECK_INT("OpenGLExpert", &g_bGLexpert);
  CHECK_INT("UseAcceleratedPanning", &g_GLOptions.m_bUseAcceleratedPanning);
  CHECK_INT("GPUTextureCompression", &g_GLOptions.m_bTextureCompression);
  CHECK_INT("GPUTextureCompressionCaching",
            &g_GLOptions.m_bTextureCompressionCaching);
  CHECK_INT("PolygonSmoothing", &g_GLOptions.m_GLPolygonSmoothing);
  CHECK_INT("LineSmoothing", &g_GLOptions.m_GLLineSmoothing);
  CHECK_INT("GPUTextureDimension", &g_GLOptions.m_iTextureDimension);
  CHECK_INT("GPUTextureMemSize", &g_GLOptions.m_iTextureMemorySize);

#endif
  CHECK_INT("SmoothPanZoom", &g_bsmoothpanzoom);

  CHECK_INT("ToolbarX", &g_maintoolbar_x);
  CHECK_INT("ToolbarY", &g_maintoolbar_y);
  CHECK_INT("ToolbarOrient", &g_maintoolbar_orient);
  CHECK_STR("GlobalToolbarConfig", g_toolbarConfig);

  CHECK_INT("iENCToolbarX", &g_iENCToolbarPosX);
  CHECK_INT("iENCToolbarY", &g_iENCToolbarPosY);

  CHECK_STR("AnchorWatch1GUID", g_AW1GUID);
  CHECK_STR("AnchorWatch2GUID", g_AW2GUID);

  CHECK_INT("MobileTouch", &g_btouch);
  CHECK_INT("ResponsiveGraphics", &g_bresponsive);

  CHECK_INT("ZoomDetailFactor", &g_chart_zoom_modifier_raster);
  CHECK_INT("ZoomDetailFactorVector", &g_chart_zoom_modifier_vector);

  CHECK_INT("CM93DetailFactor", &g_cm93_zoom_factor);
  CHECK_INT("CM93DetailZoomPosX", &g_detailslider_dialog_x);
  CHECK_INT("CM93DetailZoomPosY", &g_detailslider_dialog_y);
  CHECK_INT("ShowCM93DetailSlider", &g_bShowDetailSlider);

  CHECK_INT("SENC_LOD_Pixels", &g_SENC_LOD_pixels);

  CHECK_INT("SkewCompUpdatePeriod", &g_SkewCompUpdatePeriod);

  CHECK_INT("ShowStatusBar", &g_bShowStatusBar);
#ifndef __WXOSX__
  CHECK_INT("ShowMenuBar", &g_bShowMenuBar);
#endif
  CHECK_INT("Fullscreen", &g_bFullscreen);
  CHECK_INT("ShowCompassWindow", &g_bShowCompassWin);
  CHECK_INT("PlayShipsBells", &g_bPlayShipsBells);
  CHECK_INT("SoundDeviceIndex", &g_iSoundDeviceIndex);
  CHECK_INT("FullscreenToolbar", &g_bFullscreenToolbar);
  // CHECK_INT( "TransparentToolbar", &g_bTransparentToolbar );
  CHECK_INT("PermanentMOBIcon", &g_bPermanentMOBIcon);
  CHECK_INT("ShowLayers", &g_bShowLayers);
  CHECK_INT("ShowDepthUnits", &g_bShowDepthUnits);
  CHECK_INT("AutoAnchorDrop", &g_bAutoAnchorMark);
  CHECK_INT("ShowActiveRouteHighway", &g_bShowActiveRouteHighway);
  CHECK_INT("ShowActiveRouteTotal", &g_bShowRouteTotal);
  CHECK_STR("MostRecentGPSUploadConnection", g_uploadConnection);
  CHECK_INT("ShowChartBar", &g_bShowChartBar);
  CHECK_INT("SDMMFormat",
            &g_iSDMMFormat);  // 0 = "Degrees, Decimal minutes"), 1 = "Decimal
                              // degrees", 2 = "Degrees,Minutes, Seconds"

  CHECK_INT("DistanceFormat",
            &g_iDistanceFormat);  // 0 = "Nautical miles"), 1 = "Statute miles",
                                  // 2 = "Kilometers", 3 = "Meters"
  CHECK_INT("SpeedFormat",
            &g_iSpeedFormat);  // 0 = "kts"), 1 = "mph", 2 = "km/h", 3 = "m/s"
  CHECK_INT(
      "WindSpeedFormat",
      &g_iWindSpeedFormat);  // 0 = "knots"), 1 = "m/s", 2 = "Mph", 3 = "km/h"

  // LIVE ETA OPTION
  CHECK_INT("LiveETA", &g_bShowLiveETA);
  CHECK_INT("DefaultBoatSpeed", &g_defaultBoatSpeed);

  CHECK_INT("OwnshipCOGPredictorMinutes", &g_ownship_predictor_minutes);
  CHECK_INT("OwnshipCOGPredictorWidth", &g_cog_predictor_width);
  CHECK_INT("OwnshipHDTPredictorMiles", &g_ownship_HDTpredictor_miles);

  CHECK_INT("OwnShipIconType", &g_OwnShipIconType);
  CHECK_FLT("OwnShipLength", &g_n_ownship_length_meters, 0.1);
  CHECK_FLT("OwnShipWidth", &g_n_ownship_beam_meters, 0.1);
  CHECK_FLT("OwnShipGPSOffsetX", &g_n_gps_antenna_offset_x, 0.1);
  CHECK_FLT("OwnShipGPSOffsetY", &g_n_gps_antenna_offset_y, 0.1);
  CHECK_INT("OwnShipMinSize", &g_n_ownship_min_mm);
  CHECK_INT("OwnShipSogCogCalc", &g_own_ship_sog_cog_calc);
  CHECK_INT("OwnShipSogCogCalcDampSec", &g_own_ship_sog_cog_calc_damp_sec);

  CHECK_FLT("RouteArrivalCircleRadius", &g_n_arrival_circle_radius, .01);

  CHECK_INT("FullScreenQuilt", &g_bFullScreenQuilt);

  CHECK_INT("StartWithTrackActive", &g_bTrackCarryOver);
  CHECK_INT("AutomaticDailyTracks", &g_bTrackDaily);
  CHECK_INT("TrackRotateAt", &g_track_rotate_time);
  CHECK_INT("TrackRotateTimeType", &g_track_rotate_time_type);
  CHECK_INT("HighlightTracks", &g_bHighliteTracks);

  CHECK_STR("DateTimeFormat", g_datetime_format);

  CHECK_FLT("PlanSpeed", &g_PlanSpeed, 0.1)

  /// CHECK_STR( "VisibleLayers", g_VisibleLayers );
  /// CHECK_STR( "InvisibleLayers", g_InvisibleLayers );

  CHECK_INT("PreserveScaleOnX", &g_bPreserveScaleOnX);

  CHECK_STR("Locale", g_locale);
  CHECK_STR("LocaleOverride", g_localeOverride);

  // We allow 0-99 backups ov navobj.xml
  CHECK_INT("KeepNavobjBackups", &g_navobjbackups);

  //     NMEALogWindow::Get().SetSize(Read("NMEALogWindowSizeX", 600L),
  //     Read("NMEALogWindowSizeY", 400L));
  //     NMEALogWindow::Get().SetPos(Read("NMEALogWindowPosX", 10L),
  //     Read("NMEALogWindowPosY", 10L));
  //     NMEALogWindow::Get().CheckPos(display_width, display_height);

  // Boolean to cater for legacy Input COM Port filer behaviour, i.e. show msg
  // filtered but put msg on bus.
  CHECK_INT("LegacyInputCOMPortFilterBehaviour",
            &g_b_legacy_input_filter_behaviour);

  CHECK_INT("AdvanceRouteWaypointOnArrivalOnly",
            &g_bAdvanceRouteWaypointOnArrivalOnly);

  CHECK_INT("EnableRotateKeys", &g_benable_rotate);
  CHECK_INT("EmailCrashReport", &g_bEmailCrashReport);

  CHECK_INT("EnableAISNameCache", &g_benableAISNameCache);

  CHECK_INT("EnableUDPNullHeader", &g_benableUDPNullHeader);

  conf.SetPath("/Settings/GlobalState");

  CHECK_INT("FrameWinX", &g_nframewin_x);
  CHECK_INT("FrameWinY", &g_nframewin_y);
  CHECK_INT("FrameWinPosX", &g_nframewin_posx);
  CHECK_INT("FrameWinPosY", &g_nframewin_posy);
  CHECK_INT("FrameMax", &g_bframemax);

  CHECK_INT("ClientPosX", &g_lastClientRectx);
  CHECK_INT("ClientPosY", &g_lastClientRecty);
  CHECK_INT("ClientSzX", &g_lastClientRectw);
  CHECK_INT("ClientSzY", &g_lastClientRecth);

  CHECK_INT("RoutePropSizeX", &g_route_prop_sx);
  CHECK_INT("RoutePropSizeY", &g_route_prop_sy);
  CHECK_INT("RoutePropPosX", &g_route_prop_x);
  CHECK_INT("RoutePropPosY", &g_route_prop_y);

  CHECK_INT("S52_DEPTH_UNIT_SHOW",
            &g_nDepthUnitDisplay);  // default is metres

  //    AIS
  conf.SetPath("/Settings/AIS");
  CHECK_INT("bNoCPAMax", &g_bCPAMax);
  CHECK_FLT("NoCPAMaxNMi", &g_CPAMax_NM, .01)
  CHECK_INT("bCPAWarn", &g_bCPAWarn);
  CHECK_FLT("CPAWarnNMi", &g_CPAWarn_NM, .01)
  CHECK_INT("bTCPAMax", &g_bTCPA_Max);
  CHECK_FLT("TCPAMaxMinutes", &g_TCPA_Max, 1)
  CHECK_INT("bMarkLostTargets", &g_bMarkLost);
  CHECK_FLT("MarkLost_Minutes", &g_MarkLost_Mins, 1)
  CHECK_INT("bRemoveLostTargets", &g_bRemoveLost);
  CHECK_FLT("RemoveLost_Minutes", &g_RemoveLost_Mins, 1)
  CHECK_INT("bShowCOGArrows", &g_bShowCOG);
  CHECK_INT("bSyncCogPredictors", &g_bSyncCogPredictors);
  CHECK_FLT("CogArrowMinutes", &g_ShowCOG_Mins, 1);
  CHECK_INT("bShowTargetTracks", &g_bAISShowTracks);
  CHECK_FLT("TargetTracksMinutes", &g_AISShowTracks_Mins, 1)
  CHECK_FLT("TargetTracksLimit", &g_AISShowTracks_Limit, 300)
  CHECK_INT("bHideMooredTargets", &g_bHideMoored)
  CHECK_FLT("MooredTargetMaxSpeedKnots", &g_ShowMoored_Kts, .1)
  CHECK_INT("bShowScaledTargets", &g_bAllowShowScaled);
  CHECK_INT("AISScaledNumber", &g_ShowScaled_Num);
  CHECK_INT("AISScaledNumberWeightSOG", &g_ScaledNumWeightSOG);
  CHECK_INT("AISScaledNumberWeightCPA", &g_ScaledNumWeightCPA);
  CHECK_INT("AISScaledNumberWeightTCPA", &g_ScaledNumWeightTCPA);
  CHECK_INT("AISScaledNumberWeightRange", &g_ScaledNumWeightRange);
  CHECK_INT("AISScaledNumberWeightSizeOfTarget", &g_ScaledNumWeightSizeOfT);
  CHECK_INT("AISScaledSizeMinimal", &g_ScaledSizeMinimal);
  CHECK_INT("AISShowScaled", &g_bShowScaled);
  CHECK_INT("bShowAreaNotices", &g_bShowAreaNotices);
  CHECK_INT("bDrawAISSize", &g_bDrawAISSize);
  CHECK_INT("bDrawAISRealtime", &g_bDrawAISRealtime);
  CHECK_FLT("AISRealtimeMinSpeedKnots", &g_AIS_RealtPred_Kts, .1);
  CHECK_INT("bShowAISName", &g_bShowAISName);
  CHECK_INT("bAISAlertDialog", &g_bAIS_CPA_Alert);
  CHECK_INT("ShowAISTargetNameScale", &g_Show_Target_Name_Scale);
  CHECK_INT("bWplIsAprsPositionReport", &g_bWplUsePosition);
  CHECK_INT("WplSelAction", &g_WplAction);
  CHECK_INT("AISCOGPredictorWidth", &g_ais_cog_predictor_width);
  CHECK_INT("bAISAlertAudio", &g_bAIS_CPA_Alert_Audio);
  CHECK_STR("AISAlertAudioFile", g_sAIS_Alert_Sound_File);
  CHECK_INT("bAISAlertSuppressMoored", &g_bAIS_CPA_Alert_Suppress_Moored);
  CHECK_INT("bAISAlertAckTimeout", &g_bAIS_ACK_Timeout);
  CHECK_FLT("AlertAckTimeoutMinutes", &g_AckTimeout_Mins, 1)
  CHECK_STR("AISTargetListPerspective", g_AisTargetList_perspective);
  CHECK_INT("AISTargetListRange", &g_AisTargetList_range);
  CHECK_INT("AISTargetListSortColumn", &g_AisTargetList_sortColumn);
  CHECK_INT("bAISTargetListSortReverse", &g_bAisTargetList_sortReverse);
  CHECK_STR("AISTargetListColumnSpec", g_AisTargetList_column_spec);
  CHECK_STR("AISTargetListColumnOrder", g_AisTargetList_column_order);
  CHECK_INT("bAISRolloverShowClass", &g_bAISRolloverShowClass);
  CHECK_INT("bAISRolloverShowCOG", &g_bAISRolloverShowCOG);
  CHECK_INT("bAISRolloverShowCPA", &g_bAISRolloverShowCPA);

  CHECK_INT("S57QueryDialogSizeX", &g_S57_dialog_sx);
  CHECK_INT("S57QueryDialogSizeY", &g_S57_dialog_sy);
  CHECK_INT("AlertDialogSizeX", &g_ais_alert_dialog_sx);
  CHECK_INT("AlertDialogSizeY", &g_ais_alert_dialog_sy);
  CHECK_INT("AlertDialogPosX", &g_ais_alert_dialog_x);
  CHECK_INT("AlertDialogPosY", &g_ais_alert_dialog_y);
  CHECK_INT("QueryDialogPosX", &g_ais_query_dialog_x);
  CHECK_INT("QueryDialogPosY", &g_ais_query_dialog_y);

  conf.SetPath("/Directories");
  CHECK_STR("PresentationLibraryData", g_UserPresLibData)
  /// CHECK_STRP( "InitChartDir", pInit_Chart_Dir)

  CHECK_STR("SENCFileLocation", g_SENCPrefix)

  CHECK_STR("GPXIODir", g_gpx_path);     // Get the Directory name
  CHECK_STR("TCDataDir", g_TCData_Dir);  // Get the Directory name
  CHECK_STR("BasemapDir", gWorldMapLocation);
  CHECK_STR("BaseShapefileDir", gWorldShapefileLocation);

  //    Fonts

#if 0
    //  Load the persistent Auxiliary Font descriptor Keys
    conf.SetPath ( "/Settings/AuxFontKeys" );

    wxString strk;
    long dummyk;
    wxString kval;
    bool bContk = conf,GetFirstEntry( strk, dummyk );
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
  conf.SetPath("/Settings/X11Fonts");
#endif

#ifdef __WXGTK__
  conf.SetPath("/Settings/GTKFonts");
#endif

#ifdef __WXMSW__
  conf.SetPath("/Settings/MSWFonts");
#endif

#ifdef __WXMAC__
  conf.SetPath("/Settings/MacFonts");
#endif

#ifdef __WXQT__
  conf.SetPath("/Settings/QTFonts");
#endif

  conf.SetPath("/Settings/Others");

  // Radar rings
  CHECK_INT("RadarRingsNumberVisible", &g_iNavAidRadarRingsNumberVisible)
  CHECK_INT("RadarRingsStep", &g_fNavAidRadarRingsStep)

  CHECK_INT("RadarRingsStepUnits", &g_pNavAidRadarRingsStepUnits);

  //    wxString l_wxsOwnshipRangeRingsColour;
  //    CHECK_STR( "RadarRingsColour", &l_wxsOwnshipRangeRingsColour );
  //    if(l_wxsOwnshipRangeRingsColour.Length())
  //    g_colourOwnshipRangeRingsColour.Set( l_wxsOwnshipRangeRingsColour );

  // Waypoint Radar rings
  CHECK_INT("WaypointRangeRingsNumber", &g_iWaypointRangeRingsNumber)

  CHECK_FLT("WaypointRangeRingsStep", &g_fWaypointRangeRingsStep, .1)

  CHECK_INT("WaypointRangeRingsStepUnits", &g_iWaypointRangeRingsStepUnits);

  //    wxString l_wxsWaypointRangeRingsColour;
  //    CHECK_STR( "WaypointRangeRingsColour",
  //    &l_wxsWaypointRangeRingsColour ); g_colourWaypointRangeRingsColour.Set(
  //    l_wxsWaypointRangeRingsColour );

  CHECK_INT("ConfirmObjectDeletion", &g_bConfirmObjectDelete);

  // Waypoint dragging with mouse
  CHECK_INT("WaypointPreventDragging", &g_bWayPointPreventDragging);

  CHECK_INT("EnableZoomToCursor", &g_bEnableZoomToCursor);

  CHECK_FLT("TrackIntervalSeconds", &g_TrackIntervalSeconds, 1)

  CHECK_FLT("TrackDeltaDistance", &g_TrackDeltaDistance, .1)

  CHECK_INT("TrackPrecision", &g_nTrackPrecision);

  // CHECK_STR( "NavObjectFileName", m_sNavObjSetFile );

  CHECK_INT("RouteLineWidth", &g_route_line_width);
  CHECK_INT("TrackLineWidth", &g_track_line_width);

  //     wxString l_wxsTrackLineColour;
  //     CHECK_STR( "TrackLineColour", l_wxsTrackLineColour )
  //         g_colourTrackLineColour.Set( l_wxsTrackLineColour );

  CHECK_STR("DefaultWPIcon", g_default_wp_icon)

  // S57 template items

#define CHECK_BFN(s, t)    \
  conf.Read(s, &read_int); \
  bval = t;                \
  bval0 = read_int != 0;   \
  if (bval != bval0) return false;

#define CHECK_IFN(s, t)    \
  conf.Read(s, &read_int); \
  if (read_int != t) return false;

#define CHECK_FFN(s, t) \
  conf.Read(s, &dval);  \
  if (fabs(dval - t) > 0.1) return false;

  if (ps52plib) {
    int read_int;
    double dval;
    bool bval, bval0;

    conf.SetPath("/Settings/GlobalState");

    CHECK_BFN("bShowS57Text", ps52plib->GetShowS57Text());

    CHECK_BFN("bShowS57ImportantTextOnly",
              ps52plib->GetShowS57ImportantTextOnly());
    CHECK_BFN("bShowLightDescription", ps52plib->m_bShowLdisText);
    CHECK_BFN("bExtendLightSectors", ps52plib->m_bExtendLightSectors);
    CHECK_BFN("bShowSoundg", ps52plib->m_bShowSoundg);
    CHECK_BFN("bShowMeta", ps52plib->m_bShowMeta);
    CHECK_BFN("bUseSCAMIN", ps52plib->m_bUseSCAMIN);
    CHECK_BFN("bUseSUPERSCAMIN", ps52plib->m_bUseSUPER_SCAMIN);
    CHECK_BFN("bShowAtonText", ps52plib->m_bShowAtonText);
    CHECK_BFN("bDeClutterText", ps52plib->m_bDeClutterText);
    CHECK_BFN("bShowNationalText", ps52plib->m_bShowNationalTexts);
    CHECK_IFN("nDisplayCategory", ps52plib->GetDisplayCategory());
    CHECK_IFN("nSymbolStyle", ps52plib->m_nSymbolStyle);
    CHECK_IFN("nBoundaryStyle", ps52plib->m_nBoundaryStyle);
    CHECK_FFN("S52_MAR_SAFETY_CONTOUR",
              S52_getMarinerParam(S52_MAR_SAFETY_CONTOUR));
    CHECK_FFN("S52_MAR_SHALLOW_CONTOUR",
              S52_getMarinerParam(S52_MAR_SHALLOW_CONTOUR));
    CHECK_FFN("S52_MAR_DEEP_CONTOUR",
              S52_getMarinerParam(S52_MAR_DEEP_CONTOUR));
    CHECK_FFN("S52_MAR_TWO_SHADES", S52_getMarinerParam(S52_MAR_TWO_SHADES));
    CHECK_INT("S52_DEPTH_UNIT_SHOW", &g_nDepthUnitDisplay);

    //    S57 Object Class Visibility

    OBJLElement *pOLE;

    conf.SetPath("/Settings/ObjectFilter");

    unsigned int iOBJMax = conf.GetNumberOfEntries();

    if (iOBJMax != ps52plib->pOBJLArray->GetCount()) return false;

    if (iOBJMax) {
      wxString str, sObj;
      long val;
      long dummy;

      bool bCont = conf.GetFirstEntry(str, dummy);
      while (bCont) {
        conf.Read(str, &val);  // Get an Object Viz

        // scan for the same key in the global list
        bool bfound = false;
        if (str.StartsWith("viz", &sObj)) {
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
        bCont = conf.GetNextEntry(str, dummy);
      }
    }
  }

  conf.SetPath("/MmsiProperties");
  int iPMax = conf.GetNumberOfEntries();
  if (iPMax) {
    wxString str, val;
    long dummy;

    bool bCont = conf.GetFirstEntry(str, dummy);
    while (bCont) {
      conf.Read(str, &val);  // Get an entry

      bool bfound = false;
      for (unsigned int j = 0; j < g_MMSI_Props_Array.GetCount(); j++) {
        MmsiProperties *pProps = g_MMSI_Props_Array.Item(j);
        if (pProps->Serialize().IsSameAs(val)) {
          bfound = true;
          break;
        }
      }
      if (!bfound) return false;

      bCont = conf.GetNextEntry(str, dummy);
    }
  }

  return rv;
}
