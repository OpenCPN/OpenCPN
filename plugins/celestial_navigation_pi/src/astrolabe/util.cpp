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

/* Collection of miscellaneous functions */

#include "astrolabe.hpp"
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <list>
#include <sstream>

#include <string.h>

using std::string;
using std::vector;
using std::ifstream;
using std::list;
using std::getline;
using std::ostringstream;

using astrolabe::Error;
using astrolabe::constants::pi;
using astrolabe::constants::pi2;
using astrolabe::constants::seconds_per_day;
using astrolabe::constants::minutes_per_day;
using astrolabe::util::split;

void astrolabe::util::d_to_dms(double x, int &deg, int &mn, double &sec) {
    /* Convert an angle in decimal degrees to degree components.
    
    Return a tuple (degrees, minutes, seconds). Degrees and minutes
    will be integers, seconds may be floating.
    
    If the argument is negative: 
        The return value of degrees will be negative. 
        If degrees is 0, minutes will be negative. 
        If minutes is 0, seconds will be negative.
        
    Parameters:
        x : degrees
    
    Returns:
        degrees
        minutes
        seconds
        
    */
    const bool negative = (x < 0.0);
    x = fabs(x);
    deg = int(x);
    x -= deg;
    mn = int(x * 60);
    x -= mn / 60.0;
    sec = x * 3600;
    
    if (negative) {
        if (deg > 0)
            deg = -deg;
        else if (mn > 0)
            mn = -mn;
        else
            sec = -sec;
    }
}

double astrolabe::util::d_to_r(double d) {
    /*Convert degrees to radians.
    
    Parameters:
        d : degrees
        
    Returns:
        radians
    
    */
    static const double _DtoR = pi / 180.0;

    return d * _DtoR;
    }

double astrolabe::util::diff_angle(double a, double b) {
    /* Return angle b - a, accounting for circular values.
    
    Parameters a and b should be in the range 0..pi*2. The
    result will be in the range -pi..pi.
    
    This allows us to directly compare angles which cross through 0:
    
        359 degress... 0 degrees... 1 degree... etc
        
    Parameters:
        a : first angle, in radians
        b : second angle, in radians
        
    Returns:
        b - a, in radians
    
    */
    double result;
    if (b < a)
        result = b + pi2 - a;
    else
        result = b - a;
    if (result > pi)
        result -= pi2;
    return result;
    }

double astrolabe::util::dms_to_d(int deg, int min, double sec) {
    /* Convert an angle in degree components to decimal degrees. 
    
    If any of the components are negative the result will also be negative.
    
    Parameters:
        deg : degrees
        min : minutes
        sec : seconds
        
    Returns:
        decimal degrees
    
    */
    double result = abs(deg) + abs(min) / 60.0 + fabs(sec) / 3600.0;
    if (deg < 0 || min < 0 || sec < 0)
        result = -result;
    return result;
    }

void astrolabe::util::ecl_to_equ(double longitude, double latitude, double obliquity, double &ra, double &dec) {
    /* Convert ecliptic to equitorial coordinates. 
    
    [Meeus-1998: equations 13.3, 13.4]
    
    Parameters:
        longitude : ecliptic longitude in radians
        latitude : ecliptic latitude in radians
        obliquity : obliquity of the ecliptic in radians
    
    Returns:
        Right accension in radians
        Declination in radians
    
    */
    const double cose = cos(obliquity);
    const double sine = sin(obliquity);
    const double sinl = sin(longitude);
    ra = modpi2(atan2(sinl * cose - tan(latitude) * sine, cos(longitude)));
    dec = asin(sin(latitude) * cose + cos(latitude) * sine * sinl);
    }

