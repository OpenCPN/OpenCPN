/**************************************************************************
 *   Copyright (C) 2024 by David S. Register                               *
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
 * \file
 * PlugIn GUI API Functions
 */

#include "dychart.h"  // Must be ahead due to buggy GL includes handling

#include <wx/wx.h>
#include <wx/arrstr.h>
#include <wx/dc.h>
#include <wx/dcmemory.h>
#include <wx/event.h>
#include <wx/glcanvas.h>
#include <wx/notebook.h>
#include <wx/string.h>
#include <wx/window.h>

#include "model/ais_decoder.h"
#include "model/comm_navmsg_bus.h"
#include "model/idents.h"
#include "model/multiplexer.h"
#include "model/notification_manager.h"
#include "model/own_ship.h"
#include "model/plugin_comm.h"
#include "model/route.h"
#include "model/track.h"

#include "ais.h"
#include "chartdb.h"
#include "chcanv.h"
#include "ConfigMgr.h"
#include "FontMgr.h"
#include "glChartCanvas.h"
#include "gui_lib.h"
#include "navutil.h"
#include "ocpn_app.h"
#include "OCPN_AUIManager.h"
#include "ocpn_frame.h"
#include "OCPNPlatform.h"
#include "ocpn_plugin.h"
#include "options.h"
#include "piano.h"
#include "pluginmanager.h"
#include "routemanagerdialog.h"
#include "routeman_gui.h"
#include "s52plib.h"
#include "SoundFactory.h"
#include "svg_utils.h"
#include "SystemCmdSound.h"
#include "toolbar.h"
#include "waypointman_gui.h"
#include "shapefile_basemap.h"
#include "model/navobj_db.h"

extern PlugInManager* s_ppim;
extern MyConfig* pConfig;
extern OCPN_AUIManager* g_pauimgr;

#if wxUSE_XLOCALE || !wxCHECK_VERSION(3, 0, 0)
extern wxLocale* plocale_def_lang;
#endif

extern OCPNPlatform* g_Platform;
extern ChartDB* ChartData;
extern MyFrame* gFrame;
extern ocpnStyle::StyleManager* g_StyleManager;
extern options* g_pOptions;
extern Multiplexer* g_pMUX;
extern bool g_bShowChartBar;
extern Routeman* g_pRouteMan;
extern Select* pSelect;
extern RouteManagerDialog* pRouteManagerDialog;
extern RouteList* pRouteList;
extern std::vector<Track*> g_TrackList;
extern PlugInManager* g_pi_manager;
extern s52plib* ps52plib;
extern wxString ChartListFileName;
extern options* g_options;
extern ColorScheme global_color_scheme;
extern wxArrayString g_locale_catalog_array;
extern int g_GUIScaleFactor;
extern int g_ChartScaleFactor;
extern wxString g_locale;
extern ocpnFloatingToolbarDialog* g_MainToolbar;

extern int g_chart_zoom_modifier_raster;
extern int g_chart_zoom_modifier_vector;
extern double g_display_size_mm;
extern bool g_bopengl;
extern AisDecoder* g_pAIS;
extern ChartGroupArray* g_pGroupArray;
extern ShapeBaseChartSet gShapeBasemap;

// extern ChartGroupArray* g_pGroupArray;
extern unsigned int g_canvasConfig;

extern wxString g_CmdSoundString;

unsigned int gs_plib_flags;
extern ChartCanvas* g_focusCanvas;
extern ChartCanvas* g_overlayCanvas;
extern bool g_bquiting;
extern bool g_disable_main_toolbar;
extern bool g_btenhertz;
extern bool g_CanvasHideNotificationIcon;

WX_DEFINE_ARRAY_PTR(ChartCanvas*, arrayofCanvasPtr);
extern arrayofCanvasPtr g_canvasArray;

void NotifySetupOptionsPlugin(const PlugInData* pic);

//---------------------------------------------------------------------------
/*  Implementation of OCPN core functions callable by plugins
 *  Sorted by API version number
 *  The definitions of this API are found in ocpn_plugin.h
 *  PlugIns may call these static functions as necessary for system services
 */
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//    API 1.6
//---------------------------------------------------------------------------
/*  Main Toolbar support  */
int InsertPlugInTool(wxString label, wxBitmap* bitmap, wxBitmap* bmpRollover,
                     wxItemKind kind, wxString shortHelp, wxString longHelp,
                     wxObject* clientData, int position, int tool_sel,
                     opencpn_plugin* pplugin) {
  if (s_ppim)
    return s_ppim->AddToolbarTool(label, bitmap, bmpRollover, kind, shortHelp,
                                  longHelp, clientData, position, tool_sel,
                                  pplugin);
  else
    return -1;
}

void RemovePlugInTool(int tool_id) {
  if (s_ppim) s_ppim->RemoveToolbarTool(tool_id);
}

void SetToolbarToolViz(int item, bool viz) {
  if (s_ppim) s_ppim->SetToolbarToolViz(item, viz);
}

void SetToolbarItemState(int item, bool toggle) {
  if (s_ppim) s_ppim->SetToolbarItemState(item, toggle);
}

void SetToolbarToolBitmaps(int item, wxBitmap* bitmap, wxBitmap* bmpRollover) {
  if (s_ppim) s_ppim->SetToolbarItemBitmaps(item, bitmap, bmpRollover);
}

int InsertPlugInToolSVG(wxString label, wxString SVGfile,
                        wxString SVGfileRollover, wxString SVGfileToggled,
                        wxItemKind kind, wxString shortHelp, wxString longHelp,
                        wxObject* clientData, int position, int tool_sel,
                        opencpn_plugin* pplugin) {
  if (s_ppim)
    return s_ppim->AddToolbarTool(label, SVGfile, SVGfileRollover,
                                  SVGfileToggled, kind, shortHelp, longHelp,
                                  clientData, position, tool_sel, pplugin);
  else
    return -1;
}

void SetToolbarToolBitmapsSVG(int item, wxString SVGfile,
                              wxString SVGfileRollover,
                              wxString SVGfileToggled) {
  if (s_ppim)
    s_ppim->SetToolbarItemBitmaps(item, SVGfile, SVGfileRollover,
                                  SVGfileToggled);
}

/*  Canvas Context Menu support  */
int AddCanvasMenuItem(wxMenuItem* pitem, opencpn_plugin* pplugin,
                      const char* name) {
  if (s_ppim)
    return s_ppim->AddCanvasContextMenuItemPIM(pitem, pplugin, name, false);
  else
    return -1;
}

void SetCanvasMenuItemViz(int item, bool viz, const char* name) {
  if (s_ppim) s_ppim->SetCanvasContextMenuItemViz(item, viz, name);
}

void SetCanvasMenuItemGrey(int item, bool grey, const char* name) {
  if (s_ppim) s_ppim->SetCanvasContextMenuItemGrey(item, grey, name);
}

void RemoveCanvasMenuItem(int item, const char* name) {
  if (s_ppim) s_ppim->RemoveCanvasContextMenuItem(item, name);
}

int AddCanvasContextMenuItem(wxMenuItem* pitem, opencpn_plugin* pplugin) {
  /* main context popup menu */
  if (s_ppim)
    return s_ppim->AddCanvasContextMenuItemPIM(pitem, pplugin, "", false);
  else
    return -1;
}

void SetCanvasContextMenuItemViz(int item, bool viz) {
  SetCanvasMenuItemViz(item, viz);
}

void SetCanvasContextMenuItemGrey(int item, bool grey) {
  SetCanvasMenuItemGrey(item, grey);
}

void RemoveCanvasContextMenuItem(int item) { RemoveCanvasMenuItem(item); }

int AddCanvasContextMenuItemExt(wxMenuItem* pitem, opencpn_plugin* pplugin,
                                const std::string object_type) {
  /* main context popup menu */
  if (s_ppim)
    return s_ppim->AddCanvasContextMenuItemPIM(pitem, pplugin,
                                               object_type.c_str(), true);
  else
    return -1;
}

/*  Utility functions  */
wxFileConfig* GetOCPNConfigObject(void) {
  if (s_ppim)
    return reinterpret_cast<wxFileConfig*>(
        pConfig);  // return the global application config object
  else
    return NULL;
}

wxWindow* GetOCPNCanvasWindow() {
  wxWindow* pret = NULL;
  if (s_ppim) {
    MyFrame* pFrame = s_ppim->GetParentFrame();
    pret = (wxWindow*)pFrame->GetPrimaryCanvas();
  }
  return pret;
}

void RequestRefresh(wxWindow* win) {
  if (win) win->Refresh(true);
}

void GetCanvasPixLL(PlugIn_ViewPort* vp, wxPoint* pp, double lat, double lon) {
  //    Make enough of an application viewport to run its method....
  ViewPort ocpn_vp;
  ocpn_vp.clat = vp->clat;
  ocpn_vp.clon = vp->clon;
  ocpn_vp.m_projection_type = vp->m_projection_type;
  ocpn_vp.view_scale_ppm = vp->view_scale_ppm;
  ocpn_vp.skew = vp->skew;
  ocpn_vp.rotation = vp->rotation;
  ocpn_vp.pix_width = vp->pix_width;
  ocpn_vp.pix_height = vp->pix_height;

  wxPoint ret = ocpn_vp.GetPixFromLL(lat, lon);
  pp->x = ret.x;
  pp->y = ret.y;
}

void GetDoubleCanvasPixLL(PlugIn_ViewPort* vp, wxPoint2DDouble* pp, double lat,
                          double lon) {
  //    Make enough of an application viewport to run its method....
  ViewPort ocpn_vp;
  ocpn_vp.clat = vp->clat;
  ocpn_vp.clon = vp->clon;
  ocpn_vp.m_projection_type = vp->m_projection_type;
  ocpn_vp.view_scale_ppm = vp->view_scale_ppm;
  ocpn_vp.skew = vp->skew;
  ocpn_vp.rotation = vp->rotation;
  ocpn_vp.pix_width = vp->pix_width;
  ocpn_vp.pix_height = vp->pix_height;

  *pp = ocpn_vp.GetDoublePixFromLL(lat, lon);
}

void GetCanvasLLPix(PlugIn_ViewPort* vp, wxPoint p, double* plat,
                    double* plon) {
  //    Make enough of an application viewport to run its method....
  ViewPort ocpn_vp;
  ocpn_vp.clat = vp->clat;
  ocpn_vp.clon = vp->clon;
  ocpn_vp.m_projection_type = vp->m_projection_type;
  ocpn_vp.view_scale_ppm = vp->view_scale_ppm;
  ocpn_vp.skew = vp->skew;
  ocpn_vp.rotation = vp->rotation;
  ocpn_vp.pix_width = vp->pix_width;
  ocpn_vp.pix_height = vp->pix_height;

  return ocpn_vp.GetLLFromPix(p, plat, plon);
}

bool GetGlobalColor(wxString colorName, wxColour* pcolour) {
  wxColour c = GetGlobalColor(colorName);
  *pcolour = c;

  return true;
}

wxFont* OCPNGetFont(wxString TextElement, int default_size) {
  return FontMgr::Get().GetFont(TextElement, default_size);
}

wxFont* GetOCPNScaledFont_PlugIn(wxString TextElement, int default_size) {
  return GetOCPNScaledFont(TextElement, default_size);
}

double GetOCPNGUIToolScaleFactor_PlugIn(int GUIScaleFactor) {
  return g_Platform->GetToolbarScaleFactor(GUIScaleFactor);
}

double GetOCPNGUIToolScaleFactor_PlugIn() {
  return g_Platform->GetToolbarScaleFactor(g_GUIScaleFactor);
}

float GetOCPNChartScaleFactor_Plugin() {
  return g_Platform->GetChartScaleFactorExp(g_ChartScaleFactor);
}

wxFont GetOCPNGUIScaledFont_PlugIn(wxString item) {
  return GetOCPNGUIScaledFont(item);
}

bool AddPersistentFontKey(wxString TextElement) {
  return FontMgr::Get().AddAuxKey(TextElement);
}

wxString GetActiveStyleName() {
  if (g_StyleManager)
    return g_StyleManager->GetCurrentStyle()->name;
  else
    return _T("");
}

wxBitmap GetBitmapFromSVGFile(wxString filename, unsigned int width,
                              unsigned int height) {
  wxBitmap bmp = LoadSVG(filename, width, height);

  if (bmp.IsOk())
    return bmp;
  else {
    // On error in requested width/height parameters,
    // try to find and use dimensions embedded in the SVG file
    unsigned int w, h;
    SVGDocumentPixelSize(filename, w, h);
    if (w == 0 || h == 0) {
      // We did not succeed in deducing the size from SVG (svg element
      // x misses width, height or both attributes), let's use some "safe"
      // default
      w = 32;
      h = 32;
    }
    return LoadSVG(filename, w, h);
  }
}

bool IsTouchInterface_PlugIn(void) { return g_btouch; }

wxColour GetFontColour_PlugIn(wxString TextElement) {
  return FontMgr::Get().GetFontColor(TextElement);
}

wxString* GetpSharedDataLocation(void) {
  return g_Platform->GetSharedDataDirPtr();
}

ArrayOfPlugIn_AIS_Targets* GetAISTargetArray(void) {
  if (!g_pAIS) return NULL;

  ArrayOfPlugIn_AIS_Targets* pret = new ArrayOfPlugIn_AIS_Targets;

  //      Iterate over the AIS Target Hashmap
  for (const auto& it : g_pAIS->GetTargetList()) {
    auto td = it.second;
    PlugIn_AIS_Target* ptarget = Create_PI_AIS_Target(td.get());
    pret->Add(ptarget);
  }

//  Test one alarm target
#if 0
    AisTargetData td;
    td.n_alarm_state = AIS_ALARM_SET;
    PlugIn_AIS_Target *ptarget = Create_PI_AIS_Target(&td);
    pret->Add(ptarget);
#endif
  return pret;
}

wxAuiManager* GetFrameAuiManager(void) { return g_pauimgr; }

void SendPluginMessage(wxString message_id, wxString message_body) {
  SendMessageToAllPlugins(message_id, message_body);

  //  We will send an event to the main application frame (gFrame)
  //  for informational purposes.
  //  Of course, gFrame is encouraged to use any or all the
  //  data flying by if judged useful and dependable....

  OCPN_MsgEvent Nevent(wxEVT_OCPN_MSG, 0);
  Nevent.SetID(message_id);
  Nevent.SetJSONText(message_body);
  gFrame->GetEventHandler()->AddPendingEvent(Nevent);
}

