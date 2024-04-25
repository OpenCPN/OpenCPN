/******************************************************************************
 * $Id: MagneticPlotMap.cpp,v 1.0 2011/02/26 01:54:37 nohal Exp $
 *
 * Project:  OpenCPN
 * Purpose:  WMM Plugin
 * Author:   Sean D'Epagnier
 *
 ***************************************************************************
 *   Copyright (C) 2013 by Sean D'Epagnier   *
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
 ***************************************************************************
 */

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // precompiled headers

#include <wx/progdlg.h>

#include "ocpn_plugin.h"
#include "pi_ocpndc.h"

#ifndef __OCPN__ANDROID__
#include <GL/gl.h>
#include <GL/glu.h>
#else
#include "qopengl.h"  // this gives us the qt runtime gles2.h
#include "GL/gl_private.h"
#endif

#include "GeomagnetismHeader.h"
#include "MagneticPlotMap.h"

//static const long long lNaN = 0xfff8000000000000;
//#define qNan (*(double *)&lNaN)

double square(double x) { return x * x; }

/* initialize cache to contain data */
void ParamCache::Initialize(double step) {
  if (m_step != step) {
    m_step = step;
    delete[] values;
    int size = 360 / step;
    values = new double[size];
  }
  m_lat = 100; /* invalidate data */
}

/* attempt a cache read returning a hit or miss */
bool ParamCache::Read(double lat, double lon, double &value) {
  if (lat != m_lat) return false;
  lon += 180;
  if (lon > 360) lon -= 360;
  if (lon < 0 || lon >= 360) return false;
  double div = lon / m_step;
  if (div != floor(div)) return false;

  value = values[(int)div];
  return true;
}

/* set the accuracy of the map */
void MagneticPlotMap::ConfigureAccuracy(int step, int poleaccuracy) {
  /* keeping m_Step powers of 2 */
  switch (step) {
    case 1:
      m_Step = .0625;
      break;
    case 2:
      m_Step = .125;
      break;
    case 3:
      m_Step = .25;
      break;
    case 4:
      m_Step = .5;
      break;
    case 5:
      m_Step = 1;
      break;
    case 6:
      m_Step = 2;
      break;
    case 7:
      m_Step = 4;
      break;
    default:
      m_Step = 8;
      break;
  }

  /* keeping m_PoleAccuracy logarithmic */
  switch (poleaccuracy) {
    case 1:
      m_PoleAccuracy = 5e-1;
      break;
    case 2:
      m_PoleAccuracy = 1e-1;
      break;
    case 3:
      m_PoleAccuracy = 1e-2;
      break;
    case 4:
      m_PoleAccuracy = 1e-3;
      break;
    default:
      m_PoleAccuracy = 1e-4;
      break;
  }
}

/* compute the graphed parameter for one lat/lon location */
double MagneticPlotMap::CalcParameter(double lat, double lon) {
  MAGtype_CoordSpherical CoordSpherical;
  MAGtype_CoordGeodetic CoordGeodetic;
  MAGtype_GeoMagneticElements GeoMagneticElements;

  CoordGeodetic.lambda = lon;
  CoordGeodetic.phi = lat;
  CoordGeodetic.HeightAboveEllipsoid = 0;
  CoordGeodetic.HeightAboveGeoid = 0;
  CoordGeodetic.UseGeoid = 0;

  /* Convert from geodeitic to Spherical Equations: 17-18, WMM Technical report
   */
  MAG_GeodeticToSpherical(*Ellip, CoordGeodetic, &CoordSpherical);

  /* Computes the geoMagnetic field elements and their time change */
  MAG_Geomag(*Ellip, CoordSpherical, CoordGeodetic, TimedMagneticModel,
             &GeoMagneticElements);
  MAG_CalculateGridVariation(CoordGeodetic, &GeoMagneticElements);

  double ret = 0;
  switch (m_type) {
    case DECLINATION_PLOT:
      ret = GeoMagneticElements.Decl >= 180 ? GeoMagneticElements.Decl - 360
                                            : GeoMagneticElements.Decl;
      break;
    case INCLINATION_PLOT:
      ret = GeoMagneticElements.Incl;
      break;
    case FIELD_STRENGTH_PLOT:
      ret = GeoMagneticElements.F;
      break;
  }

  return ret;
}

