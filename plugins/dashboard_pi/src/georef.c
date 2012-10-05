/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  OpenCPN Georef utility
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.             *
 ***************************************************************************

 ***************************************************************************
 *  Parts of this file were adapted from source code found in              *
 *  John F. Waers (jfwaers@csn.net) public domain program MacGPS45         *
 ***************************************************************************
 * This is a stripped down version of a file from OpenCPN's core, please   *
 * also fix any bugs there...                                              *
 ***************************************************************************
 */

#include <stdlib.h>
#include <string.h>

#include "georef.h"

#ifdef __MSVC__
#define snprintf mysnprintf
#endif

void toSM_ECC(double lat, double lon, double lat0, double lon0, double *x, double *y)
{
      double xlon, z, x1, s, y3, s0, y30, y4;

      double falsen;
      double test;
      double ypy;

      double f = 1.0 / WGSinvf;       // WGS84 ellipsoid flattening parameter
      double e2 = 2 * f - f * f;      // eccentricity^2  .006700
      double e = sqrt(e2);

      xlon = lon;

      z = WGS84_semimajor_axis_meters * mercator_k0;

      x1 = (xlon - lon0) * DEGREE * z;
      *x = x1;

// y =.5 ln( (1 + sin t) / (1 - sin t) )
      s = sin(lat * DEGREE);
      y3 = (.5 * log((1 + s) / (1 - s))) * z;

      s0 = sin(lat0 * DEGREE);
      y30 = (.5 * log((1 + s0) / (1 - s0))) * z;
      y4 = y3 - y30;
//      *y = y4;

    //Add eccentricity terms

      falsen =  z *log(tan(PI/4 + lat0 * DEGREE / 2)*pow((1. - e * s0)/(1. + e * s0), e/2.));
      test =    z *log(tan(PI/4 + lat  * DEGREE / 2)*pow((1. - e * s )/(1. + e * s ), e/2.));
      ypy = test - falsen;

      *y = ypy;
}

#define TOL 1e-10
#define CONV      1e-10
#define N_ITER    10
#define I_ITER 20
#define ITOL 1.e-12

/* --------------------------------------------------------------------------------- */
/*
      Geodesic Forward and Reverse calculation functions
      Abstracted and adapted from PROJ-4.5.0 by David S.Register

      Original source code contains the following license:

      Copyright (c) 2000, Frank Warmerdam

 Permission is hereby granted, free of charge, to any person obtaining a
 copy of this software and associated documentation files (the "Software"),
 to deal in the Software without restriction, including without limitation
 the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included
 in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 DEALINGS IN THE SOFTWARE.
*/
/* --------------------------------------------------------------------------------- */




#define DTOL                 1e-12

#define HALFPI  1.5707963267948966
#define SPI     3.14159265359
#define TWOPI   6.2831853071795864769
#define ONEPI   3.14159265358979323846
#define MERI_TOL 1e-9

static double th1,costh1,sinth1,sina12,cosa12,M,N,c1,c2,D,P,s1;
static int merid, signS;

/*   Input/Output from geodesic functions   */
static double al12;           /* Forward azimuth */
static double al21;           /* Back azimuth    */
static double geod_S;         /* Distance        */
static double phi1, lam1, phi2, lam2;

static int ellipse;
static double geod_f;
static double geod_a;
static double es, onef, f, f64, f2, f4;

double adjlon (double lon) {
      if (fabs(lon) <= SPI) return( lon );
      lon += ONEPI;  /* adjust to 0..2pi rad */
      lon -= TWOPI * floor(lon / TWOPI); /* remove integral # of 'revolutions'*/
      lon -= ONEPI;  /* adjust back to -pi..pi rad */
      return( lon );
}

