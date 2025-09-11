/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Navigation Utility Functions
 * Authors:   David Register
 *            Sean D'Epagnier
 * Project:  OpenCPN
 * Purpose:  Navigation Utility Functions
 * Author:   David Register
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

/* Tracks are broken into SubTracks to allow for efficient rendering and
   selection on tracks with thousands or millions of track points

   Each level of subtracks has exactly half the number of the previous level
   forming a binary tree of subtracks.
   The 0th level contains n-1 subtracks where n is the number of track points

For example, a track with 5 points:

Subtracks[2]                     0
                            __/     \__
                           /           \
Subtracks[1]             0               1
                       /   \           /   \
Subtracks[0]         0       1       2       3
                   /    \ /     \ /     \ /    \
TrackPoints      0       1       2       3       5


The BoundingBox for Subtracks[2][0] will include the entire track and is the
starting point for assembling the track.

Subtracks[1][0] is from 0 to 2
Subtracks[1][1] is from 2 to 5
Subtracks[0][2] is from 2 to 3

The scale factor in Subtracks[2] will determine if it's allowed to just
draw a simple line segment from 0 to 5, or if we need to recurse to find
more detail.

At large scale factors, a long track will mostly be off-screen, so
the bounding box tests quickly eliminate the invisible sections.

At small scale factors, the scale test allows representing a section
of track using a single line segment greatly reducing the number of
segments rendered.  The scale is set so the difference is less than 1 pixel
and mostly impossible to notice.

In practice,  I never exceed 170 segments in all cases assembling a real track
with over 86,000 segments.  If the track is particularly not-straight, and
the size of the screen particularly large (lots of pixels) the number
of segments will be higher, though it should be managable with tracks with
millions of points.
*/

#include <memory>
#include <string>
#include <vector>

#include <wx/colour.h>
#include <wx/datetime.h>
#include <wx/event.h>
#include <wx/jsonval.h>
#include <wx/pen.h>
#include <wx/progdlg.h>
#include <wx/string.h>
#include <wx/utils.h>

#include "model/track.h"

#include "model/config_vars.h"
#include "model/georef.h"
#include "model/json_event.h"
#include "model/nav_object_database.h"
#include "model/navutil_base.h"
#include "model/own_ship.h"
#include "model/routeman.h"
#include "model/select.h"
#include "ocpn_plugin.h"
#include "model/navobj_db.h"

std::vector<Track *> g_TrackList;

class ActiveTrack;  // forward
ActiveTrack *g_pActiveTrack;

#if defined(__UNIX__) && \
    !defined(__WXOSX__)  // high resolution stopwatch for profiling
class OCPNStopWatch {
public:
  OCPNStopWatch() { Reset(); }
  void Reset() { clock_gettime(CLOCK_REALTIME, &tp); }

  double GetTime() {
    timespec tp_end;
    clock_gettime(CLOCK_REALTIME, &tp_end);
    return (tp_end.tv_sec - tp.tv_sec) * 1.e3 +
           (tp_end.tv_nsec - tp.tv_nsec) / 1.e6;
  }

private:
  timespec tp;
};
#endif

TrackPoint::TrackPoint(double lat, double lon, wxString ts)
    : m_lat(lat), m_lon(lon), m_GPXTrkSegNo(1) {
  SetCreateTime(ts);
}

TrackPoint::TrackPoint(double lat, double lon, wxDateTime dt)
    : m_lat(lat), m_lon(lon), m_GPXTrkSegNo(1) {
  SetCreateTime(dt);
}

// Copy Constructor
TrackPoint::TrackPoint(TrackPoint *orig)
    : m_lat(orig->m_lat), m_lon(orig->m_lon), m_GPXTrkSegNo(1) {
  SetCreateTime(orig->GetCreateTime());
}

TrackPoint::~TrackPoint() {}

wxDateTime TrackPoint::GetCreateTime() {
  wxDateTime CreateTimeX;
  ParseGPXDateTime(CreateTimeX, wxString(m_stimestring.c_str()));
  return CreateTimeX;
}

void TrackPoint::SetCreateTime(wxDateTime dt) {
  wxString ts;
  if (dt.IsValid())
    ts = dt.FormatISODate()
             .Append(_T("T"))
             .Append(dt.FormatISOTime())
             .Append(_T("Z"));

  SetCreateTime(ts);
}

void TrackPoint::SetCreateTime(wxString ts) {
  if (ts.Length()) {
    m_stimestring = ts.mb_str();
  } else
    m_stimestring = "";
}