/* build up cache for all longitudes */
void MagneticPlotMap::BuildParamCache(ParamCache &cache, double lat) {
  int i = 0;
  for (double lon = -180; lon < 180; lon += m_Step, i++)
    cache.values[i] = CalcParameter(lat, lon);
  cache.m_lat = lat;
}

/* a possible speedup would be to cache the last 4-10 values
   calculated as well as the two main cache banks to speed
   up the recursion in PlotRegion */
double MagneticPlotMap::CachedCalcParameter(double lat, double lon) {
  double value;
  if (!m_Cache[0].Read(lat, lon, value) && !m_Cache[1].Read(lat, lon, value))
    value = CalcParameter(lat, lon);
  return value;
}

/* given a line segment (x1, y1) (x2, y2), find the point (rx, ry) along it
   which crosses a contour.  if lat is true, x1 and x2 are latitudes, other
   wise longitudes.  lonval is the complement to this which is for both x1 and
   x2 to allow computing new y values along te segment.   rx is set to nan if
   there is no intersection.  True is returned if success, otherwise false
   to signify that we need to dig deeper to get a decent map. */
bool MagneticPlotMap::Interpolate(double x1, double x2, double y1, double y2,
                                  bool lat, double lonval, double &rx,
                                  double &ry) {
  if (fabs(x1 - x2) < m_PoleAccuracy) { /* to avoid recursing too far. make this
                                           value smaller to get more accuracy
                                           especially near the magnetic poles */
    rx = NAN;                          /* set as no intersections */
    return true;
  }

  /* this really only happens between geographic and magnetic pole, but to
   * correct it... */
  if (m_type == DECLINATION_PLOT) {
    if (y1 - y2 > 180) y2 += 360;
    if (y2 - y1 > 180) y1 += 360;
  }

  y1 /= m_Spacing;
  y2 /= m_Spacing;

  double fy1 = floor(y1), fy2 = floor(y2);
  if (fy1 == fy2) {
    rx = NAN; /* no intersections occured */
    return true;
  }

  if (fabs(fy1 - fy2) >
      1) /* stepped over too many lines, trigger more recursion */
    return false;

  /* make y2 larger */
  if (y1 > y2) {
    double t = y2;
    y2 = y1;
    y1 = t;
    t = x2;
    x2 = x1;
    x1 = t;
  }

  ry = floor(y2); /* don't need this in loop? */
  for (int i = 0;; i++) {
    //  x1=m*y1+b   x2=m*y2+b
    //  (x1-b)/y1=(x2-b)/y2    x1/y1-x2/y2=b*(1/y1-1/y2)
    //  b = (x1/y1-x2/y2)/(1/y1-1/y2)
    //  b = [(x1/y1-x2/y2)* (y1*y2)]/[(1/y1-1/y2) *(y1*y2)]
    //  b = (x1*y2-x2*y1)/(y2-y1)

    rx = (x1 * (y2 - ry) - x2 * (y1 - ry)) / (y2 - y1);

    if (fabs(x1 - x2) < m_PoleAccuracy) /* to avoid recursing too far close */
      return true;

    double p;
    if (lat)
      p = CalcParameter(rx, lonval);
    else
      p = CalcParameter(lonval, rx);

    if (std::isnan(p)) /* is this actually correct? */
      return true;

    if (m_type == DECLINATION_PLOT &&
        p - ry * m_Spacing < -180) /* way off, try other way around */
      p += 360;

    p /= m_Spacing;

    double err = p - ry;

    /* this valuee of 1e-3 could be reduces to increase accuracy, this value
     * seems ok */
    if (fabs(err) < 1e-3 || p == y1 || p == y2) /* close enough */
      return true;

    if (err < 0) {
      if (p < y1) /* undershot.. this case should not hit */
        return false;
      x1 = rx;
      y1 = p;
    } else {
      if (p > y2) /* overshot.. this case should not hit */
        return false;
      x2 = rx;
      y2 = p;
    }
  }
}

