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

/* Lunar position model ELP2000-82 of Chapront.

The result values are for the equinox of date and have been adjusted
for light-time.
        
This is the simplified version of Jean Meeus, _Astronomical Algorithms_, 
second edition, 1998, Willmann-Bell, Inc.

*/

#include <cmath>
#include "astrolabe.hpp"

using std::vector;

using astrolabe::calendar::jd_to_jcent;
using astrolabe::Coords;
using astrolabe::util::d_to_r;
using astrolabe::util::modpi2;
using astrolabe::util::polynomial;
using astrolabe::util::int_to_string;

// [Meeus-1998: table 47.A]

struct TableA {
    int D, M, M1, F;
    long l, r;
    };
    
namespace { 
    const TableA _tblLR[] = {
        {0,  0,  1,  0, 6288774, -20905355},
        {2,  0, -1,  0, 1274027,  -3699111},
        {2,  0,  0,  0,  658314,  -2955968},
        {0,  0,  2,  0,  213618,   -569925},
        {0,  1,  0,  0, -185116,     48888},
        {0,  0,  0,  2, -114332,     -3149},
        {2,  0, -2,  0,   58793,    246158},
        {2, -1, -1,  0,   57066,   -152138},
        {2,  0,  1,  0,   53322,   -170733},
        {2, -1,  0,  0,   45758,   -204586},
        {0,  1, -1,  0,  -40923,   -129620},
        {1,  0,  0,  0,  -34720,    108743},
        {0,  1,  1,  0,  -30383,    104755},
        {2,  0,  0, -2,   15327,     10321},
        {0,  0,  1,  2,  -12528,         0},
        {0,  0,  1, -2,   10980,     79661},
        {4,  0, -1,  0,   10675,    -34782},
        {0,  0,  3,  0,   10034,    -23210},
        {4,  0, -2,  0,    8548,    -21636},
        {2,  1, -1,  0,   -7888,     24208},
        {2,  1,  0,  0,   -6766,     30824},
        {1,  0, -1,  0,   -5163,     -8379},
        {1,  1,  0,  0,    4987,    -16675},
        {2, -1,  1,  0,    4036,    -12831},
        {2,  0,  2,  0,    3994,    -10445},
        {4,  0,  0,  0,    3861,    -11650},
        {2,  0, -3,  0,    3665,     14403},
        {0,  1, -2,  0,   -2689,     -7003},
        {2,  0, -1,  2,   -2602,         0},
        {2, -1, -2,  0,    2390,     10056},
        {1,  0,  1,  0,   -2348,      6322},
        {2, -2,  0,  0,    2236,     -9884},
        {0,  1,  2,  0,   -2120,      5751},
        {0,  2,  0,  0,   -2069,         0},
        {2, -2, -1,  0,    2048,     -4950},
        {2,  0,  1, -2,   -1773,      4130},
        {2,  0,  0,  2,   -1595,         0},
        {4, -1, -1,  0,    1215,     -3958},
        {0,  0,  2,  2,   -1110,         0},
        {3,  0, -1,  0,    -892,      3258},
        {2,  1,  1,  0,    -810,      2616},
        {4, -1, -2,  0,     759,     -1897},
        {0,  2, -1,  0,    -713,     -2117},
        {2,  2, -1,  0,    -700,      2354},
        {2,  1, -2,  0,     691,         0},
        {2, -1,  0, -2,     596,         0},
        {4,  0,  1,  0,     549,     -1423},
        {0,  0,  4,  0,     537,     -1117},
        {4, -1,  0,  0,     520,     -1571},
        {1,  0, -2,  0,    -487,     -1739},
        {2,  1,  0, -2,    -399,         0},
        {0,  0,  2, -2,    -381,     -4421},
        {1,  1,  1,  0,     351,         0},
        {3,  0, -2,  0,    -340,         0},
        {4,  0, -3,  0,     330,         0},
        {2, -1,  2,  0,     327,         0},
        {0,  2,  1,  0,    -323,      1165},
        {1,  1, -1,  0,     299,         0},
        {2,  0,  3,  0,     294,         0},
        {2,  0, -1, -2,       0,      8752}
        };