//---------------------------------------------------------------------------------
//    Track Implementation
//---------------------------------------------------------------------------------

double _distance2(vector2D &a, vector2D &b) {
  return (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y);
}
double _distance(vector2D &a, vector2D &b) { return sqrt(_distance2(a, b)); }

Track::Track() {
  m_bVisible = true;
  m_bListed = true;

  m_width = WIDTH_UNDEFINED;
  m_style = wxPENSTYLE_INVALID;

  m_GUID = pWayPointMan->CreateGUID(NULL);
  m_bIsInLayer = false;
  m_btemp = false;

  m_TrackHyperlinkList = new HyperlinkList;
  m_HighlightedTrackPoint = -1;
}

Track::~Track(void) {
  for (size_t i = 0; i < TrackPoints.size(); i++) delete TrackPoints[i];

  delete m_TrackHyperlinkList;
}

#define TIMER_TRACK1 778

BEGIN_EVENT_TABLE(ActiveTrack, wxEvtHandler)
EVT_TIMER(TIMER_TRACK1, ActiveTrack::OnTimerTrack)
END_EVENT_TABLE()

ActiveTrack::ActiveTrack() {
  m_TimerTrack.SetOwner(this, TIMER_TRACK1);
  m_TimerTrack.Stop();
  m_bRunning = false;

  SetPrecision(g_nTrackPrecision);

  m_prev_time = wxInvalidDateTime;
  m_lastStoredTP = NULL;

  wxDateTime now = wxDateTime::Now();
  //    m_ConfigRouteNum = now.GetTicks();        // a unique number....
  trackPointState = firstPoint;
  m_lastStoredTP = NULL;
  m_removeTP = NULL;
  m_fixedTP = NULL;
  m_track_run = 0;
  m_CurrentTrackSeg = 0;
  m_prev_dist = 999.0;
}

ActiveTrack::~ActiveTrack() { Stop(); }

void ActiveTrack::SetPrecision(int prec) {
  m_nPrecision = prec;
  switch (m_nPrecision) {
    case 0: {  // Low
      m_allowedMaxAngle = 10;
      m_allowedMaxXTE = 0.008;
      m_TrackTimerSec = 8;
      m_minTrackpoint_delta = .004;
      break;
    }
    case 1: {  // Medium
      m_allowedMaxAngle = 10;
      m_allowedMaxXTE = 0.004;
      m_TrackTimerSec = 4;
      m_minTrackpoint_delta = .002;
      break;
    }
    case 2: {  // High
      m_allowedMaxAngle = 10;
      m_allowedMaxXTE = 0.0015;
      m_TrackTimerSec = 2;
      m_minTrackpoint_delta = .001;
      break;
    }
  }
}

void ActiveTrack::Start(void) {
  if (!m_bRunning) {
    AddPointNow(true);  // Add initial point
    m_TimerTrack.Start(1000, wxTIMER_CONTINUOUS);
    m_bRunning = true;
  }
}

void ActiveTrack::Stop(bool do_add_point) {
  if (m_bRunning) {
    if (do_add_point)
      AddPointNow(true);  // Force add last point
    else {
      double delta = 0.0;
      if (m_lastStoredTP)
        delta = DistGreatCircle(gLat, gLon, m_lastStoredTP->m_lat,
                                m_lastStoredTP->m_lon);

      if (delta > m_minTrackpoint_delta) AddPointNow(true);  // Add last point
    }
  }

  m_TimerTrack.Stop();
  m_bRunning = false;
  m_track_run = 0;
}