void DimeWindow(wxWindow* win) { DimeControl(win); }

void JumpToPosition(double lat, double lon, double scale) {
  gFrame->JumpToPosition(gFrame->GetFocusCanvas(), lat, lon, scale);
}

/*  Locale (i18N) support  */
bool AddLocaleCatalog(wxString catalog) {
#if wxUSE_XLOCALE || !wxCHECK_VERSION(3, 0, 0)

  if (plocale_def_lang) {
    // Add this catalog to the persistent catalog array
    g_locale_catalog_array.Add(catalog);

    return plocale_def_lang->AddCatalog(catalog);
  } else
#endif
    return false;
}

wxString GetLocaleCanonicalName() { return g_locale; }

/*  NMEA interface support  */
void PushNMEABuffer(wxString buf) {
  std::string full_sentence = buf.ToStdString();

  if ((full_sentence[0] == '$') || (full_sentence[0] == '!')) {  // Sanity check
    std::string identifier;
    // We notify based on full message, including the Talker ID
    identifier = full_sentence.substr(1, 5);

    // notify message listener and also "ALL" N0183 messages, to support plugin
    // API using original talker id
    auto address = std::make_shared<NavAddr0183>("virtual");
    auto msg =
        std::make_shared<const Nmea0183Msg>(identifier, full_sentence, address);
    auto msg_all = std::make_shared<const Nmea0183Msg>(*msg, "ALL");

    auto& msgbus = NavMsgBus::GetInstance();

    msgbus.Notify(std::move(msg));
    msgbus.Notify(std::move(msg_all));
  }
}

/*  Chart database access support  */
wxXmlDocument GetChartDatabaseEntryXML(int dbIndex, bool b_getGeom) {
  wxXmlDocument doc = ChartData->GetXMLDescription(dbIndex, b_getGeom);

  return doc;
}

bool UpdateChartDBInplace(wxArrayString dir_array, bool b_force_update,
                          bool b_ProgressDialog) {
  //    Make an array of CDI
  ArrayOfCDI ChartDirArray;
  for (unsigned int i = 0; i < dir_array.GetCount(); i++) {
    wxString dirname = dir_array[i];
    ChartDirInfo cdi;
    cdi.fullpath = dirname;
    cdi.magic_number = _T("");
    ChartDirArray.Add(cdi);
  }
  bool b_ret = gFrame->UpdateChartDatabaseInplace(ChartDirArray, b_force_update,
                                                  b_ProgressDialog,
                                                  ChartData->GetDBFileName());
  gFrame->ChartsRefresh();
  return b_ret;
}

wxArrayString GetChartDBDirArrayString() {
  return ChartData->GetChartDirArrayString();
}

int AddChartToDBInPlace(wxString& full_path, bool b_RefreshCanvas) {
  // extract the path from the chart name
  wxFileName fn(full_path);
  wxString fdir = fn.GetPath();

  bool bret = false;
  if (ChartData) {
    bret = ChartData->AddSingleChart(full_path);

    if (bret) {
      // Save to disk
      pConfig->UpdateChartDirs(ChartData->GetChartDirArray());
      ChartData->SaveBinary(ChartListFileName);

      //  Completely reload the chart database, for a fresh start
      ArrayOfCDI XnewChartDirArray;
      pConfig->LoadChartDirArray(XnewChartDirArray);
      delete ChartData;
      ChartData = new ChartDB();
      ChartData->LoadBinary(ChartListFileName, XnewChartDirArray);

      // Update group contents
      if (g_pGroupArray) ChartData->ApplyGroupArray(g_pGroupArray);

      if (g_options && g_options->IsShown())
        g_options->UpdateDisplayedChartDirList(ChartData->GetChartDirArray());

      if (b_RefreshCanvas || !gFrame->GetPrimaryCanvas()->GetQuiltMode()) {
        gFrame->ChartsRefresh();
      }
    }
  }
  return bret;
}

int RemoveChartFromDBInPlace(wxString& full_path) {
  bool bret = false;
  if (ChartData) {
    bret = ChartData->RemoveSingleChart(full_path);

    // Save to disk
    pConfig->UpdateChartDirs(ChartData->GetChartDirArray());
    ChartData->SaveBinary(ChartListFileName);

    //  Completely reload the chart database, for a fresh start
    ArrayOfCDI XnewChartDirArray;
    pConfig->LoadChartDirArray(XnewChartDirArray);
    delete ChartData;
    ChartData = new ChartDB();
    ChartData->LoadBinary(ChartListFileName, XnewChartDirArray);

    // Update group contents
    if (g_pGroupArray) ChartData->ApplyGroupArray(g_pGroupArray);

    if (g_options && g_options->IsShown())
      g_options->UpdateDisplayedChartDirList(ChartData->GetChartDirArray());

    gFrame->ChartsRefresh();
  }

  return bret;
}

//---------------------------------------------------------------------------
//    API 1.9
//---------------------------------------------------------------------------
wxScrolledWindow* AddOptionsPage(OptionsParentPI parent, wxString title) {
  if (!g_pOptions) return NULL;

  size_t parentid;
  switch (parent) {
    case PI_OPTIONS_PARENT_DISPLAY:
      parentid = g_pOptions->m_pageDisplay;
      break;
    case PI_OPTIONS_PARENT_CONNECTIONS:
      parentid = g_pOptions->m_pageConnections;
      break;
    case PI_OPTIONS_PARENT_CHARTS:
      parentid = g_pOptions->m_pageCharts;
      break;
    case PI_OPTIONS_PARENT_SHIPS:
      parentid = g_pOptions->m_pageShips;
      break;
    case PI_OPTIONS_PARENT_UI:
      parentid = g_pOptions->m_pageUI;
      break;
    case PI_OPTIONS_PARENT_PLUGINS:
      parentid = g_pOptions->m_pagePlugins;
      break;
    default:
      wxLogMessage(
          _T("Error in PluginManager::AddOptionsPage: Unknown parent"));
      return NULL;
      break;
  }

  return g_pOptions->AddPage(parentid, title);
}

bool DeleteOptionsPage(wxScrolledWindow* page) {
  if (!g_pOptions) return false;
  return g_pOptions->DeletePluginPage(page);
}

bool DecodeSingleVDOMessage(const wxString& str, PlugIn_Position_Fix_Ex* pos,
                            wxString* accumulator) {
  if (!pos) return false;

  GenericPosDatEx gpd;
  AisError nerr = AIS_GENERIC_ERROR;
  if (g_pAIS) nerr = g_pAIS->DecodeSingleVDO(str, &gpd, accumulator);
  if (nerr == AIS_NoError) {
    pos->Lat = gpd.kLat;
    pos->Lon = gpd.kLon;
    pos->Cog = gpd.kCog;
    pos->Sog = gpd.kSog;
    pos->Hdt = gpd.kHdt;

    //  Fill in the dummy values
    pos->FixTime = 0;
    pos->Hdm = 1000;
    pos->Var = 1000;
    pos->nSats = 0;

    return true;
  }

  return false;
}

int GetChartbarHeight(void) {
  int val = 0;
  if (g_bShowChartBar) {
    ChartCanvas* cc = gFrame->GetPrimaryCanvas();
    if (cc && cc->GetPiano()) {
      val = cc->GetPiano()->GetHeight();
    }
  }
  return val;
}

bool GetRoutepointGPX(RoutePoint* pRoutePoint, char* buffer,
                      unsigned int buffer_length) {
  bool ret = false;

  NavObjectCollection1* pgpx = new NavObjectCollection1;
  pgpx->AddGPXWaypoint(pRoutePoint);
  wxString gpxfilename = wxFileName::CreateTempFileName(wxT("gpx"));
  pgpx->SaveFile(gpxfilename);
  delete pgpx;

  wxFFile gpxfile(gpxfilename);
  wxString s;
  if (gpxfile.ReadAll(&s)) {
    if (s.Length() < buffer_length) {
      strncpy(buffer, (const char*)s.mb_str(wxConvUTF8), buffer_length - 1);
      ret = true;
    }
  }

  gpxfile.Close();
  ::wxRemoveFile(gpxfilename);

  return ret;
}

bool GetActiveRoutepointGPX(char* buffer, unsigned int buffer_length) {
  if (g_pRouteMan->IsAnyRouteActive())
    return GetRoutepointGPX(g_pRouteMan->GetpActivePoint(), buffer,
                            buffer_length);
  else
    return false;
}

void PositionBearingDistanceMercator_Plugin(double lat, double lon, double brg,
                                            double dist, double* dlat,
                                            double* dlon) {
  PositionBearingDistanceMercator(lat, lon, brg, dist, dlat, dlon);
}

void DistanceBearingMercator_Plugin(double lat0, double lon0, double lat1,
                                    double lon1, double* brg, double* dist) {
  DistanceBearingMercator(lat0, lon0, lat1, lon1, brg, dist);
}

double DistGreatCircle_Plugin(double slat, double slon, double dlat,
                              double dlon) {
  return DistGreatCircle(slat, slon, dlat, dlon);
}

void toTM_Plugin(float lat, float lon, float lat0, float lon0, double* x,
                 double* y) {
  toTM(lat, lon, lat0, lon0, x, y);
}

void fromTM_Plugin(double x, double y, double lat0, double lon0, double* lat,
                   double* lon) {
  fromTM(x, y, lat0, lon0, lat, lon);
}

void toSM_Plugin(double lat, double lon, double lat0, double lon0, double* x,
                 double* y) {
  toSM(lat, lon, lat0, lon0, x, y);
}

void fromSM_Plugin(double x, double y, double lat0, double lon0, double* lat,
                   double* lon) {
  fromSM(x, y, lat0, lon0, lat, lon);
}

void toSM_ECC_Plugin(double lat, double lon, double lat0, double lon0,
                     double* x, double* y) {
  toSM_ECC(lat, lon, lat0, lon0, x, y);
}

void fromSM_ECC_Plugin(double x, double y, double lat0, double lon0,
                       double* lat, double* lon) {
  fromSM_ECC(x, y, lat0, lon0, lat, lon);
}

double toUsrDistance_Plugin(double nm_distance, int unit) {
  return toUsrDistance(nm_distance, unit);
}

double fromUsrDistance_Plugin(double usr_distance, int unit) {
  return fromUsrDistance(usr_distance, unit);
}

double toUsrSpeed_Plugin(double kts_speed, int unit) {
  return toUsrSpeed(kts_speed, unit);
}

double toUsrWindSpeed_Plugin(double kts_speed, int unit) {
  return toUsrWindSpeed(kts_speed, unit);
}

double fromUsrSpeed_Plugin(double usr_speed, int unit) {
  return fromUsrSpeed(usr_speed, unit);
}

double fromUsrWindSpeed_Plugin(double usr_wspeed, int unit) {
  return fromUsrWindSpeed(usr_wspeed, unit);
}

double toUsrTemp_Plugin(double cel_temp, int unit) {
  return toUsrTemp(cel_temp, unit);
}

double fromUsrTemp_Plugin(double usr_temp, int unit) {
  return fromUsrTemp(usr_temp, unit);
}

wxString getUsrDistanceUnit_Plugin(int unit) {
  return getUsrDistanceUnit(unit);
}

wxString getUsrSpeedUnit_Plugin(int unit) { return getUsrSpeedUnit(unit); }

wxString getUsrWindSpeedUnit_Plugin(int unit) {
  return getUsrWindSpeedUnit(unit);
}

wxString getUsrTempUnit_Plugin(int unit) { return getUsrTempUnit(unit); }

/*
 * Depth Conversion Functions
 */
double toUsrDepth_Plugin(double m_depth, int unit) {
  return toUsrDepth(m_depth, unit);
}

double fromUsrDepth_Plugin(double usr_depth, int unit) {
  return fromUsrDepth(usr_depth, unit);
}

wxString getUsrDepthUnit_Plugin(int unit) { return getUsrDepthUnit(unit); }

double fromDMM_PlugIn(wxString sdms) { return fromDMM(sdms); }

bool PlugIn_GSHHS_CrossesLand(double lat1, double lon1, double lat2,
                              double lon2) {
  // TODO: Enable call to gShapeBasemap.CrossesLand after fixing performance
  // issues. if (gShapeBasemap.IsUsable()) {
  //   return gShapeBasemap.CrossesLand(lat1, lon1, lat2, lon2);
  // } else {
  //  Fall back to the GSHHS data.
  static bool loaded = false;
  if (!loaded) {
    gshhsCrossesLandInit();
    loaded = true;
  }
  return gshhsCrossesLand(lat1, lon1, lat2, lon2);
  //}
}

void PlugInPlaySound(wxString& sound_file) {
  PlugInPlaySoundEx(sound_file, -1);
}

//---------------------------------------------------------------------------
//    API 1.10
//---------------------------------------------------------------------------

// API Route and Waypoint Support
PlugIn_Waypoint::PlugIn_Waypoint() { m_HyperlinkList = NULL; }

PlugIn_Waypoint::PlugIn_Waypoint(double lat, double lon,
                                 const wxString& icon_ident,
                                 const wxString& wp_name,
                                 const wxString& GUID) {
  wxDateTime now = wxDateTime::Now();
  m_CreateTime = now.ToUTC();
  m_HyperlinkList = NULL;

  m_lat = lat;
  m_lon = lon;
  m_IconName = icon_ident;
  m_MarkName = wp_name;
  m_GUID = GUID;
}

PlugIn_Waypoint::~PlugIn_Waypoint() {}

//      PlugInRoute implementation
PlugIn_Route::PlugIn_Route(void) { pWaypointList = new Plugin_WaypointList; }

PlugIn_Route::~PlugIn_Route(void) {
  pWaypointList->DeleteContents(false);  // do not delete Waypoints
  pWaypointList->Clear();

  delete pWaypointList;
}

//      PlugInTrack implementation
PlugIn_Track::PlugIn_Track(void) { pWaypointList = new Plugin_WaypointList; }

PlugIn_Track::~PlugIn_Track(void) {
  pWaypointList->DeleteContents(false);  // do not delete Waypoints
  pWaypointList->Clear();

  delete pWaypointList;
}

wxString GetNewGUID(void) { return GpxDocument::GetUUID(); }

bool AddCustomWaypointIcon(wxBitmap* pimage, wxString key,
                           wxString description) {
  wxImage image = pimage->ConvertToImage();
  WayPointmanGui(*pWayPointMan).ProcessIcon(image, key, description);
  return true;
}

