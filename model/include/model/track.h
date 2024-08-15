/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Navigation Utility Functions
 * Authors:   David Register
 *            Sean D'Epagnier
 *
 ***************************************************************************
 *   Copyright (C) 2016 by David S. Register                               *
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
#ifndef _TRACK_H__
#define _TRACK_H__

#include <wx/progdlg.h>

#include <deque>
#include <list>
#include <vector>

#include "bbox.h"
#include "hyperlink.h"
#include "route.h"
#include "vector2D.h"

extern std::vector<Track *> g_TrackList;

struct SubTrack {
  SubTrack() {}

  LLBBox m_box;
  double m_scale;
};

class TrackPoint {
public:
  TrackPoint(double lat, double lon, wxString ts = "");
  TrackPoint(double lat, double lon, wxDateTime dt);
  TrackPoint(TrackPoint *orig);
  ~TrackPoint();

  wxDateTime GetCreateTime(void);
  void SetCreateTime(wxDateTime dt);
  const char *GetTimeString() { return m_stimestring.c_str(); }
  bool HasValidTimestamp() {
    if (m_stimestring.size() < strlen("YYYY-MM-DDTHH:MM:SSZ")) return false;
    return true;
  };

  double m_lat, m_lon;
  int m_GPXTrkSegNo;

private:
  void SetCreateTime(wxString ts);
  std::string m_stimestring;
};

//----------------------------------------------------------------------------
//    Track
//----------------------------------------------------------------------------

class Track {
  friend class TrackGui;

public:
  Track();
  virtual ~Track();

  int GetnPoints(void) { return TrackPoints.size(); }

  void SetVisible(bool visible = true) { m_bVisible = visible; }
  TrackPoint *GetPoint(int nWhichPoint);
  TrackPoint *GetLastPoint();
  void AddPoint(TrackPoint *pNewPoint);
  void AddPointFinalized(TrackPoint *pNewPoint);
  TrackPoint *AddNewPoint(vector2D point, wxDateTime time);

  void SetListed(bool listed = true) { m_bListed = listed; }
  virtual bool IsRunning() { return false; }

  bool IsVisible() { return m_bVisible; }
  bool IsListed() { return m_bListed; }

  int GetCurrentTrackSeg() { return m_CurrentTrackSeg; }
  void SetCurrentTrackSeg(int seg) { m_CurrentTrackSeg = seg; }

  double Length();
  int Simplify(double maxDelta);
  Route *RouteFromTrack(wxGenericProgressDialog *pprog);

  void ClearHighlights();

  wxString GetName(bool auto_if_empty = false) const {
    if (!auto_if_empty || !m_TrackNameString.IsEmpty()) {
      return m_TrackNameString;
    } else {
      wxString name;
      TrackPoint *rp = NULL;
      if ((int)TrackPoints.size() > 0) rp = TrackPoints[0];
      if (rp && rp->GetCreateTime().IsValid())
        name = rp->GetCreateTime().FormatISODate() + _T(" ") +
               rp->GetCreateTime()
                   .FormatISOTime();  // name = rp->m_CreateTime.Format();
      else
        name = _("(Unnamed Track)");
      return name;
    }
  }
  void SetName(const wxString name) { m_TrackNameString = name; }

  wxString GetDate(bool auto_if_empty = false) const {
    wxString name;
    TrackPoint *rp = NULL;
    if ((int)TrackPoints.size() > 0) rp = TrackPoints[0];
    if (rp && rp->GetCreateTime().IsValid())
      name = rp->GetCreateTime().FormatISODate() + _T(" ") +
             rp->GetCreateTime()
                 .FormatISOTime();  // name = rp->m_CreateTime.Format();
    else
      name = _("(Unknown Date)");
    return name;
  }

  wxString m_GUID;
  bool m_bIsInLayer;
  int m_LayerID;

  wxString m_TrackDescription;

  wxString m_TrackStartString;
  wxString m_TrackEndString;

  int m_width;
  wxPenStyle m_style;
  wxString m_Colour;

  bool m_bVisible;
  bool m_bListed;
  bool m_btemp;

  int m_CurrentTrackSeg;

  HyperlinkList *m_HyperlinkList;
  int m_HighlightedTrackPoint;

  void Clone(Track *psourcetrack, int start_nPoint, int end_nPoint,
             const wxString &suffix);

protected:
  //  void Segments(ChartCanvas *cc, std::list<std::list<wxPoint> > &pointlists,
  //                const LLBBox &box, double scale);
  void DouglasPeuckerReducer(std::vector<TrackPoint *> &list,
                             std::vector<bool> &keeplist, int from, int to,
                             double delta);
  double GetXTE(TrackPoint *fm1, TrackPoint *fm2, TrackPoint *to);
  double GetXTE(double fm1Lat, double fm1Lon, double fm2Lat, double fm2Lon,
                double toLat, double toLon);

  std::vector<TrackPoint *> TrackPoints;
  std::vector<std::vector<SubTrack> > SubTracks;

private:
  //  void GetPointLists(ChartCanvas *cc,
  //                     std::list<std::list<wxPoint> > &pointlists, ViewPort
  //                     &VP, const LLBBox &box);
  void Finalize();
  double ComputeScale(int left, int right);
  void InsertSubTracks(LLBBox &box, int level, int pos);
  //
  //  void AddPointToList(ChartCanvas *cc,
  //                      std::list<std::list<wxPoint> > &pointlists, int n);
  //  void AddPointToLists(ChartCanvas *cc,
  //                       std::list<std::list<wxPoint> > &pointlists, int
  //                       &last, int n);
  //
  //  void Assemble(ChartCanvas *cc, std::list<std::list<wxPoint> > &pointlists,
  //                const LLBBox &box, double scale, int &last, int level, int
  //                pos);
  //
  wxString m_TrackNameString;
};

class Route;
class ActiveTrack : public wxEvtHandler, public Track {
public:
  ActiveTrack();
  ~ActiveTrack();

  void SetPrecision(int precision);

  void Start(void);
  void Stop(bool do_add_point = false);
  Track *DoExtendDaily();
  bool IsRunning() { return m_bRunning; }

  void AdjustCurrentTrackPoint(TrackPoint *prototype);

private:
  void OnTimerTrack(wxTimerEvent &event);
  void AddPointNow(bool do_add_point = false);

  bool m_bRunning;
  wxTimer m_TimerTrack;

  int m_nPrecision;
  double m_TrackTimerSec;
  double m_allowedMaxXTE;
  double m_allowedMaxAngle;

  vector2D m_lastAddedPoint;
  double m_prev_dist;
  wxDateTime m_prev_time;

  TrackPoint *m_lastStoredTP;
  TrackPoint *m_removeTP;
  TrackPoint *m_prevFixedTP;
  TrackPoint *m_fixedTP;
  int m_track_run;
  double m_minTrackpoint_delta;

  enum eTrackPointState {
    firstPoint,
    secondPoint,
    potentialPoint
  } trackPointState;

  std::deque<vector2D> skipPoints;
  std::deque<wxDateTime> skipTimes;

  DECLARE_EVENT_TABLE()
};

#endif