Track *ActiveTrack::DoExtendDaily() {
  Track *pExtendTrack = NULL;
  TrackPoint *pExtendPoint = NULL;

  TrackPoint *pLastPoint = GetPoint(0);
  if (!pLastPoint->GetCreateTime().IsValid()) return NULL;

  for (Track *ptrack : g_TrackList) {
    if (!ptrack->m_bIsInLayer && ptrack->m_GUID != m_GUID) {
      TrackPoint *track_node = ptrack->GetLastPoint();
      if (!track_node->GetCreateTime().IsValid())
        continue;  // Skip this bad track
      if (track_node->GetCreateTime() <= pLastPoint->GetCreateTime()) {
        if (!pExtendPoint ||
            track_node->GetCreateTime() > pExtendPoint->GetCreateTime()) {
          pExtendPoint = track_node;
          pExtendTrack = ptrack;
        }
      }
    }
  }
  if (pExtendTrack && pExtendTrack->GetPoint(0)
                          ->GetCreateTime()
                          .FromTimezone(wxDateTime::GMT0)
                          .IsSameDate(pLastPoint->GetCreateTime().FromTimezone(
                              wxDateTime::GMT0))) {
    int begin = 1;
    if (pLastPoint->GetCreateTime() == pExtendPoint->GetCreateTime()) begin = 2;
    pSelect->DeleteAllSelectableTrackSegments(pExtendTrack);
    wxString suffix = _T("");
    if (GetName().IsNull()) {
      suffix = pExtendTrack->GetName();
      if (suffix.IsNull()) suffix = wxDateTime::Today().FormatISODate();
    }
    pExtendTrack->Clone(this, begin, GetnPoints(), suffix);
    pSelect->AddAllSelectableTrackSegments(pExtendTrack);
    pSelect->DeleteAllSelectableTrackSegments(this);

    return pExtendTrack;
  } else {
    if (GetName().IsNull()) SetName(wxDateTime::Today().FormatISODate());
    return NULL;
  }
}

void Track::Clone(Track *psourcetrack, int start_nPoint, int end_nPoint,
                  const wxString &suffix) {
  if (psourcetrack->m_bIsInLayer) return;

  m_TrackNameString = psourcetrack->m_TrackNameString + suffix;
  m_TrackStartString = psourcetrack->m_TrackStartString;
  m_TrackEndString = psourcetrack->m_TrackEndString;

  bool b_splitting = GetnPoints() == 0;

  int startTrkSegNo;
  if (b_splitting) {
    startTrkSegNo = psourcetrack->GetPoint(start_nPoint)->m_GPXTrkSegNo;
  } else {
    startTrkSegNo = GetLastPoint()->m_GPXTrkSegNo;
  }
  int i;
  for (i = start_nPoint; i <= end_nPoint; i++) {
    TrackPoint *psourcepoint = psourcetrack->GetPoint(i);
    if (psourcepoint) {
      TrackPoint *ptargetpoint = new TrackPoint(psourcepoint);

      AddPoint(ptargetpoint);
    }
  }
}

void ActiveTrack::AdjustCurrentTrackPoint(TrackPoint *prototype) {
  if (prototype) {
    *m_lastStoredTP = *prototype;
    m_prev_time = prototype->GetCreateTime().FromUTC();
  }
}

void ActiveTrack::OnTimerTrack(wxTimerEvent &event) {
  m_TimerTrack.Stop();
  m_track_run++;

  if (m_lastStoredTP)
    m_prev_dist = DistGreatCircle(gLat, gLon, m_lastStoredTP->m_lat,
                                  m_lastStoredTP->m_lon);
  else
    m_prev_dist = 999.0;

  bool b_addpoint = false;

  if ((m_TrackTimerSec > 0.) && ((double)m_track_run >= m_TrackTimerSec) &&
      (m_prev_dist > m_minTrackpoint_delta)) {
    b_addpoint = true;
    m_track_run = 0;
  }

  if (b_addpoint)
    AddPointNow();
  else  // continuously update track beginning point timestamp if no movement.
    if ((trackPointState == firstPoint) && !g_bTrackDaily) {
      wxDateTime now = wxDateTime::Now();
      if (TrackPoints.empty()) TrackPoints.front()->SetCreateTime(now.ToUTC());
    }

  m_TimerTrack.Start(1000, wxTIMER_CONTINUOUS);
}

