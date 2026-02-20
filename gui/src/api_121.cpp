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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

/**
 * \file
 *
 * ocpn_plugin.h HostApi121 implementation
 */
#include <algorithm>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <wx/wx.h>
#include <wx/event.h>
#include <wx/string.h>
#include <wx/window.h>

#include "model/ais_decoder.h"
#include "model/ais_target_data.h"
#include "model/gui_events.h"
#include "model/gui_vars.h"
#include "model/navobj_db.h"
#include "model/notification_manager.h"
#include "model/own_ship.h"
#include "model/route.h"
#include "model/track.h"

#include "chcanv.h"
#include "ocpn_plugin.h"
#include "tcmgr.h"

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

static RoutePoint* CreateNewPoint(const PlugIn_Waypoint_ExV2* src,
                                  bool b_permanent) {
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

static bool AddPlugInRouteExV3(HostApi121::Route* proute, bool b_permanent) {
  ::Route* route = new ::Route();

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

  route->m_PlannedSpeed = proute->m_PlannedSpeed;
  route->m_Colour = proute->m_Colour;
  route->m_style = proute->m_style;
  route->m_PlannedDeparture = proute->m_PlannedDeparture;
  route->m_TimeDisplayFormat = proute->m_TimeDisplayFormat;

  pRouteList->push_back(route);

  if (b_permanent) {
    // pConfig->AddNewRoute(route);
    NavObj_dB::GetInstance().InsertRoute(route);
  }

  GuiEvents::GetInstance().on_routes_update.Notify();

  return true;
}

static bool UpdatePlugInRouteExV3(HostApi121::Route* proute) {
  bool b_found = false;

  // Find the Route
  Route* pRoute = g_pRouteMan->FindRouteByGUID(proute->m_GUID);
  if (pRoute) b_found = true;

  if (b_found) {
    bool b_permanent = !pRoute->m_btemp;
    g_pRouteMan->DeleteRoute(pRoute);

    b_found = AddPlugInRouteExV3(proute, b_permanent);
  }

  return b_found;
}

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

static wxString DropMarkPI(double lat, double lon) {
  if ((fabs(lat) > 80.0) || (fabs(lon) > 180.)) return "";

  RoutePoint* pWP =
      new RoutePoint(lat, lon, g_default_wp_icon, wxEmptyString, wxEmptyString);
  pWP->m_bIsolatedMark = true;  // This is an isolated mark
  pSelect->AddSelectableRoutePoint(lat, lon, pWP);
  NavObj_dB::GetInstance().InsertRoutePoint(pWP);
  return pWP->m_GUID;
}

static wxString RouteCreatePI(int canvas_index, bool start) {
  if ((size_t)canvas_index < g_canvasArray.GetCount()) {
    ChartCanvas* cc = g_canvasArray.Item(canvas_index);
    if (cc) {
      if (start) {
        cc->StartRoute();
        return "0";
      } else {
        return cc->FinishRoute();
      }
    }
  }
  return "-1";
}

static bool DoMeasurePI(int canvas_index, bool start) {
  if ((size_t)canvas_index < g_canvasArray.GetCount()) {
    ChartCanvas* cc = g_canvasArray.Item(canvas_index);
    if (cc) {
      if (start) {
        cc->StartMeasureRoute();
        return true;
      } else {
        cc->CancelMeasureRoute();
        cc->Refresh(false);
        return true;
      }
    }
  }
  return false;
}

static void EnableDefaultConsole(bool enable) {
  g_bhide_route_console = !enable;
}

static wxString NavToHerePI(double lat, double lon) {
  RoutePoint* pWP_dest =
      new RoutePoint(lat, lon, g_default_wp_icon, wxEmptyString, wxEmptyString);
  pSelect->AddSelectableRoutePoint(lat, lon, pWP_dest);

  RoutePoint* pWP_src = new RoutePoint(gLat, gLon, g_default_wp_icon,
                                       wxEmptyString, wxEmptyString);
  pSelect->AddSelectableRoutePoint(gLat, gLon, pWP_src);

  Route* temp_route = new Route();
  pRouteList->push_back(temp_route);

  temp_route->AddPoint(pWP_src);
  temp_route->AddPoint(pWP_dest);

  pSelect->AddSelectableRouteSegment(gLat, gLon, lat, lon, pWP_src, pWP_dest,
                                     temp_route);

  temp_route->m_RouteNameString = _("Temporary GOTO Route");
  temp_route->m_RouteStartString = _("Here");
  temp_route->m_RouteEndString = _("There");
  temp_route->m_bDeleteOnArrival = true;

  if (g_pRouteMan->GetpActiveRoute()) g_pRouteMan->DeactivateRoute();

  g_pRouteMan->ActivateRoute(temp_route, pWP_dest);
  return temp_route->m_GUID;
}

static bool ActivateRoutePI(wxString route_guid, bool activate) {
  Route* route = g_pRouteMan->FindRouteByGUID(route_guid);
  if (!route) return false;

  if (activate) {
    if (g_pRouteMan->GetpActiveRoute()) g_pRouteMan->DeactivateRoute();
    RoutePoint* best_point =
        g_pRouteMan->FindBestActivatePoint(route, gLat, gLon, gCog, gSog);
    g_pRouteMan->ActivateRoute(route, best_point);
    route->m_bRtIsSelected = false;
    return true;
  } else {
    g_pRouteMan->DeactivateRoute();
    route->m_bRtIsSelected = false;
    return true;
  }
  return false;
}

static void SetMaxZoomScale(double max_scale) {
  g_maxzoomin = wxRound(wxMax(max_scale, 100.));
}

static void EnableDefaultContextMenus(bool enable) {
  g_bhide_context_menus = !enable;
}

static void SetMinZoomScale(double min_scale) {
  for (unsigned int i = 0; i < g_canvasArray.GetCount(); i++) {
    ChartCanvas* cc = g_canvasArray.Item(i);
    cc->SetAbsoluteMinScale(min_scale);
  }
}

static std::shared_ptr<HostApi121::PiPointContext> GetContextAtPoint(
    int x, int y, int canvas_index) {
  ChartCanvas* cc = g_canvasArray.Item(canvas_index);
  if (cc) {
    return cc->GetCanvasContextAtPoint(x, y);
  } else {
    auto rstruct = std::make_shared<HostApi121::PiPointContext>();
    rstruct->object_type = HostApi121::PiContextObjectType::kObjectUnknown;
    rstruct->object_ident = "";
    return rstruct;
  }
}

static wxBitmap GetObjectIcon_PlugIn(const wxString& name) {
  if (pWayPointMan)
    return *pWayPointMan->GetIconBitmap(name);
  else
    return wxNullBitmap;
}

static bool IsRouteActive(wxString route_guid) {
  if (g_pRouteMan->GetpActiveRoute())
    return (route_guid.IsSameAs(g_pRouteMan->GetpActiveRoute()->m_GUID));
  else
    return false;
}

static void SetBoatPosition(double zlat, double zlon) {
  gLat = zlat;
  gLon = zlon;
  GuiEvents::GetInstance().gframe_update_status_bar.Notify();
}

static void RouteInsertWaypoint(int canvas_index, wxString route_guid,
                                double zlat, double zlon) {
  ChartCanvas* parent =
      static_cast<ChartCanvas*>(GetCanvasByIndex(canvas_index));
  if (!parent) return;

  Route* route = g_pRouteMan->FindRouteByGUID(route_guid);
  if (!route) return;

  if (route->m_bIsInLayer) return;

  int seltype = parent->PrepareContextSelections(zlat, zlon);
  if ((seltype & SELTYPE_ROUTESEGMENT) != SELTYPE_ROUTESEGMENT) return;

  bool rename = false;
  route->InsertPointAfter(parent->GetFoundRoutepoint(), zlat, zlon, rename);

  pSelect->DeleteAllSelectableRoutePoints(route);
  pSelect->DeleteAllSelectableRouteSegments(route);
  pSelect->AddAllSelectableRouteSegments(route);
  pSelect->AddAllSelectableRoutePoints(route);

  NavObj_dB::GetInstance().UpdateRoute(route);
}

static void RouteAppendWaypoint(int canvas_index, wxString route_guid) {
  Route* route = g_pRouteMan->FindRouteByGUID(route_guid);
  if (!route) return;

  ChartCanvas* parent =
      static_cast<ChartCanvas*>(GetCanvasByIndex(canvas_index));
  if (!parent) return;

  parent->m_pMouseRoute = route;
  parent->m_routeState = route->GetnPoints() + 1;
  parent->m_pMouseRoute->m_lastMousePointIndex = route->GetnPoints();
  parent->m_pMouseRoute->SetHiLite(50);

  auto pLast = route->GetLastPoint();

  parent->m_prev_rlat = pLast->m_lat;
  parent->m_prev_rlon = pLast->m_lon;
  parent->m_prev_pMousePoint = pLast;

  parent->m_bAppendingRoute = true;
}

static void FinishRoute(int canvas_index) {
  ChartCanvas* parent =
      static_cast<ChartCanvas*>(GetCanvasByIndex(canvas_index));
  if (!parent) return;

  parent->FinishRoute();
}

static bool IsRouteBeingCreated(int canvas_index) {
  ChartCanvas* parent =
      static_cast<ChartCanvas*>(GetCanvasByIndex(canvas_index));
  if (!parent) return false;
  return !(parent->m_pMouseRoute == NULL);
}

static bool AreRouteWaypointNamesVisible(wxString route_guid) {
  Route* route = g_pRouteMan->FindRouteByGUID(route_guid);
  if (!route) return false;
  return route->AreWaypointNamesVisible();
}

static void ShowRouteWaypointNames(wxString route_guid, bool show) {
  Route* route = g_pRouteMan->FindRouteByGUID(route_guid);
  if (!route) return;
  route->ShowWaypointNames(show);
}

static void NavigateToWaypoint(wxString waypoint_guid) {
  RoutePoint* prp = pWayPointMan->FindRoutePointByGUID(waypoint_guid);
  if (!prp) return;

  RoutePoint* pWP_src = new RoutePoint(gLat, gLon, g_default_wp_icon,
                                       wxEmptyString, wxEmptyString);
  pSelect->AddSelectableRoutePoint(gLat, gLon, pWP_src);

  Route* temp_route = new Route();
  pRouteList->push_back(temp_route);

  temp_route->AddPoint(pWP_src);
  temp_route->AddPoint(prp);
  prp->SetShared(true);

  pSelect->AddSelectableRouteSegment(gLat, gLon, prp->m_lat, prp->m_lon,
                                     pWP_src, prp, temp_route);

  wxString name = prp->GetName();
  if (name.IsEmpty()) name = _("(Unnamed Waypoint)");
  wxString rteName = _("Go to ");
  rteName.Append(name);
  temp_route->m_RouteNameString = rteName;
  temp_route->m_RouteStartString = _("Here");
  temp_route->m_RouteEndString = name;
  temp_route->m_bDeleteOnArrival = true;

  if (g_pRouteMan->GetpActiveRoute()) g_pRouteMan->DeactivateRoute();
  g_pRouteMan->ActivateRoute(temp_route, prp);
}

// AIS related
static bool IsAISTrackVisible(wxString ais_mmsi) {
  long mmsi = 0;
  ais_mmsi.ToLong(&mmsi);
  auto myptarget = g_pAIS->Get_Target_Data_From_MMSI(mmsi);
  if (myptarget)
    return myptarget->b_show_track;
  else
    return false;
}

static void AISToggleShowTrack(wxString ais_mmsi) {
  long mmsi = 0;
  ais_mmsi.ToLong(&mmsi);
  auto myptarget = g_pAIS->Get_Target_Data_From_MMSI(mmsi);
  if (myptarget) myptarget->ToggleShowTrack();
}

static void AISToggleShowCPA(wxString ais_mmsi) {
  long mmsi = 0;
  ais_mmsi.ToLong(&mmsi);
  auto myptarget = g_pAIS->Get_Target_Data_From_MMSI(mmsi);
  if (myptarget) myptarget->Toggle_AIS_CPA();
}

static void ShowAISTargetQueryDialog(int canvas_index, wxString ais_mmsi) {
  ChartCanvas* parent =
      static_cast<ChartCanvas*>(GetCanvasByIndex(canvas_index));
  if (!parent) return;

  long mmsi = 0;
  ais_mmsi.ToLong(&mmsi);
  ShowAISTargetQueryDialog(parent, mmsi);
}

static void ShowAISTargetList(int canvas_index) {
  ChartCanvas* parent =
      static_cast<ChartCanvas*>(GetCanvasByIndex(canvas_index));
  if (!parent) return;
  parent->ShowAISTargetList();
}

static bool IsMeasureActive(int canvas_index) {
  ChartCanvas* parent =
      static_cast<ChartCanvas*>(GetCanvasByIndex(canvas_index));
  if (!parent) return false;
  return parent->m_bMeasure_Active;
}

static void CancelMeasure(int canvas_index) {
  ChartCanvas* parent =
      static_cast<ChartCanvas*>(GetCanvasByIndex(canvas_index));
  if (!parent) return;
  parent->CancelMeasureRoute();
}

static void SetDepthUnitVisible(bool bviz) { g_bhide_depth_units = !bviz; }

static void SetOverzoomFlagVisible(bool bviz) { g_bhide_overzoom_flag = !bviz; }

// Extended Chart table management support
static void AddNoShowDirectory(std::string chart_dir) {
  ChartDirectoryExcludedVector.push_back(chart_dir);
}

static void RemoveNoShowDirectory(std::string chart_dir) {
  auto it = std::find(ChartDirectoryExcludedVector.begin(),
                      ChartDirectoryExcludedVector.end(), chart_dir);
  if (it != ChartDirectoryExcludedVector.end())
    ChartDirectoryExcludedVector.erase(it);  // Erase the element
}

static void ClearNoShowVector() { ChartDirectoryExcludedVector.clear(); }

static const std::vector<std::string>& GetNoShowVector() {
  return ChartDirectoryExcludedVector;
}

static bool SelectChartFamily(int CanvasIndex, ChartFamilyEnumPI Family) {
  auto window = GetCanvasByIndex(CanvasIndex);
  auto oCanvas = dynamic_cast<ChartCanvas*>(window);
  if (oCanvas) {
    // Chose the "best" chart in the new family
    // Strategy: Chose a chart from the new family that is the same native scale
    // as the current refernce chart.
    // If this chart is not present in the new family. chose the next larger
    // scale chart.
    // If there are no larger scale charts available in the new family,
    // chose the next smaller scale chart.
    int ref_index = oCanvas->GetQuiltReferenceChartIndex();
    if (ref_index == -1) return false;  // No chart loaded yet
    const ChartTableEntry& cte_ref = ChartData->GetChartTableEntry(ref_index);

    // No action needed if ref chart is already same as target
    //  unless the ref chart is a basemep
    if (cte_ref.GetChartFamily() == Family) {
      if (!cte_ref.IsBasemap()) return false;
    }

    // Special case for switching to ENC
    int index_smallest_nobasemap = -1;
    if (Family == PI_CHART_FAMILY_VECTOR) {
      // Find the smallest scale chart that is not a basemap
      for (auto index : oCanvas->m_pQuilt->GetFullscreenIndexArray()) {
        const ChartTableEntry& cte = ChartData->GetChartTableEntry(index);
        if ((cte.GetChartFamily() == Family) && !cte.IsBasemap())
          index_smallest_nobasemap = index;
      }
      if (index_smallest_nobasemap < 0) {
        // There is no ENC except basemap
        // So choose the smallest scale basemap as reference
        int index_smallest_basemap = -1;
        int scale_smallest_basemap = 1;
        for (auto index : oCanvas->m_pQuilt->GetFullscreenIndexArray()) {
          const ChartTableEntry& cte = ChartData->GetChartTableEntry(index);
          if ((cte.GetChartFamily() == Family) && cte.IsBasemap()) {
            if (cte.GetScale() > scale_smallest_basemap) {
              scale_smallest_basemap = cte.GetScale();
              index_smallest_basemap = index;
            }
          }
        }
        if (index_smallest_basemap >= 0) {
          const ChartTableEntry& cte =
              ChartData->GetChartTableEntry(index_smallest_basemap);
          oCanvas->SelectQuiltRefdbChart(index_smallest_basemap, false);
          // Induce a recomposition of the quilt
          oCanvas->ZoomCanvasSimple(.9999);
          oCanvas->DoCanvasUpdate();
          oCanvas->ReloadVP();
          return true;
        }
      }
    }

    int target_scale = cte_ref.GetScale();

    int target_index = -1;
    for (auto index : oCanvas->m_pQuilt->GetFullscreenIndexArray()) {
      const ChartTableEntry& cte = ChartData->GetChartTableEntry(index);
      if (cte.GetChartFamily() != Family) continue;

      if (cte.GetScale() == target_scale) {
        target_index = index;
        break;
      }
    }

    if (target_index < 0) {
      // Find the largest scale chart that is lower than the reference chart
      for (auto index : oCanvas->m_pQuilt->GetFullscreenIndexArray()) {
        const ChartTableEntry& cte = ChartData->GetChartTableEntry(index);
        if (cte.GetChartFamily() != Family) continue;
        if (cte.GetScale() <= target_scale) {
          target_index = index;
        }
      }
    }

    if (target_index < 0) {
      // Find the largest scale chart that is higher than the reference chart
      for (auto index : oCanvas->m_pQuilt->GetFullscreenIndexArray()) {
        const ChartTableEntry& cte = ChartData->GetChartTableEntry(index);
        if (cte.GetChartFamily() != Family) continue;
        if (cte.GetScale() > target_scale) {
          target_index = index;
          break;
        }
      }
    }

    if (target_index >= 0) {
      const ChartTableEntry& cte = ChartData->GetChartTableEntry(target_index);
      // Found a suitable chart in the new family
      if (oCanvas->IsChartQuiltableRef(target_index)) {
        oCanvas->SelectQuiltRefdbChart(target_index, false);  // no autoscale

        // Induce a recomposition of the quilt
        oCanvas->ZoomCanvasSimple(.9999);
        oCanvas->DoCanvasUpdate();
        oCanvas->ReloadVP();
        return true;
      }
    }
  }
  return false;
}

static void CenterToAisTarget(wxString ais_mmsi) {
  long mmsi = 0;
  if (ais_mmsi.ToLong(&mmsi)) {
    std::shared_ptr<AisTargetData> pAISTarget = nullptr;
    if (g_pAIS) pAISTarget = g_pAIS->Get_Target_Data_From_MMSI(mmsi);

    if (pAISTarget)
      GuiEvents::GetInstance().on_center_ais_target.Notify(pAISTarget);
  }
}  // same as AISTargetListDialog::CenterToTarget ( false )

static void AisTargetCreateWpt(wxString ais_mmsi) {
  long mmsi = 0;
  if (ais_mmsi.ToLong(&mmsi)) {
    std::shared_ptr<AisTargetData> pAISTarget = NULL;
    if (g_pAIS) pAISTarget = g_pAIS->Get_Target_Data_From_MMSI(mmsi);

    if (pAISTarget) {
      RoutePoint* pWP =
          new RoutePoint(pAISTarget->Lat, pAISTarget->Lon, g_default_wp_icon,
                         wxEmptyString, wxEmptyString);
      pWP->m_bIsolatedMark = true;  // This is an isolated mark
      pSelect->AddSelectableRoutePoint(pAISTarget->Lat, pAISTarget->Lon, pWP);
      NavObj_dB::GetInstance().InsertRoutePoint(pWP);

      GuiEvents::GetInstance().on_routes_update.Notify();
    }
  }
}  // same as AISTargetListDialog::OnTargetCreateWpt

static void AisShowAllTracks(bool show) {
  if (g_pAIS) {
    for (const auto& it : g_pAIS->GetTargetList()) {
      auto pAISTarget = it.second;
      if (NULL != pAISTarget) {
        pAISTarget->b_show_track = show;

        // Check for any persistently tracked target, force b_show_track_old
        std::map<int, Track*>::iterator itt;
        itt = g_pAIS->m_persistent_tracks.find(pAISTarget->MMSI);
        if (itt != g_pAIS->m_persistent_tracks.end()) {
          pAISTarget->b_show_track_old = show;
        }
      }
    }
  }  // same as AISTargetListDialog::OnHideAllTracks /
     // AISTargetListDialog::OnShowAllTracks
}

static void AisToggleTrack(wxString ais_mmsi) {
  long mmsi = 0;
  if (ais_mmsi.ToLong(&mmsi)) {
    std::shared_ptr<AisTargetData> pAISTarget = NULL;
    if (g_pAIS) pAISTarget = g_pAIS->Get_Target_Data_From_MMSI(mmsi);

    if (pAISTarget) {
      pAISTarget->b_show_track_old =
          pAISTarget->b_show_track;  // Store current state before toggling
      pAISTarget->b_show_track =
          !pAISTarget->b_show_track;  // Toggle visibility
    }
  }
}

static int GetContextMenuMask() { return g_canvas_context_menu_disable_mask; }

static void SetContextMenuMask(int mask) {
  g_canvas_context_menu_disable_mask = mask;
}

static bool IsAIS_CPAVisible(wxString ais_mmsi) {
  long mmsi = 0;
  ais_mmsi.ToLong(&mmsi);
  auto myptarget = g_pAIS->Get_Target_Data_From_MMSI(mmsi);
  if (myptarget)
    return myptarget->b_show_AIS_CPA;
  else
    return false;
}

static void SetTrackVisibility(const wxString& track_GUID, bool viz) {
  for (Track* ptrack : g_TrackList) {
    if (ptrack->m_GUID == track_GUID) {
      ptrack->SetVisible(viz);
      break;
    }
  }
}

// Tide query API for plugins

bool GetNearestTideStation(double lat, double lon,
                           PlugIn_TideStation* station) {
  if (!ptcmgr || !ptcmgr->IsReady() || !station) return false;

  auto stations = ptcmgr->GetStationsForLL(lat, lon);
  for (auto& [dist, pIDX] : stations) {
    if (pIDX->IDX_type == 'T' || pIDX->IDX_type == 't') {
      // Find the array index for this IDX_entry pointer
      for (int i = 1; i <= ptcmgr->Get_max_IDX(); i++) {
        if (ptcmgr->GetIDX_entry(i) == pIDX) {
          station->index = i;
          strncpy(station->name, pIDX->IDX_station_name, 89);
          station->name[89] = '\0';
          station->lat = pIDX->IDX_lat;
          station->lon = pIDX->IDX_lon;
          return true;
        }
      }
    }
  }
  return false;
}

bool GetTideHeight(int stationIndex, time_t time, float* height) {
  if (!ptcmgr || !ptcmgr->IsReady() || !height) return false;
  float dir = 0.0f;
  return ptcmgr->GetTideOrCurrent(time, stationIndex, *height, dir);
}

std::unique_ptr<HostApi> GetHostApi() {
  return std::make_unique<HostApi121>(HostApi121());
}

bool HostApi121::AddRoute(HostApi121::Route* route, bool permanent) {
  return ::AddPlugInRouteExV3(route, permanent);
}

bool HostApi121::UpdateRoute(HostApi121::Route* route) {
  return ::UpdatePlugInRouteExV3(route);
}

std::unique_ptr<HostApi121::Route> HostApi121::GetRoute(const wxString& guid) {
  ::Route* route = g_pRouteMan->FindRouteByGUID(guid);
  if (!route) return nullptr;

  auto dst_route = std::make_unique<HostApi121::Route>();

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
  dst_route->m_PlannedSpeed = route->m_PlannedSpeed;
  dst_route->m_Colour = route->m_Colour;
  dst_route->m_style = route->m_style;
  dst_route->m_PlannedDeparture = route->m_PlannedDeparture;
  dst_route->m_TimeDisplayFormat = route->m_TimeDisplayFormat;

  return dst_route;
}

wxString HostApi121::DropMarkPI(double lat, double lon) {
  return ::DropMarkPI(lat, lon);
}

wxString HostApi121::RouteCreatePI(int canvas_index, bool start) {
  return ::RouteCreatePI(canvas_index, start);
}

bool HostApi121::DoMeasurePI(int canvas_index, bool start) {
  return ::DoMeasurePI(canvas_index, start);
}

wxString HostApi121::NavToHerePI(double lat, double lon) {
  return ::NavToHerePI(lat, lon);
}

bool HostApi121::ActivateRoutePI(wxString route_guid, bool activate) {
  return ::ActivateRoutePI(route_guid, activate);
}

void HostApi121::EnableDefaultConsole(bool enable) {
  ::EnableDefaultConsole(enable);
}

void HostApi121::EnableDefaultContextMenus(bool enable) {
  ::EnableDefaultContextMenus(enable);
}

void HostApi121::SetMinZoomScale(double min_scale) {
  ::SetMinZoomScale(min_scale);
}

void HostApi121::SetMaxZoomScale(double max_scale) {
  ::SetMaxZoomScale(max_scale);
}

std::shared_ptr<HostApi121::PiPointContext> HostApi121::GetContextAtPoint(
    int x, int y, int canvas_index) {
  return ::GetContextAtPoint(x, y, canvas_index);
}

wxBitmap HostApi121::GetObjectIcon_PlugIn(const wxString& name) {
  return ::GetObjectIcon_PlugIn(name);
}

bool HostApi121::IsRouteActive(wxString route_guid) {
  return ::IsRouteActive(route_guid);
}

void HostApi121::SetBoatPosition(double zlat, double zlon) {
  ::SetBoatPosition(zlat, zlon);
}

void HostApi121::RouteInsertWaypoint(int canvas_index, wxString route_guid,
                                     double zlat, double zlon) {
  ::RouteInsertWaypoint(canvas_index, route_guid, zlat, zlon);
}

void HostApi121::RouteAppendWaypoint(int canvas_index, wxString route_guid) {
  ::RouteAppendWaypoint(canvas_index, route_guid);
}

void HostApi121::FinishRoute(int canvas_index) { ::FinishRoute(canvas_index); }

bool HostApi121::IsRouteBeingCreated(int canvas_index) {
  return ::IsRouteBeingCreated(canvas_index);
}

bool HostApi121::AreRouteWaypointNamesVisible(wxString route_guid) {
  return ::AreRouteWaypointNamesVisible(route_guid);
}

void HostApi121::ShowRouteWaypointNames(wxString route_guid, bool show) {
  ::ShowRouteWaypointNames(route_guid, show);
}

void HostApi121::NavigateToWaypoint(wxString waypoint_guid) {
  ::NavigateToWaypoint(waypoint_guid);
}

bool HostApi121::IsAISTrackVisible(const wxString& ais_mmsi) const {
  return ::IsAISTrackVisible(ais_mmsi);
}

void HostApi121::AISToggleShowTrack(const wxString& ais_mmsi) {
  ::AISToggleShowTrack(ais_mmsi);
}

bool HostApi121::IsAIS_CPAVisible(const wxString& ais_mmsi) const {
  return ::IsAIS_CPAVisible(ais_mmsi);
}

void HostApi121::AISToggleShowCPA(const wxString& ais_mmsi) {
  ::AISToggleShowCPA(ais_mmsi);
}

void HostApi121::ShowAISTargetQueryDialog(int canvas_index,
                                          const wxString& ais_mmsi) {
  ::ShowAISTargetQueryDialog(canvas_index, ais_mmsi);
}

void HostApi121::ShowAISTargetList(int canvas_index) {
  ::ShowAISTargetList(canvas_index);
}

bool HostApi121::IsMeasureActive(int canvas_index) {
  return ::IsMeasureActive(canvas_index);
}

void HostApi121::CancelMeasure(int canvas_index) {
  ::CancelMeasure(canvas_index);
}

void HostApi121::SetDepthUnitVisible(bool bviz) { ::SetDepthUnitVisible(bviz); }

void HostApi121::SetOverzoomFlagVisible(bool viz) {
  ::SetOverzoomFlagVisible(viz);
}

void HostApi121::AddNoShowDirectory(std::string chart_dir) {
  ::AddNoShowDirectory(chart_dir);
}

void HostApi121::RemoveNoShowDirectory(std::string chart_dir) {
  ::RemoveNoShowDirectory(chart_dir);
}

void HostApi121::ClearNoShowVector() { ::ClearNoShowVector(); }

const std::vector<std::string>& HostApi121::GetNoShowVector() {
  return ::GetNoShowVector();
}

bool HostApi121::SelectChartFamily(int CanvasIndex, ChartFamilyEnumPI Family) {
  return ::SelectChartFamily(CanvasIndex, Family);
};

// Enhanced AIS Target List support

void HostApi121::CenterToAisTarget(wxString ais_mmsi) {
  ::CenterToAisTarget(ais_mmsi);
}

void HostApi121::AisTargetCreateWpt(wxString ais_mmsi) {
  ::AisTargetCreateWpt(ais_mmsi);
}

void HostApi121::AisShowAllTracks(bool show) { ::AisShowAllTracks(show); }

void HostApi121::AisToggleTrack(wxString ais_mmsi) {
  ::AisToggleTrack(ais_mmsi);
}

//  Context menu enable/disable, by object type
int HostApi121::GetContextMenuMask() { return ::GetContextMenuMask(); }

void HostApi121::SetContextMenuMask(int mask) { ::SetContextMenuMask(mask); }

void HostApi121::SetTrackVisibiiity(const wxString& track_GUID, bool viz) {
  ::SetTrackVisibility(track_GUID, viz);
}

bool HostApi121::GetNearestTideStation(double lat, double lon,
                                       PlugIn_TideStation* station) {
  return ::GetNearestTideStation(lat, lon, station);
}

bool HostApi121::GetTideHeight(int stationIndex, time_t time, float* height) {
  return ::GetTideHeight(stationIndex, time, height);
}
