/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  CanvasMenuHandler
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2015 by David S. Register                               *
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

// For compilers that support precompilation, includes "wx.h".
#include <wx/wxprec.h>

#include <wx/aui/aui.h>
#include <wx/clipbrd.h>
#include <wx/dynarray.h>
#include <wx/event.h>
#include <wx/font.h>
#include <wx/gdicmn.h>
#include <wx/graphics.h>
#include <wx/image.h>
#include <wx/listbook.h>
#include <wx/listimpl.cpp>
#include <wx/menu.h>

#include "model/ais_decoder.h"
#include "model/ais_state_vars.h"
#include "model/ais_target_data.h"
#include "model/config_vars.h"
#include "model/cutil.h"
#include "model/georef.h"
#include "model/mDNS_query.h"
#include "model/nav_object_database.h"
#include "model/own_ship.h"
#include "model/own_ship.h"
#include "model/route.h"
#include "model/routeman.h"
#include "model/select.h"
#include "model/track.h"

#include "ais.h"
#include "canvasMenu.h"
#include "chartdb.h"
#include "chcanv.h"
#include "cm93.h"  // for chart outline draw
#include "config.h"
#include "FontMgr.h"
#include "kml.h"
#include "MarkInfo.h"
#include "navutil.h"
#include "ocpn_frame.h"
#include "OCPNPlatform.h"
#include "peer_client_dlg.h"
#include "pluginmanager.h"
#include "Quilt.h"
#include "route_gui.h"
#include "routemanagerdialog.h"
#include "routeman_gui.h"
#include "route_point_gui.h"
#include "RoutePropDlgImpl.h"
#include "s52plib.h"
#include "s57chart.h"  // for ArrayOfS57Obj
#include "SendToGpsDlg.h"
#include "SendToPeerDlg.h"
#include "styles.h"
#include "tcmgr.h"
#include "TCWin.h"
#include "tide_time.h"
#include "track_gui.h"
#include "TrackPropDlg.h"
#include "undo.h"

#ifdef __ANDROID__
#include "androidUTIL.h"
#endif

// ----------------------------------------------------------------------------
// Useful Prototypes
// ----------------------------------------------------------------------------
extern void pupHandler_PasteRoute();
extern void pupHandler_PasteTrack();
extern void pupHandler_PasteWaypoint();

extern Routeman *g_pRouteMan;
extern bool g_bskew_comp;
extern double vLat, vLon;
extern MyFrame *gFrame;
extern ChartGroupArray *g_pGroupArray;
extern PlugInManager *g_pi_manager;
extern int g_nAWMax;
extern int g_nAWDefault;
extern wxString g_AW1GUID;
extern wxString g_AW2GUID;
extern int g_click_stop;
extern RouteManagerDialog *pRouteManagerDialog;
extern MarkInfoDlg *g_pMarkInfoDialog;
extern RoutePropDlgImpl *pRoutePropDialog;
extern ActiveTrack *g_pActiveTrack;
extern bool g_bConfirmObjectDelete;
extern MyConfig *pConfig;
extern OCPNPlatform *g_Platform;

extern CM93OffsetDialog *g_pCM93OffsetDialog;

extern GoToPositionDialog *pGoToPositionDialog;
extern RouteList *pRouteList;
extern wxString g_default_wp_icon;
extern bool g_bBasicMenus;
extern TrackPropDlg *pTrackPropDialog;
extern bool g_FlushNavobjChanges;
extern ColorScheme global_color_scheme;
extern std::vector<std::shared_ptr<ocpn_DNS_record_t>> g_DNS_cache;
extern wxDateTime g_DNS_cache_time;

//    Constants for right click menus
enum {
  ID_DEF_MENU_MAX_DETAIL = 1,
  ID_DEF_MENU_SCALE_IN,
  ID_DEF_MENU_SCALE_OUT,
  ID_DEF_MENU_DROP_WP,
  ID_DEF_MENU_NEW_RT,
  ID_DEF_MENU_QUERY,
  ID_DEF_MENU_MOVE_BOAT_HERE,
  ID_DEF_MENU_GOTO_HERE,
  ID_DEF_MENU_GOTOPOSITION,

  ID_WP_MENU_GOTO,
  ID_WP_MENU_DELPOINT,
  ID_WP_MENU_PROPERTIES,
  ID_RT_MENU_ACTIVATE,
  ID_RT_MENU_DEACTIVATE,
  ID_RT_MENU_INSERT,
  ID_RT_MENU_APPEND,
  ID_RT_MENU_COPY,
  ID_RT_MENU_SPLIT_LEG,
  ID_RT_MENU_SPLIT_WPT,
  ID_TK_MENU_COPY,
  ID_WPT_MENU_COPY,
  ID_WPT_MENU_SENDTOGPS,
  ID_WPT_MENU_SENDTONEWGPS,
  ID_WPT_MENU_SENDTOPEER,
  ID_PASTE_WAYPOINT,
  ID_PASTE_ROUTE,
  ID_PASTE_TRACK,
  ID_RT_MENU_DELETE,
  ID_RT_MENU_REVERSE,
  ID_RT_MENU_DELPOINT,
  ID_RT_MENU_ACTPOINT,
  ID_RT_MENU_DEACTPOINT,
  ID_RT_MENU_ACTNXTPOINT,
  ID_RT_MENU_REMPOINT,
  ID_RT_MENU_PROPERTIES,
  ID_RT_MENU_SENDTOGPS,
  ID_RT_MENU_SENDTONEWGPS,
  ID_RT_MENU_SHOWNAMES,
  ID_RT_MENU_RESEQUENCE,
  ID_RT_MENU_SENDTOPEER,
  ID_WP_MENU_SET_ANCHORWATCH,
  ID_WP_MENU_CLEAR_ANCHORWATCH,
  ID_DEF_MENU_AISTARGETLIST,
  ID_DEF_MENU_AIS_CPAWARNING,

  ID_RC_MENU_SCALE_IN,
  ID_RC_MENU_SCALE_OUT,
  ID_RC_MENU_ZOOM_IN,
  ID_RC_MENU_ZOOM_OUT,
  ID_RC_MENU_FINISH,
  ID_DEF_MENU_AIS_QUERY,
  ID_DEF_MENU_AIS_CPA,
  ID_DEF_MENU_AISSHOWTRACK,
  ID_DEF_MENU_ACTIVATE_MEASURE,
  ID_DEF_MENU_DEACTIVATE_MEASURE,
  ID_DEF_MENU_COPY_MMSI,

  ID_UNDO,
  ID_REDO,

  ID_DEF_MENU_CM93OFFSET_DIALOG,

  ID_TK_MENU_PROPERTIES,
  ID_TK_MENU_DELETE,
  ID_TK_MENU_SENDTOPEER,
  ID_WP_MENU_ADDITIONAL_INFO,

  ID_DEF_MENU_QUILTREMOVE,
  ID_DEF_MENU_COGUP,
  ID_DEF_MENU_NORTHUP,
  ID_DEF_MENU_HEADUP,
  ID_DEF_MENU_TOGGLE_FULL,
  ID_DEF_MENU_TIDEINFO,
  ID_DEF_MENU_CURRENTINFO,
  ID_DEF_ZERO_XTE,

  ID_DEF_MENU_GROUPBASE,  // Must be last entry, as chart group identifiers are
                          // created dynamically

  ID_DEF_MENU_LAST
};

//------------------------------------------------------------------------------
//    CanvasMenuHandler Implementation
//------------------------------------------------------------------------------
int CanvasMenuHandler::GetNextContextMenuId() {
  return ID_DEF_MENU_LAST +
         100;  // Allowing for 100 dynamic menu item identifiers
}

wxFont CanvasMenuHandler::m_scaledFont;

// Define a constructor for my canvas
CanvasMenuHandler::CanvasMenuHandler(ChartCanvas *parentCanvas,
                                     Route *selectedRoute, Track *selectedTrack,
                                     RoutePoint *selectedPoint,
                                     int selectedAIS_MMSI,
                                     void *selectedTCIndex)

{
  parent = parentCanvas;
  m_pSelectedRoute = selectedRoute;
  m_pSelectedTrack = selectedTrack;
  m_pFoundRoutePoint = selectedPoint;
  m_FoundAIS_MMSI = selectedAIS_MMSI;
  m_pIDXCandidate = selectedTCIndex;
  if (!m_scaledFont.IsOk()) {
    wxFont *qFont = GetOCPNScaledFont(_("Menu"));
    m_scaledFont = *qFont;
  }

  m_DIPFactor = g_Platform->GetDisplayDIPMult(gFrame);
}

CanvasMenuHandler::~CanvasMenuHandler() {}

//-------------------------------------------------------------------------------
//          Popup Menu Handling
//-------------------------------------------------------------------------------

void CanvasMenuHandler::PrepareMenuItem(wxMenuItem *item) {
#if defined(__WXMSW__)
  wxColour ctrl_back_color = GetGlobalColor(_T("DILG1"));  // Control Background
  item->SetBackgroundColour(ctrl_back_color);
  wxColour menu_text_color = GetGlobalColor(_T ( "UITX1" ));
  item->SetTextColour(menu_text_color);
#endif
}

void CanvasMenuHandler::MenuPrepend1(wxMenu *menu, int id, wxString label) {
  wxMenuItem *item = new wxMenuItem(menu, id, label);
#if defined(__WXMSW__)
  item->SetFont(m_scaledFont);
#endif

#ifdef __ANDROID__
  wxFont sFont = GetOCPNGUIScaledFont(_("Menu"));
  item->SetFont(sFont);
#endif

  PrepareMenuItem(item);

  if (g_btouch) menu->InsertSeparator(0);
  menu->Prepend(item);
}

void CanvasMenuHandler::MenuAppend1(wxMenu *menu, int id, wxString label) {
  wxMenuItem *item = new wxMenuItem(menu, id, label);
#if defined(__WXMSW__)
  item->SetFont(m_scaledFont);
#endif

#ifdef __ANDROID__
  wxFont sFont = GetOCPNGUIScaledFont(_T("Menu"));
  item->SetFont(sFont);
#endif

  PrepareMenuItem(item);

  menu->Append(item);
  if (g_btouch) menu->AppendSeparator();
}

