/***************************************************************************
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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

/**
 * \file
 *
 * Waypoint or mark abstraction
 */

#ifndef ROUTEPOINT_H_
#define ROUTEPOINT_H_

#include <functional>
#include <vector>

#include <wx/bitmap.h>
#include <wx/colour.h>
#include <wx/datetime.h>
#include <wx/string.h>

#include "model/hyperlink.h"

#include "bbox.h"

#define MAX_INT_VAL 2147483647  // max possible integer value before 'rollover'

// minimal allowed ScaMin setting. prevents always hiding
#define SCAMIN_MIN 800

#define ETA_FORMAT_STR "%x %H:%M"
//"%d/%m/%Y %H:%M" //"%Y-%m-%d %H:%M"

// Default color, global state
extern wxColour g_colourWaypointRangeRingsColour; /**< Global instance */

extern int g_LayerIdx; /**< Global instance */

extern wxRect g_blink_rect; /**< Global instance */

class RoutePoint;

RoutePoint *DuplicateRoutePointForRoute(const RoutePoint *source,
                                        const wxString &name_override,
                                        bool register_with_wp_man = false);

/**
 * Represents a waypoint or mark within the navigation system.
 *
 * RoutePoint represents both standalone marks and waypoints that are part of
 * routes or tracks. A RoutePoint contains geographical coordinates, display
 * properties, and various navigation-related attributes.
 *
 * RoutePoints can exist in several states:
 * - As standalone marks (isolated waypoints)
 * - As part of one or more routes
 * - As shared waypoints (used in multiple routes)
 *
 * The class manages visual properties like icons, visibility, and range rings,
 * as well as navigation data like arrival radius, planned speed, and ETAs.
 *
 * @see Route
 */
class RoutePoint {
  friend class RoutePointGui;

public:
  RoutePoint(double lat, double lon, const wxString &icon_ident,
             const wxString &name, const wxString &pGUID = wxEmptyString,
             bool bAddToList = true);
  RoutePoint(RoutePoint *orig);
  RoutePoint();
  virtual ~RoutePoint(void);

  /**
   * Horrible Hack (tm). The destructor needs to call glDeleteTextures, but
   * this is not visible for RoutePoint. This is basically a global, initially
   * doing nothing but at an "early stage" initiated do do the actual
   * glDeleteTextures call.
   */
  static std::function<void(unsigned, const unsigned *)> delete_gl_textures;

  void ReLoadIcon() { m_IconIsDirty = true; }

  void SetPosition(double lat, double lon);
  double GetLatitude() { return m_lat; };
  double GetLongitude() { return m_lon; };
  LLBBox &GetBBox() { return m_wpBBox; }

  bool IsSame(RoutePoint *pOtherRP);  // toh, 2009.02.11
  bool IsVisible() { return m_bIsVisible; }
  bool IsListed() { return m_bIsListed; }
  bool IsNameShown() { return m_bShowName; }
  bool IsNameDynamic();

  bool IsShared() { return m_bsharedMark; }
  void SetShared(bool bshared) { m_bsharedMark = bshared; }

  bool IsSharedInVisibleRoute(void);

  bool IsVisibleSelectable(double scale_val, bool boverrideViz = false);
  void SetVisible(bool viz = true) { m_bIsVisible = viz; }
  void SetListed(bool viz = true) { m_bIsListed = viz; }
  void SetNameShown(bool viz = true) { m_bShowName = viz; }
  bool GetNameShown() { return m_bShowName; }
  virtual wxString GetName(void) { return m_MarkName; }
  wxString GetDescription(void) { return m_MarkDescription; }

  /** Returns the Create Time of this RoutePoint in UTC. */
  wxDateTime GetCreateTime(void);
  /** Sets the create time of this RoutePoint in UTC. */
  void SetCreateTime(wxDateTime dt);

  wxString GetIconName(void) { return m_IconName; }
  void SetIconName(wxString name) { m_IconName = name; }

  void *GetSelectNode(void) { return m_SelectNode; }
  void SetSelectNode(void *node) { m_SelectNode = node; }

  void *GetManagerListNode(void) { return m_ManagerNode; }
  void SetManagerListNode(void *node) { m_ManagerNode = node; }

  void SetName(const wxString &name);
  void CalculateNameExtents(void);