/* once we have a final line segment, store it in the database */
void AddLineSeg(std::list<PlotLineSeg *> &region, double lat1, double lon1,
                double lat2, double lon2, double contour1, double contour2) {
  if (contour1 != contour2) /* this should not be possible */
    return;

  PlotLineSeg *seg = new PlotLineSeg(lat1, lon1, lat2, lon2, contour1);
  region.push_back(seg);
}

/* we generate contour maps by sampling the value at various
   latitude and longitude positions:

         lon1  lon3  lon2
    lat1  1-----*-----2
          |           |
    lat3  *           * lat4
          |           |
    lat2  3-----*-----4
              lon4

*/
void MagneticPlotMap::PlotRegion(std::list<PlotLineSeg *> &region, double lat1,
                                 double lon1, double lat2, double lon2) {
  double p1 = CachedCalcParameter(lat1, lon1);
  double p2 = CachedCalcParameter(lat1, lon2);
  double p3 = CachedCalcParameter(lat2, lon1);
  double p4 = CachedCalcParameter(lat2, lon2);

  if (std::isnan(p1) || std::isnan(p2) || std::isnan(p3) || std::isnan(p4))
    return;

  double ry1, ry2, ry3, ry4 = 0.0;
  double lon3, lon4, lat3, lat4 = 0.0;
  /* horizontal interpolate to determine intermediate longitudes as well
     as the contours they are on. */
  if (!Interpolate(lon1, lon2, p1, p2, false, lat1, lon3, ry1) ||
      !Interpolate(lon1, lon2, p3, p4, false, lat2, lon4, ry2)) {
    lon3 = (lon1 + lon2) / 2;
    PlotRegion(region, lat1, lon1, lat2, lon3);
    PlotRegion(region, lat1, lon3, lat2, lon2);
    return;
  }

  /* vertical interpolate */
  if (!Interpolate(lat1, lat2, p1, p3, true, lon1, lat3, ry3) ||
      !Interpolate(lat1, lat2, p2, p4, true, lon2, lat4, ry4)) {
    lat3 = (lat1 + lat2) / 2;
    PlotRegion(region, lat1, lon1, lat3, lon2);
    PlotRegion(region, lat3, lon1, lat2, lon2);
    return;
  }

  /* un-normalize contours */
  ry1 *= m_Spacing, ry2 *= m_Spacing, ry3 *= m_Spacing, ry4 *= m_Spacing;

  /* determine which interpolations need line segments */
  switch (((std::isnan(lat4) * 2 + std::isnan(lat3)) * 2 + std::isnan(lon4)) *
              2 +
          std::isnan(lon3)) {
    case 0: /* all 4 sides? need to recurse to get better resolution */
      lon3 = (lon1 + lon2) / 2;
      lat3 = (lat1 + lat2) / 2;
      PlotRegion(region, lat1, lon1, lat3, lon3);
      PlotRegion(region, lat1, lon3, lat3, lon2);
      PlotRegion(region, lat3, lon1, lat2, lon3);
      PlotRegion(region, lat3, lon3, lat2, lon2);
      break;
    case 1:
    case 2:
    case 4:
    case 8:
    case 7:
    case 11:
    case 13:
    case 14:
      break; /* impossible! */
    case 3:  /* horizontal */
      AddLineSeg(region, lat3, lon1, lat4, lon2, ry3, ry4);
      break;
    case 5: /* diagonal */
      AddLineSeg(region, lat2, lon4, lat4, lon2, ry2, ry4);
      break;
    case 6: /* diagonal */
      AddLineSeg(region, lat1, lon3, lat4, lon2, ry1, ry4);
      break;
    case 9: /* diagonal */
      AddLineSeg(region, lat3, lon1, lat2, lon4, ry2, ry3);
      break;
    case 10: /* diagonal */
      AddLineSeg(region, lat3, lon1, lat1, lon3, ry1, ry3);
      break;
    case 12: /* vertical */
      AddLineSeg(region, lat1, lon3, lat2, lon4, ry1, ry2);
      break;
    case 15: /* no intersections */
      break;
  }
}

