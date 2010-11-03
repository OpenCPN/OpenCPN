/*
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 3
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA  02110-1301, USA.

    ---
    Copyright (C) 2010, Sean D'Epagnier <geckosenator@gmail.com>
 */

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif //precompiled headers

#include <wx/tokenzr.h>
#include <wx/sstream.h>
#include <wx/image.h>
#include <wx/filename.h>
#include <wx/graphics.h>


#include <stdlib.h>
#include <math.h>
#include <time.h>

#include <wx/listimpl.cpp>


#include "../../../include/ocpn_plugin.h"

#include "sight.h"
#include "geodesic.h"

WX_DEFINE_LIST ( SightList );
WX_DEFINE_LIST ( wxRealPointList );

//-----------------------------------------------------------------------------
//          Sight Implementation
//-----------------------------------------------------------------------------

Sight::Sight(wxString body, BodyLimb bodylimb, wxDateTime datetime,
             double timecertainty, double measurement, double measurementcertainty,
             double height, double temperature, double pressure, wxColour colour)
   : m_bVisible(true), m_Body(body), m_BodyLimb(bodylimb),
     m_DateTime(datetime), m_TimeCertainty(timecertainty),
     m_Measurement(measurement), m_MeasurementCertainty(measurementcertainty),
     m_Height(height), m_Temperature(temperature), m_Pressure(pressure), m_Colour(colour)
{}

Sight::~Sight ( )
{
}

void Sight::SetVisible(bool visible)
{
      m_bVisible = visible;
}

#include "astrolabe/astrolabe.hpp"

using namespace astrolabe;

using namespace astrolabe::calendar;
using namespace astrolabe::constants;
using namespace astrolabe::dynamical;
using namespace astrolabe::elp2000;
using namespace astrolabe::nutation;
using namespace astrolabe::sun;
using namespace astrolabe::util;
using namespace astrolabe::vsop87d;

//extern ChartCanvas      *cc1;

