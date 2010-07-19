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

/* Compute Rise, Set, and Transit times.

Each of the routines requires three equatorial coordinates for the
object: yesterday, today and tomorrow, all at 0hr UT.

This approach is inadequate for the Moon, which moves too fast to
be accurately interpolated from three daily positions.

Bug: each of the routines drops some events which occur near 0hr UT.

*/

#include <cmath>
#include "astrolabe.hpp"

using std::vector;

using astrolabe::Error;
using astrolabe::calendar::sidereal_time_greenwich;
using astrolabe::constants::earth_equ_radius;
using astrolabe::constants::standard_rst_altitude;
using astrolabe::constants::pi2;
using astrolabe::constants::seconds_per_day;
using astrolabe::dynamical::deltaT_seconds;
using astrolabe::util::d_to_r;
using astrolabe::util::diff_angle;
using astrolabe::util::equ_to_horiz;
using astrolabe::util::interpolate3;
using astrolabe::util::interpolate_angle3;
using astrolabe::util::modpi2;

namespace {
    const double _k1 = d_to_r(360.985647);
    };
    
double astrolabe::riseset::rise(double jd, const vector<double> &raList, const vector<double> &decList, double h0, double delta) {
    /* Return the Julian Day of the rise time of an object.
    
    Parameters:
        jd      : Julian Day number of the day in question, at 0 hr UT
        raList  : a sequence of three right accension values, in radians,
            for (jd-1, jd, jd+1)
        decList : a sequence of three right declination values, in radians,
            for (jd-1, jd, jd+1)
        h0      : the standard altitude in radians
        delta   : desired accuracy in days. Times less than one minute are
            infeasible for rise times because of atmospheric refraction.
            
    Returns:
        Julian Day of the rise time
    
    */
    const double THETA0 = sidereal_time_greenwich(jd);
    const double deltaT_days = deltaT_seconds(jd) / seconds_per_day;

    const double cosH0 = (sin(h0) - sin(astrolabe::globals::latitude) * sin(decList[1])) / (cos(astrolabe::globals::latitude) * cos(decList[1]));
    //
    // future: return some indicator when the object is circumpolar or always
    // below the horizon.
    //
    if (cosH0 < -1.0) // circumpolar
        return -1.0;
    if (cosH0 > 1.0)  // never rises
        return -1.0;

    const double H0 = acos(cosH0);    
    double m0 = (raList[1] + astrolabe::globals::longitude - THETA0) / pi2;
    double m = m0 - H0 / pi2;  // this is the only difference between rise() and set()
    if (m < 0)
        m += 1;
    else if (m > 1)
        m -= 1;
    if (m < 0 || m > 1)
        throw Error("astrolabe::riseset::rise: m is out of range");
    for (int i = 0; i < 20; i++) {
        m0 = m;
        const double theta0 = modpi2(THETA0 + _k1 * m);
        const double n = m + deltaT_days;
        if (n < -1 || n > 1)
            return -1.0; // Bug: this is where we drop some events
        const double ra = interpolate_angle3(n, raList);
        const double dec = interpolate3(n, decList);
        double H = theta0 - astrolabe::globals::longitude - ra;
        H = diff_angle(0.0, H);
        double A, h;
        equ_to_horiz(H, dec, A, h);
        const double dm = (h - h0) / (pi2 * cos(dec) * cos(astrolabe::globals::latitude) * sin(H));
        m += dm;
        if (fabs(m - m0) < delta) 
            return jd + m;
        }

    throw Error("astrolabe::riseset::rise: bailout");
    }
    