void ActiveTrack::AddPointNow(bool do_add_point) {
  wxDateTime now = wxDateTime::Now();

  if (m_prev_dist < 0.0005)  // avoid zero length segs
    if (!do_add_point) return;

  if (m_prev_time.IsValid())
    if (m_prev_time == now)  // avoid zero time segs
      if (!do_add_point) return;

  vector2D gpsPoint(gLon, gLat);

  // Check if gps point is not too far from the last point
  // which, if it is the case, means that there is probably a GPS bug on the two
  // positions... So, it is better not to add this false new point.

  // Calculate the distance between two points of the track based on georef lib
  if (g_trackFilterMax) {
    if (trackPointState == potentialPoint) {
      double distToLastGpsPoint = DistLoxodrome(
          m_lastStoredTP->m_lat, m_lastStoredTP->m_lon, gLat, gLon);
      if (distToLastGpsPoint > g_trackFilterMax) return;
    }
  }

  // The dynamic interval algorithm will gather all track points in a queue,
  // and analyze the cross track errors for each point before actually adding
  // a point to the track.

  switch (trackPointState) {
    case firstPoint: {
      TrackPoint *pTrackPoint = AddNewPoint(gpsPoint, now.ToUTC());
      m_lastStoredTP = pTrackPoint;
      trackPointState = secondPoint;
      do_add_point = false;
      break;
    }
    case secondPoint: {
      vector2D pPoint(gLon, gLat);
      skipPoints.push_back(pPoint);
      skipTimes.push_back(now.ToUTC());
      trackPointState = potentialPoint;
      break;
    }
    case potentialPoint: {
      if (gpsPoint == skipPoints[skipPoints.size() - 1]) break;

      unsigned int xteMaxIndex = 0;
      double xteMax = 0;

      // Scan points skipped so far and see if anyone has XTE over the
      // threshold.
      for (unsigned int i = 0; i < skipPoints.size(); i++) {
        double xte = GetXTE(m_lastStoredTP->m_lat, m_lastStoredTP->m_lon, gLat,
                            gLon, skipPoints[i].lat, skipPoints[i].lon);
        if (xte > xteMax) {
          xteMax = xte;
          xteMaxIndex = i;
        }
      }
      if (xteMax > m_allowedMaxXTE) {
        TrackPoint *pTrackPoint =
            AddNewPoint(skipPoints[xteMaxIndex], skipTimes[xteMaxIndex]);
        pSelect->AddSelectableTrackSegment(
            m_lastStoredTP->m_lat, m_lastStoredTP->m_lon, pTrackPoint->m_lat,
            pTrackPoint->m_lon, m_lastStoredTP, pTrackPoint, this);

        m_prevFixedTP = m_fixedTP;
        m_fixedTP = m_removeTP;
        m_removeTP = m_lastStoredTP;
        m_lastStoredTP = pTrackPoint;
        for (unsigned int i = 0; i <= xteMaxIndex; i++) {
          skipPoints.pop_front();
          skipTimes.pop_front();
        }

        // Now back up and see if we just made 3 points in a straight line and
        // the middle one (the next to last) point can possibly be eliminated.
        // Here we reduce the allowed XTE as a function of leg length. (Half the
        // XTE for very short legs).
        if (GetnPoints() > 2) {
          double dist =
              DistGreatCircle(m_fixedTP->m_lat, m_fixedTP->m_lon,
                              m_lastStoredTP->m_lat, m_lastStoredTP->m_lon);
          double xte = GetXTE(m_fixedTP, m_lastStoredTP, m_removeTP);
          if (xte < m_allowedMaxXTE / wxMax(1.0, 2.0 - dist * 2.0)) {
            TrackPoints.pop_back();
            TrackPoints.pop_back();
            TrackPoints.push_back(m_lastStoredTP);
            pSelect->DeletePointSelectableTrackSegments(m_removeTP);
            pSelect->AddSelectableTrackSegment(
                m_fixedTP->m_lat, m_fixedTP->m_lon, m_lastStoredTP->m_lat,
                m_lastStoredTP->m_lon, m_fixedTP, m_lastStoredTP, this);
            delete m_removeTP;
            m_removeTP = m_fixedTP;
            m_fixedTP = m_prevFixedTP;
          }
        }
      }

      skipPoints.push_back(gpsPoint);
      skipTimes.push_back(now.ToUTC());
      break;
    }
  }

  // Check if this is the last point of the track.
  if (do_add_point) {
    TrackPoint *pTrackPoint = AddNewPoint(gpsPoint, now.ToUTC());
    pSelect->AddSelectableTrackSegment(
        m_lastStoredTP->m_lat, m_lastStoredTP->m_lon, pTrackPoint->m_lat,
        pTrackPoint->m_lon, m_lastStoredTP, pTrackPoint, this);
  }

  m_prev_time = now;
}

void Track::ClearHighlights() { m_HighlightedTrackPoint = -1; }

TrackPoint *Track::GetPoint(int nWhichPoint) {
  if (nWhichPoint < (int)TrackPoints.size())
    return TrackPoints[nWhichPoint];
  else
    return NULL;
}

TrackPoint *Track::GetLastPoint() {
  if (TrackPoints.empty()) return NULL;

  return TrackPoints.back();
}

static double heading_diff(double x) {
  if (x > 180) return 360 - x;
  if (x < -180) return -360 + x;
  return x;
}

/* Computes the scale factor when these particular segments
   essentially are smaller than 1 pixel,  This is assuming
   a simplistic flat projection, it might be useful to
   add a mercator or other term, but this works in practice */
