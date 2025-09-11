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

#include "model/datetime.h"
#include "bbox.h"
#include "hyperlink.h"
#include "route.h"
#include "vector2D.h"

extern std::vector<Track *> g_TrackList;

class ActiveTrack;                  // forward
extern ActiveTrack *g_pActiveTrack; /**< global instance */

struct SubTrack {
  SubTrack() {}

  LLBBox m_box;
  double m_scale;
};

/**
 * Represents a single point in a track.
 */
class TrackPoint {
public:
  TrackPoint(double lat, double lon, wxString ts = "");
  TrackPoint(double lat, double lon, wxDateTime dt);
  TrackPoint(TrackPoint *orig);
  ~TrackPoint();

  /**
   * Retrieves the creation timestamp of a track point as a wxDateTime object.
   *
   * @return wxDateTime object representing the creation time in UTC.
   *         If the internal timestamp string is invalid or empty, the
   *         returned wxDateTime may be invalid.
   */
  wxDateTime GetCreateTime(void);
  /**
   * Sets the creation timestamp for a track point.
   *
   * @param dt The wxDateTime object containing the timestamp to set.
   *           Should be in UTC time already, as no time zone conversion is
   * performed. The time is directly formatted and marked with 'Z' (UTC
   * indicator). If the provided datetime is invalid, an empty string will be
   * stored.
   *
   * Format: YYYY-MM-DDThh:mm:ssZ
   * Example: 2023-04-15T14:22:38Z
   */
  void SetCreateTime(wxDateTime dt);
  const char *GetTimeString() { return m_stimestring.c_str(); }
  bool HasValidTimestamp() {
    if (m_stimestring.size() < strlen("YYYY-MM-DDTHH:MM:SSZ")) return false;
    return true;
  };

  double m_lat, m_lon;
  int m_GPXTrkSegNo;

private:
  /**
   * Sets the creation timestamp for a track point from a string.
   *
   * @param ts The timestamp string to store. Should be in ISO 8601 format.
   *           If empty, the track point will have no timestamp.
   *
   * For example:
   * - "2023-04-15T14:22:38Z" (UTC)
   * - "2023-04-15T10:22:38-04:00" (EDT, 4 hours west of UTC)
   *
   * Time zone information will be correctly interpreted when the timestamp is
   * read via GetCreateTime() which will return a wxDateTime object in UTC.
   */
  void SetCreateTime(wxString ts);
  std::string m_stimestring;
};

/**
 * Represents a track, which is a series of connected track points.
 */
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

  /* Return the name of the track, or the start date/time of the track if no
   * name has been set. */
  wxString GetName(bool auto_if_empty = false) const {
    if (!auto_if_empty || !m_TrackNameString.IsEmpty()) {
      return m_TrackNameString;
    } else {
      return GetDateTime(_("(Unnamed Track)"));
    }
  }
  void SetName(const wxString name) { m_TrackNameString = name; }

  /* Return the start date/time of the track, formatted as ISO 8601 timestamp.
   * The separator between date and time is a space character. */
  wxString GetIsoDateTime(
      const wxString label_for_invalid_date = _("(Unknown Date)")) const;

  /* Return the start date/time of the track, formatted using the global
   * timezone settings. */
  wxString GetDateTime(
      const wxString label_for_invalid_date = _("(Unknown Date)")) const;

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

  HyperlinkList *m_TrackHyperlinkList;
  int m_HighlightedTrackPoint;

  void Clone(Track *psourcetrack, int start_nPoint, int end_nPoint,
             const wxString &suffix);

protected:
  void DouglasPeuckerReducer(std::vector<TrackPoint *> &list,
                             std::vector<bool> &keeplist, int from, int to,
                             double delta);
  double GetXTE(TrackPoint *fm1, TrackPoint *fm2, TrackPoint *to);
  double GetXTE(double fm1Lat, double fm1Lon, double fm2Lat, double fm2Lon,
                double toLat, double toLon);

  std::vector<TrackPoint *> TrackPoints;
  std::vector<std::vector<SubTrack> > SubTracks;

private:
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
/**
 * Represents an active track that is currently being recorded.
 */
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