  void SetCourse(double course) { m_routeprop_course = course; };
  double GetCourse() { return m_routeprop_course; };
  void SetDistance(double distance) { m_routeprop_distance = distance; };
  double GetDistance() { return m_routeprop_distance; };

  void SetWaypointArrivalRadius(double dArrivalDistance) {
    m_WaypointArrivalRadius = dArrivalDistance;
  };
  void SetWaypointArrivalRadius(wxString wxArrivalDistance) {
    wxArrivalDistance.ToDouble(&m_WaypointArrivalRadius);
  };
  double GetWaypointArrivalRadius();
  bool GetShowWaypointRangeRings(void) { return m_bShowWaypointRangeRings; };
  int GetWaypointRangeRingsNumber(void);
  float GetWaypointRangeRingsStep(void);
  int GetWaypointRangeRingsStepUnits(void);
  void SetShowWaypointRangeRings(bool b_showWaypointRangeRings) {
    m_bShowWaypointRangeRings = b_showWaypointRangeRings;
  };
  void SetWaypointRangeRingsNumber(int i_WaypointRangeRingsNumber) {
    m_iWaypointRangeRingsNumber = i_WaypointRangeRingsNumber;
  };
  void SetWaypointRangeRingsStep(float f_WaypointRangeRingsStep) {
    m_fWaypointRangeRingsStep = f_WaypointRangeRingsStep;
  };
  void SetWaypointRangeRingsStepUnits(int i_WaypointRangeRingsStepUnits) {
    m_iWaypointRangeRingsStepUnits = i_WaypointRangeRingsStepUnits;
  };

  /**
   * Creates a duplicate for use in routes, detaching layer metadata and
   * applying the default routepoint icon.
   */
  friend RoutePoint *DuplicateRoutePointForRoute(
      const RoutePoint *source, const wxString &name_override,
      bool register_with_wp_man);
  void SetWaypointRangeRingsColour(wxColour wxc_WaypointRangeRingsColour) {
    m_wxcWaypointRangeRingsColour = wxc_WaypointRangeRingsColour;
  };
  void SetTideStation(wxString TideStation) { m_TideStation = TideStation; };
  void SetScaMin(wxString str);
  void SetScaMin(long val);
  long GetScaMin() { return m_ScaMin; };
  void SetScaMax(wxString str);
  void SetScaMax(long val);
  long GetScaMax() { return m_ScaMax; };
  bool GetUseSca() { return b_UseScamin; };
  void SetUseSca(bool value) { b_UseScamin = value; };
  bool IsDragHandleEnabled() { return m_bDrawDragHandle; }
  void SetPlannedSpeed(double spd);
  /**
   * Return the planned speed associated with this waypoint.
   *
   * For a waypoint within a route, this represents the speed to be used when
   * traveling FROM this waypoint TO the next waypoint in the route. For the
   * last waypoint in a route, this value has no navigational significance.
   *
   * This value is used for:
   * - Calculating the ETA at the next waypoint
   * - Determining the total time to complete a route segment
   * - Route planning and navigation displays
   *
   * The speed is stored in knots (nautical miles per hour).
   */
  double GetPlannedSpeed();
  /**
   * Retrieves the Estimated Time of Departure for this waypoint, in UTC.
   *
   * This function returns the waypoint's ETD (Estimated Time of Departure),
   * considering both explicitly set ETD values and values embedded in the
   * waypoint description.
   *
   * @return The estimated time of departure as a wxDateTime object, or
   * wxInvalidDateTime if no valid ETD information exists.
   */
  wxDateTime GetETD();
  /**
   * Retrieves the manually set Estimated Time of Departure for this waypoint,
   * in UTC.
   *
   * This function returns the manually set ETD (Estimated Time of Departure)
   * value only if the ETD was explicitly set by a user rather than calculated
   * automatically. If the ETD is not manually set or if the ETD is invalid, the
   * function returns an invalid datetime value.
   *
   * The function checks two conditions:
   * 1. The m_manual_etd flag must be true, indicating the ETD was manually set
   * 2. The m_seg_etd value must be valid
   *
   * @return The manually set ETD as a wxDateTime object if available, or
   *         wxInvalidDateTime if no manual ETD has been set.
   */
  wxDateTime GetManualETD();
  /**
   * Sets the Estimated Time of Departure for this waypoint, in UTC.
   *
   * This function sets the ETD (Estimated Time of Departure) for the waypoint
   * and marks it as manually set by setting the m_manual_etd flag to true.
   *
   * @param etd The wxDateTime object representing the estimated time of
   * departure.
   */
  void SetETD(const wxDateTime &etd);
  /**
   * Sets the Estimated Time of Departure from a string.
   *
   * This function attempts to parse a datetime string and set it as the ETD
   * value. If successful, it marks the ETD as manually set. If the input string
   * is empty, it clears the ETD value and resets the manual flag.
   *
   * The function tries two parsing methods:
   * 1. ISO combined format (YYYY-MM-DDThh:mm:ss)
   * 2. Generic datetime parsing
   *
   * @param ts String containing the datetime to set as ETD.
   * @return True if parsing was successful or the string was empty, false if
   * parsing failed.
   *
   * @note The input string is assumed to be in UTC format. No timezone
   * conversion is performed.
   * @todo: add support to parse timezone information from the string.
   */
  bool SetETD(const wxString &ts);
  /**
   * Retrieves the Estimated Time of Arrival for this waypoint, in UTC.
   *
   * This function returns the stored Estimated Time of Arrival (ETA) if it's
   * valid. The ETA is typically calculated based on route planning data such as
   * distances and planned speeds.
   *
   * For the first waypoint in a route, the ETA represents when the vessel is
   * expected to reach this waypoint from its current position or a designated
   * starting point. It is calculated based on:
   *   - The current vessel position
   *   - The distance to the first waypoint
   *   - The vessel's current or planned speed
   *
   * For subsequent waypoints, it represents when the vessel is expected to
   * arrive after leaving the previous waypoint.
   *
   * The relationship between waypoints creates a timing chain:
   * The ETA at one waypoint determines the default ETD from that waypoint,
   * which then affects the ETA at the next waypoint, and so on through the
   * route.
   *
   * @return The estimated time of arrival as a wxDateTime object, or
   * wxInvalidDateTime if no valid ETA information exists.
   */
  wxDateTime GetETA();
  /**
   * Retrieves the Estimated Time En route as a formatted string.
   *
   * This function returns the Estimated Time En route (ETE) for this waypoint
   * as a formatted time delta string. The ETE represents the expected travel
   * time to reach this waypoint from the previous point in a route.
   *
   * @return Formatted string representation of the time en route, or an empty
   * string if no ETE value is available.
   */
  wxString GetETE();
  void SetETE(wxLongLong secs);

