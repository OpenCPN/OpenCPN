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

/* Functions to calculate nutation and obliquity values.

The IAU "1980 Theory of Nutation" is used, but terms with coefficients
smaller than 0.0003" have been dropped.

Reference: Jean Meeus, _Astronomical Algorithms_, second edition, 1998, Willmann-Bell, Inc.

The first edition of the Meeus book had some errors in the table. These may be 
corrected in the second edition. I recall correcting my values from those
published in _Explanatory Supplement to the Astronomical Almanac_, revised
edition edited by P. Kenneth Seidelman, 1992
*/

#include <cmath>
#include "astrolabe.hpp"

using std::vector;

using astrolabe::calendar::jd_to_jcent;
using astrolabe::util::d_to_r;
using astrolabe::util::dms_to_d;
using astrolabe::util::modpi2;
using astrolabe::util::polynomial;

// [Meeus-1998: table 22.A]

struct Data {
    int D, M, M1, F, omega;
    long psiK;
    int psiT;
    long epsK;
    int epsT;
    };
    
namespace {
    const Data _tbl[] = {
        { 0,  0,  0,  0,  1, -171996, -1742, 92025,  89},
        {-2,  0,  0,  2,  2,  -13187,   -16,  5736, -31},
        { 0,  0,  0,  2,  2,   -2274,    -2,   977,  -5},
        { 0,  0,  0,  0,  2,    2062,     2,  -895,   5},
        { 0,  1,  0,  0,  0,    1426,   -34,    54,  -1},
        { 0,  0,  1,  0,  0,     712,     1,    -7,   0},
        {-2,  1,  0,  2,  2,    -517,    12,   224,  -6},
        { 0,  0,  0,  2,  1,    -386,    -4,   200,   0},
        { 0,  0,  1,  2,  2,    -301,     0,   129,  -1},
        {-2, -1,  0,  2,  2,     217,    -5,   -95,   3},
        {-2,  0,  1,  0,  0,    -158,     0,     0,   0},
        {-2,  0,  0,  2,  1,     129,     1,   -70,   0},
        { 0,  0, -1,  2,  2,     123,     0,   -53,   0},
        { 2,  0,  0,  0,  0,      63,     0,     0,   0},
        { 0,  0,  1,  0,  1,      63,     1,   -33,   0},
        { 2,  0, -1,  2,  2,     -59,     0,    26,   0},
        { 0,  0, -1,  0,  1,     -58,    -1,    32,   0},
        { 0,  0,  1,  2,  1,     -51,     0,    27,   0},
        {-2,  0,  2,  0,  0,      48,     0,     0,   0},
        { 0,  0, -2,  2,  1,      46,     0,   -24,   0},
        { 2,  0,  0,  2,  2,     -38,     0,    16,   0},
        { 0,  0,  2,  2,  2,     -31,     0,    13,   0},
        { 0,  0,  2,  0,  0,      29,     0,     0,   0},
        {-2,  0,  1,  2,  2,      29,     0,   -12,   0},
        { 0,  0,  0,  2,  0,      26,     0,     0,   0},
        {-2,  0,  0,  2,  0,     -22,     0,     0,   0},
        { 0,  0, -1,  2,  1,      21,     0,   -10,   0},
        { 0,  2,  0,  0,  0,      17,    -1,     0,   0},
        { 2,  0, -1,  0,  1,      16,     0,    -8,   0},
        {-2,  2,  0,  2,  2,     -16,     1,     7,   0},
        { 0,  1,  0,  0,  1,     -15,     0,     9,   0},
        {-2,  0,  1,  0,  1,     -13,     0,     7,   0},
        { 0, -1,  0,  0,  1,     -12,     0,     6,   0},
        { 0,  0,  2, -2,  0,      11,     0,     0,   0},
        { 2,  0, -1,  2,  1,     -10,     0,     5,   0},
        { 2,  0,  1,  2,  2,      -8,     0,     3,   0},
        { 0,  1,  0,  2,  2,       7,     0,    -3,   0},
        {-2,  1,  1,  0,  0,      -7,     0,     0,   0},
        { 0, -1,  0,  2,  2,      -7,     0,     3,   0},
        { 2,  0,  0,  2,  1,      -7,     0,     3,   0},
        { 2,  0,  1,  0,  0,       6,     0,     0,   0},
        {-2,  0,  2,  2,  2,       6,     0,    -3,   0},
        {-2,  0,  1,  2,  1,       6,     0,    -3,   0},
        { 2,  0, -2,  0,  1,      -6,     0,     3,   0},
        { 2,  0,  0,  0,  1,      -6,     0,     3,   0},
        { 0, -1,  1,  0,  0,       5,     0,     0,   0},
        {-2, -1,  0,  2,  1,      -5,     0,     3,   0},
        {-2,  0,  0,  0,  1,      -5,     0,     3,   0},
        { 0,  0,  2,  2,  1,      -5,     0,     3,   0},
        {-2,  0,  2,  0,  1,       4,     0,     0,   0},
        {-2,  1,  0,  2,  1,       4,     0,     0,   0},
        { 0,  0,  1, -2,  0,       4,     0,     0,   0},
        {-1,  0,  1,  0,  0,      -4,     0,     0,   0},
        {-2,  1,  0,  0,  0,      -4,     0,     0,   0},
        { 1,  0,  0,  0,  0,      -4,     0,     0,   0},
        { 0,  0,  1,  2,  0,       3,     0,     0,   0},
        { 0,  0, -2,  2,  2,      -3,     0,     0,   0},
        {-1, -1,  1,  0,  0,      -3,     0,     0,   0},
        { 0,  1,  1,  0,  0,      -3,     0,     0,   0},
        { 0, -1,  1,  2,  2,      -3,     0,     0,   0},
        { 2, -1, -1,  2,  2,      -3,     0,     0,   0},
        { 0,  0,  3,  2,  2,      -3,     0,     0,   0},
        { 2, -1,  0,  2,  2,      -3,     0,     0,   0}
        };