    const VECTOR(tblLR, TableA);
    };
    
// [Meeus-1998: table 47.B]

struct TableB {
    int D, M, M1, F;
    long b;
    };
    
namespace {
    const TableB _tblB[] = {
        {0,  0,  0,  1, 5128122},
        {0,  0,  1,  1,  280602},
        {0,  0,  1, -1,  277693},
        {2,  0,  0, -1,  173237},
        {2,  0, -1,  1,   55413},
        {2,  0, -1, -1,   46271},
        {2,  0,  0,  1,   32573},
        {0,  0,  2,  1,   17198},
        {2,  0,  1, -1,    9266},
        {0,  0,  2, -1,    8822},
        {2, -1,  0, -1,    8216},
        {2,  0, -2, -1,    4324},
        {2,  0,  1,  1,    4200},
        {2,  1,  0, -1,   -3359},
        {2, -1, -1,  1,    2463},
        {2, -1,  0,  1,    2211},
        {2, -1, -1, -1,    2065},
        {0,  1, -1, -1,   -1870},
        {4,  0, -1, -1,    1828},
        {0,  1,  0,  1,   -1794},
        {0,  0,  0,  3,   -1749},
        {0,  1, -1,  1,   -1565},
        {1,  0,  0,  1,   -1491},
        {0,  1,  1,  1,   -1475},
        {0,  1,  1, -1,   -1410},
        {0,  1,  0, -1,   -1344},
        {1,  0,  0, -1,   -1335},
        {0,  0,  3,  1,    1107},
        {4,  0,  0, -1,    1021},
        {4,  0, -1,  1,     833},
        {0,  0,  1, -3,     777},
        {4,  0, -2,  1,     671},
        {2,  0,  0, -3,     607},
        {2,  0,  2, -1,     596},
        {2, -1,  1, -1,     491},
        {2,  0, -2,  1,    -451},
        {0,  0,  3, -1,     439},
        {2,  0,  2,  1,     422},
        {2,  0, -3, -1,     421},
        {2,  1, -1,  1,    -366},
        {2,  1,  0,  1,    -351},
        {4,  0,  0,  1,     331},
        {2, -1,  1,  1,     315},
        {2, -2,  0, -1,     302},
        {0,  0,  1,  3,    -283},
        {2,  1,  1, -1,    -229},
        {1,  1,  0, -1,     223},
        {1,  1,  0,  1,     223},
        {0,  1, -2, -1,    -220},
        {2,  1, -1, -1,    -220},
        {1,  0,  1,  1,    -185},
        {2, -1, -2, -1,     181},
        {0,  1,  2,  1,    -177},
        {4,  0, -2, -1,     176},
        {4, -1, -1, -1,     166},
        {1,  0,  1, -1,    -164},
        {4,  0,  1, -1,     132},
        {1,  0, -1, -1,    -119},
        {4, -1,  0, -1,     115},
        {2, -2,  0,  1,     107}
        };

    const VECTOR(tblB, TableB);
    };
    
