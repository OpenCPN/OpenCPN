/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2013 by David S. Register                               *
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

#ifndef _ROUTE_H__
#define _ROUTE_H__

#include <functional>

#include <wx/colour.h>
#include <wx/datetime.h>
#include <wx/gdicmn.h>
#include <wx/object.h>
#include <wx/pen.h>
#include <wx/string.h>

#include "model/route_point.h"
#include "model/routeman.h"
#include "model/hyperlink.h"

#include "bbox.h"

#define WIDTH_UNDEFINED -1

#define ROUTE_DEFAULT_SPEED 5.0
#define RTE_TIME_DISP_UTC _T("UTC")
#define RTE_TIME_DISP_PC _T("PC")
#define RTE_TIME_DISP_LOCAL _T("LOCAL")
#define RTE_UNDEF_DEPARTURE wxInvalidDateTime

class WayPointman;  // FIXME (leamas) why? routeman.h defines this.

extern WayPointman *pWayPointMan;

const wxString GpxxColorNames[] = {
    _T("Black"),      _T("DarkRed"),   _T("DarkGreen"),
    _T("DarkYellow"), _T("DarkBlue"),  _T("DarkMagenta"),
    _T("DarkCyan"),   _T("LightGray"), _T("DarkGray"),
    _T("Red"),        _T("Green"),     _T  ("Yellow"),
    _T("Blue"),       _T("Magenta"),   _T("Cyan"),
    _T("White")};  // The last color defined by Garmin is transparent - we
                   // ignore it
const wxColour GpxxColors[] = {
    wxColour(0x00, 0x00, 0x00), wxColour(0x60, 0x00, 0x00),
    wxColour(0x00, 0x60, 0x00), wxColour(0x80, 0x80, 0x00),
    wxColour(0x00, 0x00, 0x60), wxColour(0x60, 0x00, 0x60),
    wxColour(0x00, 0x80, 0x80), wxColour(0xC0, 0xC0, 0xC0),
    wxColour(0x60, 0x60, 0x60), wxColour(0xFF, 0x00, 0x00),
    wxColour(0x00, 0xFF, 0x00), wxColour(0xF0, 0xF0, 0x00),
    wxColour(0x00, 0x00, 0xFF), wxColour(0xFE, 0x00, 0xFE),
    wxColour(0x00, 0xFF, 0xFF), wxColour(0xFF, 0xFF, 0xFF)};
const int StyleValues[] = {-1,          wxSOLID,      wxDOT,
                           wxLONG_DASH, wxSHORT_DASH, wxDOT_DASH};
const int WidthValues[] = {-1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

class Route : public wxObject {
  friend class RouteGui;

public:
  Route();
  ~Route();

  virtual int GetnPoints(void) { return pRoutePointList->GetCount(); }
  wxString IsPointNameValid(RoutePoint *pRP, const wxString &name) const;

  void AddPoint(RoutePoint *pNewPoint, bool b_rename_in_sequence = true,
                bool b_deferBoxCalc = false);
  void InsertPointAndSegment(RoutePoint *pNewPoint, int insert_after,
                             bool b_rename_in_sequence = true,
                             bool b_deferBoxCalc = false);
  void AddPointAndSegment(RoutePoint *pNewPoint,
                          bool b_rename_in_sequence = true,
                          bool b_deferBoxCalc = false);
  RoutePoint *GetPoint(int nPoint);
  RoutePoint *GetPoint(const wxString &guid);
  int GetIndexOf(RoutePoint *prp);
  RoutePoint *InsertPointBefore(RoutePoint *pRP, double rlat, double rlon,
                                bool bRenamePoints = false);
  RoutePoint *InsertPointAfter(RoutePoint *pRP, double rlat, double rlon,
                               bool bRenamePoints = false);

  RoutePoint *GetLastPoint();
  void DeletePoint(RoutePoint *rp, bool bRenamePoints = false);
  void RemovePoint(RoutePoint *rp, bool bRenamePoints = false);
  void DeSelectRoute();
  void FinalizeForRendering();
  void UpdateSegmentDistance(RoutePoint *prp0, RoutePoint *prp,
                             double planspeed = -1.0);
  void UpdateSegmentDistances(double planspeed = -1.0);
  LLBBox &GetBBox();
  void SetHiLite(int width) { m_hiliteWidth = width; }
  void Reverse(bool bRenamePoints = false);
  void RebuildGUIDList(void);
  void RenameRoutePoints();
  void ReloadRoutePointIcons();
  wxString GetNewMarkSequenced(void);
  void AssembleRoute();
  bool IsEqualTo(Route *ptargetroute);
  void CloneRoute(Route *psourceroute, int start_nPoint, int end_nPoint,
                  const wxString &suffix,
                  const bool duplicate_first_point = false);
  void ClearHighlights(void);

  void SetVisible(bool visible = true, bool includeWpts = true);
  void SetListed(bool visible = true);
  bool IsVisible() { return m_bVisible; }
  bool IsListed() { return m_bListed; }
  bool IsActive() { return m_bRtIsActive; }
  bool IsSelected() { return m_bRtIsSelected; }

  bool ContainsSharedWP();
  void SetSharedWPViz(bool sharedWPVIZ) { m_bsharedWPViz = sharedWPVIZ; }
  bool GetSharedWPViz() { return m_bsharedWPViz; }

  double GetRouteArrivalRadius(void) { return m_ArrivalRadius; }
  void SetRouteArrivalRadius(double radius) { m_ArrivalRadius = radius; }
  void SetDepartureDate(const wxDateTime &dt) {
    if (dt.IsValid()) m_PlannedDeparture = dt;
  }

  wxString GetName() const { return m_RouteNameString; }
  wxString GetTo() const { return m_RouteEndString; }
  wxString GetGUID() const { return m_GUID; }
  void ShowWaypointNames(bool bshow);
  bool AreWaypointNamesVisible();

  int m_ConfigRouteNum;
  bool m_bRtIsSelected;
  bool m_bRtIsActive;
  RoutePoint *m_pRouteActivePoint;
  bool m_bIsBeingCreated;
  bool m_bIsBeingEdited;
  double m_route_length;
  double m_route_time;
  wxString m_RouteNameString;
  wxString m_RouteStartString;
  wxString m_RouteEndString;
  wxString m_RouteDescription;
  bool m_bDeleteOnArrival;
  wxString m_GUID;
  bool m_bIsInLayer;
  int m_LayerID;
  int m_width;
  wxPenStyle m_style;
  int m_lastMousePointIndex;
  bool m_NextLegGreatCircle;
  double m_PlannedSpeed;
  wxDateTime m_PlannedDeparture;
  wxString m_TimeDisplayFormat;

  RoutePointList *pRoutePointList;

  wxRect active_pt_rect;
  wxString m_Colour;
  bool m_btemp;
  int m_hiliteWidth;
  HyperlinkList *m_HyperlinkList;

private:
  LLBBox RBBox;

  int m_nm_sequence;
  bool m_bVisible;  // should this route be drawn?
  bool m_bListed;
  double m_ArrivalRadius;
  bool m_bsharedWPViz;
};

WX_DECLARE_LIST(Route, RouteList);  // establish class Route as list member

#endif  // _ROUTE_H__