/* rebuild the map at a given date */
bool MagneticPlotMap::Recompute(wxDateTime date) {
  if (!m_bEnabled) return true;

  UserDate.Year = date.GetYear();
  UserDate.Month = date.GetMonth();
  UserDate.Day = date.GetDay();

  char err[255];
  MAG_DateToYear(&UserDate, err);

  /* Time adjust the coefficients, Equation 19, WMM Technical report */
  MAG_TimelyModifyMagneticModel(UserDate, MagneticModel, TimedMagneticModel);

  /* clear out old data */
  ClearMap();

  wxGenericProgressDialog progressdialog(
      _("Building Magnetic Map"),
      m_type == DECLINATION_PLOT   ? _("Variation")
      : m_type == INCLINATION_PLOT ? _("Inclination")
                                   : _("Field Strength"),
      180, NULL,
      wxPD_SMOOTH | wxPD_ELAPSED_TIME | wxPD_REMAINING_TIME | wxPD_CAN_ABORT);

  int cachepage = 0;
  m_Cache[0].Initialize(m_Step);
  m_Cache[1].Initialize(m_Step);

  BuildParamCache(m_Cache[cachepage], -MAX_LAT);

  for (double lat = -MAX_LAT; lat + m_Step <= MAX_LAT; lat += m_Step) {
    if (!progressdialog.Update(lat + 90)) {
      return false;
    }

    cachepage = !cachepage;
    BuildParamCache(m_Cache[cachepage], lat + m_Step);

    int latind = floor((lat + MAX_LAT) / ZONE_SIZE);
    if (latind > LATITUDE_ZONES - 1) latind = LATITUDE_ZONES - 1;
    for (double lon = -180; lon + m_Step <= 180; lon += m_Step) {
      int lonind = floor((lon + 180) / ZONE_SIZE);
      PlotRegion(m_map[latind][lonind], lat, lon, lat + m_Step, lon + m_Step);
    }
  }

  return true;
}

/* draw a line segment in opengl from lat/lon and viewport */
void DrawLineSeg(pi_ocpnDC *dc, PlugIn_ViewPort &VP, double lat1, double lon1,
                 double lat2, double lon2) {
  /* avoid lines which cross over the view port the long way */
  if (lon1 + 180 < VP.clon && lon2 + 180 > VP.clon) return;
  if (lon1 + 180 > VP.clon && lon2 + 180 < VP.clon) return;
  if (lon1 - 180 < VP.clon && lon2 - 180 > VP.clon) return;
  if (lon1 - 180 > VP.clon && lon2 - 180 < VP.clon) return;

  wxPoint r1, r2;
  GetCanvasPixLL(&VP, &r1, lat1, lon1);
  GetCanvasPixLL(&VP, &r2, lat2, lon2);

  dc->DrawLine(r1.x, r1.y, r2.x, r2.y);
#if 0
    if(dc)
        dc->DrawLine(r1.x, r1.y, r2.x, r2.y);
    else {
        glBegin(GL_LINES);
        glVertex2i(r1.x, r1.y);
        glVertex2i(r2.x, r2.y);
        glEnd();
    }
#endif
}

/* reset the map and clear all the data so it can be reused */
void MagneticPlotMap::ClearMap() {
  for (int latind = 0; latind < LATITUDE_ZONES; latind++)
    for (int lonind = 0; lonind < LONGITUDE_ZONES; lonind++)
      m_map[latind][lonind].clear();
}