static void _constants(double T, double &L1, double &D, double &M, double &M1, double &F, double &A1, double &A2, double &A3, double &E, double &E2) {
    // Calculate values required by several other functions"""
    //
    // Constant terms.
    //  
    static const double _kL1[] = {d_to_r(218.3164477), d_to_r(481267.88123421), d_to_r(-0.0015786), d_to_r( 1.0/  538841), d_to_r(-1.0/ 65194000)};
    static const VECTOR(kL1, double);

    static const double _kD[]  = {d_to_r(297.8501921), d_to_r(445267.1114034),  d_to_r(-0.0018819), d_to_r( 1.0/  545868), d_to_r(-1.0/113065000)};
    static const VECTOR(kD, double);

    static const double _kM[]  = {d_to_r(357.5291092), d_to_r( 35999.0502909),  d_to_r(-0.0001536), d_to_r( 1.0/24490000)};
    static const VECTOR(kM, double);

    static const double _kM1[] = {d_to_r(134.9633964), d_to_r(477198.8675055),  d_to_r( 0.0087414), d_to_r( 1.0/   69699), d_to_r(-1.0/ 14712000)};
    static const VECTOR(kM1, double);

    static const double _kF[]  = {d_to_r( 93.2720950), d_to_r(483202.0175233),  d_to_r(-0.0036539), d_to_r(-1.0/ 3526000), d_to_r( 1.0/863310000)};
    static const VECTOR(kF, double);

    static const double _kA1[] = {d_to_r(119.75), d_to_r(   131.849)};
    static const VECTOR(kA1, double);

    static const double _kA2[] = {d_to_r( 53.09), d_to_r(479264.290)};
    static const VECTOR(kA2, double);

    static const double _kA3[] = {d_to_r(313.45), d_to_r(481266.484)};
    static const VECTOR(kA3, double);

    static const double _kE[] = {1.0, -0.002516, -0.0000074};
    static const VECTOR(kE, double);
    L1 = modpi2(polynomial(kL1, T));
    D  = modpi2(polynomial(kD,  T));
    M  = modpi2(polynomial(kM,  T));
    M1 = modpi2(polynomial(kM1, T));
    F  = modpi2(polynomial(kF,  T));

    A1 = modpi2(polynomial(kA1, T));
    A2 = modpi2(polynomial(kA2, T));
    A3 = modpi2(polynomial(kA3, T));
    
    E = polynomial(kE, T);
    E2 = E*E;
    }

void astrolabe::elp2000::ELP2000::dimension3(double jd, double &longitude, double &latitude, double &radius) const {
    /* Return geocentric ecliptic longitude, latitude and radius.

    When we need all three dimensions it is more efficient to combine the 
    calculations in one routine.

    Parameters:
        jd : Julian Day in dynamical time

    Returns:
        longitude in radians
        latitude in radians
        radius in km, Earth's center to Moon's center

    */
    const double T = jd_to_jcent(jd);
    double L1, D, M, M1, F, A1, A2, A3, E, E2;
    _constants(T, L1, D, M, M1, F, A1, A2, A3, E, E2);

    //
    // longitude and radius
    //
    double lsum = 0.0;
    double rsum = 0.0;
    for (std::vector<TableA>::const_iterator p = tblLR.begin(); p != tblLR.end(); p++) {
        double tl = p->l;
        double tr = p->r;
        const double arg = p->D * D + p->M * M + p->M1 * M1 + p->F * F;
        if (fabs(p->M) == 1) {
            tl *= E;
            tr *= E;
            }
        else if (fabs(p->M == 2)) {
            tl *= E2;
            tr *= E2;
            }
        lsum += tl * sin(arg);
        rsum += tr * cos(arg);
        }

    //
    // latitude
    //
    double bsum = 0.0;
    for (std::vector<TableB>::const_iterator q = tblB.begin(); q != tblB.end(); q++) {
        double tb = q->b;
        const double arg = q->D * D + q->M * M + q->M1 * M1 + q->F * F;
        if (fabs(q->M) == 1) 
            tb *= E;
        else if (fabs(q->M) == 2) 
            tb *= E2;
        bsum += tb * sin(arg);
        }

    lsum += 3958 * sin(A1) +       
            1962 * sin(L1 - F) +   
             318 * sin(A2);

    bsum += -2235 * sin(L1) +      
              382 * sin(A3) +      
              175 * sin(A1 - F) +  
              175 * sin(A1 + F) +  
              127 * sin(L1 - M1) - 
              115 * sin(L1 + M1);

    longitude = L1 + d_to_r(lsum / 1000000);
    latitude = d_to_r(bsum / 1000000);          
    radius = 385000.56 + rsum / 1000;
    }