void CanvasMenuHandler::SetMenuItemFont1(wxMenuItem *item) {
#if defined(__WXMSW__)
  item->SetFont(m_scaledFont);
#endif

#if defined(__ANDROID__)
  wxFont *qFont = GetOCPNScaledFont(_("Menu"));
  item->SetFont(*qFont);
#endif

  PrepareMenuItem(item);
}

void CanvasMenuHandler::CanvasPopupMenu(int x, int y, int seltype) {
  wxMenu *contextMenu = new wxMenu;
  wxMenu *menuWaypoint = NULL;
  wxMenu *menuRoute = NULL;
  wxMenu *menuTrack = NULL;
  wxMenu *menuAIS = NULL;

  wxMenu *subMenuChart = new wxMenu;
  wxMenu *subMenuUndo = new wxMenu("Undo...Ctrl-Z");

#ifdef __WXOSX__
  wxMenu *subMenuRedo = new wxMenu("Redo...Shift-Ctrl-Z");
#else
  wxMenu *subMenuRedo = new wxMenu("Redo...Ctrl-Y");
#endif

  wxMenu *menuFocus = contextMenu;  // This is the one that will be shown

  popx = x;
  popy = y;

  if (!g_bBasicMenus || (seltype != SELTYPE_ROUTECREATE)) {
    bool bsubMenus = false;

    if (bsubMenus) {
      if (parent->undo->AnythingToUndo()) {
        //  Undo SubMenu
        wxMenuItem *subMenuItemundo =
            contextMenu->AppendSubMenu(subMenuUndo, _("Undo"));

        wxString undoItem;
        undoItem << _("Undo") << _T(" ")
                 << parent->undo->GetNextUndoableAction()->Description();
        MenuAppend1(subMenuUndo, ID_UNDO, undoItem);
      }
      if (parent->undo->AnythingToRedo()) {
        //  Redo SubMenu
        wxMenuItem *subMenuItemRedo =
            contextMenu->AppendSubMenu(subMenuRedo, _("Redo"));

        wxString redoItem;
        redoItem << _("Redo") << _T(" ")
                 << parent->undo->GetNextRedoableAction()->Description();
        MenuAppend1(subMenuRedo, ID_REDO, redoItem);
      }
    } else {
      if (parent->undo->AnythingToUndo()) {
        wxString undoItem;
        undoItem << _("Undo") << _T(" ")
                 << parent->undo->GetNextUndoableAction()->Description();
        MenuAppend1(contextMenu, ID_UNDO, _menuText(undoItem, _T("Ctrl-Z")));
      }

      if (parent->undo->AnythingToRedo()) {
        wxString redoItem;
        redoItem << _("Redo") << _T(" ")
                 << parent->undo->GetNextRedoableAction()->Description();
#ifdef __WXOSX__
        MenuAppend1(contextMenu, ID_REDO,
                    _menuText(redoItem, _T("Shift-Ctrl-Z")));
#else
        MenuAppend1(contextMenu, ID_REDO, _menuText(redoItem, _T("Ctrl-Y")));
#endif
      }
    }
  }

  if (seltype == SELTYPE_ROUTECREATE) {
    MenuAppend1(contextMenu, ID_RC_MENU_FINISH,
                _menuText(_("End Route"), _T("Esc")));
  }

  if (!parent->m_pMouseRoute) {
    if (parent->m_bMeasure_Active)
      MenuAppend1(contextMenu, ID_DEF_MENU_DEACTIVATE_MEASURE,
                  _menuText(_("Measure Off"), _T("Esc")));
    else
      MenuAppend1(contextMenu, ID_DEF_MENU_ACTIVATE_MEASURE,
                  _menuText(_("Measure"), _T("M")));
  }

  bool ais_areanotice = false;
  if (g_pAIS && parent->GetShowAIS() && g_bShowAreaNotices) {
    float vp_scale = parent->GetVPScale();

    for (const auto &target : g_pAIS->GetAreaNoticeSourcesList()) {
      auto target_data = target.second;
      if (!target_data->area_notices.empty()) {
        for (auto &ani : target_data->area_notices) {
          Ais8_001_22 &area_notice = ani.second;
          BoundingBox bbox;

          for (Ais8_001_22_SubAreaList::iterator sa =
                   area_notice.sub_areas.begin();
               sa != area_notice.sub_areas.end(); ++sa) {
            switch (sa->shape) {
              case AIS8_001_22_SHAPE_CIRCLE: {
                wxPoint target_point;
                parent->GetCanvasPointPix(sa->latitude, sa->longitude,
                                          &target_point);
                bbox.Expand(target_point);
                if (sa->radius_m > 0.0) bbox.EnLarge(sa->radius_m * vp_scale);
                break;
              }
              case AIS8_001_22_SHAPE_RECT:
              case AIS8_001_22_SHAPE_POLYGON:
              case AIS8_001_22_SHAPE_POLYLINE: {
                double lat = sa->latitude;
                double lon = sa->longitude;
                for (int i = 0; i < 4; ++i) {
                  ll_gc_ll(lat, lon, sa->angles[i], sa->dists_m[i] / 1852.0,
                           &lat, &lon);
                  wxPoint target_point;
                  parent->GetCanvasPointPix(lat, lon, &target_point);
                  bbox.Expand(target_point);
                }
                break;
              }
              case AIS8_001_22_SHAPE_SECTOR: {
                double lat1 = sa->latitude;
                double lon1 = sa->longitude;
                double lat, lon;
                wxPoint target_point;
                parent->GetCanvasPointPix(lat1, lon1, &target_point);
                bbox.Expand(target_point);
                for (int i = 0; i < 18; ++i) {
                  ll_gc_ll(lat1, lon1, sa->left_bound_deg + i * (sa->right_bound_deg - sa->left_bound_deg) / 18 , sa->radius_m / 1852.0,
                         &lat, &lon);
                  parent->GetCanvasPointPix(lat, lon, &target_point);
                  bbox.Expand(target_point);
                }
                ll_gc_ll(lat1, lon1, sa->right_bound_deg , sa->radius_m / 1852.0,
                         &lat, &lon);
                parent->GetCanvasPointPix(lat, lon, &target_point);
                bbox.Expand(target_point);
                break;
              }
            }
          }

          if (bbox.GetValid() && bbox.PointInBox(x, y)) {
            ais_areanotice = true;
            break;
          }
        }
      }
    }
  }

  int nChartStack = 0;
  if (parent->GetpCurrentStack())
    nChartStack = parent->GetpCurrentStack()->nEntry;

  if (!parent->GetVP().b_quilt) {
    if (nChartStack > 1) {
      MenuAppend1(contextMenu, ID_DEF_MENU_MAX_DETAIL, _("Max Detail Here"));
      MenuAppend1(contextMenu, ID_DEF_MENU_SCALE_IN,
                  _menuText(_("Scale In"), _T("Ctrl-Left")));
      MenuAppend1(contextMenu, ID_DEF_MENU_SCALE_OUT,
                  _menuText(_("Scale Out"), _T("Ctrl-Right")));
    }

    if ((parent->m_singleChart &&
         (parent->m_singleChart->GetChartFamily() == CHART_FAMILY_VECTOR)) ||
        ais_areanotice) {
      MenuAppend1(contextMenu, ID_DEF_MENU_QUERY,
                  _("Object Query") + _T( "..." ));
    }

  } else {
    ChartBase *pChartTest =
        parent->m_pQuilt->GetChartAtPix(parent->GetVP(), wxPoint(x, y));
    if ((pChartTest && (pChartTest->GetChartFamily() == CHART_FAMILY_VECTOR)) ||
        ais_areanotice) {
      MenuAppend1(contextMenu, ID_DEF_MENU_QUERY,
                  _("Object Query") + _T( "..." ));
    } else {
#ifndef __ANDROID__
      if (!g_bBasicMenus && (nChartStack > 1)) {
        MenuAppend1(contextMenu, ID_DEF_MENU_SCALE_IN,
                    _menuText(_("Scale In"), _T("Ctrl-Left")));
        MenuAppend1(contextMenu, ID_DEF_MENU_SCALE_OUT,
                    _menuText(_("Scale Out"), _T("Ctrl-Right")));
      }
#endif
    }
  }

  if (!g_bBasicMenus || (seltype != SELTYPE_ROUTECREATE)) {
    bool b_dm_add = true;
    if (g_btouch && parent->IsMeasureActive()) b_dm_add = false;

    if (b_dm_add) {
      MenuAppend1(contextMenu, ID_DEF_MENU_DROP_WP,
                  _menuText(_("Drop Mark"), _T("Ctrl-M")));
      MenuAppend1(contextMenu, ID_DEF_MENU_NEW_RT,
                  _menuText(_("New Route..."), _T("Ctrl-R")));
    }

    if (!bGPSValid)
      MenuAppend1(contextMenu, ID_DEF_MENU_MOVE_BOAT_HERE, _("Move Boat Here"));
  }

  if (!g_bBasicMenus &&
      (!(g_pRouteMan->GetpActiveRoute() || (seltype & SELTYPE_MARKPOINT))))
    MenuAppend1(contextMenu, ID_DEF_MENU_GOTO_HERE, _("Navigate To Here"));

  if (!g_bBasicMenus)
    MenuAppend1(contextMenu, ID_DEF_MENU_GOTOPOSITION,
                _("Center view") + _T("..."));

  if (!g_bBasicMenus) {
    if (parent->GetVP().b_quilt) {
      if (parent->GetUpMode() == NORTH_UP_MODE) {
        MenuAppend1(contextMenu, ID_DEF_MENU_COGUP, _("Course Up Mode"));
        if (!std::isnan(gHdt))
          MenuAppend1(contextMenu, ID_DEF_MENU_HEADUP, _("Heading Up Mode"));
      } else {
        MenuAppend1(contextMenu, ID_DEF_MENU_NORTHUP, _("North Up Mode"));
      }
    } else {
      if (parent->m_singleChart &&
          (fabs(parent->m_singleChart->GetChartSkew()) > .01) && !g_bskew_comp)
        MenuAppend1(contextMenu, ID_DEF_MENU_NORTHUP, _("Chart Up Mode"));
      else
        MenuAppend1(contextMenu, ID_DEF_MENU_NORTHUP, _("North Up Mode"));
    }
  }

  if (!g_bBasicMenus) {
    bool full_toggle_added = false;
#ifndef __ANDROID__
    if (g_btouch) {
      MenuAppend1(contextMenu, ID_DEF_MENU_TOGGLE_FULL,
                  _("Toggle Full Screen"));
      full_toggle_added = true;
    }

    if (!full_toggle_added) {
      // if(gFrame->IsFullScreen())
      MenuAppend1(contextMenu, ID_DEF_MENU_TOGGLE_FULL,
                  _("Toggle Full Screen"));
    }
#endif

    if (g_pRouteMan->IsAnyRouteActive() &&
        g_pRouteMan->GetCurrentXTEToActivePoint() > 0.)
      MenuAppend1(contextMenu, ID_DEF_ZERO_XTE, _("Zero XTE"));

    Kml *kml = new Kml;
    int pasteBuffer = kml->ParsePasteBuffer();
    if (pasteBuffer != KML_PASTE_INVALID) {
      switch (pasteBuffer) {
        case KML_PASTE_WAYPOINT: {
          MenuAppend1(contextMenu, ID_PASTE_WAYPOINT, _("Paste Waypoint"));
          break;
        }
        case KML_PASTE_ROUTE: {
          MenuAppend1(contextMenu, ID_PASTE_ROUTE, _("Paste Route"));
          break;
        }
        case KML_PASTE_TRACK: {
          MenuAppend1(contextMenu, ID_PASTE_TRACK, _("Paste Track"));
          break;
        }
        case KML_PASTE_ROUTE_TRACK: {
          MenuAppend1(contextMenu, ID_PASTE_ROUTE, _("Paste Route"));
          MenuAppend1(contextMenu, ID_PASTE_TRACK, _("Paste Track"));
          break;
        }
      }
    }
    delete kml;

    if (!parent->GetVP().b_quilt && parent->m_singleChart &&
        (parent->m_singleChart->GetChartType() == CHART_TYPE_CM93COMP)) {
      MenuAppend1(contextMenu, ID_DEF_MENU_CM93OFFSET_DIALOG,
                  _("CM93 Offset Dialog..."));
    }

  }  // if( !g_bBasicMenus){

#ifndef __ANDROID__
// TODO stack
//     if( ( parent->GetVP().b_quilt ) && ( pCurrentStack &&
//     pCurrentStack->b_valid ) ) {
//         int dbIndex = parent->m_pQuilt->GetChartdbIndexAtPix(
//         parent->GetVP(), wxPoint( popx, popy ) ); if( dbIndex != -1 )
//             MenuAppend1( contextMenu, ID_DEF_MENU_QUILTREMOVE, _( "Hide This
//             Chart" ) );
//     }
#endif

#ifdef __WXMSW__
  //  If we dismiss the context menu without action, we need to discard some
  //  mouse events.... Eat the next 2 button events, which happen as down-up on
  //  MSW XP
  g_click_stop = 2;
#endif

  //  ChartGroup SubMenu
  wxMenuItem *subItemChart =
      contextMenu->AppendSubMenu(subMenuChart, _("Chart Groups"));
  if (g_btouch) contextMenu->AppendSeparator();

  SetMenuItemFont1(subItemChart);

  if (g_pGroupArray->GetCount()) {
#ifdef __WXMSW__
    MenuAppend1(subMenuChart, wxID_CANCEL, _("temporary"));
#endif
    wxMenuItem *subItem0 = subMenuChart->AppendRadioItem(
        ID_DEF_MENU_GROUPBASE, _("All Active Charts"));

    SetMenuItemFont1(subItem0);

    for (unsigned int i = 0; i < g_pGroupArray->GetCount(); i++) {
      subItem0 = subMenuChart->AppendRadioItem(
          ID_DEF_MENU_GROUPBASE + i + 1, g_pGroupArray->Item(i)->m_group_name);
      SetMenuItemFont1(subItem0);
    }

#ifdef __WXMSW__
    subMenuChart->Remove(wxID_CANCEL);
#endif
    subMenuChart->Check(ID_DEF_MENU_GROUPBASE + parent->m_groupIndex, true);
  }

  //  This is the default context menu
  menuFocus = contextMenu;

  wxString name;
  if (!g_bBasicMenus || (seltype != SELTYPE_ROUTECREATE)) {
    if (g_pAIS) {
      if (parent->GetShowAIS() && (seltype & SELTYPE_AISTARGET)) {
        auto myptarget = g_pAIS->Get_Target_Data_From_MMSI(m_FoundAIS_MMSI);
        if (!g_bBasicMenus && myptarget) {
          name = myptarget->GetFullName();
          if (name.IsEmpty()) name.Printf(_T("%d"), m_FoundAIS_MMSI);
          name.Prepend(_T(" ( ")).Append(_T(" )"));
        } else
          name = wxEmptyString;
        menuAIS = new wxMenu(_("AIS") + name);
        MenuAppend1(menuAIS, ID_DEF_MENU_AIS_QUERY, _("Target Query..."));
        if (myptarget && myptarget->bCPA_Valid &&
            (myptarget->n_alert_state != AIS_ALERT_SET)) {
          if (myptarget->b_show_AIS_CPA)
            MenuAppend1(menuAIS, ID_DEF_MENU_AIS_CPA, _("Hide Target CPA"));
          else
            MenuAppend1(menuAIS, ID_DEF_MENU_AIS_CPA, _("Show Target CPA"));
        }
        MenuAppend1(menuAIS, ID_DEF_MENU_AISTARGETLIST, _("Target List..."));
        if (myptarget->Class != AIS_METEO /*g_bAISShowTracks*/) {
          if (myptarget && !myptarget->b_PersistTrack) {
            if (myptarget->b_show_track)
              MenuAppend1(menuAIS, ID_DEF_MENU_AISSHOWTRACK,
                          _("Hide Target Track"));
            else
              MenuAppend1(menuAIS, ID_DEF_MENU_AISSHOWTRACK,
                          _("Show Target Track"));
          }
        }

        MenuAppend1(menuAIS, ID_DEF_MENU_COPY_MMSI, _("Copy Target MMSI"));
        menuAIS->AppendSeparator();

        if (!parent->GetVP().b_quilt) {
          if ((parent->m_singleChart &&
               (parent->m_singleChart->GetChartFamily() ==
                CHART_FAMILY_VECTOR))) {
            MenuAppend1(menuAIS, ID_DEF_MENU_QUERY, _("Object Query..."));
          }

        } else {
          ChartBase *pChartTest =
              parent->m_pQuilt->GetChartAtPix(parent->GetVP(), wxPoint(x, y));
          if ((pChartTest &&
               (pChartTest->GetChartFamily() == CHART_FAMILY_VECTOR))) {
            MenuAppend1(menuAIS, ID_DEF_MENU_QUERY, _("Object Query..."));
          }
        }

        menuFocus = menuAIS;
      } else {
        MenuAppend1(contextMenu, ID_DEF_MENU_AISTARGETLIST,
                    _("AIS target list") + _T("..."));

        wxString nextCPAstatus = g_bCPAWarn ? _("Hide") : _("Show");
        MenuAppend1(contextMenu, ID_DEF_MENU_AIS_CPAWARNING,
                    _menuText(nextCPAstatus + " " + _("CPA alarm "), "W"));
      }
    }
  }

  if (seltype & SELTYPE_ROUTESEGMENT) {
    if (!g_bBasicMenus && m_pSelectedRoute) {
      name = m_pSelectedRoute->m_RouteNameString;
      if (name.IsEmpty()) name = _("Unnamed Route");
      name.Prepend(_T(" ( ")).Append(_T(" )"));
    } else
      name = wxEmptyString;
    bool blay = false;
    if (m_pSelectedRoute && m_pSelectedRoute->m_bIsInLayer) blay = true;

    if (blay) {
      menuRoute = new wxMenu(_("Layer Route") + name);
      MenuAppend1(menuRoute, ID_RT_MENU_PROPERTIES,
                  _("Properties") + _T( "..." ));
      if (m_pSelectedRoute) {
        if (m_pSelectedRoute->IsActive()) {
          int indexActive = m_pSelectedRoute->GetIndexOf(
              m_pSelectedRoute->m_pRouteActivePoint);
          if ((indexActive + 1) <= m_pSelectedRoute->GetnPoints()) {
            MenuAppend1(menuRoute, ID_RT_MENU_ACTNXTPOINT,
                        _("Activate Next Waypoint"));
          }
          MenuAppend1(menuRoute, ID_RT_MENU_DEACTIVATE, _("Deactivate"));
          MenuAppend1(menuRoute, ID_DEF_ZERO_XTE, _("Zero XTE"));
        } else {
          MenuAppend1(menuRoute, ID_RT_MENU_ACTIVATE, _("Activate"));
        }
      }
    } else {
      menuRoute = new wxMenu(_("Route") + name);
      MenuAppend1(menuRoute, ID_RT_MENU_PROPERTIES,
                  _("Properties") + _T( "..." ));
      if (m_pSelectedRoute) {
        if (m_pSelectedRoute->IsActive()) {
          int indexActive = m_pSelectedRoute->GetIndexOf(
              m_pSelectedRoute->m_pRouteActivePoint);
          if ((indexActive + 1) <= m_pSelectedRoute->GetnPoints()) {
            MenuAppend1(menuRoute, ID_RT_MENU_ACTNXTPOINT,
                        _("Activate Next Waypoint"));
          }
          MenuAppend1(menuRoute, ID_RT_MENU_DEACTIVATE, _("Deactivate"));
          MenuAppend1(menuRoute, ID_DEF_ZERO_XTE, _("Zero XTE"));
        } else {
          MenuAppend1(menuRoute, ID_RT_MENU_ACTIVATE, _("Activate"));
        }
      }
      MenuAppend1(menuRoute, ID_RT_MENU_INSERT, _("Insert Waypoint"));
      MenuAppend1(menuRoute, ID_RT_MENU_APPEND, _("Append Waypoint"));
      if (!(seltype & SELTYPE_ROUTEPOINT) && m_pSelectedRoute) {
        m_SelectedIdx = m_pSelectedRoute->GetIndexOf(m_pFoundRoutePoint);
        if (m_SelectedIdx > 1 &&
            m_SelectedIdx < m_pSelectedRoute->GetnPoints() - 1)
          MenuAppend1(menuRoute, ID_RT_MENU_SPLIT_LEG, _("Split around Leg"));
      }
      MenuAppend1(menuRoute, ID_RT_MENU_COPY, _("Copy as KML") + _T( "..." ));
      MenuAppend1(menuRoute, ID_RT_MENU_DELETE, _("Delete") + _T( "..." ));
      MenuAppend1(menuRoute, ID_RT_MENU_REVERSE, _("Reverse..."));
      if (m_pSelectedRoute) {
        if (m_pSelectedRoute->AreWaypointNamesVisible())
          MenuAppend1(menuRoute, ID_RT_MENU_SHOWNAMES,
                      _("Hide Waypoint Names"));
        else
          MenuAppend1(menuRoute, ID_RT_MENU_SHOWNAMES,
                      _("Show Waypoint Names"));
      }
      MenuAppend1(menuRoute, ID_RT_MENU_RESEQUENCE,
                  _("Resequence Waypoints..."));

      // #ifndef __ANDROID__
      wxString port = parent->FindValidUploadPort();
      parent->m_active_upload_port = port;
      wxString item = _("Send to GPS");
      if (!port.IsEmpty()) {
        item.Append(_T(" ( "));
        item.Append(port);
        item.Append(_T(" )"));
      }
      MenuAppend1(menuRoute, ID_RT_MENU_SENDTOGPS, item);

      if (!port.IsEmpty()) {
        wxString item = _("Send to new GPS");
        MenuAppend1(menuRoute, ID_RT_MENU_SENDTONEWGPS, item);
      }
      // #endif
      wxString itemstp = SYMBOL_STP_TITLE;  // Send to Peer
      MenuAppend1(menuRoute, ID_RT_MENU_SENDTOPEER, itemstp);
    }
    // Eventually set this menu as the "focused context menu"
    if (menuFocus != menuAIS) menuFocus = menuRoute;
  }

  if (seltype & SELTYPE_TRACKSEGMENT) {
    name = wxEmptyString;
    if (!g_bBasicMenus && m_pSelectedTrack)
      name = _T(" ( ") + m_pSelectedTrack->GetName(true) + _T(" )");
    else
      name = wxEmptyString;
    bool blay = false;
    if (m_pSelectedTrack && m_pSelectedTrack->m_bIsInLayer) blay = true;

    if (blay) {
      menuTrack = new wxMenu(_("Layer Track") + name);
      MenuAppend1(menuTrack, ID_TK_MENU_PROPERTIES,
                  _("Properties") + _T( "..." ));
    } else {
      menuTrack = new wxMenu(_("Track") + name);
      MenuAppend1(menuTrack, ID_TK_MENU_PROPERTIES,
                  _("Properties") + _T( "..." ));
      MenuAppend1(menuTrack, ID_TK_MENU_COPY, _("Copy as KML"));
      MenuAppend1(menuTrack, ID_TK_MENU_DELETE, _("Delete") + _T( "..." ));
    }

    wxString itemstp = SYMBOL_STP_TITLE;  // Send to Peer
    MenuAppend1(menuTrack, ID_TK_MENU_SENDTOPEER, itemstp);

    // Eventually set this menu as the "focused context menu"
    if (menuFocus != menuAIS) menuFocus = menuTrack;
  }

  if (seltype & SELTYPE_ROUTEPOINT) {
    if (!g_bBasicMenus && m_pFoundRoutePoint) {
      name = m_pFoundRoutePoint->GetName();
      if (name.IsEmpty()) name = _("Unnamed Waypoint");
      name.Prepend(_T(" ( ")).Append(_T(" )"));
    } else
      name = wxEmptyString;
    bool blay = false;
    if (m_pFoundRoutePoint && m_pFoundRoutePoint->m_bIsInLayer) blay = true;

    if (blay) {
      menuWaypoint = new wxMenu(_("Layer Waypoint") + name);
      MenuAppend1(menuWaypoint, ID_WP_MENU_PROPERTIES,
                  _("Properties") + _T( "..." ));

      if (m_pSelectedRoute && m_pSelectedRoute->IsActive())
        MenuAppend1(menuWaypoint, ID_RT_MENU_ACTPOINT, _("Activate"));
    } else {
      menuWaypoint = new wxMenu(_("Waypoint") + name);
      MenuAppend1(menuWaypoint, ID_WP_MENU_PROPERTIES,
                  _("Properties") + _T( "..." ));
      if (m_pSelectedRoute && m_pSelectedRoute->IsActive()) {
        if (m_pSelectedRoute->m_pRouteActivePoint != m_pFoundRoutePoint)
          MenuAppend1(menuWaypoint, ID_RT_MENU_ACTPOINT, _("Activate"));
      }

      if (m_pSelectedRoute && m_pSelectedRoute->IsActive()) {
        if (m_pSelectedRoute->m_pRouteActivePoint == m_pFoundRoutePoint) {
          int indexActive = m_pSelectedRoute->GetIndexOf(
              m_pSelectedRoute->m_pRouteActivePoint);
          if ((indexActive + 1) <= m_pSelectedRoute->GetnPoints())
            MenuAppend1(menuWaypoint, ID_RT_MENU_ACTNXTPOINT,
                        _("Activate Next Waypoint"));
        }
      }
      if (m_pSelectedRoute && m_pSelectedRoute->GetnPoints() > 2) {
        MenuAppend1(menuWaypoint, ID_RT_MENU_REMPOINT, _("Remove from Route"));

        m_SelectedIdx = m_pSelectedRoute->GetIndexOf(m_pFoundRoutePoint);
        if (m_SelectedIdx > 1 && m_SelectedIdx < m_pSelectedRoute->GetnPoints())
          MenuAppend1(menuWaypoint, ID_RT_MENU_SPLIT_WPT,
                      _("Split Route at Waypoint"));
      }

      MenuAppend1(menuWaypoint, ID_WPT_MENU_COPY, _("Copy as KML"));

      if (m_pFoundRoutePoint && m_pFoundRoutePoint->GetIconName() != _T("mob"))
        MenuAppend1(menuWaypoint, ID_RT_MENU_DELPOINT, _("Delete"));

      // #ifndef __ANDROID__
      wxString port = parent->FindValidUploadPort();
      parent->m_active_upload_port = port;
      wxString item = _("Send to GPS");
      if (!port.IsEmpty()) {
        item.Append(_T(" ( "));
        item.Append(port);
        item.Append(_T(" )"));
      }
      MenuAppend1(menuWaypoint, ID_WPT_MENU_SENDTOGPS, item);

      if (!port.IsEmpty()) {
        wxString item = _("Send to new GPS");
        MenuAppend1(menuWaypoint, ID_WPT_MENU_SENDTONEWGPS, item);
      }

      MenuAppend1(menuWaypoint, ID_WPT_MENU_SENDTOPEER,
                  SYMBOL_STP_TITLE);  // Send to Peer
    }

    // Eventually set this menu as the "focused context menu"
    if (menuFocus != menuAIS) menuFocus = menuWaypoint;
  }

  if (seltype & SELTYPE_MARKPOINT) {
    if (!g_bBasicMenus && m_pFoundRoutePoint) {
      name = m_pFoundRoutePoint->GetName();
      if (name.IsEmpty()) name = _("Unnamed Waypoint");
      name.Prepend(_T(" ( ")).Append(_T(" )"));
    } else
      name = wxEmptyString;
    bool blay = false;
    if (m_pFoundRoutePoint && m_pFoundRoutePoint->m_bIsInLayer) blay = true;

    if (blay) {
      menuWaypoint = new wxMenu(_("Layer Waypoint") + name);
      MenuAppend1(menuWaypoint, ID_WP_MENU_PROPERTIES,
                  _("Properties") + _T( "..." ));
    } else {
      menuWaypoint = new wxMenu(_("Waypoint") + name);
      MenuAppend1(menuWaypoint, ID_WP_MENU_PROPERTIES,
                  _("Properties") + _T( "..." ));

      if (!g_pRouteMan->GetpActiveRoute())
        MenuAppend1(menuWaypoint, ID_WP_MENU_GOTO, _("Navigate To This"));

      MenuAppend1(menuWaypoint, ID_WPT_MENU_COPY, _("Copy as KML"));

      if (m_pFoundRoutePoint && m_pFoundRoutePoint->GetIconName() != _T("mob"))
        MenuAppend1(menuWaypoint, ID_WP_MENU_DELPOINT, _("Delete"));

      // #ifndef __ANDROID__
      wxString port = parent->FindValidUploadPort();
      parent->m_active_upload_port = port;
      wxString item = _("Send to GPS");
      if (!port.IsEmpty()) {
        item.Append(_T(" ( "));
        item.Append(port);
        item.Append(_T(" )"));
      }
      MenuAppend1(menuWaypoint, ID_WPT_MENU_SENDTOGPS, item);

      MenuAppend1(menuWaypoint, ID_WPT_MENU_SENDTOPEER,
                  SYMBOL_STP_TITLE);  // Send to Peer
      // #endif

      if ((m_pFoundRoutePoint == pAnchorWatchPoint1) ||
          (m_pFoundRoutePoint == pAnchorWatchPoint2))
        MenuAppend1(menuWaypoint, ID_WP_MENU_CLEAR_ANCHORWATCH,
                    _("Clear Anchor Watch"));
      else {
        if (m_pFoundRoutePoint && !(m_pFoundRoutePoint->m_bIsInLayer) &&
            ((NULL == pAnchorWatchPoint1) || (NULL == pAnchorWatchPoint2))) {
          double dist;
          double brg;
          DistanceBearingMercator(m_pFoundRoutePoint->m_lat,
                                  m_pFoundRoutePoint->m_lon, gLat, gLon, &brg,
                                  &dist);
          if (dist * 1852. <= g_nAWMax)
            MenuAppend1(menuWaypoint, ID_WP_MENU_SET_ANCHORWATCH,
                        _("Set Anchor Watch"));
        }
      }
    }
    // Eventually set this menu as the "focused context menu"
    if (menuFocus != menuAIS) menuFocus = menuWaypoint;
  }
  /*add the relevant submenus*/
  enum { WPMENU = 1, TKMENU = 2, RTMENU = 4, MMMENU = 8 };
  int sub_menu = 0;
  if (!g_bBasicMenus && menuFocus != contextMenu) {
    if (global_color_scheme != GLOBAL_COLOR_SCHEME_DUSK &&
        global_color_scheme != GLOBAL_COLOR_SCHEME_NIGHT) {
      menuFocus->AppendSeparator();
    }
    wxMenuItem *subMenu1;
    if (menuWaypoint && menuFocus != menuWaypoint) {
      subMenu1 =
          menuFocus->AppendSubMenu(menuWaypoint, menuWaypoint->GetTitle());
      SetMenuItemFont1(subMenu1);
      sub_menu |= WPMENU;
#ifdef __WXMSW__
      menuWaypoint->SetTitle(wxEmptyString);
#endif
    }
    if (menuTrack && menuFocus != menuTrack) {
      subMenu1 = menuFocus->AppendSubMenu(menuTrack, menuTrack->GetTitle());
      SetMenuItemFont1(subMenu1);
      sub_menu |= TKMENU;
#ifdef __WXMSW__
      menuTrack->SetTitle(wxEmptyString);
#endif
    }
    if (menuRoute && menuFocus != menuRoute) {
      subMenu1 = menuFocus->AppendSubMenu(menuRoute, menuRoute->GetTitle());
      SetMenuItemFont1(subMenu1);
      sub_menu |= RTMENU;
#ifdef __WXMSW__
      menuRoute->SetTitle(wxEmptyString);
#endif
    }
    subMenu1 = menuFocus->AppendSubMenu(contextMenu, _("Main Menu"));
    SetMenuItemFont1(subMenu1);
    sub_menu |= MMMENU;
  }

  if (!subMenuChart->GetMenuItemCount()) contextMenu->Destroy(subItemChart);

  //  Add the Tide/Current selections if the item was not activated by shortcut
  //  in right-click handlers
  bool bsep = false;
  if (seltype & SELTYPE_TIDEPOINT) {
    menuFocus->AppendSeparator();
    bsep = true;
    MenuAppend1(menuFocus, ID_DEF_MENU_TIDEINFO, _("Show Tide Information"));
  }

  if (seltype & SELTYPE_CURRENTPOINT) {
    if (!bsep) menuFocus->AppendSeparator();
    MenuAppend1(menuFocus, ID_DEF_MENU_CURRENTINFO,
                _("Show Current Information"));
  }

  // Give the plugins a chance to update their menu items
  g_pi_manager->PrepareAllPluginContextMenus();

  //  Add PlugIn Context Menu items
  ArrayOfPlugInMenuItems item_array =
      g_pi_manager->GetPluginContextMenuItemArray();

  for (unsigned int i = 0; i < item_array.GetCount(); i++) {
    PlugInMenuItemContainer *pimis = item_array[i];
    if (!pimis->b_viz) continue;

    wxMenu *submenu = NULL;
    if (pimis->pmenu_item->GetSubMenu()) {
      submenu = new wxMenu();
      const wxMenuItemList &items =
          pimis->pmenu_item->GetSubMenu()->GetMenuItems();
      for (wxMenuItemList::const_iterator it = items.begin(); it != items.end();
           ++it) {
        int id = -1;
        for (unsigned int j = 0; j < item_array.GetCount(); j++) {
          PlugInMenuItemContainer *pimis = item_array[j];
          if (pimis->pmenu_item == *it) id = pimis->id;
        }

        wxMenuItem *pmi = new wxMenuItem(submenu, id,
#if wxCHECK_VERSION(3, 0, 0)
                                         (*it)->GetItemLabelText(),
#else
                                         (*it)->GetLabel(),
#endif
                                         (*it)->GetHelp(), (*it)->GetKind());

#ifdef __WXMSW__
        pmi->SetFont(m_scaledFont);
#endif

#ifdef __ANDROID__
        wxFont sFont = GetOCPNGUIScaledFont(_("Menu"));
        pmi->SetFont(sFont);
#endif

        PrepareMenuItem(pmi);
        submenu->Append(pmi);
        pmi->Check((*it)->IsChecked());
      }
    }

    wxMenuItem *pmi = new wxMenuItem(contextMenu, pimis->id,
#if wxCHECK_VERSION(3, 0, 0)
                                     pimis->pmenu_item->GetItemLabelText(),
#else
                                     pimis->pmenu_item->GetLabel(),
#endif
                                     pimis->pmenu_item->GetHelp(),
                                     pimis->pmenu_item->GetKind(), submenu);
#ifdef __WXMSW__
    pmi->SetFont(m_scaledFont);
#endif

#ifdef __ANDROID__
    wxFont sFont = GetOCPNGUIScaledFont(_("Menu"));
    pmi->SetFont(sFont);
#endif

    PrepareMenuItem(pmi);

    wxMenu *dst = contextMenu;
    if (pimis->m_in_menu == "Waypoint")
      dst = menuWaypoint;
    else if (pimis->m_in_menu == "Route")
      dst = menuRoute;
    else if (pimis->m_in_menu == "Track")
      dst = menuTrack;
    else if (pimis->m_in_menu == "AIS")
      dst = menuAIS;

    if (dst != NULL) {
      dst->Append(pmi);
      dst->Enable(pimis->id, !pimis->b_grey);
    }
  }

  //        Invoke the correct focused drop-down menu

#ifdef __ANDROID__
  androidEnableBackButton(false);
  androidEnableOptionsMenu(false);

  setMenuStyleSheet(menuRoute, GetOCPNGUIScaledFont(_T("Menu")));
  setMenuStyleSheet(menuWaypoint, GetOCPNGUIScaledFont(_T("Menu")));
  setMenuStyleSheet(menuTrack, GetOCPNGUIScaledFont(_T("Menu")));
  setMenuStyleSheet(menuAIS, GetOCPNGUIScaledFont(_T("Menu")));
#endif

  parent->PopupMenu(menuFocus, x, y);

#ifdef __ANDROID__
  androidEnableBackButton(true);
  androidEnableOptionsMenu(true);
#endif

  /* Cleanup if necessary.
  Do not delete menus witch are submenu as they will be deleted by their parent
  menu. This could create a crash*/
  delete menuAIS;
  if (!(sub_menu & MMMENU)) delete contextMenu;
  if (!(sub_menu & RTMENU)) delete menuRoute;
  if (!(sub_menu & TKMENU)) delete menuTrack;
  if (!(sub_menu & WPMENU)) delete menuWaypoint;
}

