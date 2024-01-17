#include <list>

#include <wx/colour.h>
#include <wx/gdicmn.h>
#include <wx/pen.h>

#include "color_handler.h"
#include "navutil.h"
#include "model/own_ship.h"
#include "model/routeman.h"
#include "track_gui.h"
#include "glChartCanvas.h"

extern Routeman* g_pRouteMan;
extern wxColour g_colourTrackLineColour;

extern wxColor GetDimColor(wxColor c);
extern bool g_bHighliteTracks;

extern ocpnGLOptions g_GLOptions;

void TrackPointGui::Draw(ChartCanvas* cc, ocpnDC& dc) {
  wxPoint r;
  wxRect hilitebox;

  cc->GetCanvasPointPix(m_point.m_lat, m_point.m_lon, &r);

  wxPen *pen;
  pen = g_pRouteMan->GetRoutePointPen();

  int sx2 = 8;
  int sy2 = 8;

  wxRect r1(r.x - sx2, r.y - sy2, sx2 * 2, sy2 * 2);  // the bitmap extents

  hilitebox = r1;
  hilitebox.x -= r.x;
  hilitebox.y -= r.y;
  float radius;
  hilitebox.Inflate(4);
  radius = 4.0f;

  wxColour hi_colour = pen->GetColour();
  unsigned char transparency = 100;

  //  Highlite any selected point
  AlphaBlending(dc, r.x + hilitebox.x, r.y + hilitebox.y, hilitebox.width,
                hilitebox.height, radius, hi_colour, transparency);
}

void TrackGui::Finalize() {
  if (m_track.SubTracks.size())  // subtracks already computed
    return;

  //    OCPNStopWatch sw1;

  int n = m_track.TrackPoints.size() - 1;
  int level = 0;
  while (n > 0) {
    std::vector<SubTrack> new_level;
    new_level.resize(n);
    if (level == 0)
      for (int i = 0; i < n; i++) {
        new_level[i].m_box.SetFromSegment(
            m_track.TrackPoints[i]->m_lat, m_track.TrackPoints[i]->m_lon,
            m_track.TrackPoints[i + 1]->m_lat,
            m_track.TrackPoints[i + 1]->m_lon);
        new_level[i].m_scale = 0;
      }
    else {
      for (int i = 0; i < n; i++) {
        int p = i << 1;
        new_level[i].m_box = m_track.SubTracks[level - 1][p].m_box;
        if (p + 1 < (int)m_track.SubTracks[level - 1].size())
          new_level[i].m_box.Expand(m_track.SubTracks[level - 1][p + 1].m_box);

        int left = i << level;
        int right = wxMin(left + (1 << level), m_track.TrackPoints.size() - 1);
        new_level[i].m_scale = m_track.ComputeScale(left, right);
      }
    }
    m_track.SubTracks.push_back(new_level);

    if (n > 1 && n & 1) n++;
    n >>= 1;
    level++;
  }
  //    if(m_track.TrackPoints.size() > 100)
  //        printf("fin time %f %d\n", sw1.GetTime(), (int)m_track.TrackPoints.size());
}


void TrackGui::GetPointLists(ChartCanvas *cc,
                             std::list<std::list<wxPoint> > &pointlists,
                             ViewPort &VP, const LLBBox &box) {

  if (!m_track.IsVisible() || m_track.GetnPoints() == 0) return;
  Finalize();
  //    OCPNStopWatch sw;
  Segments(cc, pointlists, box, VP.view_scale_ppm);
#if 0
    if(GetnPoints() > 40000) {
        double t = sw.GetTime();
        double c = 0;
        for(std::list< std::list<wxPoint> >::iterator lines = pointlists.begin();
        lines != pointlists.end(); lines++) {
            if(lines->size() > 1)
                c += lines->size();
                continue;
        }
        printf("assemble time %f %f segments %f seg/ms\n", sw.GetTime(), c, c/t);
    }
#endif

  //    Add last segment, dynamically, maybe.....
  // we should not add this segment if it is not on the screen...
  if (m_track.IsRunning()) {
    std::list<wxPoint> new_list;
    pointlists.push_back(new_list);
    AddPointToList(cc, pointlists, m_track.TrackPoints.size() - 1);
    wxPoint r;
    cc->GetCanvasPointPix(gLat, gLon, &r);
    pointlists.back().push_back(r);
  }
}