static void cloneHyperlinkList(RoutePoint* dst, const PlugIn_Waypoint* src) {
  //  Transcribe (clone) the html HyperLink List, if present
  if (src->m_HyperlinkList == nullptr) return;

  if (src->m_HyperlinkList->GetCount() > 0) {
    wxPlugin_HyperlinkListNode* linknode = src->m_HyperlinkList->GetFirst();
    while (linknode) {
      Plugin_Hyperlink* link = linknode->GetData();

      Hyperlink* h = new Hyperlink();
      h->DescrText = link->DescrText;
      h->Link = link->Link;
      h->LType = link->Type;

      dst->m_HyperlinkList->Append(h);

      linknode = linknode->GetNext();
    }
  }
}

bool AddSingleWaypoint(PlugIn_Waypoint* pwaypoint, bool b_permanent) {
  //  Validate the waypoint parameters a little bit

  //  GUID
  //  Make sure that this GUID is indeed unique in the Routepoint list
  bool b_unique = true;
  wxRoutePointListNode* prpnode = pWayPointMan->GetWaypointList()->GetFirst();
  while (prpnode) {
    RoutePoint* prp = prpnode->GetData();

    if (prp->m_GUID == pwaypoint->m_GUID) {
      b_unique = false;
      break;
    }
    prpnode = prpnode->GetNext();  // RoutePoint
  }

  if (!b_unique) return false;

  RoutePoint* pWP =
      new RoutePoint(pwaypoint->m_lat, pwaypoint->m_lon, pwaypoint->m_IconName,
                     pwaypoint->m_MarkName, pwaypoint->m_GUID);

  pWP->m_bIsolatedMark = true;  // This is an isolated mark

  cloneHyperlinkList(pWP, pwaypoint);

  pWP->m_MarkDescription = pwaypoint->m_MarkDescription;

  if (pwaypoint->m_CreateTime.IsValid())
    pWP->SetCreateTime(pwaypoint->m_CreateTime);
  else {
    pWP->SetCreateTime(wxDateTime::Now().ToUTC());
  }

  pWP->m_btemp = (b_permanent == false);

  pSelect->AddSelectableRoutePoint(pwaypoint->m_lat, pwaypoint->m_lon, pWP);
  if (b_permanent) {
    // pConfig->AddNewWayPoint(pWP, -1);
    NavObj_dB::GetInstance().InsertRoutePoint(pWP);
  }

  if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
    pRouteManagerDialog->UpdateWptListCtrl();

  return true;
}

bool DeleteSingleWaypoint(wxString& GUID) {
  //  Find the RoutePoint
  bool b_found = false;
  RoutePoint* prp = pWayPointMan->FindRoutePointByGUID(GUID);

  if (prp) b_found = true;

  if (b_found) {
    pWayPointMan->DestroyWaypoint(prp);
    if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
      pRouteManagerDialog->UpdateWptListCtrl();
  }

  return b_found;
}

bool UpdateSingleWaypoint(PlugIn_Waypoint* pwaypoint) {
  //  Find the RoutePoint
  bool b_found = false;
  RoutePoint* prp = pWayPointMan->FindRoutePointByGUID(pwaypoint->m_GUID);

  if (prp) b_found = true;

  if (b_found) {
    double lat_save = prp->m_lat;
    double lon_save = prp->m_lon;

    prp->m_lat = pwaypoint->m_lat;
    prp->m_lon = pwaypoint->m_lon;
    prp->SetIconName(pwaypoint->m_IconName);
    prp->SetName(pwaypoint->m_MarkName);
    prp->m_MarkDescription = pwaypoint->m_MarkDescription;
    prp->SetVisible(pwaypoint->m_IsVisible);
    if (pwaypoint->m_CreateTime.IsValid())
      prp->SetCreateTime(pwaypoint->m_CreateTime);

    //  Transcribe (clone) the html HyperLink List, if present

    if (pwaypoint->m_HyperlinkList) {
      prp->m_HyperlinkList->Clear();
      if (pwaypoint->m_HyperlinkList->GetCount() > 0) {
        wxPlugin_HyperlinkListNode* linknode =
            pwaypoint->m_HyperlinkList->GetFirst();
        while (linknode) {
          Plugin_Hyperlink* link = linknode->GetData();

          Hyperlink* h = new Hyperlink();
          h->DescrText = link->DescrText;
          h->Link = link->Link;
          h->LType = link->Type;

          prp->m_HyperlinkList->Append(h);

          linknode = linknode->GetNext();
        }
      }
    }

    if (prp) prp->ReLoadIcon();

    auto canvas = gFrame->GetPrimaryCanvas();
    SelectCtx ctx(canvas->m_bShowNavobjects, canvas->GetCanvasTrueScale(),
                  canvas->GetScaleValue());
    SelectItem* pFind =
        pSelect->FindSelection(ctx, lat_save, lon_save, SELTYPE_ROUTEPOINT);
    if (pFind) {
      pFind->m_slat = pwaypoint->m_lat;  // update the SelectList entry
      pFind->m_slon = pwaypoint->m_lon;
    }

    if (!prp->m_btemp) {
      // pConfig->UpdateWayPoint(prp);
      NavObj_dB::GetInstance().UpdateRoutePoint(prp);
    }

    if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
      pRouteManagerDialog->UpdateWptListCtrl();
  }

  return b_found;
}

// translate O route class to Plugin one
static void PlugInFromRoutePoint(PlugIn_Waypoint* dst,
                                 /* const*/ RoutePoint* src) {
  dst->m_lat = src->m_lat;
  dst->m_lon = src->m_lon;
  dst->m_IconName = src->GetIconName();
  dst->m_MarkName = src->GetName();
  dst->m_MarkDescription = src->m_MarkDescription;
  dst->m_IsVisible = src->IsVisible();
  dst->m_CreateTime = src->GetCreateTime();  // not const
  dst->m_GUID = src->m_GUID;

  //  Transcribe (clone) the html HyperLink List, if present
  if (src->m_HyperlinkList == nullptr) return;

  delete dst->m_HyperlinkList;
  dst->m_HyperlinkList = nullptr;

  if (src->m_HyperlinkList->GetCount() > 0) {
    dst->m_HyperlinkList = new Plugin_HyperlinkList;

    wxHyperlinkListNode* linknode = src->m_HyperlinkList->GetFirst();
    while (linknode) {
      Hyperlink* link = linknode->GetData();

      Plugin_Hyperlink* h = new Plugin_Hyperlink();
      h->DescrText = link->DescrText;
      h->Link = link->Link;
      h->Type = link->LType;

      dst->m_HyperlinkList->Append(h);

      linknode = linknode->GetNext();
    }
  }
}

bool GetSingleWaypoint(wxString GUID, PlugIn_Waypoint* pwaypoint) {
  //  Find the RoutePoint
  RoutePoint* prp = pWayPointMan->FindRoutePointByGUID(GUID);

  if (!prp) return false;

  PlugInFromRoutePoint(pwaypoint, prp);

  return true;
}

wxArrayString GetWaypointGUIDArray(void) {
  wxArrayString result;
  const RoutePointList* list = pWayPointMan->GetWaypointList();

  wxRoutePointListNode* prpnode = list->GetFirst();
  while (prpnode) {
    RoutePoint* prp = prpnode->GetData();
    result.Add(prp->m_GUID);

    prpnode = prpnode->GetNext();  // RoutePoint
  }

  return result;
}

wxArrayString GetRouteGUIDArray(void) {
  wxArrayString result;
  RouteList* list = pRouteList;

  wxRouteListNode* prpnode = list->GetFirst();
  while (prpnode) {
    Route* proute = prpnode->GetData();
    result.Add(proute->m_GUID);

    prpnode = prpnode->GetNext();  // Route
  }

  return result;
}

wxArrayString GetTrackGUIDArray(void) {
  wxArrayString result;
  for (Track* ptrack : g_TrackList) {
    result.Add(ptrack->m_GUID);
  }

  return result;
}

wxArrayString GetWaypointGUIDArray(OBJECT_LAYER_REQ req) {
  wxArrayString result;
  const RoutePointList* list = pWayPointMan->GetWaypointList();

  wxRoutePointListNode* prpnode = list->GetFirst();
  while (prpnode) {
    RoutePoint* prp = prpnode->GetData();
    switch (req) {
      case OBJECTS_ALL:
        result.Add(prp->m_GUID);
        break;
      case OBJECTS_NO_LAYERS:
        if (!prp->m_bIsInLayer) result.Add(prp->m_GUID);
        break;
      case OBJECTS_ONLY_LAYERS:
        if (prp->m_bIsInLayer) result.Add(prp->m_GUID);
        break;
    }

    prpnode = prpnode->GetNext();  // RoutePoint
  }

  return result;
}

wxArrayString GetRouteGUIDArray(OBJECT_LAYER_REQ req) {
  wxArrayString result;
  RouteList* list = pRouteList;

  wxRouteListNode* prpnode = list->GetFirst();
  while (prpnode) {
    Route* proute = prpnode->GetData();
    switch (req) {
      case OBJECTS_ALL:
        result.Add(proute->m_GUID);
        break;
      case OBJECTS_NO_LAYERS:
        if (!proute->m_bIsInLayer) result.Add(proute->m_GUID);
        break;
      case OBJECTS_ONLY_LAYERS:
        if (proute->m_bIsInLayer) result.Add(proute->m_GUID);
        break;
    }

    prpnode = prpnode->GetNext();  // Route
  }

  return result;
}

wxArrayString GetTrackGUIDArray(OBJECT_LAYER_REQ req) {
  wxArrayString result;
  for (Track* ptrack : g_TrackList) {
    switch (req) {
      case OBJECTS_ALL:
        result.Add(ptrack->m_GUID);
        break;
      case OBJECTS_NO_LAYERS:
        if (!ptrack->m_bIsInLayer) result.Add(ptrack->m_GUID);
        break;
      case OBJECTS_ONLY_LAYERS:
        if (ptrack->m_bIsInLayer) result.Add(ptrack->m_GUID);
        break;
    }
  }

  return result;
}

wxArrayString GetIconNameArray(void) {
  wxArrayString result;

  for (int i = 0; i < pWayPointMan->GetNumIcons(); i++) {
    wxString* ps = pWayPointMan->GetIconKey(i);
    result.Add(*ps);
  }
  return result;
}

bool AddPlugInRoute(PlugIn_Route* proute, bool b_permanent) {
  Route* route = new Route();

  PlugIn_Waypoint* pwp;
  RoutePoint* pWP_src;
  int ip = 0;
  wxDateTime plannedDeparture;

  wxPlugin_WaypointListNode* pwpnode = proute->pWaypointList->GetFirst();
  while (pwpnode) {
    pwp = pwpnode->GetData();

    RoutePoint* pWP = new RoutePoint(pwp->m_lat, pwp->m_lon, pwp->m_IconName,
                                     pwp->m_MarkName, pwp->m_GUID);

    //  Transcribe (clone) the html HyperLink List, if present
    cloneHyperlinkList(pWP, pwp);
    pWP->m_MarkDescription = pwp->m_MarkDescription;
    pWP->m_bShowName = false;
    pWP->SetCreateTime(pwp->m_CreateTime);

    route->AddPoint(pWP);

    pSelect->AddSelectableRoutePoint(pWP->m_lat, pWP->m_lon, pWP);

    if (ip > 0)
      pSelect->AddSelectableRouteSegment(pWP_src->m_lat, pWP_src->m_lon,
                                         pWP->m_lat, pWP->m_lon, pWP_src, pWP,
                                         route);
    else
      plannedDeparture = pwp->m_CreateTime;
    ip++;
    pWP_src = pWP;

    pwpnode = pwpnode->GetNext();  // PlugInWaypoint
  }

  route->m_PlannedDeparture = plannedDeparture;

  route->m_RouteNameString = proute->m_NameString;
  route->m_RouteStartString = proute->m_StartString;
  route->m_RouteEndString = proute->m_EndString;
  if (!proute->m_GUID.IsEmpty()) {
    route->m_GUID = proute->m_GUID;
  }
  route->m_btemp = (b_permanent == false);

  pRouteList->Append(route);

  if (b_permanent) {
    // pConfig->AddNewRoute(route);
    NavObj_dB::GetInstance().InsertRoute(route);
  }
  if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
    pRouteManagerDialog->UpdateRouteListCtrl();

  return true;
}

bool DeletePlugInRoute(wxString& GUID) {
  bool b_found = false;

  //  Find the Route
  Route* pRoute = g_pRouteMan->FindRouteByGUID(GUID);
  if (pRoute) {
    g_pRouteMan->DeleteRoute(pRoute);
    b_found = true;
  }
  return b_found;
}

bool UpdatePlugInRoute(PlugIn_Route* proute) {
  bool b_found = false;

  //  Find the Route
  Route* pRoute = g_pRouteMan->FindRouteByGUID(proute->m_GUID);
  if (pRoute) b_found = true;

  if (b_found) {
    bool b_permanent = (pRoute->m_btemp == false);
    g_pRouteMan->DeleteRoute(pRoute);
    b_found = AddPlugInRoute(proute, b_permanent);
  }

  return b_found;
}

bool AddPlugInTrack(PlugIn_Track* ptrack, bool b_permanent) {
  Track* track = new Track();

  PlugIn_Waypoint* pwp = 0;
  TrackPoint* pWP_src = 0;
  int ip = 0;

  wxPlugin_WaypointListNode* pwpnode = ptrack->pWaypointList->GetFirst();
  while (pwpnode) {
    pwp = pwpnode->GetData();

    TrackPoint* pWP = new TrackPoint(pwp->m_lat, pwp->m_lon);
    pWP->SetCreateTime(pwp->m_CreateTime);

    track->AddPoint(pWP);

    if (ip > 0)
      pSelect->AddSelectableTrackSegment(pWP_src->m_lat, pWP_src->m_lon,
                                         pWP->m_lat, pWP->m_lon, pWP_src, pWP,
                                         track);
    ip++;
    pWP_src = pWP;

    pwpnode = pwpnode->GetNext();  // PlugInWaypoint
  }

  track->SetName(ptrack->m_NameString);
  track->m_TrackStartString = ptrack->m_StartString;
  track->m_TrackEndString = ptrack->m_EndString;
  track->m_GUID = ptrack->m_GUID;
  track->m_btemp = (b_permanent == false);

  g_TrackList.push_back(track);
  if (b_permanent) NavObj_dB::GetInstance().InsertTrack(track);
  // if (b_permanent) pConfig->AddNewTrack(track);

  if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
    pRouteManagerDialog->UpdateTrkListCtrl();

  return true;
}

