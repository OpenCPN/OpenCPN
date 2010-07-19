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

/* A collection of date and time functions.

The functions which use Julian Day Numbers are valid only for positive values, 
i.e., for dates after -4712 (4713BC).

Unless otherwise specified, Julian Day Numbers may be fractional values.

Numeric years use the astronomical convention of a year 0: 0 = 1BC, -1 = 2BC, etc.

Numeric months are 1-based: Jan = 1...Dec = 12.

Numeric days are the same as the calendar value.

Reference: Jean Meeus, _Astronomical Algorithms_, second edition, 1998, Willmann-Bell, Inc.

*/

#include "cmath"
#include "cstdio"
#include "astrolabe.hpp"

using std::string;

using astrolabe::dicts::monthToString;
using astrolabe::util::d_to_r;
using astrolabe::util::fday_to_hms;
using astrolabe::util::modpi2;

double astrolabe::calendar::cal_to_jd(int yr, int mo, double day, bool gregorian) {
    /* Convert a date in the Julian or Gregorian calendars to the Julian Day Number (Meeus 7.1). 
    
    Parameters:
        yr        : year
        mo        : month (default: 1)
        day       : day, may be fractional day (default: 1)
        gregorian : If true, use Gregorian calendar, else use Julian calendar (default: true)
        
    Return:
        jd        : julian day number
    
    */
    if (mo <= 2) {
        yr--;
        mo += 12;
        }
    int B;
    if (gregorian) {
        const int A = yr / 100;
        B = 2 - A + (A / 4);
        }
    else
        B = 0;
    return int(365.25 * (yr + 4716)) + int(30.6001 * (mo + 1)) + day + B - 1524.5;
    }

int astrolabe::calendar::cal_to_day_of_year(int yr, int mo, int dy, bool gregorian) {
    /* Convert a date in the Julian or Gregorian calendars to day of the year (Meeus 7.1).
    
    Parameters:
        yr        : year
        mo        : month 
        day       : day 
        gregorian : If true, use Gregorian calendar, else use Julian calendar (default: true)
        
    Return:
        day number : 1 = Jan 1...365 (or 366 for leap years) = Dec 31.

    */
    int K;
    if (is_leap_year(yr, gregorian)) 
        K = 1;
    else 
        K = 2;
    return int(275 * mo / 9.0) - (K * int((mo + 9) / 12.0)) + dy - 30;
    }

void astrolabe::calendar::day_of_year_to_cal(int yr, int N, bool gregorian, int &mo, int &dy) {
    /*Convert a day of year number to a month and day in the Julian or Gregorian calendars.
    
    Parameters:
        yr        : year
        N         : day of year, 1..365 (or 366 for leap years) 
        gregorian : If true, use Gregorian calendar, else use Julian calendar (default: true)
        
    Return:
        month
        day
    
    */
    int K;
    if (is_leap_year(yr, gregorian)) 
        K = 1;
    else 
        K = 2;
    if (N < 32)
        mo = 1;
    else
        mo = int(9 * (K+N) / 275.0 + 0.98);
    dy = int(N - int(275 * mo / 9.0) + K * int((mo + 9) / 12.0) + 30);
    }

void astrolabe::calendar::easter(int yr, bool gregorian, int &mo, int &dy) {
    /* Return the date of Western ecclesiastical Easter for a year in the Julian or Gregorian calendars.
    
    Parameters:
        yr        : year
        gregorian : If true, use Gregorian calendar, else use Julian calendar (default: true)
        
    Return:
        month
        day    

    */
    int tmp;
    if (gregorian) { 
        const int a = yr % 19;
        const int b = yr / 100;
        const int c = yr % 100;
        const int d = b / 4;
        const int e = b % 4;
        const int f = (b + 8) / 25;
        const int g = (b - f + 1) / 3;
        const int h = (19 * a + b - d - g + 15) % 30;
        const int i = c / 4;
        const int k = c % 4;
        const int l = (32 + 2 * e + 2 * i - h - k) % 7;
        const int m = (a + 11 * h + 22 * l) / 451;
        tmp = h + l - 7 * m + 114;
        }
    else {
        const int a = yr % 4;
        const int b = yr % 7;
        const int c = yr % 19;
        const int d = (19 * c + 15) % 30;
        const int e = (2 * a + 4 * b - d + 34) % 7;
        tmp = d + e + 114;
        }
    mo = tmp / 31;
    dy = (tmp % 31) + 1;
    }

bool astrolabe::calendar::is_dst(double jd) {
    /* Is this instant within the Daylight Savings Time period as used in the US?
    
    If astrolabe.globals.daylight_timezone_name is None, the function always returns
    false.
    
    Parameters:
        jd : Julian Day number representing an instant in Universal Time
        
    Return:
        true if Daylight Savings Time is in effect, false otherwise.
           
    */
    if (astrolabe::globals::daylight_timezone_name.empty()) 
        return false;

    //
    // What year is this?
    // 
    int yr, mon;
    double day;
    jd_to_cal(jd, true, yr, mon, day);
    
    //
    // First day in April
    // 
    double start = cal_to_jd(yr, 4, 1);
    
    //
    // Advance to the first Sunday
    //
    int dow = jd_to_day_of_week(start);
    if (dow > 0)
        start += (7 - dow);

    //
    // Advance to 2AM
    //         
    start += 2.0 / 24;
    
    //
    // Convert to Universal Time
    //
    start += astrolabe::globals::standard_timezone_offset;

    if (jd < start)
        return false;
        
    //
    // Last day in October
    //
    double stop = cal_to_jd(yr, 10, 31);
    
    //
    // Backup to the last Sunday
    // 
    dow = jd_to_day_of_week(stop);
    stop -= dow;

    //
    // Advance to 2AM
    //         
    stop += 2.0 / 24;
    
    //
    // Convert to Universal Time
    //
    stop += astrolabe::globals::daylight_timezone_offset;
    
    if (jd < stop)
        return true;
        
    return false;
    }