/* calculate what longitude the body for this sight is directly over at a given time */ 
void Sight::BodyLocation(wxDateTime time, double &lat, double &lon)
{
   astrolabe::globals::vsop87d_text_path = (const char *)GetpSharedDataLocation()->mb_str();
   astrolabe::globals::vsop87d_text_path.append("plugins/celestial_navigation/data/");
   astrolabe::globals::vsop87d_text_path.append("vsop87d.txt");

   time.MakeUTC(); /* convert to universal time */

   double jd = cal_to_jd(time.GetYear(), time.GetMonth()+1, time.GetDay()
                  + (time.GetHour()
                     + (time.GetMinute()
                        + (time.GetSecond()
                           + (time.GetMillisecond() / 1000.0))
                        / 60.0)
                     / 60.0)
                  / 24.0);

    double l, b, r;
    double ra, dec;
    // nutation in longitude
    const double deltaPsi = nut_in_lon(jd);
    
    // apparent obliquity
    const double eps = obliquity(jd) + nut_in_obl(jd);

       Sun sun;
       sun.dimension3(jd, l, b, r);

       // correct vsop coordinates    
       vsop_to_fk5(jd, l, b);
       
       // nutation in longitude
       l += deltaPsi;
       
       // aberration
       l += aberration_low(r);

       // sun position is the definition longitude
       double lsub = l; 

    if(!m_Body.Cmp(_T("Sun"))) {
       // equatorial coordinates
       ecl_to_equ(l, b, eps, ra, dec);

    } else
    if(!m_Body.Cmp(_T("Moon"))) {
          ELP2000 moon;
          moon.dimension3(jd, l, b, r);

          // nutation in longitude
          l += deltaPsi;

          // equatorial coordinates
          ecl_to_equ(l, b, eps, ra, dec);

       }
    else {
       vPlanets planet;
       if(!m_Body.Cmp(_T("Mercury")))
          planet = vMercury;
       else if(!m_Body.Cmp(_T("Venus")))
          planet = vVenus;
       else if(!m_Body.Cmp(_T("Mars")))
          planet = vMars;
       else if(!m_Body.Cmp(_T("Jupiter")))
          planet = vJupiter;
       else if(!m_Body.Cmp(_T("Saturn")))
          planet = vSaturn;
       else { /* star maybe */
          if(!m_Body.Cmp(_T("Alpheratz"))) ra = 0.0365979981624047, dec = 0.508585349622516;
          else if(!m_Body.Cmp(_T("Ankaa"))) ra = 0.289219237568502, dec = -0.738378993348721;
          else if(!m_Body.Cmp(_T("Schedar"))) ra = 0.176746620828405, dec = 0.986762578179113;
          else if(!m_Body.Cmp(_T("Diphda"))) ra = 0.190182710825649, dec = -0.284027125450659;
          else if(!m_Body.Cmp(_T("Achernar"))) ra = 0.426358483543957, dec = -0.998969740640864;
          else if(!m_Body.Cmp(_T("Hamal"))) ra = 0.554898863177301, dec = 0.409496532815273;
          else if(!m_Body.Cmp(_T("Polaris"))) ra = 0.662403356568365, dec = 1.55795361238231;
          else if(!m_Body.Cmp(_T("Acamar"))) ra = 0.777812962468281, dec = -0.703449961435399;
          else if(!m_Body.Cmp(_T("Menkar"))) ra = 0.795346540133816, dec = 0.071379118269757;
          else if(!m_Body.Cmp(_T("Mirfak"))) ra = 0.891528726329137, dec = 0.870240557591617;
          else if(!m_Body.Cmp(_T("Aldebaran"))) ra = 1.20393095418572, dec = 0.288141690680868;
          else if(!m_Body.Cmp(_T("Rigel"))) ra = 1.37243034888306, dec = -0.143145651152089;
          else if(!m_Body.Cmp(_T("Capella"))) ra = 1.38181782770328, dec = 0.802816394191974;
          else if(!m_Body.Cmp(_T("Bellatrix"))) ra = 1.41865452145751, dec = 0.110823559364829;
          else if(!m_Body.Cmp(_T("Elnath"))) ra = 1.42371695076379, dec = 0.499297460743863;
          else if(!m_Body.Cmp(_T("Alnilam"))) ra = 1.46700741394297, dec = -0.0209774031679285;
          else if(!m_Body.Cmp(_T("Betelgeuse"))) ra = 1.54972913370916, dec = 0.129277633374139;
          else if(!m_Body.Cmp(_T("Canopus"))) ra = 1.67530590791159, dec = -0.919712778207749;
          else if(!m_Body.Cmp(_T("Sirius"))) ra = 1.76779435200004, dec = -0.291751259436422;
          else if(!m_Body.Cmp(_T("Adhara"))) ra = 1.82660341749553, dec = -0.505655821260435;
          else if(!m_Body.Cmp(_T("Procyon"))) ra = 2.00406704139727, dec = 0.0911692127326482;
          else if(!m_Body.Cmp(_T("Pollux"))) ra = 2.03035606325544, dec = 0.489152763555466;
          else if(!m_Body.Cmp(_T("Avior"))) ra = 2.19262805045961, dec = -1.03864058972501;
          else if(!m_Body.Cmp(_T("Suhail"))) ra = 2.3910865308218, dec = -0.758040127372437;
          else if(!m_Body.Cmp(_T("Miaplacidus"))) ra = 2.41379035550816, dec = -1.21679507312234;
          else if(!m_Body.Cmp(_T("Alphard"))) ra = 2.47656717980308, dec = -0.151121122246412;
          else if(!m_Body.Cmp(_T("Regulus"))) ra = 2.65435490407471, dec = 0.20886743009561;
          else if(!m_Body.Cmp(_T("Dubhe"))) ra = 2.65435490407471, dec = 0.20886743009561;
          else if(!m_Body.Cmp(_T("Denebola"))) ra = 3.09385626957378, dec = 0.254330430646302;
          else if(!m_Body.Cmp(_T("Gienah"))) ra = 3.21056008074728, dec = -0.306164425958095;
          else if(!m_Body.Cmp(_T("Acrux"))) ra = 3.25764947120958, dec = -1.10128690460105;
          else if(!m_Body.Cmp(_T("Gacrux"))) ra = 3.2775756189358, dec = -0.996815713455695;
          else if(!m_Body.Cmp(_T("Alioth"))) ra = 3.37732845789249, dec = 0.97668334053394;
          else if(!m_Body.Cmp(_T("Spica"))) ra = 3.51331719009724, dec = -0.194801816805651;
          else if(!m_Body.Cmp(_T("Alkaid"))) ra = 3.61082442298847, dec = 0.860680031800137;
          else if(!m_Body.Cmp(_T("Hadar"))) ra = 3.68187386795507, dec = -1.0537085989339;
          else if(!m_Body.Cmp(_T("Menkent"))) ra = 3.69437478872248, dec = -0.634753462707325;
          else if(!m_Body.Cmp(_T("Arcturus"))) ra = 3.73352834160889, dec = 0.334792935627001;
          else if(!m_Body.Cmp(_T("Rigil"))) ra = 3.83801502982356, dec = -1.06175315112964;
          else if(!m_Body.Cmp(_T("Zubenelgenubi"))) ra = 3.88635809755977, dec = -0.279201774882576;
          else if(!m_Body.Cmp(_T("Kochab"))) ra = 3.88643372849402, dec = 1.29425860309002;
          else if(!m_Body.Cmp(_T("Alphecca"))) ra = 4.07834490432777, dec = 0.466259352479109;
          else if(!m_Body.Cmp(_T("Antares"))) ra = 4.31707190480797, dec = -0.461324458259779;
          else if(!m_Body.Cmp(_T("Atria"))) ra = 4.40113132490715, dec = -1.2047619975572;
          else if(!m_Body.Cmp(_T("Sabik"))) ra = 4.49587361446958, dec = -0.274451570435065;
          else if(!m_Body.Cmp(_T("Shaula"))) ra = 4.59724088298436, dec = 0.647730470509584;
          else if(!m_Body.Cmp(_T("Rasalhague"))) ra = 4.6030222861316, dec = 0.219213354050488;
          else if(!m_Body.Cmp(_T("Eltanin"))) ra = 4.6975827705636, dec = 0.898652093745679;
          else if(!m_Body.Cmp(_T("Kaus Australis"))) ra = 4.81785922708271, dec = -0.600124675906715;
          else if(!m_Body.Cmp(_T("Vega"))) ra = 4.87356551168385, dec = 0.676903120509757;
          else if(!m_Body.Cmp(_T("Nunki"))) ra = 4.95353021482492, dec = -0.458964385260138;
          else if(!m_Body.Cmp(_T("Altair"))) ra = 5.1957723884129, dec = 0.154781417057421;
          else if(!m_Body.Cmp(_T("Peacock"))) ra = 5.34789972206191, dec = -0.964556211114666;
          else if(!m_Body.Cmp(_T("Deneb"))) ra = 5.41676750546352, dec = 0.790289933439844;
          else if(!m_Body.Cmp(_T("Enif"))) ra = 5.69058479415959, dec = 0.172351457559912;
          else if(!m_Body.Cmp(_T("Al Na'ir"))) ra = 5.7955112253515, dec = -0.819623585215376;
          else if(!m_Body.Cmp(_T("Fomalhaut"))) ra = 6.01113938223019, dec = -0.517005309535209;
          else if(!m_Body.Cmp(_T("Markab"))) ra = 6.04216260968439, dec = 0.265381676088868;
          else {
             wxString s;
             s.Printf ( _T ( "Unknown celestial body: " ) + m_Body);
             wxLogMessage ( s );
          }
          goto notplanet;
       }
       geocentric_planet(jd, planet, deltaPsi, eps, days_per_second, ra, dec);
    notplanet:;
    }

    // account for earth's hour angle
    ra -= lsub;
    ra += M_PI - M_PI/180*15*
       (time.GetHour()
        + (time.GetMinute()
           + (time.GetSecond()
              + (time.GetMillisecond() / 1000.0))
           / 60.0)
        / 60.0);
     
   lat = dec*180/M_PI;
   lon = ra*180/M_PI;
 }