double Track::ComputeScale(int left, int right) {
  const double z = WGS84_semimajor_axis_meters * mercator_k0;
  const double mult = DEGREE * z;
  // could multiply by a smaller factor to get
  // better performance with loss of rendering track accuracy

  double max_dist = 0;
  double lata = TrackPoints[left]->m_lat, lona = TrackPoints[left]->m_lon;
  double latb = TrackPoints[right]->m_lat, lonb = TrackPoints[right]->m_lon;

  double bx = heading_diff(lonb - lona), by = latb - lata;

  double lengthSquared = bx * bx + by * by;

  // avoid this calculation for large distances... slight optimization
  // at building with expense rendering zoomed out. is it needed?
  if (lengthSquared > 3) return INFINITY;

  if (lengthSquared == 0.0) {
    for (int i = left + 1; i < right; i++) {
      double lat = TrackPoints[i]->m_lat, lon = TrackPoints[i]->m_lon;
      // v == w case
      double vx = heading_diff(lon - lona);
      double vy = lat - lata;
      double dist = vx * vx + vy * vy;

      if (dist > max_dist) max_dist = dist;
    }
  } else {
    double invLengthSquared = 1 / lengthSquared;
    for (int i = left + 1; i < right; i++) {
      double lat = TrackPoints[i]->m_lat, lon = TrackPoints[i]->m_lon;

      double vx = heading_diff(lon - lona);
      double vy = lat - lata;
      double t = (vx * bx + vy * by) * invLengthSquared;
      double dist;

      if (t < 0.0)
        dist = vx * vx + vy * vy;  // Beyond the 'v' end of the segment
      else if (t > 1.0) {
        double wx = heading_diff(lona - lon);
        double wy = lata - lat;
        dist = wx * wx + wy * wy;  // Beyond the 'w' end of the segment
      } else {
        double projx = vx - t * bx;  // Projection falls on the segment
        double projy = vy - t * by;
        dist = projx * projx + projy * projy;
      }

      if (dist > max_dist) max_dist = dist;
    }
  }

  return max_dist * mult * mult;
}

/* Add a point to a track, should be iterated
   on to build up a track from data.  If a track
   is being slowing enlarged, see AddPointFinalized below */
void Track::AddPoint(TrackPoint *pNewPoint) {
  TrackPoints.push_back(pNewPoint);
  SubTracks.clear();  // invalidate subtracks
}

/* ensures the SubTracks are valid for assembly use */
void Track::Finalize() {
  if (SubTracks.size())  // subtracks already computed
    return;

  //    OCPNStopWatch sw1;

  int n = TrackPoints.size() - 1;
  int level = 0;
  while (n > 0) {
    std::vector<SubTrack> new_level;
    new_level.resize(n);
    if (level == 0)
      for (int i = 0; i < n; i++) {
        new_level[i].m_box.SetFromSegment(
            TrackPoints[i]->m_lat, TrackPoints[i]->m_lon,
            TrackPoints[i + 1]->m_lat, TrackPoints[i + 1]->m_lon);
        new_level[i].m_scale = 0;
      }
    else {
      for (int i = 0; i < n; i++) {
        int p = i << 1;
        new_level[i].m_box = SubTracks[level - 1][p].m_box;
        if (p + 1 < (int)SubTracks[level - 1].size())
          new_level[i].m_box.Expand(SubTracks[level - 1][p + 1].m_box);

        int left = i << level;
        int right = wxMin(left + (1 << level), TrackPoints.size() - 1);
        new_level[i].m_scale = ComputeScale(left, right);
      }
    }
    SubTracks.push_back(new_level);

    if (n > 1 && n & 1) n++;
    n >>= 1;
    level++;
  }
  //    if(TrackPoints.size() > 100)
  //        printf("fin time %f %d\n", sw1.GetTime(), (int)TrackPoints.size());
}

// recursive subtracks fixer for appending a single point
void Track::InsertSubTracks(LLBBox &box, int level, int pos) {
  if (level == (int)SubTracks.size()) {
    std::vector<SubTrack> new_level;
    if (level > 0) box.Expand(SubTracks[level - 1][0].m_box);
    new_level.push_back(SubTrack());
    new_level[pos].m_box = box;
    SubTracks.push_back(new_level);
  } else if (pos < (int)SubTracks[level].size())
    SubTracks[level][pos].m_box.Expand(box);
  else {
    SubTracks[level].push_back(SubTrack());
    SubTracks[level][pos].m_box = box;
  }

  if (level == 0)
    SubTracks[level][pos].m_scale = 0;
  else {
    int left = pos << level;
    int right = wxMin(left + (1 << level), TrackPoints.size() - 1);
    SubTracks[level][pos].m_scale = ComputeScale(left, right);
  }

  if (pos > 0) InsertSubTracks(box, level + 1, pos >> 1);
}