bool DeletePlugInTrack(wxString& GUID) {
  bool b_found = false;

  //  Find the Route
  Track* pTrack = g_pRouteMan->FindTrackByGUID(GUID);
  if (pTrack) {
    NavObj_dB::GetInstance().DeleteTrack(pTrack);
    RoutemanGui(*g_pRouteMan).DeleteTrack(pTrack);
    b_found = true;
  }

  if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
    pRouteManagerDialog->UpdateTrkListCtrl();

  return b_found;
}

bool UpdatePlugInTrack(PlugIn_Track* ptrack) {
  bool b_found = false;

  //  Find the Track
  Track* pTrack = g_pRouteMan->FindTrackByGUID(ptrack->m_GUID);
  if (pTrack) b_found = true;

  if (b_found) {
    bool b_permanent = (pTrack->m_btemp == false);
    NavObj_dB::GetInstance().DeleteTrack(pTrack);
    RoutemanGui(*g_pRouteMan).DeleteTrack(pTrack);

    b_found = AddPlugInTrack(ptrack, b_permanent);
  }

  return b_found;
}

bool PlugInHasNormalizedViewPort(PlugIn_ViewPort* vp) {
#ifdef ocpnUSE_GL
  ViewPort ocpn_vp;
  ocpn_vp.m_projection_type = vp->m_projection_type;

  return glChartCanvas::HasNormalizedViewPort(ocpn_vp);
#else
  return false;
#endif
}

void PlugInMultMatrixViewport(PlugIn_ViewPort* vp, float lat, float lon) {
#ifdef ocpnUSE_GL
  ViewPort ocpn_vp;
  ocpn_vp.clat = vp->clat;
  ocpn_vp.clon = vp->clon;
  ocpn_vp.m_projection_type = vp->m_projection_type;
  ocpn_vp.view_scale_ppm = vp->view_scale_ppm;
  ocpn_vp.skew = vp->skew;
  ocpn_vp.rotation = vp->rotation;
  ocpn_vp.pix_width = vp->pix_width;
  ocpn_vp.pix_height = vp->pix_height;

// TODO fix for multicanvas    glChartCanvas::MultMatrixViewPort(ocpn_vp, lat,
// lon);
#endif
}

void PlugInNormalizeViewport(PlugIn_ViewPort* vp, float lat, float lon) {
#ifdef ocpnUSE_GL
  ViewPort ocpn_vp;
  glChartCanvas::NormalizedViewPort(ocpn_vp, lat, lon);

  vp->clat = ocpn_vp.clat;
  vp->clon = ocpn_vp.clon;
  vp->view_scale_ppm = ocpn_vp.view_scale_ppm;
  vp->rotation = ocpn_vp.rotation;
  vp->skew = ocpn_vp.skew;
#endif
}

//          Helper and interface classes

//-------------------------------------------------------------------------------
//    PlugIn_AIS_Target Implementation
//-------------------------------------------------------------------------------

PlugIn_AIS_Target* Create_PI_AIS_Target(AisTargetData* ptarget) {
  PlugIn_AIS_Target* pret = new PlugIn_AIS_Target;

  pret->MMSI = ptarget->MMSI;
  pret->Class = ptarget->Class;
  pret->NavStatus = ptarget->NavStatus;
  pret->SOG = ptarget->SOG;
  pret->COG = ptarget->COG;
  pret->HDG = ptarget->HDG;
  pret->Lon = ptarget->Lon;
  pret->Lat = ptarget->Lat;
  pret->ROTAIS = ptarget->ROTAIS;
  pret->ShipType = ptarget->ShipType;
  pret->IMO = ptarget->IMO;

  pret->Range_NM = ptarget->Range_NM;
  pret->Brg = ptarget->Brg;

  //      Per target collision parameters
  pret->bCPA_Valid = ptarget->bCPA_Valid;
  pret->TCPA = ptarget->TCPA;  // Minutes
  pret->CPA = ptarget->CPA;    // Nautical Miles

  pret->alarm_state = (plugin_ais_alarm_type)ptarget->n_alert_state;

  memcpy(pret->CallSign, ptarget->CallSign, sizeof(ptarget->CallSign) - 1);
  memcpy(pret->ShipName, ptarget->ShipName, sizeof(ptarget->ShipName) - 1);

  return pret;
}

//---------------------------------------------------------------------------
//    API 1.11
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//    API 1.12
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//    API 1.13
//---------------------------------------------------------------------------
double fromDMM_Plugin(wxString sdms) { return fromDMM(sdms); }

void SetCanvasRotation(double rotation) {
  gFrame->GetPrimaryCanvas()->DoRotateCanvas(rotation);
}

double GetCanvasTilt() { return gFrame->GetPrimaryCanvas()->GetVPTilt(); }

void SetCanvasTilt(double tilt) {
  gFrame->GetPrimaryCanvas()->DoTiltCanvas(tilt);
}

void SetCanvasProjection(int projection) {
  gFrame->GetPrimaryCanvas()->SetVPProjection(projection);
}

OcpnSound* g_PluginSound = SoundFactory();
static void onPlugInPlaySoundExFinished(void* ptr) {}

// Start playing a sound to a given device and return status to plugin
bool PlugInPlaySoundEx(wxString& sound_file, int deviceIndex) {
  bool ok = g_PluginSound->Load(sound_file, deviceIndex);
  if (!ok) {
    wxLogWarning("Cannot load sound file: %s", sound_file);
    return false;
  }
  auto cmd_sound = dynamic_cast<SystemCmdSound*>(g_PluginSound);
  if (cmd_sound) cmd_sound->SetCmd(g_CmdSoundString.mb_str(wxConvUTF8));

  g_PluginSound->SetFinishedCallback(onPlugInPlaySoundExFinished, NULL);
  ok = g_PluginSound->Play();
  if (!ok) {
    wxLogWarning("Cannot play sound file: %s", sound_file);
  }
  return ok;
}

bool CheckEdgePan_PlugIn(int x, int y, bool dragging, int margin, int delta) {
  return gFrame->GetPrimaryCanvas()->CheckEdgePan(x, y, dragging, margin,
                                                  delta);
}

wxBitmap GetIcon_PlugIn(const wxString& name) {
  ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();
  return style->GetIcon(name);
}

void SetCursor_PlugIn(wxCursor* pCursor) {
  gFrame->GetPrimaryCanvas()->pPlugIn_Cursor = pCursor;
}

void AddChartDirectory(wxString& path) {
  if (g_options) {
    g_options->AddChartDir(path);
  }
}

void ForceChartDBUpdate() {
  if (g_options) {
    g_options->pScanCheckBox->SetValue(true);
    g_options->pUpdateCheckBox->SetValue(true);
  }
}

void ForceChartDBRebuild() {
  if (g_options) {
    g_options->pUpdateCheckBox->SetValue(true);
  }
}

wxDialog* GetActiveOptionsDialog() { return g_options; }

int PlatformDirSelectorDialog(wxWindow* parent, wxString* file_spec,
                              wxString Title, wxString initDir) {
  return g_Platform->DoDirSelectorDialog(parent, file_spec, Title, initDir);
}

int PlatformFileSelectorDialog(wxWindow* parent, wxString* file_spec,
                               wxString Title, wxString initDir,
                               wxString suggestedName, wxString wildcard) {
  return g_Platform->DoFileSelectorDialog(parent, file_spec, Title, initDir,
                                          suggestedName, wildcard);
}

//---------------------------------------------------------------------------
//    API 1.14
//---------------------------------------------------------------------------

ViewPort CreateCompatibleViewportEx(const PlugIn_ViewPort& pivp) {
  //    Create a system ViewPort
  ViewPort vp;

  vp.clat = pivp.clat;  // center point
  vp.clon = pivp.clon;
  vp.view_scale_ppm = pivp.view_scale_ppm;
  vp.skew = pivp.skew;
  vp.rotation = pivp.rotation;
  vp.chart_scale = pivp.chart_scale;
  vp.pix_width = pivp.pix_width;
  vp.pix_height = pivp.pix_height;
  vp.rv_rect = pivp.rv_rect;
  vp.b_quilt = pivp.b_quilt;
  vp.m_projection_type = pivp.m_projection_type;

  if (gFrame->GetPrimaryCanvas())
    vp.ref_scale = gFrame->GetPrimaryCanvas()->GetVP().ref_scale;
  else
    vp.ref_scale = vp.chart_scale;

  vp.SetBoxes();
  vp.Validate();  // This VP is valid

  return vp;
}

void PlugInAISDrawGL(wxGLCanvas* glcanvas, const PlugIn_ViewPort& vp) {
  ViewPort ocpn_vp = CreateCompatibleViewportEx(vp);

  ocpnDC dc(*glcanvas);
  dc.SetVP(ocpn_vp);

  AISDraw(dc, ocpn_vp, NULL);
}

bool PlugInSetFontColor(const wxString TextElement, const wxColour color) {
  return FontMgr::Get().SetFontColor(TextElement, color);
}

//---------------------------------------------------------------------------
//    API 1.15
//---------------------------------------------------------------------------

double PlugInGetDisplaySizeMM() { return g_Platform->GetDisplaySizeMM(); }

wxFont* FindOrCreateFont_PlugIn(int point_size, wxFontFamily family,
                                wxFontStyle style, wxFontWeight weight,
                                bool underline, const wxString& facename,
                                wxFontEncoding encoding) {
  return FontMgr::Get().FindOrCreateFont(point_size, family, style, weight,
                                         underline, facename, encoding);
}

int PluginGetMinAvailableGshhgQuality() {
  return gFrame->GetPrimaryCanvas()->GetMinAvailableGshhgQuality();
}
int PluginGetMaxAvailableGshhgQuality() {
  return gFrame->GetPrimaryCanvas()->GetMaxAvailableGshhgQuality();
}

// disable builtin console canvas, and autopilot nmea sentences
void PlugInHandleAutopilotRoute(bool enable) {
  g_bPluginHandleAutopilotRoute = enable;
}

bool LaunchDefaultBrowser_Plugin(wxString url) {
  if (g_Platform) g_Platform->platformLaunchDefaultBrowser(url);

  return true;
}

//---------------------------------------------------------------------------
//    API 1.16
//---------------------------------------------------------------------------
wxString GetSelectedWaypointGUID_Plugin() {
  ChartCanvas* cc = gFrame->GetFocusCanvas();
  if (cc && cc->GetSelectedRoutePoint()) {
    return cc->GetSelectedRoutePoint()->m_GUID;
  }
  return wxEmptyString;
}

wxString GetSelectedRouteGUID_Plugin() {
  ChartCanvas* cc = gFrame->GetFocusCanvas();
  if (cc && cc->GetSelectedRoute()) {
    return cc->GetSelectedRoute()->m_GUID;
  }
  return wxEmptyString;
}

wxString GetSelectedTrackGUID_Plugin() {
  ChartCanvas* cc = gFrame->GetFocusCanvas();
  if (cc && cc->GetSelectedTrack()) {
    return cc->GetSelectedTrack()->m_GUID;
  }
  return wxEmptyString;
}

std::unique_ptr<PlugIn_Waypoint> GetWaypoint_Plugin(const wxString& GUID) {
  std::unique_ptr<PlugIn_Waypoint> w(new PlugIn_Waypoint);
  GetSingleWaypoint(GUID, w.get());
  return w;
}

std::unique_ptr<PlugIn_Route> GetRoute_Plugin(const wxString& GUID) {
  std::unique_ptr<PlugIn_Route> r;
  Route* route = g_pRouteMan->FindRouteByGUID(GUID);
  if (route == nullptr) return r;

  r = std::unique_ptr<PlugIn_Route>(new PlugIn_Route);
  PlugIn_Route* dst_route = r.get();

  // PlugIn_Waypoint *pwp;
  RoutePoint* src_wp;
  wxRoutePointListNode* node = route->pRoutePointList->GetFirst();

  while (node) {
    src_wp = node->GetData();

    PlugIn_Waypoint* dst_wp = new PlugIn_Waypoint();
    PlugInFromRoutePoint(dst_wp, src_wp);

    dst_route->pWaypointList->Append(dst_wp);

    node = node->GetNext();
  }
  dst_route->m_NameString = route->m_RouteNameString;
  dst_route->m_StartString = route->m_RouteStartString;
  dst_route->m_EndString = route->m_RouteEndString;
  dst_route->m_GUID = route->m_GUID;

  return r;
}

std::unique_ptr<PlugIn_Track> GetTrack_Plugin(const wxString& GUID) {
  std::unique_ptr<PlugIn_Track> t;
  //  Find the Track
  Track* pTrack = g_pRouteMan->FindTrackByGUID(GUID);
  if (!pTrack) return t;

  std::unique_ptr<PlugIn_Track> tk =
      std::unique_ptr<PlugIn_Track>(new PlugIn_Track);
  PlugIn_Track* dst_track = tk.get();
  dst_track->m_NameString = pTrack->GetName();
  dst_track->m_StartString = pTrack->m_TrackStartString;
  dst_track->m_EndString = pTrack->m_TrackEndString;
  dst_track->m_GUID = pTrack->m_GUID;

  for (int i = 0; i < pTrack->GetnPoints(); i++) {
    TrackPoint* ptp = pTrack->GetPoint(i);

    PlugIn_Waypoint* dst_wp = new PlugIn_Waypoint();

    dst_wp->m_lat = ptp->m_lat;
    dst_wp->m_lon = ptp->m_lon;
    dst_wp->m_CreateTime = ptp->GetCreateTime();  // not const

    dst_track->pWaypointList->Append(dst_wp);
  }

  return tk;
}

wxWindow* PluginGetFocusCanvas() { return g_focusCanvas; }

wxWindow* PluginGetOverlayRenderCanvas() {
  // if(g_overlayCanvas)
  return g_overlayCanvas;
  // else
}

void CanvasJumpToPosition(wxWindow* canvas, double lat, double lon,
                          double scale) {
  auto oCanvas = dynamic_cast<ChartCanvas*>(canvas);
  if (oCanvas) gFrame->JumpToPosition(oCanvas, lat, lon, scale);
}

bool ShuttingDown(void) { return g_bquiting; }

wxWindow* GetCanvasUnderMouse(void) { return gFrame->GetCanvasUnderMouse(); }