void astrolabe::util::equ_to_horiz(double H, double decl, double &A, double &h) {
    /* Convert equitorial to horizontal coordinates.
    
    [Meeus-1998: equations 13.5, 13.6]

    Note that azimuth is measured westward starting from the south.
    
    This is mot a good formula for using near the poles.
    
    Parameters:
        H : hour angle in radians
        decl : declination in radians
        
    Returns:
        azimuth in radians
        altitude in radians
    
    */
    const double cosH = cos(H);
    const double sinLat = sin(astrolabe::globals::latitude);
    const double cosLat = cos(astrolabe::globals::latitude);
    A = atan2(sin(H), cosH * sinLat - tan(decl) * cosLat);
    h = asin(sinLat * sin(decl) + cosLat * cos(decl) * cosH);
    }
    
void astrolabe::util::equ_to_ecl(double ra, double dec, double obliquity, double &longitude, double &latitude) {
    /* Convert equitorial to ecliptic coordinates. 
    
    [Meeus-1998: equations 13.1, 13.2]
    
    Parameters:
        ra : right accension in radians
        dec : declination in radians
        obliquity : obliquity of the ecliptic in radians
        
    Returns:
        ecliptic longitude in radians
        ecliptic latitude in radians
    
    */
    const double cose = cos(obliquity);
    const double sine = sin(obliquity);
    const double sina = sin(ra);
    longitude = modpi2(atan2(sina * cose + tan(dec) * sine, cos(ra)));
    latitude = modpi2(asin(sin(dec) * cose - cos(dec) * sine * sina));
    }
    
void astrolabe::util::fday_to_hms(double day, int &hour, int &min, int &sec) {
    /* Convert fractional day (0.0..1.0) to integral hours, minutes, seconds.

    Parameters:
        day : a fractional day in the range 0.0..1.0
        
    Returns:
        hour : 0..23
        minute : 0..59
        seccond : 0..59
    
    */
    const long tsec = long(day * seconds_per_day);
    const int tmin = tsec / 60;
    const int thour = tmin / 60;
    hour = thour % 24;
    min = tmin % 60;
    sec = tsec % 60;
    }

double astrolabe::util::hms_to_fday(int hr, int mn, double sec) {
    /* Convert hours-minutes-seconds into a fractional day 0.0..1.0.
    
    Parameters:
        hr : hours, 0..23
        mn : minutes, 0..59
        sec : seconds, 0..59
        
    Returns:
        fractional day, 0.0..1.0
    
    */
    return ((hr  / 24.0) + (mn  / minutes_per_day) + (sec / seconds_per_day));
    }
          
string astrolabe::util::int_to_string(int i) {
    /* Format an integer as a string.
    
    Parameters:
        i : integer
    
    Returns:
        string representation
        
    */
    ostringstream ost;
    ost << i;
    return ost.str();
    }
    
double astrolabe::util::interpolate3(double n, const vector<double> &y) {
    /* Interpolate from three equally spaced tabular values.
    
    [Meeus-1998; equation 3.3]
    
    Parameters:
        n : the interpolating factor, must be between -1 and 1
        y : a sequence of three values 
    
    Results:
        the interpolated value of y
        
    */
    if (n < -1 || n > 1)
        throw Error("astrolabe::util::interpolate3: interpolating factor out of range");
        
    const double a = y[1] - y[0];
    const double b = y[2] - y[1];
    const double c = b - a;
    return y[1] + n/2 * (a + b + n*c);
    }

double astrolabe::util::interpolate_angle3(double n, const vector<double> &y) {
    /* Interpolate from three equally spaced tabular angular values.
    
    [Meeus-1998; equation 3.3]
    
    This version is suitable for interpolating from a table of
    angular values which may cross the origin of the circle, 
    for example: 359 degrees...0 degrees...1 degree.
    
    Parameters:
        n : the interpolating factor, must be between -1 and 1
        y : a sequence of three values 
    
    Results:
        the interpolated value of y
        
    */
    if (n < -1 || n > 1)
        throw Error("astrolabe::util::interpolate_angle3: interpolating factor out of range");

    const double a = diff_angle(y[0], y[1]);
    const double b = diff_angle(y[1], y[2]);
    const double c = diff_angle(a, b);
    return y[1] + n/2 * (a + b + n*c);
    }

/* Token and Lex are used for parsing the parameter file. */
class Token {
    public:
        Token(const string &value, int line) : value(value), line(line) {};