extern "C" int geomag_calc(double latitude, double longitude, double alt,
                int day, int month, double year,
                           double results[14]);

/* Combine two lists of points by appending p2 to p1 */
wxRealPointList *Sight::MergePoints(wxRealPointList *p1, wxRealPointList *p2)
{
   /* combine lists of points */
   wxRealPointList *p = new wxRealPointList;
   wxRealPointList::iterator it;
   for (it = p1->begin(); it != p1->end(); ++it)
      p->Append(new wxRealPoint(**it));
   for (it = p2->begin(); it != p2->end(); ++it)
      p->Append(new wxRealPoint(**it));
   return p;
}

/* give the angle between two points from 0 to 2 PI */
double Sight::CalcAngle(wxRealPoint p1, wxRealPoint p2)
{
#if 1
   /* rectangular coords */
   double phi = atan2(p1.y - p2.y, p1.x - p2.x);
   if(phi < 0)
      phi += 2*M_PI;
   return phi;
#else
   /* spherical coords (not sure why this is not working right) */
   double phi;
   DistanceBearingMercator(p1.x, p1.y, p2.x, p2.y, &phi, 0);
   if(phi < 0)
      phi += 2*M_PI;
   return phi;
#endif
}

/* take a list of points, and return a list of points
   which form a convex polygon which encompasses all the points with vertices at points. */