/* This function adds a new point ensuring the resulting track is finalized
   The runtime of this routine is O(log(n)) which is an improvment over
   blowing away the subtracks and calling Finalize which is O(n),
   but should not be used for building a large track O(n log(n)) which
   _is_ worse than blowing the subtracks and calling Finalize.
*/
void Track::AddPointFinalized(TrackPoint *pNewPoint) {
  TrackPoints.push_back(pNewPoint);

  int pos = TrackPoints.size() - 1;

  if (pos > 0) {
    LLBBox box;
    box.SetFromSegment(TrackPoints[pos - 1]->m_lat, TrackPoints[pos - 1]->m_lon,
                       TrackPoints[pos]->m_lat, TrackPoints[pos]->m_lon);
    InsertSubTracks(box, 0, pos - 1);
  }
}

TrackPoint *Track::AddNewPoint(vector2D point, wxDateTime time) {
  TrackPoint *tPoint = new TrackPoint(point.lat, point.lon, time);

  AddPointFinalized(tPoint);

  // NavObjectChanges::getInstance()->AddNewTrackPoint(
  //   tPoint, m_GUID);  // This will update the "changes" file only

  NavObj_dB::GetInstance().AddTrackPoint(this, tPoint);

  // send a wxJson message to all plugins
  wxJSONValue v;
  v["lat"] = tPoint->m_lat;
  v["lon"] = tPoint->m_lon;
  v["Track_ID"] = m_GUID;
  std::string msg_id("OCPN_TRK_POINT_ADDED");
  JsonEvent::getInstance().Notify(msg_id, std::make_shared<wxJSONValue>(v));

  return tPoint;
}

void Track::DouglasPeuckerReducer(std::vector<TrackPoint *> &list,
                                  std::vector<bool> &keeplist, int from, int to,
                                  double delta) {
  keeplist[from] = true;
  keeplist[to] = true;

  int maxdistIndex = -1;
  double maxdist = 0;

  for (int i = from + 1; i < to; i++) {
    double dist = 1852.0 * GetXTE(list[from], list[to], list[i]);

    if (dist > maxdist) {
      maxdist = dist;
      maxdistIndex = i;
    }
  }

  if (maxdist > delta) {
    DouglasPeuckerReducer(list, keeplist, from, maxdistIndex, delta);
    DouglasPeuckerReducer(list, keeplist, maxdistIndex, to, delta);
  }
}

double Track::Length() {
  TrackPoint *l = NULL;
  double total = 0.0;
  for (size_t i = 0; i < TrackPoints.size(); i++) {
    TrackPoint *t = TrackPoints[i];
    if (l) {
      const double offsetLat = 1e-6;
      const double deltaLat = l->m_lat - t->m_lat;
      if (fabs(deltaLat) > offsetLat)
        total += DistGreatCircle(l->m_lat, l->m_lon, t->m_lat, t->m_lon);
      else
        total += DistGreatCircle(l->m_lat + copysign(offsetLat, deltaLat),
                                 l->m_lon, t->m_lat, t->m_lon);
    }
    l = t;
  }

  return total;
}

int Track::Simplify(double maxDelta) {
  int reduction = 0;

  std::vector<TrackPoint *> pointlist;
  std::vector<bool> keeplist;

  ::wxBeginBusyCursor();

  for (size_t i = 0; i < TrackPoints.size(); i++) {
    TrackPoint *trackpoint = TrackPoints[i];

    pointlist.push_back(trackpoint);
    keeplist.push_back(false);
  }

  DouglasPeuckerReducer(pointlist, keeplist, 0, pointlist.size() - 1, maxDelta);

  pSelect->DeleteAllSelectableTrackSegments(this);
  SubTracks.clear();
  TrackPoints.clear();

  for (size_t i = 0; i < pointlist.size(); i++) {
    if (keeplist[i])
      TrackPoints.push_back(pointlist[i]);
    else {
      delete pointlist[i];
      reduction++;
    }
  }
  Finalize();

  pSelect->AddAllSelectableTrackSegments(this);

  //    UpdateSegmentDistances();
  ::wxEndBusyCursor();
  return reduction;
}

