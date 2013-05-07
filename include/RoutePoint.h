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

#ifndef __ROUTEPOINT_H__
#define __ROUTEPOINT_H__

#include <wx/string.h>
#include <wx/datetime.h>
#include <wx/gdicmn.h>
#include <wx/gauge.h>
#include "Hyperlink.h"
#include "gpxdocument.h"

class ocpnDC;
class wxDC;

class RoutePoint
{
public:
      RoutePoint(double lat, double lon, const wxString& icon_ident, const wxString& name, const wxString &pGUID = GPX_EMPTY_STRING, bool bAddToList = true);
      RoutePoint( RoutePoint* orig );
      RoutePoint();
      ~RoutePoint(void);
      void Draw(ocpnDC& dc, wxPoint *rpn = NULL);
      void ReLoadIcon(void);

      wxString CreatePropString(void);
      void SetPropFromString(const wxString &prop_string);

      void SetPosition(double lat, double lon);
      double GetLatitude()  { return m_lat; };
      double GetLongitude() { return m_lon; };
      void CalculateDCRect(wxDC& dc, wxRect *prect);

      bool IsSame(RoutePoint *pOtherRP);        // toh, 2009.02.11
      bool IsVisible() { return m_bIsVisible; }
      bool IsListed() { return m_bIsListed; }
      bool IsNameShown() { return m_bShowName; }
      void SetVisible(bool viz = true){ m_bIsVisible = viz; }
      void SetListed(bool viz = true){ m_bIsListed = viz; }
      void SetNameShown(bool viz = true) { m_bShowName = viz; }
      wxString GetName(void){ return m_MarkName; }
      wxString GetDescription(void) { return m_MarkDescription; }

      void SetName(const wxString & name);
      void CalculateNameExtents(void);

      void SetCourse( double course) { m_routeprop_course = course; };
      double GetCourse() { return m_routeprop_course; };
      void SetDistance( double distance) { m_routeprop_distance = distance; };
      double GetDistance() { return m_routeprop_distance; };


      bool SendToGPS(const wxString& com_name, wxGauge *pProgress);


      double             m_lat;
      double             m_lon;
      double             m_seg_len;              // length in NMI to this point
                                                // undefined for starting point
      double            m_seg_vmg;
      wxDateTime        m_seg_etd;

      bool              m_bPtIsSelected;
      bool              m_bIsBeingEdited;

      bool              m_bIsInRoute;
      bool              m_bIsInTrack;

      bool              m_bIsolatedMark;        // This is an isolated mark

      bool              m_bKeepXRoute;          // This is a mark which is part of a route/track
                                                //  and is also an isolated mark, so should not be deleted with
                                                //  route

      bool              m_bIsVisible;           // true if should be drawn, false if invisible
      bool              m_bIsListed;
      bool              m_bIsActive;
      int               m_ConfigWPNum;
      wxString          m_MarkDescription;
      wxString          m_GUID;
      wxString          m_IconName;
      wxString          m_prop_string_format;         // Alpha character, like "A", giving version of property string

      wxFont            *m_pMarkFont;
      wxColour          m_FontColor;

      wxSize            m_NameExtents;

      wxBitmap          *m_pbmIcon;
      bool              m_bBlink;
      bool              m_bDynamicName;
      bool              m_bShowName;
      wxRect            CurrentRect_in_DC;
      int               m_NameLocationOffsetX;
      int               m_NameLocationOffsetY;
      wxDateTime        m_CreateTime;
      int               m_GPXTrkSegNo;
      bool              m_bIsInLayer;
      int               m_LayerID;

      double            m_routeprop_course;         // course from this waypoint to the next waypoint if in a route.
      double            m_routeprop_distance;       // distance from this waypoint to the next waypoint if in a route.

      HyperlinkList     *m_HyperlinkList;
      bool              m_btemp;

private:
      wxString          m_MarkName;
};

WX_DECLARE_LIST(RoutePoint, RoutePointList);// establish class as list member

#endif