void geod_inv() {
      double      th1,th2,thm,dthm,dlamm,dlam,sindlamm,costhm,sinthm,cosdthm,
      sindthm,L,E,cosd,d,X,Y,T,sind,tandlammp,u,v,D,A,B;


            /*   Stuff the WGS84 projection parameters as necessary
      To avoid having to include <geodesic,h>
            */

      ellipse = 1;
      f = 1.0 / WGSinvf;       /* WGS84 ellipsoid flattening parameter */
      geod_a = WGS84_semimajor_axis_meters;

      es = 2 * f - f * f;
      onef = sqrt(1. - es);
      geod_f = 1 - onef;
      f2 = geod_f/2;
      f4 = geod_f/4;
      f64 = geod_f*geod_f/64;


      if (ellipse) {
            th1 = atan(onef * tan(phi1));
            th2 = atan(onef * tan(phi2));
      } else {
            th1 = phi1;
            th2 = phi2;
      }
      thm = .5 * (th1 + th2);
      dthm = .5 * (th2 - th1);
      dlamm = .5 * ( dlam = adjlon(lam2 - lam1) );
      if (fabs(dlam) < DTOL && fabs(dthm) < DTOL) {
            al12 =  al21 = geod_S = 0.;
            return;
      }
      sindlamm = sin(dlamm);
      costhm = cos(thm);      sinthm = sin(thm);
      cosdthm = cos(dthm);    sindthm = sin(dthm);
      L = sindthm * sindthm + (cosdthm * cosdthm - sinthm * sinthm)
                  * sindlamm * sindlamm;
      d = acos(cosd = 1 - L - L);
      if (ellipse) {
            E = cosd + cosd;
            sind = sin( d );
            Y = sinthm * cosdthm;
            Y *= (Y + Y) / (1. - L);
            T = sindthm * costhm;
            T *= (T + T) / L;
            X = Y + T;
            Y -= T;
            T = d / sind;
            D = 4. * T * T;
            A = D * E;
            B = D + D;
            geod_S = geod_a * sind * (T - f4 * (T * X - Y) +
                        f64 * (X * (A + (T - .5 * (A - E)) * X) -
                        Y * (B + E * Y) + D * X * Y));
            tandlammp = tan(.5 * (dlam - .25 * (Y + Y - E * (4. - X)) *
                        (f2 * T + f64 * (32. * T - (20. * T - A)
                        * X - (B + 4.) * Y)) * tan(dlam)));
      } else {
            geod_S = geod_a * d;
            tandlammp = tan(dlamm);
      }
      u = atan2(sindthm , (tandlammp * costhm));
      v = atan2(cosdthm , (tandlammp * sinthm));
      al12 = adjlon(TWOPI + v - u);
      al21 = adjlon(TWOPI - v - u);
}


/* --------------------------------------------------------------------------------- */
/*
// Given the lat/long of starting point and ending point,
// calculates the distance along a geodesic curve, using elliptic earth model.
*/
/* --------------------------------------------------------------------------------- */


double DistGreatCircle(double slat, double slon, double dlat, double dlon)
{

      double d5;
      phi1 = slat * DEGREE;
      lam1 = slon * DEGREE;
      phi2 = dlat * DEGREE;
      lam2 = dlon * DEGREE;

      geod_inv();
      d5 = geod_S / 1852.0;

      return d5;
}

void DistanceBearingMercator(double lat0, double lon0, double lat1, double lon1, double *brg, double *dist)
{
      double east, north, brgt, C;
      double lon0x, lon1x, dlat;
      double mlat0;

      //    Calculate bearing by conversion to SM (Mercator) coordinates, then simple trigonometry

      lon0x = lon0;
      lon1x = lon1;

      //    Make lon points the same phase
      if((lon0x * lon1x) < 0.)
      {
            if(lon0x < 0.)
                  lon0x += 360.;
            else
                  lon1x += 360.;

            //    Choose the shortest distance
            if(fabs(lon0x - lon1x) > 180.)
            {
                  if(lon0x > lon1x)
                        lon0x -= 360.;
                  else
                        lon1x -= 360.;
            }

            //    Make always positive
            lon1x += 360.;
            lon0x += 360.;
      }

      //    In the case of exactly east or west courses
      //    we must make an adjustment if we want true Mercator distances

      //    This idea comes from Thomas(Cagney)
      //    We simply require the dlat to be (slightly) non-zero, and carry on.
      //    MAS022210 for HamishB from 1e-4 && .001 to 1e-9 for better precision
      //    on small latitude diffs
      mlat0 = lat0;
      if(fabs(lat1 - lat0) < 1e-9)
            mlat0 += 1e-9;

      toSM_ECC(lat1, lon1x, mlat0, lon0x, &east, &north);

      C = atan2(east, north);
      dlat = (lat1 - mlat0) * 60.;              // in minutes

      //    Classic formula, which fails for due east/west courses....

      if(dist)
      {
            if(cos(C))
                  *dist = (dlat /cos(C));
            else
                  *dist = DistGreatCircle(lat0, lon0, lat1, lon1);

      }

      //    Calculate the bearing using the un-adjusted original latitudes and Mercator Sailing
      if(brg)
      {
            toSM_ECC(lat1, lon1x, lat0, lon0x, &east, &north);

            C = atan2(east, north);
            brgt = 180. + (C * 180. / PI);
            if (brgt < 0)
                  brgt += 360.;
            if (brgt > 360.)
                  brgt -= 360;

            *brg = brgt;
      }
}