wxRealPointList *Sight::ReduceToConvexPolygon(wxRealPointList *points)
{
   wxRealPointList *polygon = new wxRealPointList;
   wxRealPointList::iterator it, min;
   /* get min y point to start out at */
   for (min = it = points->begin(); it != points->end(); ++it)
      if((*it)->y < (*min)->y)
         min = it;

   double theta = 0;
   while(!points->IsEmpty()) {
      polygon->Append(*min);
      points->DeleteObject(*min);

      /* delete duplicates (optimization) */
      it = points->begin();
      while(it != points->end())
         if(**it == *polygon->back()) {
            wxRealPointList::iterator l = it;
            ++it;
            points->DeleteObject(*l);
         } else
            ++it;

      double minphi = 2*M_PI, maxdist = 0;
      for (min = it = points->begin(); it != points->end(); ++it) {
         double phi = CalcAngle(**it, *polygon->back());
         double dist = hypot((*it)->x - polygon->back()->x,
                             (*it)->y - polygon->back()->y);
         if(maxdist == 0)
            maxdist = dist;

         if((phi >= theta && phi < minphi)
            || (phi == minphi && dist > maxdist)) {
            min = it;
            minphi = phi;
            maxdist = dist;
         }
      }

      if(polygon->size() > 1 &&
         CalcAngle(*polygon->front(), *polygon->back()) < minphi)
         break;

      theta = minphi;
   }

   return polygon;
}


// Calculates if two boxes intersect. If so, the function returns _ON.
// If they do not intersect, two scenario's are possible:
// other is outside this -> return _OUT
// other is inside this -> return _IN

static OVERLAP Intersect(PlugIn_ViewPort *vp,
       double lat_min, double lat_max, double lon_min, double lon_max, double Marge)
{

    if (((vp->lon_min - Marge) > (lon_max + Marge)) ||
         ((vp->lon_max + Marge) < (lon_min - Marge)) ||
         ((vp->lat_max + Marge) < (lat_min - Marge)) ||
         ((vp->lat_min - Marge) > (lat_max + Marge)))
        return _OUT;

    // Check if other.bbox is inside this bbox
    if ((vp->lon_min <= lon_min) &&
         (vp->lon_max >= lon_max) &&
         (vp->lat_max >= lat_max) &&
         (vp->lat_min <= lat_min))
        return _IN;

    // Boundingboxes intersect
    return _ON;
}

