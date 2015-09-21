/******************************************************************************
 * $Id: MagneticPlotMap.h,v 1.0 2011/02/26 01:54:37 nohal Exp $
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

#include <list>
#include "TexFont.h"

enum MagneticPlotType {DECLINATION, INCLINATION, FIELD_STRENGTH};

/* must be a power of 2, and also divide 360 and 176;
   really only 8 works without more modifications */
#define ZONE_SIZE 8

/* divisible by 8 and any closer to the pole than this plots
   horribly anyway on a flattened earth. */
#define MAX_LAT 88
#define LATITUDE_ZONES (2*MAX_LAT/ZONE_SIZE) /* perfectly divisible */
#define LONGITUDE_ZONES (360/ZONE_SIZE)

/* a single line segment in the plot */
class PlotLineSeg
{
public:
    PlotLineSeg(double _lat1, double _lon1, double _lat2, double _lon2, double _contour)
        : lat1(_lat1), lon1(_lon1), lat2(_lat2), lon2(_lon2), contour(_contour) {}
    double lat1, lon1, lat2, lon2;
    double contour;
};

/* cache values computed from wmm to improve speed */
class ParamCache
{
public:
ParamCache() : values(NULL), m_step(0), m_lat(0.0) {}
    ~ParamCache() { delete [] values; }
    void Initialize(double step);
    bool Read(double lat, double lon, double &value);

    double *values;
    double m_step;
    double m_lat;
};

/* main model map suitable for a single plot type */
class MagneticPlotMap
{
public:
    MagneticPlotMap(MagneticPlotType type,
                    WMMtype_MagneticModel *&mm,
                    WMMtype_MagneticModel *&tmm,
                    WMMtype_Ellipsoid *ellip)
        : m_type(type), m_bEnabled(false), m_Spacing(0.0), m_Step(0.0), m_PoleAccuracy(0.0), MagneticModel(mm), TimedMagneticModel(tmm), Ellip(ellip), lastx(0), lasty(0)
    {
        UserDate.Year = 2015;
        UserDate.Month = 1;
        UserDate.Day = 1;
        UserDate.DecimalYear = 2015.0;
    }

    ~MagneticPlotMap() { ClearMap(); }

    void ConfigureAccuracy(int stepsize, int poleaccuracy);
    double CalcParameter(double lat, double lon);
    void BuildParamCache(ParamCache &cache, double lat);
    double CachedCalcParameter(double lat, double lon);
    bool Interpolate(double x1, double x2, double y1, double y2, bool lat,
                     double lonval, double &rx, double &ry);
    void PlotRegion(std::list<PlotLineSeg*> &region,
                    double lat1, double lon1, double lat2, double lon2);
    bool Recompute(wxDateTime date);
    void Plot(wxDC *dc, PlugIn_ViewPort *vp, wxColour color);

    void ClearMap();
    void DrawContour(wxDC *dc, PlugIn_ViewPort &VP, double contour, double lat, double lon);

    MagneticPlotType m_type;
    bool m_bEnabled;
    double m_Spacing;
    double m_Step;
    double m_PoleAccuracy;

    /* two caches for all longitudes alternate
       places (step over each other) to cover the two latitudes
       currently being built */
    ParamCache m_Cache[2];

    WMMtype_MagneticModel *&MagneticModel;
    WMMtype_MagneticModel *&TimedMagneticModel;
    WMMtype_Ellipsoid *Ellip;
    WMMtype_Date UserDate;

    /* the line segments for the entire globe split into zones */
    std::list<PlotLineSeg*> m_map[LATITUDE_ZONES][LONGITUDE_ZONES];

    TexFont m_TexFont;
    int lastx, lasty; /* when rendering to prevent overcluttering */
};
