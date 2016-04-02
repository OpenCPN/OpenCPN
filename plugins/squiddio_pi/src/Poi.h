/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose   Squiddio plugin
 * Note:     This work is based on the original RoutePoint.h file which is:
 * Copyright (C) 2013 by David S. Register
 *
 ***************************************************************************
 *   Copyright (C) 2014 by Mauro Calvi                                     *
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

#ifndef __ROUTEPOINTSQ_H__
#define __ROUTEPOINTSQ_H__

#include <wx/string.h>
#include <wx/datetime.h>
#include <wx/gdicmn.h>
#include <wx/gauge.h>

#include "Hyperlink.h"
#include "gpxdocument.h"

class ocpnDC;
class wxDC;


class Poi
{
public:
      Poi(double lat, double lon, const wxString& icon_ident, const wxString& name, const wxString &pGUID = GPX_EMPTY_STRING, bool bAddToList = true);
      Poi( Poi* orig );
      Poi();
      ~Poi(void);
//      void Draw(ocpnDC& dc, wxPoint *rpn = NULL);
//      void ReLoadIcon(void);

      wxDateTime GetCreateTime(void);
      void SetCreateTime( wxDateTime dt );
      
      void SetPosition(double lat, double lon);
      double GetLatitude()  { return m_lat; };
      double GetLongitude() { return m_lon; };
//      void CalculateDCRect(wxDC& dc, wxRect *prect);

      bool IsSame(Poi *pOtherRP);        // toh, 2009.02.11
      bool IsVisible() { return m_bIsVisible; }
      bool IsListed() { return m_bIsListed; }
      bool IsNameShown() { return m_bShowName; }
      void SetVisible(bool viz = true){ m_bIsVisible = viz; }
      void SetListed(bool viz = true){ m_bIsListed = viz; }
      void SetNameShown(bool viz = true) { m_bShowName = viz; }
      wxString GetName(void){ return m_MarkName; }
      wxString GetDescription(void) { return m_MarkDescription; }

      void *GetSelectNode(void) { return m_SelectNode; }
      void SetSelectNode(void* node) { m_SelectNode = node; }

      void *GetManagerListNode(void) { return m_ManagerNode; }
      void SetManagerListNode(void* node) { m_ManagerNode = node; }
      
      void SetName(const wxString & name);
      void CalculateNameExtents(void);

      void SetCourse( double course) { m_routeprop_course = course; };
      double GetCourse() { return m_routeprop_course; };
      void SetDistance( double distance) { m_routeprop_distance = distance; };
      double GetDistance() { return m_routeprop_distance; };

	  const wxChar *ParseGPXDateTime( wxDateTime &dt, const wxChar *datetime ); // from navutil.h


//      bool SendToGPS(const wxString& com_name, wxGauge *pProgress);


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
      wxString          m_MarkDescription;
      wxString          m_GUID;
      wxString          m_IconName;

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
      wxString          m_timestring;
      int               m_GPXTrkSegNo;
      bool              m_bIsInLayer;
      int               m_LayerID;

      double            m_routeprop_course;         // course from this waypoint to the next waypoint if in a route.
      double            m_routeprop_distance;       // distance from this waypoint to the next waypoint if in a route.

      HyperlinkList     *m_HyperlinkList;
      bool              m_btemp;

/*
#ifdef ocpnUSE_GL
      void DrawGL( ViewPort &vp, OCPNRegion &region );
      unsigned int m_iTextTexture;
      int m_iTextTextureWidth, m_iTextTextureHeight;

      LLBBox m_wpBBox;
      double m_wpBBox_chart_scale, m_wpBBox_rotation;

      static bool s_bUpdateWaypointsDisplayList;
#endif
*/

private:
      wxString          m_MarkName;
      wxDateTime        m_CreateTimeX;
      
      void              *m_SelectNode;
      void              *m_ManagerNode;
      
};

WX_DECLARE_LIST(Poi, PoiList);// establish class as list member

#endif