Route *Track::RouteFromTrack(wxGenericProgressDialog *pprog) {
  Route *route = new Route();

  TrackPoint *pWP_src = TrackPoints.front();
  size_t prpnodeX;
  RoutePoint *pWP_dst, *pWP_prev;
  TrackPoint *prp_OK =
      NULL;  // last routepoint known not to exceed xte limit, if not yet added

  wxString icon = _T("xmblue");
  if (g_TrackDeltaDistance >= 0.1) icon = _T("diamond");

  int next_ic = 0;
  int back_ic = 0;
  int nPoints = TrackPoints.size();
  bool isProminent = true;
  double delta_dist = 0.;
  double delta_hdg, xte;
  double leg_speed = 0.1;

  leg_speed = g_PlanSpeed;

  // add first point

  pWP_dst = new RoutePoint(pWP_src->m_lat, pWP_src->m_lon, icon, _T ( "" ),
                           wxEmptyString);
  route->AddPoint(pWP_dst);

  pWP_dst->m_bShowName = false;

  pSelect->AddSelectableRoutePoint(pWP_dst->m_lat, pWP_dst->m_lon, pWP_dst);
  pWP_prev = pWP_dst;
  // add intermediate points as needed
  int dProg = 0;
  for (size_t i = 1; i < TrackPoints.size();) {
    TrackPoint *prp = TrackPoints[i];
    prpnodeX = i;
    pWP_dst->m_lat = pWP_prev->m_lat;
    pWP_dst->m_lon = pWP_prev->m_lon;
    pWP_prev = pWP_dst;

    delta_dist = 0.0;
    delta_hdg = 0.0;
    back_ic = next_ic;

    DistanceBearingMercator(prp->m_lat, prp->m_lon, pWP_prev->m_lat,
                            pWP_prev->m_lon, &delta_hdg, &delta_dist);

    if ((delta_dist > (leg_speed * 6.0)) && !prp_OK) {
      int delta_inserts = floor(delta_dist / (leg_speed * 4.0));
      delta_dist = delta_dist / (delta_inserts + 1);
      double tlat = 0.0;
      double tlon = 0.0;

      while (delta_inserts--) {
        ll_gc_ll(pWP_prev->m_lat, pWP_prev->m_lon, delta_hdg, delta_dist, &tlat,
                 &tlon);
        pWP_dst = new RoutePoint(tlat, tlon, icon, _T ( "" ), wxEmptyString);
        route->AddPoint(pWP_dst);
        pWP_dst->m_bShowName = false;
        pSelect->AddSelectableRoutePoint(pWP_dst->m_lat, pWP_dst->m_lon,
                                         pWP_dst);

        pSelect->AddSelectableRouteSegment(pWP_prev->m_lat, pWP_prev->m_lon,
                                           pWP_dst->m_lat, pWP_dst->m_lon,
                                           pWP_prev, pWP_dst, route);

        pWP_prev = pWP_dst;
      }
      prpnodeX = i;
      pWP_dst = pWP_prev;
      next_ic = 0;
      delta_dist = 0.0;
      back_ic = next_ic;
      prp_OK = prp;
      isProminent = true;
    } else {
      isProminent = false;
      if (delta_dist >= (leg_speed * 4.0)) isProminent = true;
      if (!prp_OK) prp_OK = prp;
    }
    while (prpnodeX < TrackPoints.size()) {
      TrackPoint *prpX = TrackPoints[prpnodeX];
      //            TrackPoint src(pWP_prev->m_lat, pWP_prev->m_lon);
      xte = GetXTE(pWP_src, prpX, prp);
      if (isProminent || (xte > g_TrackDeltaDistance)) {
        pWP_dst = new RoutePoint(prp_OK->m_lat, prp_OK->m_lon, icon, _T ( "" ),
                                 wxEmptyString);

        route->AddPoint(pWP_dst);
        pWP_dst->m_bShowName = false;

        pSelect->AddSelectableRoutePoint(pWP_dst->m_lat, pWP_dst->m_lon,
                                         pWP_dst);

        pSelect->AddSelectableRouteSegment(pWP_prev->m_lat, pWP_prev->m_lon,
                                           pWP_dst->m_lat, pWP_dst->m_lon,
                                           pWP_prev, pWP_dst, route);

        pWP_prev = pWP_dst;
        next_ic = 0;
        prpnodeX = TrackPoints.size();
        prp_OK = NULL;
      }

      if (prpnodeX != TrackPoints.size()) prpnodeX--;
      if (back_ic-- <= 0) {
        prpnodeX = TrackPoints.size();
      }
    }

    if (prp_OK) {
      prp_OK = prp;
    }

    DistanceBearingMercator(prp->m_lat, prp->m_lon, pWP_prev->m_lat,
                            pWP_prev->m_lon, NULL, &delta_dist);

    if (!((delta_dist > (g_TrackDeltaDistance)) && !prp_OK)) {
      i++;
      next_ic++;
    }
    int iProg = (i * 100) / nPoints;
    if (pprog && (iProg > dProg)) {
      dProg = iProg;
      pprog->Update(dProg);
    }
  }

  // add last point, if needed
  if (delta_dist >= g_TrackDeltaDistance) {
    pWP_dst =
        new RoutePoint(TrackPoints.back()->m_lat, TrackPoints.back()->m_lon,
                       icon, _T ( "" ), wxEmptyString);
    route->AddPoint(pWP_dst);

    pWP_dst->m_bShowName = false;

    pSelect->AddSelectableRoutePoint(pWP_dst->m_lat, pWP_dst->m_lon, pWP_dst);

    pSelect->AddSelectableRouteSegment(pWP_prev->m_lat, pWP_prev->m_lon,
                                       pWP_dst->m_lat, pWP_dst->m_lon, pWP_prev,
                                       pWP_dst, route);
  }
  route->m_RouteNameString = m_TrackNameString;
  route->m_RouteStartString = m_TrackStartString;
  route->m_RouteEndString = m_TrackEndString;
  route->m_bDeleteOnArrival = false;

  return route;
}

