/* Copyright 2000, 2001 William McClain

    This file is part of Astrolabe.

    Astrolabe is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Astrolabe is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Astrolabe; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
    */

/* Geocentric solar position and radius, both low and high precision. */

/*   
   High precision position calculations.
    
    This is a very light wrapper around the VSOP87d class. The geocentric longitude of
    the Sun is simply the heliocentric longitude of the Earth + 180 degrees. The 
    geocentric latitude of the Sun is the negative of the heliocentric latitude of the
    Earth. The radius is of course the same in both coordinate systems.
    
*/

#include <cmath>
#include "astrolabe.hpp"

using std::vector;

using astrolabe::Coords;
using astrolabe::util::d_to_r;
using astrolabe::util::dms_to_d;
using astrolabe::util::modpi2;
using astrolabe::util::polynomial;
using astrolabe::calendar::jd_to_jcent;
using astrolabe::constants::pi;

double astrolabe::sun::Sun::dimension(double jd, Coords dim) const {
    /* Return one of geocentric ecliptic longitude, latitude and radius.

    Parameters:
      jd : Julian Day in dynamical time
      dim : one of "L" (longitude) or "B" (latitude) or "R" (radius).

    Returns:
      Either longitude in radians, or
      latitude in radians, or
      radius in au.

    */
    double X = vsop.dimension(jd, vEarth, dim);
    if (dim == vL)
      X = modpi2(X + pi);
    else if (dim == vB)
      X = -X;
    return X;
    }

void astrolabe::sun::Sun::dimension3(double jd, double &L, double &B, double &R) const {
    /* Return geocentric ecliptic longitude, latitude and radius.

    Parameters:
        jd : Julian Day in dynamical time

    Returns:
        longitude in radians
        latitude in radians
        radius in au

    */
    L = dimension(jd, vL);
    B = dimension(jd, vB);
    R = dimension(jd, vR);
    }

void astrolabe::sun::longitude_radius_low(double jd, double &L, double &R) {
    /* Return geometric longitude and radius vector. 
    
    Low precision. The longitude is accurate to 0.01 degree. 
    The latitude should be presumed to be 0.0. [Meeus-1998: equations 25.2 through 25.5
    
    Parameters:
        jd : Julian Day in dynamical time

    Returns:
        longitude in radians
        radius in au

    */
    //
    // Constant terms
    //
    static const double _kL0[] = {d_to_r(280.46646),  d_to_r(36000.76983),  d_to_r( 0.0003032)};
    static const VECTOR(kL0, double);

    static const double _kM[]  = {d_to_r(357.52911),  d_to_r(35999.05029),  d_to_r(-0.0001537)};
    static const VECTOR(kM, double);

    static const double _kC[]  = {d_to_r(  1.914602), d_to_r(   -0.004817), d_to_r(-0.000014)};
    static const VECTOR(kC, double);

    static const double _ke[] = {0.016708634, -0.000042037, -0.0000001267};
    static const VECTOR(ke, double);

    static const double _ck3 = d_to_r( 0.019993);
    static const double _ck4 = d_to_r(-0.000101);
    static const double _ck5 = d_to_r( 0.000289);

    const double T = jd_to_jcent(jd);
    const double L0 = polynomial(kL0, T);
    const double M = polynomial(kM, T);
    const double e = polynomial(ke, T);
    const double C = polynomial(kC, T) * sin(M) 
        + (_ck3 - _ck4 * T) * sin(2 * M) 
        + _ck5 * sin(3 * M);
    L = modpi2(L0 + C);
    const double v = M + C;
    R = 1.000001018 * (1 - e * e) / (1 + e * cos(v));
    }

double astrolabe::sun::apparent_longitude_low(double jd, double L) {
    /* Correct the geometric longitude for nutation and aberration.
    
    Low precision. [Meeus-1998: pg 164]
    
    Parameters:
        jd : Julian Day in dynamical time
        L : longitude in radians

    Returns:
        corrected longitude in radians

    */   
    //
    // Constant terms
    //
    static const double _lk0 = d_to_r(125.04);
    static const double _lk1 = d_to_r(1934.136);
    static const double _lk2 = d_to_r(0.00569);
    static const double _lk3 = d_to_r(0.00478);

    const double T = jd_to_jcent(jd);
    const double omega = _lk0 - _lk1 * T;
    return modpi2(L - _lk2 - _lk3 * sin(omega));
    }
    
double astrolabe::sun::aberration_low(double R) {
    /* Correct for aberration; low precision, but good enough for most uses. 
    
    [Meeus-1998: pg 164]
    
    Parameters:
        R : radius in au

    Returns:
        correction in radians

    */
    //
    // Constant terms
    //
    static const double _lk4 = d_to_r(dms_to_d(0, 0, 20.4898));

    return -_lk4 / R;
    }
    