/* Draw a polygon (specified in lat/lon coords) to dc given a list of points */
void Sight::DrawPolygon(wxMemoryDC& dc, PlugIn_ViewPort &VP, wxRealPointList &area)
{
   int n = area.size();
   wxPoint *ppoints = new wxPoint[n];
   bool rear1 = false, rear2 = false;
   wxRealPointList::iterator it = area.begin();

   double minx = 1000;
   double maxx = -1000;
   double miny = 1000;
   double maxy = -1000;

   for(int i=0; i<n && it != area.end(); i++, it++) {
      wxPoint r;

      /* don't draw areas crossing opposite from center longitude */
      double lon = (*it)->y - VP.clon;
      while(lon < 0)
         lon += 360;
      while(lon >= 360)
         lon -= 360;
      if(lon > 90 && lon <= 180)
         rear1 = true;
      if(lon > 180 && lon < 270)
         rear2 = true;

      while((*it)->y - VP.clon < -180)
         (*it)->y += 360;
      while((*it)->y - VP.clon > 180)
         (*it)->y -= 360;

      minx = wxMin(minx, (*it)->x);
      miny = wxMin(miny, (*it)->y);
      maxx = wxMax(maxx, (*it)->x);
      maxy = wxMax(maxy, (*it)->y);

      GetCanvasPixLL(&VP, &r, (*it)->x, (*it)->y);

      ppoints[i] = r;
   }

   if(!(rear1 && rear2)
      &&      Intersect(&VP, minx, maxx,
                        miny, maxy, 0) != _OUT
                                     )
      dc.DrawPolygon(n, ppoints);

   delete[] ppoints;
}

/* Compute trace areas for one dimension, given center certainty, and constant */
double Sight::ComputeStepSize(double certainty, double stepsize, double min, double max)
{
   return (max - min) / (floor(certainty / stepsize) + 1);
}

/* render the area of position for this sight */
void Sight::Render( wxMemoryDC& dc, PlugIn_ViewPort &VP )
{
      if ( !m_bVisible )
            return;
      
      dc.SetPen ( wxPen(m_Colour, 1) );
      dc.SetBrush ( wxBrush(m_Colour) );

      std::list<wxRealPointList*>::iterator it = polygons.begin();
      while(it != polygons.end()) {
            DrawPolygon(dc, VP, **it);
            it++;
      }
}

AltitudeSight::AltitudeSight(wxString body, BodyLimb bodylimb, wxDateTime datetime,
             double timecertainty, double measurement, double measurementcertainty,
             double height, double temperature, double pressure, wxColour colour)
   : Sight(body, bodylimb, datetime, timecertainty, measurement, measurementcertainty,
           height, temperature, pressure, colour)
{      

   /* correct for height of observer
      The dip of the sea horizon in minutes = 1.753*sqrt(height) */
   m_HeightCorrection = 1.753*sqrt(m_Height) * cos(M_PI/180 * m_Measurement) / 60.0;
      
   /* compensate for refraction */
#if 1
   m_RefractionCorrection = (.267*m_Pressure/(m_Temperature + 273.15)) /
      tan(M_PI/180 * (m_Measurement + .04848 / (tan(M_PI/180 * m_Measurement) + .028)));
#else
      if(m_Measurement >= 15)
         m_RefractionCorrection = (.97127 / tan(M_PI/180 * m_Measurement)
                                   - .00137 / pow(tan(M_PI/180 * m_Measurement), 3)) / 60.0;
      else
         m_RefractionCorrection = ((34.122 +
                                    4.197*m_Measurement +
                                    .00427*m_Measurement*m_Measurement)
                                   / (1 + .505*m_Measurement
                                      + .0845*m_Measurement*m_Measurement))
            / 60.0;
#endif 
      /* correct for limb shot */
      m_LimbCorrection = 0;
      if(!m_Body.Cmp(_T("Moon")) || !m_Body.Cmp(_T("Sun"))) {
         if(m_BodyLimb == UPPER)
            m_LimbCorrection =  .25;
         else if(m_BodyLimb == LOWER)
            m_LimbCorrection = -.25;
      }
      
      /* correct for parallax 
         earth radius: 6357 km
         distance to moon: 384400 km */
      if(!m_Body.Cmp(_T("Moon")))
         m_ParallaxCorrection = -180/M_PI*asin(6357/384400.0)
            * cos(M_PI/180*m_Measurement);
      else
         m_ParallaxCorrection = 0;
      
      /* apply corrections */
      m_CorrectedAltitude = m_Measurement - m_HeightCorrection - m_RefractionCorrection -
         m_LimbCorrection - m_ParallaxCorrection;
}