int GetCanvasIndexUnderMouse(void) {
  ChartCanvas* l_canvas = gFrame->GetCanvasUnderMouse();
  if (l_canvas) {
    for (unsigned int i = 0; i < g_canvasArray.GetCount(); ++i) {
      if (l_canvas == g_canvasArray[i]) return i;
    }
  }
  return 0;
}

// std::vector<wxWindow *> GetCanvasArray()
// {
//     std::vector<wxWindow *> rv;
//     for(unsigned int i=0 ; i < g_canvasArray.GetCount() ; i++){
//         ChartCanvas *cc = g_canvasArray.Item(i);
//         rv.push_back(cc);
//     }
//
//     return rv;
// }

wxWindow* GetCanvasByIndex(int canvasIndex) {
  if (g_canvasConfig == 0)
    return gFrame->GetPrimaryCanvas();
  else {
    if ((canvasIndex >= 0) && g_canvasArray[canvasIndex]) {
      return g_canvasArray[canvasIndex];
    }
  }
  return NULL;
}

bool CheckMUIEdgePan_PlugIn(int x, int y, bool dragging, int margin, int delta,
                            int canvasIndex) {
  if (g_canvasConfig == 0)
    return gFrame->GetPrimaryCanvas()->CheckEdgePan(x, y, dragging, margin,
                                                    delta);
  else {
    if ((canvasIndex >= 0) && g_canvasArray[canvasIndex]) {
      return g_canvasArray[canvasIndex]->CheckEdgePan(x, y, dragging, margin,
                                                      delta);
    }
  }

  return false;
}

void SetMUICursor_PlugIn(wxCursor* pCursor, int canvasIndex) {
  if (g_canvasConfig == 0)
    gFrame->GetPrimaryCanvas()->pPlugIn_Cursor = pCursor;
  else {
    if ((canvasIndex >= 0) && g_canvasArray[canvasIndex]) {
      g_canvasArray[canvasIndex]->pPlugIn_Cursor = pCursor;
    }
  }
}

int GetCanvasCount() {
  if (g_canvasConfig == 1) return 2;
  //     else
  return 1;
}

int GetLatLonFormat() { return g_iSDMMFormat; }

wxRect GetMasterToolbarRect() {
  if (g_MainToolbar)
    return g_MainToolbar->GetToolbarRect();
  else
    return wxRect(0, 0, 1, 1);
}

//---------------------------------------------------------------------------
//    API 1.17
//---------------------------------------------------------------------------

void ZeroXTE() {
  if (g_pRouteMan) {
    g_pRouteMan->ZeroCurrentXTEToActivePoint();
  }
}

static PlugIn_ViewPort CreatePlugInViewportEx(const ViewPort& vp) {
  //    Create a PlugIn Viewport
  ViewPort tvp = vp;
  PlugIn_ViewPort pivp;

  pivp.clat = tvp.clat;  // center point
  pivp.clon = tvp.clon;
  pivp.view_scale_ppm = tvp.view_scale_ppm;
  pivp.skew = tvp.skew;
  pivp.rotation = tvp.rotation;
  pivp.chart_scale = tvp.chart_scale;
  pivp.pix_width = tvp.pix_width;
  pivp.pix_height = tvp.pix_height;
  pivp.rv_rect = tvp.rv_rect;
  pivp.b_quilt = tvp.b_quilt;
  pivp.m_projection_type = tvp.m_projection_type;

  pivp.lat_min = tvp.GetBBox().GetMinLat();
  pivp.lat_max = tvp.GetBBox().GetMaxLat();
  pivp.lon_min = tvp.GetBBox().GetMinLon();
  pivp.lon_max = tvp.GetBBox().GetMaxLon();

  pivp.bValid = tvp.IsValid();  // This VP is valid

  return pivp;
}

ListOfPI_S57Obj* PlugInManager::GetLightsObjRuleListVisibleAtLatLon(
    ChartPlugInWrapper* target, float zlat, float zlon, const ViewPort& vp) {
  ListOfPI_S57Obj* list = NULL;
  if (target) {
    PlugInChartBaseGLPlus2* picbgl =
        dynamic_cast<PlugInChartBaseGLPlus2*>(target->GetPlugInChart());
    if (picbgl) {
      PlugIn_ViewPort pi_vp = CreatePlugInViewportEx(vp);
      list = picbgl->GetLightsObjRuleListVisibleAtLatLon(zlat, zlon, &pi_vp);

      return list;
    }
    PlugInChartBaseExtendedPlus2* picbx =
        dynamic_cast<PlugInChartBaseExtendedPlus2*>(target->GetPlugInChart());
    if (picbx) {
      PlugIn_ViewPort pi_vp = CreatePlugInViewportEx(vp);
      list = picbx->GetLightsObjRuleListVisibleAtLatLon(zlat, zlon, &pi_vp);

      return list;
    } else
      return list;
  } else
    return list;
}

//      PlugInWaypointEx implementation

#include <wx/listimpl.cpp>
WX_DEFINE_LIST(Plugin_WaypointExList)

//  The class implementations
PlugIn_Waypoint_Ex::PlugIn_Waypoint_Ex() { InitDefaults(); }

PlugIn_Waypoint_Ex::PlugIn_Waypoint_Ex(
    double lat, double lon, const wxString& icon_ident, const wxString& wp_name,
    const wxString& GUID, const double ScaMin, const bool bNameVisible,
    const int nRangeRings, const double RangeDistance,
    const wxColor RangeColor) {
  InitDefaults();

  m_lat = lat;
  m_lon = lon;
  IconName = icon_ident;
  m_MarkName = wp_name;
  m_GUID = GUID;
  scamin = ScaMin;
  IsNameVisible = bNameVisible;
  nrange_rings = nRangeRings;
  RangeRingSpace = RangeDistance;
  RangeRingColor = RangeColor;
}

void PlugIn_Waypoint_Ex::InitDefaults() {
  m_HyperlinkList = nullptr;
  scamin = 1e9;
  b_useScamin = false;
  nrange_rings = 0;
  RangeRingSpace = 1;
  IsNameVisible = false;
  IsVisible = true;
  RangeRingColor = *wxBLACK;
  m_CreateTime = wxDateTime::Now().ToUTC();
  IsActive = false;
  m_lat = 0;
  m_lon = 0;
}

bool PlugIn_Waypoint_Ex::GetFSStatus() {
  RoutePoint* prp = pWayPointMan->FindRoutePointByGUID(m_GUID);
  if (!prp) return false;

  if (prp->m_bIsInRoute && !prp->IsShared()) return false;

  return true;
}

int PlugIn_Waypoint_Ex::GetRouteMembershipCount() {
  // Search all routes to count the membership of this point
  RoutePoint* pWP = pWayPointMan->FindRoutePointByGUID(m_GUID);
  if (!pWP) return 0;

  int nCount = 0;
  wxRouteListNode* node = pRouteList->GetFirst();
  while (node) {
    Route* proute = node->GetData();
    wxRoutePointListNode* pnode = (proute->pRoutePointList)->GetFirst();
    while (pnode) {
      RoutePoint* prp = pnode->GetData();
      if (prp == pWP) nCount++;
      pnode = pnode->GetNext();
    }

    node = node->GetNext();
  }

  return nCount;
}

PlugIn_Waypoint_Ex::~PlugIn_Waypoint_Ex() {}

WX_DEFINE_LIST(Plugin_WaypointExV2List)

PlugIn_Waypoint_ExV2::PlugIn_Waypoint_ExV2() { InitDefaults(); }

PlugIn_Waypoint_ExV2::PlugIn_Waypoint_ExV2(
    double lat, double lon, const wxString& icon_ident, const wxString& wp_name,
    const wxString& GUID, const double ScaMin, const double ScaMax,
    const bool bNameVisible, const int nRangeRings, const double RangeDistance,
    const int RangeDistanceUnits, const wxColor RangeColor,
    const double WaypointArrivalRadius, const bool ShowWaypointRangeRings,
    const double PlannedSpeed, const wxString TideStation) {
  // Initialize all to defaults first
  InitDefaults();
  // Then set the specific values provided
  m_lat = lat;
  m_lon = lon;
  IconName = icon_ident;
  m_MarkName = wp_name;
  m_GUID = GUID;
  scamin = ScaMin;
  scamax = ScaMax;

  IsNameVisible = bNameVisible;
  nrange_rings = nRangeRings;
  RangeRingSpace = RangeDistance;
  RangeRingSpaceUnits = RangeDistanceUnits;  // 0 = nm, 1 = km
  RangeRingColor = RangeColor;
  m_TideStation = TideStation;

  m_PlannedSpeed = PlannedSpeed;
  m_WaypointArrivalRadius = WaypointArrivalRadius;
  m_bShowWaypointRangeRings = ShowWaypointRangeRings;
}

void PlugIn_Waypoint_ExV2::InitDefaults() {
  m_HyperlinkList = nullptr;
  scamin = 1e9;
  scamax = 1e6;
  b_useScamin = false;
  nrange_rings = 0;
  RangeRingSpace = 1;
  RangeRingSpaceUnits = 0;  // 0 = nm, 1 = km
  m_TideStation = wxEmptyString;
  IsNameVisible = false;
  IsVisible = true;
  RangeRingColor = *wxBLACK;
  m_CreateTime = wxDateTime::Now().ToUTC();
  IsActive = false;
  m_lat = 0;
  m_lon = 0;

  m_PlannedSpeed = 0.0;
  m_WaypointArrivalRadius = 0.0;
  m_bShowWaypointRangeRings = false;
}

PlugIn_Waypoint_ExV2::~PlugIn_Waypoint_ExV2() {}

bool PlugIn_Waypoint_ExV2::GetFSStatus() {
  RoutePoint* prp = pWayPointMan->FindRoutePointByGUID(m_GUID);
  if (!prp) return false;
  if (prp->m_bIsInRoute && !prp->IsShared()) return false;
  return true;
}

int PlugIn_Waypoint_ExV2::GetRouteMembershipCount() {
  // Search all routes to count the membership of this point
  RoutePoint* pWP = pWayPointMan->FindRoutePointByGUID(m_GUID);
  if (!pWP) return 0;

  int nCount = 0;
  wxRouteListNode* node = pRouteList->GetFirst();
  while (node) {
    Route* proute = node->GetData();
    wxRoutePointListNode* pnode = (proute->pRoutePointList)->GetFirst();
    while (pnode) {
      RoutePoint* prp = pnode->GetData();
      if (prp == pWP) nCount++;
      pnode = pnode->GetNext();
    }

    node = node->GetNext();
  }

  return nCount;
}

PlugIn_Route_ExV2::PlugIn_Route_ExV2() {
  pWaypointList = new Plugin_WaypointExV2List;
  m_GUID = wxEmptyString;
  m_NameString = wxEmptyString;
  m_StartString = wxEmptyString;
  m_EndString = wxEmptyString;
  m_isActive = false;
  m_isVisible = true;
  m_Description = wxEmptyString;

  // Generate a unique GUID if none provided
  if (m_GUID.IsEmpty()) {
    wxDateTime now = wxDateTime::Now();
    m_GUID = wxString::Format("RT%d%d%d%d", (int)now.GetMillisecond(),
                              (int)now.GetSecond(), (int)now.GetMinute(),
                              (int)now.GetHour());
  }
}

PlugIn_Route_ExV2::~PlugIn_Route_ExV2() {
  if (pWaypointList) {
    pWaypointList->DeleteContents(true);
    delete pWaypointList;
  }
}

// translate O route class to PlugIn_Waypoint_ExV2
static void PlugInExV2FromRoutePoint(PlugIn_Waypoint_ExV2* dst,
                                     /* const*/ RoutePoint* src) {
  dst->m_lat = src->m_lat;
  dst->m_lon = src->m_lon;
  dst->IconName = src->GetIconName();
  dst->m_MarkName = src->GetName();
  dst->m_MarkDescription = src->GetDescription();
  dst->IconDescription = pWayPointMan->GetIconDescription(src->GetIconName());
  dst->IsVisible = src->IsVisible();
  dst->m_CreateTime = src->GetCreateTime();  // not const
  dst->m_GUID = src->m_GUID;

  //  Transcribe (clone) the html HyperLink List, if present
  if (src->m_HyperlinkList) {
    delete dst->m_HyperlinkList;
    dst->m_HyperlinkList = nullptr;

    if (src->m_HyperlinkList->GetCount() > 0) {
      dst->m_HyperlinkList = new Plugin_HyperlinkList;

      wxHyperlinkListNode* linknode = src->m_HyperlinkList->GetFirst();
      while (linknode) {
        Hyperlink* link = linknode->GetData();

        Plugin_Hyperlink* h = new Plugin_Hyperlink();
        h->DescrText = link->DescrText;
        h->Link = link->Link;
        h->Type = link->LType;

        dst->m_HyperlinkList->Append(h);

        linknode = linknode->GetNext();
      }
    }
  }

  // Get the range ring info
  dst->nrange_rings = src->m_iWaypointRangeRingsNumber;
  dst->RangeRingSpace = src->m_fWaypointRangeRingsStep;
  dst->RangeRingSpaceUnits = src->m_iWaypointRangeRingsStepUnits;
  dst->RangeRingColor = src->m_wxcWaypointRangeRingsColour;
  dst->m_TideStation = src->m_TideStation;

  // Get other extended info
  dst->IsNameVisible = src->m_bShowName;
  dst->scamin = src->GetScaMin();
  dst->b_useScamin = src->GetUseSca();
  dst->IsActive = src->m_bIsActive;

  dst->scamax = src->GetScaMax();
  dst->m_PlannedSpeed = src->GetPlannedSpeed();
  dst->m_ETD = src->GetManualETD();
  dst->m_WaypointArrivalRadius = src->GetWaypointArrivalRadius();
  dst->m_bShowWaypointRangeRings = src->GetShowWaypointRangeRings();
}

bool GetSingleWaypointExV2(wxString GUID, PlugIn_Waypoint_ExV2* pwaypoint) {
  //  Find the RoutePoint
  RoutePoint* prp = pWayPointMan->FindRoutePointByGUID(GUID);

  if (!prp) return false;

  PlugInExV2FromRoutePoint(pwaypoint, prp);

  return true;
}