bool astrolabe::calendar::is_leap_year(int yr, bool gregorian) {
    /* Return true if this is a leap year in the Julian or Gregorian calendars
    
    Parameters:
        yr        : year
        gregorian : If true, use Gregorian calendar, else use Julian calendar (default: true)
        
    Return:
        True is this is a leap year, else false.
        
    */
    if (gregorian)
        return (yr % 4 == 0) && ((yr % 100 != 0) || (yr % 400 == 0));
    else    
        return yr % 4 == 0;
    }

void astrolabe::calendar::jd_to_cal(double jd, bool gregorian, int &yr, int &mo, double &day) {
    /* Convert a Julian day number to a date in the Julian or Gregorian calendars.
    
    Parameters:
        jd        : Julian Day number
        gregorian : If true, use Gregorian calendar, else use Julian calendar (default: true)
        
    Return:
        year
        month
        day (may be fractional)

    Return a tuple (year, month, day). 
    
    */
    double Z;
    const double F = modf(jd + 0.5, &Z);
    int A;
    if (gregorian) {
        int alpha = int((Z - 1867216.25) / 36524.25);
        A = int(Z + 1 + alpha - alpha / 4);
        }
    else
        A = int(Z);
    const int B = A + 1524;
    const int C = int((B - 122.1) / 365.25);
    const int D = int(365.25 * C);
    const int E = int((B - D) / 30.6001);
    day = B - D - int(30.6001 * E) + F;
    if (E < 14) 
        mo = E - 1;
    else
        mo = E - 13;
    if (mo > 2)
        yr = C - 4716;
    else
        yr = C - 4715;
    }

int astrolabe::calendar::jd_to_day_of_week(double jd) {
    /* Return the day of week for a Julian Day Number.
    
    The Julian Day Number must be for 0h UT.

    Parameters:
        jd : Julian Day number
        
    Return:
        day of week: 0 = Sunday...6 = Saturday.
    
    */
    const int i = int(jd + 1.5);
    return i % 7;
    }

double astrolabe::calendar::jd_to_jcent(double jd) {
    /* Return the number of Julian centuries since J2000.0

    Parameters:
        jd : Julian Day number
        
    Return:
        Julian centuries
        
    */
    return (jd - 2451545.0) / 36525.0;
    }

string astrolabe::calendar::lt_to_str(double jd, const string &zone, const string &level) {
    /* Convert local time in Julian Days to a formatted string.
    
    The general format is:
    
        YYYY-MMM-DD HH:MM:SS ZZZ
    
    Truncate the time value to seconds, minutes, hours or days as
    indicated. If level = "day", don't print the time zone string.
    
    Pass an empty string ("", the default) for zone if you want to do 
    your own zone formatting in the calling module.
    
    Parameters:
        jd    : Julian Day number
        zone  : Time zone string (default = "")
        level : "day" or "hour" or "minute" or "second" (default = "second")
        
    Return:
        formatted date/time string
    
    */
    int yr;
    int mon;
    double day;
    jd_to_cal(jd, true, yr, mon, day);
    double zday;
    const double fday = modf(day, &zday);
    const int iday = int(zday);
    int hr;
    int mn;
    int sec;
    fday_to_hms(fday, hr, mn, sec);
    string month = monthToString[mon];

    char cstr[50];
    if (level == "second") {
        sprintf(cstr, "%d-%s-%02d %02d:%02d:%02d %s",  yr, month.c_str(), iday, hr, mn, sec, zone.c_str());
        return cstr;
        }
    if (level == "minute") {
        sprintf(cstr, "%d-%s-%02d %02d:%02d %s", yr, month.c_str(), iday, hr, mn, zone.c_str());
        return cstr;
        }
    if (level == "hour") {    
        sprintf(cstr, "%d-%s-%02d %02d %s", yr, month.c_str(), iday, hr, zone.c_str());
        return cstr;
        }
    if (level == "day") {    
        sprintf(cstr, "%d-%s-%02d", yr, month.c_str(), iday);
        return cstr;
        }
    throw Error("astrolabe::calendar::lt_to_str: unknown time level = " + level);
    }

double astrolabe::calendar::sidereal_time_greenwich(double jd) {
    /* Return the mean sidereal time at Greenwich.
    
    The Julian Day number must represent Universal Time.
    
    Parameters:
        jd : Julian Day number
        
    Return:
        sidereal time in radians (2pi radians = 24 hrs)
    
    */
    const double T = jd_to_jcent(jd);
    const double T2 = T * T;
    const double T3 = T2 * T;
    const double theta0 = 280.46061837 + 360.98564736629 * (jd - 2451545.0)  + 0.000387933 * T2 - T3 / 38710000;
    const double result = d_to_r(theta0);
    return modpi2(result);
    }

void astrolabe::calendar::ut_to_lt(double jd, double &lt, string &zone) {
    /* Convert universal time in Julian Days to a local time.
    
    Include Daylight Savings Time offset, if any.
        
    Parameters:
        jd : Julian Day number, universal time
        
    Return:
        Julian Day number, local time
        zone string of the zone used for the conversion

    */
    double offset;
    if (is_dst(jd)) {
        zone = astrolabe::globals::daylight_timezone_name;
        offset = astrolabe::globals::daylight_timezone_offset;
        }
    else {
        zone = astrolabe::globals::standard_timezone_name;
        offset = astrolabe::globals::standard_timezone_offset;
        }
        
    lt = jd - offset;
    }