  double m_lat, m_lon;
  /**
   * Length of the leg from previous waypoint to this waypoint in nautical
   * miles. Undefined for the starting point of a route.
   * @note Calculated field - Length of leg from previous waypoint.
   */
  double m_seg_len;

  /**
   * Planned speed for traveling FROM this waypoint TO the next waypoint.
   *
   * This value represents the Velocity Made Good (VMG) expected when traveling
   * from this waypoint to the next waypoint in a route. It is used to calculate
   * the ETE (Estimated Time Enroute) for this leg, and consequently the ETA
   * (Estimated Time of Arrival) at the next waypoint.
   *
   * If this value is not explicitly set (or is less than 0.1), the route's
   * default planned speed will be used instead. The unit is knots (nautical
   * miles per hour).
   *
   * Not applicable for the last waypoint in a route.
   */
  double m_seg_vmg;
  /**
   * Estimated Time of Departure from this waypoint, in UTC.
   *
   * For normal waypoints in a route, this represents when the vessel is
   * expected to depart from this waypoint toward the next waypoint. By default,
   * it is set to the same value as the ETA (m_seg_eta) to this waypoint,
   * creating a continuous timing chain through the route.
   *
   * For the first waypoint in a route:
   * - During route planning, it is initially set to the route's planned
   * departure time
   * - During active navigation, it represents the time when the route was
   * activated
   *
   * This value can be manually set by the user (indicated by m_manual_etd
   * flag), in which case it takes precedence over the automatically calculated
   * value.
   */
  wxDateTime m_seg_etd;