static void cloneHyperlinkListExV2(RoutePoint* dst,
                                   const PlugIn_Waypoint_ExV2* src) {
  //  Transcribe (clone) the html HyperLink List, if present
  if (src->m_HyperlinkList == nullptr) return;

  if (src->m_HyperlinkList->GetCount() > 0) {
    wxPlugin_HyperlinkListNode* linknode = src->m_HyperlinkList->GetFirst();
    while (linknode) {
      Plugin_Hyperlink* link = linknode->GetData();

      Hyperlink* h = new Hyperlink();
      h->DescrText = link->DescrText;
      h->Link = link->Link;
      h->LType = link->Type;

      dst->m_HyperlinkList->Append(h);

      linknode = linknode->GetNext();
    }
  }
}

RoutePoint* CreateNewPoint(const PlugIn_Waypoint_ExV2* src, bool b_permanent) {
  RoutePoint* pWP = new RoutePoint(src->m_lat, src->m_lon, src->IconName,
                                   src->m_MarkName, src->m_GUID);

  pWP->m_bIsolatedMark = true;  // This is an isolated mark

  cloneHyperlinkListExV2(pWP, src);

  pWP->m_MarkDescription = src->m_MarkDescription;

  if (src->m_CreateTime.IsValid())
    pWP->SetCreateTime(src->m_CreateTime);
  else {
    pWP->SetCreateTime(wxDateTime::Now().ToUTC());
  }

  pWP->m_btemp = (b_permanent == false);

  // Extended fields
  pWP->SetIconName(src->IconName);
  pWP->SetWaypointRangeRingsNumber(src->nrange_rings);
  pWP->SetWaypointRangeRingsStep(src->RangeRingSpace);
  pWP->SetWaypointRangeRingsStepUnits(src->RangeRingSpaceUnits);
  pWP->SetWaypointRangeRingsColour(src->RangeRingColor);
  pWP->SetTideStation(src->m_TideStation);
  pWP->SetScaMin(src->scamin);
  pWP->SetUseSca(src->b_useScamin);
  pWP->SetNameShown(src->IsNameVisible);
  pWP->SetVisible(src->IsVisible);

  pWP->SetWaypointArrivalRadius(src->m_WaypointArrivalRadius);
  pWP->SetShowWaypointRangeRings(src->m_bShowWaypointRangeRings);
  pWP->SetScaMax(src->scamax);
  pWP->SetPlannedSpeed(src->m_PlannedSpeed);
  if (src->m_ETD.IsValid())
    pWP->SetETD(src->m_ETD);
  else
    pWP->SetETD(wxEmptyString);
  return pWP;
}

bool AddSingleWaypointExV2(PlugIn_Waypoint_ExV2* pwaypointex,
                           bool b_permanent) {
  //  Validate the waypoint parameters a little bit

  //  GUID
  //  Make sure that this GUID is indeed unique in the Routepoint list
  bool b_unique = true;
  wxRoutePointListNode* prpnode = pWayPointMan->GetWaypointList()->GetFirst();
  while (prpnode) {
    RoutePoint* prp = prpnode->GetData();

    if (prp->m_GUID == pwaypointex->m_GUID) {
      b_unique = false;
      break;
    }
    prpnode = prpnode->GetNext();  // RoutePoint
  }

  if (!b_unique) return false;

  RoutePoint* pWP = CreateNewPoint(pwaypointex, b_permanent);

  pWP->SetShowWaypointRangeRings(pwaypointex->nrange_rings > 0);

  pSelect->AddSelectableRoutePoint(pWP->m_lat, pWP->m_lon, pWP);
  if (b_permanent) {
    // pConfig->AddNewWayPoint(pWP, -1);
    NavObj_dB::GetInstance().InsertRoutePoint(pWP);
  }

  if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
    pRouteManagerDialog->UpdateWptListCtrl();

  return true;
}

bool UpdateSingleWaypointExV2(PlugIn_Waypoint_ExV2* pwaypoint) {
  //  Find the RoutePoint
  bool b_found = false;
  RoutePoint* prp = pWayPointMan->FindRoutePointByGUID(pwaypoint->m_GUID);

  if (prp) b_found = true;

  if (b_found) {
    double lat_save = prp->m_lat;
    double lon_save = prp->m_lon;

    prp->m_lat = pwaypoint->m_lat;
    prp->m_lon = pwaypoint->m_lon;
    prp->SetIconName(pwaypoint->IconName);
    prp->SetName(pwaypoint->m_MarkName);
    prp->m_MarkDescription = pwaypoint->m_MarkDescription;
    prp->SetVisible(pwaypoint->IsVisible);
    if (pwaypoint->m_CreateTime.IsValid())
      prp->SetCreateTime(pwaypoint->m_CreateTime);

    //  Transcribe (clone) the html HyperLink List, if present

    if (pwaypoint->m_HyperlinkList) {
      prp->m_HyperlinkList->Clear();
      if (pwaypoint->m_HyperlinkList->GetCount() > 0) {
        wxPlugin_HyperlinkListNode* linknode =
            pwaypoint->m_HyperlinkList->GetFirst();
        while (linknode) {
          Plugin_Hyperlink* link = linknode->GetData();

          Hyperlink* h = new Hyperlink();
          h->DescrText = link->DescrText;
          h->Link = link->Link;
          h->LType = link->Type;

          prp->m_HyperlinkList->Append(h);

          linknode = linknode->GetNext();
        }
      }
    }

    // Extended fields
    prp->SetWaypointRangeRingsNumber(pwaypoint->nrange_rings);
    prp->SetWaypointRangeRingsStep(pwaypoint->RangeRingSpace);
    prp->SetWaypointRangeRingsStepUnits(pwaypoint->RangeRingSpaceUnits);
    prp->SetWaypointRangeRingsColour(pwaypoint->RangeRingColor);
    prp->SetTideStation(pwaypoint->m_TideStation);
    prp->SetScaMin(pwaypoint->scamin);
    prp->SetUseSca(pwaypoint->b_useScamin);
    prp->SetNameShown(pwaypoint->IsNameVisible);

    prp->SetShowWaypointRangeRings(pwaypoint->nrange_rings > 0);

    if (prp) prp->ReLoadIcon();

    auto canvas = gFrame->GetPrimaryCanvas();
    SelectCtx ctx(canvas->m_bShowNavobjects, canvas->GetCanvasTrueScale(),
                  canvas->GetScaleValue());
    SelectItem* pFind =
        pSelect->FindSelection(ctx, lat_save, lon_save, SELTYPE_ROUTEPOINT);
    if (pFind) {
      pFind->m_slat = pwaypoint->m_lat;  // update the SelectList entry
      pFind->m_slon = pwaypoint->m_lon;
    }

    if (!prp->m_btemp) {
      // pConfig->UpdateWayPoint(prp);
      NavObj_dB::GetInstance().UpdateRoutePoint(prp);
    }

    if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
      pRouteManagerDialog->UpdateWptListCtrl();

    prp->SetPlannedSpeed(pwaypoint->m_PlannedSpeed);
    if (pwaypoint->m_ETD.IsValid())
      prp->SetETD(pwaypoint->m_ETD);
    else
      prp->SetETD(wxEmptyString);
    prp->SetWaypointArrivalRadius(pwaypoint->m_WaypointArrivalRadius);
    prp->SetShowWaypointRangeRings(pwaypoint->m_bShowWaypointRangeRings);
    prp->SetScaMax(pwaypoint->scamax);
  }

  return b_found;
}

std::unique_ptr<PlugIn_Waypoint_ExV2> GetWaypointExV2_Plugin(
    const wxString& GUID) {
  std::unique_ptr<PlugIn_Waypoint_ExV2> w(new PlugIn_Waypoint_ExV2);
  GetSingleWaypointExV2(GUID, w.get());
  return w;
}

bool AddPlugInRouteExV2(PlugIn_Route_ExV2* proute, bool b_permanent) {
  Route* route = new Route();

  PlugIn_Waypoint_ExV2* pwaypointex;
  RoutePoint *pWP, *pWP_src;
  int ip = 0;
  wxDateTime plannedDeparture;

  wxPlugin_WaypointExV2ListNode* pwpnode = proute->pWaypointList->GetFirst();
  while (pwpnode) {
    pwaypointex = pwpnode->GetData();

    pWP = pWayPointMan->FindRoutePointByGUID(pwaypointex->m_GUID);
    if (!pWP) {
      pWP = CreateNewPoint(pwaypointex, b_permanent);
      pWP->m_bIsolatedMark = false;
    }

    route->AddPoint(pWP);

    pSelect->AddSelectableRoutePoint(pWP->m_lat, pWP->m_lon, pWP);

    if (ip > 0)
      pSelect->AddSelectableRouteSegment(pWP_src->m_lat, pWP_src->m_lon,
                                         pWP->m_lat, pWP->m_lon, pWP_src, pWP,
                                         route);

    plannedDeparture = pwaypointex->m_CreateTime;
    ip++;
    pWP_src = pWP;

    pwpnode = pwpnode->GetNext();  // PlugInWaypoint
  }

  route->m_PlannedDeparture = plannedDeparture;

  route->m_RouteNameString = proute->m_NameString;
  route->m_RouteStartString = proute->m_StartString;
  route->m_RouteEndString = proute->m_EndString;
  if (!proute->m_GUID.IsEmpty()) {
    route->m_GUID = proute->m_GUID;
  }
  route->m_btemp = (b_permanent == false);
  route->SetVisible(proute->m_isVisible);
  route->m_RouteDescription = proute->m_Description;

  pRouteList->Append(route);

  if (b_permanent) {
    // pConfig->AddNewRoute(route);
    NavObj_dB::GetInstance().InsertRoute(route);
  }

  if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
    pRouteManagerDialog->UpdateRouteListCtrl();

  return true;
}

bool UpdatePlugInRouteExV2(PlugIn_Route_ExV2* proute) {
  bool b_found = false;

  // Find the Route
  Route* pRoute = g_pRouteMan->FindRouteByGUID(proute->m_GUID);
  if (pRoute) b_found = true;

  if (b_found) {
    bool b_permanent = !pRoute->m_btemp;
    g_pRouteMan->DeleteRoute(pRoute);

    b_found = AddPlugInRouteExV2(proute, b_permanent);
  }

  return b_found;
}

std::unique_ptr<PlugIn_Route_ExV2> GetRouteExV2_Plugin(const wxString& GUID) {
  std::unique_ptr<PlugIn_Route_ExV2> r;
  Route* route = g_pRouteMan->FindRouteByGUID(GUID);
  if (route == nullptr) return r;

  r = std::unique_ptr<PlugIn_Route_ExV2>(new PlugIn_Route_ExV2);
  PlugIn_Route_ExV2* dst_route = r.get();

  RoutePoint* src_wp;
  wxRoutePointListNode* node = route->pRoutePointList->GetFirst();

  while (node) {
    src_wp = node->GetData();

    PlugIn_Waypoint_ExV2* dst_wp = new PlugIn_Waypoint_ExV2();
    PlugInExV2FromRoutePoint(dst_wp, src_wp);

    dst_route->pWaypointList->Append(dst_wp);

    node = node->GetNext();
  }
  dst_route->m_NameString = route->m_RouteNameString;
  dst_route->m_StartString = route->m_RouteStartString;
  dst_route->m_EndString = route->m_RouteEndString;
  dst_route->m_GUID = route->m_GUID;
  dst_route->m_isActive = g_pRouteMan->GetpActiveRoute() == route;
  dst_route->m_isVisible = route->IsVisible();
  dst_route->m_Description = route->m_RouteDescription;

  return r;
}

//      PlugInRouteExtended implementation
PlugIn_Route_Ex::PlugIn_Route_Ex(void) {
  pWaypointList = new Plugin_WaypointExList;
}

PlugIn_Route_Ex::~PlugIn_Route_Ex(void) {
  pWaypointList->DeleteContents(false);  // do not delete Waypoints
  pWaypointList->Clear();

  delete pWaypointList;
}

//  The utility methods implementations

// translate O route class to PlugIn_Waypoint_Ex
static void PlugInExFromRoutePoint(PlugIn_Waypoint_Ex* dst,
                                   /* const*/ RoutePoint* src) {
  dst->m_lat = src->m_lat;
  dst->m_lon = src->m_lon;
  dst->IconName = src->GetIconName();
  dst->m_MarkName = src->GetName();
  dst->m_MarkDescription = src->GetDescription();
  dst->IconDescription = pWayPointMan->GetIconDescription(src->GetIconName());
  dst->IsVisible = src->IsVisible();
  dst->m_CreateTime = src->GetCreateTime();  // not const
  dst->m_GUID = src->m_GUID;

  //  Transcribe (clone) the html HyperLink List, if present
  if (src->m_HyperlinkList) {
    delete dst->m_HyperlinkList;
    dst->m_HyperlinkList = nullptr;

    if (src->m_HyperlinkList->GetCount() > 0) {
      dst->m_HyperlinkList = new Plugin_HyperlinkList;

      wxHyperlinkListNode* linknode = src->m_HyperlinkList->GetFirst();
      while (linknode) {
        Hyperlink* link = linknode->GetData();

        Plugin_Hyperlink* h = new Plugin_Hyperlink();
        h->DescrText = link->DescrText;
        h->Link = link->Link;
        h->Type = link->LType;

        dst->m_HyperlinkList->Append(h);

        linknode = linknode->GetNext();
      }
    }
  }

  // Get the range ring info
  dst->nrange_rings = src->m_iWaypointRangeRingsNumber;
  dst->RangeRingSpace = src->m_fWaypointRangeRingsStep;
  dst->RangeRingColor = src->m_wxcWaypointRangeRingsColour;

  // Get other extended info
  dst->IsNameVisible = src->m_bShowName;
  dst->scamin = src->GetScaMin();
  dst->b_useScamin = src->GetUseSca();
  dst->IsActive = src->m_bIsActive;
}

static void cloneHyperlinkListEx(RoutePoint* dst,
                                 const PlugIn_Waypoint_Ex* src) {
  //  Transcribe (clone) the html HyperLink List, if present
  if (src->m_HyperlinkList == nullptr) return;

  if (src->m_HyperlinkList->GetCount() > 0) {
    wxPlugin_HyperlinkListNode* linknode = src->m_HyperlinkList->GetFirst();
    while (linknode) {
      Plugin_Hyperlink* link = linknode->GetData();

      Hyperlink* h = new Hyperlink();
      h->DescrText = link->DescrText;
      h->Link = link->Link;
      h->LType = link->Type;

      dst->m_HyperlinkList->Append(h);

      linknode = linknode->GetNext();
    }
  }
}