void CanvasMenuHandler::PopupMenuHandler(wxCommandEvent &event) {
  RoutePoint *pLast;

  wxPoint r;
  double zlat, zlon;

  int splitMode = 0;  // variables for split
  bool dupFirstWpt = true, showRPD;

  parent->GetCanvasPixPoint(popx * parent->GetDisplayScale(),
                            popy * parent->GetDisplayScale(), zlat, zlon);

  switch (event.GetId()) {
    case ID_DEF_MENU_MAX_DETAIL:
      vLat = zlat;
      vLon = zlon;
      parent->ClearbFollow();

      parent->parent_frame->DoChartUpdate();

      parent->SelectChartFromStack(0, false, CHART_TYPE_DONTCARE,
                                   CHART_FAMILY_RASTER);
      break;

    case ID_DEF_MENU_SCALE_IN:
      parent->DoCanvasStackDelta(-1);
      break;

    case ID_DEF_MENU_SCALE_OUT:
      parent->DoCanvasStackDelta(1);
      break;

    case ID_UNDO:
      parent->undo->UndoLastAction();
      parent->InvalidateGL();
      parent->Refresh(false);
      break;

    case ID_REDO:
      parent->undo->RedoNextAction();
      parent->InvalidateGL();
      parent->Refresh(false);
      break;

    case ID_DEF_MENU_MOVE_BOAT_HERE:
      gLat = zlat;
      gLon = zlon;
      gFrame->UpdateStatusBar();
      break;

    case ID_DEF_MENU_GOTO_HERE: {
      RoutePoint *pWP_dest = new RoutePoint(zlat, zlon, g_default_wp_icon,
                                            wxEmptyString, wxEmptyString);
      pSelect->AddSelectableRoutePoint(zlat, zlon, pWP_dest);

      RoutePoint *pWP_src = new RoutePoint(gLat, gLon, g_default_wp_icon,
                                           wxEmptyString, wxEmptyString);
      pSelect->AddSelectableRoutePoint(gLat, gLon, pWP_src);

      Route *temp_route = new Route();
      pRouteList->Append(temp_route);

      temp_route->AddPoint(pWP_src);
      temp_route->AddPoint(pWP_dest);

      pSelect->AddSelectableRouteSegment(gLat, gLon, zlat, zlon, pWP_src,
                                         pWP_dest, temp_route);

      temp_route->m_RouteNameString = _("Temporary GOTO Route");
      temp_route->m_RouteStartString = _("Here");
      ;
      temp_route->m_RouteEndString = _("There");

      temp_route->m_bDeleteOnArrival = true;

      if (g_pRouteMan->GetpActiveRoute()) g_pRouteMan->DeactivateRoute();

      g_pRouteMan->ActivateRoute(temp_route, pWP_dest);

      break;
    }

    case ID_DEF_MENU_DROP_WP: {
      RoutePoint *pWP = new RoutePoint(zlat, zlon, g_default_wp_icon,
                                       wxEmptyString, wxEmptyString);
      pWP->m_bIsolatedMark = true;  // This is an isolated mark
      pSelect->AddSelectableRoutePoint(zlat, zlon, pWP);
      pConfig->AddNewWayPoint(pWP, -1);  // use auto next num
      if (!RoutePointGui(*pWP).IsVisibleSelectable(this->parent))
        RoutePointGui(*pWP).ShowScaleWarningMessage(parent);

      if (RouteManagerDialog::getInstanceFlag()) {
        if (pRouteManagerDialog && pRouteManagerDialog->IsShown()) {
          pRouteManagerDialog->UpdateWptListCtrl();
        }
      }

      parent->undo->BeforeUndoableAction(Undo_CreateWaypoint, pWP,
                                         Undo_HasParent, NULL);
      parent->undo->AfterUndoableAction(NULL);
      gFrame->RefreshAllCanvas(false);
      gFrame->InvalidateAllGL();
      g_FlushNavobjChanges = true;
      break;
    }

    case ID_DEF_MENU_NEW_RT: {
      parent->StartRoute();
      break;
    }

    case ID_DEF_MENU_AISTARGETLIST:
      parent->ShowAISTargetList();
      break;

    case ID_DEF_MENU_AIS_CPAWARNING:
      parent->ToggleCPAWarn();
      break;

    case ID_WP_MENU_GOTO: {
      RoutePoint *pWP_src = new RoutePoint(gLat, gLon, g_default_wp_icon,
                                           wxEmptyString, wxEmptyString);
      pSelect->AddSelectableRoutePoint(gLat, gLon, pWP_src);

      Route *temp_route = new Route();
      pRouteList->Append(temp_route);

      temp_route->AddPoint(pWP_src);
      temp_route->AddPoint(m_pFoundRoutePoint);
      m_pFoundRoutePoint->SetShared(true);

      pSelect->AddSelectableRouteSegment(gLat, gLon, m_pFoundRoutePoint->m_lat,
                                         m_pFoundRoutePoint->m_lon, pWP_src,
                                         m_pFoundRoutePoint, temp_route);

      wxString name = m_pFoundRoutePoint->GetName();
      if (name.IsEmpty()) name = _("(Unnamed Waypoint)");
      wxString rteName = _("Go to ");
      rteName.Append(name);
      temp_route->m_RouteNameString = rteName;
      temp_route->m_RouteStartString = _("Here");
      ;
      temp_route->m_RouteEndString = name;
      temp_route->m_bDeleteOnArrival = true;

      if (g_pRouteMan->GetpActiveRoute()) g_pRouteMan->DeactivateRoute();

      g_pRouteMan->ActivateRoute(temp_route, m_pFoundRoutePoint);

      break;
    }

    case ID_DEF_MENU_COGUP:
      parent->SetUpMode(COURSE_UP_MODE);
      break;

    case ID_DEF_MENU_HEADUP:
      parent->SetUpMode(HEAD_UP_MODE);
      break;

    case ID_DEF_MENU_NORTHUP:
      parent->SetUpMode(NORTH_UP_MODE);
      break;

    case ID_DEF_MENU_TOGGLE_FULL:
      gFrame->ToggleFullScreen();
      break;

    case ID_DEF_MENU_GOTOPOSITION:
      if (NULL == pGoToPositionDialog)  // There is one global instance of the
                                        // Go To Position Dialog
        pGoToPositionDialog = new GoToPositionDialog(parent);
      pGoToPositionDialog->SetCanvas(parent);
      pGoToPositionDialog->CheckPasteBufferForPosition();
      pGoToPositionDialog->Show();
      break;

    case ID_WP_MENU_DELPOINT: {
      if (m_pFoundRoutePoint == pAnchorWatchPoint1) {
        pAnchorWatchPoint1 = NULL;
        g_AW1GUID.Clear();
      } else if (m_pFoundRoutePoint == pAnchorWatchPoint2) {
        pAnchorWatchPoint2 = NULL;
        g_AW2GUID.Clear();
      }

      if (m_pFoundRoutePoint && !(m_pFoundRoutePoint->m_bIsInLayer) &&
          (m_pFoundRoutePoint->GetIconName() != _T("mob"))) {
        // If the WP belongs to an invisible route, we come here instead of to
        // ID_RT_MENU_DELPOINT
        //  Check it, and if so then remove the point from its routes
        wxArrayPtrVoid *proute_array =
            g_pRouteMan->GetRouteArrayContaining(m_pFoundRoutePoint);
        if (proute_array) {
          pWayPointMan->DestroyWaypoint(m_pFoundRoutePoint);
        } else {
          parent->undo->BeforeUndoableAction(
              Undo_DeleteWaypoint, m_pFoundRoutePoint, Undo_IsOrphanded,
              NULL /*m_pFoundPoint*/);
          pConfig->DeleteWayPoint(m_pFoundRoutePoint);
          pSelect->DeleteSelectablePoint(m_pFoundRoutePoint,
                                         SELTYPE_ROUTEPOINT);
          if (NULL != pWayPointMan)
            pWayPointMan->RemoveRoutePoint(m_pFoundRoutePoint);
          parent->undo->AfterUndoableAction(NULL);
        }

        if (g_pMarkInfoDialog) {
          g_pMarkInfoDialog->ClearData();
        }

        if (RouteManagerDialog::getInstanceFlag()) {
          if (pRouteManagerDialog) {
            if (pRouteManagerDialog->IsShown())
              pRouteManagerDialog->UpdateWptListCtrl();
          }
        }

        gFrame->RefreshAllCanvas(false);
        gFrame->InvalidateAllGL();
      }
      break;
    }
    case ID_WP_MENU_PROPERTIES:
      parent->ShowMarkPropertiesDialog(m_pFoundRoutePoint);
      break;

    case ID_WP_MENU_CLEAR_ANCHORWATCH: {
      wxString guid = wxEmptyString;
      if (pAnchorWatchPoint1 == m_pFoundRoutePoint) {
        pAnchorWatchPoint1 = NULL;
        guid = g_AW1GUID;
        g_AW1GUID.Clear();
      } else if (pAnchorWatchPoint2 == m_pFoundRoutePoint) {
        pAnchorWatchPoint2 = NULL;
        guid = g_AW2GUID;
        g_AW2GUID.Clear();
      }
      if (!guid.IsEmpty()) {
        wxJSONValue v;
        v[_T("GUID")] = guid;
        wxString msg_id(_T("OCPN_ANCHOR_WATCH_CLEARED"));
        g_pi_manager->SendJSONMessageToAllPlugins(msg_id, v);
      }
      break;
    }

    case ID_WP_MENU_SET_ANCHORWATCH: {
      wxString guid = wxEmptyString;
      if (pAnchorWatchPoint1 == NULL) {
        pAnchorWatchPoint1 = m_pFoundRoutePoint;
        g_AW1GUID = pAnchorWatchPoint1->m_GUID;
        guid = g_AW1GUID;
        wxString nn;
        nn = m_pFoundRoutePoint->GetName();
        if (nn.IsNull()) {
          nn.Printf(_T("%d m"), g_nAWDefault);
          m_pFoundRoutePoint->SetName(nn);
        }
      } else if (pAnchorWatchPoint2 == NULL) {
        pAnchorWatchPoint2 = m_pFoundRoutePoint;
        g_AW2GUID = pAnchorWatchPoint2->m_GUID;
        guid = g_AW2GUID;
        wxString nn;
        nn = m_pFoundRoutePoint->GetName();
        if (nn.IsNull()) {
          nn.Printf(_T("%d m"), g_nAWDefault);
          m_pFoundRoutePoint->SetName(nn);
        }
      }
      if (!guid.IsEmpty()) {
        wxJSONValue v;
        v[_T("GUID")] = guid;
        wxString msg_id(_T("OCPN_ANCHOR_WATCH_SET"));
        g_pi_manager->SendJSONMessageToAllPlugins(msg_id, v);
      }
      break;
    }

    case ID_DEF_MENU_ACTIVATE_MEASURE:
      parent->StartMeasureRoute();
      break;

    case ID_DEF_MENU_DEACTIVATE_MEASURE:
      parent->CancelMeasureRoute();
      //gFrame->SurfaceAllCanvasToolbars();
      parent->InvalidateGL();
      parent->Refresh(false);
      break;

    case ID_DEF_MENU_CM93OFFSET_DIALOG: {
      if (NULL == g_pCM93OffsetDialog) {
        g_pCM93OffsetDialog = new CM93OffsetDialog(parent->parent_frame);
      }

      cm93compchart *pch = NULL;
      if (!parent->GetVP().b_quilt && parent->m_singleChart &&
          (parent->m_singleChart->GetChartType() == CHART_TYPE_CM93COMP)) {
        pch = (cm93compchart *)parent->m_singleChart;
      }

      if (g_pCM93OffsetDialog) {
        g_pCM93OffsetDialog->SetCM93Chart(pch);
        g_pCM93OffsetDialog->Show();
        g_pCM93OffsetDialog->UpdateMCOVRList(parent->GetVP());
      }

      break;
    }
    case ID_DEF_MENU_QUERY: {
      parent->ShowObjectQueryWindow(popx, popy, zlat, zlon);
      break;
    }
    case ID_DEF_MENU_AIS_QUERY: {
      wxWindow *pwin = wxDynamicCast(parent, wxWindow);
      ShowAISTargetQueryDialog(pwin, m_FoundAIS_MMSI);
      break;
    }

    case ID_DEF_MENU_AIS_CPA: {
      auto myptarget = g_pAIS->Get_Target_Data_From_MMSI(m_FoundAIS_MMSI);
      if (myptarget) myptarget->Toggle_AIS_CPA();
      break;
    }

    case ID_DEF_MENU_AISSHOWTRACK: {
      auto myptarget = g_pAIS->Get_Target_Data_From_MMSI(m_FoundAIS_MMSI);
      if (myptarget) myptarget->ToggleShowTrack();
      break;
    }

    case ID_DEF_MENU_COPY_MMSI: {
      // Write MMSI # as text to the clipboard
      if (wxTheClipboard->Open()) {
        wxTheClipboard->SetData(new wxTextDataObject(
            wxString::Format(wxT("%09d"), m_FoundAIS_MMSI)));
        wxTheClipboard->Close();
      }
      break;
    }

    case ID_DEF_MENU_QUILTREMOVE: {
      if (parent->GetVP().b_quilt) {
        int dbIndex = parent->m_pQuilt->GetChartdbIndexAtPix(
            parent->GetVP(), wxPoint(popx, popy));
        parent->RemoveChartFromQuilt(dbIndex);

        parent->ReloadVP();
      }

      break;
    }

    case ID_DEF_MENU_CURRENTINFO: {
      parent->DrawTCWindow(popx, popy, (void *)m_pIDXCandidate);
      parent->Refresh(false);

      break;
    }

    case ID_DEF_MENU_TIDEINFO: {
      parent->DrawTCWindow(popx, popy, (void *)m_pIDXCandidate);
      parent->Refresh(false);

      break;
    }
    case ID_RT_MENU_REVERSE: {
      if (m_pSelectedRoute->m_bIsInLayer) break;

      int ask_return =
          OCPNMessageBox(parent, g_pRouteMan->GetRouteReverseMessage(),
                         _("Rename Waypoints?"), wxYES_NO | wxCANCEL);

      if (ask_return != wxID_CANCEL) {
        pSelect->DeleteAllSelectableRouteSegments(m_pSelectedRoute);
        m_pSelectedRoute->Reverse(ask_return == wxID_YES);
        pSelect->AddAllSelectableRouteSegments(m_pSelectedRoute);

        pConfig->UpdateRoute(m_pSelectedRoute);

        if (pRoutePropDialog && (pRoutePropDialog->IsShown())) {
          pRoutePropDialog->SetRouteAndUpdate(m_pSelectedRoute);
          // pNew->UpdateProperties();
        }
        gFrame->InvalidateAllGL();
        gFrame->RefreshAllCanvas();
      }
      break;
    }

    case ID_RT_MENU_SHOWNAMES: {
      if (m_pSelectedRoute) {
        m_pSelectedRoute->ShowWaypointNames(
            !m_pSelectedRoute->AreWaypointNamesVisible());
      }

      break;
    }

    case ID_RT_MENU_RESEQUENCE: {
      if (m_pSelectedRoute) {
        if (m_pSelectedRoute->m_bIsInLayer) break;

        int ask_return =
            OCPNMessageBox(parent, g_pRouteMan->GetRouteResequenceMessage(),
                           _("Rename Waypoints?"), wxYES_NO | wxCANCEL);

        if (ask_return != wxID_CANCEL) {
          m_pSelectedRoute->RenameRoutePoints();
        }

        gFrame->InvalidateAllGL();
        gFrame->RefreshAllCanvas();
      }

      break;
    }

    case ID_RT_MENU_DELETE: {
      int dlg_return = wxID_YES;
      if (g_bConfirmObjectDelete) {
        dlg_return = OCPNMessageBox(
            parent, _("Are you sure you want to delete this route?"),
            _("OpenCPN Route Delete"),
            (long)wxYES_NO | wxCANCEL | wxYES_DEFAULT);
      }

      if (dlg_return == wxID_YES) {
        if (g_pRouteMan->GetpActiveRoute() == m_pSelectedRoute)
          g_pRouteMan->DeactivateRoute();

        if (m_pSelectedRoute->m_bIsInLayer) break;

        if (!g_pRouteMan->DeleteRoute(m_pSelectedRoute,
                                      NavObjectChanges::getInstance()))
          break;

        if (RouteManagerDialog::getInstanceFlag()) {
          if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
            pRouteManagerDialog->UpdateRouteListCtrl();
        }

        if (g_pMarkInfoDialog && g_pMarkInfoDialog->IsShown()) {
          g_pMarkInfoDialog->ValidateMark();
          g_pMarkInfoDialog->UpdateProperties();
        }

        parent->undo->InvalidateUndo();

        gFrame->InvalidateAllGL();
        gFrame->RefreshAllCanvas();
      }
      break;
    }

    case ID_RT_MENU_ACTIVATE: {
      if (g_pRouteMan->GetpActiveRoute()) g_pRouteMan->DeactivateRoute();

      //  If this is an auto-created MOB route, always select the second point
      //  (the MOB)
      // as the destination.
      RoutePoint *best_point;
      if (m_pSelectedRoute) {
        if (wxNOT_FOUND ==
            m_pSelectedRoute->m_RouteNameString.Find(_T("MOB"))) {
          best_point = g_pRouteMan->FindBestActivatePoint(
              m_pSelectedRoute, gLat, gLon, gCog, gSog);
        } else
          best_point = m_pSelectedRoute->GetPoint(2);

        g_pRouteMan->ActivateRoute(m_pSelectedRoute, best_point);
        m_pSelectedRoute->m_bRtIsSelected = false;
      }

      break;
    }

    case ID_RT_MENU_DEACTIVATE:
      g_pRouteMan->DeactivateRoute();
      m_pSelectedRoute->m_bRtIsSelected = false;

      break;

    case ID_RT_MENU_INSERT: {
      if (m_pSelectedRoute->m_bIsInLayer) break;
      bool rename = false;
      m_pSelectedRoute->InsertPointAfter(m_pFoundRoutePoint, zlat, zlon,
                                         rename);

      pSelect->DeleteAllSelectableRoutePoints(m_pSelectedRoute);
      pSelect->DeleteAllSelectableRouteSegments(m_pSelectedRoute);

      pSelect->AddAllSelectableRouteSegments(m_pSelectedRoute);
      pSelect->AddAllSelectableRoutePoints(m_pSelectedRoute);

      //    As a special case (which comes up often)...
      //    If the inserted waypoint is on the active leg of an active route
      /*            if(m_pSelectedRoute->m_bRtIsActive)
       {
       if(m_pSelectedRoute->m_nRouteActivePoint == np + 1)
       {
       pNew_Point = m_pSelectedRoute->GetPoint(np + 2);
       pRouteMan->ActivateRoutePoint(m_pSelectedRoute, pNew_Point);
       }
       }
       */
      pConfig->UpdateRoute(m_pSelectedRoute);

      if (pRoutePropDialog && (pRoutePropDialog->IsShown())) {
        pRoutePropDialog->SetRouteAndUpdate(m_pSelectedRoute, true);
      }

      break;
    }

    case ID_RT_MENU_APPEND:

      if (m_pSelectedRoute->m_bIsInLayer) break;

      parent->m_pMouseRoute = m_pSelectedRoute;
      parent->m_routeState = m_pSelectedRoute->GetnPoints() + 1;
      parent->m_pMouseRoute->m_lastMousePointIndex =
          m_pSelectedRoute->GetnPoints();
      parent->m_pMouseRoute->SetHiLite(50);

      pLast = m_pSelectedRoute->GetLastPoint();

      parent->m_prev_rlat = pLast->m_lat;
      parent->m_prev_rlon = pLast->m_lon;
      parent->m_prev_pMousePoint = pLast;

      parent->m_bAppendingRoute = true;

      parent->SetCursor(*parent->pCursorPencil);
#ifdef __ANDROID__
      androidSetRouteAnnunciator(true);
#endif

      parent->HideGlobalToolbar();

      break;

    case ID_RT_MENU_SPLIT_LEG:  // split route around a leg
      splitMode++;
      dupFirstWpt = false;
    case ID_RT_MENU_SPLIT_WPT:  // split route at a wpt

      showRPD = (pRoutePropDialog && pRoutePropDialog->IsShown());

      if (g_pRouteMan->GetpActiveRoute() == m_pSelectedRoute)
        g_pRouteMan->DeactivateRoute();

      m_pHead = new Route();
      m_pTail = new Route();
      m_pHead->CloneRoute(m_pSelectedRoute, 1, m_SelectedIdx, _("_A"));
      m_pTail->CloneRoute(m_pSelectedRoute, m_SelectedIdx + splitMode,
                          m_pSelectedRoute->GetnPoints(), _("_B"), dupFirstWpt);
      pRouteList->Append(m_pHead);
      pConfig->AddNewRoute(m_pHead);

      pRouteList->Append(m_pTail);
      pConfig->AddNewRoute(m_pTail);

      pConfig->DeleteConfigRoute(m_pSelectedRoute);

      pSelect->DeleteAllSelectableRoutePoints(m_pSelectedRoute);
      pSelect->DeleteAllSelectableRouteSegments(m_pSelectedRoute);
      g_pRouteMan->DeleteRoute(m_pSelectedRoute,
                               NavObjectChanges::getInstance());
      pSelect->AddAllSelectableRouteSegments(m_pTail);
      pSelect->AddAllSelectableRoutePoints(m_pTail);
      pSelect->AddAllSelectableRouteSegments(m_pHead);
      pSelect->AddAllSelectableRoutePoints(m_pHead);

      if (showRPD) {
        pRoutePropDialog->SetRouteAndUpdate(m_pHead);
        pRoutePropDialog->Show();
      }
      if (RouteManagerDialog::getInstanceFlag() && pRouteManagerDialog &&
          (pRouteManagerDialog->IsShown()))
        pRouteManagerDialog->UpdateRouteListCtrl();
      break;

    case ID_RT_MENU_COPY:
      if (m_pSelectedRoute) Kml::CopyRouteToClipboard(m_pSelectedRoute);
      break;

    case ID_TK_MENU_COPY:
      if (m_pSelectedTrack) Kml::CopyTrackToClipboard(m_pSelectedTrack);
      break;

    case ID_WPT_MENU_COPY:
      if (m_pFoundRoutePoint) Kml::CopyWaypointToClipboard(m_pFoundRoutePoint);
      break;

    case ID_WPT_MENU_SENDTOGPS:
      if (m_pFoundRoutePoint) {
        if (parent->m_active_upload_port.Length())
          RoutePointGui(*m_pFoundRoutePoint)
              .SendToGPS(parent->m_active_upload_port.BeforeFirst(' '), NULL);
        else {
          SendToGpsDlg dlg;
          dlg.SetWaypoint(m_pFoundRoutePoint);
          wxFont fo = GetOCPNGUIScaledFont(_T("Dialog"));
          dlg.SetFont(fo);

          dlg.Create(NULL, -1, _("Send to GPS") + _T( "..." ), _T(""));
          dlg.ShowModal();
        }
      }
      break;

    case ID_WPT_MENU_SENDTONEWGPS:
      if (m_pFoundRoutePoint) {
        SendToGpsDlg dlg;
        dlg.SetWaypoint(m_pFoundRoutePoint);

        dlg.Create(NULL, -1, _("Send to GPS") + _T( "..." ), _T(""));
        dlg.ShowModal();
      }
      break;

    case ID_WPT_MENU_SENDTOPEER:
      if (m_pFoundRoutePoint) {
        SendToPeerDlg dlg;
        dlg.SetWaypoint(m_pFoundRoutePoint);

        // Perform initial scan, if necessary

        // Check for stale cache...
        bool bDNScacheStale = true;
        wxDateTime tnow = wxDateTime::Now();
        if (g_DNS_cache_time.IsValid()) {
          wxTimeSpan delta = tnow.Subtract(g_DNS_cache_time);
          if (delta.GetMinutes() < 5) bDNScacheStale = false;
        }

        if ((g_DNS_cache.size() == 0) || bDNScacheStale)
          dlg.SetScanOnCreate(true);

        dlg.SetScanTime(5);  // seconds
        dlg.Create(NULL, -1, _("Send Waypoint to OpenCPN Peer") + _T( "..." ),
                   _T(""));
        dlg.ShowModal();
      }
      break;

    case ID_RT_MENU_SENDTOGPS:
      if (m_pSelectedRoute) {
        if (parent->m_active_upload_port.Length())
          RouteGui(*m_pSelectedRoute)
              .SendToGPS(parent->m_active_upload_port.BeforeFirst(' '), true,
                         NULL);
        else {
          SendToGpsDlg dlg;
          dlg.SetRoute(m_pSelectedRoute);

          dlg.Create(NULL, -1, _("Send to GPS") + _T( "..." ), _T(""));
          dlg.ShowModal();
        }
      }
      break;

    case ID_RT_MENU_SENDTONEWGPS:
      if (m_pSelectedRoute) {
        SendToGpsDlg dlg;
        dlg.SetRoute(m_pSelectedRoute);

        dlg.Create(NULL, -1, _("Send to GPS") + _T( "..." ), _T(""));
        dlg.ShowModal();
      }
      break;

    case ID_RT_MENU_SENDTOPEER:
      if (m_pSelectedRoute) {
        SendToPeerDlg dlg;
        dlg.SetRoute(m_pSelectedRoute);

        // Perform initial scan, if necessary

        // Check for stale cache...
        bool bDNScacheStale = true;
        wxDateTime tnow = wxDateTime::Now();
        if (g_DNS_cache_time.IsValid()) {
          wxTimeSpan delta = tnow.Subtract(g_DNS_cache_time);
          if (delta.GetMinutes() < 5) bDNScacheStale = false;
        }

        if ((g_DNS_cache.size() == 0) || bDNScacheStale)
          dlg.SetScanOnCreate(true);

        dlg.SetScanTime(5);  // seconds
        dlg.Create(NULL, -1, _("Send Route to OpenCPN Peer") + _T( "..." ),
                   _T(""));
        dlg.ShowModal();
      }
      break;

    case ID_PASTE_WAYPOINT:
      pupHandler_PasteWaypoint();
      break;

    case ID_PASTE_ROUTE:
      pupHandler_PasteRoute();
      break;

    case ID_PASTE_TRACK:
      pupHandler_PasteTrack();
      break;

    case ID_RT_MENU_DELPOINT:
      if (m_pSelectedRoute) {
        if (m_pSelectedRoute->m_bIsInLayer) break;

        pWayPointMan->DestroyWaypoint(m_pFoundRoutePoint);

        if (pRoutePropDialog && (pRoutePropDialog->IsShown())) {
          //    Selected route may have been deleted as one-point route, so
          //    check it
          if (g_pRouteMan->IsRouteValid(m_pSelectedRoute)) {
            pRoutePropDialog->SetRouteAndUpdate(m_pSelectedRoute, true);
          } else
            pRoutePropDialog->Hide();
        }

        if (RouteManagerDialog::getInstanceFlag()) {
          if (pRouteManagerDialog && pRouteManagerDialog->IsShown()) {
            pRouteManagerDialog->UpdateWptListCtrl();
            pRouteManagerDialog->UpdateRouteListCtrl();
          }
        }

        gFrame->InvalidateAllGL();
        gFrame->RefreshAllCanvas(true);
      }

      break;

    case ID_RT_MENU_REMPOINT:
      if (m_pSelectedRoute) {
        if (m_pSelectedRoute->m_bIsInLayer) break;
        g_pRouteMan->RemovePointFromRoute(m_pFoundRoutePoint, m_pSelectedRoute,
                                          parent->m_routeState);
        gFrame->InvalidateAllGL();
        gFrame->RefreshAllCanvas();
      }
      break;

    case ID_RT_MENU_ACTPOINT:
      if (g_pRouteMan->GetpActiveRoute() == m_pSelectedRoute) {
        g_pRouteMan->ActivateRoutePoint(m_pSelectedRoute, m_pFoundRoutePoint);
        m_pSelectedRoute->m_bRtIsSelected = false;
      }

      break;

    case ID_RT_MENU_DEACTPOINT:
      break;

    case ID_RT_MENU_ACTNXTPOINT:
      if (g_pRouteMan->GetpActiveRoute() == m_pSelectedRoute) {
        g_pRouteMan->ActivateNextPoint(m_pSelectedRoute, true);
        m_pSelectedRoute->m_bRtIsSelected = false;
      }

      break;

    case ID_RT_MENU_PROPERTIES: {
      parent->ShowRoutePropertiesDialog(_("Route Properties"),
                                        m_pSelectedRoute);
      break;
    }

    case ID_TK_MENU_PROPERTIES: {
      parent->ShowTrackPropertiesDialog(m_pSelectedTrack);
      break;
    }

    case ID_TK_MENU_DELETE: {
      int dlg_return = wxID_YES;
      if (g_bConfirmObjectDelete) {
        dlg_return = OCPNMessageBox(
            parent, _("Are you sure you want to delete this track?"),
            _("OpenCPN Track Delete"),
            (long)wxYES_NO | wxCANCEL | wxYES_DEFAULT);
      }

      if (dlg_return == wxID_YES) {
        if (m_pSelectedTrack == g_pActiveTrack)
          m_pSelectedTrack = parent->parent_frame->TrackOff();
        g_pAIS->DeletePersistentTrack(m_pSelectedTrack);
        pConfig->DeleteConfigTrack(m_pSelectedTrack);

        RoutemanGui(*g_pRouteMan).DeleteTrack(m_pSelectedTrack);

        if (TrackPropDlg::getInstanceFlag() && pTrackPropDialog &&
            (pTrackPropDialog->IsShown()) &&
            (m_pSelectedTrack == pTrackPropDialog->GetTrack())) {
          pTrackPropDialog->Hide();
        }

        if (RoutePropDlgImpl::getInstanceFlag() && pRouteManagerDialog &&
            pRouteManagerDialog->IsShown()) {
          pRouteManagerDialog->UpdateTrkListCtrl();
          pRouteManagerDialog->UpdateRouteListCtrl();
        }
        gFrame->InvalidateAllGL();
        gFrame->RefreshAllCanvas();
      }
      break;
    }

    case ID_TK_MENU_SENDTOPEER:
      if (m_pSelectedTrack) {
        SendToPeerDlg dlg;
        dlg.SetTrack(m_pSelectedTrack);

        // Perform initial scan, if necessary

        // Check for stale cache...
        bool bDNScacheStale = true;
        wxDateTime tnow = wxDateTime::Now();
        if (g_DNS_cache_time.IsValid()) {
          wxTimeSpan delta = tnow.Subtract(g_DNS_cache_time);
          if (delta.GetMinutes() < 5) bDNScacheStale = false;
        }

        if ((g_DNS_cache.size() == 0) || bDNScacheStale)
          dlg.SetScanOnCreate(true);

        dlg.SetScanTime(5);  // seconds
        dlg.Create(NULL, -1, _("Send Track to OpenCPN Peer") + _T( "..." ),
                   _T(""));
        dlg.ShowModal();
      }
      break;

    case ID_RC_MENU_SCALE_IN:
      parent->parent_frame->DoStackDown(parent);
      parent->GetCanvasPointPix(zlat, zlon, &r);
      parent->WarpPointer(r.x, r.y);
      break;

    case ID_RC_MENU_SCALE_OUT:
      parent->parent_frame->DoStackUp(parent);
      parent->GetCanvasPointPix(zlat, zlon, &r);
      parent->WarpPointer(r.x, r.y);
      break;

    case ID_RC_MENU_ZOOM_IN:
      parent->SetVPScale(parent->GetVPScale() * 2);
      parent->GetCanvasPointPix(zlat, zlon, &r);
      parent->WarpPointer(r.x, r.y);
      break;

    case ID_RC_MENU_ZOOM_OUT:
      parent->SetVPScale(parent->GetVPScale() / 2);
      parent->GetCanvasPointPix(zlat, zlon, &r);
      parent->WarpPointer(r.x, r.y);
      break;

    case ID_RC_MENU_FINISH:
      parent->FinishRoute();
      //gFrame->SurfaceAllCanvasToolbars();
      parent->Refresh(false);
      g_FlushNavobjChanges = true;
      break;

    case ID_DEF_ZERO_XTE:
      g_pRouteMan->ZeroCurrentXTEToActivePoint();
      break;

    default: {
      //  Look for PlugIn Context Menu selections
      //  If found, make the callback
      ArrayOfPlugInMenuItems item_array =
          g_pi_manager->GetPluginContextMenuItemArray();

      for (unsigned int i = 0; i < item_array.GetCount(); i++) {
        PlugInMenuItemContainer *pimis = item_array[i];
        {
          if (pimis->id == event.GetId()) {
            if (pimis->m_pplugin)
              pimis->m_pplugin->OnContextMenuItemCallback(pimis->id);
          }
        }
      }

      break;
    }
  }  // switch

  //  Chart Groups....
  if ((event.GetId() >= ID_DEF_MENU_GROUPBASE) &&
      (event.GetId() <=
       ID_DEF_MENU_GROUPBASE + (int)g_pGroupArray->GetCount())) {
    parent->SetGroupIndex(event.GetId() - ID_DEF_MENU_GROUPBASE);
  }

  parent->InvalidateGL();

  g_click_stop = 0;  // Context menu was processed, all is well
}