double Track::GetXTE(double fm1Lat, double fm1Lon, double fm2Lat, double fm2Lon,
                     double toLat, double toLon) {
  vector2D v, w, p;

  // First we get the cartesian coordinates to the line endpoints, using
  // the current position as origo.

  double brg1, dist1, brg2, dist2;
  DistanceBearingMercator(toLat, toLon, fm1Lat, fm1Lon, &brg1, &dist1);
  w.x = dist1 * sin(brg1 * PI / 180.);
  w.y = dist1 * cos(brg1 * PI / 180.);

  DistanceBearingMercator(toLat, toLon, fm2Lat, fm2Lon, &brg2, &dist2);
  v.x = dist2 * sin(brg2 * PI / 180.);
  v.y = dist2 * cos(brg2 * PI / 180.);

  p.x = 0.0;
  p.y = 0.0;

  const double lengthSquared = _distance2(v, w);
  if (lengthSquared == 0.0) {
    // v == w case
    return _distance(p, v);
  }

  // Consider the line extending the segment, parameterized as v + t (w - v).
  // We find projection of origo onto the line.
  // It falls where t = [(p-v) . (w-v)] / |w-v|^2

  vector2D a = p - v;
  vector2D b = w - v;

  double t = vDotProduct(&a, &b) / lengthSquared;

  if (t < 0.0)
    return _distance(p, v);  // Beyond the 'v' end of the segment
  else if (t > 1.0)
    return _distance(p, w);               // Beyond the 'w' end of the segment
  vector2D projection = v + t * (w - v);  // Projection falls on the segment
  return _distance(p, projection);
}

double Track::GetXTE(TrackPoint *fm1, TrackPoint *fm2, TrackPoint *to) {
  if (!fm1 || !fm2 || !to) return 0.0;
  if (fm1 == to) return 0.0;
  if (fm2 == to) return 0.0;
  return GetXTE(fm1->m_lat, fm1->m_lon, fm2->m_lat, fm2->m_lon, to->m_lat,
                to->m_lon);
  ;
}

wxString Track::GetIsoDateTime(const wxString label_for_invalid_date) const {
  wxString name;
  TrackPoint *rp = NULL;
  if ((int)TrackPoints.size() > 0) rp = TrackPoints[0];
  if (rp && rp->GetCreateTime().IsValid())
    name = rp->GetCreateTime().FormatISOCombined(' ');
  else
    name = label_for_invalid_date;
  return name;
}

wxString Track::GetDateTime(const wxString label_for_invalid_date) const {
  wxString name;
  TrackPoint *rp = NULL;
  if ((int)TrackPoints.size() > 0) rp = TrackPoints[0];
  if (rp && rp->GetCreateTime().IsValid())
    name = ocpn::toUsrDateTimeFormat(rp->GetCreateTime().FromUTC());
  else
    name = label_for_invalid_date;
  return name;
}