        const string value;
        const int line;
    };

class Lex {
    public:
        Lex(ifstream &infile) {
            string line;
            getline(infile, line);
            int count = 1;
            while(infile) {
                const vector<string> fields = split(line);
                for (std::vector<string>::const_iterator p = fields.begin(); p != fields.end(); p++) {
                    const string str = *p;
                    if (str[0] == '#')
                        break;
                    tokens.push_back(Token(str, count));
                    }
                getline(infile, line);
                count++;
                }
            p = tokens.begin();
            };

        string get_token() {
            if (p == tokens.end()) {
                return "";
                }
            string token = p->value;
            lineno = p->line;
            p++;
            return token;
            };

        int lineno;

    private:
        list<Token> tokens;
	std::list<Token>::const_iterator p;
    };
        
void astrolabe::util::load_params() {
    /*Read a parameter file and assign global values.

      This is ugly code. Will have to be cleanup up.
    
    Parameters:
        none

    Returns: 
        nothing
    
    */
    const char *fname = getenv("ASTROLABE_PARAMS");
    if (!fname)
        fname = "astrolabe_params.txt";
    ifstream infile(fname);
    if (!infile)
        throw Error("astrolabe::util::load_params: unable to open param file. Either set ASTROLABE_PARAMS correctly or create astrolabe_params.txt in the current directory");

    Lex lex(infile);
    infile.close();
    string token = lex.get_token();
    while (!token.empty()) {
        if (token == "standard_timezone_name")
            astrolabe::globals::standard_timezone_name = lex.get_token();
        else if (token == "standard_timezone_offset") {
            double offset = string_to_double(lex.get_token());
            string unit = lower(lex.get_token());
            if (unit != "day" && unit != "days" && unit != "hour" && unit != "hours" && unit != "minute" && unit != "minutes" && unit != "second" && unit != "seconds")
                throw Error("astrolabe::util::load_params: bad value for standard_timezone_offset units");
            if (unit == "hour" || unit == "hours")
                offset /= 24.0;
            else if (unit == "minute" || unit == "minutes")
                offset /= minutes_per_day;
            else if (unit == "second" || unit == "seconds")
                offset /= seconds_per_day;
            astrolabe::globals::standard_timezone_offset = offset;
            }                
        else if (token == "daylight_timezone_name")
            astrolabe::globals::daylight_timezone_name = lex.get_token();
        else if (token == "daylight_timezone_offset") {
            double offset = string_to_double(lex.get_token());
            string unit = lower(lex.get_token());
            if (unit != "day" && unit != "days" && unit != "hour" && unit != "hours" && unit != "minute" && unit != "minutes" && unit != "second" && unit != "seconds")
                throw Error("astrolabe::util::load_params: bad value for standard_timezone_offset units");
            if (unit == "hour" || unit == "hours")
                offset /= 24.0;
            else if (unit == "minute" || unit == "minutes")
                offset /= minutes_per_day;
            else if (unit == "second" || unit == "seconds")
                offset /= seconds_per_day;
            astrolabe::globals::daylight_timezone_offset = offset;
            }
        else if (token == "longitude") {
            double xlongitude = string_to_double(lex.get_token());
            string direction = lower(lex.get_token());
            if (direction != "east" && direction != "west")
                throw Error("astrolabe::util::load_params: longitude direction must be 'west' or 'east'");
            if (direction == "east")
                xlongitude = -xlongitude;
            astrolabe::globals::longitude = d_to_r(xlongitude);
            }
        else if (token == "latitude") {
            double xlatitude = string_to_double(lex.get_token());
            string direction = lower(lex.get_token());
            if (direction != "north" && direction != "south") 
                throw Error("astrolabe::util::load_params: latitude direction must be 'north' or 'south'");
            if (direction == "south")
                xlatitude = -xlatitude;
            astrolabe::globals::latitude = d_to_r(xlatitude);
            }
        else if (token == "vsop87d_text_path")
            astrolabe::globals::vsop87d_text_path = lex.get_token();
        else if (token == "vsop87d_binary_path")
            astrolabe::globals::vsop87d_binary_path = lex.get_token();
        else
            throw Error("astrolabe::util::load_params: unknown token '" + token + "' at line " + int_to_string(lex.lineno) + " in param file");
        token = lex.get_token();
        }
    }

