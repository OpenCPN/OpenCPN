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
 *
 * ocpn_plugin.h GUI API funtions up to api level 1.20
 */
#include <cstddef>
#include <cstdint>
#include <climits>
#include <chrono>
#include <cmath>
#include <cstring>
#include <fstream>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include "dychart.h"  // Must be ahead due to buggy GL includes handling

#include <wx/wx.h>
#include <wx/arrstr.h>
#include <wx/dc.h>
#include <wx/dcmemory.h>
#include <wx/event.h>
#include <wx/glcanvas.h>
#include <wx/thread.h>
#include <wx/notebook.h>
#include <wx/string.h>
#include <wx/window.h>
#include <wx/filefn.h>
#include <wx/filename.h>
#include <wx/jsonreader.h>
#include <wx/jsonval.h>
#include <wx/jsonwriter.h>
#include <wx/wfstream.h>

#include "o_sound/o_sound.h"

#include "model/ais_decoder.h"
#include "model/comm_bridge.h"
#include "model/comm_navmsg_bus.h"
#include "model/gui_vars.h"
#include "model/idents.h"
#include "model/multiplexer.h"
#include "model/navobj_db.h"
#include "model/notification_manager.h"
#include "model/own_ship.h"
#include "model/plugin_loader.h"
#include "model/plugin_comm.h"
#include "model/svg_utils.h"
#include "model/route.h"
#include "model/track.h"

#include "ais.h"
#include "chartdb.h"
#include "chcanv.h"
#include "cm93.h"
#include "config_mgr.h"
#include "font_mgr.h"
#include "gl_chart_canvas.h"
#include "chart_safety_depth.h"
#include "chart_safety_service.h"
#include "gui_lib.h"
#include "navutil.h"
#include "ocpn_aui_manager.h"
#include "ocpn_frame.h"
#include "ocpn_platform.h"
#include "ocpn_plugin.h"
#include "options.h"
#include "piano.h"
#include "pluginmanager.h"
#include "routemanagerdialog.h"
#include "routeman_gui.h"
#include "s52plib.h"
#include "s57chart.h"
#include "shapefile_basemap.h"
#include "toolbar.h"
#include "waypointman_gui.h"

#if wxUSE_XLOCALE || !wxCHECK_VERSION(3, 0, 0)
extern wxLocale* plocale_def_lang;
#endif

extern PlugInManager* s_ppim;  // FIXME (leamas) another name for global mgr

extern options* g_pOptions;  // FIXME (leamas) merge to g_options

extern arrayofCanvasPtr g_canvasArray;  // FIXME (leamas) find new home

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
wxFileConfig* GetOCPNConfigObject() {
  if (s_ppim)
    return reinterpret_cast<wxFileConfig*>(
        pConfig);  // return the global application config object
  else
    return NULL;
}

wxWindow* GetOCPNCanvasWindow() {
  wxWindow* pret = NULL;
  if (s_ppim) {
    AbstractTopFrame* pFrame = s_ppim->GetParentFrame();
    pret = (wxWindow*)pFrame->GetAbstractPrimaryCanvas();
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
  return FontMgr::Get().GetFontLegacy(TextElement, default_size);
}

wxFont* GetOCPNScaledFont_PlugIn(wxString TextElement, int default_size) {
  return FontMgr::Get().GetFontLegacy(TextElement, default_size);
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
    return "";
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

bool IsTouchInterface_PlugIn() { return g_btouch; }

wxColour GetFontColour_PlugIn(wxString TextElement) {
  return FontMgr::Get().GetFontColor(TextElement);
}

wxString* GetpSharedDataLocation() { return g_Platform->GetSharedDataDirPtr(); }

ArrayOfPlugIn_AIS_Targets* GetAISTargetArray() {
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

wxAuiManager* GetFrameAuiManager() { return g_pauimgr; }

void SendPluginMessage(wxString message_id, wxString message_body) {
  SendMessageToAllPlugins(message_id.ToStdString(), message_body.ToStdString());

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
    // We notify based on full message, including the Talker ID
    std::string id = full_sentence.substr(1, 5);

    // notify message listener
    auto address = std::make_shared<NavAddr0183>("virtual");
    auto msg = std::make_shared<const Nmea0183Msg>(id, full_sentence, address);
    NavMsgBus::GetInstance().Notify(std::move(msg));
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
    cdi.magic_number = "";
    ChartDirArray.Add(cdi);
  }
  bool b_ret = gFrame->UpdateChartDatabaseInplace(
      ChartDirArray, b_force_update, b_ProgressDialog, ChartListFileName);
  gFrame->RefreshGroupIndices();
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
      wxLogMessage("Error in PluginManager::AddOptionsPage: Unknown parent");
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

int GetChartbarHeight() {
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
  wxString gpxfilename = wxFileName::CreateTempFileName("gpx");
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

/**
 * Height Conversion Functions
 */
double toUsrHeight_Plugin(double m_height, int unit) {
  return toUsrHeight(m_height, unit);
}

double fromUsrHeight_Plugin(double usr_height, int unit) {
  return fromUsrHeight(usr_height, unit);
}

wxString getUsrHeightUnit_Plugin(int unit) { return getUsrHeightUnit(unit); }

double fromDMM_PlugIn(wxString sdms) { return fromDMM(sdms); }

bool PlugIn_GSHHS_CrossesLand(double lat1, double lon1, double lat2,
                              double lon2) {
  // TODO: Enable call to gShapeBasemap.CrossesLand after fixing performance
  // issues. if (gShapeBasemap.IsUsable()) {
  //   return gShapeBasemap.CrossesLand(lat1, lon1, lat2, lon2);
  // } else {
  //  Fall back to the GSHHS data.
  return gshhsCrossesLand(lat1, lon1, lat2, lon2);
  //}
}

namespace {

void CopySegmentSafetyString(char* dest, size_t dest_size, const char* source) {
  if (!dest || dest_size == 0) return;
  snprintf(dest, dest_size, "%s", source ? source : "");
}

void SetSegmentSafetyMessage(PlugInSegmentSafetyResult* result,
                             const char* message) {
  if (!result) return;
  if (result->struct_size < (int)(offsetof(PlugInSegmentSafetyResult, message) +
                                  sizeof(result->message)))
    return;
  CopySegmentSafetyString(result->message, sizeof(result->message), message);
}

void InitSegmentSafetyResult(PlugInSegmentSafetyResult* result) {
  if (!result) return;
  if (result->struct_size >= (int)(offsetof(PlugInSegmentSafetyResult, status) +
                                   sizeof(result->status)))
    result->status = PI_SEGMENT_SAFETY_ERROR;
  if (result->struct_size >= (int)(offsetof(PlugInSegmentSafetyResult, source) +
                                   sizeof(result->source)))
    result->source = PI_SEGMENT_SAFETY_SOURCE_NONE;
  if (result->struct_size >=
      (int)(offsetof(PlugInSegmentSafetyResult, used_fallback) +
            sizeof(result->used_fallback)))
    result->used_fallback = 0;
  if (result->struct_size >=
      (int)(offsetof(PlugInSegmentSafetyResult, message) +
            sizeof(result->message)))
    result->message[0] = '\0';
  if (result->struct_size >=
      (int)(offsetof(PlugInSegmentSafetyResult, geometry_check_ms) +
            sizeof(result->geometry_check_ms))) {
    result->diagnostic_reason = PI_SEGMENT_SAFETY_DIAG_NONE;
    result->chart_stack_entries = 0;
    result->candidate_chart_count = 0;
    result->raster_chart_count = 0;
    result->unsupported_chart_count = 0;
    result->s57_chart_count = 0;
    result->land_ring_count = 0;
    result->bbox_ring_tests = 0;
    result->edge_tests = 0;
    result->cache_build_ms = 0;
    result->chart_select_ms = 0;
    result->geometry_check_ms = 0;
  }
  if (result->struct_size >=
      (int)(offsetof(PlugInSegmentSafetyResult, chart_path) +
            sizeof(result->chart_path))) {
    result->chart_db_index = -1;
    result->hit_cause = PI_SEGMENT_SAFETY_HIT_NONE;
    result->hit_ring_min_lat = 0.0;
    result->hit_ring_max_lat = 0.0;
    result->hit_ring_min_lon = 0.0;
    result->hit_ring_max_lon = 0.0;
    result->hit_ring_point_count = 0;
    result->hit_edge_index = -1;
    result->chart_path[0] = '\0';
  }
  if (result->struct_size >=
      (int)(offsetof(PlugInSegmentSafetyResult, hit_object) +
            sizeof(result->hit_object))) {
    result->hit_sample_lat = 0.0;
    result->hit_sample_lon = 0.0;
    result->hit_sample_index = -1;
    result->hit_sample_count = 0;
    result->chart_scale = -1;
    result->hit_object[0] = '\0';
  }
  if (result->struct_size >=
      (int)(offsetof(PlugInSegmentSafetyResult, point_cache_misses) +
            sizeof(result->point_cache_misses))) {
    result->point_cache_hits = 0;
    result->point_cache_misses = 0;
  }
  if (result->struct_size >=
      (int)(offsetof(PlugInSegmentSafetyResult, grid_lookups) +
            sizeof(result->grid_lookups))) {
    result->grid_cache_hits = 0;
    result->grid_cache_misses = 0;
    result->grid_build_ms = 0;
    result->grid_cells_total = 0;
    result->grid_cells_land = 0;
    result->grid_cells_water = 0;
    result->grid_cells_drying = 0;
    result->grid_cells_unknown = 0;
    result->grid_lookups = 0;
  }
  if (result->struct_size >=
      (int)(offsetof(PlugInSegmentSafetyResult, unexpected_tile_builds) +
            sizeof(result->unexpected_tile_builds))) {
    result->grid_lookup_ms = 0;
    result->segment_sample_count = 0;
    result->water_tile_shortcuts = 0;
    result->unexpected_tile_builds = 0;
  }
  if (result->struct_size >=
      (int)(offsetof(PlugInSegmentSafetyResult, unexpected_tile_min_lon) +
            sizeof(result->unexpected_tile_min_lon))) {
    result->unexpected_lat_tile = 0;
    result->unexpected_lon_tile = 0;
    result->unexpected_tile_min_lat = 0.0;
    result->unexpected_tile_min_lon = 0.0;
  }
  if (result->struct_size >=
      (int)(offsetof(PlugInSegmentSafetyResult, grid_cache_evictions) +
            sizeof(result->grid_cache_evictions))) {
    result->segment_cache_hits = 0;
    result->segment_cache_misses = 0;
    result->segment_cache_stores = 0;
    result->grid_cache_size = 0;
    result->grid_cache_evictions = 0;
  }
  if (result->struct_size >=
      (int)(offsetof(PlugInSegmentSafetyResult, depth_source_attribute) +
            sizeof(result->depth_source_attribute))) {
    result->has_depth = 0;
    result->min_depth_m = 0.0;
    result->required_depth_m = 0.0;
    result->hit_depth_m = 0.0;
    result->has_drying = 0;
    result->depth_source_object[0] = '\0';
    result->depth_source_attribute[0] = '\0';
  }
  if (result->struct_size >=
      (int)(offsetof(PlugInSegmentSafetyResult, prewarm_fine_tiles_avoided) +
            sizeof(result->prewarm_fine_tiles_avoided))) {
    result->prewarm_requested_tiles = 0;
    result->prewarm_base_tiles_built = 0;
    result->prewarm_base_tiles_reused = 0;
    result->prewarm_masks_built = 0;
    result->prewarm_masks_reused = 0;
    result->prewarm_fine_tiles_avoided = 0;
  }
}

void SetSegmentSafetyStatus(PlugInSegmentSafetyResult* result,
                            PlugInSegmentSafetyStatus status) {
  if (!result) return;
  if (result->struct_size >= (int)(offsetof(PlugInSegmentSafetyResult, status) +
                                   sizeof(result->status)))
    result->status = status;
}

void SetSegmentSafetySource(PlugInSegmentSafetyResult* result,
                            PlugInSegmentSafetySource source) {
  if (!result) return;
  if (result->struct_size >= (int)(offsetof(PlugInSegmentSafetyResult, source) +
                                   sizeof(result->source)))
    result->source = source;
}

void SetSegmentSafetyFallback(PlugInSegmentSafetyResult* result,
                              bool used_fallback) {
  if (!result) return;
  if (result->struct_size >=
      (int)(offsetof(PlugInSegmentSafetyResult, used_fallback) +
            sizeof(result->used_fallback)))
    result->used_fallback = used_fallback ? 1 : 0;
}

PlugInSegmentSafetySource GetSegmentSafetySource(
    const PlugInSegmentSafetyResult* result) {
  if (!result) return PI_SEGMENT_SAFETY_SOURCE_NONE;
  if (result->struct_size >= (int)(offsetof(PlugInSegmentSafetyResult, source) +
                                   sizeof(result->source)))
    return (PlugInSegmentSafetySource)result->source;
  return PI_SEGMENT_SAFETY_SOURCE_NONE;
}

bool SegmentSafetyResultHas(const PlugInSegmentSafetyResult* result,
                            size_t offset, size_t size) {
  return result && result->struct_size >= (int)(offset + size);
}

void SetSegmentSafetyDiagnosticReason(
    PlugInSegmentSafetyResult* result,
    PlugInSegmentSafetyDiagnosticReason reason) {
  if (!SegmentSafetyResultHas(
          result, offsetof(PlugInSegmentSafetyResult, diagnostic_reason),
          sizeof(result->diagnostic_reason)))
    return;
  result->diagnostic_reason = reason;
}

void SetSegmentSafetyDiagnosticInt(PlugInSegmentSafetyResult* result,
                                   size_t offset, int value) {
  if (!SegmentSafetyResultHas(result, offset, sizeof(int))) return;
  *reinterpret_cast<int*>(reinterpret_cast<char*>(result) + offset) = value;
}

void SetSegmentSafetyDiagnosticDouble(PlugInSegmentSafetyResult* result,
                                      size_t offset, double value) {
  if (!SegmentSafetyResultHas(result, offset, sizeof(double))) return;
  *reinterpret_cast<double*>(reinterpret_cast<char*>(result) + offset) = value;
}

bool SegmentSafetyOptionsHas(const PlugInSegmentSafetyOptions* options,
                             size_t offset, size_t size) {
  return options && options->struct_size >= (int)(offset + size);
}

double SegmentSafetyOptionMargin(const PlugInSegmentSafetyOptions* options) {
  if (SegmentSafetyOptionsHas(
          options, offsetof(PlugInSegmentSafetyOptions, safety_margin_nm),
          sizeof(options->safety_margin_nm)))
    return wxMax(0.0, options->safety_margin_nm);
  return 0.0;
}

bool SegmentSafetyOptionCheckLand(const PlugInSegmentSafetyOptions* options) {
  if (SegmentSafetyOptionsHas(options,
                              offsetof(PlugInSegmentSafetyOptions, check_land),
                              sizeof(options->check_land)))
    return options->check_land != 0;
  return true;
}

bool SegmentSafetyOptionAllowGshhsFallback(
    const PlugInSegmentSafetyOptions* options) {
  if (SegmentSafetyOptionsHas(
          options, offsetof(PlugInSegmentSafetyOptions, allow_gshhs_fallback),
          sizeof(options->allow_gshhs_fallback)))
    return options->allow_gshhs_fallback != 0;
  return true;
}

bool SegmentSafetyOptionCheckDepth(const PlugInSegmentSafetyOptions* options) {
  if (SegmentSafetyOptionsHas(options,
                              offsetof(PlugInSegmentSafetyOptions, check_depth),
                              sizeof(options->check_depth)))
    return options->check_depth != 0;
  return false;
}

double SegmentSafetyOptionMinimumDepthM(
    const PlugInSegmentSafetyOptions* options) {
  if (SegmentSafetyOptionsHas(
          options, offsetof(PlugInSegmentSafetyOptions, minimum_depth_m),
          sizeof(options->minimum_depth_m)))
    return wxMax(0.0, options->minimum_depth_m);
  return 0.0;
}

bool SegmentSafetyOptionForceAuthoritativeFineValidation(
    const PlugInSegmentSafetyOptions* options) {
  if (SegmentSafetyOptionsHas(
          options,
          offsetof(PlugInSegmentSafetyOptions,
                   force_authoritative_fine_validation),
          sizeof(options->force_authoritative_fine_validation)))
    return options->force_authoritative_fine_validation != 0;
  return false;
}

bool IsSegmentSafetyLandObject(const char* feature_name) {
  return feature_name && !strncmp(feature_name, "LNDARE", 6);
}

bool IsCm93Chart(ChartBase* chart) {
  return chart && chart->GetChartType() == CHART_TYPE_CM93COMP;
}

std::string SegmentSafetyCacheKey(int db_index, bool cm93, double lat,
                                  double lon) {
  if (!cm93) return std::to_string(db_index);

  double lat_bucket = floor(lat * 4.0) / 4.0;
  double lon_bucket = floor(lon * 4.0) / 4.0;
  return wxString::Format("%d:%.2f:%.2f", db_index, lat_bucket, lon_bucket)
      .ToStdString();
}

ViewPort SegmentSafetyViewPortAt(double lat, double lon) {
  ViewPort vp;
  ChartCanvas* canvas = gFrame ? gFrame->GetFocusCanvas() : NULL;
  if (canvas) vp = canvas->GetVP();

  vp.clat = lat;
  vp.clon = lon;
  if (vp.pix_width <= 0) vp.pix_width = 1024;
  if (vp.pix_height <= 0) vp.pix_height = 768;
  if (vp.view_scale_ppm <= 0.0) vp.view_scale_ppm = 1.0 / 1852.0;
  if (vp.chart_scale <= 0.0) vp.chart_scale = 100000;
  if (vp.ref_scale <= 0) vp.ref_scale = vp.chart_scale;
  vp.SetBoxes();
  return vp;
}

ViewPort SegmentSafetyHighestDetailViewPortAt(double lat, double lon) {
  ViewPort vp = SegmentSafetyViewPortAt(lat, lon);
  // CM93 composite chart selection is viewport-scale dependent.  A safety
  // classification must not depend on the user's current zoom level: request
  // the highest local CM93 scale and let cm93compchart fall back only when
  // that cell scale is unavailable at this position.
  vp.view_scale_ppm = 1.0;
  vp.chart_scale = 5000.0;
  vp.ref_scale = 5000;
  vp.SetBoxes();
  return vp;
}

double SegmentSafetyNormalizeBearing(double bearing) {
  while (bearing < 0.0) bearing += 360.0;
  while (bearing >= 360.0) bearing -= 360.0;
  return bearing;
}

bool ChartPointIsLand(s57chart* chart, double lat, double lon, ViewPort& vp) {
  if (!chart) return false;

  ListOfObjRazRules* rule_list =
      chart->GetObjRuleListAtLatLon(lat, lon, 0.0, &vp, MASK_AREA);
  if (!rule_list) return false;

  bool is_land = false;
  for (ListOfObjRazRules::Node* node = rule_list->GetFirst(); node;
       node = node->GetNext()) {
    ObjRazRules* rule = node->GetData();
    if (rule && rule->obj &&
        IsSegmentSafetyLandObject(rule->obj->FeatureName)) {
      is_land = true;
      break;
    }
  }

  rule_list->Clear();
  delete rule_list;
  return is_land;
}

enum SegmentSafetyPointClass {
  SEGMENT_SAFETY_POINT_NO_DATA = 0,
  SEGMENT_SAFETY_POINT_WATER,
  SEGMENT_SAFETY_POINT_LAND,
  SEGMENT_SAFETY_POINT_DRYING
};

enum SegmentSafetyHazardFlags {
  SEGMENT_SAFETY_HAZARD_NONE = 0,
  SEGMENT_SAFETY_HAZARD_LAND = 1 << 0,
  SEGMENT_SAFETY_HAZARD_DRYING = 1 << 1,
  SEGMENT_SAFETY_HAZARD_NO_CHART = 1 << 4,
  SEGMENT_SAFETY_HAZARD_UNKNOWN_CLASS = 1 << 5
};

enum SegmentSafetyRouteBlockFlags {
  SEGMENT_SAFETY_ROUTE_CLEAR = 0,
  SEGMENT_SAFETY_ROUTE_BLOCK_LAND = 1 << 0,
  SEGMENT_SAFETY_ROUTE_BLOCK_DRYING = 1 << 1,
  SEGMENT_SAFETY_ROUTE_BLOCK_TOO_SHALLOW = 1 << 2,
  SEGMENT_SAFETY_ROUTE_BLOCK_UNKNOWN_DEPTH = 1 << 3,
  SEGMENT_SAFETY_ROUTE_BLOCK_NO_CHART = 1 << 4,
  SEGMENT_SAFETY_ROUTE_BLOCK_UNKNOWN_CLASS = 1 << 5,
  SEGMENT_SAFETY_ROUTE_BLOCK_MARGIN = 1 << 6,
  SEGMENT_SAFETY_ROUTE_NEEDS_TILE = 1 << 15
};

const char* SegmentSafetyPrimitiveName(GeoPrim_t primitive) {
  switch (primitive) {
    case GEO_POINT:
      return "point";
    case GEO_LINE:
      return "line";
    case GEO_AREA:
      return "area";
    case GEO_META:
      return "meta";
    case GEO_PRIM:
      return "prim";
    default:
      return "unknown";
  }
}

wxString SegmentSafetyObjectAttr(S57Obj* obj, const char* attr) {
  if (!obj || !attr) return wxString();
  wxString value = obj->GetAttrValueAsString(attr);
  value.Replace("\"", "'");
  value.Replace(";", ",");
  return value;
}

wxString SegmentSafetyRuleSummary(ObjRazRules* rule) {
  if (!rule || !rule->obj) return wxString();

  S57Obj* obj = rule->obj;
  wxString summary =
      wxString::Format("%s/%s", obj->FeatureName,
                       SegmentSafetyPrimitiveName(obj->Primitive_type));
  if (rule->LUP) {
    summary += wxString::Format("/TNAM=%d/DPRI=%c/DISC=%c", rule->LUP->TNAM,
                                rule->LUP->DPRI, rule->LUP->DISC);
    if (!rule->LUP->INST.empty()) {
      wxString inst = rule->LUP->INST.Left(80);
      inst.Replace("\"", "'");
      inst.Replace(";", ",");
      summary += wxString::Format("/INST=%s", inst);
    }
  }

  const char* attrs[] = {"DRVAL1", "DRVAL2", "VALSOU", "VALDCO",
                         "WATLEV", "CATWAT", "CATWRK", "CATOBS",
                         "EXPSOU"};
  for (size_t i = 0; i < WXSIZEOF(attrs); ++i) {
    wxString value = SegmentSafetyObjectAttr(obj, attrs[i]);
    if (!value.empty()) summary += wxString::Format("/%s=%s", attrs[i], value);
  }

  return summary;
}

bool SegmentSafetyParseDouble(wxString value, double* out) {
  value.Trim(true);
  value.Trim(false);
  if (value.empty()) return false;
  double parsed = 0.0;
  if (!value.ToDouble(&parsed)) return false;
  if (out) *out = parsed;
  return true;
}

bool SegmentSafetyRuleDepthMinM(ObjRazRules* rule, double* depth_m) {
  if (!rule || !rule->obj ||
      (strncmp(rule->obj->FeatureName, "DEPARE", 6) &&
       strncmp(rule->obj->FeatureName, "DRGARE", 6)))
    return false;
  return SegmentSafetyParseDouble(rule->obj->GetAttrValueAsString("DRVAL1"),
                                  depth_m);
}

bool SegmentSafetyWaterLevelIs(const wxString& value, int code) {
  wxString normalized = value;
  normalized.Trim(true);
  normalized.Trim(false);
  long parsed = -1;
  if (normalized.ToLong(&parsed)) return parsed == code;
  return normalized.Find(wxString::Format("(%d)", code)) != wxNOT_FOUND;
}

bool SegmentSafetyIsIsolatedDanger(const char* feature_name) {
  return feature_name &&
         (!strncmp(feature_name, "WRECKS", 6) ||
          !strncmp(feature_name, "UWTROC", 6) ||
          !strncmp(feature_name, "OBSTRN", 6));
}

bool SegmentSafetyRuleIsDrying(ObjRazRules* rule) {
  if (!rule || !rule->obj) return false;
  wxString watlev = rule->obj->GetAttrValueAsString("WATLEV");
  return SegmentSafetyWaterLevelIs(watlev, 4) ||
         SegmentSafetyWaterLevelIs(watlev, 5);
}

bool SegmentSafetyRuleIsAlwaysDry(ObjRazRules* rule) {
  if (!rule || !rule->obj) return false;
  return SegmentSafetyWaterLevelIs(
      rule->obj->GetAttrValueAsString("WATLEV"), 2);
}

bool SegmentSafetyRuleDangerDepthM(ObjRazRules* rule, double* depth_m,
                                   bool* unknown_depth) {
  if (unknown_depth) *unknown_depth = false;
  if (!rule || !rule->obj ||
      !SegmentSafetyIsIsolatedDanger(rule->obj->FeatureName))
    return false;
  if (SegmentSafetyParseDouble(rule->obj->GetAttrValueAsString("VALSOU"),
                               depth_m))
    return true;
  if (unknown_depth) *unknown_depth = true;
  return false;
}

int SegmentSafetyPluginAttributeIndex(PI_S57Obj* obj, const char* attr) {
  if (!obj || !attr || !obj->att_array || obj->n_attr <= 0) return -1;
  const char* current = obj->att_array;
  for (int index = 0; index < obj->n_attr; ++index, current += 6) {
    if (!strncmp(current, attr, 6)) return index;
  }
  return -1;
}

wxString SegmentSafetyPluginObjectAttr(PI_S57Obj* obj, const char* attr) {
  const int index = SegmentSafetyPluginAttributeIndex(obj, attr);
  if (index < 0 || !obj->attVal || index >= (int)obj->attVal->GetCount())
    return wxString();
  S57attVal* value = obj->attVal->Item(index);
  if (!value || !value->value) return wxString();
  switch (value->valType) {
    case OGR_STR:
      return wxString(static_cast<const char*>(value->value), wxConvUTF8);
    case OGR_REAL:
      return wxString::Format("%.12g", *static_cast<double*>(value->value));
    case OGR_INT:
      return wxString::Format("%d", *static_cast<int*>(value->value));
    default:
      return wxString();
  }
}

wxString SegmentSafetyPluginObjectSummary(PI_S57Obj* obj) {
  if (!obj) return wxString();
  wxString summary =
      wxString::Format("%s/%s", obj->FeatureName,
                       SegmentSafetyPrimitiveName(
                           static_cast<GeoPrim_t>(obj->Primitive_type)));
  const char* attrs[] = {"DRVAL1", "DRVAL2", "VALSOU", "WATLEV",
                         "CATWRK", "CATOBS", "EXPSOU"};
  for (size_t i = 0; i < WXSIZEOF(attrs); ++i) {
    wxString value = SegmentSafetyPluginObjectAttr(obj, attrs[i]);
    value.Replace("\"", "'");
    value.Replace(";", ",");
    if (!value.empty()) summary += wxString::Format("/%s=%s", attrs[i], value);
  }
  return summary;
}

bool SegmentSafetyPluginObjectIsDrying(PI_S57Obj* obj) {
  if (!obj) return false;
  const wxString watlev = SegmentSafetyPluginObjectAttr(obj, "WATLEV");
  return SegmentSafetyWaterLevelIs(watlev, 4) ||
         SegmentSafetyWaterLevelIs(watlev, 5);
}

bool SegmentSafetyPluginObjectIsAlwaysDry(PI_S57Obj* obj) {
  return obj && SegmentSafetyWaterLevelIs(
                    SegmentSafetyPluginObjectAttr(obj, "WATLEV"), 2);
}

bool SegmentSafetyPluginObjectDepthM(PI_S57Obj* obj, double* depth_m,
                                     wxString* source_attribute,
                                     bool* unknown_danger_depth) {
  if (unknown_danger_depth) *unknown_danger_depth = false;
  if (!obj) return false;
  if (!strncmp(obj->FeatureName, "DEPARE", 6) ||
      !strncmp(obj->FeatureName, "DRGARE", 6)) {
    if (SegmentSafetyParseDouble(
            SegmentSafetyPluginObjectAttr(obj, "DRVAL1"), depth_m)) {
      if (source_attribute)
        *source_attribute = wxString::Format("%s/DRVAL1", obj->FeatureName);
      return true;
    }
    return false;
  }
  if (SegmentSafetyIsIsolatedDanger(obj->FeatureName)) {
    if (SegmentSafetyParseDouble(
            SegmentSafetyPluginObjectAttr(obj, "VALSOU"), depth_m)) {
      if (source_attribute)
        *source_attribute = wxString::Format("%s/VALSOU", obj->FeatureName);
      return true;
    }
    if (unknown_danger_depth) *unknown_danger_depth = true;
  }
  return false;
}

bool IsSupportedSegmentSafetyPluginChart(ChartBase* chart) {
  ChartPlugInWrapper* wrapper = dynamic_cast<ChartPlugInWrapper*>(chart);
  if (!wrapper || chart->GetChartFamily() != CHART_FAMILY_VECTOR) return false;
  PlugInChartBase* plugin_chart = wrapper->GetPlugInChart();
  return dynamic_cast<PlugInChartBaseGL*>(plugin_chart) != NULL ||
         dynamic_cast<PlugInChartBaseExtended*>(plugin_chart) != NULL;
}

s57chart* GetSegmentSafetyChartAtPoint(ChartCanvas* canvas, double lat,
                                       double lon, ViewPort& vp,
                                       PlugInSegmentSafetySource* source) {
  if (!canvas) return NULL;

  wxPoint point;
  if (!canvas->GetCanvasPointPixVP(vp, lat, lon, &point)) return NULL;

  ChartBase* chart = NULL;
  if (canvas->GetQuiltMode() && canvas->m_pQuilt) {
    chart = canvas->m_pQuilt->GetChartAtPix(vp, point);
    if (!chart) chart = canvas->m_pQuilt->GetOverlayChartAtPix(vp, point);
  } else {
    chart = canvas->m_singleChart;
  }

  s57chart* s57 = dynamic_cast<s57chart*>(chart);
  if (s57 && source) {
    *source = IsCm93Chart(chart) ? PI_SEGMENT_SAFETY_SOURCE_CM93
                                 : PI_SEGMENT_SAFETY_SOURCE_VECTOR_CHART;
  }
  return s57;
}

bool ChartSegmentPointSamplesHitLand(double lat1, double lon1, double lat2,
                                     double lon2, double safety_margin_nm,
                                     PlugInSegmentSafetyResult* result,
                                     int* chart_sample_count,
                                     int* total_sample_count) {
  ChartCanvas* canvas = gFrame && gFrame->GetFocusCanvas()
                            ? gFrame->GetFocusCanvas()
                            : (gFrame ? gFrame->GetPrimaryCanvas() : NULL);
  if (!canvas) return false;

  ViewPort vp = canvas->GetVP();
  double bearing = 0.0;
  double dist_nm = 0.0;
  ll_gc_ll_reverse(lat1, lon1, lat2, lon2, &bearing, &dist_nm);

  const int max_samples = 256;
  int samples = wxMax(2, wxMin(max_samples, (int)ceil(dist_nm / 0.1) + 1));
  if (total_sample_count) *total_sample_count = samples;

  for (int i = 0; i < samples; ++i) {
    double sample_dist = samples == 1 ? 0.0 : dist_nm * i / (samples - 1);
    double lat = lat1;
    double lon = lon1;
    if (sample_dist > 0.0)
      ll_gc_ll(lat1, lon1, bearing, sample_dist, &lat, &lon);

    PlugInSegmentSafetySource source = PI_SEGMENT_SAFETY_SOURCE_NONE;
    s57chart* chart =
        GetSegmentSafetyChartAtPoint(canvas, lat, lon, vp, &source);
    if (!chart) continue;

    if (chart_sample_count) ++*chart_sample_count;
    if (GetSegmentSafetySource(result) == PI_SEGMENT_SAFETY_SOURCE_NONE)
      SetSegmentSafetySource(result, source);

    if (ChartPointIsLand(chart, lat, lon, vp)) {
      if (result) {
        SetSegmentSafetyStatus(result, PI_SEGMENT_SAFETY_CROSSES_LAND);
        SetSegmentSafetySource(result, source);
        SetSegmentSafetyMessage(result, "segment intersects chart land area");
      }
      return true;
    }

    if (safety_margin_nm > 0.0) {
      double left_lat, left_lon, right_lat, right_lon;
      ll_gc_ll(lat, lon, SegmentSafetyNormalizeBearing(bearing - 90.0),
               safety_margin_nm, &left_lat, &left_lon);
      ll_gc_ll(lat, lon, SegmentSafetyNormalizeBearing(bearing + 90.0),
               safety_margin_nm, &right_lat, &right_lon);
      if (ChartPointIsLand(chart, left_lat, left_lon, vp) ||
          ChartPointIsLand(chart, right_lat, right_lon, vp)) {
        if (result) {
          SetSegmentSafetyStatus(result, PI_SEGMENT_SAFETY_WITHIN_LAND_MARGIN);
          SetSegmentSafetySource(result, source);
          SetSegmentSafetyMessage(result,
                                  "segment is within chart land safety margin");
        }
        return true;
      }
    }
  }

  return false;
}

struct SegmentSafetyBBox {
  double min_lat;
  double max_lat;
  double min_lon;
  double max_lon;
};

struct CachedLandRing {
  std::vector<wxPoint2DDouble> points;
  SegmentSafetyBBox bbox;
};

struct CachedChartLandGeometry {
  bool loaded;
  PlugInSegmentSafetySource source;
  std::string cache_key;
  wxString chart_path;
  std::vector<CachedLandRing> rings;

  CachedChartLandGeometry()
      : loaded(false), source(PI_SEGMENT_SAFETY_SOURCE_NONE) {}
};

std::map<std::string, CachedChartLandGeometry> s_segment_safety_land_cache;

enum SegmentSafetySnapshotDecision {
  SEGMENT_SAFETY_SNAPSHOT_UNKNOWN = 0,
  SEGMENT_SAFETY_SNAPSHOT_SAFE
};

struct SegmentSafetySnapshotChart {
  int db_index;
  int native_scale;
  time_t edition_date;
  time_t file_time;
  bool vector_supported;
  PlugInSegmentSafetySource source;
  wxString chart_path;
  SegmentSafetyBBox bbox;
  std::vector<CachedLandRing> coverage;
  std::vector<CachedLandRing> no_coverage;
  std::vector<CachedLandRing> hazards;

  SegmentSafetySnapshotChart()
      : db_index(-1),
        native_scale(INT_MAX),
        edition_date(0),
        file_time(0),
        vector_supported(false),
        source(PI_SEGMENT_SAFETY_SOURCE_NONE) {}
};

struct SegmentSafetyHazardSnapshot {
  wxString chart_identity;
  int group_index;
  SegmentSafetyBBox area;
  bool fast_path_enabled;
  bool shadow_compare;
  std::vector<SegmentSafetySnapshotChart> charts;
  std::set<std::pair<long, long> > certified_safe_large_cells;
  std::set<std::pair<long, long> > certified_safe_fine_cells;
  long coverage_rings;
  long hazard_rings;

  SegmentSafetyHazardSnapshot()
      : group_index(0),
        fast_path_enabled(false),
        shadow_compare(true),
        coverage_rings(0),
        hazard_rings(0) {}
};

std::shared_ptr<const SegmentSafetyHazardSnapshot>
    s_segment_safety_hazard_snapshot;
long s_segment_safety_snapshot_queries = 0;
long s_segment_safety_snapshot_safe = 0;
long s_segment_safety_snapshot_unknown = 0;
long s_segment_safety_snapshot_shadow_disagreements = 0;

struct CachedPointSafetyClassification {
  SegmentSafetyPointClass point_class;
  PlugInSegmentSafetySource source;
  int chart_db_index;
  int chart_scale;
  bool has_depth;
  double min_depth_m;
  bool has_drying;
  char chart_path[256];
  char hit_object[128];
  char depth_source_object[128];
  char depth_source_attribute[32];

  CachedPointSafetyClassification()
      : point_class(SEGMENT_SAFETY_POINT_NO_DATA),
        source(PI_SEGMENT_SAFETY_SOURCE_NONE),
        chart_db_index(-1),
        chart_scale(-1),
        has_depth(false),
        min_depth_m(0.0),
        has_drying(false) {
    chart_path[0] = '\0';
    hit_object[0] = '\0';
    depth_source_object[0] = '\0';
    depth_source_attribute[0] = '\0';
  }
};

std::map<std::string, CachedPointSafetyClassification>
    s_segment_safety_point_cache;
const size_t kMaxSegmentSafetyPointCacheEntries = 250000;

const double kSegmentSafetyGridTileDegrees = 0.05;
const double kSegmentSafetyGridResolutionDegrees = 0.00125;
const int kSegmentSafetyCoarseRouteMaskFactor = 4;
const int kSegmentSafetyPersistentCacheVersion = 3;
const int kSegmentSafetyPersistentBaseTileVersion = 4;
const int kSegmentSafetyRouteMaskAlgorithmVersion = 5;
const size_t kMaxSegmentSafetyGridTiles = 4096;
// Keep the hot in-memory working set bounded, but retain a larger certified
// disk-backed history so repeated routes do not rebuild recently used waters.
const size_t kMaxSegmentSafetyPersistentGridTiles = 8192;
// Each entry certifies a 0.2-degree coarse cell for one exact margin/depth
// policy.  Keep the expanding regional proof cache bounded on disk.
const size_t kMaxSegmentSafetyPersistentCertifiedCells = 32768;
const long kSegmentSafetyPersistentCheckpointTiles = 64;
const size_t kMaxSegmentSafetySegmentCacheEntries = 100000;
long s_segment_safety_grid_cache_evictions = 0;

struct CachedPointSafetyGridTile {
  int group_index;
  long lat_tile;
  long lon_tile;
  double min_lat;
  double min_lon;
  double resolution;
  int rows;
  int cols;
  int land_count;
  int water_count;
  int drying_count;
  int unknown_count;
  bool built;
  int chart_db_index;
  int chart_scale;
  PlugInSegmentSafetySource source;
  char chart_path[256];
  char dependency_identity[80];
  std::vector<unsigned char> classes;
  std::vector<uint16_t> hazard_flags;
  std::vector<unsigned char> has_depth;
  std::vector<float> min_depth_m;
  std::vector<unsigned char> has_drying;
  uint32_t hazard_summary_flags;
  bool persistent_loaded;
  bool depth_complete;
  bool persistent_cache_allowed;

  CachedPointSafetyGridTile()
      : group_index(0),
        lat_tile(0),
        lon_tile(0),
        min_lat(0.0),
        min_lon(0.0),
        resolution(kSegmentSafetyGridResolutionDegrees),
        rows(0),
        cols(0),
        land_count(0),
        water_count(0),
        drying_count(0),
        unknown_count(0),
        built(false),
        chart_db_index(-1),
        chart_scale(-1),
        source(PI_SEGMENT_SAFETY_SOURCE_NONE),
        hazard_summary_flags(SEGMENT_SAFETY_HAZARD_NONE),
        persistent_loaded(false),
        depth_complete(true),
        persistent_cache_allowed(false) {
    chart_path[0] = '\0';
    dependency_identity[0] = '\0';
  }
};

std::map<std::string, CachedPointSafetyGridTile> s_segment_safety_grid_cache;
std::set<std::string> s_segment_safety_pinned_grid_keys;
std::map<std::string, CachedPointSafetyGridTile>
    s_segment_safety_persistent_base_tile_cache;
PlugInSegmentSafetyTileCacheCallbacks
    s_segment_safety_external_tile_cache = {};

uint16_t SegmentSafetyPointHazardFlags(SegmentSafetyPointClass point_class) {
  switch (point_class) {
    case SEGMENT_SAFETY_POINT_LAND:
      return SEGMENT_SAFETY_HAZARD_LAND;
    case SEGMENT_SAFETY_POINT_DRYING:
      return SEGMENT_SAFETY_HAZARD_DRYING;
    case SEGMENT_SAFETY_POINT_WATER:
      return SEGMENT_SAFETY_HAZARD_NONE;
    case SEGMENT_SAFETY_POINT_NO_DATA:
    default:
      return SEGMENT_SAFETY_HAZARD_NO_CHART;
  }
}

struct CachedSegmentSafetyRouteMaskTile {
  int group_index;
  long lat_tile;
  long lon_tile;
  double min_lat;
  double min_lon;
  double resolution;
  int rows;
  int cols;
  bool check_depth;
  double minimum_depth_m;
  double safety_margin_nm;
  int margin_cells;
  bool built;
  PlugInSegmentSafetySource source;
  int chart_db_index;
  int chart_scale;
  char chart_path[256];
  std::vector<uint16_t> block_flags;
  uint32_t block_summary_flags;
  int clear_count;
  int land_count;
  int drying_count;
  int shallow_count;
  int unknown_depth_count;
  int no_chart_count;
  int margin_count;
  bool authoritative_fine;
  bool persistent_certified_safe;
  bool uses_plugin_vector;
  bool persistent_cache_allowed;

  CachedSegmentSafetyRouteMaskTile()
      : group_index(0),
        lat_tile(0),
        lon_tile(0),
        min_lat(0.0),
        min_lon(0.0),
        resolution(kSegmentSafetyGridResolutionDegrees),
        rows(0),
        cols(0),
        check_depth(false),
        minimum_depth_m(0.0),
        safety_margin_nm(0.0),
        margin_cells(0),
        built(false),
        source(PI_SEGMENT_SAFETY_SOURCE_NONE),
        chart_db_index(-1),
        chart_scale(-1),
        block_summary_flags(SEGMENT_SAFETY_ROUTE_CLEAR),
        clear_count(0),
        land_count(0),
        drying_count(0),
        shallow_count(0),
        unknown_depth_count(0),
        no_chart_count(0),
        margin_count(0),
        authoritative_fine(false),
        persistent_certified_safe(false),
        uses_plugin_vector(false),
        persistent_cache_allowed(false) {
    chart_path[0] = '\0';
  }
};

std::map<std::string, CachedSegmentSafetyRouteMaskTile>
    s_segment_safety_route_mask_cache;
std::set<std::string> s_segment_safety_pinned_route_mask_keys;

struct SegmentSafetyRouteMaskRequest {
  std::string key;
  int group_index;
  long lat_tile;
  long lon_tile;
  double safety_margin_nm;
  bool check_depth;
  double minimum_depth_m;
  bool force_authoritative_fine;

  SegmentSafetyRouteMaskRequest()
      : group_index(0),
        lat_tile(0),
        lon_tile(0),
        safety_margin_nm(0.0),
        check_depth(false),
        minimum_depth_m(0.0),
        force_authoritative_fine(false) {}
};

std::map<std::string, SegmentSafetyRouteMaskRequest>
    s_segment_safety_pending_route_mask_requests;
std::set<std::string> s_segment_safety_inflight_route_mask_requests;

enum SegmentSafetyCoarseRouteMaskState {
  SEGMENT_SAFETY_COARSE_CERTIFIED_SAFE = 0,
  SEGMENT_SAFETY_COARSE_MIXED,
  SEGMENT_SAFETY_COARSE_NO_CHART,
  SEGMENT_SAFETY_COARSE_DEPTH_UNPROVEN,
  SEGMENT_SAFETY_COARSE_MISSING
};

struct CachedSegmentSafetyCoarseRouteMaskCell {
  int group_index;
  long lat_cell;
  long lon_cell;
  double min_lat;
  double min_lon;
  double degrees;
  bool check_depth;
  double minimum_depth_m;
  double safety_margin_nm;
  SegmentSafetyCoarseRouteMaskState state;
  uint32_t block_summary_flags;
  int fine_tiles_checked;
  int fine_tiles_clear;
  int fine_tiles_mixed;
  PlugInSegmentSafetySource source;
  bool persistent_cache_allowed;

  CachedSegmentSafetyCoarseRouteMaskCell()
      : group_index(0),
        lat_cell(0),
        lon_cell(0),
        min_lat(0.0),
        min_lon(0.0),
        degrees(kSegmentSafetyGridTileDegrees *
                kSegmentSafetyCoarseRouteMaskFactor),
        check_depth(false),
        minimum_depth_m(0.0),
        safety_margin_nm(0.0),
        state(SEGMENT_SAFETY_COARSE_MISSING),
        block_summary_flags(SEGMENT_SAFETY_ROUTE_NEEDS_TILE),
        fine_tiles_checked(0),
        fine_tiles_clear(0),
        fine_tiles_mixed(0),
        source(PI_SEGMENT_SAFETY_SOURCE_NONE),
        persistent_cache_allowed(true) {}
};

std::map<std::string, CachedSegmentSafetyCoarseRouteMaskCell>
    s_segment_safety_coarse_route_mask_cache;

std::map<std::string, CachedSegmentSafetyCoarseRouteMaskCell>
    s_segment_safety_persistent_certified_safe_cache;
bool s_segment_safety_persistent_cache_enabled = false;
bool s_segment_safety_persistent_cache_loaded = false;
bool s_segment_safety_persistent_cache_dirty = false;
bool s_segment_safety_persistent_base_tiles_loaded = false;
bool s_segment_safety_persistent_base_tiles_dirty = false;
wxString s_segment_safety_chart_identity;
wxString s_segment_safety_chart_catalog_identity;
long s_segment_safety_persistent_entries_loaded = 0;
long s_segment_safety_persistent_entries_saved = 0;
long s_segment_safety_persistent_entries_used = 0;
long s_segment_safety_persistent_entries_ignored = 0;
long s_segment_safety_persistent_stale_ignored = 0;
long s_segment_safety_persistent_malformed_ignored = 0;
long s_segment_safety_persistent_entries_stored = 0;
long s_segment_safety_persistent_base_tiles_loaded_count = 0;
long s_segment_safety_persistent_base_tiles_saved = 0;
long s_segment_safety_persistent_base_tiles_used = 0;
long s_segment_safety_persistent_base_tiles_ignored = 0;
long s_segment_safety_persistent_tiles_since_checkpoint = 0;

int SegmentSafetyCurrentGroupIndex();
std::string SegmentSafetyGridTileKeyForIndices(long lat_tile, long lon_tile);

std::string SegmentSafetyRouteMaskKey(long lat_tile, long lon_tile,
                                      double safety_margin_nm, bool check_depth,
                                      double minimum_depth_m) {
  long margin_mm = lround(wxMax(0.0, safety_margin_nm) * 1000.0);
  long depth_cm = check_depth ? lround(wxMax(0.0, minimum_depth_m) * 100.0) : 0;
  char buf[160];
  snprintf(buf, sizeof(buf), "%d:%ld:%ld:r%.8f:m%ld:d%d:%ld",
           SegmentSafetyCurrentGroupIndex(), lat_tile, lon_tile,
           kSegmentSafetyGridResolutionDegrees, margin_mm, check_depth ? 1 : 0,
           depth_cm);
  return std::string(buf);
}

std::string SegmentSafetyCoarseRouteMaskKey(long lat_cell, long lon_cell,
                                            double safety_margin_nm,
                                            bool check_depth,
                                            double minimum_depth_m) {
  long margin_mm = lround(wxMax(0.0, safety_margin_nm) * 1000.0);
  long depth_cm = check_depth ? lround(wxMax(0.0, minimum_depth_m) * 100.0) : 0;
  char buf[160];
  snprintf(buf, sizeof(buf), "%d:%ld:%ld:cr%.8f:f%d:m%ld:d%d:%ld",
           SegmentSafetyCurrentGroupIndex(), lat_cell, lon_cell,
           kSegmentSafetyGridTileDegrees * kSegmentSafetyCoarseRouteMaskFactor,
           kSegmentSafetyCoarseRouteMaskFactor, margin_mm, check_depth ? 1 : 0,
           depth_cm);
  return std::string(buf);
}

struct CachedSegmentSafetyResult {
  int status;
  int source;
  int diagnostic_reason;
  int chart_db_index;
  int chart_scale;
  int hit_sample_index;
  int hit_sample_count;
  double hit_sample_lat;
  double hit_sample_lon;
  int has_depth;
  double min_depth_m;
  double required_depth_m;
  double hit_depth_m;
  int has_drying;
  char message[256];
  char chart_path[256];
  char hit_object[128];
  char depth_source_object[128];
  char depth_source_attribute[32];

  CachedSegmentSafetyResult()
      : status(PI_SEGMENT_SAFETY_ERROR),
        source(PI_SEGMENT_SAFETY_SOURCE_NONE),
        diagnostic_reason(PI_SEGMENT_SAFETY_DIAG_NONE),
        chart_db_index(-1),
        chart_scale(-1),
        hit_sample_index(-1),
        hit_sample_count(0),
        hit_sample_lat(0.0),
        hit_sample_lon(0.0),
        has_depth(0),
        min_depth_m(0.0),
        required_depth_m(0.0),
        hit_depth_m(0.0),
        has_drying(0) {
    message[0] = '\0';
    chart_path[0] = '\0';
    hit_object[0] = '\0';
    depth_source_object[0] = '\0';
    depth_source_attribute[0] = '\0';
  }
};

std::map<std::string, CachedSegmentSafetyResult> s_segment_safety_segment_cache;

long s_segment_safety_chart_hit_logs = 0;
const long kMaxSegmentSafetyChartHitLogs = 20;
wxMutex s_segment_safety_cache_mutex;
long s_segment_safety_worker_tile_miss_logs = 0;
const long kMaxSegmentSafetyWorkerTileMissLogs = 20;
long s_segment_safety_query_logs = 0;
const long kMaxSegmentSafetyQueryLogs = 160;

int SegmentSafetyMarginTileRadius(double safety_margin_nm, double max_abs_lat) {
  if (safety_margin_nm <= 0.0) return 0;
  double limited_lat = wxMin(89.9, max_abs_lat);
  double cos_lat = fabs(cos(limited_lat * M_PI / 180.0));
  double tile_width_nm =
      kSegmentSafetyGridTileDegrees * 60.0 * wxMax(0.001, cos_lat);
  double tile_height_nm = kSegmentSafetyGridTileDegrees * 60.0;
  return (int)ceil(safety_margin_nm /
                   wxMax(0.001, wxMin(tile_width_nm, tile_height_nm)));
}

void PinSegmentSafetyRouteMaskTiles(
    const std::set<std::pair<long, long> >& tiles, double safety_margin_nm,
    bool check_depth, double minimum_depth_m) {
  if (tiles.empty()) return;

  double max_abs_lat = 0.0;
  for (std::set<std::pair<long, long> >::const_iterator it = tiles.begin();
       it != tiles.end(); ++it) {
    max_abs_lat =
        wxMax(max_abs_lat, fabs(it->first * kSegmentSafetyGridTileDegrees));
  }
  int margin_radius =
      SegmentSafetyMarginTileRadius(safety_margin_nm, max_abs_lat);

  wxMutexLocker lock(s_segment_safety_cache_mutex);
  for (std::set<std::pair<long, long> >::const_iterator it = tiles.begin();
       it != tiles.end(); ++it) {
    s_segment_safety_pinned_route_mask_keys.insert(SegmentSafetyRouteMaskKey(
        it->first, it->second, safety_margin_nm, check_depth, minimum_depth_m));
    for (int lat_offset = -margin_radius; lat_offset <= margin_radius;
         ++lat_offset) {
      for (int lon_offset = -margin_radius; lon_offset <= margin_radius;
           ++lon_offset) {
        s_segment_safety_pinned_grid_keys.insert(
            SegmentSafetyGridTileKeyForIndices(it->first + lat_offset,
                                               it->second + lon_offset));
      }
    }
  }
}

void PinSegmentSafetyRouteMaskEnvelope(long min_lat_tile, long max_lat_tile,
                                       long min_lon_tile, long max_lon_tile,
                                       double safety_margin_nm,
                                       bool check_depth,
                                       double minimum_depth_m) {
  std::set<std::pair<long, long> > tiles;
  for (long lat_tile = min_lat_tile; lat_tile <= max_lat_tile; ++lat_tile)
    for (long lon_tile = min_lon_tile; lon_tile <= max_lon_tile; ++lon_tile)
      tiles.insert(std::make_pair(lat_tile, lon_tile));
  PinSegmentSafetyRouteMaskTiles(tiles, safety_margin_nm, check_depth,
                                 minimum_depth_m);
}

struct SegmentSafetyCoreStats {
  int chart_stack_entries;
  int candidate_chart_count;
  int raster_chart_count;
  int unsupported_chart_count;
  int s57_chart_count;
  int land_ring_count;
  int bbox_ring_tests;
  int edge_tests;
  int cache_build_ms;
  int chart_select_ms;
  int geometry_check_ms;
  int point_cache_hits;
  int point_cache_misses;
  int grid_cache_hits;
  int grid_cache_misses;
  int grid_build_ms;
  int grid_cells_total;
  int grid_cells_land;
  int grid_cells_water;
  int grid_cells_drying;
  int grid_cells_unknown;
  int grid_lookups;
  int grid_lookup_ms;
  int segment_sample_count;
  int water_tile_shortcuts;
  int segment_cache_hits;
  int segment_cache_misses;
  int segment_cache_stores;
  int unexpected_tile_builds;
  int coarse_cells_checked;
  int coarse_certified_safe_hits;
  int coarse_mixed_fallbacks;
  int coarse_unknown_fallbacks;
  int coarse_no_chart;
  int coarse_depth_unproven;
  int coarse_missing;
  int fine_tiles_avoided;
  int coarse_build_ms;
  int unexpected_lat_tile;
  int unexpected_lon_tile;
  double unexpected_tile_min_lat;
  double unexpected_tile_min_lon;
  bool no_chart_database;
  bool chart_load_failed;
  bool zero_land_geometry;

  SegmentSafetyCoreStats()
      : chart_stack_entries(0),
        candidate_chart_count(0),
        raster_chart_count(0),
        unsupported_chart_count(0),
        s57_chart_count(0),
        land_ring_count(0),
        bbox_ring_tests(0),
        edge_tests(0),
        cache_build_ms(0),
        chart_select_ms(0),
        geometry_check_ms(0),
        point_cache_hits(0),
        point_cache_misses(0),
        grid_cache_hits(0),
        grid_cache_misses(0),
        grid_build_ms(0),
        grid_cells_total(0),
        grid_cells_land(0),
        grid_cells_water(0),
        grid_cells_drying(0),
        grid_cells_unknown(0),
        grid_lookups(0),
        grid_lookup_ms(0),
        segment_sample_count(0),
        water_tile_shortcuts(0),
        segment_cache_hits(0),
        segment_cache_misses(0),
        segment_cache_stores(0),
        unexpected_tile_builds(0),
        coarse_cells_checked(0),
        coarse_certified_safe_hits(0),
        coarse_mixed_fallbacks(0),
        coarse_unknown_fallbacks(0),
        coarse_no_chart(0),
        coarse_depth_unproven(0),
        coarse_missing(0),
        fine_tiles_avoided(0),
        coarse_build_ms(0),
        unexpected_lat_tile(0),
        unexpected_lon_tile(0),
        unexpected_tile_min_lat(0.0),
        unexpected_tile_min_lon(0.0),
        no_chart_database(false),
        chart_load_failed(false),
        zero_land_geometry(false) {}
};

void ApplySegmentSafetyStats(PlugInSegmentSafetyResult* result,
                             const SegmentSafetyCoreStats& stats) {
  SetSegmentSafetyDiagnosticInt(
      result, offsetof(PlugInSegmentSafetyResult, chart_stack_entries),
      stats.chart_stack_entries);
  SetSegmentSafetyDiagnosticInt(
      result, offsetof(PlugInSegmentSafetyResult, candidate_chart_count),
      stats.candidate_chart_count);
  SetSegmentSafetyDiagnosticInt(
      result, offsetof(PlugInSegmentSafetyResult, raster_chart_count),
      stats.raster_chart_count);
  SetSegmentSafetyDiagnosticInt(
      result, offsetof(PlugInSegmentSafetyResult, unsupported_chart_count),
      stats.unsupported_chart_count);
  SetSegmentSafetyDiagnosticInt(
      result, offsetof(PlugInSegmentSafetyResult, s57_chart_count),
      stats.s57_chart_count);
  SetSegmentSafetyDiagnosticInt(
      result, offsetof(PlugInSegmentSafetyResult, land_ring_count),
      stats.land_ring_count);
  SetSegmentSafetyDiagnosticInt(
      result, offsetof(PlugInSegmentSafetyResult, bbox_ring_tests),
      stats.bbox_ring_tests);
  SetSegmentSafetyDiagnosticInt(result,
                                offsetof(PlugInSegmentSafetyResult, edge_tests),
                                stats.edge_tests);
  SetSegmentSafetyDiagnosticInt(
      result, offsetof(PlugInSegmentSafetyResult, cache_build_ms),
      stats.cache_build_ms);
  SetSegmentSafetyDiagnosticInt(
      result, offsetof(PlugInSegmentSafetyResult, chart_select_ms),
      stats.chart_select_ms);
  SetSegmentSafetyDiagnosticInt(
      result, offsetof(PlugInSegmentSafetyResult, geometry_check_ms),
      stats.geometry_check_ms);
  SetSegmentSafetyDiagnosticInt(
      result, offsetof(PlugInSegmentSafetyResult, point_cache_hits),
      stats.point_cache_hits);
  SetSegmentSafetyDiagnosticInt(
      result, offsetof(PlugInSegmentSafetyResult, point_cache_misses),
      stats.point_cache_misses);
  SetSegmentSafetyDiagnosticInt(
      result, offsetof(PlugInSegmentSafetyResult, grid_cache_hits),
      stats.grid_cache_hits);
  SetSegmentSafetyDiagnosticInt(
      result, offsetof(PlugInSegmentSafetyResult, grid_cache_misses),
      stats.grid_cache_misses);
  SetSegmentSafetyDiagnosticInt(
      result, offsetof(PlugInSegmentSafetyResult, grid_build_ms),
      stats.grid_build_ms);
  SetSegmentSafetyDiagnosticInt(
      result, offsetof(PlugInSegmentSafetyResult, grid_cells_total),
      stats.grid_cells_total);
  SetSegmentSafetyDiagnosticInt(
      result, offsetof(PlugInSegmentSafetyResult, grid_cells_land),
      stats.grid_cells_land);
  SetSegmentSafetyDiagnosticInt(
      result, offsetof(PlugInSegmentSafetyResult, grid_cells_water),
      stats.grid_cells_water);
  SetSegmentSafetyDiagnosticInt(
      result, offsetof(PlugInSegmentSafetyResult, grid_cells_drying),
      stats.grid_cells_drying);
  SetSegmentSafetyDiagnosticInt(
      result, offsetof(PlugInSegmentSafetyResult, grid_cells_unknown),
      stats.grid_cells_unknown);
  SetSegmentSafetyDiagnosticInt(
      result, offsetof(PlugInSegmentSafetyResult, grid_lookups),
      stats.grid_lookups);
  SetSegmentSafetyDiagnosticInt(
      result, offsetof(PlugInSegmentSafetyResult, grid_lookup_ms),
      stats.grid_lookup_ms);
  SetSegmentSafetyDiagnosticInt(
      result, offsetof(PlugInSegmentSafetyResult, segment_sample_count),
      stats.segment_sample_count);
  SetSegmentSafetyDiagnosticInt(
      result, offsetof(PlugInSegmentSafetyResult, water_tile_shortcuts),
      stats.water_tile_shortcuts);
  SetSegmentSafetyDiagnosticInt(
      result, offsetof(PlugInSegmentSafetyResult, segment_cache_hits),
      stats.segment_cache_hits);
  SetSegmentSafetyDiagnosticInt(
      result, offsetof(PlugInSegmentSafetyResult, segment_cache_misses),
      stats.segment_cache_misses);
  SetSegmentSafetyDiagnosticInt(
      result, offsetof(PlugInSegmentSafetyResult, segment_cache_stores),
      stats.segment_cache_stores);
  size_t grid_cache_size = 0;
  {
    wxMutexLocker lock(s_segment_safety_cache_mutex);
    grid_cache_size = s_segment_safety_grid_cache.size();
  }
  SetSegmentSafetyDiagnosticInt(
      result, offsetof(PlugInSegmentSafetyResult, grid_cache_size),
      (int)grid_cache_size);
  SetSegmentSafetyDiagnosticInt(
      result, offsetof(PlugInSegmentSafetyResult, grid_cache_evictions),
      (int)s_segment_safety_grid_cache_evictions);
  SetSegmentSafetyDiagnosticInt(
      result, offsetof(PlugInSegmentSafetyResult, unexpected_tile_builds),
      stats.unexpected_tile_builds);
  SetSegmentSafetyDiagnosticInt(
      result, offsetof(PlugInSegmentSafetyResult, unexpected_lat_tile),
      stats.unexpected_lat_tile);
  SetSegmentSafetyDiagnosticInt(
      result, offsetof(PlugInSegmentSafetyResult, unexpected_lon_tile),
      stats.unexpected_lon_tile);
  if (SegmentSafetyResultHas(
          result, offsetof(PlugInSegmentSafetyResult, unexpected_tile_min_lon),
          sizeof(result->unexpected_tile_min_lon))) {
    result->unexpected_tile_min_lat = stats.unexpected_tile_min_lat;
    result->unexpected_tile_min_lon = stats.unexpected_tile_min_lon;
  }
}

PlugInSegmentSafetyDiagnosticReason SegmentSafetyUnavailableReason(
    const SegmentSafetyCoreStats& stats) {
  if (stats.no_chart_database) return PI_SEGMENT_SAFETY_DIAG_NO_CHART_DATABASE;
  if (stats.chart_load_failed) return PI_SEGMENT_SAFETY_DIAG_CHART_LOAD_FAILED;
  if (stats.candidate_chart_count == 0) {
    if (stats.raster_chart_count > 0 && stats.unsupported_chart_count == 0)
      return PI_SEGMENT_SAFETY_DIAG_RASTER_ONLY;
    if (stats.unsupported_chart_count > 0 && stats.raster_chart_count == 0)
      return PI_SEGMENT_SAFETY_DIAG_UNSUPPORTED_CHART_TYPE;
    return PI_SEGMENT_SAFETY_DIAG_NO_CANDIDATE_CHART;
  }
  if (stats.zero_land_geometry)
    return PI_SEGMENT_SAFETY_DIAG_NO_LANDARE_GEOMETRY;
  return PI_SEGMENT_SAFETY_DIAG_NO_CANDIDATE_CHART;
}

const char* SegmentSafetyUnavailableMessage(
    PlugInSegmentSafetyDiagnosticReason reason) {
  switch (reason) {
    case PI_SEGMENT_SAFETY_DIAG_NO_CHART_DATABASE:
      return "no chart database available for chart land checks";
    case PI_SEGMENT_SAFETY_DIAG_NO_CANDIDATE_CHART:
      return "no candidate vector chart found for segment";
    case PI_SEGMENT_SAFETY_DIAG_RASTER_ONLY:
      return "only raster chart coverage found for segment";
    case PI_SEGMENT_SAFETY_DIAG_UNSUPPORTED_CHART_TYPE:
      return "only unsupported chart types found for segment";
    case PI_SEGMENT_SAFETY_DIAG_CHART_LOAD_FAILED:
      return "candidate chart could not be loaded for segment safety";
    case PI_SEGMENT_SAFETY_DIAG_NO_LANDARE_GEOMETRY:
      return "candidate vector chart has no LNDARE land geometry";
    default:
      return "chart land geometry unavailable for segment";
  }
}

double SegmentSafetyDegToRad(double degrees) {
  return degrees * 3.14159265358979323846 / 180.0;
}

double SegmentSafetyCross(const wxPoint2DDouble& a, const wxPoint2DDouble& b,
                          const wxPoint2DDouble& c) {
  return (b.m_x - a.m_x) * (c.m_y - a.m_y) - (b.m_y - a.m_y) * (c.m_x - a.m_x);
}

bool SegmentSafetyBBoxIntersects(const SegmentSafetyBBox& a,
                                 const SegmentSafetyBBox& b) {
  return !(a.max_lat < b.min_lat || a.min_lat > b.max_lat ||
           a.max_lon < b.min_lon || a.min_lon > b.max_lon);
}

SegmentSafetyBBox SegmentSafetyRingBBox(
    const std::vector<wxPoint2DDouble>& points) {
  SegmentSafetyBBox box;
  box.min_lat = box.max_lat = points.empty() ? 0.0 : points[0].m_y;
  box.min_lon = box.max_lon = points.empty() ? 0.0 : points[0].m_x;
  for (size_t i = 1; i < points.size(); ++i) {
    box.min_lat = wxMin(box.min_lat, points[i].m_y);
    box.max_lat = wxMax(box.max_lat, points[i].m_y);
    box.min_lon = wxMin(box.min_lon, points[i].m_x);
    box.max_lon = wxMax(box.max_lon, points[i].m_x);
  }
  return box;
}

SegmentSafetyBBox SegmentSafetySegmentBBox(double lat1, double lon1,
                                           double lat2, double lon2,
                                           double margin_nm) {
  double margin_lat = margin_nm / 60.0;
  double mid_lat = (lat1 + lat2) / 2.0;
  double cos_lat = wxMax(0.1, fabs(cos(SegmentSafetyDegToRad(mid_lat))));
  double margin_lon = margin_nm / (60.0 * cos_lat);

  SegmentSafetyBBox box;
  box.min_lat = wxMin(lat1, lat2) - margin_lat;
  box.max_lat = wxMax(lat1, lat2) + margin_lat;
  box.min_lon = wxMin(lon1, lon2) - margin_lon;
  box.max_lon = wxMax(lon1, lon2) + margin_lon;
  return box;
}

bool SegmentSafetyPointInRing(double lat, double lon,
                              const std::vector<wxPoint2DDouble>& ring) {
  bool inside = false;
  size_t count = ring.size();
  if (count < 3) return false;

  for (size_t i = 0, j = count - 1; i < count; j = i++) {
    double xi = ring[i].m_x, yi = ring[i].m_y;
    double xj = ring[j].m_x, yj = ring[j].m_y;
    bool intersect = ((yi > lat) != (yj > lat)) &&
                     (lon < (xj - xi) * (lat - yi) / (yj - yi) + xi);
    if (intersect) inside = !inside;
  }
  return inside;
}

bool SegmentSafetyOnSegment(const wxPoint2DDouble& a, const wxPoint2DDouble& b,
                            const wxPoint2DDouble& p) {
  const double eps = 1e-10;
  return fabs(SegmentSafetyCross(a, b, p)) < eps &&
         p.m_x >= wxMin(a.m_x, b.m_x) - eps &&
         p.m_x <= wxMax(a.m_x, b.m_x) + eps &&
         p.m_y >= wxMin(a.m_y, b.m_y) - eps &&
         p.m_y <= wxMax(a.m_y, b.m_y) + eps;
}

bool SegmentSafetySegmentsIntersect(const wxPoint2DDouble& a,
                                    const wxPoint2DDouble& b,
                                    const wxPoint2DDouble& c,
                                    const wxPoint2DDouble& d) {
  double c1 = SegmentSafetyCross(a, b, c);
  double c2 = SegmentSafetyCross(a, b, d);
  double c3 = SegmentSafetyCross(c, d, a);
  double c4 = SegmentSafetyCross(c, d, b);

  if (((c1 > 0 && c2 < 0) || (c1 < 0 && c2 > 0)) &&
      ((c3 > 0 && c4 < 0) || (c3 < 0 && c4 > 0)))
    return true;

  return SegmentSafetyOnSegment(a, b, c) || SegmentSafetyOnSegment(a, b, d) ||
         SegmentSafetyOnSegment(c, d, a) || SegmentSafetyOnSegment(c, d, b);
}

double SegmentSafetyPointSegmentDistanceNm(const wxPoint2DDouble& p,
                                           const wxPoint2DDouble& a,
                                           const wxPoint2DDouble& b,
                                           double mean_lat) {
  double cos_lat = wxMax(0.1, fabs(cos(SegmentSafetyDegToRad(mean_lat))));
  double px = p.m_x * 60.0 * cos_lat;
  double py = p.m_y * 60.0;
  double ax = a.m_x * 60.0 * cos_lat;
  double ay = a.m_y * 60.0;
  double bx = b.m_x * 60.0 * cos_lat;
  double by = b.m_y * 60.0;

  double dx = bx - ax;
  double dy = by - ay;
  double denom = dx * dx + dy * dy;
  double t = denom > 0.0 ? ((px - ax) * dx + (py - ay) * dy) / denom : 0.0;
  t = wxMax(0.0, wxMin(1.0, t));
  double cx = ax + t * dx;
  double cy = ay + t * dy;
  return sqrt((px - cx) * (px - cx) + (py - cy) * (py - cy));
}

double SegmentSafetySegmentDistanceNm(const wxPoint2DDouble& a,
                                      const wxPoint2DDouble& b,
                                      const wxPoint2DDouble& c,
                                      const wxPoint2DDouble& d) {
  if (SegmentSafetySegmentsIntersect(a, b, c, d)) return 0.0;
  double mean_lat = (a.m_y + b.m_y + c.m_y + d.m_y) / 4.0;
  return wxMin(wxMin(SegmentSafetyPointSegmentDistanceNm(a, c, d, mean_lat),
                     SegmentSafetyPointSegmentDistanceNm(b, c, d, mean_lat)),
               wxMin(SegmentSafetyPointSegmentDistanceNm(c, a, b, mean_lat),
                     SegmentSafetyPointSegmentDistanceNm(d, a, b, mean_lat)));
}

bool SegmentSafetyPointInsideBBox(const wxPoint2DDouble& point,
                                  const SegmentSafetyBBox& box) {
  return point.m_y >= box.min_lat && point.m_y <= box.max_lat &&
         point.m_x >= box.min_lon && point.m_x <= box.max_lon;
}

bool SegmentSafetyRingContainsBBox(const CachedLandRing& ring,
                                   const SegmentSafetyBBox& box) {
  const wxPoint2DDouble corners[] = {wxPoint2DDouble(box.min_lon, box.min_lat),
                                     wxPoint2DDouble(box.max_lon, box.min_lat),
                                     wxPoint2DDouble(box.max_lon, box.max_lat),
                                     wxPoint2DDouble(box.min_lon, box.max_lat)};
  for (size_t i = 0; i < WXSIZEOF(corners); ++i)
    if (!SegmentSafetyPointInRing(corners[i].m_y, corners[i].m_x, ring.points))
      return false;

  // Corners alone are insufficient for a concave coverage polygon.  If its
  // boundary enters the rectangle, the entire buffered segment cannot be
  // certified as covered.
  for (size_t i = 0; i < ring.points.size(); ++i) {
    const wxPoint2DDouble& a = ring.points[i];
    const wxPoint2DDouble& b = ring.points[(i + 1) % ring.points.size()];
    if (SegmentSafetyPointInsideBBox(a, box) ||
        SegmentSafetyPointInsideBBox(b, box))
      return false;
    for (size_t edge = 0; edge < WXSIZEOF(corners); ++edge)
      if (SegmentSafetySegmentsIntersect(
              a, b, corners[edge], corners[(edge + 1) % WXSIZEOF(corners)]))
        return false;
  }
  return true;
}

bool SegmentSafetyChartFullyCoversBBox(const SegmentSafetySnapshotChart& chart,
                                       const SegmentSafetyBBox& box) {
  if (!SegmentSafetyBBoxIntersects(chart.bbox, box)) return false;
  bool covered = false;
  for (std::vector<CachedLandRing>::const_iterator it = chart.coverage.begin();
       it != chart.coverage.end(); ++it) {
    if (SegmentSafetyRingContainsBBox(*it, box)) {
      covered = true;
      break;
    }
  }
  if (!covered) return false;

  // Any possible contact with a no-coverage polygon invalidates a positive
  // certificate.  Falling back is intentionally more conservative than
  // attempting to infer hole topology here.
  for (std::vector<CachedLandRing>::const_iterator it =
           chart.no_coverage.begin();
       it != chart.no_coverage.end(); ++it)
    if (SegmentSafetyBBoxIntersects(it->bbox, box)) return false;
  return true;
}

bool SegmentSafetyChartHazardMayAffectBBox(
    const SegmentSafetySnapshotChart& chart, const SegmentSafetyBBox& box) {
  const wxPoint2DDouble corners[] = {wxPoint2DDouble(box.min_lon, box.min_lat),
                                     wxPoint2DDouble(box.max_lon, box.min_lat),
                                     wxPoint2DDouble(box.max_lon, box.max_lat),
                                     wxPoint2DDouble(box.min_lon, box.max_lat)};
  for (std::vector<CachedLandRing>::const_iterator it = chart.hazards.begin();
       it != chart.hazards.end(); ++it) {
    if (!SegmentSafetyBBoxIntersects(it->bbox, box)) continue;

    // Resolve broad coastline bounding-box overlap against the actual ring.
    // Contact and boundary ambiguity both remain hazardous; only a proven
    // disjoint ring permits an open-sea certificate.
    for (size_t corner = 0; corner < WXSIZEOF(corners); ++corner)
      if (SegmentSafetyPointInRing(corners[corner].m_y, corners[corner].m_x,
                                   it->points))
        return true;
    for (std::vector<wxPoint2DDouble>::const_iterator point =
             it->points.begin();
         point != it->points.end(); ++point)
      if (SegmentSafetyPointInsideBBox(*point, box)) return true;
    for (size_t edge = 0; edge < it->points.size(); ++edge) {
      const wxPoint2DDouble& a = it->points[edge];
      const wxPoint2DDouble& b = it->points[(edge + 1) % it->points.size()];
      for (size_t box_edge = 0; box_edge < WXSIZEOF(corners); ++box_edge)
        if (SegmentSafetySegmentsIntersect(
                a, b, corners[box_edge],
                corners[(box_edge + 1) % WXSIZEOF(corners)]))
          return true;
    }
  }
  return false;
}

bool SegmentSafetySnapshotChartPrecedes(const SegmentSafetySnapshotChart& a,
                                        const SegmentSafetySnapshotChart& b) {
  if (a.native_scale != b.native_scale) return a.native_scale < b.native_scale;
  if (a.edition_date != b.edition_date) return a.edition_date > b.edition_date;
  if (a.file_time != b.file_time) return a.file_time > b.file_time;
  if (a.chart_path != b.chart_path) return a.chart_path < b.chart_path;
  return a.db_index < b.db_index;
}

SegmentSafetySnapshotDecision ClassifySegmentSafetySnapshotBBox(
    const SegmentSafetyHazardSnapshot& snapshot,
    const SegmentSafetyBBox& query) {
  const SegmentSafetySnapshotChart* best = NULL;
  std::vector<const SegmentSafetySnapshotChart*> covering;
  for (std::vector<SegmentSafetySnapshotChart>::const_iterator it =
           snapshot.charts.begin();
       it != snapshot.charts.end(); ++it) {
    if (!SegmentSafetyChartFullyCoversBBox(*it, query)) continue;
    covering.push_back(&*it);
    if (!best || SegmentSafetySnapshotChartPrecedes(*it, *best)) best = &*it;
  }
  if (!best || !best->vector_supported) return SEGMENT_SAFETY_SNAPSHOT_UNKNOWN;

  // A hazard on any overlapping supported chart represents disagreement.
  // Do not fast-reject and potentially lose a valid passage; fall back to the
  // existing best-chart classifier to resolve it.
  for (std::vector<const SegmentSafetySnapshotChart*>::const_iterator it =
           covering.begin();
       it != covering.end(); ++it) {
    if (!(*it)->vector_supported ||
        SegmentSafetyChartHazardMayAffectBBox(**it, query))
      return SEGMENT_SAFETY_SNAPSHOT_UNKNOWN;
  }
  return SEGMENT_SAFETY_SNAPSHOT_SAFE;
}

bool SegmentSafetyBBoxInsideCertifiedCell(
    const SegmentSafetyBBox& query, double cell_degrees,
    const std::set<std::pair<long, long> >& certified) {
  const long min_lat_cell = floor(query.min_lat / cell_degrees);
  const long max_lat_cell = floor(query.max_lat / cell_degrees);
  const long min_lon_cell = floor(query.min_lon / cell_degrees);
  const long max_lon_cell = floor(query.max_lon / cell_degrees);
  return min_lat_cell == max_lat_cell && min_lon_cell == max_lon_cell &&
         certified.find(std::make_pair(min_lat_cell, min_lon_cell)) !=
             certified.end();
}

void BuildSegmentSafetySnapshotHierarchy(
    SegmentSafetyHazardSnapshot* snapshot) {
  if (!snapshot) return;
  constexpr double kLargeCellDegrees = 1.0;
  constexpr double kFineCellDegrees = 0.25;
  const long first_lat = floor(snapshot->area.min_lat / kLargeCellDegrees);
  const long last_lat =
      floor(nextafter(snapshot->area.max_lat, snapshot->area.min_lat) /
            kLargeCellDegrees);
  const long first_lon = floor(snapshot->area.min_lon / kLargeCellDegrees);
  const long last_lon =
      floor(nextafter(snapshot->area.max_lon, snapshot->area.min_lon) /
            kLargeCellDegrees);
  for (long lat = first_lat; lat <= last_lat; ++lat) {
    for (long lon = first_lon; lon <= last_lon; ++lon) {
      const SegmentSafetyBBox large = {
          lat * kLargeCellDegrees, (lat + 1) * kLargeCellDegrees,
          lon * kLargeCellDegrees, (lon + 1) * kLargeCellDegrees};
      const bool large_inside = large.min_lat >= snapshot->area.min_lat &&
                                large.max_lat <= snapshot->area.max_lat &&
                                large.min_lon >= snapshot->area.min_lon &&
                                large.max_lon <= snapshot->area.max_lon;
      if (large_inside && ClassifySegmentSafetySnapshotBBox(*snapshot, large) ==
                              SEGMENT_SAFETY_SNAPSHOT_SAFE) {
        snapshot->certified_safe_large_cells.insert(std::make_pair(lat, lon));
        continue;
      }
      // Subdivide only uncertain large cells.  These quarter-degree cells
      // bridge the open-sea certificate and the existing coarse/fine tiles.
      for (int lat_part = 0; lat_part < 4; ++lat_part) {
        for (int lon_part = 0; lon_part < 4; ++lon_part) {
          const long fine_lat = lat * 4 + lat_part;
          const long fine_lon = lon * 4 + lon_part;
          const SegmentSafetyBBox fine = {
              fine_lat * kFineCellDegrees, (fine_lat + 1) * kFineCellDegrees,
              fine_lon * kFineCellDegrees, (fine_lon + 1) * kFineCellDegrees};
          const bool fine_inside = fine.min_lat >= snapshot->area.min_lat &&
                                   fine.max_lat <= snapshot->area.max_lat &&
                                   fine.min_lon >= snapshot->area.min_lon &&
                                   fine.max_lon <= snapshot->area.max_lon;
          if (fine_inside &&
              ClassifySegmentSafetySnapshotBBox(*snapshot, fine) ==
                  SEGMENT_SAFETY_SNAPSHOT_SAFE)
            snapshot->certified_safe_fine_cells.insert(
                std::make_pair(fine_lat, fine_lon));
        }
      }
    }
  }
}

SegmentSafetySnapshotDecision QuerySegmentSafetyHazardSnapshot(
    double lat1, double lon1, double lat2, double lon2, double safety_margin_nm,
    bool check_depth) {
  std::shared_ptr<const SegmentSafetyHazardSnapshot> snapshot;
  {
    wxMutexLocker lock(s_segment_safety_cache_mutex);
    snapshot = s_segment_safety_hazard_snapshot;
  }
  if (!snapshot || check_depth || lon1 < -180.0 || lon1 > 180.0 ||
      lon2 < -180.0 || lon2 > 180.0)
    return SEGMENT_SAFETY_SNAPSHOT_UNKNOWN;

  const SegmentSafetyBBox query =
      SegmentSafetySegmentBBox(lat1, lon1, lat2, lon2, safety_margin_nm);
  if (query.min_lat < snapshot->area.min_lat ||
      query.max_lat > snapshot->area.max_lat ||
      query.min_lon < snapshot->area.min_lon ||
      query.max_lon > snapshot->area.max_lon)
    return SEGMENT_SAFETY_SNAPSHOT_UNKNOWN;

  if (SegmentSafetyBBoxInsideCertifiedCell(
          query, 1.0, snapshot->certified_safe_large_cells) ||
      SegmentSafetyBBoxInsideCertifiedCell(query, 0.25,
                                           snapshot->certified_safe_fine_cells))
    return SEGMENT_SAFETY_SNAPSHOT_SAFE;
  return ClassifySegmentSafetySnapshotBBox(*snapshot, query);
}

CachedLandRing SegmentSafetyRingFromFloatTable(const float* points, int count) {
  CachedLandRing ring;
  if (!points || count < 3) return ring;
  ring.points.reserve(count);
  for (int i = 0; i < count; ++i)
    ring.points.push_back(wxPoint2DDouble(points[i * 2 + 1], points[i * 2]));
  ring.bbox = SegmentSafetyRingBBox(ring.points);
  return ring;
}

void SegmentSafetyAppendFeatureRings(s57chart* chart, const char* feature_name,
                                     std::vector<CachedLandRing>* destination,
                                     std::set<std::string>* seen = NULL) {
  if (!chart || !feature_name || !destination) return;
  std::vector<std::vector<wxPoint2DDouble> > rings;
  chart->CollectFeatureAreaRings(feature_name, rings);
  for (std::vector<std::vector<wxPoint2DDouble> >::iterator it = rings.begin();
       it != rings.end(); ++it) {
    if (it->size() < 3) continue;
    CachedLandRing ring;
    ring.points.swap(*it);
    ring.bbox = SegmentSafetyRingBBox(ring.points);
    if (seen) {
      std::ostringstream signature;
      signature << ring.points.size();
      for (std::vector<wxPoint2DDouble>::const_iterator point =
               ring.points.begin();
           point != ring.points.end(); ++point)
        signature << ':' << llround(point->m_y * 1e7) << ','
                  << llround(point->m_x * 1e7);
      if (!seen->insert(signature.str()).second) continue;
    }
    destination->push_back(ring);
  }
}

int SegmentSafetyCurrentGroupIndex() {
  ChartCanvas* canvas = gFrame && gFrame->GetFocusCanvas()
                            ? gFrame->GetFocusCanvas()
                            : (gFrame ? gFrame->GetPrimaryCanvas() : NULL);
  return canvas ? canvas->m_groupIndex : 0;
}

void SegmentSafetyHashAdd(uint64_t* hash, const wxString& text) {
  if (!hash) return;
  wxCharBuffer utf8 = text.ToUTF8();
  const char* data = utf8.data() ? utf8.data() : "";
  while (*data) {
    *hash ^= (unsigned char)*data++;
    *hash *= 1099511628211ULL;
  }
  *hash ^= (unsigned char)'|';
  *hash *= 1099511628211ULL;
}

wxString SegmentSafetyPluginBatchProviderIdentity() {
  wxArrayString providers;
  wxLogNull suppress_missing_optional_symbols;
  auto plugin_array = PluginLoader::GetInstance()->GetPlugInArray();
  for (unsigned int i = 0; i < plugin_array->GetCount(); ++i) {
    PlugInContainer* pic = plugin_array->Item(i);
    if (!pic || !pic->m_enabled || !pic->m_init_state ||
        !pic->m_library.HasSymbol(
            OCPN_PLUGIN_CHART_SAFETY_GRID_SYMBOL_V1))
      continue;
    wxFileName file(pic->m_plugin_file);
    const wxULongLong size = file.GetSize();
    const wxDateTime modified = file.GetModificationTime();
    providers.Add(wxString::Format(
        "%s:size=%s:mtime=%lld:version=%s", pic->m_plugin_file,
        size != wxInvalidSize ? size.ToString() : wxString("unknown"),
        modified.IsValid() ? static_cast<long long>(modified.GetTicks()) : -1LL,
        pic->m_version_str));
  }
  providers.Sort();
  return wxJoin(providers, ';');
}

wxString SegmentSafetyChartIdentity() {
  if (!ChartData) return wxEmptyString;

  uint64_t hash = 1469598103934665603ULL;
  // Keep only global semantic inputs here.  Individual chart metadata is
  // fingerprinted per tile, allowing a chart update to invalidate affected
  // waters without discarding an otherwise valid regional atlas.
  SegmentSafetyHashAdd(&hash, "semantic-grid-v6");
  SegmentSafetyHashAdd(
      &hash, wxString::Format("group=%d", SegmentSafetyCurrentGroupIndex()));
  if (g_pi_manager)
    SegmentSafetyHashAdd(
        &hash, "batch-providers=" + SegmentSafetyPluginBatchProviderIdentity());
  // v6 also identifies canonical global-grid coordinates and padded chart
  // candidate discovery at shared provider-batch edges.  Earlier v4 stores
  // can contain edge cells classified from an incomplete candidate set, so
  // they must be invalidated once even when the chart database is unchanged.
  return wxString::Format("ocpn-chart-safety-v6-%016llx",
                          (unsigned long long)hash);
}

wxString SegmentSafetyChartCatalogIdentity() {
  if (!ChartData) return wxEmptyString;
  uint64_t hash = 1469598103934665603ULL;
  SegmentSafetyHashAdd(&hash, ChartData->GetDBFileName());
  SegmentSafetyHashAdd(&hash,
                       wxString::Format("dbv=%d", ChartData->GetVersion()));
  const int entries = ChartData->GetChartTableEntries();
  SegmentSafetyHashAdd(&hash, wxString::Format("entries=%d", entries));
  for (int i = 0; i < entries; ++i) {
    const ChartTableEntry& entry = ChartData->GetChartTableEntry(i);
    SegmentSafetyHashAdd(&hash,
                         wxString::FromUTF8(entry.GetFullPath().c_str()));
    SegmentSafetyHashAdd(
        &hash, wxString::Format("t=%lld:e=%lld:scale=%d:type=%d:family=%d",
                                static_cast<long long>(entry.GetFileTime()),
                                static_cast<long long>(
                                    entry.GetChartEditionDate()),
                                entry.GetScale(), entry.GetChartType(),
                                entry.GetChartFamily()));
    const std::vector<int>& groups = entry.GetGroupArray();
    for (size_t group = 0; group < groups.size(); ++group)
      SegmentSafetyHashAdd(&hash,
                           wxString::Format("g%d", groups[group]));
  }
  return wxString::Format("chart-catalog-v1-%016llx",
                          static_cast<unsigned long long>(hash));
}

void SegmentSafetyRefreshPersistentChartIdentity() {
  if (!wxThread::IsMain()) return;
  wxString identity = SegmentSafetyChartIdentity();
  wxString catalog_identity = SegmentSafetyChartCatalogIdentity();
  PlugInSegmentSafetyTileCacheIdentityFn identity_callback = nullptr;
  PlugInSegmentSafetyTileCacheDependenciesChangedFn dependencies_callback =
      nullptr;
  void* identity_context = nullptr;
  void* dependencies_context = nullptr;
  {
    wxMutexLocker lock(s_segment_safety_cache_mutex);
    if (identity != s_segment_safety_chart_identity) {
      wxLogMessage("WR_CERT_SAFE_CACHE chart_identity old=\"%s\" new=\"%s\"",
                   s_segment_safety_chart_identity, identity);
      s_segment_safety_chart_identity = identity;
      // Every classification and derived proof is chart-set specific. Clear
      // session state as well as pending persistent data when the chart
      // identity changes so stale tiles cannot survive a chart database
      // update.
      s_segment_safety_land_cache.clear();
      s_segment_safety_hazard_snapshot.reset();
      s_segment_safety_point_cache.clear();
      s_segment_safety_grid_cache.clear();
      s_segment_safety_route_mask_cache.clear();
      s_segment_safety_coarse_route_mask_cache.clear();
      s_segment_safety_segment_cache.clear();
      s_segment_safety_pinned_grid_keys.clear();
      s_segment_safety_pinned_route_mask_keys.clear();
      s_segment_safety_persistent_base_tile_cache.clear();
      s_segment_safety_persistent_certified_safe_cache.clear();
      s_segment_safety_persistent_cache_loaded = false;
      s_segment_safety_persistent_base_tiles_loaded = false;
      s_segment_safety_persistent_cache_dirty = false;
      s_segment_safety_persistent_base_tiles_dirty = false;
      s_segment_safety_persistent_tiles_since_checkpoint = 0;
      identity_callback =
          s_segment_safety_external_tile_cache.identity_changed;
      identity_context = s_segment_safety_external_tile_cache.context;
    }
    if (catalog_identity != s_segment_safety_chart_catalog_identity) {
      const bool initial_catalog =
          s_segment_safety_chart_catalog_identity.IsEmpty();
      wxLogMessage(
          "WR_CERT_SAFE_CACHE chart_catalog old=\"%s\" new=\"%s\" "
          "selective_base_tile_validation=1",
          s_segment_safety_chart_catalog_identity, catalog_identity);
      s_segment_safety_chart_catalog_identity = catalog_identity;
      // Base semantic tiles carry a local dependency fingerprint and remain
      // available for selective validation.  Derived masks and broad proofs
      // do not, so discard them whenever the catalogue changes.
      if (!initial_catalog) {
        s_segment_safety_land_cache.clear();
        s_segment_safety_hazard_snapshot.reset();
        s_segment_safety_point_cache.clear();
        s_segment_safety_route_mask_cache.clear();
        s_segment_safety_coarse_route_mask_cache.clear();
        s_segment_safety_segment_cache.clear();
        s_segment_safety_pinned_route_mask_keys.clear();
        s_segment_safety_persistent_certified_safe_cache.clear();
        s_segment_safety_persistent_cache_dirty = true;
        dependencies_callback =
            s_segment_safety_external_tile_cache.dependencies_changed;
        dependencies_context =
            s_segment_safety_external_tile_cache.context;
      }
    }
  }
  // Plugin callbacks may perform disk I/O and use their own mutex. Never call
  // them while holding the host cache mutex.
  if (identity_callback) {
    wxCharBuffer utf8 = identity.ToUTF8();
    identity_callback(identity_context, utf8.data() ? utf8.data() : "");
  }
  if (dependencies_callback)
    dependencies_callback(dependencies_context);
}

wxString SegmentSafetyPersistentCachePath() {
  wxString base = g_Platform ? g_Platform->GetPrivateDataDir()
                             : *GetpPrivateApplicationDataLocation();
  wxFileName dir(base, "");
  dir.AppendDir("weather_routing");
  wxFileName::Mkdir(dir.GetPath(), wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
  return dir.GetPathWithSep() + "chart_safety_certified_cache.json";
}

wxString SegmentSafetyPersistentBaseTileCachePath() {
  wxFileName certified(SegmentSafetyPersistentCachePath());
  return certified.GetPathWithSep() + "chart_safety_base_tiles.bin";
}

std::string SegmentSafetyPersistentProofKey(long lat_cell, long lon_cell,
                                            double safety_margin_nm,
                                            bool check_depth,
                                            double minimum_depth_m) {
  wxMutexLocker lock(s_segment_safety_cache_mutex);
  if (s_segment_safety_chart_identity.IsEmpty()) return std::string();
  return (s_segment_safety_chart_identity + ":" +
          wxString::FromUTF8(SegmentSafetyCoarseRouteMaskKey(
                                 lat_cell, lon_cell, safety_margin_nm,
                                 check_depth, minimum_depth_m)
                                 .c_str()))
      .ToStdString();
}

bool SegmentSafetyPersistentCacheLoadLocked(const wxString& path) {
  if (s_segment_safety_persistent_cache_loaded) return true;
  s_segment_safety_persistent_cache_loaded = true;
  s_segment_safety_persistent_certified_safe_cache.clear();
  s_segment_safety_persistent_entries_loaded = 0;
  s_segment_safety_persistent_entries_ignored = 0;
  s_segment_safety_persistent_stale_ignored = 0;
  s_segment_safety_persistent_malformed_ignored = 0;

  if (!wxFileExists(path)) {
    wxLogMessage(
        "WR_CERT_SAFE_CACHE load enabled=1 entries_loaded=0 "
        "cache_file_path=\"%s\" reason=missing",
        path);
    return true;
  }

  wxFileInputStream input(path);
  if (!input.IsOk()) {
    ++s_segment_safety_persistent_malformed_ignored;
    wxLogMessage(
        "WR_CERT_SAFE_CACHE load enabled=1 cache_malformed_ignored=1 "
        "cache_file_path=\"%s\" reason=open_failed",
        path);
    return false;
  }

  wxJSONReader reader;
  wxJSONValue root;
  int errors = reader.Parse(input, &root);
  if (errors || !root.IsObject() ||
      root.Get("format_version", 0).AsInt() !=
          kSegmentSafetyPersistentCacheVersion ||
      root.Get("route_mask_algorithm_version", 0).AsInt() !=
          kSegmentSafetyRouteMaskAlgorithmVersion ||
      !root.HasMember("entries") || !root["entries"].IsArray()) {
    ++s_segment_safety_persistent_malformed_ignored;
    wxLogMessage(
        "WR_CERT_SAFE_CACHE load enabled=1 cache_malformed_ignored=1 "
        "cache_file_path=\"%s\" reason=parse_or_version errors=%d",
        path, errors);
    return false;
  }

  wxJSONValue entries = root["entries"];
  for (int i = 0; i < entries.Size(); ++i) {
    wxJSONValue entry = entries[i];
    if (!entry.IsObject() || !entry.HasMember("key") ||
        !entry.Get("certified_safe", false).AsBool()) {
      ++s_segment_safety_persistent_entries_ignored;
      continue;
    }
    CachedSegmentSafetyCoarseRouteMaskCell cell;
    cell.group_index = entry.Get("group_index", 0).AsInt();
    cell.lat_cell = entry.Get("lat_cell", 0).AsInt();
    cell.lon_cell = entry.Get("lon_cell", 0).AsInt();
    cell.degrees = entry.Get("coarse_degrees", cell.degrees).AsDouble();
    cell.min_lat =
        entry.Get("min_lat", cell.lat_cell * cell.degrees).AsDouble();
    cell.min_lon =
        entry.Get("min_lon", cell.lon_cell * cell.degrees).AsDouble();
    cell.check_depth = entry.Get("check_depth", false).AsBool();
    cell.minimum_depth_m = entry.Get("minimum_depth_m", 0.0).AsDouble();
    cell.safety_margin_nm = entry.Get("safety_margin_nm", 0.0).AsDouble();
    cell.state = SEGMENT_SAFETY_COARSE_CERTIFIED_SAFE;
    cell.block_summary_flags = SEGMENT_SAFETY_ROUTE_CLEAR;
    cell.fine_tiles_checked = kSegmentSafetyCoarseRouteMaskFactor *
                              kSegmentSafetyCoarseRouteMaskFactor;
    cell.fine_tiles_clear = cell.fine_tiles_checked;
    cell.fine_tiles_mixed = 0;
    cell.source = (PlugInSegmentSafetySource)entry
                      .Get("source", (int)PI_SEGMENT_SAFETY_SOURCE_VECTOR_CHART)
                      .AsInt();
    cell.persistent_cache_allowed =
        entry.Get("persistent_cache_allowed",
                  cell.source != PI_SEGMENT_SAFETY_SOURCE_PLUGIN_VECTOR)
            .AsBool();
    if (cell.source == PI_SEGMENT_SAFETY_SOURCE_PLUGIN_VECTOR &&
        !cell.persistent_cache_allowed) {
      ++s_segment_safety_persistent_entries_ignored;
      continue;
    }
    wxString key = entry.Get("key", "").AsString();
    if (key.IsEmpty()) {
      ++s_segment_safety_persistent_entries_ignored;
      continue;
    }
    s_segment_safety_persistent_certified_safe_cache[key.ToStdString()] = cell;
    while (s_segment_safety_persistent_certified_safe_cache.size() >
           kMaxSegmentSafetyPersistentCertifiedCells)
      s_segment_safety_persistent_certified_safe_cache.erase(
          s_segment_safety_persistent_certified_safe_cache.begin());
    ++s_segment_safety_persistent_entries_loaded;
  }

  wxLogMessage(
      "WR_CERT_SAFE_CACHE load enabled=1 entries_loaded=%ld "
      "entries_ignored=%ld "
      "cache_malformed_ignored=%ld cache_file_path=\"%s\"",
      s_segment_safety_persistent_entries_loaded,
      s_segment_safety_persistent_entries_ignored,
      s_segment_safety_persistent_malformed_ignored, path);
  return true;
}

template <typename T>
bool SegmentSafetyWriteBinary(std::ostream& output, const T& value) {
  output.write(reinterpret_cast<const char*>(&value), sizeof(value));
  return output.good();
}

template <typename T>
bool SegmentSafetyReadBinary(std::istream& input, T* value) {
  if (!value) return false;
  input.read(reinterpret_cast<char*>(value), sizeof(*value));
  return input.good();
}

bool SegmentSafetyWriteBinaryString(std::ostream& output,
                                    const std::string& value) {
  if (value.size() > 4096) return false;
  uint32_t size = static_cast<uint32_t>(value.size());
  if (!SegmentSafetyWriteBinary(output, size)) return false;
  output.write(value.data(), size);
  return output.good();
}

bool SegmentSafetyReadBinaryString(std::istream& input, std::string* value) {
  if (!value) return false;
  uint32_t size = 0;
  if (!SegmentSafetyReadBinary(input, &size) || size > 4096) return false;
  value->assign(size, '\0');
  if (size) input.read(&(*value)[0], size);
  return input.good();
}

bool SegmentSafetyPersistentBaseTileCacheLoadLocked(const wxString& path) {
  if (s_segment_safety_persistent_base_tiles_loaded) return true;
  s_segment_safety_persistent_base_tiles_loaded = true;
  s_segment_safety_persistent_base_tile_cache.clear();
  s_segment_safety_persistent_base_tiles_loaded_count = 0;
  s_segment_safety_persistent_base_tiles_ignored = 0;

  if (!wxFileExists(path)) {
    wxLogMessage(
        "WR_BASE_TILE_CACHE load enabled=1 tiles_loaded=0 "
        "cache_file_path=\"%s\" reason=missing",
        path);
    return true;
  }

  std::ifstream input(path.ToStdString().c_str(),
                      std::ios::in | std::ios::binary);
  char magic[8] = {};
  uint32_t format_version = 0;
  uint32_t algorithm_version = 0;
  uint32_t endian_marker = 0;
  double tile_degrees = 0.0;
  double resolution = 0.0;
  uint32_t tile_count = 0;
  std::string identity;
  input.read(magic, sizeof(magic));
  bool header_ok = input.good() && !memcmp(magic, "WRBASE1", 7) &&
                   SegmentSafetyReadBinary(input, &format_version) &&
                   SegmentSafetyReadBinary(input, &algorithm_version) &&
                   SegmentSafetyReadBinary(input, &endian_marker) &&
                   SegmentSafetyReadBinary(input, &tile_degrees) &&
                   SegmentSafetyReadBinary(input, &resolution) &&
                   SegmentSafetyReadBinary(input, &tile_count) &&
                   SegmentSafetyReadBinaryString(input, &identity);
  const std::string expected_identity =
      s_segment_safety_chart_identity.ToStdString();
  if (!header_ok || format_version != kSegmentSafetyPersistentBaseTileVersion ||
      algorithm_version != kSegmentSafetyRouteMaskAlgorithmVersion ||
      endian_marker != 0x01020304u ||
      fabs(tile_degrees - kSegmentSafetyGridTileDegrees) > 1e-12 ||
      fabs(resolution - kSegmentSafetyGridResolutionDegrees) > 1e-12 ||
      tile_count > kMaxSegmentSafetyPersistentGridTiles ||
      identity != expected_identity) {
    ++s_segment_safety_persistent_base_tiles_ignored;
    wxLogMessage(
        "WR_BASE_TILE_CACHE load enabled=1 tiles_loaded=0 tiles_ignored=1 "
        "cache_file_path=\"%s\" reason=%s cache_key_match=%d",
        path, header_ok ? "identity_or_version_mismatch" : "malformed",
        header_ok && identity == expected_identity ? 1 : 0);
    return false;
  }

  const uint16_t valid_hazards =
      SEGMENT_SAFETY_HAZARD_LAND | SEGMENT_SAFETY_HAZARD_DRYING |
      SEGMENT_SAFETY_HAZARD_NO_CHART | SEGMENT_SAFETY_HAZARD_UNKNOWN_CLASS;
  for (uint32_t record = 0; record < tile_count; ++record) {
    int32_t group_index = 0;
    int64_t lat_tile = 0;
    int64_t lon_tile = 0;
    int32_t rows = 0;
    int32_t cols = 0;
    int32_t chart_db_index = -1;
    int32_t chart_scale = -1;
    int32_t source = PI_SEGMENT_SAFETY_SOURCE_NONE;
    uint32_t hazard_summary = 0;
    uint8_t depth_complete = 0;
    uint32_t cell_count = 0;
    std::string chart_path;
    std::string dependency_identity;
    bool record_ok = SegmentSafetyReadBinary(input, &group_index) &&
                     SegmentSafetyReadBinary(input, &lat_tile) &&
                     SegmentSafetyReadBinary(input, &lon_tile) &&
                     SegmentSafetyReadBinary(input, &rows) &&
                     SegmentSafetyReadBinary(input, &cols) &&
                     SegmentSafetyReadBinary(input, &chart_db_index) &&
                     SegmentSafetyReadBinary(input, &chart_scale) &&
                     SegmentSafetyReadBinary(input, &source) &&
                     SegmentSafetyReadBinary(input, &hazard_summary) &&
                     SegmentSafetyReadBinary(input, &depth_complete) &&
                     SegmentSafetyReadBinaryString(input, &chart_path) &&
                     SegmentSafetyReadBinaryString(input,
                                                   &dependency_identity) &&
                     SegmentSafetyReadBinary(input, &cell_count);
    const bool dimensions_ok =
        rows > 0 && cols > 0 && rows <= 256 && cols <= 256;
    const uint32_t expected_cells =
        dimensions_ok
            ? static_cast<uint32_t>(rows) * static_cast<uint32_t>(cols)
            : 0;
    if (!record_ok || group_index != SegmentSafetyCurrentGroupIndex() ||
        !dimensions_ok || lat_tile < -1800 || lat_tile > 1800 ||
        lon_tile < -7200 || lon_tile > 7200 ||
        source < PI_SEGMENT_SAFETY_SOURCE_NONE ||
        source > PI_SEGMENT_SAFETY_SOURCE_PLUGIN_VECTOR ||
        source == PI_SEGMENT_SAFETY_SOURCE_PLUGIN_VECTOR ||
        expected_cells == 0 || cell_count != expected_cells ||
        depth_complete > 1 || dependency_identity.empty() ||
        dependency_identity.size() >=
            sizeof(CachedPointSafetyGridTile().dependency_identity) ||
        (hazard_summary & ~valid_hazards)) {
      ++s_segment_safety_persistent_base_tiles_ignored;
      wxLogMessage(
          "WR_BASE_TILE_CACHE load cache_malformed_ignored=1 record=%u "
          "cache_file_path=\"%s\"",
          record, path);
      s_segment_safety_persistent_base_tile_cache.clear();
      return false;
    }

    CachedPointSafetyGridTile tile;
    tile.group_index = group_index;
    tile.lat_tile = static_cast<long>(lat_tile);
    tile.lon_tile = static_cast<long>(lon_tile);
    tile.min_lat = tile.lat_tile * kSegmentSafetyGridTileDegrees;
    tile.min_lon = tile.lon_tile * kSegmentSafetyGridTileDegrees;
    tile.resolution = kSegmentSafetyGridResolutionDegrees;
    tile.rows = rows;
    tile.cols = cols;
    tile.chart_db_index = chart_db_index;
    tile.chart_scale = chart_scale;
    tile.source = static_cast<PlugInSegmentSafetySource>(source);
    tile.hazard_summary_flags = hazard_summary;
    tile.depth_complete = depth_complete != 0;
    strncpy(tile.chart_path, chart_path.c_str(), sizeof(tile.chart_path) - 1);
    tile.chart_path[sizeof(tile.chart_path) - 1] = '\0';
    strncpy(tile.dependency_identity, dependency_identity.c_str(),
            sizeof(tile.dependency_identity) - 1);
    tile.dependency_identity[sizeof(tile.dependency_identity) - 1] = '\0';
    tile.hazard_flags.resize(cell_count);
    tile.has_depth.resize(cell_count);
    tile.min_depth_m.resize(cell_count);
    input.read(reinterpret_cast<char*>(&tile.hazard_flags[0]),
               cell_count * sizeof(tile.hazard_flags[0]));
    input.read(reinterpret_cast<char*>(&tile.has_depth[0]),
               cell_count * sizeof(tile.has_depth[0]));
    input.read(reinterpret_cast<char*>(&tile.min_depth_m[0]),
               cell_count * sizeof(tile.min_depth_m[0]));
    if (!input.good()) {
      ++s_segment_safety_persistent_base_tiles_ignored;
      s_segment_safety_persistent_base_tile_cache.clear();
      return false;
    }

    tile.classes.resize(cell_count);
    tile.has_drying.resize(cell_count);
    tile.land_count = tile.water_count = tile.drying_count =
        tile.unknown_count = 0;
    for (uint32_t i = 0; i < cell_count; ++i) {
      uint16_t hazards = tile.hazard_flags[i];
      if ((hazards & ~valid_hazards) || tile.has_depth[i] > 1 ||
          (tile.has_depth[i] && !std::isfinite(tile.min_depth_m[i]))) {
        ++s_segment_safety_persistent_base_tiles_ignored;
        s_segment_safety_persistent_base_tile_cache.clear();
        return false;
      }
      if (hazards & SEGMENT_SAFETY_HAZARD_LAND) {
        tile.classes[i] = SEGMENT_SAFETY_POINT_LAND;
        ++tile.land_count;
      } else if (hazards & SEGMENT_SAFETY_HAZARD_DRYING) {
        tile.classes[i] = SEGMENT_SAFETY_POINT_DRYING;
        ++tile.drying_count;
      } else if (hazards & (SEGMENT_SAFETY_HAZARD_NO_CHART |
                            SEGMENT_SAFETY_HAZARD_UNKNOWN_CLASS)) {
        tile.classes[i] = SEGMENT_SAFETY_POINT_NO_DATA;
        ++tile.unknown_count;
      } else {
        tile.classes[i] = SEGMENT_SAFETY_POINT_WATER;
        ++tile.water_count;
      }
      tile.has_drying[i] = (hazards & SEGMENT_SAFETY_HAZARD_DRYING) ? 1 : 0;
    }
    tile.built = true;
    tile.persistent_loaded = true;
    const std::string key =
        SegmentSafetyGridTileKeyForIndices(tile.lat_tile, tile.lon_tile);
    s_segment_safety_persistent_base_tile_cache[key] = tile;
    ++s_segment_safety_persistent_base_tiles_loaded_count;
  }

  wxLogMessage(
      "WR_BASE_TILE_CACHE load enabled=1 cache_key_match=1 tiles_loaded=%ld "
      "tiles_ignored=%ld cache_file_path=\"%s\" cache_file_size=%llu",
      s_segment_safety_persistent_base_tiles_loaded_count,
      s_segment_safety_persistent_base_tiles_ignored, path,
      (unsigned long long)wxFileName(path).GetSize().GetValue());
  return true;
}

void SegmentSafetyPersistentCacheEnsureLoaded() {
  if (!s_segment_safety_persistent_cache_enabled) return;
  wxString path = SegmentSafetyPersistentCachePath();
  wxString base_path = SegmentSafetyPersistentBaseTileCachePath();
  wxMutexLocker lock(s_segment_safety_cache_mutex);
  SegmentSafetyPersistentCacheLoadLocked(path);
  SegmentSafetyPersistentBaseTileCacheLoadLocked(base_path);
}

bool SegmentSafetyPersistentCertifiedCacheSave() {
  wxString path = SegmentSafetyPersistentCachePath();
  wxMutexLocker lock(s_segment_safety_cache_mutex);
  if (!s_segment_safety_persistent_cache_enabled) {
    wxLogMessage("WR_CERT_SAFE_CACHE save enabled=0 cache_file_path=\"%s\"",
                 path);
    return true;
  }
  if (!s_segment_safety_persistent_cache_dirty) return true;

  wxJSONValue root;
  root["format_version"] = kSegmentSafetyPersistentCacheVersion;
  root["route_mask_algorithm_version"] =
      kSegmentSafetyRouteMaskAlgorithmVersion;
  root["updated_utc"] = wxDateTime::UNow().ToUTC().FormatISOCombined('T');
  wxJSONValue entries;
  long saved = 0;
  for (std::map<std::string,
                CachedSegmentSafetyCoarseRouteMaskCell>::const_iterator it =
           s_segment_safety_persistent_certified_safe_cache.begin();
       it != s_segment_safety_persistent_certified_safe_cache.end(); ++it) {
    const CachedSegmentSafetyCoarseRouteMaskCell& cell = it->second;
    if (cell.state != SEGMENT_SAFETY_COARSE_CERTIFIED_SAFE ||
        (cell.source == PI_SEGMENT_SAFETY_SOURCE_PLUGIN_VECTOR &&
         !cell.persistent_cache_allowed))
      continue;
    wxJSONValue entry;
    entry["key"] = wxString::FromUTF8(it->first.c_str());
    entry["certified_safe"] = true;
    entry["group_index"] = cell.group_index;
    entry["lat_cell"] = (wxInt64)cell.lat_cell;
    entry["lon_cell"] = (wxInt64)cell.lon_cell;
    entry["min_lat"] = cell.min_lat;
    entry["min_lon"] = cell.min_lon;
    entry["coarse_degrees"] = cell.degrees;
    entry["coarse_factor"] = kSegmentSafetyCoarseRouteMaskFactor;
    entry["grid_resolution_degrees"] = kSegmentSafetyGridResolutionDegrees;
    entry["safety_margin_nm"] = cell.safety_margin_nm;
    entry["check_depth"] = cell.check_depth;
    entry["minimum_depth_m"] = cell.minimum_depth_m;
    entry["source"] = (int)cell.source;
    entry["persistent_cache_allowed"] = cell.persistent_cache_allowed;
    entries.Append(entry);
    ++saved;
  }
  root["entries"] = entries;

  wxFileName::Mkdir(wxFileName(path).GetPath(), wxS_DIR_DEFAULT,
                    wxPATH_MKDIR_FULL);
  wxString tmp_path = path + ".tmp";
  if (wxFileExists(tmp_path)) wxRemoveFile(tmp_path);
  bool write_ok = false;
  {
    wxFileOutputStream output(tmp_path);
    if (output.IsOk()) {
      wxJSONWriter writer(wxJSONWRITER_STYLED);
      writer.Write(root, output);
      write_ok = output.IsOk();
    }
  }
  if (!write_ok) {
    wxLogMessage(
        "WR_CERT_SAFE_CACHE save failed cache_file_path=\"%s\" entries=%ld",
        path, saved);
    if (wxFileExists(tmp_path)) wxRemoveFile(tmp_path);
    return false;
  }
  if (!wxRenameFile(tmp_path, path, true)) {
    wxLogMessage(
        "WR_CERT_SAFE_CACHE save failed cache_file_path=\"%s\" entries=%ld "
        "reason=rename_failed",
        path, saved);
    if (wxFileExists(tmp_path)) wxRemoveFile(tmp_path);
    return false;
  }
  s_segment_safety_persistent_entries_saved = saved;
  s_segment_safety_persistent_cache_dirty = false;
  wxLogMessage(
      "WR_CERT_SAFE_CACHE save enabled=1 entries_saved=%ld "
      "cache_file_path=\"%s\" "
      "cache_file_size=%llu",
      saved, path, (unsigned long long)wxFileName(path).GetSize().GetValue());
  return true;
}

bool SegmentSafetyPersistentBaseTileCacheSave() {
  const wxString path = SegmentSafetyPersistentBaseTileCachePath();
  wxMutexLocker lock(s_segment_safety_cache_mutex);
  if (!s_segment_safety_persistent_cache_enabled) {
    wxLogMessage("WR_BASE_TILE_CACHE save enabled=0 cache_file_path=\"%s\"",
                 path);
    return true;
  }
  if (!s_segment_safety_persistent_base_tiles_dirty) return true;

  std::map<std::string, CachedPointSafetyGridTile> tiles =
      s_segment_safety_persistent_base_tile_cache;
  for (std::map<std::string, CachedPointSafetyGridTile>::const_iterator it =
           s_segment_safety_grid_cache.begin();
       it != s_segment_safety_grid_cache.end(); ++it) {
    // Licensed plugin-vector semantics are checkpointed by the registered
    // external cache only when the provider advertised permission.  Keep the
    // core raw-tile file native-only so its older format never serializes a
    // plugin tile without carrying that permission bit.
    if (it->second.built &&
        it->second.source != PI_SEGMENT_SAFETY_SOURCE_PLUGIN_VECTOR)
      tiles[it->first] = it->second;
  }
  for (std::map<std::string, CachedPointSafetyGridTile>::iterator it =
           tiles.begin();
       it != tiles.end();) {
    if (it->second.source == PI_SEGMENT_SAFETY_SOURCE_PLUGIN_VECTOR)
      tiles.erase(it++);
    else
      ++it;
  }
  while (tiles.size() > kMaxSegmentSafetyPersistentGridTiles)
    tiles.erase(tiles.begin());

  wxFileName::Mkdir(wxFileName(path).GetPath(), wxS_DIR_DEFAULT,
                    wxPATH_MKDIR_FULL);
  const wxString tmp_path = path + ".tmp";
  if (wxFileExists(tmp_path)) wxRemoveFile(tmp_path);
  std::ofstream output(tmp_path.ToStdString().c_str(),
                       std::ios::out | std::ios::binary | std::ios::trunc);
  const char magic[8] = {'W', 'R', 'B', 'A', 'S', 'E', '1', '\0'};
  output.write(magic, sizeof(magic));
  const uint32_t format_version = kSegmentSafetyPersistentBaseTileVersion;
  const uint32_t algorithm_version = kSegmentSafetyRouteMaskAlgorithmVersion;
  const uint32_t endian_marker = 0x01020304u;
  const uint32_t tile_count = static_cast<uint32_t>(tiles.size());
  bool ok =
      output.good() && SegmentSafetyWriteBinary(output, format_version) &&
      SegmentSafetyWriteBinary(output, algorithm_version) &&
      SegmentSafetyWriteBinary(output, endian_marker) &&
      SegmentSafetyWriteBinary(output, kSegmentSafetyGridTileDegrees) &&
      SegmentSafetyWriteBinary(output, kSegmentSafetyGridResolutionDegrees) &&
      SegmentSafetyWriteBinary(output, tile_count) &&
      SegmentSafetyWriteBinaryString(
          output, s_segment_safety_chart_identity.ToStdString());
  long saved = 0;
  for (std::map<std::string, CachedPointSafetyGridTile>::const_iterator it =
           tiles.begin();
       ok && it != tiles.end(); ++it) {
    const CachedPointSafetyGridTile& tile = it->second;
    const size_t cells = static_cast<size_t>(tile.rows) * tile.cols;
    if (!tile.built || tile.rows <= 0 || tile.cols <= 0 || cells > 65536 ||
        tile.hazard_flags.size() != cells || tile.has_depth.size() != cells ||
        tile.min_depth_m.size() != cells) {
      ok = false;
      break;
    }
    const int32_t group_index = tile.group_index;
    const int64_t lat_tile = tile.lat_tile;
    const int64_t lon_tile = tile.lon_tile;
    const int32_t rows = tile.rows;
    const int32_t cols = tile.cols;
    const int32_t chart_db_index = tile.chart_db_index;
    const int32_t chart_scale = tile.chart_scale;
    const int32_t source = tile.source;
    const uint32_t hazard_summary = tile.hazard_summary_flags;
    const uint8_t depth_complete = tile.depth_complete ? 1 : 0;
    const uint32_t cell_count = static_cast<uint32_t>(cells);
    ok = SegmentSafetyWriteBinary(output, group_index) &&
         SegmentSafetyWriteBinary(output, lat_tile) &&
         SegmentSafetyWriteBinary(output, lon_tile) &&
         SegmentSafetyWriteBinary(output, rows) &&
         SegmentSafetyWriteBinary(output, cols) &&
         SegmentSafetyWriteBinary(output, chart_db_index) &&
         SegmentSafetyWriteBinary(output, chart_scale) &&
         SegmentSafetyWriteBinary(output, source) &&
         SegmentSafetyWriteBinary(output, hazard_summary) &&
         SegmentSafetyWriteBinary(output, depth_complete) &&
         SegmentSafetyWriteBinaryString(output, tile.chart_path) &&
         SegmentSafetyWriteBinaryString(output, tile.dependency_identity) &&
         SegmentSafetyWriteBinary(output, cell_count);
    if (!ok) break;
    output.write(reinterpret_cast<const char*>(&tile.hazard_flags[0]),
                 cells * sizeof(tile.hazard_flags[0]));
    output.write(reinterpret_cast<const char*>(&tile.has_depth[0]),
                 cells * sizeof(tile.has_depth[0]));
    output.write(reinterpret_cast<const char*>(&tile.min_depth_m[0]),
                 cells * sizeof(tile.min_depth_m[0]));
    ok = output.good();
    if (ok) ++saved;
  }
  output.close();
  if (!ok || saved != static_cast<long>(tile_count)) {
    if (wxFileExists(tmp_path)) wxRemoveFile(tmp_path);
    wxLogMessage(
        "WR_BASE_TILE_CACHE save failed tiles=%ld cache_file_path=\"%s\"",
        saved, path);
    return false;
  }
  if (!wxRenameFile(tmp_path, path, true)) {
    if (wxFileExists(tmp_path)) wxRemoveFile(tmp_path);
    wxLogMessage(
        "WR_BASE_TILE_CACHE save failed tiles=%ld cache_file_path=\"%s\" "
        "reason=rename_failed",
        saved, path);
    return false;
  }
  s_segment_safety_persistent_base_tile_cache.swap(tiles);
  s_segment_safety_persistent_base_tiles_saved = saved;
  s_segment_safety_persistent_base_tiles_dirty = false;
  s_segment_safety_persistent_tiles_since_checkpoint = 0;
  wxLogMessage(
      "WR_BASE_TILE_CACHE save enabled=1 tiles_saved=%ld "
      "cache_file_path=\"%s\" cache_file_size=%llu",
      saved, path, (unsigned long long)wxFileName(path).GetSize().GetValue());
  return true;
}

bool SegmentSafetyPersistentCacheSave() {
  bool certified_ok = SegmentSafetyPersistentCertifiedCacheSave();
  bool base_ok = SegmentSafetyPersistentBaseTileCacheSave();
  return certified_ok && base_ok;
}

bool SegmentSafetyPersistentLookupCertifiedSafe(
    long lat_cell, long lon_cell, double safety_margin_nm, bool check_depth,
    double minimum_depth_m, CachedSegmentSafetyCoarseRouteMaskCell* cell) {
  if (!s_segment_safety_persistent_cache_enabled) return false;
  SegmentSafetyPersistentCacheEnsureLoaded();
  std::string key = SegmentSafetyPersistentProofKey(
      lat_cell, lon_cell, safety_margin_nm, check_depth, minimum_depth_m);
  if (key.empty()) return false;
  wxMutexLocker lock(s_segment_safety_cache_mutex);
  std::map<std::string, CachedSegmentSafetyCoarseRouteMaskCell>::const_iterator
      it = s_segment_safety_persistent_certified_safe_cache.find(key);
  if (it == s_segment_safety_persistent_certified_safe_cache.end()) {
    ++s_segment_safety_persistent_stale_ignored;
    return false;
  }
  if (cell) *cell = it->second;
  ++s_segment_safety_persistent_entries_used;
  wxLogMessage(
      "WR_CERT_SAFE_CACHE use cache_used_as_proof=1 cache_key_match=1 "
      "lat_cell=%ld lon_cell=%ld entries_used=%ld fine_tiles_avoided=%d",
      lat_cell, lon_cell, s_segment_safety_persistent_entries_used,
      kSegmentSafetyCoarseRouteMaskFactor *
          kSegmentSafetyCoarseRouteMaskFactor);
  return true;
}

void SegmentSafetyPersistentStoreCertifiedSafe(
    const CachedSegmentSafetyCoarseRouteMaskCell& cell) {
  if (!s_segment_safety_persistent_cache_enabled ||
      cell.state != SEGMENT_SAFETY_COARSE_CERTIFIED_SAFE ||
      (cell.source == PI_SEGMENT_SAFETY_SOURCE_PLUGIN_VECTOR &&
       !cell.persistent_cache_allowed))
    return;
  std::string key = SegmentSafetyPersistentProofKey(
      cell.lat_cell, cell.lon_cell, cell.safety_margin_nm, cell.check_depth,
      cell.minimum_depth_m);
  if (key.empty()) return;
  wxMutexLocker lock(s_segment_safety_cache_mutex);
  s_segment_safety_persistent_certified_safe_cache[key] = cell;
  while (s_segment_safety_persistent_certified_safe_cache.size() >
         kMaxSegmentSafetyPersistentCertifiedCells)
    s_segment_safety_persistent_certified_safe_cache.erase(
        s_segment_safety_persistent_certified_safe_cache.begin());
  s_segment_safety_persistent_cache_dirty = true;
  ++s_segment_safety_persistent_entries_stored;
}

std::string SegmentSafetyPointCacheKey(double lat, double lon) {
  const double bucket_degrees = 0.00025;
  long lat_bucket = lround(lat / bucket_degrees);
  long lon_bucket = lround(lon / bucket_degrees);
  return wxString::Format("%d:%ld:%ld", SegmentSafetyCurrentGroupIndex(),
                          lat_bucket, lon_bucket)
      .ToStdString();
}

std::string SegmentSafetySegmentCacheKey(double lat1, double lon1, double lat2,
                                         double lon2, double safety_margin_nm,
                                         bool check_depth = false,
                                         double minimum_depth_m = 0.0) {
  long a_lat = lround(lat1 / kSegmentSafetyGridResolutionDegrees);
  long a_lon = lround(lon1 / kSegmentSafetyGridResolutionDegrees);
  long b_lat = lround(lat2 / kSegmentSafetyGridResolutionDegrees);
  long b_lon = lround(lon2 / kSegmentSafetyGridResolutionDegrees);
  if (std::make_pair(b_lat, b_lon) < std::make_pair(a_lat, a_lon)) {
    std::swap(a_lat, b_lat);
    std::swap(a_lon, b_lon);
  }
  double mid_lat = (lat1 + lat2) / 2.0;
  double cell_nm =
      wxMin(kSegmentSafetyGridResolutionDegrees * 60.0,
            kSegmentSafetyGridResolutionDegrees * 60.0 *
                wxMax(0.1, fabs(cos(SegmentSafetyDegToRad(mid_lat)))));
  long margin_bucket = lround(safety_margin_nm / wxMax(0.01, cell_nm));
  long depth_bucket = lround(minimum_depth_m * 100.0);
  return wxString::Format("%d:%.6f:%ld:%d:%ld:%ld:%ld:%ld:%ld",
                          SegmentSafetyCurrentGroupIndex(),
                          kSegmentSafetyGridResolutionDegrees, margin_bucket,
                          check_depth ? 1 : 0, depth_bucket, a_lat, a_lon,
                          b_lat, b_lon)
      .ToStdString();
}

void StoreSegmentSafetySegmentCache(const std::string& key,
                                    const CachedSegmentSafetyResult& value,
                                    SegmentSafetyCoreStats* stats) {
  wxMutexLocker lock(s_segment_safety_cache_mutex);
  if (s_segment_safety_segment_cache.size() >=
      kMaxSegmentSafetySegmentCacheEntries) {
    s_segment_safety_segment_cache.clear();
  }
  s_segment_safety_segment_cache[key] = value;
  if (stats) ++stats->segment_cache_stores;
}

bool LookupSegmentSafetySegmentCache(const std::string& key,
                                     CachedSegmentSafetyResult* value) {
  wxMutexLocker lock(s_segment_safety_cache_mutex);
  std::map<std::string, CachedSegmentSafetyResult>::const_iterator it =
      s_segment_safety_segment_cache.find(key);
  if (it == s_segment_safety_segment_cache.end()) return false;
  if (value) *value = it->second;
  return true;
}

void CopyCachedSegmentSafetyToResult(const CachedSegmentSafetyResult& cached,
                                     PlugInSegmentSafetyResult* result) {
  if (!result) return;
  SetSegmentSafetyStatus(result, (PlugInSegmentSafetyStatus)cached.status);
  SetSegmentSafetySource(result, (PlugInSegmentSafetySource)cached.source);
  SetSegmentSafetyDiagnosticReason(
      result, (PlugInSegmentSafetyDiagnosticReason)cached.diagnostic_reason);
  SetSegmentSafetyMessage(result, cached.message);
  if (!SegmentSafetyResultHas(result,
                              offsetof(PlugInSegmentSafetyResult, hit_object),
                              sizeof(result->hit_object)))
    return;

  result->chart_db_index = cached.chart_db_index;
  result->chart_scale = cached.chart_scale;
  result->hit_sample_lat = cached.hit_sample_lat;
  result->hit_sample_lon = cached.hit_sample_lon;
  result->hit_sample_index = cached.hit_sample_index;
  result->hit_sample_count = cached.hit_sample_count;
  CopySegmentSafetyString(result->chart_path, sizeof(result->chart_path),
                          cached.chart_path);
  CopySegmentSafetyString(result->hit_object, sizeof(result->hit_object),
                          cached.hit_object);
  if (SegmentSafetyResultHas(
          result, offsetof(PlugInSegmentSafetyResult, depth_source_attribute),
          sizeof(result->depth_source_attribute))) {
    result->has_depth = cached.has_depth;
    result->min_depth_m = cached.min_depth_m;
    result->required_depth_m = cached.required_depth_m;
    result->hit_depth_m = cached.hit_depth_m;
    result->has_drying = cached.has_drying;
    CopySegmentSafetyString(result->depth_source_object,
                            sizeof(result->depth_source_object),
                            cached.depth_source_object);
    CopySegmentSafetyString(result->depth_source_attribute,
                            sizeof(result->depth_source_attribute),
                            cached.depth_source_attribute);
  }
}

CachedSegmentSafetyResult MakeCachedSegmentSafetyResult(
    const PlugInSegmentSafetyResult* result, int diagnostic_reason_override) {
  CachedSegmentSafetyResult cached;
  if (!result) return cached;
  cached.status = result->status;
  cached.source = result->source;
  cached.diagnostic_reason = diagnostic_reason_override;
  cached.chart_db_index = result->chart_db_index;
  cached.chart_scale = result->chart_scale;
  cached.hit_sample_lat = result->hit_sample_lat;
  cached.hit_sample_lon = result->hit_sample_lon;
  cached.hit_sample_index = result->hit_sample_index;
  cached.hit_sample_count = result->hit_sample_count;
  if (SegmentSafetyResultHas(
          result, offsetof(PlugInSegmentSafetyResult, depth_source_attribute),
          sizeof(result->depth_source_attribute))) {
    cached.has_depth = result->has_depth;
    cached.min_depth_m = result->min_depth_m;
    cached.required_depth_m = result->required_depth_m;
    cached.hit_depth_m = result->hit_depth_m;
    cached.has_drying = result->has_drying;
    CopySegmentSafetyString(cached.depth_source_object,
                            sizeof(cached.depth_source_object),
                            result->depth_source_object);
    CopySegmentSafetyString(cached.depth_source_attribute,
                            sizeof(cached.depth_source_attribute),
                            result->depth_source_attribute);
  }
  CopySegmentSafetyString(cached.message, sizeof(cached.message),
                          result->message);
  CopySegmentSafetyString(cached.chart_path, sizeof(cached.chart_path),
                          result->chart_path);
  CopySegmentSafetyString(cached.hit_object, sizeof(cached.hit_object),
                          result->hit_object);
  return cached;
}

void StoreSegmentSafetyPointCache(
    const std::string& key, const CachedPointSafetyClassification& value) {
  wxMutexLocker lock(s_segment_safety_cache_mutex);
  if (s_segment_safety_point_cache.size() >=
      kMaxSegmentSafetyPointCacheEntries) {
    s_segment_safety_point_cache.clear();
  }
  s_segment_safety_point_cache[key] = value;
}

bool LookupSegmentSafetyPointCache(const std::string& key,
                                   CachedPointSafetyClassification* value) {
  wxMutexLocker lock(s_segment_safety_cache_mutex);
  std::map<std::string, CachedPointSafetyClassification>::const_iterator it =
      s_segment_safety_point_cache.find(key);
  if (it == s_segment_safety_point_cache.end()) return false;
  if (value) *value = it->second;
  return true;
}

void CopySegmentSafetyPointCacheToResult(
    const CachedPointSafetyClassification& cached,
    PlugInSegmentSafetyResult* result) {
  if (!SegmentSafetyResultHas(result,
                              offsetof(PlugInSegmentSafetyResult, hit_object),
                              sizeof(result->hit_object)))
    return;

  result->chart_db_index = cached.chart_db_index;
  result->chart_scale = cached.chart_scale;
  strncpy(result->chart_path, cached.chart_path,
          sizeof(result->chart_path) - 1);
  result->chart_path[sizeof(result->chart_path) - 1] = '\0';
  strncpy(result->hit_object, cached.hit_object,
          sizeof(result->hit_object) - 1);
  result->hit_object[sizeof(result->hit_object) - 1] = '\0';
  if (SegmentSafetyResultHas(
          result, offsetof(PlugInSegmentSafetyResult, depth_source_attribute),
          sizeof(result->depth_source_attribute))) {
    result->has_depth = cached.has_depth ? 1 : 0;
    result->min_depth_m = cached.min_depth_m;
    result->has_drying = cached.has_drying ? 1 : 0;
    strncpy(result->depth_source_object, cached.depth_source_object,
            sizeof(result->depth_source_object) - 1);
    result->depth_source_object[sizeof(result->depth_source_object) - 1] = '\0';
    strncpy(result->depth_source_attribute, cached.depth_source_attribute,
            sizeof(result->depth_source_attribute) - 1);
    result->depth_source_attribute[sizeof(result->depth_source_attribute) - 1] =
        '\0';
  }
}

CachedPointSafetyClassification MakeSegmentSafetyPointCacheEntry(
    SegmentSafetyPointClass point_class, PlugInSegmentSafetySource source,
    int chart_db_index, int chart_scale, const char* chart_path,
    const char* hit_object, bool has_depth = false, double min_depth_m = 0.0,
    bool has_drying = false, const char* depth_source_object = NULL,
    const char* depth_source_attribute = NULL) {
  CachedPointSafetyClassification cached;
  cached.point_class = point_class;
  cached.source = source;
  cached.chart_db_index = chart_db_index;
  cached.chart_scale = chart_scale;
  cached.has_depth = has_depth;
  cached.min_depth_m = min_depth_m;
  cached.has_drying = has_drying;
  if (chart_path) {
    strncpy(cached.chart_path, chart_path, sizeof(cached.chart_path) - 1);
    cached.chart_path[sizeof(cached.chart_path) - 1] = '\0';
  }
  if (hit_object) {
    strncpy(cached.hit_object, hit_object, sizeof(cached.hit_object) - 1);
    cached.hit_object[sizeof(cached.hit_object) - 1] = '\0';
  }
  if (depth_source_object) {
    strncpy(cached.depth_source_object, depth_source_object,
            sizeof(cached.depth_source_object) - 1);
    cached.depth_source_object[sizeof(cached.depth_source_object) - 1] = '\0';
  }
  if (depth_source_attribute) {
    strncpy(cached.depth_source_attribute, depth_source_attribute,
            sizeof(cached.depth_source_attribute) - 1);
    cached.depth_source_attribute[sizeof(cached.depth_source_attribute) - 1] =
        '\0';
  }
  return cached;
}

std::string SegmentSafetyGridTileKeyForIndices(long lat_tile, long lon_tile) {
  char key[96];
  snprintf(key, sizeof(key), "%d:%ld:%ld:%.6f",
           SegmentSafetyCurrentGroupIndex(), lat_tile, lon_tile,
           kSegmentSafetyGridResolutionDegrees);
  key[sizeof(key) - 1] = '\0';
  return std::string(key);
}

std::string SegmentSafetyGridTileKey(double lat, double lon, long* lat_tile,
                                     long* lon_tile) {
  long lt = floor(lat / kSegmentSafetyGridTileDegrees);
  long ln = floor(lon / kSegmentSafetyGridTileDegrees);
  if (lat_tile) *lat_tile = lt;
  if (lon_tile) *lon_tile = ln;
  return SegmentSafetyGridTileKeyForIndices(lt, ln);
}

SegmentSafetyPointClass ChartPointSafetyClassAtRaw(
    double lat, double lon, PlugInSegmentSafetySource* source,
    SegmentSafetyCoreStats* stats, PlugInSegmentSafetyResult* result = NULL);
CachedPointSafetyGridTile BuildSegmentSafetyGridTile(
    double lat, double lon, long lat_tile, long lon_tile,
    SegmentSafetyCoreStats* stats, bool require_depth);

bool SegmentSafetyExternalTileCacheLookup(
    long lat_tile, long lon_tile, bool require_depth,
    CachedPointSafetyGridTile* result) {
  if (!result || !wxThread::IsMain()) return false;
  PlugInSegmentSafetyTileCacheCallbacks callbacks = {};
  {
    wxMutexLocker lock(s_segment_safety_cache_mutex);
    callbacks = s_segment_safety_external_tile_cache;
  }
  if (!callbacks.lookup) return false;

  const int side = static_cast<int>(
                       lround(kSegmentSafetyGridTileDegrees /
                              kSegmentSafetyGridResolutionDegrees)) +
                   1;
  const int cells = side * side;
  CachedPointSafetyGridTile tile;
  tile.hazard_flags.resize(cells);
  tile.has_depth.resize(cells);
  tile.min_depth_m.resize(cells);
  PlugInSegmentSafetyTile external = {};
  external.struct_size = sizeof(external);
  external.group_index = SegmentSafetyCurrentGroupIndex();
  external.lat_tile = lat_tile;
  external.lon_tile = lon_tile;
  external.resolution = kSegmentSafetyGridResolutionDegrees;
  external.rows = side;
  external.cols = side;
  external.hazard_flags = tile.hazard_flags.data();
  external.has_depth = tile.has_depth.data();
  external.min_depth_m = tile.min_depth_m.data();
  external.cell_capacity = cells;
  if (!callbacks.lookup(callbacks.context, lat_tile, lon_tile,
                        require_depth ? 1 : 0, &external))
    return false;

  const uint16_t valid_hazards =
      SEGMENT_SAFETY_HAZARD_LAND | SEGMENT_SAFETY_HAZARD_DRYING |
      SEGMENT_SAFETY_HAZARD_NO_CHART | SEGMENT_SAFETY_HAZARD_UNKNOWN_CLASS;
  if (external.group_index != SegmentSafetyCurrentGroupIndex() ||
      external.lat_tile != lat_tile || external.lon_tile != lon_tile ||
      external.rows != side || external.cols != side ||
      external.cell_capacity < cells ||
      fabs(external.resolution - kSegmentSafetyGridResolutionDegrees) >
          1e-12 ||
      external.source < PI_SEGMENT_SAFETY_SOURCE_NONE ||
      external.source > PI_SEGMENT_SAFETY_SOURCE_PLUGIN_VECTOR ||
      (external.hazard_summary_flags & ~valid_hazards) ||
      (require_depth && !external.depth_complete))
    return false;

  tile.group_index = external.group_index;
  tile.lat_tile = lat_tile;
  tile.lon_tile = lon_tile;
  tile.min_lat = lat_tile * kSegmentSafetyGridTileDegrees;
  tile.min_lon = lon_tile * kSegmentSafetyGridTileDegrees;
  tile.resolution = external.resolution;
  tile.rows = external.rows;
  tile.cols = external.cols;
  tile.chart_db_index = external.chart_db_index;
  tile.chart_scale = external.chart_scale;
  tile.source =
      static_cast<PlugInSegmentSafetySource>(external.source);
  tile.hazard_summary_flags = external.hazard_summary_flags;
  tile.depth_complete = external.depth_complete != 0;
  // Plugin-vector tiles can reach the external cache only after the provider
  // advertised the derived-cache contract in its grid result.
  tile.persistent_cache_allowed = true;
  strncpy(tile.chart_path, external.chart_path, sizeof(tile.chart_path) - 1);
  tile.chart_path[sizeof(tile.chart_path) - 1] = '\0';
  strncpy(tile.dependency_identity, external.dependency_identity,
          sizeof(tile.dependency_identity) - 1);
  tile.dependency_identity[sizeof(tile.dependency_identity) - 1] = '\0';
  tile.classes.resize(cells);
  tile.has_drying.resize(cells);
  tile.land_count = tile.water_count = tile.drying_count =
      tile.unknown_count = 0;
  for (int i = 0; i < cells; ++i) {
    const uint16_t hazards = tile.hazard_flags[i];
    if ((hazards & ~valid_hazards) || tile.has_depth[i] > 1 ||
        (tile.has_depth[i] && !std::isfinite(tile.min_depth_m[i])))
      return false;
    if (hazards & SEGMENT_SAFETY_HAZARD_LAND) {
      tile.classes[i] = SEGMENT_SAFETY_POINT_LAND;
      ++tile.land_count;
    } else if (hazards & SEGMENT_SAFETY_HAZARD_DRYING) {
      tile.classes[i] = SEGMENT_SAFETY_POINT_DRYING;
      ++tile.drying_count;
    } else if (hazards & (SEGMENT_SAFETY_HAZARD_NO_CHART |
                          SEGMENT_SAFETY_HAZARD_UNKNOWN_CLASS)) {
      tile.classes[i] = SEGMENT_SAFETY_POINT_NO_DATA;
      ++tile.unknown_count;
    } else {
      tile.classes[i] = SEGMENT_SAFETY_POINT_WATER;
      ++tile.water_count;
    }
    tile.has_drying[i] =
        (hazards & SEGMENT_SAFETY_HAZARD_DRYING) ? 1 : 0;
  }
  tile.built = true;
  tile.persistent_loaded = true;
  *result = tile;
  return true;
}

wxString SegmentSafetyTileDependencyIdentity(long lat_tile, long lon_tile);
bool SegmentSafetyTileDependencyIsCurrent(
    const CachedPointSafetyGridTile& tile);

void SegmentSafetyExternalTileCacheStore(
    const CachedPointSafetyGridTile& tile) {
  if (!tile.built || tile.persistent_loaded || !wxThread::IsMain()) return;
  if (tile.source == PI_SEGMENT_SAFETY_SOURCE_PLUGIN_VECTOR &&
      !tile.persistent_cache_allowed)
    return;
  PlugInSegmentSafetyTileCacheCallbacks callbacks = {};
  {
    wxMutexLocker lock(s_segment_safety_cache_mutex);
    callbacks = s_segment_safety_external_tile_cache;
  }
  if (!callbacks.store) return;
  const size_t cells = static_cast<size_t>(tile.rows) * tile.cols;
  if (!cells || tile.hazard_flags.size() != cells ||
      tile.has_depth.size() != cells || tile.min_depth_m.size() != cells)
    return;
  PlugInSegmentSafetyTile external = {};
  external.struct_size = sizeof(external);
  external.group_index = tile.group_index;
  external.lat_tile = tile.lat_tile;
  external.lon_tile = tile.lon_tile;
  external.resolution = tile.resolution;
  external.rows = tile.rows;
  external.cols = tile.cols;
  external.chart_db_index = tile.chart_db_index;
  external.chart_scale = tile.chart_scale;
  external.source = tile.source;
  external.hazard_summary_flags = tile.hazard_summary_flags;
  external.depth_complete = tile.depth_complete ? 1 : 0;
  const size_t chart_path_size =
      strnlen(tile.chart_path, sizeof(external.chart_path) - 1);
  memcpy(external.chart_path, tile.chart_path, chart_path_size);
  external.chart_path[chart_path_size] = '\0';
  CopySegmentSafetyString(external.dependency_identity,
                          sizeof(external.dependency_identity),
                          tile.dependency_identity);
  external.hazard_flags =
      const_cast<unsigned short*>(tile.hazard_flags.data());
  external.has_depth =
      const_cast<unsigned char*>(tile.has_depth.data());
  external.min_depth_m = const_cast<float*>(tile.min_depth_m.data());
  external.cell_capacity = static_cast<int>(cells);
  callbacks.store(callbacks.context, &external);
}

void StoreSegmentSafetyGridTile(const std::string& key,
                                const CachedPointSafetyGridTile& tile) {
  CachedPointSafetyGridTile stored = tile;
  if (wxThread::IsMain()) {
    const wxString dependency =
        SegmentSafetyTileDependencyIdentity(tile.lat_tile, tile.lon_tile);
    CopySegmentSafetyString(stored.dependency_identity,
                            sizeof(stored.dependency_identity),
                            dependency.mb_str());
  }
  {
    wxMutexLocker lock(s_segment_safety_cache_mutex);
    if (s_segment_safety_grid_cache.find(key) ==
        s_segment_safety_grid_cache.end()) {
      while (s_segment_safety_grid_cache.size() >=
             kMaxSegmentSafetyGridTiles) {
        std::map<std::string, CachedPointSafetyGridTile>::iterator victim =
            s_segment_safety_grid_cache.end();
        for (std::map<std::string, CachedPointSafetyGridTile>::iterator it =
                 s_segment_safety_grid_cache.begin();
             it != s_segment_safety_grid_cache.end(); ++it) {
          if (s_segment_safety_pinned_grid_keys.find(it->first) ==
              s_segment_safety_pinned_grid_keys.end()) {
            victim = it;
            break;
          }
        }
        if (victim == s_segment_safety_grid_cache.end()) break;
        s_segment_safety_grid_cache.erase(victim);
        ++s_segment_safety_grid_cache_evictions;
      }
    }
    s_segment_safety_grid_cache[key] = stored;
    if (s_segment_safety_persistent_cache_enabled && stored.built &&
        !stored.persistent_loaded &&
        stored.source != PI_SEGMENT_SAFETY_SOURCE_PLUGIN_VECTOR) {
      // Compatibility store for older plugins which use the host-owned
      // persistent cache. New weather-routing builds register an external
      // plugin-owned tile cache instead.
      s_segment_safety_persistent_base_tile_cache[key] = stored;
      while (s_segment_safety_persistent_base_tile_cache.size() >
             kMaxSegmentSafetyPersistentGridTiles)
        s_segment_safety_persistent_base_tile_cache.erase(
            s_segment_safety_persistent_base_tile_cache.begin());
      s_segment_safety_persistent_base_tiles_dirty = true;
      ++s_segment_safety_persistent_tiles_since_checkpoint;
    }
  }
  SegmentSafetyExternalTileCacheStore(stored);
}

bool LookupSegmentSafetyGridTile(const std::string& key,
                                 CachedPointSafetyGridTile* tile) {
  wxMutexLocker lock(s_segment_safety_cache_mutex);
  std::map<std::string, CachedPointSafetyGridTile>::const_iterator it =
      s_segment_safety_grid_cache.find(key);
  if (it == s_segment_safety_grid_cache.end()) return false;
  if (tile) *tile = it->second;
  return true;
}

size_t SegmentSafetyGridCacheSize() {
  wxMutexLocker lock(s_segment_safety_cache_mutex);
  return s_segment_safety_grid_cache.size();
}

void RecordUnexpectedSegmentSafetyTileBuild(SegmentSafetyCoreStats* stats,
                                            long lat_tile, long lon_tile);
bool SegmentSafetyCachedTileProviderIsCurrent(
    const CachedPointSafetyGridTile& tile);

bool EnsureSegmentSafetyGridTile(long lat_tile, long lon_tile,
                                 SegmentSafetyCoreStats* stats,
                                 bool* built = NULL,
                                 bool require_depth = false) {
  if (built) *built = false;
  std::string key = SegmentSafetyGridTileKeyForIndices(lat_tile, lon_tile);
  CachedPointSafetyGridTile active_tile;
  const bool active_found = LookupSegmentSafetyGridTile(key, &active_tile);
  const bool active_current =
      active_found &&
      (!wxThread::IsMain() ||
       SegmentSafetyTileDependencyIsCurrent(active_tile));
  if (active_current && (!require_depth || active_tile.depth_complete)) {
    if (stats) ++stats->grid_cache_hits;
    return true;
  }
  if (active_found &&
      (!active_current ||
       (require_depth && active_tile.built && !active_tile.depth_complete))) {
    wxMutexLocker lock(s_segment_safety_cache_mutex);
    s_segment_safety_grid_cache.erase(key);
    s_segment_safety_persistent_base_tile_cache.erase(key);
    s_segment_safety_persistent_base_tiles_dirty = true;
  }

  if (stats) ++stats->grid_cache_misses;
  if (!wxThread::IsMain()) {
    RecordUnexpectedSegmentSafetyTileBuild(stats, lat_tile, lon_tile);
    bool log_worker_miss = false;
    {
      wxMutexLocker lock(s_segment_safety_cache_mutex);
      if (s_segment_safety_worker_tile_miss_logs <
          kMaxSegmentSafetyWorkerTileMissLogs) {
        ++s_segment_safety_worker_tile_miss_logs;
        log_worker_miss = true;
      }
    }
    if (log_worker_miss) {
      wxLogMessage(
          "WR_GRID_WORKER_TILE_MISS thread=%p main_thread=%p tile=(%ld,%ld) "
          "tile_min=(%.6f,%.6f). Chart object classification is main-thread "
          "only; worker will treat this segment as not chart-safe.",
          wxThread::GetCurrentId(), wxThread::GetMainId(), lat_tile, lon_tile,
          lat_tile * kSegmentSafetyGridTileDegrees,
          lon_tile * kSegmentSafetyGridTileDegrees);
    }
    return false;
  }

  CachedPointSafetyGridTile external_tile;
  if (SegmentSafetyExternalTileCacheLookup(
          lat_tile, lon_tile, require_depth, &external_tile) &&
      SegmentSafetyTileDependencyIsCurrent(external_tile) &&
      SegmentSafetyCachedTileProviderIsCurrent(external_tile)) {
    StoreSegmentSafetyGridTile(key, external_tile);
    if (stats) ++stats->grid_cache_hits;
    return true;
  }

  if (s_segment_safety_persistent_cache_enabled) {
    SegmentSafetyPersistentCacheEnsureLoaded();
    CachedPointSafetyGridTile persistent_tile;
    bool found_persistent = false;
    {
      wxMutexLocker lock(s_segment_safety_cache_mutex);
      std::map<std::string, CachedPointSafetyGridTile>::const_iterator it =
          s_segment_safety_persistent_base_tile_cache.find(key);
      if (it != s_segment_safety_persistent_base_tile_cache.end()) {
        persistent_tile = it->second;
        found_persistent =
            persistent_tile.built &&
            persistent_tile.source != PI_SEGMENT_SAFETY_SOURCE_PLUGIN_VECTOR &&
                           (!require_depth || persistent_tile.depth_complete);
        if (found_persistent) ++s_segment_safety_persistent_base_tiles_used;
      }
    }
    if (found_persistent &&
        SegmentSafetyTileDependencyIsCurrent(persistent_tile) &&
        SegmentSafetyCachedTileProviderIsCurrent(persistent_tile)) {
      StoreSegmentSafetyGridTile(key, persistent_tile);
      if (stats) ++stats->grid_cache_hits;
      return true;
    }
  }

  CachedPointSafetyGridTile tile =
      BuildSegmentSafetyGridTile(lat_tile * kSegmentSafetyGridTileDegrees,
                                 lon_tile * kSegmentSafetyGridTileDegrees,
                                 lat_tile, lon_tile, stats, require_depth);
  StoreSegmentSafetyGridTile(key, tile);
  if (built) *built = true;
  return LookupSegmentSafetyGridTile(key, NULL);
}

void StoreSegmentSafetyRouteMaskTile(
    const std::string& key, const CachedSegmentSafetyRouteMaskTile& tile) {
  wxMutexLocker lock(s_segment_safety_cache_mutex);
  if (s_segment_safety_route_mask_cache.find(key) ==
      s_segment_safety_route_mask_cache.end()) {
    while (s_segment_safety_route_mask_cache.size() >=
           kMaxSegmentSafetyGridTiles) {
      std::map<std::string, CachedSegmentSafetyRouteMaskTile>::iterator victim =
          s_segment_safety_route_mask_cache.end();
      for (std::map<std::string, CachedSegmentSafetyRouteMaskTile>::iterator
               it = s_segment_safety_route_mask_cache.begin();
           it != s_segment_safety_route_mask_cache.end(); ++it) {
        if (s_segment_safety_pinned_route_mask_keys.find(it->first) ==
            s_segment_safety_pinned_route_mask_keys.end()) {
          victim = it;
          break;
        }
      }
      if (victim == s_segment_safety_route_mask_cache.end()) break;
      s_segment_safety_route_mask_cache.erase(victim);
    }
  }
  s_segment_safety_route_mask_cache[key] = tile;
}

bool LookupSegmentSafetyRouteMaskTile(const std::string& key,
                                      CachedSegmentSafetyRouteMaskTile* tile) {
  wxMutexLocker lock(s_segment_safety_cache_mutex);
  std::map<std::string, CachedSegmentSafetyRouteMaskTile>::const_iterator it =
      s_segment_safety_route_mask_cache.find(key);
  if (it == s_segment_safety_route_mask_cache.end()) return false;
  if (tile) *tile = it->second;
  return true;
}

void StoreSegmentSafetyCoarseRouteMaskCell(
    const std::string& key,
    const CachedSegmentSafetyCoarseRouteMaskCell& cell) {
  wxMutexLocker lock(s_segment_safety_cache_mutex);
  if (s_segment_safety_coarse_route_mask_cache.size() >=
      kMaxSegmentSafetyGridTiles)
    s_segment_safety_coarse_route_mask_cache.clear();
  s_segment_safety_coarse_route_mask_cache[key] = cell;
}

bool LookupSegmentSafetyCoarseRouteMaskCell(
    const std::string& key, CachedSegmentSafetyCoarseRouteMaskCell* cell) {
  wxMutexLocker lock(s_segment_safety_cache_mutex);
  std::map<std::string, CachedSegmentSafetyCoarseRouteMaskCell>::const_iterator
      it = s_segment_safety_coarse_route_mask_cache.find(key);
  if (it == s_segment_safety_coarse_route_mask_cache.end()) return false;
  if (cell) *cell = it->second;
  return true;
}

bool BuildSegmentSafetyCoarseRouteMaskCellFromFine(
    long coarse_lat_cell, long coarse_lon_cell, double safety_margin_nm,
    bool check_depth, double minimum_depth_m,
    CachedSegmentSafetyCoarseRouteMaskCell* cell,
    SegmentSafetyCoreStats* stats) {
  if (!cell) return false;

  wxStopWatch timer;
  CachedSegmentSafetyCoarseRouteMaskCell coarse;
  coarse.group_index = SegmentSafetyCurrentGroupIndex();
  coarse.lat_cell = coarse_lat_cell;
  coarse.lon_cell = coarse_lon_cell;
  coarse.min_lat = coarse_lat_cell * coarse.degrees;
  coarse.min_lon = coarse_lon_cell * coarse.degrees;
  coarse.check_depth = check_depth;
  coarse.minimum_depth_m = minimum_depth_m;
  coarse.safety_margin_nm = safety_margin_nm;
  coarse.block_summary_flags = SEGMENT_SAFETY_ROUTE_CLEAR;

  bool missing = false;
  bool all_clear = true;
  long start_lat_tile = coarse_lat_cell * kSegmentSafetyCoarseRouteMaskFactor;
  long start_lon_tile = coarse_lon_cell * kSegmentSafetyCoarseRouteMaskFactor;

  for (int dlat = 0; dlat < kSegmentSafetyCoarseRouteMaskFactor; ++dlat) {
    for (int dlon = 0; dlon < kSegmentSafetyCoarseRouteMaskFactor; ++dlon) {
      long fine_lat_tile = start_lat_tile + dlat;
      long fine_lon_tile = start_lon_tile + dlon;
      std::string mask_key = SegmentSafetyRouteMaskKey(
          fine_lat_tile, fine_lon_tile, safety_margin_nm, check_depth,
          minimum_depth_m);
      CachedSegmentSafetyRouteMaskTile mask;
      if (!LookupSegmentSafetyRouteMaskTile(mask_key, &mask) || !mask.built ||
          mask.block_flags.empty()) {
        missing = true;
        continue;
      }

      ++coarse.fine_tiles_checked;
      if (mask.uses_plugin_vector) {
        coarse.source = PI_SEGMENT_SAFETY_SOURCE_PLUGIN_VECTOR;
        coarse.persistent_cache_allowed =
            coarse.persistent_cache_allowed &&
            mask.persistent_cache_allowed;
      } else if (coarse.source == PI_SEGMENT_SAFETY_SOURCE_NONE)
        coarse.source = mask.source;
      coarse.block_summary_flags |= mask.block_summary_flags;
      bool tile_clear =
          mask.block_summary_flags == SEGMENT_SAFETY_ROUTE_CLEAR &&
          mask.clear_count == mask.rows * mask.cols;
      if (tile_clear) {
        ++coarse.fine_tiles_clear;
      } else {
        ++coarse.fine_tiles_mixed;
        all_clear = false;
      }
    }
  }

  if (missing) {
    coarse.state = SEGMENT_SAFETY_COARSE_MISSING;
    coarse.block_summary_flags |= SEGMENT_SAFETY_ROUTE_NEEDS_TILE;
    return false;
  }

  if (all_clear) {
    coarse.state = SEGMENT_SAFETY_COARSE_CERTIFIED_SAFE;
  } else if (coarse.block_summary_flags & SEGMENT_SAFETY_ROUTE_BLOCK_NO_CHART) {
    coarse.state = SEGMENT_SAFETY_COARSE_NO_CHART;
  } else if (coarse.block_summary_flags &
             SEGMENT_SAFETY_ROUTE_BLOCK_UNKNOWN_DEPTH) {
    coarse.state = SEGMENT_SAFETY_COARSE_DEPTH_UNPROVEN;
  } else {
    coarse.state = SEGMENT_SAFETY_COARSE_MIXED;
  }

  if (stats) stats->coarse_build_ms += timer.Time();
  *cell = coarse;
  return true;
}

bool EnsureSegmentSafetyCoarseRouteMaskCell(
    long coarse_lat_cell, long coarse_lon_cell, double safety_margin_nm,
    bool check_depth, double minimum_depth_m,
    CachedSegmentSafetyCoarseRouteMaskCell* cell,
    SegmentSafetyCoreStats* stats) {
  std::string key = SegmentSafetyCoarseRouteMaskKey(
      coarse_lat_cell, coarse_lon_cell, safety_margin_nm, check_depth,
      minimum_depth_m);
  if (LookupSegmentSafetyCoarseRouteMaskCell(key, cell)) return true;

  CachedSegmentSafetyCoarseRouteMaskCell persistent;
  if (SegmentSafetyPersistentLookupCertifiedSafe(
          coarse_lat_cell, coarse_lon_cell, safety_margin_nm, check_depth,
          minimum_depth_m, &persistent)) {
    StoreSegmentSafetyCoarseRouteMaskCell(key, persistent);
    if (cell) *cell = persistent;
    return true;
  }

  CachedSegmentSafetyCoarseRouteMaskCell built;
  if (!BuildSegmentSafetyCoarseRouteMaskCellFromFine(
          coarse_lat_cell, coarse_lon_cell, safety_margin_nm, check_depth,
          minimum_depth_m, &built, stats))
    return false;
  StoreSegmentSafetyCoarseRouteMaskCell(key, built);
  SegmentSafetyPersistentStoreCertifiedSafe(built);
  if (cell) *cell = built;
  return true;
}

bool LookupCertifiedSegmentSafetyCoarseRouteMaskCellForFineTile(
    long lat_tile, long lon_tile, double safety_margin_nm, bool check_depth,
    double minimum_depth_m, CachedSegmentSafetyCoarseRouteMaskCell* cell) {
  long coarse_lat_cell =
      (long)floor((double)lat_tile / kSegmentSafetyCoarseRouteMaskFactor);
  long coarse_lon_cell =
      (long)floor((double)lon_tile / kSegmentSafetyCoarseRouteMaskFactor);
  std::string key = SegmentSafetyCoarseRouteMaskKey(
      coarse_lat_cell, coarse_lon_cell, safety_margin_nm, check_depth,
      minimum_depth_m);

  CachedSegmentSafetyCoarseRouteMaskCell coarse;
  if (LookupSegmentSafetyCoarseRouteMaskCell(key, &coarse)) {
    if (coarse.state == SEGMENT_SAFETY_COARSE_CERTIFIED_SAFE) {
      if (cell) *cell = coarse;
      return true;
    }
  }
  if (SegmentSafetyPersistentLookupCertifiedSafe(
          coarse_lat_cell, coarse_lon_cell, safety_margin_nm, check_depth,
          minimum_depth_m, &coarse)) {
    StoreSegmentSafetyCoarseRouteMaskCell(key, coarse);
    if (coarse.state == SEGMENT_SAFETY_COARSE_CERTIFIED_SAFE) {
      if (cell) *cell = coarse;
      return true;
    }
  }
  return false;
}

CachedSegmentSafetyRouteMaskTile BuildPersistentCertifiedSafeRouteMaskTile(
    long lat_tile, long lon_tile, double safety_margin_nm, bool check_depth,
    double minimum_depth_m,
    const CachedSegmentSafetyCoarseRouteMaskCell& coarse) {
  CachedSegmentSafetyRouteMaskTile mask;
  mask.group_index = SegmentSafetyCurrentGroupIndex();
  mask.lat_tile = lat_tile;
  mask.lon_tile = lon_tile;
  mask.min_lat = lat_tile * kSegmentSafetyGridTileDegrees;
  mask.min_lon = lon_tile * kSegmentSafetyGridTileDegrees;
  mask.resolution = kSegmentSafetyGridResolutionDegrees;
  mask.rows = (int)lround(kSegmentSafetyGridTileDegrees /
                          kSegmentSafetyGridResolutionDegrees) +
              1;
  mask.cols = mask.rows;
  mask.check_depth = check_depth;
  mask.minimum_depth_m = minimum_depth_m;
  mask.safety_margin_nm = safety_margin_nm;
  mask.margin_cells = 0;
  mask.built = true;
  mask.source = coarse.source != PI_SEGMENT_SAFETY_SOURCE_NONE
                    ? coarse.source
                    : PI_SEGMENT_SAFETY_SOURCE_VECTOR_CHART;
  mask.chart_db_index = -1;
  mask.chart_scale = -1;
  snprintf(mask.chart_path, sizeof(mask.chart_path),
           "persistent certified safe coarse cell");
  mask.block_flags.assign(mask.rows * mask.cols, SEGMENT_SAFETY_ROUTE_CLEAR);
  mask.block_summary_flags = SEGMENT_SAFETY_ROUTE_CLEAR;
  mask.clear_count = mask.rows * mask.cols;
  mask.authoritative_fine = false;
  mask.persistent_certified_safe = true;
  mask.uses_plugin_vector =
      coarse.source == PI_SEGMENT_SAFETY_SOURCE_PLUGIN_VECTOR;
  mask.persistent_cache_allowed = coarse.persistent_cache_allowed;
  return mask;
}

uint16_t SegmentSafetyRouteMaskFlagsForBaseCell(
    const CachedPointSafetyGridTile& tile, int cell_index, bool check_depth,
    double minimum_depth_m) {
  uint16_t flags = SEGMENT_SAFETY_ROUTE_CLEAR;
  uint16_t hazards =
      cell_index >= 0 && cell_index < (int)tile.hazard_flags.size()
          ? tile.hazard_flags[cell_index]
          : (uint16_t)SEGMENT_SAFETY_HAZARD_NO_CHART;

  if (hazards & SEGMENT_SAFETY_HAZARD_LAND)
    flags |= SEGMENT_SAFETY_ROUTE_BLOCK_LAND;
  if (hazards & SEGMENT_SAFETY_HAZARD_DRYING)
    flags |= SEGMENT_SAFETY_ROUTE_BLOCK_DRYING;
  if (hazards & SEGMENT_SAFETY_HAZARD_NO_CHART)
    flags |= SEGMENT_SAFETY_ROUTE_BLOCK_NO_CHART;
  if (hazards & SEGMENT_SAFETY_HAZARD_UNKNOWN_CLASS)
    flags |= SEGMENT_SAFETY_ROUTE_BLOCK_UNKNOWN_CLASS;

  if (check_depth) {
    bool has_depth = cell_index >= 0 &&
                     cell_index < (int)tile.has_depth.size() &&
                     tile.has_depth[cell_index] != 0;
    if (!has_depth) {
      if (!(flags & (SEGMENT_SAFETY_ROUTE_BLOCK_LAND |
                     SEGMENT_SAFETY_ROUTE_BLOCK_DRYING |
                     SEGMENT_SAFETY_ROUTE_BLOCK_NO_CHART)))
        flags |= SEGMENT_SAFETY_ROUTE_BLOCK_UNKNOWN_DEPTH;
    } else if (cell_index >= 0 && cell_index < (int)tile.min_depth_m.size() &&
               tile.min_depth_m[cell_index] < minimum_depth_m) {
      flags |= SEGMENT_SAFETY_ROUTE_BLOCK_TOO_SHALLOW;
    }
  }

  return flags;
}

// Load an exact point-safety tile from either the active cache or the
// persistent cache without consulting chart objects.  Coarse certification
// uses this first so that a proof cannot accidentally make sparse route
// footprints more expensive than their fine-mask fallback.
bool LoadSegmentSafetyGridTileWithoutBuilding(long lat_tile, long lon_tile,
                                              CachedPointSafetyGridTile* tile) {
  const std::string key =
      SegmentSafetyGridTileKeyForIndices(lat_tile, lon_tile);
  if (LookupSegmentSafetyGridTile(key, tile)) return true;
  CachedPointSafetyGridTile external;
  if (SegmentSafetyExternalTileCacheLookup(lat_tile, lon_tile, false,
                                           &external)) {
    StoreSegmentSafetyGridTile(key, external);
    if (tile) *tile = external;
    return true;
  }
  if (!s_segment_safety_persistent_cache_enabled) return false;

  SegmentSafetyPersistentCacheEnsureLoaded();
  CachedPointSafetyGridTile persistent;
  bool found = false;
  {
    wxMutexLocker lock(s_segment_safety_cache_mutex);
    std::map<std::string, CachedPointSafetyGridTile>::const_iterator it =
        s_segment_safety_persistent_base_tile_cache.find(key);
    if (it != s_segment_safety_persistent_base_tile_cache.end() &&
        it->second.built &&
        it->second.source != PI_SEGMENT_SAFETY_SOURCE_PLUGIN_VECTOR) {
      persistent = it->second;
      found = true;
      ++s_segment_safety_persistent_base_tiles_used;
    }
  }
  if (!found) return false;
  StoreSegmentSafetyGridTile(key, persistent);
  if (tile) *tile = persistent;
  return true;
}

bool SegmentSafetyBaseTileHasClearTargetCells(
    const CachedPointSafetyGridTile& tile, bool check_depth,
    double minimum_depth_m, uint32_t* summary_flags) {
  bool clear = tile.built && !tile.classes.empty();
  if (!clear) {
    if (summary_flags) *summary_flags |= SEGMENT_SAFETY_ROUTE_NEEDS_TILE;
    return false;
  }

  for (int i = 0; i < tile.rows * tile.cols; ++i) {
    const uint16_t flags = SegmentSafetyRouteMaskFlagsForBaseCell(
        tile, i, check_depth, minimum_depth_m);
    if (summary_flags) *summary_flags |= flags;
    if (flags != SEGMENT_SAFETY_ROUTE_CLEAR) clear = false;
  }
  return clear;
}

bool SegmentSafetyBaseTileCanContributeMargin(
    const CachedPointSafetyGridTile& tile, bool check_depth,
    double minimum_depth_m) {
  if (!tile.built || tile.classes.empty()) return false;
  if (tile.hazard_summary_flags &
      (SEGMENT_SAFETY_HAZARD_LAND | SEGMENT_SAFETY_HAZARD_DRYING))
    return true;
  if (!check_depth) return false;

  for (int i = 0; i < tile.rows * tile.cols; ++i) {
    const uint16_t flags =
        SegmentSafetyRouteMaskFlagsForBaseCell(tile, i, true, minimum_depth_m);
    if (flags & SEGMENT_SAFETY_ROUTE_BLOCK_TOO_SHALLOW) return true;
  }
  return false;
}

// Build a conservative coarse certificate directly from the exact base-grid
// evidence.  A cell is certified only when every target cell is clear and the
// complete tile halo which can influence the configured margin contains no
// land, drying or too-shallow cell.  Missing evidence never becomes a proof.
bool BuildSegmentSafetyCoarseRouteMaskCellFromBase(
    long coarse_lat_cell, long coarse_lon_cell, double safety_margin_nm,
    bool check_depth, double minimum_depth_m, bool allow_chart_builds,
    CachedSegmentSafetyCoarseRouteMaskCell* cell, SegmentSafetyCoreStats* stats,
    long* base_tiles_built) {
  if (!cell || !wxThread::IsMain()) return false;

  wxStopWatch timer;
  CachedSegmentSafetyCoarseRouteMaskCell coarse;
  coarse.group_index = SegmentSafetyCurrentGroupIndex();
  coarse.lat_cell = coarse_lat_cell;
  coarse.lon_cell = coarse_lon_cell;
  coarse.min_lat = coarse_lat_cell * coarse.degrees;
  coarse.min_lon = coarse_lon_cell * coarse.degrees;
  coarse.check_depth = check_depth;
  coarse.minimum_depth_m = minimum_depth_m;
  coarse.safety_margin_nm = safety_margin_nm;
  coarse.block_summary_flags = SEGMENT_SAFETY_ROUTE_CLEAR;

  const long start_lat_tile =
      coarse_lat_cell * kSegmentSafetyCoarseRouteMaskFactor;
  const long start_lon_tile =
      coarse_lon_cell * kSegmentSafetyCoarseRouteMaskFactor;
  const double min_cell_lat = start_lat_tile * kSegmentSafetyGridTileDegrees;
  const double max_cell_lat =
      (start_lat_tile + kSegmentSafetyCoarseRouteMaskFactor) *
      kSegmentSafetyGridTileDegrees;
  const int margin_radius = SegmentSafetyMarginTileRadius(
      safety_margin_nm, wxMax(fabs(min_cell_lat), fabs(max_cell_lat)));

  bool all_targets_clear = true;
  bool margin_hit = false;
  for (int dlat = -margin_radius;
       dlat < kSegmentSafetyCoarseRouteMaskFactor + margin_radius; ++dlat) {
    for (int dlon = -margin_radius;
         dlon < kSegmentSafetyCoarseRouteMaskFactor + margin_radius; ++dlon) {
      const long lat_tile = start_lat_tile + dlat;
      const long lon_tile = start_lon_tile + dlon;
      CachedPointSafetyGridTile base;
      bool available =
          LoadSegmentSafetyGridTileWithoutBuilding(lat_tile, lon_tile, &base);
      if (!available && allow_chart_builds) {
        bool built = false;
        available =
            EnsureSegmentSafetyGridTile(lat_tile, lon_tile, stats, &built,
                                        check_depth) &&
            LookupSegmentSafetyGridTile(
                SegmentSafetyGridTileKeyForIndices(lat_tile, lon_tile), &base);
        if (built && base_tiles_built) ++*base_tiles_built;
      }
      if (!available) return false;

      if (base.source == PI_SEGMENT_SAFETY_SOURCE_PLUGIN_VECTOR) {
        coarse.source = PI_SEGMENT_SAFETY_SOURCE_PLUGIN_VECTOR;
        coarse.persistent_cache_allowed =
            coarse.persistent_cache_allowed &&
            base.persistent_cache_allowed;
      }

      const bool target =
          dlat >= 0 && dlat < kSegmentSafetyCoarseRouteMaskFactor &&
          dlon >= 0 && dlon < kSegmentSafetyCoarseRouteMaskFactor;
      if (target) {
        ++coarse.fine_tiles_checked;
        if (coarse.source == PI_SEGMENT_SAFETY_SOURCE_NONE)
          coarse.source = base.source;
        if (SegmentSafetyBaseTileHasClearTargetCells(
                base, check_depth, minimum_depth_m,
                &coarse.block_summary_flags)) {
          ++coarse.fine_tiles_clear;
        } else {
          ++coarse.fine_tiles_mixed;
          all_targets_clear = false;
        }
      }

      if (margin_radius > 0 && SegmentSafetyBaseTileCanContributeMargin(
                                   base, check_depth, minimum_depth_m))
        margin_hit = true;
    }
  }

  if (margin_hit) {
    coarse.block_summary_flags |= SEGMENT_SAFETY_ROUTE_BLOCK_MARGIN;
    all_targets_clear = false;
  }

  if (all_targets_clear) {
    coarse.state = SEGMENT_SAFETY_COARSE_CERTIFIED_SAFE;
  } else if (coarse.block_summary_flags & SEGMENT_SAFETY_ROUTE_BLOCK_NO_CHART) {
    coarse.state = SEGMENT_SAFETY_COARSE_NO_CHART;
  } else if (coarse.block_summary_flags &
             SEGMENT_SAFETY_ROUTE_BLOCK_UNKNOWN_DEPTH) {
    coarse.state = SEGMENT_SAFETY_COARSE_DEPTH_UNPROVEN;
  } else {
    coarse.state = SEGMENT_SAFETY_COARSE_MIXED;
  }

  if (stats) stats->coarse_build_ms += timer.Time();
  *cell = coarse;
  return true;
}

void AddSegmentSafetyTileHalo(long lat_tile, long lon_tile, int radius,
                              std::set<std::pair<long, long> >* tiles) {
  if (!tiles) return;
  for (int dlat = -radius; dlat <= radius; ++dlat) {
    for (int dlon = -radius; dlon <= radius; ++dlon)
      tiles->insert(std::make_pair(lat_tile + dlat, lon_tile + dlon));
  }
}

long CountSegmentSafetyTilesUnavailableWithoutBuilding(
    const std::set<std::pair<long, long> >& tiles) {
  long missing = 0;
  for (std::set<std::pair<long, long> >::const_iterator it = tiles.begin();
       it != tiles.end(); ++it) {
    if (!LoadSegmentSafetyGridTileWithoutBuilding(it->first, it->second, NULL))
      ++missing;
  }
  return missing;
}

// Compare the chart-object work required by a coarse proof with the work the
// fine fallback is already committed to for the occupied tiles.  This makes
// coarse-first deterministic and prevents it from widening sparse corridors
// merely to obtain a performance certificate.
bool SegmentSafetyCoarseProofDoesNotExpandBaseBuildSet(
    long coarse_lat_cell, long coarse_lon_cell,
    const std::set<std::pair<long, long> >& occupied_tiles,
    double safety_margin_nm) {
  const long start_lat_tile =
      coarse_lat_cell * kSegmentSafetyCoarseRouteMaskFactor;
  const long start_lon_tile =
      coarse_lon_cell * kSegmentSafetyCoarseRouteMaskFactor;
  const double min_cell_lat = start_lat_tile * kSegmentSafetyGridTileDegrees;
  const double max_cell_lat =
      (start_lat_tile + kSegmentSafetyCoarseRouteMaskFactor) *
      kSegmentSafetyGridTileDegrees;
  const int coarse_radius = SegmentSafetyMarginTileRadius(
      safety_margin_nm, wxMax(fabs(min_cell_lat), fabs(max_cell_lat)));

  std::set<std::pair<long, long> > coarse_required;
  for (int dlat = 0; dlat < kSegmentSafetyCoarseRouteMaskFactor; ++dlat) {
    for (int dlon = 0; dlon < kSegmentSafetyCoarseRouteMaskFactor; ++dlon) {
      AddSegmentSafetyTileHalo(start_lat_tile + dlat, start_lon_tile + dlon,
                               coarse_radius, &coarse_required);
    }
  }

  std::set<std::pair<long, long> > fine_required;
  for (std::set<std::pair<long, long> >::const_iterator it =
           occupied_tiles.begin();
       it != occupied_tiles.end(); ++it) {
    const double tile_min_lat = it->first * kSegmentSafetyGridTileDegrees;
    const int fine_radius = SegmentSafetyMarginTileRadius(
        safety_margin_nm,
        wxMax(fabs(tile_min_lat),
              fabs(tile_min_lat + kSegmentSafetyGridTileDegrees)));
    AddSegmentSafetyTileHalo(it->first, it->second, fine_radius,
                             &fine_required);
  }

  return CountSegmentSafetyTilesUnavailableWithoutBuilding(coarse_required) <=
         CountSegmentSafetyTilesUnavailableWithoutBuilding(fine_required);
}

bool SegmentSafetyBaseCellFlagsAt(long lat_cell, long lon_cell,
                                  bool check_depth, double minimum_depth_m,
                                  SegmentSafetyCoreStats* stats,
                                  uint16_t* flags,
                                  PlugInSegmentSafetySource* source,
                                  int* chart_db_index, int* chart_scale,
                                  const char** chart_path) {
  double lat = lat_cell * kSegmentSafetyGridResolutionDegrees;
  double lon = lon_cell * kSegmentSafetyGridResolutionDegrees;
  long lat_tile = 0;
  long lon_tile = 0;
  std::string key = SegmentSafetyGridTileKey(lat, lon, &lat_tile, &lon_tile);
  if (!LookupSegmentSafetyGridTile(key, NULL)) {
    bool built = false;
    if (!EnsureSegmentSafetyGridTile(lat_tile, lon_tile, stats, &built,
                                     check_depth))
      return false;
    if (built)
      RecordUnexpectedSegmentSafetyTileBuild(stats, lat_tile, lon_tile);
  } else if (stats) {
    ++stats->grid_cache_hits;
  }

  CachedPointSafetyGridTile tile;
  if (!LookupSegmentSafetyGridTile(key, &tile) || tile.classes.empty())
    return false;

  int row = (int)lround((lat - tile.min_lat) / tile.resolution);
  int col = (int)lround((lon - tile.min_lon) / tile.resolution);
  if (row < 0 || row >= tile.rows || col < 0 || col >= tile.cols) return false;

  int cell_index = row * tile.cols + col;
  if (flags)
    *flags = SegmentSafetyRouteMaskFlagsForBaseCell(
        tile, cell_index, check_depth, minimum_depth_m);
  if (source) *source = tile.source;
  if (chart_db_index) *chart_db_index = tile.chart_db_index;
  if (chart_scale) *chart_scale = tile.chart_scale;
  if (chart_path) *chart_path = tile.chart_path;
  return true;
}

CachedSegmentSafetyRouteMaskTile BuildSegmentSafetyRouteMaskTile(
    long lat_tile, long lon_tile, double safety_margin_nm, bool check_depth,
    double minimum_depth_m, SegmentSafetyCoreStats* stats) {
  CachedSegmentSafetyRouteMaskTile mask;
  if (!wxThread::IsMain()) {
    RecordUnexpectedSegmentSafetyTileBuild(stats, lat_tile, lon_tile);
    return mask;
  }

  wxStopWatch timer;
  CachedPointSafetyGridTile base_tile;
  std::string base_key = SegmentSafetyGridTileKeyForIndices(lat_tile, lon_tile);
  bool base_built = false;
  if (!LookupSegmentSafetyGridTile(base_key, &base_tile) ||
      (check_depth && !base_tile.depth_complete)) {
    if (!EnsureSegmentSafetyGridTile(lat_tile, lon_tile, stats, &base_built,
                                     check_depth))
      return mask;
    LookupSegmentSafetyGridTile(base_key, &base_tile);
  }
  if (base_tile.classes.empty()) return mask;

  mask.group_index = SegmentSafetyCurrentGroupIndex();
  mask.lat_tile = lat_tile;
  mask.lon_tile = lon_tile;
  mask.min_lat = base_tile.min_lat;
  mask.min_lon = base_tile.min_lon;
  mask.resolution = base_tile.resolution;
  mask.rows = base_tile.rows;
  mask.cols = base_tile.cols;
  mask.check_depth = check_depth;
  mask.minimum_depth_m = minimum_depth_m;
  mask.safety_margin_nm = safety_margin_nm;
  mask.built = true;
  mask.authoritative_fine = true;
  mask.persistent_certified_safe = false;
  mask.uses_plugin_vector =
      base_tile.source == PI_SEGMENT_SAFETY_SOURCE_PLUGIN_VECTOR;
  mask.persistent_cache_allowed =
      !mask.uses_plugin_vector || base_tile.persistent_cache_allowed;
  mask.source = base_tile.source;
  mask.chart_db_index = base_tile.chart_db_index;
  mask.chart_scale = base_tile.chart_scale;
  snprintf(mask.chart_path, sizeof(mask.chart_path), "%s",
           base_tile.chart_path);
  mask.block_flags.assign(mask.rows * mask.cols,
                          SEGMENT_SAFETY_ROUTE_NEEDS_TILE);

  double mid_lat = mask.min_lat + kSegmentSafetyGridTileDegrees / 2.0;
  double cell_nm =
      wxMin(mask.resolution * 60.0,
            mask.resolution * 60.0 *
                wxMax(0.1, fabs(cos(SegmentSafetyDegToRad(mid_lat)))));
  mask.margin_cells = safety_margin_nm > 0.0
                          ? (int)ceil(safety_margin_nm / wxMax(0.01, cell_nm))
                          : 0;
  mask.margin_cells = wxMin(mask.margin_cells, 128);

  auto CountRouteMaskFlags = [&mask](uint16_t flags) {
    mask.block_summary_flags |= flags;
    if (flags == SEGMENT_SAFETY_ROUTE_CLEAR) ++mask.clear_count;
    if (flags & SEGMENT_SAFETY_ROUTE_BLOCK_LAND) ++mask.land_count;
    if (flags & SEGMENT_SAFETY_ROUTE_BLOCK_DRYING) ++mask.drying_count;
    if (flags & SEGMENT_SAFETY_ROUTE_BLOCK_TOO_SHALLOW) ++mask.shallow_count;
    if (flags & SEGMENT_SAFETY_ROUTE_BLOCK_UNKNOWN_DEPTH)
      ++mask.unknown_depth_count;
    if (flags & SEGMENT_SAFETY_ROUTE_BLOCK_NO_CHART) ++mask.no_chart_count;
    if (flags & SEGMENT_SAFETY_ROUTE_BLOCK_MARGIN) ++mask.margin_count;
  };

  // Most route checks are open-water tiles.  If this tile has no chart-derived
  // hazards and no adjacent land/drying tile can contribute margin, publish an
  // all-clear derived mask without scanning every neighbouring cell.
  bool used_clear_tile_shortcut = false;
  int prefix_verify_cells = 0;
  int prefix_verify_mismatches = 0;
  bool can_mark_tile_clear = !check_depth && base_tile.hazard_summary_flags ==
                                                 SEGMENT_SAFETY_HAZARD_NONE;
  if (can_mark_tile_clear && mask.margin_cells > 0) {
    int lat_tile_radius =
        wxMax(1, (int)ceil((double)mask.margin_cells / wxMax(1, mask.rows)));
    int lon_tile_radius =
        wxMax(1, (int)ceil((double)mask.margin_cells / wxMax(1, mask.cols)));
    for (int dlat = -lat_tile_radius;
         dlat <= lat_tile_radius && can_mark_tile_clear; ++dlat) {
      for (int dlon = -lon_tile_radius; dlon <= lon_tile_radius; ++dlon) {
        long neighbor_lat_tile = lat_tile + dlat;
        long neighbor_lon_tile = lon_tile + dlon;
        std::string neighbor_key = SegmentSafetyGridTileKeyForIndices(
            neighbor_lat_tile, neighbor_lon_tile);
        CachedPointSafetyGridTile neighbor_tile;
        bool neighbor_built = false;
        if (!LookupSegmentSafetyGridTile(neighbor_key, &neighbor_tile) ||
            (check_depth && !neighbor_tile.depth_complete)) {
          if (!EnsureSegmentSafetyGridTile(neighbor_lat_tile, neighbor_lon_tile,
                                           stats, &neighbor_built,
                                           check_depth) ||
              !LookupSegmentSafetyGridTile(neighbor_key, &neighbor_tile)) {
            can_mark_tile_clear = false;
            break;
          }
          if (neighbor_built)
            RecordUnexpectedSegmentSafetyTileBuild(stats, neighbor_lat_tile,
                                                   neighbor_lon_tile);
        }

        if (neighbor_tile.hazard_summary_flags &
            (SEGMENT_SAFETY_HAZARD_LAND | SEGMENT_SAFETY_HAZARD_DRYING)) {
          can_mark_tile_clear = false;
          break;
        }
        if (neighbor_tile.source == PI_SEGMENT_SAFETY_SOURCE_PLUGIN_VECTOR)
          mask.uses_plugin_vector = true;
      }
    }
  }

  if (can_mark_tile_clear) {
    used_clear_tile_shortcut = true;
    std::fill(mask.block_flags.begin(), mask.block_flags.end(),
              SEGMENT_SAFETY_ROUTE_CLEAR);
    mask.clear_count = mask.rows * mask.cols;
  }

  if (!used_clear_tile_shortcut) {
    if (mask.margin_cells == 0) {
      for (int r = 0; r < mask.rows; ++r) {
        for (int c = 0; c < mask.cols; ++c) {
          const int cell_index = r * mask.cols + c;
          const uint16_t flags = SegmentSafetyRouteMaskFlagsForBaseCell(
              base_tile, cell_index, check_depth, minimum_depth_m);
          mask.block_flags[cell_index] = flags;
          CountRouteMaskFlags(flags);
        }
      }
    } else {
      // Materialize the complete halo once, then use summed-area tables for
      // conservative square-margin dilation.  The previous implementation did
      // a cache lookup (and mutex acquisition) for every neighbour of every
      // target cell: about 600,000 lookups per tile at a 0.4 NM margin.  This
      // is exactly the same square-neighbourhood safety rule in O(cells + halo)
      // time, with missing evidence still failing closed.
      const int halo = mask.margin_cells;
      const int extended_rows = mask.rows + 2 * halo;
      const int extended_cols = mask.cols + 2 * halo;
      const int cells_per_tile = (int)lround(
          kSegmentSafetyGridTileDegrees / kSegmentSafetyGridResolutionDegrees);
      const long first_lat_cell =
          lround(mask.min_lat / kSegmentSafetyGridResolutionDegrees) - halo;
      const long first_lon_cell =
          lround(mask.min_lon / kSegmentSafetyGridResolutionDegrees) - halo;
      std::vector<uint16_t> extended_flags(
          static_cast<size_t>(extended_rows) * extended_cols,
          SEGMENT_SAFETY_ROUTE_NEEDS_TILE);
      std::map<std::pair<long, long>, CachedPointSafetyGridTile> local_tiles;

      const auto FloorTileIndex = [cells_per_tile](long cell) {
        return (long)floor((double)cell / cells_per_tile);
      };
      for (int r = 0; r < extended_rows; ++r) {
        const long global_lat_cell = first_lat_cell + r;
        const long source_lat_tile = FloorTileIndex(global_lat_cell);
        const int source_row =
            (int)(global_lat_cell - source_lat_tile * cells_per_tile);
        for (int c = 0; c < extended_cols; ++c) {
          const long global_lon_cell = first_lon_cell + c;
          const long source_lon_tile = FloorTileIndex(global_lon_cell);
          const int source_col =
              (int)(global_lon_cell - source_lon_tile * cells_per_tile);
          const std::pair<long, long> tile_id(source_lat_tile, source_lon_tile);
          std::map<std::pair<long, long>, CachedPointSafetyGridTile>::iterator
              local = local_tiles.find(tile_id);
          if (local == local_tiles.end()) {
            const std::string source_key = SegmentSafetyGridTileKeyForIndices(
                source_lat_tile, source_lon_tile);
            CachedPointSafetyGridTile source_tile;
            bool source_built = false;
            if ((!LookupSegmentSafetyGridTile(source_key, &source_tile) ||
                 (check_depth && !source_tile.depth_complete)) &&
                EnsureSegmentSafetyGridTile(source_lat_tile, source_lon_tile,
                                            stats, &source_built, check_depth))
              LookupSegmentSafetyGridTile(source_key, &source_tile);
            if (source_built)
              RecordUnexpectedSegmentSafetyTileBuild(stats, source_lat_tile,
                                                     source_lon_tile);
            local =
                local_tiles.insert(std::make_pair(tile_id, source_tile)).first;
          }

          const CachedPointSafetyGridTile& source_tile = local->second;
          if (source_tile.source == PI_SEGMENT_SAFETY_SOURCE_PLUGIN_VECTOR)
            mask.uses_plugin_vector = true;
          if (!source_tile.built || source_tile.classes.empty() ||
              source_row < 0 || source_row >= source_tile.rows ||
              source_col < 0 || source_col >= source_tile.cols)
            continue;
          const int source_index = source_row * source_tile.cols + source_col;
          extended_flags[static_cast<size_t>(r) * extended_cols + c] =
              SegmentSafetyRouteMaskFlagsForBaseCell(
                  source_tile, source_index, check_depth, minimum_depth_m);
        }
      }

      const int prefix_stride = extended_cols + 1;
      std::vector<int> hazard_prefix(
          static_cast<size_t>(extended_rows + 1) * prefix_stride, 0);
      std::vector<int> missing_prefix(
          static_cast<size_t>(extended_rows + 1) * prefix_stride, 0);
      for (int r = 0; r < extended_rows; ++r) {
        int hazard_row_sum = 0;
        int missing_row_sum = 0;
        for (int c = 0; c < extended_cols; ++c) {
          const uint16_t flags =
              extended_flags[static_cast<size_t>(r) * extended_cols + c];
          hazard_row_sum += (flags & (SEGMENT_SAFETY_ROUTE_BLOCK_LAND |
                                      SEGMENT_SAFETY_ROUTE_BLOCK_DRYING |
                                      SEGMENT_SAFETY_ROUTE_BLOCK_TOO_SHALLOW))
                                ? 1
                                : 0;
          missing_row_sum += (flags & SEGMENT_SAFETY_ROUTE_NEEDS_TILE) ? 1 : 0;
          const size_t prefix_index =
              static_cast<size_t>(r + 1) * prefix_stride + c + 1;
          hazard_prefix[prefix_index] =
              hazard_prefix[static_cast<size_t>(r) * prefix_stride + c + 1] +
              hazard_row_sum;
          missing_prefix[prefix_index] =
              missing_prefix[static_cast<size_t>(r) * prefix_stride + c + 1] +
              missing_row_sum;
        }
      }
      const auto RectangleSum = [prefix_stride](const std::vector<int>& prefix,
                                                int min_row, int min_col,
                                                int max_row, int max_col) {
        const size_t a = static_cast<size_t>(min_row) * prefix_stride + min_col;
        const size_t b =
            static_cast<size_t>(min_row) * prefix_stride + max_col + 1;
        const size_t c =
            static_cast<size_t>(max_row + 1) * prefix_stride + min_col;
        const size_t d =
            static_cast<size_t>(max_row + 1) * prefix_stride + max_col + 1;
        return prefix[d] - prefix[b] - prefix[c] + prefix[a];
      };

      for (int r = 0; r < mask.rows; ++r) {
        for (int c = 0; c < mask.cols; ++c) {
          const int extended_row = r + halo;
          const int extended_col = c + halo;
          const int cell_index = r * mask.cols + c;
          uint16_t flags = SegmentSafetyRouteMaskFlagsForBaseCell(
              base_tile, cell_index, check_depth, minimum_depth_m);
          const int min_row = extended_row - halo;
          const int min_col = extended_col - halo;
          const int max_row = extended_row + halo;
          const int max_col = extended_col + halo;
          const uint16_t global_centre_flags =
              extended_flags[static_cast<size_t>(extended_row) * extended_cols +
                             extended_col];
          const int neighbour_missing =
              RectangleSum(missing_prefix, min_row, min_col, max_row, max_col) -
              ((global_centre_flags & SEGMENT_SAFETY_ROUTE_NEEDS_TILE) ? 1 : 0);
          if (neighbour_missing > 0) {
            flags |= SEGMENT_SAFETY_ROUTE_NEEDS_TILE;
          } else {
            const int neighbour_hazards =
                RectangleSum(hazard_prefix, min_row, min_col, max_row,
                             max_col) -
                ((global_centre_flags &
                  (SEGMENT_SAFETY_ROUTE_BLOCK_LAND |
                   SEGMENT_SAFETY_ROUTE_BLOCK_DRYING |
                   SEGMENT_SAFETY_ROUTE_BLOCK_TOO_SHALLOW))
                     ? 1
                     : 0);
            if (neighbour_hazards > 0)
              flags |= SEGMENT_SAFETY_ROUTE_BLOCK_MARGIN;
          }

          mask.block_flags[cell_index] = flags;
          CountRouteMaskFlags(flags);
        }
      }

      const char* verify_prefix =
          getenv("WR_SEGMENT_SAFETY_VERIFY_MASK_PREFIX");
      if (verify_prefix && !strcmp(verify_prefix, "1")) {
        for (int r = 0; r < mask.rows; ++r) {
          for (int c = 0; c < mask.cols; ++c) {
            const int cell_index = r * mask.cols + c;
            uint16_t reference_flags = SegmentSafetyRouteMaskFlagsForBaseCell(
                base_tile, cell_index, check_depth, minimum_depth_m);
            bool margin_hit = false;
            for (int dlat = -halo; dlat <= halo && !margin_hit; ++dlat) {
              for (int dlon = -halo; dlon <= halo; ++dlon) {
                if (dlat == 0 && dlon == 0) continue;
                const uint16_t neighbour_flags =
                    extended_flags[static_cast<size_t>(r + halo + dlat) *
                                       extended_cols +
                                   c + halo + dlon];
                if (neighbour_flags & SEGMENT_SAFETY_ROUTE_NEEDS_TILE) {
                  reference_flags |= SEGMENT_SAFETY_ROUTE_NEEDS_TILE;
                  margin_hit = true;
                  break;
                }
                if (neighbour_flags &
                    (SEGMENT_SAFETY_ROUTE_BLOCK_LAND |
                     SEGMENT_SAFETY_ROUTE_BLOCK_DRYING |
                     SEGMENT_SAFETY_ROUTE_BLOCK_TOO_SHALLOW)) {
                  margin_hit = true;
                  break;
                }
              }
            }
            if (margin_hit &&
                !(reference_flags & SEGMENT_SAFETY_ROUTE_NEEDS_TILE))
              reference_flags |= SEGMENT_SAFETY_ROUTE_BLOCK_MARGIN;
            ++prefix_verify_cells;
            if (reference_flags != mask.block_flags[cell_index])
              ++prefix_verify_mismatches;
          }
        }
        if (prefix_verify_mismatches > 0) mask.built = false;
      }
    }
  } else {
    CountRouteMaskFlags(SEGMENT_SAFETY_ROUTE_CLEAR);
    mask.clear_count = mask.rows * mask.cols;
  }

  wxLogMessage(
      "WR_ROUTE_MASK_BUILD key=%ld:%ld group=%d margin_nm=%.3f "
      "depth_check=%d min_depth_m=%.2f cells=%d clear=%d land=%d drying=%d "
      "shallow=%d unknown_depth=%d no_chart=%d margin=%d summary_flags=%u "
      "base_built=%d clear_shortcut=%d prefix_verify_cells=%d "
      "prefix_verify_mismatches=%d build_ms=%d",
      lat_tile, lon_tile, mask.group_index, safety_margin_nm,
      check_depth ? 1 : 0, minimum_depth_m, mask.rows * mask.cols,
      mask.clear_count, mask.land_count, mask.drying_count, mask.shallow_count,
      mask.unknown_depth_count, mask.no_chart_count, mask.margin_count,
      mask.block_summary_flags, base_built ? 1 : 0,
      used_clear_tile_shortcut ? 1 : 0, prefix_verify_cells,
      prefix_verify_mismatches, timer.Time());
  return mask;
}

void QueueSegmentSafetyRouteMaskRequest(long lat_tile, long lon_tile,
                                        double safety_margin_nm,
                                        bool check_depth,
                                        double minimum_depth_m,
                                        bool force_authoritative_fine) {
  SegmentSafetyRouteMaskRequest request;
  request.key = SegmentSafetyRouteMaskKey(lat_tile, lon_tile, safety_margin_nm,
                                          check_depth, minimum_depth_m);
  request.group_index = SegmentSafetyCurrentGroupIndex();
  request.lat_tile = lat_tile;
  request.lon_tile = lon_tile;
  request.safety_margin_nm = safety_margin_nm;
  request.check_depth = check_depth;
  request.minimum_depth_m = minimum_depth_m;
  request.force_authoritative_fine = force_authoritative_fine;

  wxMutexLocker lock(s_segment_safety_cache_mutex);
  if (s_segment_safety_inflight_route_mask_requests.count(request.key) == 0)
    s_segment_safety_pending_route_mask_requests[request.key] = request;
}

bool EnsureSegmentSafetyRouteMaskTile(long lat_tile, long lon_tile,
                                      double safety_margin_nm, bool check_depth,
                                      double minimum_depth_m,
                                      SegmentSafetyCoreStats* stats,
                                      bool* built = NULL,
                                      bool force_authoritative_fine = false) {
  if (built) *built = false;
  std::string key = SegmentSafetyRouteMaskKey(
      lat_tile, lon_tile, safety_margin_nm, check_depth, minimum_depth_m);
  CachedSegmentSafetyRouteMaskTile existing;
  if (LookupSegmentSafetyRouteMaskTile(key, &existing) &&
      (!force_authoritative_fine || existing.authoritative_fine)) {
    if (stats) ++stats->grid_cache_hits;
    return true;
  }

  if (stats) ++stats->grid_cache_misses;
  if (!wxThread::IsMain()) {
    RecordUnexpectedSegmentSafetyTileBuild(stats, lat_tile, lon_tile);
    QueueSegmentSafetyRouteMaskRequest(lat_tile, lon_tile, safety_margin_nm,
                                       check_depth, minimum_depth_m,
                                       force_authoritative_fine);
    return false;
  }

  CachedSegmentSafetyRouteMaskTile mask =
      BuildSegmentSafetyRouteMaskTile(lat_tile, lon_tile, safety_margin_nm,
                                      check_depth, minimum_depth_m, stats);
  if (!mask.built) return false;
  StoreSegmentSafetyRouteMaskTile(key, mask);
  if (built) *built = true;
  return true;
}

void RecordUnexpectedSegmentSafetyTileBuild(SegmentSafetyCoreStats* stats,
                                            long lat_tile, long lon_tile) {
  if (!stats) return;
  ++stats->unexpected_tile_builds;
  if (stats->unexpected_tile_builds == 1) {
    stats->unexpected_lat_tile = lat_tile;
    stats->unexpected_lon_tile = lon_tile;
    stats->unexpected_tile_min_lat = lat_tile * kSegmentSafetyGridTileDegrees;
    stats->unexpected_tile_min_lon = lon_tile * kSegmentSafetyGridTileDegrees;
  }
}

void SegmentSafetyCandidateChartsAt(double lat, double lon,
                                    std::set<int>& chart_indexes,
                                    SegmentSafetyCoreStats* stats) {
  if (!ChartData) {
    if (stats) stats->no_chart_database = true;
    return;
  }

  ChartStack stack;
  ChartData->BuildChartStack(&stack, lat, lon,
                             SegmentSafetyCurrentGroupIndex());
  if (stats) stats->chart_stack_entries += stack.nEntry;
  for (int i = 0; i < stack.nEntry; ++i) {
    int db_index = stack.GetDBIndex(i);
    if (db_index < 0) continue;
    ChartFamilyEnum family =
        (ChartFamilyEnum)ChartData->GetCSChartFamily(&stack, i);
    ChartTypeEnum type = (ChartTypeEnum)ChartData->GetCSChartType(&stack, i);
    // GetCSChartFamily() derives family from the built-in chart type and
    // returns UNKNOWN for CHART_TYPE_PLUGIN, even when the chart-table entry
    // records a vector family.  Keep plugin entries here and validate the
    // opened wrapper in SegmentSafetySortedChartCandidates().
    if (family == CHART_FAMILY_VECTOR || type == CHART_TYPE_PLUGIN ||
        type == CHART_TYPE_CM93 || type == CHART_TYPE_CM93COMP) {
      chart_indexes.insert(db_index);
    } else if (family == CHART_FAMILY_RASTER) {
      if (stats) ++stats->raster_chart_count;
    } else {
      if (stats) ++stats->unsupported_chart_count;
    }
  }
}

struct SegmentSafetyChartCandidate {
  int db_index;
  int provider_priority;
  int native_scale;
  time_t edition_date;
  time_t file_time;
  bool plugin_vector;
  bool cm93;
  std::string path;

  SegmentSafetyChartCandidate()
      : db_index(-1),
        provider_priority(INT_MAX),
        native_scale(INT_MAX),
        edition_date(0),
        file_time(0),
        plugin_vector(false),
        cm93(false) {}
};

bool SegmentSafetyChartCandidateLess(const SegmentSafetyChartCandidate& a,
                                     const SegmentSafetyChartCandidate& b) {
  // A licensed/native vector chart is authoritative for its coverage.  CM93
  // is retained only as a degraded vector fallback even when its dynamically
  // selected local scale happens to have a smaller denominator.
  if (a.provider_priority != b.provider_priority)
    return a.provider_priority < b.provider_priority;
  if (a.native_scale != b.native_scale) return a.native_scale < b.native_scale;
  if (a.edition_date != b.edition_date) return a.edition_date > b.edition_date;
  if (a.file_time != b.file_time) return a.file_time > b.file_time;
  if (a.path != b.path) return a.path < b.path;
  return a.db_index < b.db_index;
}

bool SegmentSafetyLongitudeRangesOverlap(double first_min, double first_max,
                                         double second_min,
                                         double second_max) {
  for (int shift = -1; shift <= 1; ++shift) {
    const double shifted_min = second_min + shift * 360.0;
    const double shifted_max = second_max + shift * 360.0;
    if (shifted_max >= first_min && shifted_min <= first_max) return true;
  }
  return false;
}

std::vector<SegmentSafetyChartCandidate>
SegmentSafetyTileChartCandidates(double min_lat, double min_lon,
                                 double max_lat, double max_lon) {
  std::vector<SegmentSafetyChartCandidate> candidates;
  if (!ChartData) return candidates;

  const ocpn::chart_safety::GeographicBounds discovery =
      ocpn::chart_safety::ExpandCandidateDiscoveryBounds(
          {min_lat, min_lon, max_lat, max_lon},
          kSegmentSafetyGridResolutionDegrees);

  const int group_index = SegmentSafetyCurrentGroupIndex();
  const int entries = ChartData->GetChartTableEntries();
  candidates.reserve(entries);
  for (int db_index = 0; db_index < entries; ++db_index) {
    ChartTableEntry& entry = ChartData->GetChartTableEntry(db_index);
    const ChartTypeEnum type =
        static_cast<ChartTypeEnum>(entry.GetChartType());
    const ChartFamilyEnum family =
        static_cast<ChartFamilyEnum>(entry.GetChartFamily());
    const bool plugin_vector =
        type == CHART_TYPE_PLUGIN && family == CHART_FAMILY_VECTOR;
    const bool cm93 =
        type == CHART_TYPE_CM93 || type == CHART_TYPE_CM93COMP;
    if (!plugin_vector && !cm93 && family != CHART_FAMILY_VECTOR) continue;
    if (ChartData->IsChartDirectoryExcluded(entry.GetFullPath())) continue;
    if (!ChartData->IsChartInGroup(db_index, group_index)) continue;
    if (type == CHART_TYPE_PLUGIN &&
        !ChartData->IsChartAvailable(db_index))
      continue;
    if (entry.GetLatMax() < discovery.min_lat ||
        entry.GetLatMin() > discovery.max_lat)
      continue;
    if (!SegmentSafetyLongitudeRangesOverlap(
            entry.GetLonMin(), entry.GetLonMax(), discovery.min_lon,
            discovery.max_lon))
      continue;

    SegmentSafetyChartCandidate candidate;
    candidate.db_index = db_index;
    candidate.provider_priority = cm93 ? 1 : 0;
    candidate.native_scale = entry.GetScale();
    candidate.edition_date = entry.GetChartEditionDate();
    candidate.file_time = entry.GetFileTime();
    candidate.plugin_vector = plugin_vector;
    candidate.cm93 = cm93;
    candidate.path = entry.GetFullPath();
    candidates.push_back(candidate);
  }
  std::sort(candidates.begin(), candidates.end(),
            SegmentSafetyChartCandidateLess);
  return candidates;
}

wxString SegmentSafetyTileDependencyIdentity(long lat_tile, long lon_tile) {
  uint64_t hash = 1469598103934665603ULL;
  SegmentSafetyHashAdd(&hash, "tile-dependencies-v1");
  SegmentSafetyHashAdd(
      &hash, wxString::Format("group=%d", SegmentSafetyCurrentGroupIndex()));
  const double min_lat = lat_tile * kSegmentSafetyGridTileDegrees;
  const double min_lon = lon_tile * kSegmentSafetyGridTileDegrees;
  const std::vector<SegmentSafetyChartCandidate> candidates =
      SegmentSafetyTileChartCandidates(
          min_lat, min_lon, min_lat + kSegmentSafetyGridTileDegrees,
          min_lon + kSegmentSafetyGridTileDegrees);
  for (std::vector<SegmentSafetyChartCandidate>::const_iterator it =
           candidates.begin();
       it != candidates.end(); ++it) {
    SegmentSafetyHashAdd(&hash, wxString::FromUTF8(it->path.c_str()));
    SegmentSafetyHashAdd(
        &hash,
        wxString::Format("p=%d:s=%d:e=%lld:f=%lld:plugin=%d:cm93=%d",
                         it->provider_priority, it->native_scale,
                         static_cast<long long>(it->edition_date),
                         static_cast<long long>(it->file_time),
                         it->plugin_vector ? 1 : 0, it->cm93 ? 1 : 0));
  }
  return wxString::Format("tile-v1-%016llx",
                          static_cast<unsigned long long>(hash));
}

bool SegmentSafetyTileDependencyIsCurrent(
    const CachedPointSafetyGridTile& tile) {
  if (!wxThread::IsMain() || !tile.dependency_identity[0]) return false;
  return SegmentSafetyTileDependencyIdentity(tile.lat_tile, tile.lon_tile) ==
         wxString::FromUTF8(tile.dependency_identity);
}

std::vector<SegmentSafetyChartCandidate> SegmentSafetySortedChartCandidates(
    double lat, double lon, const std::set<int>& chart_indexes) {
  std::vector<SegmentSafetyChartCandidate> candidates;
  ViewPort detail_vp = SegmentSafetyHighestDetailViewPortAt(lat, lon);
  for (std::set<int>::const_iterator it = chart_indexes.begin();
       it != chart_indexes.end(); ++it) {
    ChartBase* chart =
        ChartData ? ChartData->OpenChartFromDB(*it, FULL_INIT) : NULL;
    s57chart* s57 = dynamic_cast<s57chart*>(chart);
    const bool plugin_vector = IsSupportedSegmentSafetyPluginChart(chart);
    if (!s57 && !plugin_vector) continue;
    const bool cm93 = IsCm93Chart(chart);
    if (cm93) {
      cm93compchart* cm93_chart = dynamic_cast<cm93compchart*>(chart);
      if (cm93_chart) cm93_chart->SetVPParms(detail_vp);
    }
    SegmentSafetyChartCandidate candidate;
    candidate.db_index = *it;
    candidate.provider_priority = cm93 ? 1 : 0;
    candidate.native_scale = chart->GetNativeScale();
    candidate.plugin_vector = plugin_vector;
    candidate.cm93 = cm93;
    candidate.path = chart->GetFullPath().ToStdString();
    if (ChartData && *it >= 0 && *it < ChartData->GetChartTableEntries()) {
      const ChartTableEntry& entry = ChartData->GetChartTableEntry(*it);
      candidate.edition_date = entry.GetChartEditionDate();
      candidate.file_time = entry.GetFileTime();
    }
    candidates.push_back(candidate);
  }
  std::sort(candidates.begin(), candidates.end(),
            SegmentSafetyChartCandidateLess);
  return candidates;
}

bool SegmentSafetyCachedTileProviderIsCurrent(
    const CachedPointSafetyGridTile& tile) {
  if (!tile.built || tile.source == PI_SEGMENT_SAFETY_SOURCE_PLUGIN_VECTOR)
    return true;

  // A chart-set identity change invalidates the complete persistent store,
  // but provider availability can change while the same chart database is in
  // use (for example, enabling o-charts after a CM93-only run).  Probe the
  // centre and corners of the cached tile.  If current chart selection prefers
  // a different provider class anywhere in the tile, rebuild it instead of
  // allowing stale lower-priority evidence to satisfy an authoritative query.
  const double inset = wxMax(tile.resolution * 0.5, 1e-6);
  const double min_lat = tile.min_lat + inset;
  const double min_lon = tile.min_lon + inset;
  const double max_lat =
      tile.min_lat + kSegmentSafetyGridTileDegrees - inset;
  const double max_lon =
      tile.min_lon + kSegmentSafetyGridTileDegrees - inset;
  const double samples[][2] = {
      {(min_lat + max_lat) / 2.0, (min_lon + max_lon) / 2.0},
      {min_lat, min_lon},
      {min_lat, max_lon},
      {max_lat, min_lon},
      {max_lat, max_lon},
  };

  for (size_t i = 0; i < WXSIZEOF(samples); ++i) {
    std::set<int> chart_indexes;
    SegmentSafetyCandidateChartsAt(samples[i][0], samples[i][1],
                                   chart_indexes, NULL);

    // The common no-o-chart case should remain a cheap cache hit.  Opening
    // charts is only necessary when the chart table contains a provider class
    // which could supersede the cached source at this sample.
    bool provider_may_differ = false;
    for (std::set<int>::const_iterator it = chart_indexes.begin();
         it != chart_indexes.end(); ++it) {
      if (!ChartData || *it < 0 || *it >= ChartData->GetChartTableEntries())
        continue;
      const ChartTypeEnum type = (ChartTypeEnum)
          ChartData->GetChartTableEntry(*it).GetChartType();
      if (tile.source == PI_SEGMENT_SAFETY_SOURCE_CM93) {
        provider_may_differ =
            type != CHART_TYPE_CM93 && type != CHART_TYPE_CM93COMP;
      } else if (tile.source == PI_SEGMENT_SAFETY_SOURCE_VECTOR_CHART) {
        provider_may_differ = type == CHART_TYPE_PLUGIN;
      } else {
        provider_may_differ = true;
      }
      if (provider_may_differ) break;
    }
    if (!provider_may_differ) continue;

    const std::vector<SegmentSafetyChartCandidate> candidates =
        SegmentSafetySortedChartCandidates(samples[i][0], samples[i][1],
                                           chart_indexes);
    if (candidates.empty()) continue;
    const PlugInSegmentSafetySource preferred_source =
        candidates.front().plugin_vector
            ? PI_SEGMENT_SAFETY_SOURCE_PLUGIN_VECTOR
            : (candidates.front().cm93
                   ? PI_SEGMENT_SAFETY_SOURCE_CM93
                   : PI_SEGMENT_SAFETY_SOURCE_VECTOR_CHART);
    if (preferred_source == tile.source) continue;

    static long provider_rejection_logs = 0;
    if (provider_rejection_logs < 20) {
      ++provider_rejection_logs;
      wxLogMessage(
          "WR_CACHED_TILE_PROVIDER_REJECT #%ld tile=(%ld,%ld) "
          "cached_source=%d preferred_source=%d sample=(%.8f,%.8f) "
          "cached_chart_path=\"%s\" preferred_chart_path=\"%s\"",
          provider_rejection_logs, tile.lat_tile, tile.lon_tile,
          (int)tile.source, (int)preferred_source, samples[i][0],
          samples[i][1], tile.chart_path, candidates.front().path.c_str());
    }
    return false;
  }
  return true;
}

SegmentSafetyPointClass ChartPluginPointSafetyClassAtRaw(
    ChartPlugInWrapper* wrapper, const SegmentSafetyChartCandidate& candidate,
    double lat, double lon, const std::string& point_cache_key,
    PlugInSegmentSafetySource* source, SegmentSafetyCoreStats* stats,
    PlugInSegmentSafetyResult* result) {
  if (!wrapper || !g_pi_manager) return SEGMENT_SAFETY_POINT_NO_DATA;

  const PlugInSegmentSafetySource chart_source =
      PI_SEGMENT_SAFETY_SOURCE_PLUGIN_VECTOR;
  if (source) *source = chart_source;
  if (stats) ++stats->s57_chart_count;

  ViewPort vp = SegmentSafetyHighestDetailViewPortAt(lat, lon);
  // Query a complete fine-cell neighbourhood.  This ensures point and line
  // dangers such as WRECKS, UWTROC and OBSTRN cannot fall between grid
  // samples.  Route-mask dilation subsequently applies the user's margin.
  const float select_radius =
      static_cast<float>(kSegmentSafetyGridResolutionDegrees * 0.75);
  ListOfPI_S57Obj* objects = g_pi_manager->GetPlugInObjRuleListAtLatLon(
      wrapper, static_cast<float>(lat), static_cast<float>(lon),
      select_radius, vp);
  // The chart stack says this licensed chart covers the point. A null query
  // therefore means the provider/helper could not supply authoritative data;
  // it must not be converted into an empty-water answer or hidden by CM93.
  if (!objects) return SEGMENT_SAFETY_POINT_NO_DATA;

  bool land = false;
  bool drying = false;
  bool has_depth = false;
  bool unknown_danger_depth = false;
  double min_depth_m = 0.0;
  wxString hit_object;
  wxString depth_object;
  wxString depth_attribute;

  if (objects) {
    for (ListOfPI_S57Obj::Node* node = objects->GetFirst(); node;
         node = node->GetNext()) {
      PI_S57Obj* object = node->GetData();
      if (!object) continue;
      const wxString summary = SegmentSafetyPluginObjectSummary(object);
      if (!strncmp(object->FeatureName, "LNDARE", 6) ||
          SegmentSafetyPluginObjectIsAlwaysDry(object)) {
        land = true;
        hit_object = summary;
        break;
      }
      if (SegmentSafetyPluginObjectIsDrying(object)) {
        drying = true;
        if (hit_object.empty()) hit_object = summary;
      }

      double object_depth_m = 0.0;
      bool object_unknown_danger_depth = false;
      wxString object_depth_attribute;
      if (SegmentSafetyPluginObjectDepthM(
              object, &object_depth_m, &object_depth_attribute,
              &object_unknown_danger_depth)) {
        if (!has_depth || object_depth_m < min_depth_m) {
          has_depth = true;
          min_depth_m = object_depth_m;
          depth_object = summary;
          depth_attribute = object_depth_attribute;
        }
      }
      if (object_unknown_danger_depth) {
        unknown_danger_depth = true;
        depth_object = summary;
        depth_attribute =
            wxString::Format("%s/VALSOU missing", object->FeatureName);
      }
    }
    objects->Clear();
    delete objects;
  }

  const wxString chart_path = wrapper->GetFullPath();
  if (land) {
    if (SegmentSafetyResultHas(
            result, offsetof(PlugInSegmentSafetyResult, hit_object),
            sizeof(result->hit_object))) {
      result->chart_db_index = candidate.db_index;
      result->chart_scale = wrapper->GetNativeScale();
      CopySegmentSafetyString(result->chart_path, sizeof(result->chart_path),
                              chart_path.mb_str());
      CopySegmentSafetyString(result->hit_object, sizeof(result->hit_object),
                              hit_object.mb_str());
    }
    StoreSegmentSafetyPointCache(
        point_cache_key,
        MakeSegmentSafetyPointCacheEntry(
            SEGMENT_SAFETY_POINT_LAND, chart_source, candidate.db_index,
            wrapper->GetNativeScale(), chart_path.mb_str(),
            hit_object.mb_str()));
    return SEGMENT_SAFETY_POINT_LAND;
  }

  if (unknown_danger_depth) has_depth = false;
  const SegmentSafetyPointClass point_class =
      drying ? SEGMENT_SAFETY_POINT_DRYING : SEGMENT_SAFETY_POINT_WATER;
  if (SegmentSafetyResultHas(
          result, offsetof(PlugInSegmentSafetyResult, depth_source_attribute),
          sizeof(result->depth_source_attribute))) {
    result->chart_db_index = candidate.db_index;
    result->chart_scale = wrapper->GetNativeScale();
    CopySegmentSafetyString(result->chart_path, sizeof(result->chart_path),
                            chart_path.mb_str());
    result->has_depth = has_depth ? 1 : 0;
    result->min_depth_m = has_depth ? min_depth_m : 0.0;
    result->has_drying = drying ? 1 : 0;
    CopySegmentSafetyString(result->hit_object, sizeof(result->hit_object),
                            hit_object.mb_str());
    CopySegmentSafetyString(result->depth_source_object,
                            sizeof(result->depth_source_object),
                            depth_object.mb_str());
    CopySegmentSafetyString(result->depth_source_attribute,
                            sizeof(result->depth_source_attribute),
                            depth_attribute.mb_str());
  }
  StoreSegmentSafetyPointCache(
      point_cache_key,
      MakeSegmentSafetyPointCacheEntry(
          point_class, chart_source, candidate.db_index,
          wrapper->GetNativeScale(), chart_path.mb_str(), hit_object.mb_str(),
          has_depth, min_depth_m, drying, depth_object.mb_str(),
          depth_attribute.empty() ? nullptr
                                  : depth_attribute.mb_str().data()));
  return point_class;
}

SegmentSafetyPointClass ChartPointSafetyClassAtRaw(
    double lat, double lon, PlugInSegmentSafetySource* source,
    SegmentSafetyCoreStats* stats, PlugInSegmentSafetyResult* result) {
  std::string point_cache_key = SegmentSafetyPointCacheKey(lat, lon);
  CachedPointSafetyClassification cached_point;
  if (LookupSegmentSafetyPointCache(point_cache_key, &cached_point)) {
    if (stats) ++stats->point_cache_hits;
    if (source) *source = cached_point.source;
    CopySegmentSafetyPointCacheToResult(cached_point, result);
    return cached_point.point_class;
  }
  if (stats) ++stats->point_cache_misses;

  if (!wxThread::IsMain()) {
    if (source) *source = PI_SEGMENT_SAFETY_SOURCE_NONE;
    if (result) {
      SetSegmentSafetyStatus(result, PI_SEGMENT_SAFETY_NO_DATA);
      SetSegmentSafetySource(result, PI_SEGMENT_SAFETY_SOURCE_NONE);
      SetSegmentSafetyMessage(
          result,
          "chart point classification is unavailable from worker thread");
    }
    return SEGMENT_SAFETY_POINT_NO_DATA;
  }

  std::set<int> chart_indexes;
  SegmentSafetyCandidateChartsAt(lat, lon, chart_indexes, stats);

  std::vector<SegmentSafetyChartCandidate> candidates =
      SegmentSafetySortedChartCandidates(lat, lon, chart_indexes);

  bool chart_checked = false;
  bool licensed_plugin_depth_missing = false;
  for (std::vector<SegmentSafetyChartCandidate>::const_iterator it =
           candidates.begin();
       it != candidates.end(); ++it) {
    // Once the selected licensed chart has established ordinary water, later
    // candidates may supply only its missing numeric depth.  Native/CM93
    // charts use a different object path and must not replace the selected
    // licensed chart's semantic classification.
    if (licensed_plugin_depth_missing && !it->plugin_vector) break;
    ChartBase* chart =
        ChartData ? ChartData->OpenChartFromDB(it->db_index, FULL_INIT) : NULL;
    s57chart* s57 = dynamic_cast<s57chart*>(chart);
    ChartPlugInWrapper* plugin_wrapper =
        dynamic_cast<ChartPlugInWrapper*>(chart);
    if (!s57 && !(it->plugin_vector && plugin_wrapper)) continue;
    chart_checked = true;
    if (it->plugin_vector && plugin_wrapper) {
      PlugInSegmentSafetyResult supplemental_result = {};
      supplemental_result.struct_size = sizeof(supplemental_result);
      InitSegmentSafetyResult(&supplemental_result);
      PlugInSegmentSafetyResult* query_result =
          licensed_plugin_depth_missing ? &supplemental_result : result;
      const std::string query_cache_key =
          licensed_plugin_depth_missing
              ? point_cache_key + ":depth:" + std::to_string(it->db_index)
              : point_cache_key;
      const SegmentSafetyPointClass plugin_class =
          ChartPluginPointSafetyClassAtRaw(
              plugin_wrapper, *it, lat, lon, query_cache_key, source, stats,
              query_result);
      if (licensed_plugin_depth_missing) {
        // This chart is consulted for depth only.  Land, drying and unknown
        // danger semantics from a lower-priority chart cannot override the
        // selected detailed chart; try the next licensed candidate instead.
        if (plugin_class != SEGMENT_SAFETY_POINT_WATER ||
            !supplemental_result.has_depth)
          continue;
        if (SegmentSafetyResultHas(
                result,
                offsetof(PlugInSegmentSafetyResult,
                         depth_source_attribute),
                sizeof(result->depth_source_attribute))) {
          result->chart_db_index = supplemental_result.chart_db_index;
          result->chart_scale = supplemental_result.chart_scale;
          result->has_depth = 1;
          result->min_depth_m = supplemental_result.min_depth_m;
          CopySegmentSafetyString(result->chart_path,
                                  sizeof(result->chart_path),
                                  supplemental_result.chart_path);
          CopySegmentSafetyString(result->depth_source_object,
                                  sizeof(result->depth_source_object),
                                  supplemental_result.depth_source_object);
          CopySegmentSafetyString(result->depth_source_attribute,
                                  sizeof(result->depth_source_attribute),
                                  supplemental_result.depth_source_attribute);
        }
        StoreSegmentSafetyPointCache(
            point_cache_key,
            MakeSegmentSafetyPointCacheEntry(
                SEGMENT_SAFETY_POINT_WATER,
                PI_SEGMENT_SAFETY_SOURCE_PLUGIN_VECTOR, it->db_index,
                plugin_wrapper->GetNativeScale(),
                plugin_wrapper->GetFullPath().mb_str(), "", true,
                supplemental_result.min_depth_m, false,
                supplemental_result.depth_source_object,
                supplemental_result.depth_source_attribute));
        return SEGMENT_SAFETY_POINT_WATER;
      }
      const bool plain_missing_depth =
          plugin_class == SEGMENT_SAFETY_POINT_WATER && result &&
          !result->has_depth && result->depth_source_attribute[0] == '\0';
      if (!plain_missing_depth) return plugin_class;
      // Retain the detailed chart's water classification while asking only
      // later licensed/native vector charts for a numeric depth.  This is the
      // legacy point-query equivalent of the batch depth supplementation.
      licensed_plugin_depth_missing = true;
      continue;
    }
    bool cm93 = IsCm93Chart(chart);
    PlugInSegmentSafetySource chart_source =
        cm93 ? PI_SEGMENT_SAFETY_SOURCE_CM93
             : PI_SEGMENT_SAFETY_SOURCE_VECTOR_CHART;
    if (source) *source = chart_source;
    if (stats) ++stats->s57_chart_count;

    ViewPort vp = SegmentSafetyHighestDetailViewPortAt(lat, lon);
    if (cm93) {
      cm93compchart* cm93_chart = dynamic_cast<cm93compchart*>(chart);
      if (cm93_chart) cm93_chart->SetVPParms(vp);
    }

    const float select_radius =
        static_cast<float>(kSegmentSafetyGridResolutionDegrees * 0.75);
    ListOfObjRazRules* rule_list = s57->GetObjRuleListAtLatLon(
        lat, lon, select_radius, &vp, MASK_ALL);
    if (!rule_list) continue;

    bool drying = false;
    bool has_depth = false;
    bool unknown_danger_depth = false;
    double min_depth_m = 0.0;
    wxString depth_object;
    wxString depth_attribute;
    for (ListOfObjRazRules::Node* node = rule_list->GetFirst(); node;
         node = node->GetNext()) {
      ObjRazRules* rule = node->GetData();
      if (!rule || !rule->obj) continue;
      if (!strncmp(rule->obj->FeatureName, "LNDARE", 6) ||
          SegmentSafetyRuleIsAlwaysDry(rule)) {
        wxString chart_path = chart->GetFullPath();
        wxString object = SegmentSafetyRuleSummary(rule);
        if (SegmentSafetyResultHas(
                result, offsetof(PlugInSegmentSafetyResult, hit_object),
                sizeof(result->hit_object))) {
          result->chart_db_index = it->db_index;
          result->chart_scale = chart->GetNativeScale();
          strncpy(result->chart_path, chart_path.mb_str(),
                  sizeof(result->chart_path) - 1);
          result->chart_path[sizeof(result->chart_path) - 1] = '\0';
          strncpy(result->hit_object, object.mb_str(),
                  sizeof(result->hit_object) - 1);
          result->hit_object[sizeof(result->hit_object) - 1] = '\0';
        }
        StoreSegmentSafetyPointCache(
            point_cache_key,
            MakeSegmentSafetyPointCacheEntry(
                SEGMENT_SAFETY_POINT_LAND, chart_source, it->db_index,
                chart->GetNativeScale(), chart_path.mb_str(), object.mb_str()));
        rule_list->Clear();
        delete rule_list;
        return SEGMENT_SAFETY_POINT_LAND;
      }
      if (SegmentSafetyRuleIsDrying(rule)) drying = true;
      double rule_depth = 0.0;
      if (SegmentSafetyRuleDepthMinM(rule, &rule_depth)) {
        if (!has_depth || rule_depth < min_depth_m) {
          has_depth = true;
          min_depth_m = rule_depth;
          depth_object = SegmentSafetyRuleSummary(rule);
          depth_attribute = wxString::Format("%s/DRVAL1",
                                             rule->obj->FeatureName);
        }
      }
      bool danger_unknown = false;
      if (SegmentSafetyRuleDangerDepthM(rule, &rule_depth, &danger_unknown)) {
        if (!has_depth || rule_depth < min_depth_m) {
          has_depth = true;
          min_depth_m = rule_depth;
          depth_object = SegmentSafetyRuleSummary(rule);
          depth_attribute = wxString::Format("%s/VALSOU",
                                             rule->obj->FeatureName);
        }
      }
      if (danger_unknown) {
        unknown_danger_depth = true;
        depth_object = SegmentSafetyRuleSummary(rule);
        depth_attribute = wxString::Format("%s/VALSOU missing",
                                           rule->obj->FeatureName);
      }
    }

    rule_list->Clear();
    delete rule_list;
    if (unknown_danger_depth) has_depth = false;
    SegmentSafetyPointClass point_class =
        drying ? SEGMENT_SAFETY_POINT_DRYING : SEGMENT_SAFETY_POINT_WATER;
    wxString chart_path = chart->GetFullPath();
    if (SegmentSafetyResultHas(
            result, offsetof(PlugInSegmentSafetyResult, depth_source_attribute),
            sizeof(result->depth_source_attribute))) {
      result->has_depth = has_depth ? 1 : 0;
      result->min_depth_m = has_depth ? min_depth_m : 0.0;
      result->has_drying = drying ? 1 : 0;
      if (has_depth) {
        strncpy(result->depth_source_object, depth_object.mb_str(),
                sizeof(result->depth_source_object) - 1);
        result->depth_source_object[sizeof(result->depth_source_object) - 1] =
            '\0';
        CopySegmentSafetyString(result->depth_source_attribute,
                                sizeof(result->depth_source_attribute),
                                depth_attribute.mb_str());
      } else if (unknown_danger_depth) {
        CopySegmentSafetyString(result->depth_source_object,
                                sizeof(result->depth_source_object),
                                depth_object.mb_str());
        CopySegmentSafetyString(result->depth_source_attribute,
                                sizeof(result->depth_source_attribute),
                                depth_attribute.mb_str());
      }
    }
    StoreSegmentSafetyPointCache(
        point_cache_key,
        MakeSegmentSafetyPointCacheEntry(
            point_class, chart_source, it->db_index, chart->GetNativeScale(),
            chart_path.mb_str(), "", has_depth, min_depth_m, drying,
            depth_object.mb_str(),
            depth_attribute.empty() ? nullptr
                                    : depth_attribute.mb_str().data()));
    if (point_class == SEGMENT_SAFETY_POINT_WATER && !has_depth &&
        !unknown_danger_depth)
      continue;
    return point_class;
  }

  SegmentSafetyPointClass point_class =
      chart_checked ? SEGMENT_SAFETY_POINT_WATER : SEGMENT_SAFETY_POINT_NO_DATA;
  StoreSegmentSafetyPointCache(
      point_cache_key, MakeSegmentSafetyPointCacheEntry(
                           point_class,
                           chart_checked ? PI_SEGMENT_SAFETY_SOURCE_VECTOR_CHART
                                         : PI_SEGMENT_SAFETY_SOURCE_NONE,
                           -1, -1, "", ""));
  return point_class;
}

SegmentSafetyPointClass ChartPointSafetyClassAtPreparedCm93(
    cm93chart* chart, int chart_db_index, double lat, double lon,
    ViewPort* viewport, PlugInSegmentSafetySource* source,
    SegmentSafetyCoreStats* stats, PlugInSegmentSafetyResult* result) {
  if (!chart || !viewport) return SEGMENT_SAFETY_POINT_NO_DATA;

  const std::string point_cache_key = SegmentSafetyPointCacheKey(lat, lon);
  CachedPointSafetyClassification cached_point;
  if (LookupSegmentSafetyPointCache(point_cache_key, &cached_point)) {
    if (stats) ++stats->point_cache_hits;
    if (source) *source = cached_point.source;
    CopySegmentSafetyPointCacheToResult(cached_point, result);
    return cached_point.point_class;
  }
  if (stats) {
    ++stats->point_cache_misses;
    ++stats->s57_chart_count;
  }

  const PlugInSegmentSafetySource chart_source = PI_SEGMENT_SAFETY_SOURCE_CM93;
  if (source) *source = chart_source;

  ListOfObjRazRules* rule_list =
      chart->GetObjRuleListAtLatLon(lat, lon, 0.0, viewport, MASK_AREA);
  bool drying = false;
  bool has_depth = false;
  double min_depth_m = 0.0;
  wxString depth_object;
  if (rule_list) {
    for (ListOfObjRazRules::Node* node = rule_list->GetFirst(); node;
         node = node->GetNext()) {
      ObjRazRules* rule = node->GetData();
      if (!rule || !rule->obj) continue;
      if (!strncmp(rule->obj->FeatureName, "LNDARE", 6)) {
        const wxString chart_path = chart->GetFullPath();
        const wxString object = SegmentSafetyRuleSummary(rule);
        if (SegmentSafetyResultHas(
                result, offsetof(PlugInSegmentSafetyResult, hit_object),
                sizeof(result->hit_object))) {
          result->chart_db_index = chart_db_index;
          result->chart_scale = chart->GetNativeScale();
          strncpy(result->chart_path, chart_path.mb_str(),
                  sizeof(result->chart_path) - 1);
          result->chart_path[sizeof(result->chart_path) - 1] = '\0';
          strncpy(result->hit_object, object.mb_str(),
                  sizeof(result->hit_object) - 1);
          result->hit_object[sizeof(result->hit_object) - 1] = '\0';
        }
        StoreSegmentSafetyPointCache(
            point_cache_key,
            MakeSegmentSafetyPointCacheEntry(
                SEGMENT_SAFETY_POINT_LAND, chart_source, chart_db_index,
                chart->GetNativeScale(), chart_path.mb_str(), object.mb_str()));
        rule_list->Clear();
        delete rule_list;
        return SEGMENT_SAFETY_POINT_LAND;
      }
      if (SegmentSafetyRuleIsDrying(rule)) drying = true;
      double rule_depth = 0.0;
      if (SegmentSafetyRuleDepthMinM(rule, &rule_depth) &&
          (!has_depth || rule_depth < min_depth_m)) {
        has_depth = true;
        min_depth_m = rule_depth;
        depth_object = SegmentSafetyRuleSummary(rule);
      }
    }
    rule_list->Clear();
    delete rule_list;
  }

  const SegmentSafetyPointClass point_class =
      drying ? SEGMENT_SAFETY_POINT_DRYING : SEGMENT_SAFETY_POINT_WATER;
  const wxString chart_path = chart->GetFullPath();
  if (SegmentSafetyResultHas(
          result, offsetof(PlugInSegmentSafetyResult, depth_source_attribute),
          sizeof(result->depth_source_attribute))) {
    result->has_depth = has_depth ? 1 : 0;
    result->min_depth_m = has_depth ? min_depth_m : 0.0;
    result->has_drying = drying ? 1 : 0;
    if (has_depth) {
      strncpy(result->depth_source_object, depth_object.mb_str(),
              sizeof(result->depth_source_object) - 1);
      result->depth_source_object[sizeof(result->depth_source_object) - 1] =
          '\0';
      strncpy(result->depth_source_attribute, "DEPARE/DRVAL1",
              sizeof(result->depth_source_attribute) - 1);
      result
          ->depth_source_attribute[sizeof(result->depth_source_attribute) - 1] =
          '\0';
    }
  }
  StoreSegmentSafetyPointCache(
      point_cache_key,
      MakeSegmentSafetyPointCacheEntry(
          point_class, chart_source, chart_db_index, chart->GetNativeScale(),
          chart_path.mb_str(), "", has_depth, min_depth_m, drying,
          depth_object.mb_str(), has_depth ? "DEPARE/DRVAL1" : NULL));
  return point_class;
}

ocpn::chart_safety::DepthProbeClass SegmentSafetyDepthProbeClass(
    SegmentSafetyPointClass point_class) {
  using ocpn::chart_safety::DepthProbeClass;
  switch (point_class) {
    case SEGMENT_SAFETY_POINT_WATER:
      return DepthProbeClass::kWater;
    case SEGMENT_SAFETY_POINT_LAND:
      return DepthProbeClass::kLand;
    case SEGMENT_SAFETY_POINT_DRYING:
      return DepthProbeClass::kDrying;
    case SEGMENT_SAFETY_POINT_NO_DATA:
    default:
      return DepthProbeClass::kNoData;
  }
}

bool RecoverPreparedCm93BoundaryDepth(
    double lat, double lon, double resolution, SegmentSafetyCoreStats* stats,
    PlugInSegmentSafetyResult* result) {
  if (!result) return false;

  using ocpn::chart_safety::DepthProbe;
  std::array<DepthProbe, 4> probes;
  std::array<PlugInSegmentSafetyResult, 4> probe_results = {};
  const double offset = resolution * 0.25;
  const double lat_signs[4] = {-1.0, -1.0, 1.0, 1.0};
  const double lon_signs[4] = {-1.0, 1.0, -1.0, 1.0};
  int minimum_probe = -1;
  double minimum_depth = 0.0;

  for (size_t i = 0; i < probes.size(); ++i) {
    const double probe_lat = lat + lat_signs[i] * offset;
    const double probe_lon = lon + lon_signs[i] * offset;
    probe_results[i].struct_size = sizeof(probe_results[i]);
    InitSegmentSafetyResult(&probe_results[i]);
    PlugInSegmentSafetySource source = PI_SEGMENT_SAFETY_SOURCE_NONE;
    const SegmentSafetyPointClass point_class =
        ChartPointSafetyClassAtRaw(probe_lat, probe_lon, &source, stats,
                                   &probe_results[i]);
    probes[i] = {SegmentSafetyDepthProbeClass(point_class),
                 probe_results[i].has_depth != 0,
                 probe_results[i].min_depth_m};
    if (probe_results[i].has_depth &&
        (minimum_probe < 0 || probe_results[i].min_depth_m < minimum_depth)) {
      minimum_probe = static_cast<int>(i);
      minimum_depth = probe_results[i].min_depth_m;
    }
  }

  const std::optional<double> recovered =
      ocpn::chart_safety::ConservativeBoundaryDepth(probes);
  if (!recovered || minimum_probe < 0) return false;

  result->has_depth = 1;
  result->min_depth_m = *recovered;
  result->has_drying = 0;
  CopySegmentSafetyString(result->depth_source_object,
                          sizeof(result->depth_source_object),
                          probe_results[minimum_probe].depth_source_object);
  CopySegmentSafetyString(result->depth_source_attribute,
                          sizeof(result->depth_source_attribute),
                          "DEPARE/DRVAL1 boundary-min");
  return true;
}

struct SegmentSafetyPluginBatchGroup {
  int db_index;
  ChartPlugInWrapper* wrapper;
  std::vector<uint8_t> active;
  std::vector<uint8_t> land;
  std::vector<uint8_t> drying;
  std::vector<uint8_t> has_depth;
  std::vector<uint8_t> unknown_danger_depth;
  std::vector<float> min_depth_m;
  int active_count;

  explicit SegmentSafetyPluginBatchGroup(size_t cells = 0)
      : db_index(-1),
        wrapper(NULL),
        active(cells, 0),
        land(cells, 0),
        drying(cells, 0),
        has_depth(cells, 0),
        unknown_danger_depth(cells, 0),
        min_depth_m(cells, 0.0f),
        active_count(0) {}
};

void SegmentSafetyPluginBatchVisit(void* context, const void* object_ptr,
                                   const uint64_t* hit_cells,
                                   uint32_t hit_word_count) {
  SegmentSafetyPluginBatchGroup* group =
      static_cast<SegmentSafetyPluginBatchGroup*>(context);
  PI_S57Obj* object =
      const_cast<PI_S57Obj*>(static_cast<const PI_S57Obj*>(object_ptr));
  if (!group || !object || !hit_cells) return;

  const bool land = !strncmp(object->FeatureName, "LNDARE", 6) ||
                    SegmentSafetyPluginObjectIsAlwaysDry(object);
  const bool drying = SegmentSafetyPluginObjectIsDrying(object);
  double depth_m = 0.0;
  bool unknown_danger_depth = false;
  const bool has_depth = SegmentSafetyPluginObjectDepthM(
      object, &depth_m, NULL, &unknown_danger_depth);
  if (!land && !drying && !has_depth && !unknown_danger_depth) return;

  const size_t cells = group->active.size();
  for (uint32_t word = 0; word < hit_word_count; ++word) {
    uint64_t bits = hit_cells[word];
    while (bits) {
#if defined(__GNUC__) || defined(__clang__)
      const unsigned bit = static_cast<unsigned>(__builtin_ctzll(bits));
#else
      unsigned bit = 0;
      while (((bits >> bit) & 1u) == 0) ++bit;
#endif
      const size_t index = static_cast<size_t>(word) * 64 + bit;
      if (index < cells && group->active[index]) {
        if (land) group->land[index] = 1;
        if (drying) group->drying[index] = 1;
        if (has_depth &&
            (!group->has_depth[index] ||
             depth_m < group->min_depth_m[index])) {
          group->has_depth[index] = 1;
          group->min_depth_m[index] = static_cast<float>(depth_m);
        }
        if (unknown_danger_depth)
          group->unknown_danger_depth[index] = 1;
      }
      bits &= bits - 1;
    }
  }
}

std::set<std::pair<long, long> >
PrebuildSegmentSafetyPluginVectorGridTiles(
    const std::set<std::pair<long, long> >& requested_tiles,
    SegmentSafetyCoreStats* stats, bool require_depth) {
  std::set<std::pair<long, long> > built_tiles;
  if (!wxThread::IsMain() || requested_tiles.empty() || !ChartData ||
      !g_pi_manager || !g_pi_manager->HasPlugInChartSafetyGrid())
    return built_tiles;

  std::set<std::pair<long, long> > missing_tiles;
  for (const auto& tile : requested_tiles) {
    const std::string key =
        SegmentSafetyGridTileKeyForIndices(tile.first, tile.second);
    CachedPointSafetyGridTile cached;
    if (LookupSegmentSafetyGridTile(key, &cached) &&
        (!require_depth || cached.depth_complete))
      continue;

    // Consult the weather-routing-owned persistent store before planning
    // provider rectangles.  Without this, a restart found all host RAM tiles
    // empty and needlessly re-extracted every licensed chart even though the
    // exact final-identity semantic tiles were already on disk.
    CachedPointSafetyGridTile external;
    if (SegmentSafetyExternalTileCacheLookup(
            tile.first, tile.second, require_depth, &external) &&
        SegmentSafetyCachedTileProviderIsCurrent(external)) {
      StoreSegmentSafetyGridTile(key, external);
      continue;
    }
    missing_tiles.insert(tile);
  }

  constexpr int kTileCells = 40;
  constexpr int kMaximumTileSpan = 6;
  const auto blocks = ocpn::chart_safety::PlanTileBatchBlocks(
      missing_tiles, kMaximumTileSpan);
  for (const auto& block : blocks) {
    const int tile_rows =
        static_cast<int>(block.max_lat_tile - block.min_lat_tile + 1);
    const int tile_cols =
        static_cast<int>(block.max_lon_tile - block.min_lon_tile + 1);
    const int rows = tile_rows * kTileCells + 1;
    const int cols = tile_cols * kTileCells + 1;
    const size_t cell_count = static_cast<size_t>(rows) * cols;
    if (cell_count == 0 || cell_count > 65536) continue;

    wxStopWatch timer;
    const double min_lat = ocpn::chart_safety::GlobalGridCoordinate(
        block.min_lat_tile, 0, kTileCells,
        kSegmentSafetyGridResolutionDegrees);
    const double min_lon = ocpn::chart_safety::GlobalGridCoordinate(
        block.min_lon_tile, 0, kTileCells,
        kSegmentSafetyGridResolutionDegrees);
    const double max_lat = ocpn::chart_safety::GlobalGridCoordinate(
        block.min_lat_tile, rows - 1, kTileCells,
        kSegmentSafetyGridResolutionDegrees);
    const double max_lon = ocpn::chart_safety::GlobalGridCoordinate(
        block.min_lon_tile, cols - 1, kTileCells,
        kSegmentSafetyGridResolutionDegrees);
    const std::vector<SegmentSafetyChartCandidate> candidates =
        SegmentSafetyTileChartCandidates(min_lat, min_lon, max_lat, max_lon);

    std::vector<uint8_t> classified(cell_count, 0);
    std::vector<uint8_t> classes(
        cell_count, static_cast<uint8_t>(SEGMENT_SAFETY_POINT_NO_DATA));
    std::vector<uint16_t> hazards(cell_count,
                                  SEGMENT_SAFETY_HAZARD_NO_CHART);
    std::vector<uint8_t> has_depth(cell_count, 0);
    std::vector<float> min_depth_m(cell_count, 0.0f);
    std::vector<uint8_t> has_drying(cell_count, 0);
    std::vector<uint8_t> unknown_danger_depth(cell_count, 0);
    std::vector<uint8_t> persistent_cache_allowed(cell_count, 0);
    std::vector<int> selected_db_index(cell_count, -1);
    std::vector<std::vector<int> > depth_candidates(cell_count);
    std::map<int, SegmentSafetyPluginBatchGroup> groups;

    for (int row = 0; row < rows; ++row) {
      const double lat = ocpn::chart_safety::GlobalGridCoordinate(
          block.min_lat_tile, row, kTileCells,
          kSegmentSafetyGridResolutionDegrees);
      for (int col = 0; col < cols; ++col) {
        const double lon = ocpn::chart_safety::GlobalGridCoordinate(
            block.min_lon_tile, col, kTileCells,
            kSegmentSafetyGridResolutionDegrees);
        const size_t index = static_cast<size_t>(row) * cols + col;
        bool selected_plugin = false;
        for (const auto& candidate : candidates) {
          if (!ChartData->ChartCoversPosition(
                  candidate.db_index, static_cast<float>(lat),
                  static_cast<float>(lon)))
            continue;
          if (stats) ++stats->chart_stack_entries;
          if (candidate.plugin_vector) {
            depth_candidates[index].push_back(candidate.db_index);
            if (!selected_plugin) {
              auto found = groups.find(candidate.db_index);
              if (found == groups.end()) {
                found = groups
                            .insert(std::make_pair(
                                candidate.db_index,
                                SegmentSafetyPluginBatchGroup(cell_count)))
                            .first;
                found->second.db_index = candidate.db_index;
              }
              found->second.active[index] = 1;
              ++found->second.active_count;
              selected_db_index[index] = candidate.db_index;
              selected_plugin = true;
            }
            continue;
          }
          if (!candidate.cm93) break;
        }
      }
    }

    std::map<int, ChartPlugInWrapper*> wrappers;
    for (auto& item : groups) {
      ChartBase* chart = ChartData->OpenChartFromDB(item.first, FULL_INIT);
      if (!IsSupportedSegmentSafetyPluginChart(chart)) continue;
      item.second.wrapper = dynamic_cast<ChartPlugInWrapper*>(chart);
      wrappers[item.first] = item.second.wrapper;
    }

    const double centre_lat = (min_lat + max_lat) / 2.0;
    const double centre_lon = (min_lon + max_lon) / 2.0;
    ViewPort vp =
        SegmentSafetyHighestDetailViewPortAt(centre_lat, centre_lon);
    const double cos_lat = wxMax(
        0.1, fabs(cos(SegmentSafetyDegToRad(centre_lat))));
    const double width_m = (max_lon - min_lon) * 60.0 * 1852.0 * cos_lat;
    const double height_m = (max_lat - min_lat) * 60.0 * 1852.0;
    vp.pix_width = wxMax(vp.pix_width,
                         static_cast<int>(ceil(width_m * vp.view_scale_ppm)) +
                             512);
    vp.pix_height =
        wxMax(vp.pix_height,
              static_cast<int>(ceil(height_m * vp.view_scale_ppm)) + 512);
    vp.SetBoxes();

    int provider_calls = 0;
    int provider_failures = 0;
    int candidate_objects = 0;
    int hit_objects = 0;
    for (auto& item : groups) {
      SegmentSafetyPluginBatchGroup& group = item.second;
      if (!group.wrapper || group.active_count <= 0) continue;
      OCPN_PluginChartSafetyGridRequestV1 request = {};
      request.struct_size = sizeof(request);
      request.abi_version = OCPN_PLUGIN_CHART_SAFETY_GRID_ABI_V1;
      request.min_lat = min_lat;
      request.min_lon = min_lon;
      request.lat_step = kSegmentSafetyGridResolutionDegrees;
      request.lon_step = kSegmentSafetyGridResolutionDegrees;
      request.rows = rows;
      request.cols = cols;
      request.select_radius_degrees = static_cast<float>(
          kSegmentSafetyGridResolutionDegrees * 0.75);
      request.active_cells = group.active.data();
      request.visitor_context = &group;
      request.visit_object = SegmentSafetyPluginBatchVisit;
      OCPN_PluginChartSafetyGridResultV1 provider_result = {};
      provider_result.struct_size = sizeof(provider_result);
      ++provider_calls;
      const int status = g_pi_manager->QueryPlugInChartSafetyGrid(
          group.wrapper, request, &provider_result, vp);
      if (status != 1 ||
          provider_result.abi_version !=
              OCPN_PLUGIN_CHART_SAFETY_GRID_ABI_V1 ||
          provider_result.processed_cells !=
              static_cast<uint32_t>(group.active_count)) {
        ++provider_failures;
        continue;
      }
      candidate_objects += provider_result.candidate_objects;
      hit_objects += provider_result.hit_objects;
      if (stats) ++stats->s57_chart_count;
      const bool provider_cache_allowed =
          (provider_result.result_flags &
           OCPN_PLUGIN_CHART_SAFETY_RESULT_DERIVED_CACHE_ALLOWED) != 0;
      for (size_t index = 0; index < cell_count; ++index) {
        if (!group.active[index]) continue;
        classified[index] = 1;
        persistent_cache_allowed[index] = provider_cache_allowed ? 1 : 0;
        SegmentSafetyPointClass point_class = SEGMENT_SAFETY_POINT_WATER;
        if (group.land[index])
          point_class = SEGMENT_SAFETY_POINT_LAND;
        else if (group.drying[index])
          point_class = SEGMENT_SAFETY_POINT_DRYING;
        classes[index] = static_cast<uint8_t>(point_class);
        hazards[index] = SegmentSafetyPointHazardFlags(point_class);
        const bool depth_known = group.has_depth[index] &&
                                 !group.unknown_danger_depth[index];
        has_depth[index] = depth_known ? 1 : 0;
        min_depth_m[index] =
            depth_known ? group.min_depth_m[index] : 0.0f;
        has_drying[index] = group.drying[index] ? 1 : 0;
        unknown_danger_depth[index] =
            group.unknown_danger_depth[index] ? 1 : 0;
      }
    }

    size_t maximum_depth_candidate = 0;
    for (const auto& chain : depth_candidates)
      maximum_depth_candidate =
          std::max(maximum_depth_candidate, chain.size());
    for (size_t rank = 1; rank < maximum_depth_candidate; ++rank) {
      std::map<int, SegmentSafetyPluginBatchGroup> fallback_groups;
      for (size_t index = 0; index < cell_count; ++index) {
        if (!classified[index] || has_depth[index] ||
            unknown_danger_depth[index] ||
            classes[index] != SEGMENT_SAFETY_POINT_WATER ||
            depth_candidates[index].size() <= rank)
          continue;
        const int db_index = depth_candidates[index][rank];
        auto found = fallback_groups.find(db_index);
        if (found == fallback_groups.end()) {
          found = fallback_groups
                      .insert(std::make_pair(
                          db_index,
                          SegmentSafetyPluginBatchGroup(cell_count)))
                      .first;
          found->second.db_index = db_index;
        }
        found->second.active[index] = 1;
        ++found->second.active_count;
      }
      for (auto& item : fallback_groups) {
        ChartBase* chart = ChartData->OpenChartFromDB(item.first, FULL_INIT);
        if (!IsSupportedSegmentSafetyPluginChart(chart)) continue;
        item.second.wrapper = dynamic_cast<ChartPlugInWrapper*>(chart);
        wrappers[item.first] = item.second.wrapper;
      }
      for (auto& item : fallback_groups) {
        SegmentSafetyPluginBatchGroup& group = item.second;
        if (!group.wrapper || group.active_count <= 0) continue;
        OCPN_PluginChartSafetyGridRequestV1 request = {};
        request.struct_size = sizeof(request);
        request.abi_version = OCPN_PLUGIN_CHART_SAFETY_GRID_ABI_V1;
        request.min_lat = min_lat;
        request.min_lon = min_lon;
        request.lat_step = kSegmentSafetyGridResolutionDegrees;
        request.lon_step = kSegmentSafetyGridResolutionDegrees;
        request.rows = rows;
        request.cols = cols;
        request.select_radius_degrees = static_cast<float>(
            kSegmentSafetyGridResolutionDegrees * 0.75);
        request.active_cells = group.active.data();
        request.visitor_context = &group;
        request.visit_object = SegmentSafetyPluginBatchVisit;
        OCPN_PluginChartSafetyGridResultV1 provider_result = {};
        provider_result.struct_size = sizeof(provider_result);
        ++provider_calls;
        const int status = g_pi_manager->QueryPlugInChartSafetyGrid(
            group.wrapper, request, &provider_result, vp);
        if (status != 1 ||
            provider_result.abi_version !=
                OCPN_PLUGIN_CHART_SAFETY_GRID_ABI_V1 ||
            provider_result.processed_cells !=
                static_cast<uint32_t>(group.active_count)) {
          ++provider_failures;
          continue;
        }
        candidate_objects += provider_result.candidate_objects;
        hit_objects += provider_result.hit_objects;
        const bool fallback_cache_allowed =
            (provider_result.result_flags &
             OCPN_PLUGIN_CHART_SAFETY_RESULT_DERIVED_CACHE_ALLOWED) != 0;
        for (size_t index = 0; index < cell_count; ++index) {
          if (group.active[index] && !fallback_cache_allowed)
            persistent_cache_allowed[index] = 0;
          if (!group.active[index] || group.land[index] ||
              group.drying[index] || group.unknown_danger_depth[index])
            continue;
          if (group.has_depth[index]) {
            has_depth[index] = 1;
            min_depth_m[index] = group.min_depth_m[index];
          }
        }
      }
    }

    int stored_in_block = 0;
    for (const auto& requested : block.tiles) {
      const int row_offset = static_cast<int>(
          (requested.first - block.min_lat_tile) * kTileCells);
      const int col_offset = static_cast<int>(
          (requested.second - block.min_lon_tile) * kTileCells);
      bool complete = true;
      for (int row = 0; row <= kTileCells && complete; ++row)
        for (int col = 0; col <= kTileCells; ++col) {
          const size_t source = static_cast<size_t>(row_offset + row) * cols +
                                col_offset + col;
          if (!classified[source]) {
            complete = false;
            break;
          }
        }
      if (!complete) continue;

      CachedPointSafetyGridTile tile;
      tile.group_index = SegmentSafetyCurrentGroupIndex();
      tile.lat_tile = requested.first;
      tile.lon_tile = requested.second;
      tile.min_lat = requested.first * kSegmentSafetyGridTileDegrees;
      tile.min_lon = requested.second * kSegmentSafetyGridTileDegrees;
      tile.resolution = kSegmentSafetyGridResolutionDegrees;
      tile.rows = tile.cols = kTileCells + 1;
      const size_t tile_cell_count =
          static_cast<size_t>(tile.rows) * tile.cols;
      tile.classes.resize(tile_cell_count);
      tile.hazard_flags.resize(tile_cell_count);
      tile.has_depth.resize(tile_cell_count);
      tile.min_depth_m.resize(tile_cell_count);
      tile.has_drying.resize(tile_cell_count);
      tile.hazard_summary_flags = SEGMENT_SAFETY_HAZARD_NONE;
      tile.land_count = tile.water_count = tile.drying_count =
          tile.unknown_count = 0;
      int representative_db_index = -1;
      for (int row = 0; row < tile.rows; ++row) {
        for (int col = 0; col < tile.cols; ++col) {
          const size_t source = static_cast<size_t>(row_offset + row) * cols +
                                col_offset + col;
          const size_t target = static_cast<size_t>(row) * tile.cols + col;
          tile.classes[target] = classes[source];
          tile.hazard_flags[target] = hazards[source];
          tile.has_depth[target] = has_depth[source];
          tile.min_depth_m[target] = min_depth_m[source];
          tile.has_drying[target] = has_drying[source];
          tile.hazard_summary_flags |= hazards[source];
          if (representative_db_index < 0)
            representative_db_index = selected_db_index[source];
          switch (static_cast<SegmentSafetyPointClass>(classes[source])) {
            case SEGMENT_SAFETY_POINT_LAND:
              ++tile.land_count;
              break;
            case SEGMENT_SAFETY_POINT_DRYING:
              ++tile.drying_count;
              break;
            case SEGMENT_SAFETY_POINT_WATER:
              ++tile.water_count;
              break;
            default:
              ++tile.unknown_count;
              break;
          }
        }
      }
      tile.source = PI_SEGMENT_SAFETY_SOURCE_PLUGIN_VECTOR;
      tile.chart_db_index = representative_db_index;
      const auto wrapper = wrappers.find(representative_db_index);
      if (wrapper != wrappers.end() && wrapper->second) {
        tile.chart_scale = wrapper->second->GetNativeScale();
        CopySegmentSafetyString(tile.chart_path, sizeof(tile.chart_path),
                                wrapper->second->GetFullPath().mb_str());
      }
      tile.depth_complete = true;
      tile.persistent_cache_allowed = true;
      for (int row = 0; row < tile.rows && tile.persistent_cache_allowed;
           ++row)
        for (int col = 0; col < tile.cols; ++col) {
          const size_t source = static_cast<size_t>(row_offset + row) * cols +
                                col_offset + col;
          if (!persistent_cache_allowed[source]) {
            tile.persistent_cache_allowed = false;
            break;
          }
        }
      tile.built = true;
      StoreSegmentSafetyGridTile(
          SegmentSafetyGridTileKeyForIndices(requested.first,
                                             requested.second),
          tile);
      built_tiles.insert(requested);
      ++stored_in_block;
      if (stats) {
        stats->grid_cells_total += tile.rows * tile.cols;
        stats->grid_cells_land += tile.land_count;
        stats->grid_cells_water += tile.water_count;
        stats->grid_cells_drying += tile.drying_count;
        stats->grid_cells_unknown += tile.unknown_count;
      }
    }
    const long elapsed_ms = timer.Time();
    if (stats) stats->grid_build_ms += elapsed_ms;
    wxLogMessage(
        "SEGMENT_SAFETY_PLUGIN_CORRIDOR_BATCH "
        "tile_bbox=[%ld..%ld,%ld..%ld] requested_tiles=%lu "
        "stored_tiles=%d cells=%lu provider_calls=%d failures=%d "
        "candidate_objects=%d hit_objects=%d require_depth=%d "
        "elapsed_ms=%ld",
        block.min_lat_tile, block.max_lat_tile, block.min_lon_tile,
        block.max_lon_tile, static_cast<unsigned long>(block.tiles.size()),
        stored_in_block, static_cast<unsigned long>(cell_count), provider_calls,
        provider_failures, candidate_objects, hit_objects,
        require_depth ? 1 : 0, elapsed_ms);
  }
  return built_tiles;
}

CachedPointSafetyGridTile BuildSegmentSafetyGridTile(
    double lat, double lon, long lat_tile, long lon_tile,
    SegmentSafetyCoreStats* stats, bool require_depth) {
  if (!wxThread::IsMain()) {
    wxLogMessage(
        "WR_GRID_THREAD_VIOLATION BuildSegmentSafetyGridTile called from "
        "worker thread=%p main_thread=%p tile=(%ld,%ld). Refusing chart "
        "object access.",
        wxThread::GetCurrentId(), wxThread::GetMainId(), lat_tile, lon_tile);
    if (stats) stats->chart_load_failed = true;
    return CachedPointSafetyGridTile();
  }

  wxStopWatch timer;
  CachedPointSafetyGridTile tile;
  constexpr int kTileCells = 40;
  tile.group_index = SegmentSafetyCurrentGroupIndex();
  tile.lat_tile = lat_tile;
  tile.lon_tile = lon_tile;
  tile.min_lat = ocpn::chart_safety::GlobalGridCoordinate(
      lat_tile, 0, kTileCells, kSegmentSafetyGridResolutionDegrees);
  tile.min_lon = ocpn::chart_safety::GlobalGridCoordinate(
      lon_tile, 0, kTileCells, kSegmentSafetyGridResolutionDegrees);
  tile.resolution = kSegmentSafetyGridResolutionDegrees;
  tile.rows = kTileCells + 1;
  tile.cols = tile.rows;
  tile.built = true;
  tile.classes.assign(tile.rows * tile.cols,
                      (unsigned char)SEGMENT_SAFETY_POINT_NO_DATA);
  tile.hazard_flags.assign(tile.rows * tile.cols,
                           SEGMENT_SAFETY_HAZARD_NO_CHART);
  tile.has_depth.assign(tile.rows * tile.cols, 0);
  tile.min_depth_m.assign(tile.rows * tile.cols, 0.0f);
  tile.has_drying.assign(tile.rows * tile.cols, 0);
  tile.hazard_summary_flags = SEGMENT_SAFETY_HAZARD_NONE;

  // Optional plugin-vector batch path.  Chart selection remains in OpenCPN:
  // every cell is assigned to the same highest-detail/newest usable chart the
  // point path would choose.  Each upgraded provider then walks its objects
  // once and reports cell membership through the versioned C ABI.  If the
  // symbol is absent, incomplete, or errors, those cells retain the exact
  // point-query fallback below.
  const size_t grid_cell_count =
      static_cast<size_t>(tile.rows) * tile.cols;
  std::vector<uint8_t> plugin_batch_classified(grid_cell_count, 0);
  std::vector<uint8_t> plugin_batch_unknown_danger(grid_cell_count, 0);
  std::vector<uint8_t> plugin_batch_persistent_cache_allowed(
      grid_cell_count, 0);
  std::vector<std::vector<int> > plugin_depth_candidates(grid_cell_count);
  int plugin_batch_groups = 0;
  int plugin_batch_cells = 0;
  int plugin_batch_fallback_cells = 0;
  int plugin_batch_candidate_objects = 0;
  int plugin_batch_hit_objects = 0;
  long plugin_batch_select_ms = 0;
  long plugin_batch_query_ms = 0;
  if (ChartData && g_pi_manager &&
      g_pi_manager->HasPlugInChartSafetyGrid()) {
    wxStopWatch batch_select_timer;
    std::map<int, SegmentSafetyPluginBatchGroup> groups;
    const double grid_max_lat = ocpn::chart_safety::GlobalGridCoordinate(
        lat_tile, tile.rows - 1, kTileCells, tile.resolution);
    const double grid_max_lon = ocpn::chart_safety::GlobalGridCoordinate(
        lon_tile, tile.cols - 1, kTileCells, tile.resolution);
    const std::vector<SegmentSafetyChartCandidate> tile_candidates =
        SegmentSafetyTileChartCandidates(tile.min_lat, tile.min_lon,
                                         grid_max_lat, grid_max_lon);
    for (int r = 0; r < tile.rows; ++r) {
      const double cell_lat = ocpn::chart_safety::GlobalGridCoordinate(
          lat_tile, r, kTileCells, tile.resolution);
      for (int c = 0; c < tile.cols; ++c) {
        const double cell_lon = ocpn::chart_safety::GlobalGridCoordinate(
            lon_tile, c, kTileCells, tile.resolution);
        const size_t index = static_cast<size_t>(r) * tile.cols + c;
        bool selected_plugin = false;
        for (std::vector<SegmentSafetyChartCandidate>::const_iterator it =
                 tile_candidates.begin();
             it != tile_candidates.end(); ++it) {
          if (!ChartData->ChartCoversPosition(
                  it->db_index, static_cast<float>(cell_lat),
                  static_cast<float>(cell_lon)))
            continue;
          if (stats) ++stats->chart_stack_entries;
          if (it->plugin_vector) {
            plugin_depth_candidates[index].push_back(it->db_index);
            if (!selected_plugin) {
              std::map<int, SegmentSafetyPluginBatchGroup>::iterator found =
                  groups.find(it->db_index);
              if (found == groups.end()) {
                found = groups
                            .insert(std::make_pair(
                                it->db_index,
                                SegmentSafetyPluginBatchGroup(grid_cell_count)))
                            .first;
                found->second.db_index = it->db_index;
              }
              found->second.active[index] = 1;
              ++found->second.active_count;
              selected_plugin = true;
            }
            continue;
          }
          // The sorted table metadata uses the same provider, native-scale,
          // edition-date, file-time and path ordering as the point path.  A
          // native vector chart therefore supersedes later plugin charts; CM93
          // cannot, because it has the lower provider priority.
          if (!it->cm93) break;
        }
      }
    }

    // Open only the plugin charts actually selected by at least one cell, and
    // only once per tile.  Unsupported plugin-vector formats deliberately
    // retain the exact point-query fallback below.
    for (std::map<int, SegmentSafetyPluginBatchGroup>::iterator it =
             groups.begin();
         it != groups.end(); ++it) {
      ChartBase* chart =
          ChartData->OpenChartFromDB(it->second.db_index, FULL_INIT);
      if (!IsSupportedSegmentSafetyPluginChart(chart)) continue;
      it->second.wrapper = dynamic_cast<ChartPlugInWrapper*>(chart);
    }
    plugin_batch_select_ms = batch_select_timer.Time();

    wxStopWatch batch_query_timer;
    for (std::map<int, SegmentSafetyPluginBatchGroup>::iterator it =
             groups.begin();
         it != groups.end(); ++it) {
      SegmentSafetyPluginBatchGroup& group = it->second;
      if (!group.wrapper || group.active_count <= 0) continue;
      OCPN_PluginChartSafetyGridRequestV1 request = {};
      request.struct_size = sizeof(request);
      request.abi_version = OCPN_PLUGIN_CHART_SAFETY_GRID_ABI_V1;
      request.min_lat = tile.min_lat;
      request.min_lon = tile.min_lon;
      request.lat_step = tile.resolution;
      request.lon_step = tile.resolution;
      request.rows = tile.rows;
      request.cols = tile.cols;
      request.select_radius_degrees = static_cast<float>(
          kSegmentSafetyGridResolutionDegrees * 0.75);
      request.active_cells = group.active.data();
      request.visitor_context = &group;
      request.visit_object = SegmentSafetyPluginBatchVisit;
      OCPN_PluginChartSafetyGridResultV1 provider_result = {};
      provider_result.struct_size = sizeof(provider_result);
      const double centre_lat =
          tile.min_lat + kSegmentSafetyGridTileDegrees / 2.0;
      const double centre_lon =
          tile.min_lon + kSegmentSafetyGridTileDegrees / 2.0;
      const ViewPort vp =
          SegmentSafetyHighestDetailViewPortAt(centre_lat, centre_lon);
      const int status = g_pi_manager->QueryPlugInChartSafetyGrid(
          group.wrapper, request, &provider_result, vp);
      if (status != 1 ||
          provider_result.abi_version !=
              OCPN_PLUGIN_CHART_SAFETY_GRID_ABI_V1 ||
          provider_result.processed_cells !=
              static_cast<uint32_t>(group.active_count)) {
        plugin_batch_fallback_cells += group.active_count;
        continue;
      }

      ++plugin_batch_groups;
      plugin_batch_cells += group.active_count;
      plugin_batch_candidate_objects += provider_result.candidate_objects;
      plugin_batch_hit_objects += provider_result.hit_objects;
      if (stats) ++stats->s57_chart_count;
      const bool provider_cache_allowed =
          (provider_result.result_flags &
           OCPN_PLUGIN_CHART_SAFETY_RESULT_DERIVED_CACHE_ALLOWED) != 0;
      for (size_t index = 0; index < grid_cell_count; ++index) {
        if (!group.active[index]) continue;
        plugin_batch_classified[index] = 1;
        plugin_batch_persistent_cache_allowed[index] =
            provider_cache_allowed ? 1 : 0;
        SegmentSafetyPointClass point_class = SEGMENT_SAFETY_POINT_WATER;
        if (group.land[index])
          point_class = SEGMENT_SAFETY_POINT_LAND;
        else if (group.drying[index])
          point_class = SEGMENT_SAFETY_POINT_DRYING;
        tile.classes[index] = static_cast<unsigned char>(point_class);
        const uint16_t hazards = SegmentSafetyPointHazardFlags(point_class);
        tile.hazard_flags[index] = hazards;
        tile.hazard_summary_flags |= hazards;
        const bool depth_known = group.has_depth[index] &&
                                 !group.unknown_danger_depth[index];
        plugin_batch_unknown_danger[index] =
            group.unknown_danger_depth[index] ? 1 : 0;
        tile.has_depth[index] = depth_known ? 1 : 0;
        tile.min_depth_m[index] =
            depth_known ? group.min_depth_m[index] : 0.0f;
        tile.has_drying[index] = group.drying[index] ? 1 : 0;
      }
      if (tile.source == PI_SEGMENT_SAFETY_SOURCE_NONE) {
        tile.source = PI_SEGMENT_SAFETY_SOURCE_PLUGIN_VECTOR;
        tile.chart_db_index = group.db_index;
        tile.chart_scale = group.wrapper->GetNativeScale();
        CopySegmentSafetyString(tile.chart_path, sizeof(tile.chart_path),
                                group.wrapper->GetFullPath().mb_str());
      }
    }

    // A detailed inset can legitimately cover a point without supplying a
    // DEPARE there (most commonly on an M_COVR boundary).  Preserve its
    // authoritative land/drying classification, but fill an otherwise
    // unknown water depth from the next licensed chart in the same sorted
    // priority chain.  An explicit danger object with unknown depth remains
    // unknown and cannot be hidden by a smaller-scale chart.
    size_t maximum_depth_candidate = 0;
    for (size_t index = 0; index < grid_cell_count; ++index)
      maximum_depth_candidate =
          std::max(maximum_depth_candidate,
                   plugin_depth_candidates[index].size());
    for (size_t rank = 1; rank < maximum_depth_candidate; ++rank) {
      std::map<int, SegmentSafetyPluginBatchGroup> fallback_groups;
      for (size_t index = 0; index < grid_cell_count; ++index) {
        if (!plugin_batch_classified[index] || tile.has_depth[index] ||
            plugin_batch_unknown_danger[index] ||
            tile.classes[index] != SEGMENT_SAFETY_POINT_WATER ||
            plugin_depth_candidates[index].size() <= rank)
          continue;
        const int db_index = plugin_depth_candidates[index][rank];
        std::map<int, SegmentSafetyPluginBatchGroup>::iterator found =
            fallback_groups.find(db_index);
        if (found == fallback_groups.end()) {
          found = fallback_groups
                      .insert(std::make_pair(
                          db_index,
                          SegmentSafetyPluginBatchGroup(grid_cell_count)))
                      .first;
          found->second.db_index = db_index;
        }
        found->second.active[index] = 1;
        ++found->second.active_count;
      }

      for (std::map<int, SegmentSafetyPluginBatchGroup>::iterator it =
               fallback_groups.begin();
           it != fallback_groups.end(); ++it) {
        ChartBase* chart =
            ChartData->OpenChartFromDB(it->second.db_index, FULL_INIT);
        if (!IsSupportedSegmentSafetyPluginChart(chart)) continue;
        it->second.wrapper = dynamic_cast<ChartPlugInWrapper*>(chart);
      }

      for (std::map<int, SegmentSafetyPluginBatchGroup>::iterator it =
               fallback_groups.begin();
           it != fallback_groups.end(); ++it) {
        SegmentSafetyPluginBatchGroup& group = it->second;
        if (!group.wrapper || group.active_count <= 0) continue;
        OCPN_PluginChartSafetyGridRequestV1 request = {};
        request.struct_size = sizeof(request);
        request.abi_version = OCPN_PLUGIN_CHART_SAFETY_GRID_ABI_V1;
        request.min_lat = tile.min_lat;
        request.min_lon = tile.min_lon;
        request.lat_step = tile.resolution;
        request.lon_step = tile.resolution;
        request.rows = tile.rows;
        request.cols = tile.cols;
        request.select_radius_degrees = static_cast<float>(
            kSegmentSafetyGridResolutionDegrees * 0.75);
        request.active_cells = group.active.data();
        request.visitor_context = &group;
        request.visit_object = SegmentSafetyPluginBatchVisit;
        OCPN_PluginChartSafetyGridResultV1 provider_result = {};
        provider_result.struct_size = sizeof(provider_result);
        const double centre_lat =
            tile.min_lat + kSegmentSafetyGridTileDegrees / 2.0;
        const double centre_lon =
            tile.min_lon + kSegmentSafetyGridTileDegrees / 2.0;
        const ViewPort vp =
            SegmentSafetyHighestDetailViewPortAt(centre_lat, centre_lon);
        const int status = g_pi_manager->QueryPlugInChartSafetyGrid(
            group.wrapper, request, &provider_result, vp);
        if (status != 1 ||
            provider_result.abi_version !=
                OCPN_PLUGIN_CHART_SAFETY_GRID_ABI_V1 ||
            provider_result.processed_cells !=
                static_cast<uint32_t>(group.active_count)) {
          plugin_batch_fallback_cells += group.active_count;
          continue;
        }

        ++plugin_batch_groups;
        plugin_batch_cells += group.active_count;
        plugin_batch_candidate_objects += provider_result.candidate_objects;
        plugin_batch_hit_objects += provider_result.hit_objects;
        const bool fallback_cache_allowed =
            (provider_result.result_flags &
             OCPN_PLUGIN_CHART_SAFETY_RESULT_DERIVED_CACHE_ALLOWED) != 0;
        for (size_t index = 0; index < grid_cell_count; ++index) {
          if (group.active[index] && !fallback_cache_allowed)
            plugin_batch_persistent_cache_allowed[index] = 0;
          if (!group.active[index]) continue;
          if (group.land[index] || group.drying[index] ||
              group.unknown_danger_depth[index])
            continue;
          if (group.has_depth[index]) {
            tile.has_depth[index] = 1;
            tile.min_depth_m[index] = group.min_depth_m[index];
          }
        }
      }
    }
    plugin_batch_query_ms = batch_query_timer.Time();
  }

  // A CM93 composite normally selects its scale and reconstructs viewport
  // state for every queried point.  A 41x41 safety tile therefore used to do
  // 1,681 full CM93 viewport selections, taking tens of seconds even in
  // obvious open water.  Prepare every scale over the complete tile once and
  // select the highest-detail real M_COVR at each point from that immutable
  // working set.  If another vector chart competes at the tile centre, retain
  // the general per-point selection path so chart priority is unchanged.
  cm93compchart* prepared_cm93 = NULL;
  int prepared_cm93_db_index = -1;
  ViewPort prepared_cm93_vp;
  long cm93_prepare_ms = 0;
  int cm93_batch_cells = 0;
  int cm93_fallback_cells = 0;
  bool cm93_clear_shortcut = false;
  int cm93_hazard_objects = 0;
  int cm93_depth_boundary_attempts = 0;
  int cm93_depth_boundary_recoveries = 0;
  if (ChartData) {
    const double centre_lat =
        tile.min_lat + kSegmentSafetyGridTileDegrees / 2.0;
    const double centre_lon =
        tile.min_lon + kSegmentSafetyGridTileDegrees / 2.0;
    std::set<int> centre_chart_indexes;
    SegmentSafetyCandidateChartsAt(centre_lat, centre_lon, centre_chart_indexes,
                                   stats);
    if (centre_chart_indexes.size() == 1) {
      prepared_cm93_db_index = *centre_chart_indexes.begin();
      ChartBase* chart =
          ChartData->OpenChartFromDB(prepared_cm93_db_index, FULL_INIT);
      prepared_cm93 = dynamic_cast<cm93compchart*>(chart);
      if (prepared_cm93) {
        wxStopWatch prepare_timer;
        prepared_cm93_vp =
            SegmentSafetyHighestDetailViewPortAt(centre_lat, centre_lon);
        prepared_cm93_vp.b_quilt = false;
        prepared_cm93_vp.b_FullScreenQuilt = false;
        // At 1 pixel/metre an 8192-pixel viewport covers a 0.05-degree
        // tile at every navigable latitude, including a generous boundary
        // halo.  CM93 loads every intersecting native cell for each scale.
        prepared_cm93_vp.pix_width = 8192;
        prepared_cm93_vp.pix_height = 8192;
        prepared_cm93_vp.SetBoxes();
        prepared_cm93->PrepareSafetyTile(prepared_cm93_vp);
        cm93_prepare_ms = prepare_timer.Time();
      }
    }
  }

  // Most cold-route CM93 work is in 1,681 repeated point-in-object scans per
  // 0.05-degree tile.  Before doing those scans, prove that no loaded CM93
  // land/drying object's bounding box can affect this tile and that every
  // grid point has real M_COVR coverage.  This is a negative-only,
  // conservative shortcut: coastal/ambiguous tiles retain exact
  // classification, and depth-enabled routing also retains exact DEPARE
  // extraction.
  if (prepared_cm93 && !require_depth &&
      !prepared_cm93->SafetyAreaHazardMayIntersect(
          tile.min_lat, tile.min_lat + kSegmentSafetyGridTileDegrees,
          tile.min_lon, tile.min_lon + kSegmentSafetyGridTileDegrees,
          &cm93_hazard_objects)) {
    cm93_clear_shortcut = true;
    for (int r = 0; r < tile.rows && cm93_clear_shortcut; ++r) {
      const double cell_lat = ocpn::chart_safety::GlobalGridCoordinate(
          lat_tile, r, kTileCells, tile.resolution);
      for (int c = 0; c < tile.cols; ++c) {
        const double cell_lon = ocpn::chart_safety::GlobalGridCoordinate(
            lon_tile, c, kTileCells, tile.resolution);
        if (!prepared_cm93->GetHighestDetailSafetyChartAt(cell_lat, cell_lon)) {
          cm93_clear_shortcut = false;
          break;
        }
      }
    }
  }

  int land = 0, water = 0, drying = 0, unknown = 0;
  for (int r = 0; r < tile.rows; ++r) {
    const double cell_lat = ocpn::chart_safety::GlobalGridCoordinate(
        lat_tile, r, kTileCells, tile.resolution);
    for (int c = 0; c < tile.cols; ++c) {
      const int cell_index = r * tile.cols + c;
      if (plugin_batch_classified[cell_index]) {
        const SegmentSafetyPointClass point_class =
            static_cast<SegmentSafetyPointClass>(tile.classes[cell_index]);
        switch (point_class) {
          case SEGMENT_SAFETY_POINT_LAND:
            ++land;
            break;
          case SEGMENT_SAFETY_POINT_WATER:
            ++water;
            break;
          case SEGMENT_SAFETY_POINT_DRYING:
            ++drying;
            break;
          default:
            ++unknown;
            break;
        }
        continue;
      }
      if (cm93_clear_shortcut) {
        tile.classes[cell_index] = (unsigned char)SEGMENT_SAFETY_POINT_WATER;
        tile.hazard_flags[cell_index] = SEGMENT_SAFETY_HAZARD_NONE;
        ++water;
        continue;
      }
      const double cell_lon = ocpn::chart_safety::GlobalGridCoordinate(
          lon_tile, c, kTileCells, tile.resolution);
      PlugInSegmentSafetySource source = PI_SEGMENT_SAFETY_SOURCE_NONE;
      PlugInSegmentSafetyResult cell_result = {};
      cell_result.struct_size = sizeof(cell_result);
      InitSegmentSafetyResult(&cell_result);
      SegmentSafetyPointClass point_class = SEGMENT_SAFETY_POINT_NO_DATA;
      cm93chart* prepared_point_chart =
          prepared_cm93
              ? prepared_cm93->GetHighestDetailSafetyChartAt(cell_lat, cell_lon)
              : NULL;
      if (prepared_point_chart) {
        point_class = ChartPointSafetyClassAtPreparedCm93(
            prepared_point_chart, prepared_cm93_db_index, cell_lat, cell_lon,
            &prepared_cm93_vp, &source, stats, &cell_result);
        ++cm93_batch_cells;
      } else {
        point_class = ChartPointSafetyClassAtRaw(cell_lat, cell_lon, &source,
                                                 stats, &cell_result);
        if (prepared_cm93) ++cm93_fallback_cells;
      }
      if (require_depth && prepared_cm93 &&
          point_class == SEGMENT_SAFETY_POINT_WATER &&
          !cell_result.has_depth) {
        ++cm93_depth_boundary_attempts;
        if (RecoverPreparedCm93BoundaryDepth(cell_lat, cell_lon,
                                             tile.resolution, stats,
                                             &cell_result))
          ++cm93_depth_boundary_recoveries;
      }
      tile.classes[cell_index] = (unsigned char)point_class;
      uint16_t cell_hazards = SegmentSafetyPointHazardFlags(point_class);
      tile.hazard_flags[cell_index] = cell_hazards;
      tile.hazard_summary_flags |= cell_hazards;
      if (SegmentSafetyResultHas(
              &cell_result,
              offsetof(PlugInSegmentSafetyResult, depth_source_attribute),
              sizeof(cell_result.depth_source_attribute))) {
        tile.has_depth[cell_index] = cell_result.has_depth ? 1 : 0;
        tile.min_depth_m[cell_index] = (float)cell_result.min_depth_m;
        tile.has_drying[cell_index] = cell_result.has_drying ? 1 : 0;
      }
      if (tile.source == PI_SEGMENT_SAFETY_SOURCE_NONE &&
          source != PI_SEGMENT_SAFETY_SOURCE_NONE)
        tile.source = source;
      if (tile.chart_db_index < 0 && cell_result.chart_db_index >= 0) {
        tile.chart_db_index = cell_result.chart_db_index;
        tile.chart_scale = cell_result.chart_scale;
        snprintf(tile.chart_path, sizeof(tile.chart_path), "%s",
                 cell_result.chart_path);
      }
      switch (point_class) {
        case SEGMENT_SAFETY_POINT_LAND:
          ++land;
          break;
        case SEGMENT_SAFETY_POINT_WATER:
          ++water;
          break;
        case SEGMENT_SAFETY_POINT_DRYING:
          ++drying;
          break;
        default:
          ++unknown;
          break;
      }
    }
  }

  int build_ms = timer.Time();
  tile.land_count = land;
  tile.water_count = water;
  tile.drying_count = drying;
  tile.unknown_count = unknown;
  tile.depth_complete = !cm93_clear_shortcut;
  tile.persistent_cache_allowed =
      tile.source != PI_SEGMENT_SAFETY_SOURCE_PLUGIN_VECTOR;
  if (tile.source == PI_SEGMENT_SAFETY_SOURCE_PLUGIN_VECTOR) {
    tile.persistent_cache_allowed = true;
    for (size_t index = 0; index < grid_cell_count; ++index) {
      // A mixed/legacy plugin tile is kept in RAM unless every cell came from
      // a provider which explicitly advertised the derived-cache contract.
      if (!plugin_batch_classified[index] ||
          !plugin_batch_persistent_cache_allowed[index]) {
        tile.persistent_cache_allowed = false;
        break;
      }
    }
  }
  if (cm93_clear_shortcut) {
    const double centre_lat =
        tile.min_lat + kSegmentSafetyGridTileDegrees / 2.0;
    const double centre_lon =
        tile.min_lon + kSegmentSafetyGridTileDegrees / 2.0;
    cm93chart* representative =
        prepared_cm93->GetHighestDetailSafetyChartAt(centre_lat, centre_lon);
    tile.source = PI_SEGMENT_SAFETY_SOURCE_CM93;
    tile.chart_db_index = prepared_cm93_db_index;
    tile.chart_scale = representative ? representative->GetNativeScale()
                                      : prepared_cm93->GetNativeScale();
    snprintf(tile.chart_path, sizeof(tile.chart_path), "%s",
             (representative ? representative->GetFullPath()
                             : prepared_cm93->GetFullPath())
                 .mb_str()
                 .data());
  }
  if (stats) {
    stats->grid_build_ms += build_ms;
    stats->grid_cells_total += tile.rows * tile.cols;
    stats->grid_cells_land += land;
    stats->grid_cells_water += water;
    stats->grid_cells_drying += drying;
    stats->grid_cells_unknown += unknown;
  }

  wxLogMessage(
      "SEGMENT_SAFETY_GRID built key=%ld:%ld group=%d bbox=[lat %.6f..%.6f "
      "lon %.6f..%.6f] resolution_deg=%.6f cells=%d land=%d water=%d "
      "drying=%d unknown=%d build_ms=%d source=%d chart_db_index=%d "
      "chart_scale=%d chart_path=\"%s\" cm93_prepare_ms=%ld "
      "cm93_batch_cells=%d cm93_fallback_cells=%d "
      "cm93_clear_shortcut=%d cm93_hazard_objects=%d "
      "cm93_depth_boundary_attempts=%d cm93_depth_boundary_recoveries=%d "
      "depth_complete=%d",
      lat_tile, lon_tile, tile.group_index, tile.min_lat,
      tile.min_lat + kSegmentSafetyGridTileDegrees, tile.min_lon,
      tile.min_lon + kSegmentSafetyGridTileDegrees, tile.resolution,
      tile.rows * tile.cols, land, water, drying, unknown, build_ms,
      tile.source, tile.chart_db_index, tile.chart_scale, tile.chart_path,
      cm93_prepare_ms, cm93_batch_cells, cm93_fallback_cells,
      cm93_clear_shortcut ? 1 : 0, cm93_hazard_objects,
      cm93_depth_boundary_attempts, cm93_depth_boundary_recoveries,
      tile.depth_complete ? 1 : 0);
  if (plugin_batch_groups || plugin_batch_fallback_cells) {
    wxLogMessage(
        "SEGMENT_SAFETY_PLUGIN_BATCH key=%ld:%ld groups=%d cells=%d "
        "fallback_cells=%d select_ms=%ld query_ms=%ld "
        "candidate_objects=%d hit_objects=%d",
        lat_tile, lon_tile, plugin_batch_groups, plugin_batch_cells,
        plugin_batch_fallback_cells, plugin_batch_select_ms,
        plugin_batch_query_ms, plugin_batch_candidate_objects,
        plugin_batch_hit_objects);
  }

  return tile;
}

bool SegmentSafetyAllTouchedTilesAreWater(double lat1, double lon1, double lat2,
                                          double lon2, double safety_margin_nm,
                                          double bearing, double dist_nm,
                                          int samples,
                                          SegmentSafetyCoreStats* stats) {
  if (stats) stats->segment_sample_count += samples;
  std::set<std::pair<long, long> > tiles;
  for (int i = 0; i < samples; ++i) {
    double sample_dist = samples == 1 ? 0.0 : dist_nm * i / (samples - 1);
    double lat = lat1;
    double lon = lon1;
    if (sample_dist > 0.0)
      ll_gc_ll(lat1, lon1, bearing, sample_dist, &lat, &lon);

    long lat_tile = 0;
    long lon_tile = 0;
    SegmentSafetyGridTileKey(lat, lon, &lat_tile, &lon_tile);
    tiles.insert(std::make_pair(lat_tile, lon_tile));

    if (safety_margin_nm > 0.0) {
      double left_lat, left_lon, right_lat, right_lon;
      ll_gc_ll(lat, lon, SegmentSafetyNormalizeBearing(bearing - 90.0),
               safety_margin_nm, &left_lat, &left_lon);
      ll_gc_ll(lat, lon, SegmentSafetyNormalizeBearing(bearing + 90.0),
               safety_margin_nm, &right_lat, &right_lon);
      SegmentSafetyGridTileKey(left_lat, left_lon, &lat_tile, &lon_tile);
      tiles.insert(std::make_pair(lat_tile, lon_tile));
      SegmentSafetyGridTileKey(right_lat, right_lon, &lat_tile, &lon_tile);
      tiles.insert(std::make_pair(lat_tile, lon_tile));
    }
  }

  if (tiles.empty()) return false;

  for (std::set<std::pair<long, long> >::const_iterator it = tiles.begin();
       it != tiles.end(); ++it) {
    std::string key = SegmentSafetyGridTileKeyForIndices(it->first, it->second);
    CachedPointSafetyGridTile tile;
    if (!LookupSegmentSafetyGridTile(key, &tile)) {
      bool built = false;
      if (!EnsureSegmentSafetyGridTile(it->first, it->second, stats, &built))
        return false;
      if (built)
        RecordUnexpectedSegmentSafetyTileBuild(stats, it->first, it->second);
      if (!LookupSegmentSafetyGridTile(key, &tile)) return false;
    } else if (stats) {
      ++stats->grid_cache_hits;
    }

    if (tile.classes.empty() ||
        tile.hazard_summary_flags != SEGMENT_SAFETY_HAZARD_NONE)
      return false;
  }

  if (stats) ++stats->water_tile_shortcuts;
  return true;
}

SegmentSafetyPointClass ChartPointSafetyClassAt(
    double lat, double lon, PlugInSegmentSafetySource* source,
    SegmentSafetyCoreStats* stats, PlugInSegmentSafetyResult* result = NULL) {
  long lat_tile = 0;
  long lon_tile = 0;
  std::string key = SegmentSafetyGridTileKey(lat, lon, &lat_tile, &lon_tile);
  CachedPointSafetyGridTile tile;
  if (!LookupSegmentSafetyGridTile(key, &tile)) {
    bool built = false;
    EnsureSegmentSafetyGridTile(lat_tile, lon_tile, stats, &built);
    if (built)
      RecordUnexpectedSegmentSafetyTileBuild(stats, lat_tile, lon_tile);
    LookupSegmentSafetyGridTile(key, &tile);
  } else if (stats) {
    ++stats->grid_cache_hits;
  }

  if (tile.classes.empty()) {
    if (!wxThread::IsMain()) return SEGMENT_SAFETY_POINT_NO_DATA;
    return ChartPointSafetyClassAtRaw(lat, lon, source, stats, result);
  }

  if (stats) ++stats->grid_lookups;
  int row = (int)lround((lat - tile.min_lat) / tile.resolution);
  int col = (int)lround((lon - tile.min_lon) / tile.resolution);
  if (row < 0 || row >= tile.rows || col < 0 || col >= tile.cols) {
    if (!wxThread::IsMain()) return SEGMENT_SAFETY_POINT_NO_DATA;
    return ChartPointSafetyClassAtRaw(lat, lon, source, stats, result);
  }

  SegmentSafetyPointClass point_class =
      (SegmentSafetyPointClass)tile.classes[row * tile.cols + col];
  if (source) *source = tile.source;
  int cell_index = row * tile.cols + col;
  if (SegmentSafetyResultHas(result,
                             offsetof(PlugInSegmentSafetyResult, hit_object),
                             sizeof(result->hit_object))) {
    result->chart_db_index = tile.chart_db_index;
    result->chart_scale = tile.chart_scale;
    strncpy(result->chart_path, tile.chart_path,
            sizeof(result->chart_path) - 1);
    result->chart_path[sizeof(result->chart_path) - 1] = '\0';
    if (point_class == SEGMENT_SAFETY_POINT_LAND)
      strncpy(result->hit_object, "grid LAND cell",
              sizeof(result->hit_object) - 1);
  }
  if (SegmentSafetyResultHas(
          result, offsetof(PlugInSegmentSafetyResult, depth_source_attribute),
          sizeof(result->depth_source_attribute))) {
    bool has_depth = cell_index >= 0 &&
                     cell_index < (int)tile.has_depth.size() &&
                     tile.has_depth[cell_index] != 0;
    bool has_drying = cell_index >= 0 &&
                      cell_index < (int)tile.has_drying.size() &&
                      tile.has_drying[cell_index] != 0;
    result->has_depth = has_depth ? 1 : 0;
    result->min_depth_m = has_depth ? tile.min_depth_m[cell_index] : 0.0;
    result->has_drying = has_drying ? 1 : 0;
    if (has_depth) {
      strncpy(result->depth_source_object, "grid DEPARE cell",
              sizeof(result->depth_source_object) - 1);
      result->depth_source_object[sizeof(result->depth_source_object) - 1] =
          '\0';
      strncpy(result->depth_source_attribute, "DEPARE/DRVAL1",
              sizeof(result->depth_source_attribute) - 1);
      result
          ->depth_source_attribute[sizeof(result->depth_source_attribute) - 1] =
          '\0';
    }
  }
  return point_class;
}

bool SegmentSafetyWaterNeighborhoodAt(double lat, double lon, int radius_cells,
                                      SegmentSafetyCoreStats* stats) {
  for (int dlat = -radius_cells; dlat <= radius_cells; ++dlat) {
    for (int dlon = -radius_cells; dlon <= radius_cells; ++dlon) {
      double check_lat = lat + dlat * kSegmentSafetyGridResolutionDegrees;
      double check_lon = lon + dlon * kSegmentSafetyGridResolutionDegrees;
      PlugInSegmentSafetySource source = PI_SEGMENT_SAFETY_SOURCE_NONE;
      SegmentSafetyPointClass point_class =
          ChartPointSafetyClassAt(check_lat, check_lon, &source, stats);
      if (point_class != SEGMENT_SAFETY_POINT_WATER) return false;
    }
  }
  return true;
}

bool SegmentSafetyCoarseSampledCellsAreWater(double lat1, double lon1,
                                             double lat2, double lon2,
                                             double safety_margin_nm,
                                             double bearing, double dist_nm,
                                             SegmentSafetyCoreStats* stats) {
  const double coarse_spacing_nm = 0.25;
  const int max_coarse_samples = 256;
  int coarse_samples = wxMax(
      2, wxMin(max_coarse_samples, (int)ceil(dist_nm / coarse_spacing_nm) + 1));
  if (stats) stats->segment_sample_count += coarse_samples;

  for (int i = 0; i < coarse_samples; ++i) {
    double sample_dist =
        coarse_samples == 1 ? 0.0 : dist_nm * i / (coarse_samples - 1);
    double lat = lat1;
    double lon = lon1;
    if (sample_dist > 0.0)
      ll_gc_ll(lat1, lon1, bearing, sample_dist, &lat, &lon);

    if (!SegmentSafetyWaterNeighborhoodAt(lat, lon, 1, stats)) return false;

    if (safety_margin_nm > 0.0) {
      double left_lat, left_lon, right_lat, right_lon;
      ll_gc_ll(lat, lon, SegmentSafetyNormalizeBearing(bearing - 90.0),
               safety_margin_nm, &left_lat, &left_lon);
      ll_gc_ll(lat, lon, SegmentSafetyNormalizeBearing(bearing + 90.0),
               safety_margin_nm, &right_lat, &right_lon);
      if (!SegmentSafetyWaterNeighborhoodAt(left_lat, left_lon, 1, stats) ||
          !SegmentSafetyWaterNeighborhoodAt(right_lat, right_lon, 1, stats))
        return false;
    }
  }

  if (stats) ++stats->water_tile_shortcuts;
  return true;
}

bool SegmentSafetyGridCellAt(long lat_cell, long lon_cell,
                             PlugInSegmentSafetySource* source,
                             SegmentSafetyCoreStats* stats,
                             PlugInSegmentSafetyResult* result,
                             SegmentSafetyPointClass* point_class,
                             uint16_t* hazard_flags) {
  double lat = lat_cell * kSegmentSafetyGridResolutionDegrees;
  double lon = lon_cell * kSegmentSafetyGridResolutionDegrees;

  long lat_tile = 0;
  long lon_tile = 0;
  std::string key = SegmentSafetyGridTileKey(lat, lon, &lat_tile, &lon_tile);
  if (!LookupSegmentSafetyGridTile(key, NULL)) {
    bool built = false;
    if (!EnsureSegmentSafetyGridTile(lat_tile, lon_tile, stats, &built))
      return false;
    if (built)
      RecordUnexpectedSegmentSafetyTileBuild(stats, lat_tile, lon_tile);
  } else if (stats) {
    ++stats->grid_cache_hits;
  }

  wxMutexLocker lock(s_segment_safety_cache_mutex);
  std::map<std::string, CachedPointSafetyGridTile>::const_iterator it =
      s_segment_safety_grid_cache.find(key);
  if (it == s_segment_safety_grid_cache.end()) return false;

  const CachedPointSafetyGridTile& tile = it->second;
  if (tile.classes.empty()) return false;

  int row = (int)lround((lat - tile.min_lat) / tile.resolution);
  int col = (int)lround((lon - tile.min_lon) / tile.resolution);
  if (row < 0 || row >= tile.rows || col < 0 || col >= tile.cols) return false;

  int cell_index = row * tile.cols + col;
  SegmentSafetyPointClass cls =
      (SegmentSafetyPointClass)tile.classes[cell_index];
  uint16_t hazards =
      cell_index >= 0 && cell_index < (int)tile.hazard_flags.size()
          ? tile.hazard_flags[cell_index]
          : (uint16_t)SEGMENT_SAFETY_HAZARD_NO_CHART;
  if (point_class) *point_class = cls;
  if (hazard_flags) *hazard_flags = hazards;
  if (source) *source = tile.source;
  if (stats) ++stats->grid_lookups;

  if (SegmentSafetyResultHas(result,
                             offsetof(PlugInSegmentSafetyResult, hit_object),
                             sizeof(result->hit_object))) {
    result->chart_db_index = tile.chart_db_index;
    result->chart_scale = tile.chart_scale;
    strncpy(result->chart_path, tile.chart_path,
            sizeof(result->chart_path) - 1);
    result->chart_path[sizeof(result->chart_path) - 1] = '\0';
    if (cls == SEGMENT_SAFETY_POINT_LAND)
      strncpy(result->hit_object, "grid LAND cell",
              sizeof(result->hit_object) - 1);
  }
  if (SegmentSafetyResultHas(
          result, offsetof(PlugInSegmentSafetyResult, depth_source_attribute),
          sizeof(result->depth_source_attribute))) {
    bool has_depth = cell_index >= 0 &&
                     cell_index < (int)tile.has_depth.size() &&
                     tile.has_depth[cell_index] != 0;
    bool has_drying = cell_index >= 0 &&
                      cell_index < (int)tile.has_drying.size() &&
                      tile.has_drying[cell_index] != 0;
    result->has_depth = has_depth ? 1 : 0;
    result->min_depth_m = has_depth ? tile.min_depth_m[cell_index] : 0.0;
    result->has_drying = has_drying ? 1 : 0;
    if (has_depth) {
      strncpy(result->depth_source_object, "grid DEPARE cell",
              sizeof(result->depth_source_object) - 1);
      result->depth_source_object[sizeof(result->depth_source_object) - 1] =
          '\0';
      strncpy(result->depth_source_attribute, "DEPARE/DRVAL1",
              sizeof(result->depth_source_attribute) - 1);
      result
          ->depth_source_attribute[sizeof(result->depth_source_attribute) - 1] =
          '\0';
    }
  }

  return cls != SEGMENT_SAFETY_POINT_NO_DATA;
}

bool SegmentSafetyRouteMaskCellAt(
    long lat_cell, long lon_cell, double safety_margin_nm, bool check_depth,
    double minimum_depth_m, bool force_authoritative_fine,
    SegmentSafetyCoreStats* stats, PlugInSegmentSafetyResult* result,
    uint16_t* block_flags, PlugInSegmentSafetySource* source) {
  double lat = lat_cell * kSegmentSafetyGridResolutionDegrees;
  double lon = lon_cell * kSegmentSafetyGridResolutionDegrees;
  long lat_tile = 0;
  long lon_tile = 0;
  SegmentSafetyGridTileKey(lat, lon, &lat_tile, &lon_tile);
  std::string key = SegmentSafetyRouteMaskKey(
      lat_tile, lon_tile, safety_margin_nm, check_depth, minimum_depth_m);
  CachedSegmentSafetyRouteMaskTile mask;
  if (!LookupSegmentSafetyRouteMaskTile(key, &mask) ||
      (force_authoritative_fine && !mask.authoritative_fine)) {
    bool built = false;
    if (!EnsureSegmentSafetyRouteMaskTile(lat_tile, lon_tile, safety_margin_nm,
                                          check_depth, minimum_depth_m, stats,
                                          &built, force_authoritative_fine))
      return false;
    if (built)
      RecordUnexpectedSegmentSafetyTileBuild(stats, lat_tile, lon_tile);
    if (!LookupSegmentSafetyRouteMaskTile(key, &mask)) return false;
    if (force_authoritative_fine && !mask.authoritative_fine) return false;
  } else if (stats) {
    ++stats->grid_cache_hits;
  }

  if (mask.block_flags.empty()) return false;
  int row = (int)lround((lat - mask.min_lat) / mask.resolution);
  int col = (int)lround((lon - mask.min_lon) / mask.resolution);
  if (row < 0 || row >= mask.rows || col < 0 || col >= mask.cols) return false;

  int cell_index = row * mask.cols + col;
  uint16_t flags = cell_index >= 0 && cell_index < (int)mask.block_flags.size()
                       ? mask.block_flags[cell_index]
                       : (uint16_t)SEGMENT_SAFETY_ROUTE_NEEDS_TILE;
  if (block_flags) *block_flags = flags;
  if (source) *source = mask.source;
  if (stats) ++stats->grid_lookups;

  if (SegmentSafetyResultHas(result,
                             offsetof(PlugInSegmentSafetyResult, hit_object),
                             sizeof(result->hit_object))) {
    result->chart_db_index = mask.chart_db_index;
    result->chart_scale = mask.chart_scale;
    strncpy(result->chart_path, mask.chart_path,
            sizeof(result->chart_path) - 1);
    result->chart_path[sizeof(result->chart_path) - 1] = '\0';
  }
  return true;
}

void SegmentSafetySetRouteMaskHitResult(PlugInSegmentSafetyResult* result,
                                        uint16_t flags,
                                        PlugInSegmentSafetySource source,
                                        long lat_cell, long lon_cell,
                                        int sample_index, int sample_count) {
  if (!result) return;

  result->hit_sample_lat = lat_cell * kSegmentSafetyGridResolutionDegrees;
  result->hit_sample_lon = lon_cell * kSegmentSafetyGridResolutionDegrees;
  result->hit_sample_index = sample_index;
  result->hit_sample_count = sample_count;
  SetSegmentSafetySource(result, source);

  if (flags & SEGMENT_SAFETY_ROUTE_BLOCK_LAND) {
    SetSegmentSafetyStatus(result, PI_SEGMENT_SAFETY_CROSSES_LAND);
    SetSegmentSafetyMessage(result, "route mask segment intersects chart land");
    strncpy(result->hit_object, "route mask LAND cell",
            sizeof(result->hit_object) - 1);
  } else if (flags & SEGMENT_SAFETY_ROUTE_BLOCK_DRYING) {
    SetSegmentSafetyStatus(result, PI_SEGMENT_SAFETY_DRYING_AREA);
    SetSegmentSafetyMessage(result,
                            "route mask segment intersects drying area");
    strncpy(result->hit_object, "route mask DRYING cell",
            sizeof(result->hit_object) - 1);
  } else if (flags & SEGMENT_SAFETY_ROUTE_BLOCK_TOO_SHALLOW) {
    SetSegmentSafetyStatus(result, PI_SEGMENT_SAFETY_TOO_SHALLOW);
    SetSegmentSafetyMessage(result, "route mask segment is too shallow");
    strncpy(result->hit_object, "route mask TOO_SHALLOW cell",
            sizeof(result->hit_object) - 1);
  } else if (flags & SEGMENT_SAFETY_ROUTE_BLOCK_UNKNOWN_DEPTH) {
    SetSegmentSafetyStatus(result, PI_SEGMENT_SAFETY_UNKNOWN_DEPTH);
    SetSegmentSafetyMessage(result, "route mask segment has unknown depth");
    strncpy(result->hit_object, "route mask UNKNOWN_DEPTH cell",
            sizeof(result->hit_object) - 1);
  } else if (flags & SEGMENT_SAFETY_ROUTE_BLOCK_NO_CHART) {
    SetSegmentSafetyStatus(result, PI_SEGMENT_SAFETY_NO_DATA);
    SetSegmentSafetyMessage(result, "route mask segment has no chart coverage");
    strncpy(result->hit_object, "route mask NO_CHART cell",
            sizeof(result->hit_object) - 1);
  } else if (flags & SEGMENT_SAFETY_ROUTE_BLOCK_UNKNOWN_CLASS) {
    SetSegmentSafetyStatus(result, PI_SEGMENT_SAFETY_NO_DATA);
    SetSegmentSafetyMessage(result,
                            "route mask segment has unknown chart class");
    strncpy(result->hit_object, "route mask UNKNOWN_CLASS cell",
            sizeof(result->hit_object) - 1);
  } else if (flags & SEGMENT_SAFETY_ROUTE_NEEDS_TILE) {
    SetSegmentSafetyStatus(result, PI_SEGMENT_SAFETY_UNSAFE_AREA);
    SetSegmentSafetyMessage(result, "route mask tile is not built");
    strncpy(result->hit_object, "route mask NEEDS_TILE cell",
            sizeof(result->hit_object) - 1);
  } else if (flags & SEGMENT_SAFETY_ROUTE_BLOCK_MARGIN) {
    SetSegmentSafetyStatus(result, PI_SEGMENT_SAFETY_WITHIN_LAND_MARGIN);
    SetSegmentSafetyMessage(result, "route mask segment enters safety margin");
    strncpy(result->hit_object, "route mask MARGIN cell",
            sizeof(result->hit_object) - 1);
  }
  result->hit_object[sizeof(result->hit_object) - 1] = '\0';
}

bool SegmentSafetyCoarseRouteMaskCertifiedSafeCheck(
    double lat1, double lon1, double lat2, double lon2, double safety_margin_nm,
    bool check_depth, double minimum_depth_m, PlugInSegmentSafetyResult* result,
    SegmentSafetyCoreStats* stats, bool* answered) {
  if (answered) *answered = false;

  double coarse_degrees =
      kSegmentSafetyGridTileDegrees * kSegmentSafetyCoarseRouteMaskFactor;
  long min_lat_cell = (long)floor(wxMin(lat1, lat2) / coarse_degrees);
  long max_lat_cell = (long)floor(wxMax(lat1, lat2) / coarse_degrees);
  long min_lon_cell = (long)floor(wxMin(lon1, lon2) / coarse_degrees);
  long max_lon_cell = (long)floor(wxMax(lon1, lon2) / coarse_degrees);

  long lat_count = max_lat_cell - min_lat_cell + 1;
  long lon_count = max_lon_cell - min_lon_cell + 1;
  long coarse_count =
      lat_count > 0 && lon_count > 0 ? lat_count * lon_count : 0;
  const long max_coarse_bbox_cells = 256;
  if (coarse_count <= 0 || coarse_count > max_coarse_bbox_cells) {
    if (stats) ++stats->coarse_unknown_fallbacks;
    return false;
  }

  PlugInSegmentSafetySource first_source = PI_SEGMENT_SAFETY_SOURCE_NONE;
  for (long lat_cell = min_lat_cell; lat_cell <= max_lat_cell; ++lat_cell) {
    for (long lon_cell = min_lon_cell; lon_cell <= max_lon_cell; ++lon_cell) {
      CachedSegmentSafetyCoarseRouteMaskCell coarse;
      if (stats) ++stats->coarse_cells_checked;
      if (!EnsureSegmentSafetyCoarseRouteMaskCell(
              lat_cell, lon_cell, safety_margin_nm, check_depth,
              minimum_depth_m, &coarse, stats)) {
        if (stats) ++stats->coarse_missing;
        return false;
      }

      if (coarse.source != PI_SEGMENT_SAFETY_SOURCE_NONE &&
          first_source == PI_SEGMENT_SAFETY_SOURCE_NONE)
        first_source = coarse.source;

      if (coarse.state != SEGMENT_SAFETY_COARSE_CERTIFIED_SAFE) {
        if (stats) {
          if (coarse.state == SEGMENT_SAFETY_COARSE_NO_CHART)
            ++stats->coarse_no_chart;
          else if (coarse.state == SEGMENT_SAFETY_COARSE_DEPTH_UNPROVEN)
            ++stats->coarse_depth_unproven;
          else
            ++stats->coarse_mixed_fallbacks;
        }
        return false;
      }
    }
  }

  if (answered) *answered = true;
  if (stats) {
    ++stats->coarse_certified_safe_hits;
    stats->fine_tiles_avoided +=
        (int)(coarse_count * kSegmentSafetyCoarseRouteMaskFactor *
              kSegmentSafetyCoarseRouteMaskFactor);
  }
  SetSegmentSafetyStatus(result, PI_SEGMENT_SAFETY_SAFE);
  SetSegmentSafetySource(result, first_source != PI_SEGMENT_SAFETY_SOURCE_NONE
                                     ? first_source
                                     : PI_SEGMENT_SAFETY_SOURCE_VECTOR_CHART);
  SetSegmentSafetyDiagnosticReason(result,
                                   PI_SEGMENT_SAFETY_DIAG_CHART_GEOMETRY_CLEAR);
  SetSegmentSafetyMessage(result, "coarse route mask certified clear");
  return true;
}

bool SegmentSafetyRouteMaskTraversalCheck(
    double lat1, double lon1, double lat2, double lon2, double safety_margin_nm,
    bool check_depth, double minimum_depth_m, bool force_authoritative_fine,
    PlugInSegmentSafetyResult* result, bool* chart_data_available,
    SegmentSafetyCoreStats* stats, bool* open_water_shortcut, bool* answered) {
  if (answered) *answered = false;

  bool coarse_answered = false;
  if (!force_authoritative_fine &&
      SegmentSafetyCoarseRouteMaskCertifiedSafeCheck(
          lat1, lon1, lat2, lon2, safety_margin_nm, check_depth,
          minimum_depth_m, result, stats, &coarse_answered)) {
    if (chart_data_available) *chart_data_available = true;
    if (open_water_shortcut) *open_water_shortcut = true;
    if (answered) *answered = true;
    return false;
  }
  if (!force_authoritative_fine && coarse_answered) {
    if (chart_data_available) *chart_data_available = true;
    if (open_water_shortcut) *open_water_shortcut = true;
    if (answered) *answered = true;
    return false;
  }

  long y0 = lround(lat1 / kSegmentSafetyGridResolutionDegrees);
  long x0 = lround(lon1 / kSegmentSafetyGridResolutionDegrees);
  long y1 = lround(lat2 / kSegmentSafetyGridResolutionDegrees);
  long x1 = lround(lon2 / kSegmentSafetyGridResolutionDegrees);
  long dx = labs(x1 - x0);
  long dy = labs(y1 - y0);
  long sx = x0 < x1 ? 1 : -1;
  long sy = y0 < y1 ? 1 : -1;
  long err = dx - dy;
  long steps = wxMax(dx, dy) + 1;
  if (steps <= 0) steps = 1;
  if (stats) stats->segment_sample_count += steps;

  bool any_chart_data = false;
  bool all_clear = true;
  bool missing_mask = false;
  PlugInSegmentSafetySource first_source = PI_SEGMENT_SAFETY_SOURCE_NONE;

  long x = x0;
  long y = y0;
  for (long i = 0; i < steps; ++i) {
    uint16_t flags = SEGMENT_SAFETY_ROUTE_NEEDS_TILE;
    PlugInSegmentSafetySource source = PI_SEGMENT_SAFETY_SOURCE_NONE;
    if (!SegmentSafetyRouteMaskCellAt(y, x, safety_margin_nm, check_depth,
                                      minimum_depth_m, force_authoritative_fine,
                                      stats, result, &flags, &source)) {
      // Worker-side misses enqueue their exact mask tile.  Continue walking
      // the segment so a single query publishes every missing tile instead
      // of forcing one worker/GUI round trip per 0.05-degree boundary.  The
      // query still fails closed below until all samples are available.
      missing_mask = true;
      all_clear = false;
    } else {
      any_chart_data = true;
      if (first_source == PI_SEGMENT_SAFETY_SOURCE_NONE) first_source = source;
      if (flags != SEGMENT_SAFETY_ROUTE_CLEAR) {
        all_clear = false;
        if (chart_data_available) *chart_data_available = true;
        if (answered) *answered = true;
        SegmentSafetySetRouteMaskHitResult(result, flags, source, y, x, (int)i,
                                           (int)steps);
        return true;
      }
    }

    if (x == x1 && y == y1) break;
    long e2 = 2 * err;
    if (e2 > -dy) {
      err -= dy;
      x += sx;
    }
    if (e2 < dx) {
      err += dx;
      y += sy;
    }
  }

  if (missing_mask) {
    if (answered) *answered = false;
    return false;
  }

  if (any_chart_data) {
    if (chart_data_available) *chart_data_available = true;
    if (answered) *answered = true;
    if (GetSegmentSafetySource(result) == PI_SEGMENT_SAFETY_SOURCE_NONE)
      SetSegmentSafetySource(result, first_source);
    if (open_water_shortcut && all_clear) *open_water_shortcut = true;
    if (all_clear && stats) ++stats->water_tile_shortcuts;
  }
  return false;
}

void SegmentSafetyCheckGridCellNeighborhood(
    long lat_cell, long lon_cell, int radius_cells,
    PlugInSegmentSafetySource* first_source, bool* any_chart_data,
    bool* all_water, bool* land_hit, bool* drying_hit, bool* shallow_hit,
    bool* unknown_depth_hit, bool check_depth, double minimum_depth_m,
    SegmentSafetyCoreStats* stats, PlugInSegmentSafetyResult* result,
    int sample_index, int sample_count) {
  for (int dlat = -radius_cells; dlat <= radius_cells; ++dlat) {
    for (int dlon = -radius_cells; dlon <= radius_cells; ++dlon) {
      PlugInSegmentSafetySource source = PI_SEGMENT_SAFETY_SOURCE_NONE;
      SegmentSafetyPointClass point_class = SEGMENT_SAFETY_POINT_NO_DATA;
      uint16_t hazard_flags = SEGMENT_SAFETY_HAZARD_NO_CHART;
      if (!SegmentSafetyGridCellAt(lat_cell + dlat, lon_cell + dlon, &source,
                                   stats, result, &point_class,
                                   &hazard_flags)) {
        *all_water = false;
        if (check_depth) *unknown_depth_hit = true;
        continue;
      }

      *any_chart_data = true;
      if (*first_source == PI_SEGMENT_SAFETY_SOURCE_NONE)
        *first_source = source;
      if (hazard_flags != SEGMENT_SAFETY_HAZARD_NONE) *all_water = false;
      if (SegmentSafetyResultHas(
              result,
              offsetof(PlugInSegmentSafetyResult, depth_source_attribute),
              sizeof(result->depth_source_attribute))) {
        result->required_depth_m = minimum_depth_m;
      }
      if (hazard_flags & SEGMENT_SAFETY_HAZARD_LAND) {
        *land_hit = true;
        if (SegmentSafetyResultHas(
                result, offsetof(PlugInSegmentSafetyResult, hit_object),
                sizeof(result->hit_object))) {
          result->hit_sample_lat =
              (lat_cell + dlat) * kSegmentSafetyGridResolutionDegrees;
          result->hit_sample_lon =
              (lon_cell + dlon) * kSegmentSafetyGridResolutionDegrees;
          result->hit_sample_index = sample_index;
          result->hit_sample_count = sample_count;
        }
        return;
      }
      if (hazard_flags & SEGMENT_SAFETY_HAZARD_NO_CHART) {
        if (check_depth) *unknown_depth_hit = true;
        continue;
      }
      if (!check_depth) continue;
      if (point_class == SEGMENT_SAFETY_POINT_WATER &&
          SegmentSafetyResultHas(
              result,
              offsetof(PlugInSegmentSafetyResult, depth_source_attribute),
              sizeof(result->depth_source_attribute)) &&
          !result->has_depth) {
        *unknown_depth_hit = true;
        result->hit_sample_lat =
            (lat_cell + dlat) * kSegmentSafetyGridResolutionDegrees;
        result->hit_sample_lon =
            (lon_cell + dlon) * kSegmentSafetyGridResolutionDegrees;
        result->hit_sample_index = sample_index;
        result->hit_sample_count = sample_count;
        result->required_depth_m = minimum_depth_m;
        strncpy(result->hit_object, "grid UNKNOWN_DEPTH cell",
                sizeof(result->hit_object) - 1);
        result->hit_object[sizeof(result->hit_object) - 1] = '\0';
        return;
      }
      if (hazard_flags & SEGMENT_SAFETY_HAZARD_DRYING) {
        *drying_hit = true;
        if (SegmentSafetyResultHas(
                result, offsetof(PlugInSegmentSafetyResult, hit_object),
                sizeof(result->hit_object))) {
          result->hit_sample_lat =
              (lat_cell + dlat) * kSegmentSafetyGridResolutionDegrees;
          result->hit_sample_lon =
              (lon_cell + dlon) * kSegmentSafetyGridResolutionDegrees;
          result->hit_sample_index = sample_index;
          result->hit_sample_count = sample_count;
          strncpy(result->hit_object, "grid DRYING cell",
                  sizeof(result->hit_object) - 1);
          result->hit_object[sizeof(result->hit_object) - 1] = '\0';
        }
        return;
      }
      if (SegmentSafetyResultHas(
              result,
              offsetof(PlugInSegmentSafetyResult, depth_source_attribute),
              sizeof(result->depth_source_attribute)) &&
          result->has_depth && result->min_depth_m < minimum_depth_m) {
        *shallow_hit = true;
        result->hit_sample_lat =
            (lat_cell + dlat) * kSegmentSafetyGridResolutionDegrees;
        result->hit_sample_lon =
            (lon_cell + dlon) * kSegmentSafetyGridResolutionDegrees;
        result->hit_sample_index = sample_index;
        result->hit_sample_count = sample_count;
        result->hit_depth_m = result->min_depth_m;
        strncpy(result->hit_object, "grid TOO_SHALLOW cell",
                sizeof(result->hit_object) - 1);
        result->hit_object[sizeof(result->hit_object) - 1] = '\0';
        return;
      }
    }
  }
}

bool SegmentSafetyGridTraversalCheck(double lat1, double lon1, double lat2,
                                     double lon2, double safety_margin_nm,
                                     bool check_depth, double minimum_depth_m,
                                     PlugInSegmentSafetyResult* result,
                                     bool* chart_data_available,
                                     SegmentSafetyCoreStats* stats,
                                     bool* open_water_shortcut) {
  long y0 = lround(lat1 / kSegmentSafetyGridResolutionDegrees);
  long x0 = lround(lon1 / kSegmentSafetyGridResolutionDegrees);
  long y1 = lround(lat2 / kSegmentSafetyGridResolutionDegrees);
  long x1 = lround(lon2 / kSegmentSafetyGridResolutionDegrees);

  long dx = labs(x1 - x0);
  long dy = labs(y1 - y0);
  long sx = x0 < x1 ? 1 : -1;
  long sy = y0 < y1 ? 1 : -1;
  long err = dx - dy;
  long steps = wxMax(dx, dy) + 1;
  if (steps <= 0) steps = 1;

  double mid_lat = (lat1 + lat2) / 2.0;
  double cell_nm =
      wxMin(kSegmentSafetyGridResolutionDegrees * 60.0,
            kSegmentSafetyGridResolutionDegrees * 60.0 *
                wxMax(0.1, fabs(cos(SegmentSafetyDegToRad(mid_lat)))));
  int radius_cells = safety_margin_nm > 0.0
                         ? (int)ceil(safety_margin_nm / wxMax(0.01, cell_nm))
                         : 0;
  radius_cells = wxMin(radius_cells, 128);

  if (stats) stats->segment_sample_count += steps;

  bool any_chart_data = false;
  bool all_water = true;
  bool land_hit = false;
  bool drying_hit = false;
  bool shallow_hit = false;
  bool unknown_depth_hit = false;
  PlugInSegmentSafetySource first_source = PI_SEGMENT_SAFETY_SOURCE_NONE;

  long x = x0;
  long y = y0;
  for (long i = 0; i < steps; ++i) {
    SegmentSafetyCheckGridCellNeighborhood(
        y, x, radius_cells, &first_source, &any_chart_data, &all_water,
        &land_hit, &drying_hit, &shallow_hit, &unknown_depth_hit, check_depth,
        minimum_depth_m, stats, result, (int)i, (int)steps);
    if (land_hit || drying_hit || shallow_hit || unknown_depth_hit) {
      if (chart_data_available) *chart_data_available = true;
      PlugInSegmentSafetyStatus status =
          radius_cells > 0 ? PI_SEGMENT_SAFETY_WITHIN_LAND_MARGIN
                           : PI_SEGMENT_SAFETY_CROSSES_LAND;
      const char* message =
          radius_cells > 0 ? "segment grid traversal enters chart land margin"
                           : "segment grid traversal intersects chart land";
      if (drying_hit) {
        status = PI_SEGMENT_SAFETY_DRYING_AREA;
        message = "segment grid traversal intersects chart drying area";
      } else if (shallow_hit) {
        status = PI_SEGMENT_SAFETY_TOO_SHALLOW;
        message =
            "segment grid traversal intersects chart area shallower than "
            "required depth";
      } else if (unknown_depth_hit) {
        status = PI_SEGMENT_SAFETY_UNKNOWN_DEPTH;
        message = "segment grid traversal lacks chart depth data";
      }
      SetSegmentSafetyStatus(result, status);
      SetSegmentSafetySource(result, first_source);
      SetSegmentSafetyMessage(result, message);
      return true;
    }

    if (x == x1 && y == y1) break;
    long e2 = 2 * err;
    if (e2 > -dy) {
      err -= dy;
      x += sx;
    }
    if (e2 < dx) {
      err += dx;
      y += sy;
    }
  }

  if (any_chart_data) {
    if (chart_data_available) *chart_data_available = true;
    if (GetSegmentSafetySource(result) == PI_SEGMENT_SAFETY_SOURCE_NONE)
      SetSegmentSafetySource(result, first_source);
    if (open_water_shortcut) *open_water_shortcut = all_water;
  }

  if (all_water && any_chart_data && stats) ++stats->water_tile_shortcuts;
  return false;
}

wxString SegmentSafetyPointDiagnostic(double lat, double lon) {
  SegmentSafetyCoreStats stats;
  std::set<int> chart_indexes;
  SegmentSafetyCandidateChartsAt(lat, lon, chart_indexes, &stats);

  wxString objects;
  wxString source_name = "none";
  wxString chart_path;
  wxString point_class = "UNKNOWN";
  int chart_db_index = -1;
  int chart_scale = -1;
  int area_count = 0;
  int land_count = 0;
  int drying_count = 0;
  int depare_count = 0;
  bool has_depth = false;
  bool unknown_danger_depth_seen = false;
  double min_depth_m = 0.0;
  wxString depth_attr = "none";
  bool chart_checked = false;
  time_t chart_edition_date = 0;
  time_t chart_file_time = 0;

  std::vector<SegmentSafetyChartCandidate> candidates =
      SegmentSafetySortedChartCandidates(lat, lon, chart_indexes);
  for (std::vector<SegmentSafetyChartCandidate>::const_iterator it =
           candidates.begin();
       it != candidates.end(); ++it) {
    ChartBase* chart =
        ChartData ? ChartData->OpenChartFromDB(it->db_index, FULL_INIT) : NULL;
    s57chart* s57 = dynamic_cast<s57chart*>(chart);
    ChartPlugInWrapper* plugin_wrapper =
        dynamic_cast<ChartPlugInWrapper*>(chart);
    if (it->plugin_vector && plugin_wrapper) {
      chart_checked = true;
      chart_db_index = it->db_index;
      chart_path = chart->GetFullPath();
      chart_edition_date = it->edition_date;
      chart_file_time = it->file_time;
      chart_scale = chart->GetNativeScale();
      source_name = "PLUGIN_VECTOR";
      const ViewPort vp = SegmentSafetyHighestDetailViewPortAt(lat, lon);
      ListOfPI_S57Obj* object_list =
          g_pi_manager ? g_pi_manager->GetPlugInObjRuleListAtLatLon(
                             plugin_wrapper, static_cast<float>(lat),
                             static_cast<float>(lon),
                             static_cast<float>(
                                 kSegmentSafetyGridResolutionDegrees * 0.75),
                             vp)
                       : NULL;
      if (!object_list) {
        chart_checked = false;
        break;
      }
      for (ListOfPI_S57Obj::Node* node = object_list->GetFirst(); node;
           node = node->GetNext()) {
        PI_S57Obj* object = node->GetData();
        if (!object) continue;
        ++area_count;
        if (!objects.empty()) objects += ";";
        objects += SegmentSafetyPluginObjectSummary(object);
        if (!strncmp(object->FeatureName, "LNDARE", 6) ||
            SegmentSafetyPluginObjectIsAlwaysDry(object))
          ++land_count;
        if (SegmentSafetyPluginObjectIsDrying(object)) ++drying_count;
        double object_depth_m = 0.0;
        wxString object_depth_attr;
        bool unknown_danger_depth = false;
        if (SegmentSafetyPluginObjectDepthM(
                object, &object_depth_m, &object_depth_attr,
                &unknown_danger_depth)) {
          ++depare_count;
          if (!has_depth || object_depth_m < min_depth_m) {
            has_depth = true;
            min_depth_m = object_depth_m;
            depth_attr = object_depth_attr;
          }
        }
        if (unknown_danger_depth) {
          unknown_danger_depth_seen = true;
          depth_attr = wxString::Format("%s/VALSOU missing",
                                       object->FeatureName);
        }
      }
      if (unknown_danger_depth_seen) has_depth = false;
      object_list->Clear();
      delete object_list;
      break;
    }
    if (!s57) continue;

    chart_checked = true;
    chart_db_index = it->db_index;
    chart_path = chart->GetFullPath();
    chart_edition_date = it->edition_date;
    chart_file_time = it->file_time;
    bool cm93 = IsCm93Chart(chart);
    source_name = cm93 ? "CM93" : "VECTOR_CHART";
    ViewPort vp = SegmentSafetyHighestDetailViewPortAt(lat, lon);
    if (cm93) {
      cm93compchart* cm93_chart = dynamic_cast<cm93compchart*>(chart);
      if (cm93_chart) cm93_chart->SetVPParms(vp);
    }
    chart_scale = chart->GetNativeScale();

    ListOfObjRazRules* rule_list =
        s57->GetObjRuleListAtLatLon(lat, lon, 0.0, &vp, MASK_AREA);
    if (!rule_list) break;

    for (ListOfObjRazRules::Node* node = rule_list->GetFirst(); node;
         node = node->GetNext()) {
      ObjRazRules* rule = node->GetData();
      if (!rule || !rule->obj) continue;
      ++area_count;
      if (!objects.empty()) objects += ";";
      objects += SegmentSafetyRuleSummary(rule);

      if (!strncmp(rule->obj->FeatureName, "LNDARE", 6)) ++land_count;
      if (SegmentSafetyRuleIsDrying(rule)) ++drying_count;
      double rule_depth = 0.0;
      if (SegmentSafetyRuleDepthMinM(rule, &rule_depth)) {
        ++depare_count;
        if (!has_depth || rule_depth < min_depth_m) {
          has_depth = true;
          min_depth_m = rule_depth;
          depth_attr = rule->obj->GetAttrValueAsString("DRVAL1");
        }
      } else if (!strncmp(rule->obj->FeatureName, "DEPARE", 6)) {
        ++depare_count;
      }
    }

    rule_list->Clear();
    delete rule_list;
    break;
  }

  if (!chart_checked)
    point_class = "NO_DATA";
  else if (land_count > 0)
    point_class = "LAND";
  else if (drying_count > 0)
    point_class = "DRYING";
  else
    point_class = "WATER_OR_NO_UNSAFE_AREA";

  if (objects.empty()) objects = "none";
  chart_path.Replace("\"", "'");
  return wxString::Format(
             "class=%s source=%s chart_db_index=%d chart_scale=%d "
             "chart_path=\"%s\" "
             "chart_edition=%ld chart_file_time=%ld "
             "selection=highest-detail-newest "
             "chart_stack_entries=%d candidate_charts=%zu area_objects=%d "
             "land_objects=%d drying_objects=%d depare_objects=%d "
             "objects=\"%s\"",
             point_class, source_name, chart_db_index, chart_scale, chart_path,
             (long)chart_edition_date, (long)chart_file_time,
             stats.chart_stack_entries, candidates.size(), area_count,
             land_count, drying_count, depare_count, objects) +
         wxString::Format(" has_depth=%d min_depth_m=%.2f depth_attr=\"%s\"",
                          has_depth ? 1 : 0, has_depth ? min_depth_m : 0.0,
                          depth_attr);
}

CachedChartLandGeometry& SegmentSafetyLoadChartLandGeometry(
    int db_index, double lat, double lon, SegmentSafetyCoreStats* stats) {
  if (!wxThread::IsMain()) {
    if (stats) stats->chart_load_failed = true;
    static CachedChartLandGeometry worker_no_chart_data;
    wxLogMessage(
        "WR_GRID_THREAD_VIOLATION SegmentSafetyLoadChartLandGeometry called "
        "from worker thread=%p main_thread=%p db_index=%d. Chart geometry "
        "loading is main-thread only.",
        wxThread::GetCurrentId(), wxThread::GetMainId(), db_index);
    return worker_no_chart_data;
  }

  if (!ChartData) {
    if (stats) stats->no_chart_database = true;
    static CachedChartLandGeometry no_chart_data;
    return no_chart_data;
  }
  wxStopWatch cache_timer;
  ChartBase* chart = ChartData->OpenChartFromDB(db_index, FULL_INIT);
  bool cm93 = IsCm93Chart(chart);
  std::string cache_key = SegmentSafetyCacheKey(db_index, cm93, lat, lon);
  CachedChartLandGeometry& cached = s_segment_safety_land_cache[cache_key];
  if (cached.loaded) return cached;
  cached.loaded = true;
  cached.cache_key = cache_key;
  if (chart) cached.chart_path = chart->GetFullPath();

  s57chart* s57 = dynamic_cast<s57chart*>(chart);
  if (!s57) {
    if (stats) {
      stats->chart_load_failed = chart == NULL;
      stats->cache_build_ms += cache_timer.Time();
    }
    return cached;
  }

  cached.source = cm93 ? PI_SEGMENT_SAFETY_SOURCE_CM93
                       : PI_SEGMENT_SAFETY_SOURCE_VECTOR_CHART;
  if (stats) ++stats->s57_chart_count;

  if (cm93) {
    cm93compchart* cm93_chart = dynamic_cast<cm93compchart*>(chart);
    if (cm93_chart) cm93_chart->SetVPParms(SegmentSafetyViewPortAt(lat, lon));
  }

  std::vector<std::vector<wxPoint2DDouble> > rings;
  s57->CollectFeatureAreaRings("LNDARE", rings);
  for (size_t i = 0; i < rings.size(); ++i) {
    if (rings[i].size() < 3) continue;
    CachedLandRing ring;
    ring.points.swap(rings[i]);
    ring.bbox = SegmentSafetyRingBBox(ring.points);
    if (ring.bbox.max_lat < lat - 10.0 || ring.bbox.min_lat > lat + 10.0 ||
        ring.bbox.max_lon < lon - 10.0 || ring.bbox.min_lon > lon + 10.0)
      continue;
    cached.rings.push_back(ring);
  }

  wxLogMessage(
      "OpenCPN segment safety: cached chart land geometry "
      "db_index=%d key=%s source=%d type=%d scale=%d land_rings=%zu",
      db_index, cache_key.c_str(), (int)cached.source,
      chart ? (int)chart->GetChartType() : -1,
      chart ? chart->GetNativeScale() : -1, cached.rings.size());
  for (size_t i = 0; i < cached.rings.size() && i < 8; ++i) {
    const CachedLandRing& ring = cached.rings[i];
    wxLogMessage(
        "OpenCPN segment safety: land ring sample db_index=%d key=%s "
        "ring=%zu bbox=[lat %.8f..%.8f lon %.8f..%.8f] points=%zu",
        db_index, cache_key.c_str(), i, ring.bbox.min_lat, ring.bbox.max_lat,
        ring.bbox.min_lon, ring.bbox.max_lon, ring.points.size());
  }
  if (cached.rings.empty()) {
    wxString chart_path = chart ? chart->GetFullPath() : wxString();
    wxLogMessage(
        "OpenCPN segment safety: no LNDARE rings db_index=%d key=%s "
        "path=%s summary=%s",
        db_index, cache_key.c_str(), chart_path.c_str(),
        s57->GetFeatureDebugSummary());
  }
  if (stats) {
    stats->cache_build_ms += cache_timer.Time();
    if (cached.rings.empty()) stats->zero_land_geometry = true;
  }

  return cached;
}

void LogSegmentSafetyChartHit(const char* cause, int db_index,
                              const std::string& cache_key,
                              PlugInSegmentSafetySource source,
                              const CachedLandRing& ring, double lat1,
                              double lon1, double lat2, double lon2,
                              double safety_margin_nm, size_t edge_index) {
  if (s_segment_safety_chart_hit_logs >= kMaxSegmentSafetyChartHitLogs) return;
  ++s_segment_safety_chart_hit_logs;
  wxLogMessage(
      "OpenCPN segment safety: LNDARE hit #%ld cause=%s db_index=%d key=%s "
      "source=%d segment=(%.8f,%.8f)->(%.8f,%.8f) margin_nm=%.3f "
      "ring_bbox=[lat %.8f..%.8f lon %.8f..%.8f] ring_points=%zu "
      "edge_index=%zu",
      s_segment_safety_chart_hit_logs, cause, db_index, cache_key.c_str(),
      (int)source, lat1, lon1, lat2, lon2, safety_margin_nm, ring.bbox.min_lat,
      ring.bbox.max_lat, ring.bbox.min_lon, ring.bbox.max_lon,
      ring.points.size(), edge_index);
}

void SetSegmentSafetyChartHitDetails(PlugInSegmentSafetyResult* result,
                                     PlugInSegmentSafetyHitCause cause,
                                     int db_index,
                                     const CachedChartLandGeometry& chart_cache,
                                     const CachedLandRing& ring,
                                     size_t edge_index) {
  if (!SegmentSafetyResultHas(result,
                              offsetof(PlugInSegmentSafetyResult, chart_path),
                              sizeof(result->chart_path)))
    return;

  result->chart_db_index = db_index;
  result->hit_cause = cause;
  result->hit_ring_min_lat = ring.bbox.min_lat;
  result->hit_ring_max_lat = ring.bbox.max_lat;
  result->hit_ring_min_lon = ring.bbox.min_lon;
  result->hit_ring_max_lon = ring.bbox.max_lon;
  result->hit_ring_point_count = (int)ring.points.size();
  result->hit_edge_index = (int)edge_index;
  strncpy(result->chart_path, chart_cache.chart_path.mb_str(),
          sizeof(result->chart_path) - 1);
  result->chart_path[sizeof(result->chart_path) - 1] = '\0';
}

bool CachedChartSegmentSafetyCheck(double lat1, double lon1, double lat2,
                                   double lon2, double safety_margin_nm,
                                   PlugInSegmentSafetyResult* result,
                                   bool* chart_data_available,
                                   SegmentSafetyCoreStats* stats) {
  if (!wxThread::IsMain()) {
    if (stats) stats->chart_load_failed = true;
    if (chart_data_available) *chart_data_available = false;
    SetSegmentSafetyDiagnosticReason(result,
                                     PI_SEGMENT_SAFETY_DIAG_NO_CANDIDATE_CHART);
    SetSegmentSafetyMessage(
        result,
        "legacy chart geometry check is unavailable from worker thread");
    return false;
  }

  wxStopWatch select_timer;
  std::set<int> chart_indexes;
  SegmentSafetyCandidateChartsAt(lat1, lon1, chart_indexes, stats);
  SegmentSafetyCandidateChartsAt(lat2, lon2, chart_indexes, stats);
  SegmentSafetyCandidateChartsAt((lat1 + lat2) / 2.0, (lon1 + lon2) / 2.0,
                                 chart_indexes, stats);
  if (stats) {
    stats->candidate_chart_count = chart_indexes.size();
    stats->chart_select_ms += select_timer.Time();
  }
  if (chart_indexes.empty()) return false;

  wxStopWatch geometry_timer;
  wxPoint2DDouble start(lon1, lat1);
  wxPoint2DDouble end(lon2, lat2);
  SegmentSafetyBBox segment_box =
      SegmentSafetySegmentBBox(lat1, lon1, lat2, lon2, safety_margin_nm);

  for (std::set<int>::const_iterator it = chart_indexes.begin();
       it != chart_indexes.end(); ++it) {
    CachedChartLandGeometry& chart_cache = SegmentSafetyLoadChartLandGeometry(
        *it, (lat1 + lat2) / 2.0, (lon1 + lon2) / 2.0, stats);
    if (chart_cache.source == PI_SEGMENT_SAFETY_SOURCE_NONE) continue;
    if (chart_cache.rings.empty()) {
      if (stats) stats->zero_land_geometry = true;
      continue;
    }
    if (chart_data_available) *chart_data_available = true;
    if (GetSegmentSafetySource(result) == PI_SEGMENT_SAFETY_SOURCE_NONE)
      SetSegmentSafetySource(result, chart_cache.source);
    if (stats) stats->land_ring_count += chart_cache.rings.size();

    for (size_t i = 0; i < chart_cache.rings.size(); ++i) {
      const CachedLandRing& ring = chart_cache.rings[i];
      if (!SegmentSafetyBBoxIntersects(segment_box, ring.bbox)) continue;
      if (stats) ++stats->bbox_ring_tests;

      if (SegmentSafetyPointInRing(lat1, lon1, ring.points) ||
          SegmentSafetyPointInRing(lat2, lon2, ring.points)) {
        if (stats) stats->geometry_check_ms += geometry_timer.Time();
        LogSegmentSafetyChartHit("endpoint-inside-LNDARE", *it,
                                 chart_cache.cache_key, chart_cache.source,
                                 ring, lat1, lon1, lat2, lon2, safety_margin_nm,
                                 0);
        SetSegmentSafetyChartHitDetails(
            result, PI_SEGMENT_SAFETY_HIT_ENDPOINT_IN_LANDARE, *it, chart_cache,
            ring, 0);
        SetSegmentSafetyStatus(result, PI_SEGMENT_SAFETY_CROSSES_LAND);
        SetSegmentSafetySource(result, chart_cache.source);
        SetSegmentSafetyMessage(result,
                                "segment endpoint is inside chart land area");
        return true;
      }

      for (size_t j = 0; j < ring.points.size(); ++j) {
        const wxPoint2DDouble& a = ring.points[j];
        const wxPoint2DDouble& b = ring.points[(j + 1) % ring.points.size()];
        if (stats) ++stats->edge_tests;
        if (SegmentSafetySegmentsIntersect(start, end, a, b)) {
          if (stats) stats->geometry_check_ms += geometry_timer.Time();
          LogSegmentSafetyChartHit("segment-intersects-LNDARE-edge", *it,
                                   chart_cache.cache_key, chart_cache.source,
                                   ring, lat1, lon1, lat2, lon2,
                                   safety_margin_nm, j);
          SetSegmentSafetyChartHitDetails(
              result, PI_SEGMENT_SAFETY_HIT_SEGMENT_INTERSECTS_LANDARE_EDGE,
              *it, chart_cache, ring, j);
          SetSegmentSafetyStatus(result, PI_SEGMENT_SAFETY_CROSSES_LAND);
          SetSegmentSafetySource(result, chart_cache.source);
          SetSegmentSafetyMessage(result,
                                  "segment intersects chart land boundary");
          return true;
        }

        if (safety_margin_nm > 0.0 &&
            SegmentSafetySegmentDistanceNm(start, end, a, b) <=
                safety_margin_nm) {
          if (stats) stats->geometry_check_ms += geometry_timer.Time();
          LogSegmentSafetyChartHit("approx-margin-to-LNDARE-edge", *it,
                                   chart_cache.cache_key, chart_cache.source,
                                   ring, lat1, lon1, lat2, lon2,
                                   safety_margin_nm, j);
          SetSegmentSafetyChartHitDetails(
              result, PI_SEGMENT_SAFETY_HIT_MARGIN_TO_LANDARE_EDGE, *it,
              chart_cache, ring, j);
          SetSegmentSafetyStatus(result, PI_SEGMENT_SAFETY_WITHIN_LAND_MARGIN);
          SetSegmentSafetySource(result, chart_cache.source);
          SetSegmentSafetyMessage(
              result, "segment is within approximate chart land safety margin");
          return true;
        }
      }
    }
  }

  if (stats) stats->geometry_check_ms += geometry_timer.Time();
  return false;
}

bool ChartSegmentPointClassificationCheck(
    double lat1, double lon1, double lat2, double lon2, double safety_margin_nm,
    bool check_depth, double minimum_depth_m, bool force_authoritative_fine,
    PlugInSegmentSafetyResult* result, bool* chart_data_available,
    SegmentSafetyCoreStats* stats, bool* open_water_shortcut = NULL) {
  wxStopWatch geometry_timer;
  double bearing = 0.0;
  double dist_nm = 0.0;
  ll_gc_ll_reverse(lat1, lon1, lat2, lon2, &bearing, &dist_nm);
  if (open_water_shortcut) *open_water_shortcut = false;

  const int max_samples = 512;
  int samples = wxMax(2, wxMin(max_samples, (int)ceil(dist_nm / 0.05) + 1));
  bool any_chart_data = false;
  PlugInSegmentSafetySource first_source = PI_SEGMENT_SAFETY_SOURCE_NONE;

  bool route_mask_answered = false;
  if (SegmentSafetyRouteMaskTraversalCheck(
          lat1, lon1, lat2, lon2, safety_margin_nm, check_depth,
          minimum_depth_m, force_authoritative_fine, result,
          chart_data_available, stats, open_water_shortcut,
          &route_mask_answered)) {
    if (stats) stats->geometry_check_ms += geometry_timer.Time();
    return true;
  }
  if (route_mask_answered) {
    if (stats) stats->geometry_check_ms += geometry_timer.Time();
    return false;
  }

  if (!wxThread::IsMain()) {
    if (stats) stats->geometry_check_ms += geometry_timer.Time();
    if (chart_data_available) *chart_data_available = false;
    return false;
  }

  wxStopWatch grid_lookup_timer;
  if (!check_depth && SegmentSafetyAllTouchedTilesAreWater(
                          lat1, lon1, lat2, lon2, safety_margin_nm, bearing,
                          dist_nm, samples, stats)) {
    if (stats) {
      stats->grid_lookup_ms += grid_lookup_timer.Time();
      stats->geometry_check_ms += geometry_timer.Time();
    }
    if (chart_data_available) *chart_data_available = true;
    if (GetSegmentSafetySource(result) == PI_SEGMENT_SAFETY_SOURCE_NONE)
      SetSegmentSafetySource(result, PI_SEGMENT_SAFETY_SOURCE_VECTOR_CHART);
    if (open_water_shortcut) *open_water_shortcut = true;
    return false;
  }

  if (SegmentSafetyGridTraversalCheck(lat1, lon1, lat2, lon2, safety_margin_nm,
                                      check_depth, minimum_depth_m, result,
                                      chart_data_available, stats,
                                      open_water_shortcut)) {
    if (stats) stats->geometry_check_ms += geometry_timer.Time();
    return true;
  }
  if (chart_data_available && *chart_data_available) {
    if (stats) stats->geometry_check_ms += geometry_timer.Time();
    return false;
  }

  /*
   * From this point down the legacy fallback path uses point classification
   * helpers which may need to build grid tiles.  Tile building requires
   * OpenCPN chart object/rule access, so worker threads must stop here and let
   * the caller's missing-tile retry/prewarm path service the request on the
   * main thread.
   */
  if (!wxThread::IsMain()) {
    if (stats) stats->geometry_check_ms += geometry_timer.Time();
    if (chart_data_available) *chart_data_available = false;
    return false;
  }

  if (!check_depth &&
      SegmentSafetyCoarseSampledCellsAreWater(
          lat1, lon1, lat2, lon2, safety_margin_nm, bearing, dist_nm, stats)) {
    if (stats) {
      stats->grid_lookup_ms += grid_lookup_timer.Time();
      stats->geometry_check_ms += geometry_timer.Time();
    }
    if (chart_data_available) *chart_data_available = true;
    if (GetSegmentSafetySource(result) == PI_SEGMENT_SAFETY_SOURCE_NONE)
      SetSegmentSafetySource(result, PI_SEGMENT_SAFETY_SOURCE_VECTOR_CHART);
    if (open_water_shortcut) *open_water_shortcut = true;
    return false;
  }

  if (stats) stats->segment_sample_count += samples;
  for (int i = 0; i < samples; ++i) {
    double sample_dist = samples == 1 ? 0.0 : dist_nm * i / (samples - 1);
    double lat = lat1;
    double lon = lon1;
    if (sample_dist > 0.0)
      ll_gc_ll(lat1, lon1, bearing, sample_dist, &lat, &lon);

    PlugInSegmentSafetySource source = PI_SEGMENT_SAFETY_SOURCE_NONE;
    wxStopWatch lookup_timer;
    SegmentSafetyPointClass point_class =
        ChartPointSafetyClassAt(lat, lon, &source, stats, result);
    if (stats) stats->grid_lookup_ms += lookup_timer.Time();
    if (point_class == SEGMENT_SAFETY_POINT_NO_DATA) continue;

    any_chart_data = true;
    if (first_source == PI_SEGMENT_SAFETY_SOURCE_NONE) first_source = source;
    if (GetSegmentSafetySource(result) == PI_SEGMENT_SAFETY_SOURCE_NONE)
      SetSegmentSafetySource(result, source);

    if (point_class == SEGMENT_SAFETY_POINT_LAND) {
      if (stats) stats->geometry_check_ms += geometry_timer.Time();
      if (chart_data_available) *chart_data_available = true;
      if (SegmentSafetyResultHas(
              result, offsetof(PlugInSegmentSafetyResult, hit_object),
              sizeof(result->hit_object))) {
        result->hit_sample_lat = lat;
        result->hit_sample_lon = lon;
        result->hit_sample_index = i;
        result->hit_sample_count = samples;
      }
      SetSegmentSafetyStatus(result, PI_SEGMENT_SAFETY_CROSSES_LAND);
      SetSegmentSafetySource(result, source);
      SetSegmentSafetyMessage(result,
                              "segment samples intersect chart land area");
      if (s_segment_safety_chart_hit_logs < kMaxSegmentSafetyChartHitLogs) {
        ++s_segment_safety_chart_hit_logs;
        wxLogMessage(
            "FIRST_LAND_HIT source=chart-point segment=(%.8f,%.8f)->"
            "(%.8f,%.8f) sample=(%.8f,%.8f) sample_index=%d/%d "
            "object=\"%s\" chart_db_index=%d chart_scale=%d "
            "chart_path=\"%s\"",
            lat1, lon1, lat2, lon2, lat, lon, i + 1, samples,
            result ? result->hit_object : "",
            result ? result->chart_db_index : -1,
            result ? result->chart_scale : -1,
            result ? result->chart_path : "");
      }
      return true;
    }

    if (check_depth && point_class == SEGMENT_SAFETY_POINT_WATER &&
        SegmentSafetyResultHas(
            result, offsetof(PlugInSegmentSafetyResult, depth_source_attribute),
            sizeof(result->depth_source_attribute)) &&
        !result->has_depth) {
      if (stats) stats->geometry_check_ms += geometry_timer.Time();
      if (chart_data_available) *chart_data_available = true;
      result->hit_sample_lat = lat;
      result->hit_sample_lon = lon;
      result->hit_sample_index = i;
      result->hit_sample_count = samples;
      result->required_depth_m = minimum_depth_m;
      strncpy(result->hit_object, "chart UNKNOWN_DEPTH area",
              sizeof(result->hit_object) - 1);
      result->hit_object[sizeof(result->hit_object) - 1] = '\0';
      SetSegmentSafetyStatus(result, PI_SEGMENT_SAFETY_UNKNOWN_DEPTH);
      SetSegmentSafetySource(result, source);
      SetSegmentSafetyMessage(result, "segment samples lack chart depth data");
      return true;
    }

    if (check_depth && point_class == SEGMENT_SAFETY_POINT_DRYING) {
      if (stats) stats->geometry_check_ms += geometry_timer.Time();
      if (chart_data_available) *chart_data_available = true;
      if (SegmentSafetyResultHas(
              result, offsetof(PlugInSegmentSafetyResult, hit_object),
              sizeof(result->hit_object))) {
        result->hit_sample_lat = lat;
        result->hit_sample_lon = lon;
        result->hit_sample_index = i;
        result->hit_sample_count = samples;
        result->required_depth_m = minimum_depth_m;
        result->has_drying = 1;
        strncpy(result->hit_object, "chart DRYING area",
                sizeof(result->hit_object) - 1);
        result->hit_object[sizeof(result->hit_object) - 1] = '\0';
      }
      SetSegmentSafetyStatus(result, PI_SEGMENT_SAFETY_DRYING_AREA);
      SetSegmentSafetySource(result, source);
      SetSegmentSafetyMessage(result,
                              "segment samples intersect chart drying area");
      return true;
    }

    if (check_depth &&
        SegmentSafetyResultHas(
            result, offsetof(PlugInSegmentSafetyResult, depth_source_attribute),
            sizeof(result->depth_source_attribute)) &&
        result->has_depth && result->min_depth_m < minimum_depth_m) {
      if (stats) stats->geometry_check_ms += geometry_timer.Time();
      if (chart_data_available) *chart_data_available = true;
      result->hit_sample_lat = lat;
      result->hit_sample_lon = lon;
      result->hit_sample_index = i;
      result->hit_sample_count = samples;
      result->required_depth_m = minimum_depth_m;
      result->hit_depth_m = result->min_depth_m;
      strncpy(result->hit_object, "chart TOO_SHALLOW area",
              sizeof(result->hit_object) - 1);
      result->hit_object[sizeof(result->hit_object) - 1] = '\0';
      SetSegmentSafetyStatus(result, PI_SEGMENT_SAFETY_TOO_SHALLOW);
      SetSegmentSafetySource(result, source);
      SetSegmentSafetyMessage(
          result,
          "segment samples intersect chart area shallower than required depth");
      return true;
    }

    if (safety_margin_nm > 0.0) {
      double left_lat, left_lon, right_lat, right_lon;
      ll_gc_ll(lat, lon, SegmentSafetyNormalizeBearing(bearing - 90.0),
               safety_margin_nm, &left_lat, &left_lon);
      ll_gc_ll(lat, lon, SegmentSafetyNormalizeBearing(bearing + 90.0),
               safety_margin_nm, &right_lat, &right_lon);
      PlugInSegmentSafetySource left_source = PI_SEGMENT_SAFETY_SOURCE_NONE;
      PlugInSegmentSafetySource right_source = PI_SEGMENT_SAFETY_SOURCE_NONE;
      wxStopWatch margin_lookup_timer;
      SegmentSafetyPointClass left_class =
          ChartPointSafetyClassAt(left_lat, left_lon, &left_source, stats);
      SegmentSafetyPointClass right_class =
          ChartPointSafetyClassAt(right_lat, right_lon, &right_source, stats);
      if (stats) stats->grid_lookup_ms += margin_lookup_timer.Time();
      if (left_class != SEGMENT_SAFETY_POINT_NO_DATA ||
          right_class != SEGMENT_SAFETY_POINT_NO_DATA)
        any_chart_data = true;
      if (left_class == SEGMENT_SAFETY_POINT_LAND ||
          right_class == SEGMENT_SAFETY_POINT_LAND) {
        if (stats) stats->geometry_check_ms += geometry_timer.Time();
        if (chart_data_available) *chart_data_available = true;
        SetSegmentSafetyStatus(result, PI_SEGMENT_SAFETY_WITHIN_LAND_MARGIN);
        SetSegmentSafetySource(result, left_class == SEGMENT_SAFETY_POINT_LAND
                                           ? left_source
                                           : right_source);
        SetSegmentSafetyMessage(
            result, "segment samples are within approximate chart land margin");
        return true;
      }
    }
  }

  if (any_chart_data) {
    if (chart_data_available) *chart_data_available = true;
    if (GetSegmentSafetySource(result) == PI_SEGMENT_SAFETY_SOURCE_NONE)
      SetSegmentSafetySource(result, first_source);
  }
  if (stats) stats->geometry_check_ms += geometry_timer.Time();
  return false;
}

bool GshhsSegmentSafetyHitsLand(double lat1, double lon1, double lat2,
                                double lon2, double safety_margin_nm,
                                PlugInSegmentSafetyStatus* status) {
  if (PlugIn_GSHHS_CrossesLand(lat1, lon1, lat2, lon2)) {
    if (status) *status = PI_SEGMENT_SAFETY_CROSSES_LAND;
    return true;
  }

  if (safety_margin_nm <= 0.0) return false;

  double bearing = 0.0;
  double dist_nm = 0.0;
  ll_gc_ll_reverse(lat1, lon1, lat2, lon2, &bearing, &dist_nm);

  double lat_up1, lon_up1, lat_up2, lon_up2;
  double lat_down1, lon_down1, lat_down2, lon_down2;
  ll_gc_ll(lat1, lon1, SegmentSafetyNormalizeBearing(bearing - 90.0),
           safety_margin_nm, &lat_up1, &lon_up1);
  ll_gc_ll(lat2, lon2, SegmentSafetyNormalizeBearing(bearing - 90.0),
           safety_margin_nm, &lat_up2, &lon_up2);
  ll_gc_ll(lat1, lon1, SegmentSafetyNormalizeBearing(bearing + 90.0),
           safety_margin_nm, &lat_down1, &lon_down1);
  ll_gc_ll(lat2, lon2, SegmentSafetyNormalizeBearing(bearing + 90.0),
           safety_margin_nm, &lat_down2, &lon_down2);

  if (PlugIn_GSHHS_CrossesLand(lat_up1, lon_up1, lat_up2, lon_up2) ||
      PlugIn_GSHHS_CrossesLand(lat_down1, lon_down1, lat_down2, lon_down2) ||
      PlugIn_GSHHS_CrossesLand(lat_up1, lon_up1, lat_down2, lon_down2) ||
      PlugIn_GSHHS_CrossesLand(lat_down1, lon_down1, lat_up2, lon_up2)) {
    if (status) *status = PI_SEGMENT_SAFETY_WITHIN_LAND_MARGIN;
    return true;
  }

  return false;
}

}  // namespace

wxString PlugIn_SegmentSafetyPointDiagnostic(double lat, double lon) {
  return SegmentSafetyPointDiagnostic(lat, lon);
}

int PlugIn_GetPendingSegmentSafetyRequestCount() {
  wxMutexLocker lock(s_segment_safety_cache_mutex);
  return static_cast<int>(s_segment_safety_pending_route_mask_requests.size() +
                          s_segment_safety_inflight_route_mask_requests.size());
}

bool PlugIn_ServicePendingSegmentSafetyRequests(
    int max_requests, int max_milliseconds,
    PlugInSegmentSafetyRequestServiceResult* result) {
  int result_size = result ? result->struct_size : 0;
  if (result && result_size >= static_cast<int>(sizeof(int))) {
    memset(result, 0,
           wxMin(result_size, static_cast<int>(sizeof(
                                  PlugInSegmentSafetyRequestServiceResult))));
    result->struct_size = result_size;
  }
  if (!wxThread::IsMain()) return false;

  wxStopWatch timer;
  int pending_before = PlugIn_GetPendingSegmentSafetyRequestCount();
  int serviced = 0;
  int built_count = 0;
  int prefetched_count = 0;
  int failed = 0;

  while ((max_requests <= 0 || serviced < max_requests) &&
         (max_milliseconds <= 0 || serviced == 0 ||
          timer.Time() < max_milliseconds)) {
    SegmentSafetyRouteMaskRequest request;
    bool have_request = false;
    {
      wxMutexLocker lock(s_segment_safety_cache_mutex);
      if (!s_segment_safety_pending_route_mask_requests.empty()) {
        std::map<std::string, SegmentSafetyRouteMaskRequest>::iterator it =
            s_segment_safety_pending_route_mask_requests.begin();
        request = it->second;
        s_segment_safety_pending_route_mask_requests.erase(it);
        s_segment_safety_inflight_route_mask_requests.insert(request.key);
        have_request = true;
      }
    }
    if (!have_request) break;

    bool built = false;
    bool ok = request.group_index == SegmentSafetyCurrentGroupIndex() &&
              EnsureSegmentSafetyRouteMaskTile(
                  request.lat_tile, request.lon_tile, request.safety_margin_nm,
                  request.check_depth, request.minimum_depth_m, NULL, &built,
                  request.force_authoritative_fine);
    // A worker stops at the first absent mask and can otherwise incur one GUI
    // timer wake-up per 0.05-degree tile.  Populate the immediately adjacent
    // exact masks while the chart is already active on the main thread.  This
    // changes only scheduling/cache locality: every neighbour is built by the
    // same authoritative code as an on-demand request.  Do not fan out from
    // an all-blocked centre tile: propagation cannot continue through it, and
    // prefetching eight more inland tiles was pure work.  Keep independent
    // force-fine validation demand-only to avoid speculative validation work.
    CachedSegmentSafetyRouteMaskTile centre_mask;
    const bool useful_to_prefetch_neighbours =
        ok && LookupSegmentSafetyRouteMaskTile(request.key, &centre_mask) &&
        centre_mask.clear_count > 0;
    if (useful_to_prefetch_neighbours && !request.force_authoritative_fine) {
      bool budget_exhausted = false;
      for (int dlat = -1; dlat <= 1; ++dlat) {
        for (int dlon = -1; dlon <= 1; ++dlon) {
          if (dlat == 0 && dlon == 0) continue;
          if (!ocpn::chart_safety::MayPrefetchNeighbour(timer.Time(),
                                                        max_milliseconds)) {
            budget_exhausted = true;
            break;
          }
          const long neighbour_lat_tile = request.lat_tile + dlat;
          const long neighbour_lon_tile = request.lon_tile + dlon;
          const double neighbour_min_lat =
              neighbour_lat_tile * kSegmentSafetyGridTileDegrees;
          const double neighbour_min_lon =
              neighbour_lon_tile * kSegmentSafetyGridTileDegrees;
          if (neighbour_min_lat < -90.0 || neighbour_min_lat >= 90.0 ||
              neighbour_min_lon < -180.0 || neighbour_min_lon >= 180.0)
            continue;
          bool neighbour_built = false;
          if (EnsureSegmentSafetyRouteMaskTile(
                  neighbour_lat_tile, neighbour_lon_tile,
                  request.safety_margin_nm, request.check_depth,
                  request.minimum_depth_m, NULL, &neighbour_built, false) &&
              neighbour_built) {
            ++built_count;
            ++prefetched_count;
          }
        }
        if (budget_exhausted) break;
      }
    }
    {
      wxMutexLocker lock(s_segment_safety_cache_mutex);
      s_segment_safety_inflight_route_mask_requests.erase(request.key);
      if (ok) s_segment_safety_pending_route_mask_requests.erase(request.key);
    }
    ++serviced;
    if (ok && built)
      ++built_count;
    else if (!ok)
      ++failed;
  }

  int pending_after = PlugIn_GetPendingSegmentSafetyRequestCount();
  if (serviced > 0 && s_segment_safety_persistent_tiles_since_checkpoint >=
                          kSegmentSafetyPersistentCheckpointTiles)
    SegmentSafetyPersistentCacheSave();
  if (result && result_size >= static_cast<int>(sizeof(
                                   PlugInSegmentSafetyRequestServiceResult))) {
    result->pending_before = pending_before;
    result->requests_serviced = serviced;
    result->masks_built = built_count;
    result->requests_failed = failed;
    result->pending_after = pending_after;
    result->elapsed_ms = timer.Time();
  }
  if (serviced > 0 || pending_before > 0) {
    wxLogMessage(
        "WR_GRID_REQUEST_SERVICE pending_before=%d serviced=%d built=%d "
        "prefetched=%d failed=%d pending_after=%d elapsed_ms=%ld "
        "main_thread=1",
        pending_before, serviced, built_count, prefetched_count, failed,
        pending_after, timer.Time());
  }
  return failed == 0;
}

bool PlugIn_PrewarmSegmentSafetyRawTiles(
    const long* lat_tiles, const long* lon_tiles, int tile_count,
    int require_depth, PlugInSegmentSafetyResult* result) {
  wxStopWatch timer;
  InitSegmentSafetyResult(result);
  if (!wxThread::IsMain() || !ChartData) {
    SetSegmentSafetyStatus(result, PI_SEGMENT_SAFETY_NO_DATA);
    SetSegmentSafetyMessage(
        result, wxThread::IsMain()
                    ? "chart database unavailable for raw tile extraction"
                    : "raw chart tile extraction is main-thread only");
    return false;
  }
  if (result && result->struct_size < (int)sizeof(int)) return false;
  if (!lat_tiles || !lon_tiles || tile_count <= 0 || tile_count > 1000000) {
    SetSegmentSafetyStatus(result, PI_SEGMENT_SAFETY_ERROR);
    SetSegmentSafetyMessage(result, "invalid raw chart tile request");
    return false;
  }

  SegmentSafetyRefreshPersistentChartIdentity();
  SegmentSafetyPersistentCacheEnsureLoaded();
  std::set<std::pair<long, long> > tiles;
  for (int index = 0; index < tile_count; ++index)
    tiles.insert(std::make_pair(lat_tiles[index], lon_tiles[index]));

  {
    wxMutexLocker lock(s_segment_safety_cache_mutex);
    for (std::set<std::pair<long, long> >::const_iterator it = tiles.begin();
         it != tiles.end(); ++it)
      s_segment_safety_pinned_grid_keys.insert(
          SegmentSafetyGridTileKeyForIndices(it->first, it->second));
  }

  SegmentSafetyCoreStats stats;
  long built_tiles = 0;
  long reused_tiles = 0;
  long failed_tiles = 0;
  PlugInSegmentSafetySource source = PI_SEGMENT_SAFETY_SOURCE_NONE;
  // This is the primary xWeatherRouting prewarm entry point.  Classify
  // adjacent licensed plugin-vector tiles in provider-sized rectangles before
  // the compatibility loop below; any mixed, unsupported or incomplete tile
  // still falls through to the established per-tile path.
  const std::set<std::pair<long, long> > plugin_batch_built_tiles =
      PrebuildSegmentSafetyPluginVectorGridTiles(
          tiles, &stats, require_depth != 0);
  for (std::set<std::pair<long, long> >::const_iterator it = tiles.begin();
       it != tiles.end(); ++it) {
    bool built = false;
    if (!EnsureSegmentSafetyGridTile(it->first, it->second, &stats, &built,
                                     require_depth != 0)) {
      ++failed_tiles;
      continue;
    }
    CachedPointSafetyGridTile tile;
    if (!LookupSegmentSafetyGridTile(
            SegmentSafetyGridTileKeyForIndices(it->first, it->second),
            &tile) ||
        !tile.built || (require_depth && !tile.depth_complete)) {
      ++failed_tiles;
      continue;
    }
    // An already-hot host tile might predate plugin cache registration.
    // Republish it so the caller always owns a complete immutable snapshot.
    SegmentSafetyExternalTileCacheStore(tile);
    if (source == PI_SEGMENT_SAFETY_SOURCE_NONE) source = tile.source;
    if (built || plugin_batch_built_tiles.count(*it))
      ++built_tiles;
    else
      ++reused_tiles;
  }

  if (result) {
    SetSegmentSafetyStatus(result, failed_tiles == 0
                                       ? PI_SEGMENT_SAFETY_SAFE
                                       : PI_SEGMENT_SAFETY_NO_DATA);
    SetSegmentSafetySource(result, source);
    SetSegmentSafetyDiagnosticReason(
        result, failed_tiles == 0
                    ? PI_SEGMENT_SAFETY_DIAG_CHART_GEOMETRY_CLEAR
                    : PI_SEGMENT_SAFETY_DIAG_NO_CANDIDATE_CHART);
    SetSegmentSafetyMessage(
        result, failed_tiles == 0
                    ? "raw chart classification tiles published to plugin"
                    : "one or more raw chart tiles could not be extracted");
    ApplySegmentSafetyStats(result, stats);
    if (result->struct_size >=
        (int)(offsetof(PlugInSegmentSafetyResult, prewarm_fine_tiles_avoided) +
              sizeof(result->prewarm_fine_tiles_avoided))) {
      result->prewarm_requested_tiles = static_cast<int>(tiles.size());
      result->prewarm_base_tiles_built = static_cast<int>(built_tiles);
      result->prewarm_base_tiles_reused = static_cast<int>(reused_tiles);
      result->prewarm_masks_built = 0;
      result->prewarm_masks_reused = 0;
      result->prewarm_fine_tiles_avoided = 0;
    }
  }
  wxLogMessage(
      "WR_RAW_TILE_PREWARM requested=%lu built=%ld reused=%ld failed=%ld "
      "require_depth=%d elapsed_ms=%ld cells=%d land=%d water=%d drying=%d "
      "unknown=%d ownership=weather-routing-plugin",
      static_cast<unsigned long>(tiles.size()), built_tiles, reused_tiles,
      failed_tiles, require_depth ? 1 : 0, timer.Time(),
      stats.grid_cells_total, stats.grid_cells_land, stats.grid_cells_water,
      stats.grid_cells_drying, stats.grid_cells_unknown);
  return failed_tiles == 0;
}

bool PlugIn_PrewarmSegmentSafetyHazardSnapshot(
    double min_lat, double min_lon, double max_lat, double max_lon,
    int enable_fast_path, int shadow_compare,
    const PlugInSegmentSafetyOptions* options,
    PlugInSegmentSafetyResult* result) {
  wxStopWatch timer;
  InitSegmentSafetyResult(result);
  if (!wxThread::IsMain() || !ChartData) {
    SetSegmentSafetyStatus(result, PI_SEGMENT_SAFETY_NO_DATA);
    SetSegmentSafetyMessage(
        result, wxThread::IsMain()
                    ? "chart database unavailable for hazard snapshot"
                    : "hazard snapshot construction is main-thread only");
    return false;
  }
  if (options && options->struct_size < (int)sizeof(int)) {
    SetSegmentSafetyStatus(result, PI_SEGMENT_SAFETY_ERROR);
    SetSegmentSafetyMessage(result, "invalid segment safety options");
    return false;
  }
  if (result && result->struct_size < (int)sizeof(int)) return false;

  if (min_lat > max_lat) std::swap(min_lat, max_lat);
  if (min_lon > max_lon) std::swap(min_lon, max_lon);
  min_lat = wxMax(-89.0, wxMin(89.0, min_lat));
  max_lat = wxMax(-89.0, wxMin(89.0, max_lat));
  min_lon = wxMax(-180.0, wxMin(180.0, min_lon));
  max_lon = wxMax(-180.0, wxMin(180.0, max_lon));
  if (min_lat >= max_lat || min_lon >= max_lon) {
    SetSegmentSafetyStatus(result, PI_SEGMENT_SAFETY_ERROR);
    SetSegmentSafetyMessage(result, "invalid hazard snapshot bounds");
    return false;
  }

  SegmentSafetyRefreshPersistentChartIdentity();
  std::shared_ptr<SegmentSafetyHazardSnapshot> snapshot(
      new SegmentSafetyHazardSnapshot);
  snapshot->chart_identity = SegmentSafetyChartIdentity();
  snapshot->group_index = SegmentSafetyCurrentGroupIndex();
  snapshot->area = {min_lat, max_lat, min_lon, max_lon};
  snapshot->fast_path_enabled = enable_fast_path != 0;
  snapshot->shadow_compare = shadow_compare != 0;

  const SegmentSafetyBBox requested = snapshot->area;
  int candidate_entries = 0;
  int supported_entries = 0;
  int unsupported_entries = 0;
  const int entries = ChartData->GetChartTableEntries();
  for (int index = 0; index < entries; ++index) {
    const ChartTableEntry& entry = ChartData->GetChartTableEntry(index);
    ChartBase* chart = ChartData->OpenChartFromDB(index, FULL_INIT);
    const bool composite =
        chart && (chart->GetChartType() == CHART_TYPE_CM93 ||
                  chart->GetChartType() == CHART_TYPE_CM93COMP);
    const bool table_composite = entry.GetChartType() == CHART_TYPE_CM93 ||
                                 entry.GetChartType() == CHART_TYPE_CM93COMP;
    const bool global_or_invalid_bbox =
        entry.GetLatMax() > 90.0 || entry.GetLatMin() < -90.0;
    if (global_or_invalid_bbox && !table_composite && !composite) continue;
    const std::vector<int>& groups = entry.GetGroupArray();
    if (snapshot->group_index > 0 &&
        std::find(groups.begin(), groups.end(), snapshot->group_index) ==
            groups.end())
      continue;

    SegmentSafetySnapshotChart captured;
    captured.db_index = index;
    captured.native_scale = entry.GetScale();
    captured.edition_date = entry.GetChartEditionDate();
    captured.file_time = entry.GetFileTime();
    captured.chart_path = wxString::FromUTF8(entry.GetFullPath().c_str());
    captured.bbox =
        (global_or_invalid_bbox || composite)
            ? requested
            : SegmentSafetyBBox{entry.GetLatMin(), entry.GetLatMax(),
                                entry.GetLonMin(), entry.GetLonMax()};
    if (!SegmentSafetyBBoxIntersects(captured.bbox, requested)) continue;
    ++candidate_entries;

    for (int aux = 0; aux < entry.GetnAuxPlyEntries(); ++aux) {
      CachedLandRing ring = SegmentSafetyRingFromFloatTable(
          entry.GetpAuxPlyTableEntry(aux), entry.GetAuxCntTableEntry(aux));
      if (ring.points.size() >= 3) captured.coverage.push_back(ring);
    }
    if (captured.coverage.empty()) {
      CachedLandRing ring = SegmentSafetyRingFromFloatTable(
          entry.GetpPlyTable(), entry.GetnPlyEntries());
      if (ring.points.size() >= 3) captured.coverage.push_back(ring);
    }
    for (int hole = 0; hole < entry.GetnNoCovrPlyEntries(); ++hole) {
      CachedLandRing ring =
          SegmentSafetyRingFromFloatTable(entry.GetpNoCovrPlyTableEntry(hole),
                                          entry.GetNoCovrCntTableEntry(hole));
      if (ring.points.size() >= 3) captured.no_coverage.push_back(ring);
    }

    s57chart* s57 = dynamic_cast<s57chart*>(chart);
    captured.vector_supported = s57 != NULL && !composite &&
                                entry.GetChartFamily() == CHART_FAMILY_VECTOR;
    if (captured.vector_supported) {
      captured.source = PI_SEGMENT_SAFETY_SOURCE_VECTOR_CHART;
      std::set<std::string> seen_hazards;
      SegmentSafetyAppendFeatureRings(s57, "LNDARE", &captured.hazards,
                                      &seen_hazards);
      ++supported_entries;
    } else {
      // CM93 composites dynamically rebuild their active rule set while
      // selecting cells.  Identical scans can expose different ring sets, so
      // they are not accepted as immutable SAFE proof.  The persistent
      // highest-detail authoritative raster/coarse hierarchy remains active.
      captured.source = composite ? PI_SEGMENT_SAFETY_SOURCE_CM93
                                  : PI_SEGMENT_SAFETY_SOURCE_NONE;
      ++unsupported_entries;
    }
    snapshot->coverage_rings += captured.coverage.size();
    snapshot->hazard_rings += captured.hazards.size();
    snapshot->charts.push_back(std::move(captured));
  }

  BuildSegmentSafetySnapshotHierarchy(snapshot.get());
  {
    wxMutexLocker lock(s_segment_safety_cache_mutex);
    s_segment_safety_hazard_snapshot = snapshot;
    s_segment_safety_snapshot_queries = 0;
    s_segment_safety_snapshot_safe = 0;
    s_segment_safety_snapshot_unknown = 0;
    s_segment_safety_snapshot_shadow_disagreements = 0;
  }

  if (result) {
    SetSegmentSafetyStatus(result, snapshot->charts.empty()
                                       ? PI_SEGMENT_SAFETY_NO_DATA
                                       : PI_SEGMENT_SAFETY_SAFE);
    SetSegmentSafetySource(result, supported_entries > 0
                                       ? PI_SEGMENT_SAFETY_SOURCE_VECTOR_CHART
                                       : PI_SEGMENT_SAFETY_SOURCE_NONE);
    SetSegmentSafetyMessage(result,
                            snapshot->charts.empty()
                                ? "no chart coverage found for hazard snapshot"
                                : "immutable chart hazard snapshot ready");
    result->candidate_chart_count = candidate_entries;
    result->s57_chart_count = supported_entries;
    result->unsupported_chart_count = unsupported_entries;
    result->land_ring_count = snapshot->hazard_rings;
    result->cache_build_ms = timer.Time();
  }
  wxLogMessage(
      "WR_HAZARD_SNAPSHOT_BUILD area=[%.6f..%.6f,%.6f..%.6f] group=%d "
      "chart_identity=\"%s\" candidates=%d supported=%d unsupported=%d "
      "coverage_rings=%ld hazard_rings=%ld safe_large_cells=%lu "
      "safe_fine_cells=%lu fast_path=%d shadow=%d "
      "elapsed_ms=%ld selection=largest-scale-then-newest "
      "uncertainty=fallback-authoritative",
      min_lat, max_lat, min_lon, max_lon, snapshot->group_index,
      snapshot->chart_identity, candidate_entries, supported_entries,
      unsupported_entries, snapshot->coverage_rings, snapshot->hazard_rings,
      static_cast<unsigned long>(snapshot->certified_safe_large_cells.size()),
      static_cast<unsigned long>(snapshot->certified_safe_fine_cells.size()),
      snapshot->fast_path_enabled ? 1 : 0, snapshot->shadow_compare ? 1 : 0,
      timer.Time());
  return !snapshot->charts.empty();
}

bool PlugIn_CheckSegmentSafety(double lat1, double lon1, double lat2,
                               double lon2,
                               const PlugInSegmentSafetyOptions* options,
                               PlugInSegmentSafetyResult* result) {
  std::chrono::steady_clock::time_point query_start =
      std::chrono::steady_clock::now();
  InitSegmentSafetyResult(result);
  if (wxThread::IsMain()) {
    SegmentSafetyRefreshPersistentChartIdentity();
    SegmentSafetyPersistentCacheEnsureLoaded();
  }

  if (options && options->struct_size < (int)sizeof(int)) {
    SetSegmentSafetyStatus(result, PI_SEGMENT_SAFETY_ERROR);
    SetSegmentSafetyMessage(result, "invalid segment safety options");
    return false;
  }

  if (result && result->struct_size < (int)sizeof(int)) return false;

  const double safety_margin_nm = SegmentSafetyOptionMargin(options);
  const bool check_land = SegmentSafetyOptionCheckLand(options);
  const bool allow_gshhs_fallback =
      SegmentSafetyOptionAllowGshhsFallback(options);
  const bool check_depth = SegmentSafetyOptionCheckDepth(options);
  const double minimum_depth_m = SegmentSafetyOptionMinimumDepthM(options);
  const bool force_authoritative_fine =
      SegmentSafetyOptionForceAuthoritativeFineValidation(options);

  if (!check_land && !check_depth) {
    if (result) {
      SetSegmentSafetyStatus(result, PI_SEGMENT_SAFETY_SAFE);
      SetSegmentSafetyMessage(result, "land checks disabled");
    }
    return true;
  }

  bool chart_data_available = false;
  SegmentSafetyCoreStats stats;
  SegmentSafetySnapshotDecision snapshot_decision =
      SEGMENT_SAFETY_SNAPSHOT_UNKNOWN;
  bool snapshot_fast_path = false;
  bool snapshot_shadow_compare = false;
  if (check_land && !force_authoritative_fine) {
    {
      wxMutexLocker lock(s_segment_safety_cache_mutex);
      if (s_segment_safety_hazard_snapshot) {
        snapshot_fast_path =
            s_segment_safety_hazard_snapshot->fast_path_enabled;
        snapshot_shadow_compare =
            s_segment_safety_hazard_snapshot->shadow_compare;
      }
    }
    snapshot_decision = QuerySegmentSafetyHazardSnapshot(
        lat1, lon1, lat2, lon2, safety_margin_nm, check_depth);
    wxMutexLocker lock(s_segment_safety_cache_mutex);
    ++s_segment_safety_snapshot_queries;
    if (snapshot_decision == SEGMENT_SAFETY_SNAPSHOT_SAFE)
      ++s_segment_safety_snapshot_safe;
    else
      ++s_segment_safety_snapshot_unknown;
  }
  auto log_query = [&](const char* phase) {
    if (!result) return;
    if (snapshot_shadow_compare &&
        snapshot_decision == SEGMENT_SAFETY_SNAPSHOT_SAFE &&
        (result->status == PI_SEGMENT_SAFETY_CROSSES_LAND ||
         result->status == PI_SEGMENT_SAFETY_WITHIN_LAND_MARGIN ||
         result->status == PI_SEGMENT_SAFETY_DRYING_AREA ||
         result->status == PI_SEGMENT_SAFETY_TOO_SHALLOW)) {
      long disagreements = 0;
      {
        wxMutexLocker lock(s_segment_safety_cache_mutex);
        disagreements = ++s_segment_safety_snapshot_shadow_disagreements;
      }
      wxLogMessage(
          "WR_HAZARD_SNAPSHOT_SHADOW_MISMATCH #%ld "
          "segment=(%.8f,%.8f)->(%.8f,%.8f) margin_nm=%.3f "
          "snapshot=safe authoritative_status=%d phase=%s",
          disagreements, lat1, lon1, lat2, lon2, safety_margin_nm,
          result->status, phase);
    }
    long query_log_index = 0;
    {
      wxMutexLocker lock(s_segment_safety_cache_mutex);
      if (s_segment_safety_query_logs >= kMaxSegmentSafetyQueryLogs) return;
      int64_t elapsed_us =
          std::chrono::duration_cast<std::chrono::microseconds>(
              std::chrono::steady_clock::now() - query_start)
              .count();
      bool notable = result->status != PI_SEGMENT_SAFETY_SAFE ||
                     stats.unexpected_tile_builds > 0 || elapsed_us > 5000 ||
                     stats.water_tile_shortcuts > 0;
      if (!notable && s_segment_safety_query_logs >= 40) return;
      query_log_index = ++s_segment_safety_query_logs;
    }

    int unsafe_flags = 0;
    if (result->status == PI_SEGMENT_SAFETY_CROSSES_LAND ||
        result->status == PI_SEGMENT_SAFETY_WITHIN_LAND_MARGIN)
      unsafe_flags |= 1;
    if (result->status == PI_SEGMENT_SAFETY_DRYING_AREA) unsafe_flags |= 2;
    if (result->status == PI_SEGMENT_SAFETY_TOO_SHALLOW) unsafe_flags |= 4;
    if (result->status == PI_SEGMENT_SAFETY_UNKNOWN_DEPTH) unsafe_flags |= 8;
    if (result->status == PI_SEGMENT_SAFETY_NO_DATA ||
        result->status == PI_SEGMENT_SAFETY_ERROR ||
        result->status == PI_SEGMENT_SAFETY_PENDING_DATA ||
        stats.unexpected_tile_builds > 0)
      unsafe_flags |= 16;

    int64_t elapsed_us = std::chrono::duration_cast<std::chrono::microseconds>(
                             std::chrono::steady_clock::now() - query_start)
                             .count();
    int chart_api_calls = stats.point_cache_misses + stats.s57_chart_count +
                          stats.candidate_chart_count + stats.land_ring_count;
    long mixed_cell_checks =
        stats.grid_lookups > stats.water_tile_shortcuts
            ? stats.grid_lookups - stats.water_tile_shortcuts
            : stats.grid_lookups;
    wxLogMessage(
        "WR_GRID_SEGMENT_QUERY #%ld phase=%s worker=%d segment=(%.8f,%.8f)->"
        "(%.8f,%.8f) margin_nm=%.3f depth_check=%d status=%d source=%d "
        "authoritative_fine=%d persistent_cache_used_in_query=%d "
        "unsafe_flags=%d all_safe_tile_shortcuts=%d mixed_tile_cell_checks=%ld "
        "missing_tile_requests=%d tile_cache_hits=%d tile_cache_misses=%d "
        "grid_lookups=%d samples=%d query_time_us=%lld "
        "chart_api_calls_during_query=%d "
        "worker_thread_query_without_chart_api=%d "
        "message=\"%s\".",
        query_log_index, phase, wxThread::IsMain() ? 0 : 1, lat1, lon1, lat2,
        lon2, safety_margin_nm, check_depth ? 1 : 0, result->status,
        result->source, force_authoritative_fine ? 1 : 0,
        force_authoritative_fine ? 0 : stats.coarse_certified_safe_hits,
        unsafe_flags, stats.water_tile_shortcuts, mixed_cell_checks,
        stats.unexpected_tile_builds, stats.grid_cache_hits,
        stats.grid_cache_misses, stats.grid_lookups, stats.segment_sample_count,
        (long long)elapsed_us, chart_api_calls,
        (!wxThread::IsMain() && chart_api_calls == 0) ? 1 : 0, result->message);
    if (stats.coarse_cells_checked > 0 ||
        stats.coarse_certified_safe_hits > 0 || stats.coarse_missing > 0) {
      wxLogMessage(
          "WR_COARSE_SAFETY_QUERY_SUMMARY #%ld coarse_cells_checked=%d "
          "coarse_certified_safe_hits=%d coarse_mixed_fallbacks=%d "
          "coarse_unknown_fallbacks=%d coarse_no_chart=%d "
          "coarse_depth_unproven=%d coarse_missing=%d fine_tiles_avoided=%d "
          "query_time_us=%lld chart_api_calls_during_query=%d "
          "worker_thread_query_without_chart_api=%d",
          query_log_index, stats.coarse_cells_checked,
          stats.coarse_certified_safe_hits, stats.coarse_mixed_fallbacks,
          stats.coarse_unknown_fallbacks, stats.coarse_no_chart,
          stats.coarse_depth_unproven, stats.coarse_missing,
          stats.fine_tiles_avoided, (long long)elapsed_us, chart_api_calls,
          (!wxThread::IsMain() && chart_api_calls == 0) ? 1 : 0);
    }
  };
  if (snapshot_fast_path && snapshot_decision == SEGMENT_SAFETY_SNAPSHOT_SAFE) {
    SetSegmentSafetyStatus(result, PI_SEGMENT_SAFETY_SAFE);
    SetSegmentSafetySource(result, PI_SEGMENT_SAFETY_SOURCE_VECTOR_CHART);
    SetSegmentSafetyDiagnosticReason(
        result, PI_SEGMENT_SAFETY_DIAG_CHART_GEOMETRY_CLEAR);
    SetSegmentSafetyMessage(
        result, "segment is clear using immutable best-chart hazard snapshot");
    ApplySegmentSafetyStats(result, stats);
    log_query("hazard-snapshot-safe");
    return true;
  }
  const std::string segment_cache_key = SegmentSafetySegmentCacheKey(
      lat1, lon1, lat2, lon2, safety_margin_nm, check_depth, minimum_depth_m);
  if (SegmentSafetyResultHas(
          result, offsetof(PlugInSegmentSafetyResult, depth_source_attribute),
          sizeof(result->depth_source_attribute)))
    result->required_depth_m = minimum_depth_m;
  CachedSegmentSafetyResult cached_segment_result;
  if (LookupSegmentSafetySegmentCache(segment_cache_key,
                                      &cached_segment_result)) {
    // Replaying unsafe rejects is conservative.  Replaying SAFE accepts is not
    // yet used while the experimental chart/grid service is being hardened,
    // since chart-set and scale changes are not represented by a stable
    // generation id in this cache key.
    if (cached_segment_result.status != PI_SEGMENT_SAFETY_SAFE) {
      ++stats.segment_cache_hits;
      CopyCachedSegmentSafetyToResult(cached_segment_result, result);
      ApplySegmentSafetyStats(result, stats);
      log_query("segment-cache-hit");
      return true;
    }
  }
  ++stats.segment_cache_misses;

  bool open_water_shortcut = false;
  if (ChartSegmentPointClassificationCheck(
          lat1, lon1, lat2, lon2, safety_margin_nm, check_depth,
          minimum_depth_m, force_authoritative_fine, result,
          &chart_data_available, &stats, &open_water_shortcut)) {
    SetSegmentSafetyDiagnosticReason(result,
                                     PI_SEGMENT_SAFETY_DIAG_CHART_GEOMETRY_HIT);
    StoreSegmentSafetySegmentCache(
        segment_cache_key,
        MakeCachedSegmentSafetyResult(
            result, PI_SEGMENT_SAFETY_DIAG_CHART_GEOMETRY_HIT),
        &stats);
    ApplySegmentSafetyStats(result, stats);
    log_query("chart-grid-hit");
    return true;
  }

  if (!chart_data_available && !wxThread::IsMain()) {
    SetSegmentSafetyStatus(result, PI_SEGMENT_SAFETY_PENDING_DATA);
    SetSegmentSafetySource(result, PI_SEGMENT_SAFETY_SOURCE_NONE);
    SetSegmentSafetyDiagnosticReason(result,
                                     PI_SEGMENT_SAFETY_DIAG_PENDING_DATA);
    SetSegmentSafetyMessage(
        result,
        "chart safety grid tile requested; worker is waiting for main-thread "
        "chart object classification");
    ApplySegmentSafetyStats(result, stats);
    log_query("missing-tile-worker");
    return true;
  }

  if (!chart_data_available &&
      CachedChartSegmentSafetyCheck(lat1, lon1, lat2, lon2, safety_margin_nm,
                                    result, &chart_data_available, &stats)) {
    SetSegmentSafetyDiagnosticReason(result,
                                     PI_SEGMENT_SAFETY_DIAG_CHART_GEOMETRY_HIT);
    ApplySegmentSafetyStats(result, stats);
    log_query("legacy-chart-hit");
    return true;
  }

  if (chart_data_available) {
    if (result) {
      SetSegmentSafetyStatus(result, PI_SEGMENT_SAFETY_SAFE);
      if (GetSegmentSafetySource(result) == PI_SEGMENT_SAFETY_SOURCE_NONE)
        SetSegmentSafetySource(result, PI_SEGMENT_SAFETY_SOURCE_VECTOR_CHART);
      SetSegmentSafetyDiagnosticReason(
          result, PI_SEGMENT_SAFETY_DIAG_CHART_GEOMETRY_CLEAR);
      SetSegmentSafetyMessage(
          result, "segment is clear using chart point land classification");
      if (open_water_shortcut) {
        SetSegmentSafetyMessage(
            result,
            "segment is clear using cached open-water chart grid cells");
      }
      ApplySegmentSafetyStats(result, stats);
      log_query(open_water_shortcut ? "chart-grid-all-safe"
                                    : "chart-grid-clear");
    }
    return true;
  }

  PlugInSegmentSafetyDiagnosticReason unavailable_reason =
      SegmentSafetyUnavailableReason(stats);
  if (allow_gshhs_fallback) {
    PlugInSegmentSafetyStatus fallback_status = PI_SEGMENT_SAFETY_SAFE;
    bool crosses_land = GshhsSegmentSafetyHitsLand(
        lat1, lon1, lat2, lon2, safety_margin_nm, &fallback_status);
    if (result) {
      SetSegmentSafetySource(result, PI_SEGMENT_SAFETY_SOURCE_GSHHS_FALLBACK);
      SetSegmentSafetyFallback(result, true);
      SetSegmentSafetyStatus(
          result, crosses_land ? fallback_status : PI_SEGMENT_SAFETY_SAFE);
      SetSegmentSafetyDiagnosticReason(result, unavailable_reason);
      SetSegmentSafetyMessage(
          result, crosses_land
                      ? "segment crosses GSHHS shoreline fallback"
                      : SegmentSafetyUnavailableMessage(unavailable_reason));
      ApplySegmentSafetyStats(result, stats);
      log_query("gshhs-fallback");
    }
    return true;
  }

  if (result) {
    SetSegmentSafetyStatus(result, PI_SEGMENT_SAFETY_NO_DATA);
    SetSegmentSafetyDiagnosticReason(result, unavailable_reason);
    SetSegmentSafetyMessage(
        result, SegmentSafetyUnavailableMessage(unavailable_reason));
    ApplySegmentSafetyStats(result, stats);
    log_query("no-data");
  }
  return true;
}

bool PlugIn_PrewarmSegmentSafetyGrid(double min_lat, double min_lon,
                                     double max_lat, double max_lon,
                                     PlugInSegmentSafetyResult* result) {
  InitSegmentSafetyResult(result);

  if (result && result->struct_size < (int)sizeof(int)) return false;

  if (min_lat > max_lat) std::swap(min_lat, max_lat);
  if (min_lon > max_lon) std::swap(min_lon, max_lon);
  min_lat = wxMax(-90.0, wxMin(90.0, min_lat));
  max_lat = wxMax(-90.0, wxMin(90.0, max_lat));

  long min_lat_tile = floor(min_lat / kSegmentSafetyGridTileDegrees);
  long max_lat_tile = floor(max_lat / kSegmentSafetyGridTileDegrees);
  long min_lon_tile = floor(min_lon / kSegmentSafetyGridTileDegrees);
  long max_lon_tile = floor(max_lon / kSegmentSafetyGridTileDegrees);

  long lat_count = max_lat_tile - min_lat_tile + 1;
  long lon_count = max_lon_tile - min_lon_tile + 1;
  long requested_tiles =
      lat_count > 0 && lon_count > 0 ? lat_count * lon_count : 0;
  SegmentSafetyCoreStats stats;
  long built_tiles = 0;
  long reused_tiles = 0;

  for (long lat_tile = min_lat_tile; lat_tile <= max_lat_tile; ++lat_tile) {
    for (long lon_tile = min_lon_tile; lon_tile <= max_lon_tile; ++lon_tile) {
      std::string key = SegmentSafetyGridTileKeyForIndices(lat_tile, lon_tile);
      if (LookupSegmentSafetyGridTile(key, NULL)) {
        ++stats.grid_cache_hits;
        ++reused_tiles;
        continue;
      }

      bool built = false;
      EnsureSegmentSafetyGridTile(lat_tile, lon_tile, &stats, &built);
      if (built) ++built_tiles;
    }
  }

  if (result) {
    SetSegmentSafetyStatus(result, PI_SEGMENT_SAFETY_SAFE);
    SetSegmentSafetySource(result, PI_SEGMENT_SAFETY_SOURCE_VECTOR_CHART);
    SetSegmentSafetyDiagnosticReason(
        result, PI_SEGMENT_SAFETY_DIAG_CHART_GEOMETRY_CLEAR);
    SetSegmentSafetyMessage(result, "segment safety grid prewarmed");
    ApplySegmentSafetyStats(result, stats);
  }

  wxString message = wxString::Format(
      "SEGMENT_SAFETY_GRID prewarm bbox=[lat %.6f..%.6f lon %.6f..%.6f] "
      "requested_tiles=%ld built_tiles=%ld reused_tiles=%ld ",
      min_lat, max_lat, min_lon, max_lon, requested_tiles, built_tiles,
      reused_tiles);
  message += wxString::Format(
      "build_ms=%d cells=%d land=%d water=%d drying=%d unknown=%d "
      "point_cache_hits=%d point_cache_misses=%d grid_cache_size=%lu "
      "grid_cache_evictions=%ld",
      stats.grid_build_ms, stats.grid_cells_total, stats.grid_cells_land,
      stats.grid_cells_water, stats.grid_cells_drying, stats.grid_cells_unknown,
      stats.point_cache_hits, stats.point_cache_misses,
      (unsigned long)SegmentSafetyGridCacheSize(),
      s_segment_safety_grid_cache_evictions);
  wxLogMessage("%s", message.c_str());

  return true;
}

bool PlugIn_PrewarmSegmentSafetyGridForSegment(
    double lat1, double lon1, double lat2, double lon2, double safety_margin_nm,
    PlugInSegmentSafetyResult* result) {
  InitSegmentSafetyResult(result);

  if (result && result->struct_size < (int)sizeof(int)) return false;

  double bearing = 0.0;
  double dist_nm = 0.0;
  ll_gc_ll_reverse(lat1, lon1, lat2, lon2, &bearing, &dist_nm);

  const double tile_sample_spacing_nm = 1.5;
  const int max_samples = 1024;
  int samples = wxMax(
      2, wxMin(max_samples, (int)ceil(dist_nm / tile_sample_spacing_nm) + 1));
  std::set<std::pair<long, long> > tiles;

  double offset_step_nm = safety_margin_nm > 0.0 ? 2.0 : 1.0;
  int offset_count = safety_margin_nm > 0.0
                         ? (int)ceil((2.0 * safety_margin_nm) / offset_step_nm)
                         : 0;

  for (int offset_index = 0; offset_index <= offset_count; ++offset_index) {
    double offset_nm = 0.0;
    if (offset_count > 0)
      offset_nm = -safety_margin_nm +
                  (2.0 * safety_margin_nm * offset_index) / offset_count;

    for (int i = 0; i < samples; ++i) {
      double sample_dist = samples == 1 ? 0.0 : dist_nm * i / (samples - 1);
      double lat = lat1;
      double lon = lon1;
      if (sample_dist > 0.0)
        ll_gc_ll(lat1, lon1, bearing, sample_dist, &lat, &lon);
      if (fabs(offset_nm) > 0.0) {
        double offset_lat, offset_lon;
        ll_gc_ll(lat, lon,
                 SegmentSafetyNormalizeBearing(
                     bearing + (offset_nm < 0.0 ? -90.0 : 90.0)),
                 fabs(offset_nm), &offset_lat, &offset_lon);
        lat = offset_lat;
        lon = offset_lon;
      }

      long lat_tile = 0;
      long lon_tile = 0;
      SegmentSafetyGridTileKey(lat, lon, &lat_tile, &lon_tile);
      tiles.insert(std::make_pair(lat_tile, lon_tile));
    }
  }

  SegmentSafetyCoreStats stats;
  long built_tiles = 0;
  long reused_tiles = 0;

  for (std::set<std::pair<long, long> >::const_iterator it = tiles.begin();
       it != tiles.end(); ++it) {
    std::string key = SegmentSafetyGridTileKeyForIndices(it->first, it->second);
    if (LookupSegmentSafetyGridTile(key, NULL)) {
      ++stats.grid_cache_hits;
      ++reused_tiles;
      continue;
    }
    bool built = false;
    EnsureSegmentSafetyGridTile(it->first, it->second, &stats, &built);
    if (built) ++built_tiles;
  }

  if (result) {
    SetSegmentSafetyStatus(result, PI_SEGMENT_SAFETY_SAFE);
    SetSegmentSafetySource(result, PI_SEGMENT_SAFETY_SOURCE_VECTOR_CHART);
    SetSegmentSafetyDiagnosticReason(
        result, PI_SEGMENT_SAFETY_DIAG_CHART_GEOMETRY_CLEAR);
    SetSegmentSafetyMessage(result, "segment safety corridor prewarmed");
    ApplySegmentSafetyStats(result, stats);
  }

  wxString message = wxString::Format(
      "SEGMENT_SAFETY_GRID prewarm_segment start=(%.6f,%.6f) "
      "end=(%.6f,%.6f) margin_nm=%.3f samples=%d requested_tiles=%lu "
      "built_tiles=%ld reused_tiles=%ld ",
      lat1, lon1, lat2, lon2, safety_margin_nm, samples,
      static_cast<unsigned long>(tiles.size()), built_tiles, reused_tiles);
  message += wxString::Format(
      "build_ms=%d cells=%d land=%d water=%d drying=%d unknown=%d "
      "point_cache_hits=%d point_cache_misses=%d grid_cache_size=%lu "
      "grid_cache_evictions=%ld",
      stats.grid_build_ms, stats.grid_cells_total, stats.grid_cells_land,
      stats.grid_cells_water, stats.grid_cells_drying, stats.grid_cells_unknown,
      stats.point_cache_hits, stats.point_cache_misses,
      (unsigned long)SegmentSafetyGridCacheSize(),
      s_segment_safety_grid_cache_evictions);
  wxLogMessage("%s", message.c_str());

  return true;
}

bool PlugIn_PrewarmSegmentSafetyRouteMask(
    double min_lat, double min_lon, double max_lat, double max_lon,
    const PlugInSegmentSafetyOptions* options,
    PlugInSegmentSafetyResult* result) {
  InitSegmentSafetyResult(result);
  SegmentSafetyRefreshPersistentChartIdentity();
  SegmentSafetyPersistentCacheEnsureLoaded();

  if (result && result->struct_size < (int)sizeof(int)) return false;

  PlugInSegmentSafetyOptions effective_options = {};
  effective_options.struct_size = sizeof(effective_options);
  if (options) effective_options = *options;
  double safety_margin_nm = wxMax(0.0, effective_options.safety_margin_nm);
  bool check_depth = effective_options.check_depth != 0;
  double minimum_depth_m = effective_options.minimum_depth_m;

  if (min_lat > max_lat) std::swap(min_lat, max_lat);
  if (min_lon > max_lon) std::swap(min_lon, max_lon);
  min_lat = wxMax(-90.0, wxMin(90.0, min_lat));
  max_lat = wxMax(-90.0, wxMin(90.0, max_lat));

  long min_lat_tile = floor(min_lat / kSegmentSafetyGridTileDegrees);
  long max_lat_tile = floor(max_lat / kSegmentSafetyGridTileDegrees);
  long min_lon_tile = floor(min_lon / kSegmentSafetyGridTileDegrees);
  long max_lon_tile = floor(max_lon / kSegmentSafetyGridTileDegrees);

  long lat_count = max_lat_tile - min_lat_tile + 1;
  long lon_count = max_lon_tile - min_lon_tile + 1;
  long requested_tiles =
      lat_count > 0 && lon_count > 0 ? lat_count * lon_count : 0;

  // Keep the complete active envelope resident.  In particular, do this
  // before the first tile build so an envelope larger than the inactive cache
  // target cannot evict its own start tiles while it is still being built.
  PinSegmentSafetyRouteMaskEnvelope(min_lat_tile, max_lat_tile, min_lon_tile,
                                    max_lon_tile, safety_margin_nm, check_depth,
                                    minimum_depth_m);

  SegmentSafetyCoreStats stats;
  long base_built_tiles = 0;
  long base_reused_tiles = 0;
  long mask_built_tiles = 0;
  long mask_reused_tiles = 0;
  long fine_tiles_avoided_by_certified_safe = 0;
  long coarse_requested_cells = 0;
  long coarse_built_cells = 0;
  long coarse_reused_cells = 0;
  long coarse_certified_safe_cells = 0;
  long coarse_missing_cells = 0;

  for (long lat_tile = min_lat_tile; lat_tile <= max_lat_tile; ++lat_tile) {
    for (long lon_tile = min_lon_tile; lon_tile <= max_lon_tile; ++lon_tile) {
      CachedSegmentSafetyCoarseRouteMaskCell certified_coarse;
      if (LookupCertifiedSegmentSafetyCoarseRouteMaskCellForFineTile(
              lat_tile, lon_tile, safety_margin_nm, check_depth,
              minimum_depth_m, &certified_coarse)) {
        std::string mask_key = SegmentSafetyRouteMaskKey(
            lat_tile, lon_tile, safety_margin_nm, check_depth, minimum_depth_m);
        if (!LookupSegmentSafetyRouteMaskTile(mask_key, NULL)) {
          StoreSegmentSafetyRouteMaskTile(
              mask_key, BuildPersistentCertifiedSafeRouteMaskTile(
                            lat_tile, lon_tile, safety_margin_nm, check_depth,
                            minimum_depth_m, certified_coarse));
        }
        ++fine_tiles_avoided_by_certified_safe;
        ++base_reused_tiles;
        ++mask_reused_tiles;
        ++stats.fine_tiles_avoided;
        continue;
      }

      std::string base_key =
          SegmentSafetyGridTileKeyForIndices(lat_tile, lon_tile);
      if (LookupSegmentSafetyGridTile(base_key, NULL)) {
        ++stats.grid_cache_hits;
        ++base_reused_tiles;
      } else {
        bool built = false;
        EnsureSegmentSafetyGridTile(lat_tile, lon_tile, &stats, &built);
        if (built)
          ++base_built_tiles;
        else
          ++base_reused_tiles;
      }

      bool mask_built = false;
      std::string mask_key = SegmentSafetyRouteMaskKey(
          lat_tile, lon_tile, safety_margin_nm, check_depth, minimum_depth_m);
      if (LookupSegmentSafetyRouteMaskTile(mask_key, NULL)) {
        ++mask_reused_tiles;
      } else if (EnsureSegmentSafetyRouteMaskTile(
                     lat_tile, lon_tile, safety_margin_nm, check_depth,
                     minimum_depth_m, &stats, &mask_built)) {
        if (mask_built)
          ++mask_built_tiles;
        else
          ++mask_reused_tiles;
      }
    }
  }

  double coarse_degrees =
      kSegmentSafetyGridTileDegrees * kSegmentSafetyCoarseRouteMaskFactor;
  long min_coarse_lat = (long)floor(min_lat / coarse_degrees);
  long max_coarse_lat = (long)floor(max_lat / coarse_degrees);
  long min_coarse_lon = (long)floor(min_lon / coarse_degrees);
  long max_coarse_lon = (long)floor(max_lon / coarse_degrees);
  for (long lat_cell = min_coarse_lat; lat_cell <= max_coarse_lat; ++lat_cell) {
    for (long lon_cell = min_coarse_lon; lon_cell <= max_coarse_lon;
         ++lon_cell) {
      ++coarse_requested_cells;
      std::string coarse_key = SegmentSafetyCoarseRouteMaskKey(
          lat_cell, lon_cell, safety_margin_nm, check_depth, minimum_depth_m);
      CachedSegmentSafetyCoarseRouteMaskCell coarse;
      if (LookupSegmentSafetyCoarseRouteMaskCell(coarse_key, &coarse)) {
        ++coarse_reused_cells;
      } else if (EnsureSegmentSafetyCoarseRouteMaskCell(
                     lat_cell, lon_cell, safety_margin_nm, check_depth,
                     minimum_depth_m, &coarse, &stats)) {
        ++coarse_built_cells;
      } else {
        ++coarse_missing_cells;
        continue;
      }
      if (coarse.state == SEGMENT_SAFETY_COARSE_CERTIFIED_SAFE)
        ++coarse_certified_safe_cells;
    }
  }

  if (result) {
    SetSegmentSafetyStatus(result, PI_SEGMENT_SAFETY_SAFE);
    SetSegmentSafetySource(result, PI_SEGMENT_SAFETY_SOURCE_VECTOR_CHART);
    SetSegmentSafetyDiagnosticReason(
        result, PI_SEGMENT_SAFETY_DIAG_CHART_GEOMETRY_CLEAR);
    SetSegmentSafetyMessage(result, "segment safety route mask prewarmed");
    ApplySegmentSafetyStats(result, stats);
  }

  wxString message = wxString::Format(
      "WR_ROUTE_MASK_PREWARM bbox=[lat %.6f..%.6f lon %.6f..%.6f] "
      "requested_tiles=%ld base_built=%ld base_reused=%ld "
      "masks_built=%ld masks_reused=%ld fine_tiles_avoided=%ld "
      "margin_nm=%.3f depth_check=%d min_depth_m=%.2f coarse_requested=%ld "
      "coarse_built=%ld coarse_reused=%ld coarse_certified_safe=%ld "
      "coarse_missing=%ld ",
      min_lat, max_lat, min_lon, max_lon, requested_tiles, base_built_tiles,
      base_reused_tiles, mask_built_tiles, mask_reused_tiles,
      fine_tiles_avoided_by_certified_safe, safety_margin_nm,
      check_depth ? 1 : 0, minimum_depth_m, coarse_requested_cells,
      coarse_built_cells, coarse_reused_cells, coarse_certified_safe_cells,
      coarse_missing_cells);
  message += wxString::Format(
      "build_ms=%d cells=%d land=%d water=%d drying=%d unknown=%d "
      "point_cache_hits=%d point_cache_misses=%d grid_cache_size=%lu "
      "grid_cache_evictions=%ld",
      stats.grid_build_ms, stats.grid_cells_total, stats.grid_cells_land,
      stats.grid_cells_water, stats.grid_cells_drying, stats.grid_cells_unknown,
      stats.point_cache_hits, stats.point_cache_misses,
      (unsigned long)SegmentSafetyGridCacheSize(),
      s_segment_safety_grid_cache_evictions);
  wxLogMessage("%s", message.c_str());
  wxLogMessage(
      "WR_COARSE_SAFETY_BUILD scope=bbox requested=%ld built=%ld reused=%ld "
      "certified_safe=%ld missing=%ld certification_method=fine-mask-all-clear "
      "coarse_build_time_ms=%d fine_tiles_built=%ld fine_tiles_reused=%ld",
      coarse_requested_cells, coarse_built_cells, coarse_reused_cells,
      coarse_certified_safe_cells, coarse_missing_cells, stats.coarse_build_ms,
      mask_built_tiles, mask_reused_tiles);
  SegmentSafetyPersistentCacheSave();

  return true;
}

void AddSegmentSafetyRouteMaskCorridorTiles(
    double lat1, double lon1, double lat2, double lon2,
    double corridor_margin_nm, std::set<std::pair<long, long> >* tiles) {
  if (!tiles || !std::isfinite(lat1) || !std::isfinite(lon1) ||
      !std::isfinite(lat2) || !std::isfinite(lon2))
    return;

  double bearing = 0.0;
  double dist_nm = 0.0;
  ll_gc_ll_reverse(lat1, lon1, lat2, lon2, &bearing, &dist_nm);
  if (!std::isfinite(dist_nm) || dist_nm < 0.0) return;

  // With no speculative corridor width, use the identical fine-grid
  // supercover traversal used by worker safety queries.  This records the
  // exact route-mask tile footprint without distance-dependent sampling gaps.
  if (corridor_margin_nm <= 0.0) {
    long y0 = lround(lat1 / kSegmentSafetyGridResolutionDegrees);
    long x0 = lround(lon1 / kSegmentSafetyGridResolutionDegrees);
    long y1 = lround(lat2 / kSegmentSafetyGridResolutionDegrees);
    long x1 = lround(lon2 / kSegmentSafetyGridResolutionDegrees);
    long dx = labs(x1 - x0);
    long dy = labs(y1 - y0);
    long sx = x0 < x1 ? 1 : -1;
    long sy = y0 < y1 ? 1 : -1;
    long err = dx - dy;
    long x = x0;
    long y = y0;
    for (;;) {
      double lat = y * kSegmentSafetyGridResolutionDegrees;
      double lon = x * kSegmentSafetyGridResolutionDegrees;
      long lat_tile = 0;
      long lon_tile = 0;
      SegmentSafetyGridTileKey(lat, lon, &lat_tile, &lon_tile);
      tiles->insert(std::make_pair(lat_tile, lon_tile));
      if (x == x1 && y == y1) break;
      long e2 = 2 * err;
      if (e2 > -dy) {
        err -= dy;
        x += sx;
      }
      if (e2 < dx) {
        err += dx;
        y += sy;
      }
    }
    return;
  }

  const double tile_sample_spacing_nm = 1.5;
  const int max_samples = 1024;
  int samples = wxMax(
      2, wxMin(max_samples, (int)ceil(dist_nm / tile_sample_spacing_nm) + 1));
  double offset_step_nm = corridor_margin_nm > 0.0 ? 2.0 : 1.0;
  int offset_count =
      corridor_margin_nm > 0.0
          ? (int)ceil((2.0 * corridor_margin_nm) / offset_step_nm)
          : 0;

  for (int offset_index = 0; offset_index <= offset_count; ++offset_index) {
    double offset_nm = 0.0;
    if (offset_count > 0)
      offset_nm = -corridor_margin_nm +
                  (2.0 * corridor_margin_nm * offset_index) / offset_count;

    for (int i = 0; i < samples; ++i) {
      double sample_dist = dist_nm * i / (samples - 1);
      double lat = lat1;
      double lon = lon1;
      if (sample_dist > 0.0)
        ll_gc_ll(lat1, lon1, bearing, sample_dist, &lat, &lon);
      if (fabs(offset_nm) > 0.0) {
        double offset_lat = lat;
        double offset_lon = lon;
        ll_gc_ll(lat, lon,
                 SegmentSafetyNormalizeBearing(
                     bearing + (offset_nm < 0.0 ? -90.0 : 90.0)),
                 fabs(offset_nm), &offset_lat, &offset_lon);
        lat = offset_lat;
        lon = offset_lon;
      }
      long lat_tile = 0;
      long lon_tile = 0;
      SegmentSafetyGridTileKey(lat, lon, &lat_tile, &lon_tile);
      tiles->insert(std::make_pair(lat_tile, lon_tile));
    }
  }

  // Give the route ends rounded caps instead of relying on perpendicular
  // offsets alone.  This also provides the required start/end neighbourhoods.
  if (corridor_margin_nm > 0.0) {
    const int radial_bearings = 24;
    const int radial_steps = wxMax(1, (int)ceil(corridor_margin_nm / 2.0));
    const double endpoint_lats[2] = {lat1, lat2};
    const double endpoint_lons[2] = {lon1, lon2};
    for (int endpoint = 0; endpoint < 2; ++endpoint) {
      for (int radius_step = 0; radius_step <= radial_steps; ++radius_step) {
        double radius_nm = corridor_margin_nm * radius_step / radial_steps;
        for (int direction = 0; direction < radial_bearings; ++direction) {
          double lat = endpoint_lats[endpoint];
          double lon = endpoint_lons[endpoint];
          if (radius_nm > 0.0)
            ll_gc_ll(endpoint_lats[endpoint], endpoint_lons[endpoint],
                     360.0 * direction / radial_bearings, radius_nm, &lat,
                     &lon);
          long lat_tile = 0;
          long lon_tile = 0;
          SegmentSafetyGridTileKey(lat, lon, &lat_tile, &lon_tile);
          tiles->insert(std::make_pair(lat_tile, lon_tile));
        }
      }
    }
  }
}

bool PrewarmSegmentSafetyRouteMaskTileSet(
    const std::set<std::pair<long, long> >& tiles, double corridor_margin_nm,
    const PlugInSegmentSafetyOptions& options,
    PlugInSegmentSafetyResult* result, const char* scope, int polyline_count,
    int segment_count, int fine_tile_halo) {
  double safety_margin_nm = wxMax(0.0, options.safety_margin_nm);
  bool check_depth = options.check_depth != 0;
  double minimum_depth_m = options.minimum_depth_m;

  if (tiles.empty()) {
    if (result) {
      SetSegmentSafetyStatus(result, PI_SEGMENT_SAFETY_NO_DATA);
      SetSegmentSafetySource(result, PI_SEGMENT_SAFETY_SOURCE_NONE);
      SetSegmentSafetyMessage(result, "route mask prewarm geometry is empty");
    }
    return false;
  }

  PinSegmentSafetyRouteMaskTiles(tiles, safety_margin_nm, check_depth,
                                 minimum_depth_m);

  SegmentSafetyCoreStats stats;
  long base_built_tiles = 0;
  long base_reused_tiles = 0;
  long mask_built_tiles = 0;
  long mask_reused_tiles = 0;
  long fine_tiles_avoided_by_certified_safe = 0;
  long coarse_requested_cells = 0;
  long coarse_built_cells = 0;
  long coarse_reused_cells = 0;
  long coarse_certified_safe_cells = 0;
  long coarse_missing_cells = 0;

  std::map<std::pair<long, long>, std::set<std::pair<long, long> > >
      coarse_occupancy;
  for (std::set<std::pair<long, long> >::const_iterator it = tiles.begin();
       it != tiles.end(); ++it) {
    const long coarse_lat =
        (long)floor((double)it->first / kSegmentSafetyCoarseRouteMaskFactor);
    const long coarse_lon =
        (long)floor((double)it->second / kSegmentSafetyCoarseRouteMaskFactor);
    coarse_occupancy[std::make_pair(coarse_lat, coarse_lon)].insert(*it);
  }

  long coarse_base_proof_cells = 0;
  long coarse_base_proof_chart_build_cells = 0;
  for (std::map<std::pair<long, long>,
                std::set<std::pair<long, long> > >::const_iterator it =
           coarse_occupancy.begin();
       it != coarse_occupancy.end(); ++it) {
    ++coarse_requested_cells;
    const long coarse_lat = it->first.first;
    const long coarse_lon = it->first.second;
    const std::string coarse_key = SegmentSafetyCoarseRouteMaskKey(
        coarse_lat, coarse_lon, safety_margin_nm, check_depth, minimum_depth_m);
    CachedSegmentSafetyCoarseRouteMaskCell coarse;
    if (LookupSegmentSafetyCoarseRouteMaskCell(coarse_key, &coarse)) {
      ++coarse_reused_cells;
      continue;
    }
    if (SegmentSafetyPersistentLookupCertifiedSafe(
            coarse_lat, coarse_lon, safety_margin_nm, check_depth,
            minimum_depth_m, &coarse)) {
      StoreSegmentSafetyCoarseRouteMaskCell(coarse_key, coarse);
      ++coarse_reused_cells;
      continue;
    }

    bool built_from_base = BuildSegmentSafetyCoarseRouteMaskCellFromBase(
        coarse_lat, coarse_lon, safety_margin_nm, check_depth, minimum_depth_m,
        false, &coarse, &stats, &base_built_tiles);
    if (!built_from_base &&
        SegmentSafetyCoarseProofDoesNotExpandBaseBuildSet(
            coarse_lat, coarse_lon, it->second, safety_margin_nm)) {
      built_from_base = BuildSegmentSafetyCoarseRouteMaskCellFromBase(
          coarse_lat, coarse_lon, safety_margin_nm, check_depth,
          minimum_depth_m, true, &coarse, &stats, &base_built_tiles);
      if (built_from_base) ++coarse_base_proof_chart_build_cells;
    }
    if (built_from_base) {
      StoreSegmentSafetyCoarseRouteMaskCell(coarse_key, coarse);
      SegmentSafetyPersistentStoreCertifiedSafe(coarse);
      ++coarse_built_cells;
      ++coarse_base_proof_cells;
    }
  }

  // Build the complete fine-grid evidence halo before deriving any route
  // masks.  A mask with a non-zero safety margin depends on neighbouring base
  // tiles even when only its own route tile is requested.  Building masks
  // immediately after each centre tile left those neighbours to be discovered
  // on demand, causing long GUI-thread stalls and defeating the prewarm
  // contract.
  std::set<std::pair<long, long> > certified_fine_tiles;
  std::set<std::pair<long, long> > fine_base_tiles;
  for (std::set<std::pair<long, long> >::const_iterator it = tiles.begin();
       it != tiles.end(); ++it) {
    CachedSegmentSafetyCoarseRouteMaskCell certified_coarse;
    if (LookupCertifiedSegmentSafetyCoarseRouteMaskCellForFineTile(
            it->first, it->second, safety_margin_nm, check_depth,
            minimum_depth_m, &certified_coarse)) {
      certified_fine_tiles.insert(*it);
      continue;
    }
    const double tile_min_lat = it->first * kSegmentSafetyGridTileDegrees;
    const int margin_radius = SegmentSafetyMarginTileRadius(
        safety_margin_nm,
        wxMax(fabs(tile_min_lat),
              fabs(tile_min_lat + kSegmentSafetyGridTileDegrees)));
    AddSegmentSafetyTileHalo(it->first, it->second, margin_radius,
                             &fine_base_tiles);
  }

  // Licensed plugin-vector providers can classify a rectangular grid in one
  // semantic object traversal.  Prebuild adjacent requested tiles in bounded
  // blocks, then retain the existing per-tile path for mixed-provider,
  // unsupported or failed cells.  This changes only cache population; route
  // masks, missing-tile retries and routing decisions remain identical.
  const std::set<std::pair<long, long> > plugin_batch_built_tiles =
      PrebuildSegmentSafetyPluginVectorGridTiles(fine_base_tiles, &stats,
                                                  check_depth);

  for (std::set<std::pair<long, long> >::const_iterator it =
           fine_base_tiles.begin();
       it != fine_base_tiles.end(); ++it) {
    const std::string base_key =
        SegmentSafetyGridTileKeyForIndices(it->first, it->second);
    if (LookupSegmentSafetyGridTile(base_key, NULL)) {
      ++stats.grid_cache_hits;
      if (plugin_batch_built_tiles.count(*it))
        ++base_built_tiles;
      else
        ++base_reused_tiles;
    } else {
      bool built = false;
      EnsureSegmentSafetyGridTile(it->first, it->second, &stats, &built);
      if (built)
        ++base_built_tiles;
      else
        ++base_reused_tiles;
    }
  }

  for (std::set<std::pair<long, long> >::const_iterator it = tiles.begin();
       it != tiles.end(); ++it) {
    if (certified_fine_tiles.count(*it) != 0) {
      CachedSegmentSafetyCoarseRouteMaskCell certified_coarse;
      LookupCertifiedSegmentSafetyCoarseRouteMaskCellForFineTile(
          it->first, it->second, safety_margin_nm, check_depth, minimum_depth_m,
          &certified_coarse);
      std::string mask_key =
          SegmentSafetyRouteMaskKey(it->first, it->second, safety_margin_nm,
                                    check_depth, minimum_depth_m);
      if (!LookupSegmentSafetyRouteMaskTile(mask_key, NULL)) {
        StoreSegmentSafetyRouteMaskTile(
            mask_key, BuildPersistentCertifiedSafeRouteMaskTile(
                          it->first, it->second, safety_margin_nm, check_depth,
                          minimum_depth_m, certified_coarse));
      }
      ++fine_tiles_avoided_by_certified_safe;
      ++base_reused_tiles;
      ++mask_reused_tiles;
      ++stats.fine_tiles_avoided;
      continue;
    }

    bool mask_built = false;
    std::string mask_key = SegmentSafetyRouteMaskKey(
        it->first, it->second, safety_margin_nm, check_depth, minimum_depth_m);
    if (LookupSegmentSafetyRouteMaskTile(mask_key, NULL)) {
      ++mask_reused_tiles;
    } else if (EnsureSegmentSafetyRouteMaskTile(
                   it->first, it->second, safety_margin_nm, check_depth,
                   minimum_depth_m, &stats, &mask_built)) {
      if (mask_built)
        ++mask_built_tiles;
      else
        ++mask_reused_tiles;
    }
  }

  for (std::map<std::pair<long, long>,
                std::set<std::pair<long, long> > >::const_iterator it =
           coarse_occupancy.begin();
       it != coarse_occupancy.end(); ++it) {
    std::string coarse_key = SegmentSafetyCoarseRouteMaskKey(
        it->first.first, it->first.second, safety_margin_nm, check_depth,
        minimum_depth_m);
    CachedSegmentSafetyCoarseRouteMaskCell coarse;
    if (!LookupSegmentSafetyCoarseRouteMaskCell(coarse_key, &coarse) &&
        EnsureSegmentSafetyCoarseRouteMaskCell(
            it->first.first, it->first.second, safety_margin_nm, check_depth,
            minimum_depth_m, &coarse, &stats)) {
      ++coarse_built_cells;
    } else if (!LookupSegmentSafetyCoarseRouteMaskCell(coarse_key, &coarse)) {
      ++coarse_missing_cells;
      continue;
    }
    if (coarse.state == SEGMENT_SAFETY_COARSE_CERTIFIED_SAFE)
      ++coarse_certified_safe_cells;
  }

  if (result) {
    SetSegmentSafetyStatus(result, PI_SEGMENT_SAFETY_SAFE);
    SetSegmentSafetySource(result, PI_SEGMENT_SAFETY_SOURCE_VECTOR_CHART);
    SetSegmentSafetyDiagnosticReason(
        result, PI_SEGMENT_SAFETY_DIAG_CHART_GEOMETRY_CLEAR);
    SetSegmentSafetyMessage(result,
                            "segment safety route mask shape prewarmed");
    ApplySegmentSafetyStats(result, stats);
    if (result->struct_size >=
        (int)(offsetof(PlugInSegmentSafetyResult, prewarm_fine_tiles_avoided) +
              sizeof(result->prewarm_fine_tiles_avoided))) {
      result->prewarm_requested_tiles = static_cast<int>(tiles.size());
      result->prewarm_base_tiles_built = static_cast<int>(base_built_tiles);
      result->prewarm_base_tiles_reused = static_cast<int>(base_reused_tiles);
      result->prewarm_masks_built = static_cast<int>(mask_built_tiles);
      result->prewarm_masks_reused = static_cast<int>(mask_reused_tiles);
      result->prewarm_fine_tiles_avoided =
          static_cast<int>(fine_tiles_avoided_by_certified_safe);
    }
  }

  wxLogMessage(
      "WR_ROUTE_MASK_PREWARM_SHAPE scope=%s segments=%d "
      "corridor_margin_nm=%.3f fine_tile_halo=%d margin_nm=%.3f "
      "requested_tiles=%lu "
      "base_built=%ld base_reused=%ld masks_built=%ld masks_reused=%ld "
      "fine_tiles_avoided=%ld depth_check=%d min_depth_m=%.2f "
      "coarse_requested=%ld coarse_built=%ld coarse_reused=%ld "
      "coarse_certified_safe=%ld coarse_missing=%ld "
      "coarse_base_proofs=%ld coarse_base_proof_chart_builds=%ld "
      "build_ms=%d cells=%d "
      "land=%d water=%d drying=%d unknown=%d point_cache_hits=%d "
      "point_cache_misses=%d grid_cache_size=%lu grid_cache_evictions=%ld",
      scope ? scope : "unknown", segment_count, corridor_margin_nm,
      fine_tile_halo, safety_margin_nm,
      static_cast<unsigned long>(tiles.size()), base_built_tiles,
      base_reused_tiles, mask_built_tiles, mask_reused_tiles,
      fine_tiles_avoided_by_certified_safe, check_depth ? 1 : 0,
      minimum_depth_m, coarse_requested_cells, coarse_built_cells,
      coarse_reused_cells, coarse_certified_safe_cells, coarse_missing_cells,
      coarse_base_proof_cells, coarse_base_proof_chart_build_cells,
      stats.grid_build_ms, stats.grid_cells_total, stats.grid_cells_land,
      stats.grid_cells_water, stats.grid_cells_drying, stats.grid_cells_unknown,
      stats.point_cache_hits, stats.point_cache_misses,
      (unsigned long)SegmentSafetyGridCacheSize(),
      s_segment_safety_grid_cache_evictions);
  wxLogMessage(
      "WR_COARSE_SAFETY_BUILD scope=%s requested=%ld built=%ld reused=%ld "
      "certified_safe=%ld missing=%ld base_proofs=%ld "
      "base_proof_chart_builds=%ld "
      "certification_method=base-proof-first-then-fine-mask "
      "coarse_build_time_ms=%d fine_tiles_built=%ld fine_tiles_reused=%ld",
      scope ? scope : "unknown", coarse_requested_cells, coarse_built_cells,
      coarse_reused_cells, coarse_certified_safe_cells, coarse_missing_cells,
      coarse_base_proof_cells, coarse_base_proof_chart_build_cells,
      stats.coarse_build_ms, mask_built_tiles, mask_reused_tiles);
  SegmentSafetyPersistentCacheSave();
  return true;
}

bool PlugIn_PrewarmSegmentSafetyRouteMaskForSegment(
    double lat1, double lon1, double lat2, double lon2,
    double corridor_margin_nm, const PlugInSegmentSafetyOptions* options,
    PlugInSegmentSafetyResult* result) {
  InitSegmentSafetyResult(result);
  SegmentSafetyRefreshPersistentChartIdentity();
  SegmentSafetyPersistentCacheEnsureLoaded();

  if (result && result->struct_size < (int)sizeof(int)) return false;

  PlugInSegmentSafetyOptions effective_options = {};
  effective_options.struct_size = sizeof(effective_options);
  if (options) effective_options = *options;
  corridor_margin_nm = wxMax(0.0, corridor_margin_nm);
  std::set<std::pair<long, long> > tiles;
  AddSegmentSafetyRouteMaskCorridorTiles(lat1, lon1, lat2, lon2,
                                         corridor_margin_nm, &tiles);
  return PrewarmSegmentSafetyRouteMaskTileSet(
      tiles, corridor_margin_nm, effective_options, result, "segment", 1, 1, 0);
}

static bool PrewarmSegmentSafetyRouteMaskForPolylinesImpl(
    const double* latitudes, const double* longitudes, const int* point_counts,
    int polyline_count, double corridor_margin_nm, int fine_tile_halo,
    const PlugInSegmentSafetyOptions* options,
    PlugInSegmentSafetyResult* result) {
  InitSegmentSafetyResult(result);
  SegmentSafetyRefreshPersistentChartIdentity();
  SegmentSafetyPersistentCacheEnsureLoaded();
  if (result && result->struct_size < (int)sizeof(int)) return false;
  if (!latitudes || !longitudes || !point_counts || polyline_count <= 0)
    return false;

  PlugInSegmentSafetyOptions effective_options = {};
  effective_options.struct_size = sizeof(effective_options);
  if (options) effective_options = *options;
  corridor_margin_nm = wxMax(0.0, corridor_margin_nm);

  std::set<std::pair<long, long> > tiles;
  int point_offset = 0;
  int valid_polylines = 0;
  int segment_count = 0;
  for (int polyline = 0; polyline < polyline_count; ++polyline) {
    int count = point_counts[polyline];
    if (count < 0 || count > 1000000) return false;
    if (count == 1) {
      AddSegmentSafetyRouteMaskCorridorTiles(
          latitudes[point_offset], longitudes[point_offset],
          latitudes[point_offset], longitudes[point_offset], corridor_margin_nm,
          &tiles);
      ++valid_polylines;
    } else if (count >= 2) {
      ++valid_polylines;
      for (int point = 1; point < count; ++point) {
        AddSegmentSafetyRouteMaskCorridorTiles(
            latitudes[point_offset + point - 1],
            longitudes[point_offset + point - 1],
            latitudes[point_offset + point], longitudes[point_offset + point],
            corridor_margin_nm, &tiles);
        ++segment_count;
      }
    }
    point_offset += count;
  }

  fine_tile_halo = wxMax(0, wxMin(8, fine_tile_halo));
  if (fine_tile_halo > 0) {
    const std::set<std::pair<long, long> > exact_tiles = tiles;
    for (std::set<std::pair<long, long> >::const_iterator it =
             exact_tiles.begin();
         it != exact_tiles.end(); ++it)
      AddSegmentSafetyTileHalo(it->first, it->second, fine_tile_halo, &tiles);
  }
  return PrewarmSegmentSafetyRouteMaskTileSet(
      tiles, corridor_margin_nm, effective_options, result, "polylines",
      valid_polylines, segment_count, fine_tile_halo);
}

bool PlugIn_PrewarmSegmentSafetyRouteMaskForPolylines(
    const double* latitudes, const double* longitudes, const int* point_counts,
    int polyline_count, double corridor_margin_nm,
    const PlugInSegmentSafetyOptions* options,
    PlugInSegmentSafetyResult* result) {
  return PrewarmSegmentSafetyRouteMaskForPolylinesImpl(
      latitudes, longitudes, point_counts, polyline_count, corridor_margin_nm,
      0, options, result);
}

bool PlugIn_PrewarmSegmentSafetyRouteMaskForPolylinesWithTileHalo(
    const double* latitudes, const double* longitudes, const int* point_counts,
    int polyline_count, double corridor_margin_nm, int fine_tile_halo,
    const PlugInSegmentSafetyOptions* options,
    PlugInSegmentSafetyResult* result) {
  return PrewarmSegmentSafetyRouteMaskForPolylinesImpl(
      latitudes, longitudes, point_counts, polyline_count, corridor_margin_nm,
      fine_tile_halo, options, result);
}

void PlugIn_ReleaseSegmentSafetyRouteMaskPins() {
  size_t base_before = 0;
  size_t masks_before = 0;
  size_t base_after = 0;
  size_t masks_after = 0;
  size_t pinned_base = 0;
  size_t pinned_masks = 0;
  {
    wxMutexLocker lock(s_segment_safety_cache_mutex);
    base_before = s_segment_safety_grid_cache.size();
    masks_before = s_segment_safety_route_mask_cache.size();
    pinned_base = s_segment_safety_pinned_grid_keys.size();
    pinned_masks = s_segment_safety_pinned_route_mask_keys.size();
    s_segment_safety_pinned_grid_keys.clear();
    s_segment_safety_pinned_route_mask_keys.clear();

    while (s_segment_safety_grid_cache.size() > kMaxSegmentSafetyGridTiles) {
      s_segment_safety_grid_cache.erase(s_segment_safety_grid_cache.begin());
      ++s_segment_safety_grid_cache_evictions;
    }
    while (s_segment_safety_route_mask_cache.size() >
           kMaxSegmentSafetyGridTiles) {
      s_segment_safety_route_mask_cache.erase(
          s_segment_safety_route_mask_cache.begin());
    }
    base_after = s_segment_safety_grid_cache.size();
    masks_after = s_segment_safety_route_mask_cache.size();
  }
  wxLogMessage(
      "WR_ROUTE_MASK_PINS_RELEASED pinned_base=%lu pinned_masks=%lu "
      "base_before=%lu base_after=%lu masks_before=%lu masks_after=%lu "
      "evictions=%ld",
      (unsigned long)pinned_base, (unsigned long)pinned_masks,
      (unsigned long)base_before, (unsigned long)base_after,
      (unsigned long)masks_before, (unsigned long)masks_after,
      s_segment_safety_grid_cache_evictions);
}

bool PlugIn_RegisterSegmentSafetyTileCache(
    const PlugInSegmentSafetyTileCacheCallbacks* callbacks) {
  if (!wxThread::IsMain()) return false;
  wxString identity = SegmentSafetyChartIdentity();
  PlugInSegmentSafetyTileCacheIdentityFn identity_callback = nullptr;
  void* identity_context = nullptr;
  {
    wxMutexLocker lock(s_segment_safety_cache_mutex);
    if (!callbacks) {
      memset(&s_segment_safety_external_tile_cache, 0,
             sizeof(s_segment_safety_external_tile_cache));
      wxLogMessage("WR_PLUGIN_TILE_CACHE registered=0");
      return true;
    }
    if (callbacks->struct_size <
            static_cast<int>(
                offsetof(PlugInSegmentSafetyTileCacheCallbacks,
                         identity_changed) +
                sizeof(callbacks->identity_changed)) ||
        !callbacks->lookup || !callbacks->store)
      return false;
    memset(&s_segment_safety_external_tile_cache, 0,
           sizeof(s_segment_safety_external_tile_cache));
    memcpy(&s_segment_safety_external_tile_cache, callbacks,
           std::min(static_cast<size_t>(callbacks->struct_size),
                    sizeof(s_segment_safety_external_tile_cache)));
    identity_callback = callbacks->identity_changed;
    identity_context = callbacks->context;
  }
  if (identity_callback) {
    wxCharBuffer utf8 = identity.ToUTF8();
    identity_callback(identity_context, utf8.data() ? utf8.data() : "");
  }
  wxLogMessage(
      "WR_PLUGIN_TILE_CACHE registered=1 identity=\"%s\" lookup=1 store=1",
      identity);
  return true;
}

bool PlugIn_GetSegmentSafetyChartIdentity(char* identity, int identity_size) {
  if (!identity || identity_size <= 0 || !wxThread::IsMain()) return false;
  wxString value = SegmentSafetyChartIdentity();
  wxCharBuffer utf8 = value.ToUTF8();
  const char* source = utf8.data() ? utf8.data() : "";
  strncpy(identity, source, static_cast<size_t>(identity_size) - 1);
  identity[identity_size - 1] = '\0';
  return !value.IsEmpty();
}

int PlugIn_GetSegmentSafetyChartInfoCount() {
  if (!wxThread::IsMain() || !ChartData) return 0;
  return ChartData->GetChartTableEntries();
}

bool PlugIn_GetSegmentSafetyChartInfo(
    int ordinal, PlugInSegmentSafetyChartInfoV1* chart_info) {
  if (!wxThread::IsMain() || !ChartData || !chart_info || ordinal < 0 ||
      ordinal >= ChartData->GetChartTableEntries() ||
      chart_info->struct_size <
          static_cast<int>(sizeof(PlugInSegmentSafetyChartInfoV1)))
    return false;

  const ChartTableEntry& entry = ChartData->GetChartTableEntry(ordinal);
  const ChartTypeEnum type =
      static_cast<ChartTypeEnum>(entry.GetChartType());
  const ChartFamilyEnum family =
      static_cast<ChartFamilyEnum>(entry.GetChartFamily());
  const bool plugin_vector =
      type == CHART_TYPE_PLUGIN && family == CHART_FAMILY_VECTOR;
  // A proactive atlas targets bounded official/native and plugin vector
  // charts. CM93 is a global composite fallback and must never turn "all
  // charts" into an accidental whole-world atlas build.
  if (!plugin_vector && family != CHART_FAMILY_VECTOR) return false;
  if (ChartData->IsChartDirectoryExcluded(entry.GetFullPath())) return false;

  const int group_index = SegmentSafetyCurrentGroupIndex();
  const bool in_active_group =
      ChartData->IsChartInGroup(ordinal, group_index);
  if (!in_active_group) return false;
  const bool available = type != CHART_TYPE_PLUGIN ||
                         ChartData->IsChartAvailable(ordinal);
  if (!available) return false;

  PlugInSegmentSafetyChartInfoV1 result = {};
  result.struct_size = sizeof(result);
  result.abi_version = PI_SEGMENT_SAFETY_CHART_INFO_ABI_V1;
  result.db_index = ordinal;
  result.chart_type = entry.GetChartType();
  result.chart_family = entry.GetChartFamily();
  result.chart_scale = entry.GetScale();
  result.source = plugin_vector ? PI_SEGMENT_SAFETY_SOURCE_PLUGIN_VECTOR
                                : PI_SEGMENT_SAFETY_SOURCE_VECTOR_CHART;
  result.available = available ? 1 : 0;
  result.in_active_group = in_active_group ? 1 : 0;
  result.edition_time =
      static_cast<long long>(entry.GetChartEditionDate());
  result.file_time = static_cast<long long>(entry.GetFileTime());
  result.min_lat = entry.GetLatMin();
  result.min_lon = entry.GetLonMin();
  result.max_lat = entry.GetLatMax();
  result.max_lon = entry.GetLonMax();
  CopySegmentSafetyString(result.chart_path, sizeof(result.chart_path),
                          entry.GetFullPath().c_str());
  *chart_info = result;
  return true;
}

bool PlugIn_SetSegmentSafetyPersistentCacheEnabled(int enabled) {
  const bool was_enabled = s_segment_safety_persistent_cache_enabled;
  const bool will_enable = enabled != 0;
  if (was_enabled && !will_enable) SegmentSafetyPersistentCacheSave();
  s_segment_safety_persistent_cache_enabled = will_enable;
  if (will_enable) {
    SegmentSafetyRefreshPersistentChartIdentity();
    SegmentSafetyPersistentCacheEnsureLoaded();
  }
  wxLogMessage(
      "WR_CERT_SAFE_CACHE enabled=%d entries_loaded=%ld entries_used=%ld "
      "entries_ignored=%ld stale_entries_ignored=%ld base_tiles_loaded=%ld "
      "base_tiles_used=%ld cache_file_path=\"%s\" "
      "base_cache_file_path=\"%s\"",
      s_segment_safety_persistent_cache_enabled ? 1 : 0,
      s_segment_safety_persistent_entries_loaded,
      s_segment_safety_persistent_entries_used,
      s_segment_safety_persistent_entries_ignored,
      s_segment_safety_persistent_stale_ignored,
      s_segment_safety_persistent_base_tiles_loaded_count,
      s_segment_safety_persistent_base_tiles_used,
      SegmentSafetyPersistentCachePath(),
      SegmentSafetyPersistentBaseTileCachePath());
  return true;
}

int PlugIn_GetSegmentSafetyPersistentCacheEnabled() {
  return s_segment_safety_persistent_cache_enabled ? 1 : 0;
}

bool PlugIn_SaveSegmentSafetyPersistentCache() {
  return SegmentSafetyPersistentCacheSave();
}

bool PlugIn_ClearSegmentSafetyPersistentCache() {
  wxString path = SegmentSafetyPersistentCachePath();
  wxString base_path = SegmentSafetyPersistentBaseTileCachePath();
  bool removed = true;
  {
    wxMutexLocker lock(s_segment_safety_cache_mutex);
    s_segment_safety_persistent_certified_safe_cache.clear();
    s_segment_safety_persistent_cache_dirty = false;
    s_segment_safety_persistent_entries_loaded = 0;
    s_segment_safety_persistent_entries_saved = 0;
    s_segment_safety_persistent_entries_used = 0;
    s_segment_safety_persistent_entries_ignored = 0;
    s_segment_safety_persistent_stale_ignored = 0;
    s_segment_safety_persistent_malformed_ignored = 0;
    s_segment_safety_persistent_entries_stored = 0;
    s_segment_safety_persistent_cache_loaded = true;
    s_segment_safety_persistent_base_tile_cache.clear();
    s_segment_safety_persistent_base_tiles_dirty = false;
    s_segment_safety_persistent_base_tiles_loaded = true;
    s_segment_safety_persistent_base_tiles_loaded_count = 0;
    s_segment_safety_persistent_base_tiles_saved = 0;
    s_segment_safety_persistent_base_tiles_used = 0;
    s_segment_safety_persistent_base_tiles_ignored = 0;
    s_segment_safety_persistent_tiles_since_checkpoint = 0;
  }
  if (wxFileExists(path)) removed = wxRemoveFile(path);
  if (wxFileExists(base_path)) removed = wxRemoveFile(base_path) && removed;
  wxLogMessage(
      "WR_CERT_SAFE_CACHE clear success=%d cache_file_path=\"%s\" "
      "base_cache_file_path=\"%s\"",
      removed ? 1 : 0, path, base_path);
  return removed;
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
PlugIn_Route::PlugIn_Route() { pWaypointList = new Plugin_WaypointList; }

PlugIn_Route::~PlugIn_Route() {
  pWaypointList->DeleteContents(false);  // do not delete Waypoints
  pWaypointList->Clear();

  delete pWaypointList;
}

//      PlugInTrack implementation
PlugIn_Track::PlugIn_Track() { pWaypointList = new Plugin_WaypointList; }

PlugIn_Track::~PlugIn_Track() {
  pWaypointList->DeleteContents(false);  // do not delete Waypoints
  pWaypointList->Clear();

  delete pWaypointList;
}

wxString GetNewGUID() { return GpxDocument::GetUUID(); }

bool AddCustomWaypointIcon(wxBitmap* pimage, wxString key,
                           wxString description) {
  // Avoid calling waypoint manager until after LateInit()
  if (pWayPointMan == NULL) return false;

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

      dst->m_HyperlinkList->push_back(h);

      linknode = linknode->GetNext();
    }
  }
}

bool AddSingleWaypoint(PlugIn_Waypoint* pwaypoint, bool b_permanent) {
  if (!pWayPointMan) return false;

  //  Validate the waypoint parameters a little bit
  //  GUID
  //  Make sure that this GUID is indeed unique in the Routepoint list
  bool b_unique = true;
  for (RoutePoint* prp : *pWayPointMan->GetWaypointList()) {
    if (prp->m_GUID == pwaypoint->m_GUID) {
      b_unique = false;
      break;
    }
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
  if (!pWayPointMan) return false;

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
  if (!pWayPointMan) return false;

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
      prp->m_HyperlinkList->clear();
      if (pwaypoint->m_HyperlinkList->GetCount() > 0) {
        wxPlugin_HyperlinkListNode* linknode =
            pwaypoint->m_HyperlinkList->GetFirst();
        while (linknode) {
          Plugin_Hyperlink* link = linknode->GetData();

          Hyperlink* h = new Hyperlink();
          h->DescrText = link->DescrText;
          h->Link = link->Link;
          h->LType = link->Type;

          prp->m_HyperlinkList->push_back(h);

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

  if (src->m_HyperlinkList->size() > 0) {
    dst->m_HyperlinkList = new Plugin_HyperlinkList;
    for (Hyperlink* link : *src->m_HyperlinkList) {
      Plugin_Hyperlink* h = new Plugin_Hyperlink();
      h->DescrText = link->DescrText;
      h->Link = link->Link;
      h->Type = link->LType;

      dst->m_HyperlinkList->Append(h);
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

wxArrayString GetWaypointGUIDArray() {
  wxArrayString result;
  if (pWayPointMan) {
    for (RoutePoint* prp : *pWayPointMan->GetWaypointList()) {
      result.Add(prp->m_GUID);
    }
  }
  return result;
}

wxArrayString GetRouteGUIDArray() {
  wxArrayString result;
  for (Route* proute : *pRouteList) {
    result.Add(proute->m_GUID);
  }
  return result;
}

wxArrayString GetTrackGUIDArray() {
  wxArrayString result;
  for (Track* ptrack : g_TrackList) {
    result.Add(ptrack->m_GUID);
  }

  return result;
}

wxArrayString GetWaypointGUIDArray(OBJECT_LAYER_REQ req) {
  wxArrayString result;
  for (RoutePoint* prp : *pWayPointMan->GetWaypointList()) {
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
  }
  return result;
}

wxArrayString GetRouteGUIDArray(OBJECT_LAYER_REQ req) {
  wxArrayString result;

  for (Route* proute : *pRouteList) {
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

wxArrayString GetIconNameArray() {
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

    if (ip == 0) pWP_src = pWP;

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

  pRouteList->push_back(route);

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

o_sound::Sound* g_PluginSound = o_sound::Factory();
static void onPlugInPlaySoundExFinished(void* ptr) {}

// Start playing a sound to a given device and return status to plugin
bool PlugInPlaySoundEx(wxString& sound_file, int deviceIndex) {
  bool ok = g_PluginSound->Load(sound_file, deviceIndex);
  if (!ok) {
    wxLogWarning("Cannot load sound file: %s", sound_file);
    return false;
  }
  auto cmd_sound = dynamic_cast<o_sound::SystemCmdSound*>(g_PluginSound);
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
  for (RoutePoint* src_wp : *route->pRoutePointList) {
    PlugIn_Waypoint* dst_wp = new PlugIn_Waypoint();
    PlugInFromRoutePoint(dst_wp, src_wp);
    dst_route->pWaypointList->Append(dst_wp);
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

bool ShuttingDown() { return g_bquiting; }

wxWindow* GetCanvasUnderMouse() { return gFrame->GetCanvasUnderMouse(); }

int GetCanvasIndexUnderMouse() {
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
  for (Route* proute : *pRouteList) {
    for (RoutePoint* prp : *proute->pRoutePointList) {
      if (prp == pWP) nCount++;
    }
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
  for (Route* proute : *pRouteList) {
    for (RoutePoint* prp : *proute->pRoutePointList) {
      if (prp == pWP) nCount++;
    }
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

    if (src->m_HyperlinkList->size() > 0) {
      dst->m_HyperlinkList = new Plugin_HyperlinkList;

      for (Hyperlink* link : *src->m_HyperlinkList) {
        Plugin_Hyperlink* h = new Plugin_Hyperlink();
        h->DescrText = link->DescrText;
        h->Link = link->Link;
        h->Type = link->LType;
        dst->m_HyperlinkList->Append(h);
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

      dst->m_HyperlinkList->push_back(h);

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
  for (RoutePoint* prp : *pWayPointMan->GetWaypointList()) {
    if (prp->m_GUID == pwaypointex->m_GUID) {
      b_unique = false;
      break;
    }
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
      prp->m_HyperlinkList->clear();
      for (Plugin_Hyperlink* link : *pwaypoint->m_HyperlinkList) {
        Hyperlink* h = new Hyperlink();
        h->DescrText = link->DescrText;
        h->Link = link->Link;
        h->LType = link->Type;
        prp->m_HyperlinkList->push_back(h);
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

// PlugIn_Route_ExV2 utilities

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

  pRouteList->push_back(route);

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

  for (RoutePoint* src_wp : *route->pRoutePointList) {
    PlugIn_Waypoint_ExV2* dst_wp = new PlugIn_Waypoint_ExV2();
    PlugInExV2FromRoutePoint(dst_wp, src_wp);
    dst_route->pWaypointList->Append(dst_wp);
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
PlugIn_Route_Ex::PlugIn_Route_Ex() {
  pWaypointList = new Plugin_WaypointExList;
}

PlugIn_Route_Ex::~PlugIn_Route_Ex() {
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

    if (src->m_HyperlinkList->size() > 0) {
      dst->m_HyperlinkList = new Plugin_HyperlinkList;
      for (Hyperlink* link : *src->m_HyperlinkList) {
        Plugin_Hyperlink* h = new Plugin_Hyperlink();
        h->DescrText = link->DescrText;
        h->Link = link->Link;
        h->Type = link->LType;
        dst->m_HyperlinkList->Append(h);
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

      dst->m_HyperlinkList->push_back(h);

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
  for (RoutePoint* prp : *pWayPointMan->GetWaypointList()) {
    if (prp->m_GUID == pwaypointex->m_GUID) {
      b_unique = false;
      break;
    }
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
      prp->m_HyperlinkList->clear();
      if (pwaypoint->m_HyperlinkList->GetCount() > 0) {
        wxPlugin_HyperlinkListNode* linknode =
            pwaypoint->m_HyperlinkList->GetFirst();
        while (linknode) {
          Plugin_Hyperlink* link = linknode->GetData();

          Hyperlink* h = new Hyperlink();
          h->DescrText = link->DescrText;
          h->Link = link->Link;
          h->LType = link->Type;

          prp->m_HyperlinkList->push_back(h);

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

  pRouteList->push_back(route);

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
  for (RoutePoint* src_wp : *route->pRoutePointList) {
    PlugIn_Waypoint_Ex* dst_wp = new PlugIn_Waypoint_Ex();
    PlugInExFromRoutePoint(dst_wp, src_wp);

    dst_route->pWaypointList->Append(dst_wp);
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
  return (CommBridge::GetInstance().GetPriorityMaps());
}

void UpdateAndApplyPriorityMaps(std::vector<std::string> map) {
  CommBridge::GetInstance().UpdateAndApplyMaps(map);
}

std::vector<std::string> GetActivePriorityIdentifiers() {
  std::vector<std::string> result;

  auto& comm_bridge = CommBridge::GetInstance();

  std::string id = comm_bridge.GetPriorityContainer("position").active_source;
  result.push_back(id);
  id = comm_bridge.GetPriorityContainer("velocity").active_source;
  result.push_back(id);
  id = comm_bridge.GetPriorityContainer("heading").active_source;
  result.push_back(id);
  id = comm_bridge.GetPriorityContainer("variation").active_source;
  result.push_back(id);
  id = comm_bridge.GetPriorityContainer("satellites").active_source;
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
  g_bShowCompassWin = enable;
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
    // Store current locale to detect changes
    wxString oldLocale = g_locale;
    pConfig->Flush();

    // Handle system general configuration options
    pConfig->LoadMyConfigRaw(false);

    // Handle S57 configuration options
    pConfig->LoadS57Config();

    // Handle chart canvas window configuration options
    pConfig->LoadCanvasConfigs(false);
    auto& config_array = ConfigMgr::Get().GetCanvasConfigArray();
    for (auto pcc : config_array) {
      if (pcc && pcc->canvas) {
        pcc->canvas->ApplyCanvasConfig(pcc);
        pcc->canvas->Refresh();
      }
    }

#if wxUSE_XLOCALE
    // Detect and apply locale changes
    if (g_locale != oldLocale && !g_locale.IsEmpty()) {
      wxLogMessage("ConfigFlushAndReload: Locale changed, applying...");
      g_Platform->ChangeLocale(g_locale, plocale_def_lang, &plocale_def_lang);
      ApplyLocale();  // Deactivates/reactivates plugins, rebuilds UI
    }
#endif
  }
}

/**
 * Plugin Notification Framework GUI support
 */
void EnableNotificationCanvasIcon(bool enable) {
  g_CanvasHideNotificationIcon = !enable;
}