  /**
   * Estimated Time of Arrival at this waypoint, in UTC.
   *
   * For waypoints in a route, this represents when the vessel is expected to
   * arrive at this waypoint based on the ETD and planned speed from the
   * previous waypoint.
   *
   * For the first waypoint in a route:
   * - During route planning, it is initially set to the route's planned
   * departure time
   * - During active navigation, it is calculated based on:
   *   * The current vessel position
   *   * The distance to the first waypoint
   *   * The vessel's current or planned speed
   *
   * @note Calculated field - calculated from ETD + travel time.
   *
   * The relationship between waypoints creates a timing chain:
   * The ETA at one waypoint determines the default ETD from that waypoint,
   * which then affects the ETA at the next waypoint, and so on through the
   * route.
   */
  wxDateTime m_seg_eta;
  /**
   * Estimated Time Enroute for the leg leading to this waypoint.
   *
   * This value represents the expected travel time (in seconds) from the
   * previous waypoint to this one.
   *
   * For the first waypoint in a route during active navigation, this represents
   * the estimated time from the vessel's current position to the first
   * waypoint.
   *
   * @note Calculated field - derived from segment length to this waypoint and
   * planned speed for this leg (m_seg_vmg)
   */
  wxLongLong m_seg_ete = 0;
  /**
   * Flag indicating whether the ETD has been manually set by the user.
   *
   * When true, this flag indicates that the m_seg_etd value was explicitly set
   * by the user rather than being calculated automatically by the navigation
   * system. This affects how the ETD is treated in route calculations and UI
   * display.
   */
  bool m_manual_etd{false};

  /**
   * Flag indicating if this waypoint is currently selected.
   * @note Calculated field - set to true when the waypoint is selected.
   */
  bool m_bPtIsSelected;
  /**
   * Flag indicating if this waypoint is currently being edited.
   * @note Calculated field - set to true when the waypoint is in edit mode.
   */
  bool m_bRPIsBeingEdited;
  /**
   * Flag indicating if this waypoint is part of a route.
   * @note Calculated field - set to true if the waypoint is part of a route.
   */
  bool m_bIsInRoute;
  /**
   * Flag indicating if the waypoint is a standalone mark.
   * @note a RoutePoint is either an isolated mark or part of a route.
   */
  bool m_bIsolatedMark;

