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

#ifndef __ROUTE_H__
#define __ROUTE_H__

#include <wx/object.h>
#include <wx/list.h>
#include "wx28compat.h"

#include "viewport.h"
#include "RoutePoint.h"

#define WIDTH_UNDEFINED -1

#define ROUTE_DEFAULT_SPEED 5.0
#define RTE_TIME_DISP_UTC _T("UTC")
#define RTE_TIME_DISP_PC _T("PC")
#define RTE_TIME_DISP_LOCAL _T("LOCAL")
#define RTE_UNDEF_DEPARTURE wxInvalidDateTime

class ocpnDC;

class Route : public wxObject
{
public:
      Route(void);
      ~Route(void);

      void AddPoint(RoutePoint *pNewPoint,
                    bool b_rename_in_sequence = true,
                    bool b_deferBoxCalc = false,
                    bool b_isLoading = false);

      void AddTentativePoint(const wxString& GUID);
      RoutePoint *GetPoint(int nPoint);
      RoutePoint *GetPoint ( const wxString &guid );
      int GetIndexOf(RoutePoint *prp);
      RoutePoint *InsertPointBefore(RoutePoint *pRP, double rlat, double rlon, bool bRenamePoints = false);
      RoutePoint *InsertPointAfter(RoutePoint *pRP, double rlat, double rlon, bool bRenamePoints = false);
      void DrawPointWhich(ocpnDC& dc, int iPoint, wxPoint *rpn);
      void DrawSegment(ocpnDC& dc, wxPoint *rp1, wxPoint *rp2, ViewPort &VP, bool bdraw_arrow);
      virtual void Draw(ocpnDC& dc, ViewPort &pVP);
      virtual void DrawGL( ViewPort &VP, OCPNRegion &region );
      RoutePoint *GetLastPoint();
      void DeletePoint(RoutePoint *rp, bool bRenamePoints = false);
      void RemovePoint(RoutePoint *rp, bool bRenamePoints = false);
      void DeSelectRoute();
      void FinalizeForRendering();
      void UpdateSegmentDistances(double planspeed = -1.0);
      void CalculateDCRect(wxDC& dc_route, wxRect *prect, ViewPort &VP);
      int GetnPoints(void){ return m_nPoints; }
      wxBoundingBox GetBBox();
      void SetnPoints(void){ m_nPoints = pRoutePointList->GetCount(); }
      void SetHiLite( int width ) {m_hiliteWidth = width; }
      void Reverse(bool bRenamePoints = false);
      void RebuildGUIDList(void);
      void RenameRoutePoints();
      void ReloadRoutePointIcons();
      wxString GetNewMarkSequenced(void);
      void AssembleRoute();
      bool IsEqualTo(Route *ptargetroute);
      void CloneRoute(Route *psourceroute, int start_nPoint, int end_nPoint, const wxString & suffix);
      void CloneTrack(Route *psourceroute, int start_nPoint, int end_nPoint, const wxString & suffix);
      void CloneAddedTrackPoint(RoutePoint *ptargetpoint, RoutePoint *psourcepoint);
      void CloneAddedRoutePoint(RoutePoint *ptargetpoint, RoutePoint *psourcepoint);
      void ClearHighlights(void);
      void RenderSegment(ocpnDC& dc, int xa, int ya, int xb, int yb, ViewPort &VP, bool bdraw_arrow, int hilite_width = 0);
      void RenderSegmentArrowsGL( float xa, float ya, float xb, float yb, ViewPort &VP);

      bool CrossesIDL(){ return m_bcrosses_idl; }
      void SetVisible(bool visible = true, bool includeWpts = true);
      void SetListed(bool visible = true);
      bool IsVisible() { return m_bVisible; }
      bool IsListed() { return m_bListed; }
      bool IsActive() { return m_bRtIsActive; }
      bool IsSelected() { return m_bRtIsSelected; }
      bool IsTrack(){ return m_bIsTrack; }

      int SendToGPS(const wxString & com_name, bool bsend_waypoints, wxGauge *pProgress);

      double GetRouteArrivalRadius(void){ return m_ArrivalRadius;}
      void SetRouteArrivalRadius(double radius){m_ArrivalRadius = radius;}

      int         m_ConfigRouteNum;
      bool        m_bRtIsSelected;
      bool        m_bRtIsActive;
      RoutePoint  *m_pRouteActivePoint;
      bool        m_bIsBeingCreated;
      bool        m_bIsBeingEdited;
      double      m_route_length;
      double      m_route_time;
      wxString    m_RouteNameString;
      wxString    m_RouteStartString;
      wxString    m_RouteEndString;
      wxString    m_RouteDescription;
      bool        m_bIsTrack;             //TODO should use class type instead
      RoutePoint  *m_pLastAddedPoint;
      bool        m_bDeleteOnArrival;
      wxString    m_GUID;
      bool        m_bIsInLayer;
      int         m_LayerID;
      int         m_width;
      wxPenStyle  m_style;
      int         m_lastMousePointIndex;
      bool        m_NextLegGreatCircle;
      double      m_PlannedSpeed;
      wxDateTime  m_PlannedDeparture;
      wxString    m_TimeDisplayFormat;
      HyperlinkList     *m_HyperlinkList;

      wxArrayString      RoutePointGUIDList;
      RoutePointList     *pRoutePointList;

      wxRect      active_pt_rect;
      wxString    m_Colour;
      bool        m_btemp;
      int         m_hiliteWidth;

private:
      void DrawGLLines( ViewPort &VP, ocpnDC *dc = NULL );

      bool m_bNeedsUpdateBBox;
      wxBoundingBox     RBBox;

      bool        CalculateCrossesIDL();
      int         m_nPoints;
      int         m_nm_sequence;
      bool        m_bVisible; // should this route be drawn?
      bool        m_bListed;
      double      m_ArrivalRadius;
      bool        m_bcrosses_idl;
};

WX_DECLARE_LIST(Route, RouteList); // establish class Route as list member

#endif