    const VECTOR(tbl, Data);
    };

namespace {
    void _constants(double T, double &D, double &M, double &M1, double &F, double &omega) {
        // Return some values needed for both nut_in_lon() and nut_in_obl()
        //
        // Constant terms.
        // 
        static const double _kD[]  = {d_to_r(297.85036), d_to_r(445267.111480), d_to_r(-0.0019142), d_to_r( 1.0/189474)};
        static const VECTOR(kD, double);

        static const double _kM[]  = {d_to_r(357.52772), d_to_r( 35999.050340), d_to_r(-0.0001603), d_to_r(-1.0/300000)};
        static const VECTOR(kM, double);

        static const double _kM1[] = {d_to_r(134.96298), d_to_r(477198.867398), d_to_r( 0.0086972), d_to_r( 1.0/ 56250)};
        static const VECTOR(kM1, double);

        static const double _kF[]  = {d_to_r( 93.27191), d_to_r(483202.017538), d_to_r(-0.0036825), d_to_r( 1.0/327270)};
        static const VECTOR(kF, double);

        static const double _ko[]  = {d_to_r(125.04452), d_to_r( -1934.136261), d_to_r( 0.0020708), d_to_r( 1.0/450000)};
        static const VECTOR(ko, double);

        D     = modpi2(polynomial(kD,  T));
        M     = modpi2(polynomial(kM,  T));
        M1    = modpi2(polynomial(kM1, T));
        F     = modpi2(polynomial(kF,  T));
        omega = modpi2(polynomial(ko,  T));
        }
    };
        