RoutePoint* CreateNewPoint(const PlugIn_Waypoint_Ex* src, bool b_permanent) {
  RoutePoint* pWP = new RoutePoint(src->m_lat, src->m_lon, src->IconName,
                                   src->m_MarkName, src->m_GUID);

  pWP->m_bIsolatedMark = true;  // This is an isolated mark

  cloneHyperlinkListEx(pWP, src);

  pWP->m_MarkDescription = src->m_MarkDescription;

  if (src->m_CreateTime.IsValid())
    pWP->SetCreateTime(src->m_CreateTime);
  else {
    pWP->SetCreateTime(wxDateTime::Now().ToUTC());
  }

  pWP->m_btemp = (b_permanent == false);

  // Extended fields
  pWP->SetIconName(src->IconName);
  pWP->SetWaypointRangeRingsNumber(src->nrange_rings);
  pWP->SetWaypointRangeRingsStep(src->RangeRingSpace);
  pWP->SetWaypointRangeRingsColour(src->RangeRingColor);
  pWP->SetScaMin(src->scamin);
  pWP->SetUseSca(src->b_useScamin);
  pWP->SetNameShown(src->IsNameVisible);
  pWP->SetVisible(src->IsVisible);

  return pWP;
}
bool GetSingleWaypointEx(wxString GUID, PlugIn_Waypoint_Ex* pwaypoint) {
  //  Find the RoutePoint
  RoutePoint* prp = pWayPointMan->FindRoutePointByGUID(GUID);

  if (!prp) return false;

  PlugInExFromRoutePoint(pwaypoint, prp);

  return true;
}

bool AddSingleWaypointEx(PlugIn_Waypoint_Ex* pwaypointex, bool b_permanent) {
  //  Validate the waypoint parameters a little bit

  //  GUID
  //  Make sure that this GUID is indeed unique in the Routepoint list
  bool b_unique = true;
  wxRoutePointListNode* prpnode = pWayPointMan->GetWaypointList()->GetFirst();
  while (prpnode) {
    RoutePoint* prp = prpnode->GetData();

    if (prp->m_GUID == pwaypointex->m_GUID) {
      b_unique = false;
      break;
    }
    prpnode = prpnode->GetNext();  // RoutePoint
  }

  if (!b_unique) return false;

  RoutePoint* pWP = CreateNewPoint(pwaypointex, b_permanent);

  pWP->SetShowWaypointRangeRings(pwaypointex->nrange_rings > 0);

  pSelect->AddSelectableRoutePoint(pWP->m_lat, pWP->m_lon, pWP);
  if (b_permanent) {
    // pConfig->AddNewWayPoint(pWP, -1);
    NavObj_dB::GetInstance().InsertRoutePoint(pWP);
  }
  if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
    pRouteManagerDialog->UpdateWptListCtrl();

  return true;
}

bool UpdateSingleWaypointEx(PlugIn_Waypoint_Ex* pwaypoint) {
  //  Find the RoutePoint
  bool b_found = false;
  RoutePoint* prp = pWayPointMan->FindRoutePointByGUID(pwaypoint->m_GUID);

  if (prp) b_found = true;

  if (b_found) {
    double lat_save = prp->m_lat;
    double lon_save = prp->m_lon;

    prp->m_lat = pwaypoint->m_lat;
    prp->m_lon = pwaypoint->m_lon;
    prp->SetIconName(pwaypoint->IconName);
    prp->SetName(pwaypoint->m_MarkName);
    prp->m_MarkDescription = pwaypoint->m_MarkDescription;
    prp->SetVisible(pwaypoint->IsVisible);
    if (pwaypoint->m_CreateTime.IsValid())
      prp->SetCreateTime(pwaypoint->m_CreateTime);

    //  Transcribe (clone) the html HyperLink List, if present

    if (pwaypoint->m_HyperlinkList) {
      prp->m_HyperlinkList->Clear();
      if (pwaypoint->m_HyperlinkList->GetCount() > 0) {
        wxPlugin_HyperlinkListNode* linknode =
            pwaypoint->m_HyperlinkList->GetFirst();
        while (linknode) {
          Plugin_Hyperlink* link = linknode->GetData();

          Hyperlink* h = new Hyperlink();
          h->DescrText = link->DescrText;
          h->Link = link->Link;
          h->LType = link->Type;

          prp->m_HyperlinkList->Append(h);

          linknode = linknode->GetNext();
        }
      }
    }

    // Extended fields
    prp->SetWaypointRangeRingsNumber(pwaypoint->nrange_rings);
    prp->SetWaypointRangeRingsStep(pwaypoint->RangeRingSpace);
    prp->SetWaypointRangeRingsColour(pwaypoint->RangeRingColor);
    prp->SetScaMin(pwaypoint->scamin);
    prp->SetUseSca(pwaypoint->b_useScamin);
    prp->SetNameShown(pwaypoint->IsNameVisible);

    prp->SetShowWaypointRangeRings(pwaypoint->nrange_rings > 0);

    if (prp) prp->ReLoadIcon();

    auto canvas = gFrame->GetPrimaryCanvas();
    SelectCtx ctx(canvas->m_bShowNavobjects, canvas->GetCanvasTrueScale(),
                  canvas->GetScaleValue());
    SelectItem* pFind =
        pSelect->FindSelection(ctx, lat_save, lon_save, SELTYPE_ROUTEPOINT);
    if (pFind) {
      pFind->m_slat = pwaypoint->m_lat;  // update the SelectList entry
      pFind->m_slon = pwaypoint->m_lon;
    }

    if (!prp->m_btemp) {
      // pConfig->UpdateWayPoint(prp);
      NavObj_dB::GetInstance().UpdateRoutePoint(prp);
    }

    if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
      pRouteManagerDialog->UpdateWptListCtrl();
  }

  return b_found;
}

bool AddPlugInRouteEx(PlugIn_Route_Ex* proute, bool b_permanent) {
  Route* route = new Route();

  PlugIn_Waypoint_Ex* pwaypointex;
  RoutePoint *pWP, *pWP_src;
  int ip = 0;
  wxDateTime plannedDeparture;

  wxPlugin_WaypointExListNode* pwpnode = proute->pWaypointList->GetFirst();
  while (pwpnode) {
    pwaypointex = pwpnode->GetData();

    pWP = pWayPointMan->FindRoutePointByGUID(pwaypointex->m_GUID);
    if (!pWP) {
      pWP = CreateNewPoint(pwaypointex, b_permanent);
      pWP->m_bIsolatedMark = false;
    }

    route->AddPoint(pWP);

    pSelect->AddSelectableRoutePoint(pWP->m_lat, pWP->m_lon, pWP);

    if (ip > 0)
      pSelect->AddSelectableRouteSegment(pWP_src->m_lat, pWP_src->m_lon,
                                         pWP->m_lat, pWP->m_lon, pWP_src, pWP,
                                         route);

    plannedDeparture = pwaypointex->m_CreateTime;
    ip++;
    pWP_src = pWP;

    pwpnode = pwpnode->GetNext();  // PlugInWaypoint
  }

  route->m_PlannedDeparture = plannedDeparture;

  route->m_RouteNameString = proute->m_NameString;
  route->m_RouteStartString = proute->m_StartString;
  route->m_RouteEndString = proute->m_EndString;
  if (!proute->m_GUID.IsEmpty()) {
    route->m_GUID = proute->m_GUID;
  }
  route->m_btemp = (b_permanent == false);
  route->SetVisible(proute->m_isVisible);
  route->m_RouteDescription = proute->m_Description;

  pRouteList->Append(route);

  if (b_permanent) {
    // pConfig->AddNewRoute(route);
    NavObj_dB::GetInstance().InsertRoute(route);
  }

  if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
    pRouteManagerDialog->UpdateRouteListCtrl();

  return true;
}

bool UpdatePlugInRouteEx(PlugIn_Route_Ex* proute) {
  bool b_found = false;

  //  Find the Route
  Route* pRoute = g_pRouteMan->FindRouteByGUID(proute->m_GUID);
  if (pRoute) b_found = true;

  if (b_found) {
    bool b_permanent = !pRoute->m_btemp;
    g_pRouteMan->DeleteRoute(pRoute);
    b_found = AddPlugInRouteEx(proute, b_permanent);
  }

  return b_found;
}

std::unique_ptr<PlugIn_Waypoint_Ex> GetWaypointEx_Plugin(const wxString& GUID) {
  std::unique_ptr<PlugIn_Waypoint_Ex> w(new PlugIn_Waypoint_Ex);
  GetSingleWaypointEx(GUID, w.get());
  return w;
}

std::unique_ptr<PlugIn_Route_Ex> GetRouteEx_Plugin(const wxString& GUID) {
  std::unique_ptr<PlugIn_Route_Ex> r;
  Route* route = g_pRouteMan->FindRouteByGUID(GUID);
  if (route == nullptr) return r;

  r = std::unique_ptr<PlugIn_Route_Ex>(new PlugIn_Route_Ex);
  PlugIn_Route_Ex* dst_route = r.get();

  // PlugIn_Waypoint *pwp;
  RoutePoint* src_wp;
  wxRoutePointListNode* node = route->pRoutePointList->GetFirst();

  while (node) {
    src_wp = node->GetData();

    PlugIn_Waypoint_Ex* dst_wp = new PlugIn_Waypoint_Ex();
    PlugInExFromRoutePoint(dst_wp, src_wp);

    dst_route->pWaypointList->Append(dst_wp);

    node = node->GetNext();
  }
  dst_route->m_NameString = route->m_RouteNameString;
  dst_route->m_StartString = route->m_RouteStartString;
  dst_route->m_EndString = route->m_RouteEndString;
  dst_route->m_GUID = route->m_GUID;
  dst_route->m_isActive = g_pRouteMan->GetpActiveRoute() == route;
  dst_route->m_isVisible = route->IsVisible();
  dst_route->m_Description = route->m_RouteDescription;

  return r;
}

wxString GetActiveWaypointGUID(
    void) {  // if no active waypoint, returns wxEmptyString
  RoutePoint* rp = g_pRouteMan->GetpActivePoint();
  if (!rp)
    return wxEmptyString;
  else
    return rp->m_GUID;
}

wxString GetActiveRouteGUID(
    void) {  // if no active route, returns wxEmptyString
  Route* rt = g_pRouteMan->GetpActiveRoute();
  if (!rt)
    return wxEmptyString;
  else
    return rt->m_GUID;
}

/** Comm Global Watchdog Query  */
int GetGlobalWatchdogTimoutSeconds() { return gps_watchdog_timeout_ticks; }

/** Comm Priority query support methods  */
std::vector<std::string> GetPriorityMaps() {
  MyApp& app = wxGetApp();
  return (app.m_comm_bridge.GetPriorityMaps());
}

void UpdateAndApplyPriorityMaps(std::vector<std::string> map) {
  MyApp& app = wxGetApp();
  app.m_comm_bridge.UpdateAndApplyMaps(map);
}

std::vector<std::string> GetActivePriorityIdentifiers() {
  std::vector<std::string> result;

  MyApp& app = wxGetApp();

  std::string id =
      app.m_comm_bridge.GetPriorityContainer("position").active_source;
  result.push_back(id);
  id = app.m_comm_bridge.GetPriorityContainer("velocity").active_source;
  result.push_back(id);
  id = app.m_comm_bridge.GetPriorityContainer("heading").active_source;
  result.push_back(id);
  id = app.m_comm_bridge.GetPriorityContainer("variation").active_source;
  result.push_back(id);
  id = app.m_comm_bridge.GetPriorityContainer("satellites").active_source;
  result.push_back(id);

  return result;
}

double OCPN_GetDisplayContentScaleFactor() {
  double rv = 1.0;
#if defined(__WXOSX__) || defined(__WXGTK3__)
  // Support scaled HDPI displays.
  if (gFrame) rv = gFrame->GetContentScaleFactor();
#endif
  return rv;
}
double OCPN_GetWinDIPScaleFactor() {
  double scaler = 1.0;
#ifdef __WXMSW__
  if (gFrame) scaler = (double)(gFrame->ToDIP(100)) / 100.;
#endif
  return scaler;
}

//---------------------------------------------------------------------------
//    API 1.18
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//    API 1.19
//---------------------------------------------------------------------------
void ExitOCPN() {}

bool GetFullScreen() { return gFrame->IsFullScreen(); }

void SetFullScreen(bool set_full_screen_on) {
  bool state = gFrame->IsFullScreen();
  if (set_full_screen_on && !state)
    gFrame->ToggleFullScreen();
  else if (!set_full_screen_on && state)
    gFrame->ToggleFullScreen();
}

extern bool g_useMUI;
void EnableMUIBar(bool enable, int CanvasIndex) {
  bool current_mui_state = g_useMUI;

  g_useMUI = enable;
  if (enable && !current_mui_state) {  // OFF going ON
    // ..For each canvas...
    for (unsigned int i = 0; i < g_canvasArray.GetCount(); i++) {
      ChartCanvas* cc = g_canvasArray.Item(i);
      if (cc) cc->CreateMUIBar();
    }
  } else if (!enable && current_mui_state) {  // ON going OFF
    // ..For each canvas...
    for (unsigned int i = 0; i < g_canvasArray.GetCount(); i++) {
      ChartCanvas* cc = g_canvasArray.Item(i);
      if (cc) cc->DestroyMuiBar();
    }
  }
}

bool GetEnableMUIBar(int CanvasIndex) { return g_useMUI; }

void EnableCompassGPSIcon(bool enable, int CanvasIndex) {
  if (CanvasIndex < GetCanvasCount()) {
    ChartCanvas* cc = g_canvasArray.Item(CanvasIndex);
    if (cc) cc->SetShowGPSCompassWindow(enable);
  }
}

bool GetEnableCompassGPSIcon(int CanvasIndex) {
  if (CanvasIndex < GetCanvasCount()) {
    ChartCanvas* cc = g_canvasArray.Item(CanvasIndex);
    if (cc)
      return cc->GetShowGPSCompassWindow();
    else
      return false;
  }
  return false;
}

extern bool g_bShowStatusBar;
void EnableStatusBar(bool enable) {
  g_bShowStatusBar = enable;
  gFrame->ConfigureStatusBar();
}

bool GetEnableStatusBar() { return g_bShowStatusBar; }

void EnableChartBar(bool enable, int CanvasIndex) {
  bool current_chartbar_state = g_bShowChartBar;
  for (unsigned int i = 0; i < g_canvasArray.GetCount(); i++) {
    ChartCanvas* cc = g_canvasArray.Item(i);
    if (current_chartbar_state && !enable) {
      gFrame->ToggleChartBar(cc);
      g_bShowChartBar = current_chartbar_state;
    } else if (!current_chartbar_state && enable) {
      gFrame->ToggleChartBar(cc);
      g_bShowChartBar = current_chartbar_state;
    }
  }
  g_bShowChartBar = enable;
}