void AltitudeSight::RebuildPolygons(double timeoffset)
{
      polygons.clear();

      double altitudemin, altitudemax, altitudestep;
      altitudemin = m_Measurement - m_MeasurementCertainty;
      altitudemax = m_Measurement + m_MeasurementCertainty;
      altitudestep = ComputeStepSize(m_MeasurementCertainty, 1, altitudemin, altitudemax);

      double timemin, timemax, timestep;
      timemin = timeoffset - m_TimeCertainty;
      timemax = timeoffset + m_TimeCertainty;
      timestep = ComputeStepSize(m_TimeCertainty, 1, timemin, timemax);

      BuildAltitudeLineOfPosition(1, altitudemin, altitudemax, altitudestep,
                                  timemin, timemax, timestep);
}


/* Calculate latitude and longitude position for a sight taken with time, altitude,
    and trace angle */
 wxRealPoint AltitudeSight::DistancePoint(wxDateTime datetime, double altitude, double trace)
 {
    double lat, lon, rlat, rlon;
    BodyLocation(datetime, lat, lon);
    ll_gc_ll(lat, lon, trace, 60*(90-altitude),
             &rlat, &rlon);
    
    return wxRealPoint(rlat, rlon);
 }

void AltitudeSight::BuildAltitudeLineOfPosition(double tracestep,
                                        double altitudemin, double altitudemax, double altitudestep,
                                        double timemin, double timemax, double timestep)
{
   wxRealPointList *p, *l = new wxRealPointList;
   for(double trace=-180; trace<=180; trace+=tracestep) {
      p = new wxRealPointList;
      for(double altitude=altitudemin; altitude<=altitudemax
             && fabs(altitude) <= 90; altitude+=altitudestep)
         for(double time=timemin; time<=timemax; time+=timestep)
            p->Append(new wxRealPoint(DistancePoint(m_DateTime + wxTimeSpan(0, 0, time),
                                                    altitude, trace)));
      wxRealPointList *m = MergePoints(l, p);
      wxRealPointList *n = ReduceToConvexPolygon(m);
      polygons.push_back(n);

      m->DeleteContents(true);
      delete m;
      l->DeleteContents(true);
      delete l;

      l = p;
   }
}

AzimuthSight::AzimuthSight(wxString body, wxDateTime datetime,
                           double timecertainty, double measurement, double measurementcertainty,
                           bool magneticnorth, wxColour colour)
   : Sight(body, CENTER, datetime, timecertainty, measurement, measurementcertainty,
           0, 0, 0, colour),
     m_bMagneticNorth(magneticnorth)
{      
   while(m_Measurement < 0)
      m_Measurement += 360;
   while(m_Measurement >= 360)
      m_Measurement -= 360;   
}

void AzimuthSight::RebuildPolygons(double timeoffset)
{
   polygons.clear();

   double azimuthmin, azimuthmax, azimuthstep;
   azimuthmin = m_Measurement - m_MeasurementCertainty;
   azimuthmax = m_Measurement + m_MeasurementCertainty;
   azimuthstep = ComputeStepSize(m_MeasurementCertainty, 1, azimuthmin, azimuthmax);

   double timemin, timemax, timestep;
   timemin = timeoffset - m_TimeCertainty;
   timemax = timeoffset + m_TimeCertainty;
   timestep = ComputeStepSize(m_TimeCertainty, 1, timemin, timemax);

   BuildBearingLineOfPosition(1, azimuthmin, azimuthmax, azimuthstep,
                              timemin, timemax, timestep);
}

/* find latitude and longitude which sees the body at the time, altitude and bearing
   iterative method so we can easily support magnetic variation */

bool AzimuthSight::BearingPoint(wxDateTime datetime, double altitude, double bearing,
                                double &rlat, double &rlon)
{
    double lat, lon;

    BodyLocation(datetime, lat, lon);

    while(bearing < 0)
       bearing += 360;
    while(bearing > 360)
       bearing -= 360;

    double b, l = 1000, mdl = 1000;
    double trace = 0, tracestep = 240;
    while(fabs(tracestep) > m_MeasurementCertainty/16) {
       ll_gc_ll(lat, lon, trace, 60*(90-altitude), &rlat, &rlon);
       ll_gc_ll_reverse(rlat, rlon, lat, lon, &b, 0);

       /* apply magnetic correction to bearing */
       if(m_bMagneticNorth) {
          double results[14];

          geomag_calc(rlat, rlon, m_Height,
                   m_DateTime.GetDay(), m_DateTime.GetMonth(), m_DateTime.GetYear(),
                   results);
          b -= results[0];
       }

       double mdb = bearing - b;
       while(mdb > 180)
          mdb -= 360;
       while(mdb < -180)
          mdb += 360;
          
       if(mdb * mdl < 0 || fabs(mdb) >= fabs(mdl))
          tracestep = -tracestep/2.1;

       l = b;
       mdl = mdb;

       trace += tracestep;

       while(trace > 180)
          trace -= 360;
       while(trace < -180)
          trace += 360;
    }

    return fabs(bearing - b) < 64*fabs(tracestep);
 }