void TrackGui::Draw(ChartCanvas* cc, ocpnDC& dc, ViewPort& VP,
                    const LLBBox& box) {
  std::list<std::list<wxPoint> > pointlists;
  GetPointLists(cc, pointlists, VP, box);

  if (!pointlists.size()) return;

  //  Establish basic colour
  wxColour basic_colour;
  if (m_track.IsRunning())
    basic_colour = GetGlobalColor(_T ( "URED" ));
  else
    basic_colour = GetDimColor(g_colourTrackLineColour);

  wxPenStyle style = wxPENSTYLE_SOLID;
  int width = g_pRouteMan->GetTrackPen()->GetWidth();
  wxColour col;
  if (m_track.m_style != wxPENSTYLE_INVALID) style = m_track.m_style;
  if (m_track.m_width != WIDTH_UNDEFINED) width = m_track.m_width;
  if (m_track.m_Colour == wxEmptyString) {
    col = basic_colour;
  } else {
    for (unsigned int i = 0; i < sizeof(::GpxxColorNames) / sizeof(wxString);
         i++) {
      if (m_track.m_Colour == ::GpxxColorNames[i]) {
        col = ::GpxxColors[i];
        break;
      }
    }
  }

  double radius = 0.;
  if (g_bHighliteTracks) {
    double radius_meters = 20;  // 1.5 mm at original scale
    radius = radius_meters * VP.view_scale_ppm;
    if (radius < 1.0) radius = 0;
  }

  {
    dc.SetPen(*wxThePenList->FindOrCreatePen(col, width, style));
    dc.SetBrush(*wxTheBrushList->FindOrCreateBrush(col, wxBRUSHSTYLE_SOLID));
    for (std::list<std::list<wxPoint> >::iterator lines = pointlists.begin();
         lines != pointlists.end(); lines++) {
      // convert from linked list to array
      wxPoint *points = new wxPoint[lines->size()];
      int i = 0;
      for (std::list<wxPoint>::iterator line = lines->begin();
           line != lines->end(); line++) {
        points[i] = *line;
        i++;
      }

      int hilite_width = radius;
      if (hilite_width >= 1.0) {
        wxPen psave = dc.GetPen();

        dc.StrokeLines(i, points);

        wxColor trackLine_dim_colour = GetDimColor(g_colourTrackLineColour);
        wxColour hilt(trackLine_dim_colour.Red(), trackLine_dim_colour.Green(),
                      trackLine_dim_colour.Blue(), 128);

        wxPen HiPen(hilt, hilite_width, wxPENSTYLE_SOLID);
        dc.SetPen(HiPen);

        dc.StrokeLines(i, points);

        dc.SetPen(psave);
      } else
        dc.StrokeLines(i, points);

      delete[] points;
    }
  }

  if (m_track.m_HighlightedTrackPoint >= 0)
    TrackPointGui(m_track.TrackPoints[m_track.m_HighlightedTrackPoint]).Draw(cc, dc);
}

// Entry to recursive Assemble at the head of the SubTracks tree
void TrackGui::Segments(ChartCanvas *cc,
                        std::list<std::list<wxPoint> > &pointlists,
                        const LLBBox &box, double scale) {
  if (!m_track.SubTracks.size()) return;

  int level = m_track.SubTracks.size() - 1, last = -2;
  Assemble(cc, pointlists, box, 1 / scale / scale, last, level, 0);
}

/* assembles lists of line strips from the given track recursively traversing
   the subtracks data */
void TrackGui::Assemble(ChartCanvas *cc,
                        std::list<std::list<wxPoint> > &pointlists,
                        const LLBBox &box, double scale, int &last, int level,
                        int pos) {
  if (pos == (int)m_track.SubTracks[level].size()) return;

  SubTrack &s = m_track.SubTracks[level][pos];
  if (box.IntersectOut(s.m_box)) return;

  if (s.m_scale < scale) {
    pos <<= level;

    if (last < pos - 1) {
      std::list<wxPoint> new_list;
      pointlists.push_back(new_list);
    }

    if (last < pos) AddPointToList(cc, pointlists, pos);
    last = wxMin(pos + (1 << level), m_track.TrackPoints.size() - 1);
    AddPointToList(cc, pointlists, last);
  } else {
    Assemble(cc, pointlists, box, scale, last, level - 1, pos << 1);
    Assemble(cc, pointlists, box, scale, last, level - 1, (pos << 1) + 1);
  }
}

void TrackGui::AddPointToList(ChartCanvas *cc,
                              std::list<std::list<wxPoint> > &pointlists, int n) {
  wxPoint r(INVALID_COORD, INVALID_COORD);
  if ((size_t)n < m_track.TrackPoints.size())
    cc->GetCanvasPointPix(m_track.TrackPoints[n]->m_lat, m_track.TrackPoints[n]->m_lon, &r);

  std::list<wxPoint> &pointlist = pointlists.back();
  if (r.x == INVALID_COORD) {
    if (pointlist.size()) {
      std::list<wxPoint> new_list;
      pointlists.push_back(new_list);
    }
    return;
  }

  if (pointlist.size() == 0)
    pointlist.push_back(r);
  else {
    wxPoint l = pointlist.back();
    // ensure the segment is at least 2 pixels
    if ((abs(r.x - l.x) > 1) || (abs(r.y - l.y) > 1)) pointlist.push_back(r);
  }
}