/* draw text of the value of a contour at a given location */
void MagneticPlotMap::DrawContour(pi_ocpnDC *dc, PlugIn_ViewPort &VP,
                                  double contour, double lat, double lon) {
  wxPoint r;

  GetCanvasPixLL(&VP, &r, lat, lon);

  double dist_squared = square(r.x - lastx) + square(r.y - lasty);
  /* avoid printing numbers on top of each other */
  if (dist_squared < 40000) return;

  lastx = r.x;
  lasty = r.y;

  wxString msg;
  msg.Printf(_T("%.0f"), contour);

  int w, h;
  dc->GetTextExtent(msg, &w, &h);
  dc->DrawText(msg, r.x - w / 2, r.y - h / 2);
#if 0
    if(dc) {
        int w, h;
        dc->GetTextExtent( msg, &w, &h);
        dc->DrawText(msg, r.x - w/2, r.y - h/2);
    } else {
        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

        int w, h;
        m_TexFont.GetTextExtent( msg, &w, &h);

        glEnable(GL_TEXTURE_2D);
        m_TexFont.RenderString(msg, r.x - w/2, r.y - h/2);
        glDisable(GL_TEXTURE_2D);

        glDisable(GL_BLEND);
    }
#endif
}

/* plot to dc, or opengl is dc is NULL */
void MagneticPlotMap::Plot(pi_ocpnDC *dc, PlugIn_ViewPort *vp, wxColour color) {
  if (!m_bEnabled) return;

  wxFont font(15, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_ITALIC,
              wxFONTWEIGHT_NORMAL);

  dc->SetPen(wxPen(color, 3));
  dc->SetTextForeground(color);
  dc->SetFont(font);
#if 0
    if(dc) {
        dc->SetPen(wxPen(color, 3));
        dc->SetTextForeground(color);
        dc->SetFont( font );
    } else {
        glLineWidth(3.0);
        glColor4ub(color.Red(), color.Green(), color.Blue(), color.Alpha());
        m_TexFont.Build( font );
    }
#endif
  int startlatind = floor((vp->lat_min + MAX_LAT) / ZONE_SIZE);
  if (startlatind < 0) startlatind = 0;

  int endlatind = floor((vp->lat_max + MAX_LAT) / ZONE_SIZE);
  if (endlatind > LATITUDE_ZONES - 1) endlatind = LATITUDE_ZONES - 1;

  double lon_min = vp->lon_min; /* expected +- 360 convert to +- 180 */
  if (lon_min < -180)
    lon_min += 360;
  else if (lon_min >= 180)
    lon_min -= 360;
  int startlonind = floor((lon_min + 180) / ZONE_SIZE);
  if (startlonind < 0) startlonind = LONGITUDE_ZONES - 1;
  if (startlonind > LONGITUDE_ZONES - 1) startlonind = 0;

  double lon_max = vp->lon_max; /* expected +- 360 convert to +- 180 */
  if (lon_max < -180)
    lon_max += 360;
  else if (lon_max >= 180)
    lon_max -= 360;
  int endlonind = floor((lon_max + 180) / ZONE_SIZE);
  if (endlonind < 0) endlonind = LONGITUDE_ZONES - 1;
  if (endlonind > LONGITUDE_ZONES - 1) endlonind = 0;

  for (int latind = startlatind; latind <= endlatind; latind++)
    for (int lonind = startlonind;; lonind++) {
      if (lonind > LONGITUDE_ZONES - 1) lonind = 0;
      for (std::list<PlotLineSeg *>::iterator it =
               m_map[latind][lonind].begin();
           it != m_map[latind][lonind].end(); it++) {
        DrawLineSeg(dc, *vp, (*it)->lat1, (*it)->lon1, (*it)->lat2,
                    (*it)->lon2);
        wxString msg;
        DrawContour(dc, *vp, (*it)->contour, ((*it)->lat1 + (*it)->lat2) / 2,
                    ((*it)->lon1 + (*it)->lon2) / 2);
      }
      if (lonind == endlonind) break;
    }
}