string astrolabe::util::lower(const string &str) {
    /* Return lowercase version of string
    
    Parameters:
        str : a string
    
    Returns:
        lowercased version
    
    */
    string newstr;
    for (string::const_iterator p = str.begin(); p != str.end(); p++)
	    newstr += tolower(*p);
    return newstr;
    }
    
double astrolabe::util::modpi2(double x) {
    /* Reduce an angle in radians to the range 0..2pi.
    
    Parameters:
        x : angle in radians
        
    Returns:
        angle in radians in the range 0..2pi
    
    */
    x = fmod(x, pi2);
    if (x < 0.0) 
        x += pi2;
    return x;
    }

double astrolabe::util::polynomial(const vector<double> &terms, double x) {
    /* Evaluate a simple polynomial.
    
    Where: terms[0] is constant, terms[1] is for x, terms[2] is for x^2, etc.
    
    Example:
        y = polynomial((1.1, 2.2, 3.3, 4.4), t)
        
        returns the value of:
        
            1.1 + 2.2 * t + 3.3 * t^2 + 4.4 * t^3
    
    Parameters:
        terms : sequence of coefficients
        x : variable value
        
    Results:
        value of the polynomial
    
    */
    std::vector<double>::const_reverse_iterator p = terms.rbegin();
    double result = *p;
    for (p++; p != terms.rend(); p++)
        result = result * x + *p;
    return result;
    }
    
double astrolabe::util::r_to_d(double r) {
    /* Convert radians to degrees.
    
    Parameters:
        r : radians
        
    Returns:
        degrees
    
    */
    static const double _RtoD = 180.0 / pi;

    return r * _RtoD;
    }
    
vector<string> astrolabe::util::split(const string &str) {
    /* Split the whitespace-separated fields of a string.
    
    Parameters:
        str : a string
        
    Returns:
        a vector<> of strings
    
    */
    vector<string> results;
#ifdef _MSC_VER
	char cstr[1000];
#else
    char cstr[str.length() + 1];
#endif
    str.copy(cstr, string::npos);
    cstr[str.length()] = 0;
    const char delim[] = " \t\n";
    char *p = strtok(cstr, delim);
    while(p) {
        results.push_back(p);
        p = strtok(NULL, delim);
        }
    return results;
    }
    
string astrolabe::util::strip(const string &str) {
    /* Remove leading and trailing whitespace from a string.
    
    Parameters:
        str : a string
        
    Returns:
        the string without the leading and trailing whitespace
    
    */
    unsigned int start;
    for (start = 0; start < str.length(); start++)
        if (!isspace(str[start]))
            break;
            
    if (start == str.length())
        return "";
    
    unsigned int stop;
    for (stop = str.length() - 1; stop >= start; stop--)
        if (!isspace(str[stop]))
            break;
            
    return str.substr(start, stop - start + 1);
    }

// had trouble simply overloading atof().
double astrolabe::util::string_to_double(const string &str) {
    /* Convert a string to a floating point value
    
    Parameters:
        str : a string
        
    Returns:
        a double-precision floating point number
    */
    return atof(str.c_str());
    }

// had trouble simply overloading atoi().
int astrolabe::util::string_to_int(const string &str) {
    /* Convert a string to an integer value
    
    Parameters:
        str : a string
        
    Returns:
        an integer
    */
    return atoi(str.c_str());
    }

string astrolabe::util::upper(const string &str) {
    /* Convert a string to all upper case.
    
    Parameters:
        str : a string
        
    Returns:
        the uppercased version of the string
        
    */
    string newstr;
    for (string::const_iterator p = str.begin(); p != str.end(); p++)
	    newstr += toupper(*p);
    return newstr;
    }