void AzimuthSight::BuildBearingLineOfPosition(double altitudestep,
                                              double azimuthmin, double azimuthmax, double azimuthstep,
                                              double timemin, double timemax, double timestep)
{
   wxRealPointList *p, *l = new wxRealPointList;
   for(double altitude=90; altitude>=0
          && fabs(altitude) <= 90; altitude-=altitudestep) {
      p = new wxRealPointList;
      double lat, lon, llat = 1000, llon;
      bool skip_this_altitude = false;
      for(double azimuth=azimuthmin; azimuth<=azimuthmax; azimuth+=azimuthstep)
         for(double time=timemin; time<=timemax; time+=timestep)
            if(BearingPoint(m_DateTime + wxTimeSpan(0, 0, time),
                            altitude, azimuth, lat, lon)) {
               if(llat != 1000) {
                  double d;
                  ll_gc_ll_reverse(llat, llon, lat, lon, 0, &d);
                  /* if we move more than 30 degrees, then this span is no good */
                  if(d > 30*60)
                     skip_this_altitude = true;
               }
               p->Append(new wxRealPoint(lat, lon));
               llat = lat;
               llon = lon;
            }

      wxRealPointList *m = MergePoints(l, p);
      
      if(!skip_this_altitude) {
         wxRealPointList *n = ReduceToConvexPolygon(m);
         if(n->size() > 1)
            polygons.push_back(n);
         else
            delete n;
      }
      
      m->DeleteContents(true);
      delete m;
      l->DeleteContents(true);
      delete l;
      
      l = p;
   }
}

LunarSight::LunarSight(wxString body1, wxString body2, BodyLimb bodylimb, wxDateTime datetime,
             double timecertainty, double measurement, double measurementcertainty,
             double height, double temperature, double pressure, wxColour colour)
   : Sight(body1, bodylimb, datetime, timecertainty, measurement, measurementcertainty,
           height, temperature, pressure, colour),
     m_Body1(body1), m_Body2(body2)
{
   double timemin = -m_TimeCertainty;
   double timemax = +m_TimeCertainty;
   
   double distancemin = DistanceTwoPoints(timemin);
   double distancemax = DistanceTwoPoints(timemax);

   if(distancemin * distancemax > 0) {
      timeoffset = NAN;
      ::wxMessageBox(_("Lunar shot failed to converge on time interval"),
                     _("Lunar Shot"), wxOK | wxICON_INFORMATION);
      return;
   }

   double timemid;
   do {
      timemid = (timemin + timemax) / 2;
      double distancemid = DistanceTwoPoints(timemid);

      if(distancemin * distancemid < 0)
         timemax = timemid;
      else if(distancemid * distancemax < 0)
         timemin = timemid;
      else
         break;
   } while (timemax - timemin > .1);

   timeoffset = timemid;
   wxString msg;
   msg << _("Lunar shot found time offset of ") << timeoffset << _(" seconds)");
   ::wxMessageBox(msg, _("Lunar Shot"), wxOK | wxICON_INFORMATION);
}

double LunarSight::DistanceTwoPoints(double timeoffset)
{
   double lat1, lon1, lat2, lon2;
   m_Body = m_Body2;
   BodyLocation(m_DateTime + wxTimeSpan(0, 0, timeoffset), lat2, lon2);
   m_Body = m_Body1;
   BodyLocation(m_DateTime + wxTimeSpan(0, 0, timeoffset), lat1, lon1);

   double d;
   ll_gc_ll_reverse(lat1, lon1, lat2, lon2, 0, &d);
   return d * 60;
}