double astrolabe::nutation::nut_in_lon(double jd) {
    /* Return the nutation in longitude. 
    
    High precision. [Meeus-1998: pg 144]
    
    Parameters:
        jd : Julian Day in dynamical time
        
    Returns:
        nutation in longitude, in radians
    
    */
    // 
    // Future optimization: factor the /1e5 and /1e6 adjustments into the table.
    //
    const double T = jd_to_jcent(jd);
    double D, M, M1, F, omega;
    _constants(T, D, M, M1, F, omega);
    double deltaPsi = 0.0;
    for (std::vector<Data>::const_iterator p = tbl.begin(); p != tbl.end(); p++) {
        const double arg = D*p->D + M*p->M + M1*p->M1 + F*p->F + omega*p->omega;
        deltaPsi += (p->psiK/10000.0 + p->psiT/100000.0 * T) * sin(arg);
        }
    deltaPsi /= 3600;
    deltaPsi = d_to_r(deltaPsi);
    return deltaPsi;
    }
    
double astrolabe::nutation::nut_in_obl(double jd) {
    /* Return the nutation in obliquity. 
    
    High precision. [Meeus-1998: pg 144]
    
    Parameters:
        jd : Julian Day in dynamical time
        
    Returns:
        nutation in obliquity, in radians

    */
    // 
    // Future optimization: factor the /1e5 and /1e6 adjustments into the table.
    //
    const double T = jd_to_jcent(jd);
    double D, M, M1, F, omega;
    _constants(T, D, M, M1, F, omega);
    double deltaEps = 0.0;
    for (std::vector<Data>::const_iterator p = tbl.begin(); p != tbl.end(); p++) {
        const double arg = D*p->D + M*p->M + M1*p->M1 + F*p->F + omega*p->omega;
        deltaEps += (p->epsK/10000.0 + p->epsT/100000.0 * T) * cos(arg);
        }
    deltaEps /= 3600;
    deltaEps = d_to_r(deltaEps);
    return deltaEps;
    }

double astrolabe::nutation::obliquity(double jd) {
    /* Return the mean obliquity of the ecliptic. 
    
    Low precision, but good enough for most uses. [Meeus-1998: equation 22.2].
    
    Accuracy is 1" over 2000 years and 10" over 4000 years.

    Parameters:
        jd : Julian Day in dynamical time
        
    Returns:
        obliquity, in radians

    */
    //
    // Constant terms
    // 
    static const double _el0[] = {
        d_to_r(dms_to_d(23, 26,  21.448)), 
        d_to_r(dms_to_d( 0,  0, -46.8150)),
        d_to_r(dms_to_d( 0,  0,  -0.00059)),
        d_to_r(dms_to_d( 0,  0,   0.001813))
        };

    static const VECTOR(el0, double);

    const double T = jd_to_jcent(jd);
    return polynomial(el0, T);
    }

double astrolabe::nutation::obliquity_hi(double jd) {
    /* Return the mean obliquity of the ecliptic. 
    
    High precision [Meeus-1998: equation 22.3].
    
    Accuracy is 0.01" between 1000 and 3000, and "a few arc-seconds
    after 10,000 years".
    
    Parameters:
        jd : Julian Day in dynamical time
        
    Returns:
        obliquity, in radians

    */
    //
    // Constant terms
    // 
    static const double _el1[] = {
        d_to_r(dms_to_d(23, 26,    21.448)),
        d_to_r(dms_to_d( 0,  0, -4680.93)),
        d_to_r(dms_to_d( 0,  0,    -1.55)),
        d_to_r(dms_to_d( 0,  0,  1999.25)),
        d_to_r(dms_to_d( 0,  0,   -51.38)),
        d_to_r(dms_to_d( 0,  0,  -249.67)),
        d_to_r(dms_to_d( 0,  0,   -39.05)),
        d_to_r(dms_to_d( 0,  0,     7.12)),
        d_to_r(dms_to_d( 0,  0,    27.87)),
        d_to_r(dms_to_d( 0,  0,     5.79)),
        d_to_r(dms_to_d( 0,  0,     2.45))
        };

    static const VECTOR(el1, double);

    const double U = jd_to_jcent(jd) / 100;
    return polynomial(el1, U);
    }
    
    