double astrolabe::elp2000::ELP2000::dimension(double jd, Coords dim) const {
    /* Return one of geocentric ecliptic longitude, latitude and radius.

    Parameters:
        jd : Julian Day in dynamical time
        dim : "L" (longitude") or "B" (latitude) or "R" (radius)

    Returns:
        longitude in radians or
        latitude in radians or
        radius in km, Earth's center to Moon's center

    */
    if (dim == vL)
        return longitude(jd);
    if (dim == vB)
        return latitude(jd);
    if (dim == vR)
        return radius(jd);
    throw Error("astrolabe::elp2000::ELP2000::dimension: unknown dimension = " + int_to_string(dim));
    }

double astrolabe::elp2000::ELP2000::longitude(double jd) const {
    /* Return the geocentric ecliptic longitude in radians.

    A subset of the logic in dimension3()

    */
    const double T = jd_to_jcent(jd);
    double L1, D, M, M1, F, A1, A2, A3, E, E2;
    _constants(T, L1, D, M, M1, F, A1, A2, A3, E, E2);

    double lsum = 0.0;
    for (std::vector<TableA>::const_iterator p = tblLR.begin(); p != tblLR.end(); p++) {
        double tl = p->l;
        const double arg = p->D * D + p->M * M + p->M1 * M1 + p->F * F;
        if (fabs(p->M) == 1) 
            tl *= E;
        else if (fabs(p->M == 2)) 
            tl *= E2;
        lsum += tl * sin(arg);
        }

    lsum += 3958 * sin(A1) +      
            1962 * sin(L1 - F) +  
             318 * sin(A2);

    const double L = L1 + d_to_r(lsum / 1000000);   
    return L;
    }

double astrolabe::elp2000::ELP2000::latitude(double jd) const {
    /* Return the geocentric ecliptic latitude in radians.

    A subset of the logic in dimension3()

    */
    const double T = jd_to_jcent(jd);
    double L1, D, M, M1, F, A1, A2, A3, E, E2;
    _constants(T, L1, D, M, M1, F, A1, A2, A3, E, E2);

    double bsum = 0.0;
    for (std::vector<TableB>::const_iterator p = tblB.begin(); p != tblB.end(); p++) {
        double tb = p->b;
        const double arg = p->D * D + p->M * M + p->M1 * M1 + p->F * F;
        if (fabs(p->M) == 1) 
            tb *= E;
        else if (fabs(p->M) == 2) 
            tb *= E2;
        bsum += tb * sin(arg);
        }

    bsum += -2235 * sin(L1) +      
              382 * sin(A3) +      
              175 * sin(A1 - F) +  
              175 * sin(A1 + F) +  
              127 * sin(L1 - M1) - 
              115 * sin(L1 + M1);

    const double B = d_to_r(bsum / 1000000);        
    return B;
    }

double astrolabe::elp2000::ELP2000::radius(double jd) const {
    /* Return the geocentric radius in km.

    A subset of the logic in dimension3()

    */
    const double T = jd_to_jcent(jd);
    double L1, D, M, M1, F, A1, A2, A3, E, E2;
    _constants(T, L1, D, M, M1, F, A1, A2, A3, E, E2);

    double rsum = 0.0;
    for (std::vector<TableA>::const_iterator p = tblLR.begin(); p != tblLR.end(); p++) {
        double tr = p->r;
        const double arg = p->D * D + p->M * M + p->M1 * M1 + p->F * F;
        if (fabs(p->M) == 1) 
            tr *= E;
        else if (fabs(p->M == 2)) 
            tr *= E2;
        rsum += tr * cos(arg);
        }

    const double R = 385000.56 + rsum / 1000;
    return R;
    }
