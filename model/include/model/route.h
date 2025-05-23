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
#define RTE_TIME_DISP_UTC "UTC"
#define RTE_TIME_DISP_PC "PC"
#define RTE_TIME_DISP_LOCAL "LOCAL"
#define RTE_TIME_DISP_GLOBAL "GLOBAL SETTING"
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

/**
 * Represents a navigational route in the navigation system.
 *
 * The Route class manages an ordered list of RoutePoints that define a
 * navigational path between waypoints. It provides functionality for route
 * creation, manipulation, and navigation calculations such as distances,
 * bearings, and estimated times.
 *
 * A Route can be:
 * - Active or inactive for navigation
 * - Visible or hidden on the chart
 * - Temporary or persistent
 * - Part of a layer or standalone
 *
 * The class supports various route operations like adding, inserting, or
 * removing waypoints, calculating segment distances, and handling waypoint
 * sharing between multiple routes. It also maintains navigation-related
 * attributes like planned speeds, departure times, and style attributes for
 * rendering.
 *
 * @see RoutePoint
 */
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
  /**
   * Updates the navigation data for a single route segment between two
   * waypoints.
   *
   * This function calculates and updates the following data for a route
   * segment:
   * - Distance between the two waypoints (stored in the destination waypoint)
   * - Course from the source to destination waypoint
   * - Contribution to the total route length
   * - Segment VMG (Velocity Made Good) based on planned speed
   * - ETE (Estimated Time Enroute) for the segment
   * - ETD (Estimated Time of Departure) from the source waypoint
   * - ETA (Estimated Time of Arrival) at the destination waypoint
   *
   * @param prp0 Pointer to the source waypoint (departure point)
   * @param prp Pointer to the destination waypoint (arrival point)
   * @param planspeed Default planned speed in knots, used if the destination
   * waypoint doesn't specify its own speed
   */
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
  /**
   * Set the departure time of the route.
   *
   * @param dt The departure date and time to set, in UTC.
   */
  void SetDepartureDate(const wxDateTime &dt) {
    if (dt.IsValid()) m_PlannedDeparture = dt;
  }

  wxString GetName() const { return m_RouteNameString; }
  wxString GetTo() const { return m_RouteEndString; }
  wxString GetGUID() const { return m_GUID; }
  void ShowWaypointNames(bool bshow);
  bool AreWaypointNamesVisible();

  /**
   * Internal configuration/identification number for this route.
   * Used to reference the route within the application configuration.
   */
  int m_ConfigRouteNum;
  /**
   * Flag indicating whether this route is currently selected in the UI.
   * When true, the route may be highlighted or its properties shown.
   */
  bool m_bRtIsSelected;
  /**
   * Flag indicating whether this route is currently active for navigation.
   * Only one route can be active at a time in the navigation system.
   */
  bool m_bRtIsActive;
  /**
   * Pointer to the currently active waypoint within this route.
   * When navigating along a route, this indicates the waypoint currently being
   * approached.
   */
  RoutePoint *m_pRouteActivePoint;
  /**
   * Flag indicating that the route is currently being created by the user.
   * Used to control UI behavior during route creation.
   */
  bool m_bIsBeingCreated;
  /**
   * Flag indicating that the route is currently being edited by the user.
   * Used to control UI behavior during route editing.
   */
  bool m_bIsBeingEdited;
  /**
   * Total length of the route in nautical miles, calculated using rhumb line
   * (Mercator) distances.
   *
   * This value is the sum of all individual segment lengths between consecutive
   * waypoints. Each segment length is calculated using the Mercator projection
   * formula, which provides distances along rhumb lines.
   *
   * Rhumb line calculations are used because they maintain a constant bearing,
   * which can be easier to navigate with traditional methods, even though they
   * may result in longer overall distances compared to great circle routes.
   */
  double m_route_length;
  /**
   * Total estimated time to complete the route in seconds.
   * Calculated based on segment lengths and planned speeds for each segment.
   */
  double m_route_time;
  /**
   * User-assigned name for the route.
   * Used for display and identification purposes.
   */
  wxString m_RouteNameString;
  /**
   * Name or description of the route's starting point.
   * Typically derived from the first RoutePoint name.
   */
  wxString m_RouteStartString;
  /**
   * Name or description of the route's ending point.
   * Typically derived from the last RoutePoint name.
   */
  wxString m_RouteEndString;
  /**
   * Additional descriptive information about the route.
   * User-provided details about the route's purpose or characteristics.
   */
  wxString m_RouteDescription;
  /**
   * Flag indicating whether the route should be deleted once navigation reaches
   * the end. When true, the route will be automatically removed upon
   * completion.
   */
  bool m_bDeleteOnArrival;
  /**
   * Globally unique identifier for this route.
   * Used for persistence and cross-referencing in the database.
   */
  wxString m_GUID;
  /**
   * Flag indicating whether this route belongs to a layer.
   * Layers are collections of chart objects that can be managed together.
   */
  bool m_bIsInLayer;
  /**
   * Identifier of the layer containing this route.
   * Only relevant when m_bIsInLayer is true.
   */
  int m_LayerID;
  /**
   * Width of the route line in pixels when rendered on the chart.
   * A value of WIDTH_UNDEFINED indicates to use the default width.
   */
  int m_width;
  /**
   * Style of the route line when rendered on the chart.
   * Uses wxPenStyle values to determine solid, dashed, dotted, etc.
   */
  wxPenStyle m_style;
  /**
   * Index of the most recently interacted with route point.
   * Used for UI interactions and editing operations.
   */
  int m_lastMousePointIndex;
  /**
   * Flag indicating whether the next leg should be calculated using great
   * circle navigation or rhumb line navigation.
   *
   * This flag affects the visual rendering of the route segment during route
   * creation, but notably does NOT affect how m_route_length is calculated.
   *
   * The flag is set automatically during route creation when the difference
   * between great circle and rhumb line distances exceeds a threshold,
   * prompting the user to choose whether to include great circle routing points
   * for the leg.
   *
   * Great circle routes are visually represented by drawing multiple
   * intermediate points along the great circle path to create a segmented
   * approximation.
   */
  bool m_NextLegGreatCircle;
  /**
   * Default planned speed for the route in knots.
   * Used for time calculations when individual waypoint speeds aren't
   * specified.
   */
  double m_PlannedSpeed;
  /**
   * Planned departure time for the route, in UTC.
   * Used as the starting time for all ETA calculations along the route.
   */
  wxDateTime m_PlannedDeparture;
  /**
   * Format for displaying times in the UI.
   * Can be UTC, local time, PC time, or follow global settings.
   */
  wxString m_TimeDisplayFormat;
  /**
   * Ordered list of waypoints (RoutePoints) that make up this route.
   * The sequence defines the path from start to finish.
   */
  RoutePointList *pRoutePointList;
  /**
   * Bounding rectangle for the active point in screen coordinates.
   * Used for hit testing and selection in the UI.
   */
  wxRect active_pt_rect;
  /**
   * Color name for rendering the route on the chart.
   * References a named color in the application's color scheme.
   */
  wxString m_Colour;
  /**
   * Flag indicating if this is a temporary route.
   * Temporary routes might not be saved to persistent storage.
   */
  bool m_btemp;
  /**
   * Width in pixels for highlighting the route when selected.
   * Zero indicates no highlighting.
   */
  int m_hiliteWidth;
  /**
   * List of hyperlinks associated with this route.
   * Can contain URLs to external resources or documentation.
   */
  HyperlinkList *m_HyperlinkList;

private:
  LLBBox RBBox;

  /**
   * Counter for automatically generated route point names.
   * Incremented each time a new auto-named point is created.
   */
  int m_nm_sequence;
  /**
   * Flag indicating whether this route should be drawn on the chart.
   * When false, the route is hidden from view.
   */
  bool m_bVisible;
  /**
   * Flag indicating whether this route appears in lists and UI elements.
   * When false, the route may be excluded from route listings.
   */
  bool m_bListed;
  /**
   * Arrival radius in nautical miles for waypoints in this route.
   * Defines the distance at which a waypoint is considered reached.
   */
  double m_ArrivalRadius;
  /**
   * Flag controlling the visibility of shared waypoints in this route.
   * When false, waypoints used by multiple routes may be hidden.
   */
  bool m_bsharedWPViz;
};

WX_DECLARE_LIST(Route, RouteList);  // establish class Route as list member

#endif  // _ROUTE_H__