  /**
   * Flag indicating if the waypoint should be drawn on the chart.
   * When false, the waypoint is invisible.
   */
  bool m_bIsVisible;
  /**
   * Flag indicating if the waypoint should appear in the Route Manager dialog's
   * waypoint list.
   */
  bool m_bIsListed;
  /**
   * Flag indicating if this waypoint is active for navigation.
   */
  bool m_bIsActive;
  /**
   * Flag indicating if the waypoint icon needs to be reloaded or redrawn.
   */
  bool m_IconIsDirty;
  /**
   * Description text for the waypoint.
   * May contain encoded information like ETD or planned speed.
   */
  wxString m_MarkDescription;
  /**
   * Globally Unique Identifier for the waypoint.
   */
  wxString m_GUID;
  /**
   * Associated tide station identifier.
   */
  wxString m_TideStation;
  /**
   * Font used for rendering the waypoint name.
   */
  wxFont *m_pMarkFont;
  /**
   * Color used for rendering the waypoint name.
   * @note Calculated field - Obtained from the font manager.
   */
  wxColour m_FontColor;
  /**
   * Size of the waypoint name text when rendered.
   * @note Calculated field - Calculated based on font and text.
   */
  wxSize m_NameExtents;
  /**
   * Flag indicating if the waypoint should blink when displayed.
   */
  bool m_bBlink;
  /**
   * Flag indicating if the waypoint name should be shown.
   */
  bool m_bShowName;
  /**
   * Flag indicating if waypoint data should be shown with the name.
   */
  bool m_bShowNameData;
  /**
   * Current rectangle occupied by the waypoint in the display.
   */
  wxRect CurrentRect_in_DC;
  /**
   * Horizontal offset for waypoint name placement relative to the icon.
   */
  int m_NameLocationOffsetX;
  /**
   * Vertical offset for waypoint name placement relative to the icon.
   */
  int m_NameLocationOffsetY;
  /**
   * Flag indicating if the waypoint belongs to a layer.
   */
  bool m_bIsInLayer;
  /**
   * Layer identifier if the waypoint belongs to a layer.
   */
  int m_LayerID;
  /**
   * Course from this waypoint to the next waypoint, in degrees.
   * @note Calculated field - Calculated from bearing between points.
   */
  double m_routeprop_course;
  /**
   * Distance from this waypoint to the next waypoint, in nautical miles.
   * @note Calculated field - Calculated from distance between points.
   */
  double m_routeprop_distance;
  /**
   * Flag indicating if this is a temporary waypoint.
   */
  bool m_btemp;
  /**
   * Flag indicating if range rings should be shown around the waypoint.
   */
  bool m_bShowWaypointRangeRings;
  /**
   * Number of range rings to display around the waypoint.
   */
  int m_iWaypointRangeRingsNumber;
  /**
   * Distance between consecutive range rings.
   * @note Units depend on m_iWaypointRangeRingsStepUnits (0=nm, 1=km).
   */
  float m_fWaypointRangeRingsStep;
  /**
   * Units for the range rings step (0=nm, 1=km).
   */
  int m_iWaypointRangeRingsStepUnits;
  /**
   * Color for the range rings display.
   */
  wxColour m_wxcWaypointRangeRingsColour;
  /**
   * Texture identifier for rendered text.
   */
  unsigned int m_iTextTexture;
  /**
   * Width of the text texture in pixels.
   */
  int m_iTextTextureWidth;
  /**
   * Height of the text texture in pixels.
   */
  int m_iTextTextureHeight;
  /**
   * Bounding box for the waypoint.
   * @note Calculated field - Calculated from icon size and position.
   */
  LLBBox m_wpBBox;
  double m_wpBBox_view_scale_ppm, m_wpBBox_rotation;
  /**
   * Flag indicating if the waypoint is currently visible on screen.
   * @note Calculated field - Calculated from lat/lon and current view.
   */
  bool m_pos_on_screen;
  /**
   * Cached screen position of the waypoint for drawing arrows and points.
   * @note Calculated field - Calculated from lat/lon and current view.
   */
  wxPoint2DDouble m_screen_pos;
  /**
   * Arrival radius in nautical miles.
   * Distance from waypoint at which it's considered reached.
   */
  double m_WaypointArrivalRadius;
  /**
   * List of hyperlinks associated with this waypoint.
   */
  HyperlinkList *m_HyperlinkList;
  /**
   * String representation of the waypoint creation time.
   */
  wxString m_timestring;
  /**
   * Creation timestamp for the waypoint, in UTC.
   */
  wxDateTime m_CreateTimeX;

private:
  /**
   * Name of the waypoint.
   */
  wxString m_MarkName;
  wxBitmap *m_pbmIcon;
  wxString m_IconName;

  void *m_SelectNode;
  void *m_ManagerNode;

  float m_IconScaleFactor;
  wxBitmap m_ScaledBMP;
  bool m_bPreScaled;
  bool m_bDrawDragHandle;
  wxBitmap m_dragIcon;
  int m_drag_line_length_man, m_drag_icon_offset;
  double m_dragHandleLat, m_dragHandleLon;
  int m_draggingOffsetx, m_draggingOffsety;
  /**
   * Flag indicating whether to use ScaMin visibility controls.
   */
  bool b_UseScamin;
  /**
   * Minimum scale at which the waypoint is visible.
   * Waypoint is not shown at scales larger than this value.
   */
  long m_ScaMin;
  /**
   * Maximum scale at which the waypoint is visible.
   * Waypoint is not shown at scales smaller than this value.
   */
  long m_ScaMax;
  /**
   * The planned speed associated with this waypoint.
   *
   * For a waypoint within a route, this represents the speed to be used when
   * traveling FROM this waypoint TO the next waypoint in the route. For the
   * last waypoint in a route, this value has no navigational significance.
   *
   * This value is used for:
   * - Calculating the ETA at the next waypoint
   * - Determining the total time to complete a route segment
   * - Route planning and navigation displays
   *
   * The speed is stored in knots (nautical miles per hour).
   */
  double m_PlannedSpeed;

  /**
   * Flag indicating if this is a shared mark that is part of a route.
   * When true, the waypoint should not be deleted when the route is deleted.
   */
  bool m_bsharedMark /*m_bKeepXRoute*/;
  unsigned int m_dragIconTexture;
  int m_dragIconTextureWidth, m_dragIconTextureHeight;
};

using RoutePointList = std::vector<RoutePoint *>;

#endif  //  ROUTEPOINT_H_