double astrolabe::riseset::set(double jd, const vector<double> &raList, const vector<double> &decList, double h0, double delta) {
    /* Return the Julian Day of the set time of an object.
    
    Parameters:
        jd      : Julian Day number of the day in question, at 0 hr UT
        raList  : a sequence of three right accension values, in radians,
            for (jd-1, jd, jd+1)
        decList : a sequence of three right declination values, in radians,
            for (jd-1, jd, jd+1)
        h0      : the standard altitude in radians
        delta   : desired accuracy in days. Times less than one minute are
            infeasible for set times because of atmospheric refraction.
            
    Returns:
        Julian Day of the set time
    
    */
    const double THETA0 = sidereal_time_greenwich(jd);
    const double deltaT_days = deltaT_seconds(jd) / seconds_per_day;

    const double cosH0 = (sin(h0) - sin(astrolabe::globals::latitude) * sin(decList[1])) / (cos(astrolabe::globals::latitude) * cos(decList[1]));
    //
    // future: return some indicator when the object is circumpolar or always
    // below the horizon.
    //
    if (cosH0 < -1.0) // circumpolar
        return -1.0;
    if (cosH0 > 1.0)  // never rises
        return -1.0;

    const double H0 = acos(cosH0);    
    double m0 = (raList[1] + astrolabe::globals::longitude - THETA0) / pi2;
    double m = m0 + H0 / pi2;  // this is the only difference between rise() and set()
    if (m < 0)
        m += 1;
    else if (m > 1)
        m -= 1;
    if (m < 0 || m > 1)
        throw Error("astrolabe::riseset::set: m is out of range");
    for (int i = 0; i < 20; i++) {
        m0 = m;
        const double theta0 = modpi2(THETA0 + _k1 * m);
        const double n = m + deltaT_days;
        if (n < -1 || n > 1)
            return -1.0; // Bug: this is where we drop some events
        const double ra = interpolate_angle3(n, raList);
        const double dec = interpolate3(n, decList);
        double H = theta0 - astrolabe::globals::longitude - ra;
        H = diff_angle(0.0, H);
        double A, h;
        equ_to_horiz(H, dec, A, h);
        const double dm = (h - h0) / (pi2 * cos(dec) * cos(astrolabe::globals::latitude) * sin(H));
        m += dm;
        if (fabs(m - m0) < delta) 
            return jd + m;
        }

    throw Error("astrolabe::riseset::set: bailout");
    }

double astrolabe::riseset::transit(double jd, const vector<double> &raList, double delta) {
    /* Return the Julian Day of the transit time of an object.
    
    Parameters:
        jd      : Julian Day number of the day in question, at 0 hr UT
        raList  : a sequence of three right accension values, in radians,
            for (jd-1, jd, jd+1)
        delta   : desired accuracy in days. 
            
    Returns:
        Julian Day of the transit time
    
    */
    //
    // future: report both upper and lower culmination, and transits of objects below 
    // the horizon
    // 
    const double THETA0 = sidereal_time_greenwich(jd);
    const double deltaT_days = deltaT_seconds(jd) / seconds_per_day;
    
    double m = (raList[1] + astrolabe::globals::longitude - THETA0) / pi2;
    if (m < 0)
        m += 1;
    else if (m > 1)
        m -= 1;
    if (m < 0 || m > 1)
        throw Error("astrolabe::riseset::transit: m is out of range");
    for (int i = 0; i < 20; i++) {
        const double m0 = m;
        const double theta0 = modpi2(THETA0 + _k1 * m);
        const double n = m + deltaT_days;
        if (n < -1 || n > 1)
            return -1.0; // Bug: this is where we drop some events
        const double ra = interpolate_angle3(n, raList);
        double H = theta0 - astrolabe::globals::longitude - ra;
        H = diff_angle(0.0, H);
        const double dm = -H/pi2;
        m += dm;
        if (fabs(m - m0) < delta) 
            return jd + m;
        }

    throw Error("astrolabe::riseset::transit: bailout");
    }
    
double astrolabe::riseset::moon_rst_altitude(double r) {
    /* Returnn the standard altitude of the Moon.
    
    Parameters:
        r : Distance between the centers of the Earth and Moon, in km.
            
    Returns:
        Standard altitude in radians.
    
    */
    // horizontal parallax
    const double parallax = asin(earth_equ_radius / r);
    
    return 0.7275 * parallax + standard_rst_altitude;
    }