bool GetEnableChartBar(int CanvasIndex) { return g_bShowChartBar; }

extern bool g_bShowMenuBar;
void EnableMenu(bool enable) {
  if (!enable) {
    if (g_bShowMenuBar) {
      g_bShowMenuBar = false;
      if (gFrame->m_pMenuBar) {
        gFrame->SetMenuBar(NULL);
        gFrame->m_pMenuBar->Destroy();
        gFrame->m_pMenuBar = NULL;
      }
    }
  } else {
    g_bShowMenuBar = true;
    gFrame->BuildMenuBar();
  }
}

bool GetEnableMenu() { return g_bShowMenuBar; }

void SetGlobalColor(std::string table, std::string name, wxColor color) {
  if (ps52plib) ps52plib->m_chartSymbols.UpdateTableColor(table, name, color);
}

wxColor GetGlobalColorD(std::string map_name, std::string name) {
  wxColor ret = wxColor(*wxRED);
  if (ps52plib) {
    int i_table = ps52plib->m_chartSymbols.FindColorTable(map_name.c_str());
    ret = ps52plib->m_chartSymbols.GetwxColor(name.c_str(), i_table);
  }
  return ret;
}

void EnableLatLonGrid(bool enable, int CanvasIndex) {
  if (CanvasIndex < GetCanvasCount()) {
    ChartCanvas* cc = g_canvasArray.Item(CanvasIndex);
    if (cc) cc->SetShowGrid(enable);
  }
}

void EnableChartOutlines(bool enable, int CanvasIndex) {
  if (CanvasIndex < GetCanvasCount()) {
    ChartCanvas* cc = g_canvasArray.Item(CanvasIndex);
    if (cc) cc->SetShowOutlines(enable);
  }
}

void EnableDepthUnitDisplay(bool enable, int CanvasIndex) {
  if (CanvasIndex < GetCanvasCount()) {
    ChartCanvas* cc = g_canvasArray.Item(CanvasIndex);
    if (cc) cc->SetShowDepthUnits(enable);
  }
}

void EnableAisTargetDisplay(bool enable, int CanvasIndex) {
  if (CanvasIndex < GetCanvasCount()) {
    ChartCanvas* cc = g_canvasArray.Item(CanvasIndex);
    if (cc) cc->SetShowAIS(enable);
  }
}

void EnableTideStationsDisplay(bool enable, int CanvasIndex) {
  if (CanvasIndex < GetCanvasCount()) {
    ChartCanvas* cc = g_canvasArray.Item(CanvasIndex);
    if (cc) cc->ShowTides(enable);
  }
}

void EnableCurrentStationsDisplay(bool enable, int CanvasIndex) {
  if (CanvasIndex < GetCanvasCount()) {
    ChartCanvas* cc = g_canvasArray.Item(CanvasIndex);
    if (cc) cc->ShowCurrents(enable);
  }
}

void EnableENCTextDisplay(bool enable, int CanvasIndex) {
  if (CanvasIndex < GetCanvasCount()) {
    ChartCanvas* cc = g_canvasArray.Item(CanvasIndex);
    if (cc) cc->SetShowENCText(enable);
  }
}

void EnableENCDepthSoundingsDisplay(bool enable, int CanvasIndex) {
  if (CanvasIndex < GetCanvasCount()) {
    ChartCanvas* cc = g_canvasArray.Item(CanvasIndex);
    if (cc) cc->SetShowENCDepth(enable);
  }
}

void EnableBuoyLightLabelsDisplay(bool enable, int CanvasIndex) {
  if (CanvasIndex < GetCanvasCount()) {
    ChartCanvas* cc = g_canvasArray.Item(CanvasIndex);
    if (cc) cc->SetShowENCBuoyLabels(enable);
  }
}

void EnableLightsDisplay(bool enable, int CanvasIndex) {
  if (CanvasIndex < GetCanvasCount()) {
    ChartCanvas* cc = g_canvasArray.Item(CanvasIndex);
    if (cc) cc->SetShowENCLights(enable);
  }
}

void EnableLightDescriptionsDisplay(bool enable, int CanvasIndex) {
  if (CanvasIndex < GetCanvasCount()) {
    ChartCanvas* cc = g_canvasArray.Item(CanvasIndex);
    if (cc) cc->SetShowENCLightDesc(enable);
  }
}

void SetENCDisplayCategory(PI_DisCat cat, int CanvasIndex) {
  int valSet = STANDARD;
  switch (cat) {
    case PI_DISPLAYBASE:
      valSet = DISPLAYBASE;
      break;
    case PI_STANDARD:
      valSet = STANDARD;
      break;
    case PI_OTHER:
      valSet = OTHER;
      break;
    case PI_MARINERS_STANDARD:
      valSet = MARINERS_STANDARD;
      break;
    default:
      valSet = STANDARD;
      break;
  }
  if (CanvasIndex < GetCanvasCount()) {
    ChartCanvas* cc = g_canvasArray.Item(CanvasIndex);
    if (cc) cc->SetENCDisplayCategory(valSet);
  }
}
PI_DisCat GetENCDisplayCategory(int CanvasIndex) {
  ChartCanvas* cc = g_canvasArray.Item(CanvasIndex);
  if (cc)
    return ((PI_DisCat)cc->GetENCDisplayCategory());
  else
    return PI_DisCat::PI_STANDARD;
}

void SetNavigationMode(PI_NavMode mode, int CanvasIndex) {
  int newMode = NORTH_UP_MODE;
  if (mode == PI_COURSE_UP_MODE)
    newMode = COURSE_UP_MODE;
  else if (mode == PI_HEAD_UP_MODE)
    newMode = HEAD_UP_MODE;

  if (CanvasIndex < GetCanvasCount()) {
    ChartCanvas* cc = g_canvasArray.Item(CanvasIndex);
    if (cc) cc->SetUpMode(newMode);
  }
}
PI_NavMode GetNavigationMode(int CanvasIndex) {
  if (CanvasIndex < GetCanvasCount()) {
    ChartCanvas* cc = g_canvasArray.Item(CanvasIndex);
    if (cc) return ((PI_NavMode)cc->GetUpMode());
  }
  return PI_NavMode::PI_NORTH_UP_MODE;
}

bool GetEnableLatLonGrid(int CanvasIndex) {
  if (CanvasIndex < GetCanvasCount()) {
    ChartCanvas* cc = g_canvasArray.Item(CanvasIndex);
    if (cc) return (cc->GetShowGrid());
  }
  return false;
}

bool GetEnableChartOutlines(int CanvasIndex) {
  if (CanvasIndex < GetCanvasCount()) {
    ChartCanvas* cc = g_canvasArray.Item(CanvasIndex);
    if (cc) return (cc->GetShowOutlines());
  }
  return false;
}

bool GetEnableDepthUnitDisplay(int CanvasIndex) {
  if (CanvasIndex < GetCanvasCount()) {
    ChartCanvas* cc = g_canvasArray.Item(CanvasIndex);
    if (cc) return (cc->GetShowDepthUnits());
  }
  return false;
}

bool GetEnableAisTargetDisplay(int CanvasIndex) {
  if (CanvasIndex < GetCanvasCount()) {
    ChartCanvas* cc = g_canvasArray.Item(CanvasIndex);
    if (cc) return (cc->GetShowAIS());
  }
  return false;
}

bool GetEnableTideStationsDisplay(int CanvasIndex) {
  if (CanvasIndex < GetCanvasCount()) {
    ChartCanvas* cc = g_canvasArray.Item(CanvasIndex);
    if (cc) return (cc->GetbShowTide());
  }
  return false;
}

bool GetEnableCurrentStationsDisplay(int CanvasIndex) {
  if (CanvasIndex < GetCanvasCount()) {
    ChartCanvas* cc = g_canvasArray.Item(CanvasIndex);
    if (cc) return (cc->GetbShowCurrent());
  }
  return false;
}

bool GetEnableENCTextDisplay(int CanvasIndex) {
  if (CanvasIndex < GetCanvasCount()) {
    ChartCanvas* cc = g_canvasArray.Item(CanvasIndex);
    if (cc) return (cc->GetShowENCText());
  }
  return false;
}

bool GetEnableENCDepthSoundingsDisplay(int CanvasIndex) {
  if (CanvasIndex < GetCanvasCount()) {
    ChartCanvas* cc = g_canvasArray.Item(CanvasIndex);
    if (cc) return (cc->GetShowENCDepth());
  }
  return false;
}

bool GetEnableBuoyLightLabelsDisplay(int CanvasIndex) {
  if (CanvasIndex < GetCanvasCount()) {
    ChartCanvas* cc = g_canvasArray.Item(CanvasIndex);
    if (cc) return (cc->GetShowENCBuoyLabels());
  }
  return false;
}

bool GetEnableLightsDisplay(int CanvasIndex) {
  if (CanvasIndex < GetCanvasCount()) {
    ChartCanvas* cc = g_canvasArray.Item(CanvasIndex);
    if (cc) return (cc->GetShowENCLights());
  }
  return false;
}

bool GetShowENCLightDesc(int CanvasIndex) {
  if (CanvasIndex < GetCanvasCount()) {
    ChartCanvas* cc = g_canvasArray.Item(CanvasIndex);
    if (cc) return (cc->GetbShowCurrent());
  }
  return false;
}

void EnableTouchMode(bool enable) { g_btouch = enable; }

bool GetTouchMode() { return g_btouch; }

void EnableLookaheadMode(bool enable, int CanvasIndex) {
  if (CanvasIndex < GetCanvasCount()) {
    ChartCanvas* cc = g_canvasArray.Item(CanvasIndex);
    if (cc) cc->ToggleLookahead();
  }
}

bool GetEnableLookaheadMode(int CanvasIndex) {
  if (CanvasIndex < GetCanvasCount()) {
    ChartCanvas* cc = g_canvasArray.Item(CanvasIndex);
    if (cc) return (cc->GetLookahead());
  }
  return false;
}

extern bool g_bTrackActive;
void SetTrackingMode(bool enable) {
  if (!g_bTrackActive && enable)
    gFrame->TrackOn();
  else if (g_bTrackActive && !enable)
    gFrame->TrackOff();
}
bool GetTrackingMode() { return g_bTrackActive; }

void SetAppColorScheme(PI_ColorScheme cs) {
  gFrame->SetAndApplyColorScheme((ColorScheme)cs);
}
PI_ColorScheme GetAppColorScheme() {
  return (PI_ColorScheme)global_color_scheme;
}

void RequestWindowRefresh(wxWindow* win, bool eraseBackground) {
  if (win) win->Refresh(eraseBackground);
}

void EnableSplitScreenLayout(bool enable) {
  if (g_canvasConfig == 1) {
    if (enable)
      return;
    else {                 // split to single
      g_canvasConfig = 0;  // 0 => "single canvas"
      gFrame->CreateCanvasLayout();
      gFrame->DoChartUpdate();
    }
  } else {
    if (enable) {          // single to split
      g_canvasConfig = 1;  // 1 => "two canvas"
      gFrame->CreateCanvasLayout();
      gFrame->DoChartUpdate();
    } else {
      return;
    }
  }
}

// ChartCanvas control utilities

void PluginZoomCanvas(int CanvasIndex, double factor) {
  if (CanvasIndex < GetCanvasCount()) {
    ChartCanvas* cc = g_canvasArray.Item(CanvasIndex);
    if (cc) cc->ZoomCanvasSimple(factor);
  }
}

bool GetEnableMainToolbar() { return (!g_disable_main_toolbar); }
void SetEnableMainToolbar(bool enable) {
  g_disable_main_toolbar = !enable;
  if (g_MainToolbar) g_MainToolbar->RefreshToolbar();
}

void ShowGlobalSettingsDialog() {
  if (gFrame) gFrame->ScheduleSettingsDialog();
}

void PluginCenterOwnship(int CanvasIndex) {
  if (CanvasIndex < GetCanvasCount()) {
    ChartCanvas* cc = g_canvasArray.Item(CanvasIndex);
    if (cc) {
      bool bfollow = cc->GetbFollow();
      cc->ResetOwnshipOffset();
      if (bfollow)
        cc->SetbFollow();
      else
        cc->JumpToPosition(gLat, gLon, cc->GetVPScale());
    }
  }
}

void PluginSetFollowMode(int CanvasIndex, bool enable_follow) {
  if (CanvasIndex < GetCanvasCount()) {
    ChartCanvas* cc = g_canvasArray.Item(CanvasIndex);
    if (cc) {
      if (cc->GetbFollow() != enable_follow) cc->TogglebFollow();
    }
  }
}

bool PluginGetFollowMode(int CanvasIndex) {
  if (CanvasIndex < GetCanvasCount()) {
    ChartCanvas* cc = g_canvasArray.Item(CanvasIndex);
    if (cc) return cc->GetbFollow();
  }
  return false;
}

void EnableCanvasFocusBar(bool enable, int CanvasIndex) {
  if (CanvasIndex < GetCanvasCount()) {
    ChartCanvas* cc = g_canvasArray.Item(CanvasIndex);
    if (cc) cc->SetShowFocusBar(enable);
  }
}
bool GetEnableCanvasFocusBar(int CanvasIndex) {
  if (CanvasIndex < GetCanvasCount()) {
    ChartCanvas* cc = g_canvasArray.Item(CanvasIndex);
    if (cc) return (cc->GetShowFocusBar());
  }
  return false;
}

bool GetEnableTenHertzUpdate() { return g_btenhertz; }

void EnableTenHertzUpdate(bool enable) { g_btenhertz = enable; }

void ConfigFlushAndReload() {
  if (pConfig) {
    pConfig->Flush();

    // Handle system general configuration options
    pConfig->LoadMyConfigRaw(false);

    // Handle chart canvas window configuration options
    pConfig->LoadCanvasConfigs(false);
    auto& config_array = ConfigMgr::Get().GetCanvasConfigArray();
    for (auto pcc : config_array) {
      if (pcc && pcc->canvas) {
        pcc->canvas->ApplyCanvasConfig(pcc);
        pcc->canvas->Refresh();
      }
    }
  }
}

/**
 * Plugin Notification Framework GUI support
 */
void EnableNotificationCanvasIcon(bool enable) {
  g_CanvasHideNotificationIcon = !enable;
}
