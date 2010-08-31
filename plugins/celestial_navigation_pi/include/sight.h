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


#include <list>

enum OVERLAP {_IN,_ON,_OUT};

#ifdef __WXMSW__
#include <float.h>
#include <iostream>
#include <limits>
#define NAN std::numeric_limits<double>::quiet_NaN ()
#endif



WX_DECLARE_LIST(wxRealPoint, wxRealPointList);

//    Sight
//----------------------------------------------------------------------------

class Sight : public wxObject
{
public:
   enum BodyLimb {LOWER, CENTER, UPPER};

   Sight(wxString body, BodyLimb bodylimb, wxDateTime datetime,
         double timecertainty, double measurement, double measurementcertainty,
         double height, double temperature, double pressure, wxColour colour);

   ~Sight();

   void SetVisible(bool visible = true); ///< set visibility and make points selectable accordingly
   bool IsVisible() { return m_bVisible; }

   virtual void RebuildPolygons(double timeoffset) {}

   bool       m_bVisible;  // should this sight be drawn?
   wxString   m_Body;
   BodyLimb   m_BodyLimb;

   wxDateTime m_DateTime;      // Time for the sight
   float      m_TimeCertainty;

   float      m_Measurement; // Measurement angle in degrees (NaN is valid for all)
   float      m_MeasurementCertainty;

   float      m_Height; // Height above sea in meters
   float      m_Temperature; // Temperature in degrees celcius
   float      m_Pressure; // Pressure in millibars
      
   wxColour   m_Colour;     // Color of the sight

   virtual void Render(wxMemoryDC& dc, PlugIn_ViewPort &pVP);

   void BodyLocation(wxDateTime time, double &lat, double &lon);

protected:
   wxTextCtrl *txtSightTimeOffset;

   double CalcAngle(wxRealPoint p1, wxRealPoint p2);
   double ComputeStepSize(double certainty, double stepsize, double min, double max);
   wxRealPointList *MergePoints(wxRealPointList *p1, wxRealPointList *p2);
   wxRealPointList *ReduceToConvexPolygon(wxRealPointList *points);

   std::list<wxRealPointList*> polygons;

private:
   void DrawPolygon(wxMemoryDC& dc, PlugIn_ViewPort &pVP, wxRealPointList &area);
};

class AltitudeSight : public Sight
{
public:
   AltitudeSight(wxString body, BodyLimb bodylimb, wxDateTime datetime,
                 double timecertainty, double measurement, double measurementcertainty,
                 double height, double temperature, double pressure, wxColour colour);

   float      m_HeightCorrection, m_LimbCorrection, m_RefractionCorrection, m_ParallaxCorrection;
   float      m_CorrectedAltitude; /* after observer height, edge, refraction and parallax */

private:
   virtual void RebuildPolygons(double timeoffset);
   wxRealPoint DistancePoint(wxDateTime datetime, double altitude, double trace);
   void BuildAltitudeLineOfPosition(double altitudemin, double altitudemax, double altitudestep,
                                        double tracestep, double timemin, double timemax, double timestep);
};

class AzimuthSight : public Sight
{
public:
   AzimuthSight(wxString body, wxDateTime datetime,
                double timecertainty, double measurement, double measurementcertainty,
                bool magneticnorth, wxColour colour);

   virtual void RebuildPolygons(double timeoffset);

   bool       m_bMagneticNorth; // if azimuth angle is in magnetic coordinates

private:
   bool BearingPoint(wxDateTime datetime, double altitude, double trace,
                     double &rlat, double &rlon);
   void BuildBearingLineOfPosition(double altitudestep,
                                   double azimuthmin, double azimuthmax, double azimuthstep,
                                   double timemin, double timemax, double timestep);

};

/* although traditionally called "lunar" because it involves the moon,
   we support any 2 bodies */
class LunarSight : public Sight
{
public:
   LunarSight(wxString body1, wxString body2, BodyLimb bodylimb, wxDateTime datetime,
              double timecertainty, double measurement, double measurementcertainty,
              double height, double temperature, double pressure, wxColour colour);
   
   float timeoffset;

   wxString   m_Body1, m_Body2;

private:
   double DistanceTwoPoints(double timeoffset);
};

WX_DECLARE_LIST(Sight, SightList);                    // establish class Sight as list member
