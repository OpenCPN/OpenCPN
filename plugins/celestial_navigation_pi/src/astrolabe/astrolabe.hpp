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

// astrolabe.hh

#ifndef _astrolabe_hpp_
#define _astrolabe_hpp_

#ifdef _MSC_VER
// identifier truncated for debugger
#pragma warning(disable:4786) 
#endif

#include <exception>
#include <map>
#include <string>
#include <vector>

// macros

//
// The number of elements in a C-array
//
#define ARRAY_SIZE(tbl) (sizeof(tbl) / sizeof(*tbl))

//
// Declare a vector<> and initialize it from a C-array.
//
// Declaring VECTOR(x, type) presumes that a C-array called "_x" of "type"
// already exists.
//
// We wouldn't have to do this if it were possible to initialize STL
// containers directly, but I don't see how to to that...
//
#define VECTOR(vec, type) vector<type> vec(_##vec, _##vec + ARRAY_SIZE(_##vec)) 

namespace astrolabe {
    // types
    enum Coords {vL, vB, vR};
    enum vPlanets {vMercury, vVenus, vEarth, vMars, vJupiter, vSaturn, vUranus, vNeptune};
    enum Season {kSpring, kSummer, kAutumn, kWinter};

    // Exception class
    class Error : public std::exception {
        public:
            Error(const std::string &msg) :
                msg(msg) {};
            ~Error() throw() {};
            const char *what() const throw() {
                return msg.c_str();
                };
        private:
            const std::string msg;
        };

    namespace constants {
        // Don't change these unless you are moving to a new universe.

        const double pi = 3.1415926535897932;
        //
        // The number of radians in a circle.
        //
        const double pi2 = 2 * pi;

        //
        // Ratio of Earth's polar to equitorial radius.
        //
        const double flattening = 0.99664719;

        //
        // Equitorial radius of the Earth in km.
        //
        const double earth_equ_radius = 6378.14;

        //
        // How many minutes in a day?
        //
        const double minutes_per_day = 24.0 * 60.0;

        //
        // How many days in minute?
        //
        const double days_per_minute = 1.0 / minutes_per_day;

        //
        // How many seconds (time) in a day?
        //
        const double seconds_per_day = 24.0 * 60.0 * 60.0;

        //
        // How many days in a second?
        //
        const double days_per_second = 1.0 / seconds_per_day;

        //
        // How many kilometers in an astronomical unit?
        //
        const double km_per_au = 149597870;

        //
        // For rise-set-transit: altitude deflection caused by refraction
        //
        const double standard_rst_altitude = -0.00989078087105; // -0.5667 degrees
        const double sun_rst_altitude = -0.0145438286569;       // -0.8333 degrees
        };

    namespace globals {        
        extern std::string standard_timezone_name;
        extern double standard_timezone_offset;   
        extern std::string daylight_timezone_name;         
        extern double daylight_timezone_offset;
        extern double longitude;
        extern double latitude;
        extern std::string vsop87d_text_path;
        extern std::string vsop87d_binary_path;
        };

    namespace calendar {        
        // calendar
        double cal_to_jd(int yr, int mo = 1, double day = 1, bool gregorian = true);
        int cal_to_day_of_year(int yr, int mo, int dy, bool gregorian = true);
        void day_of_year_to_cal(int yr, int N, bool gregorian, int &mo, int &dy);
        void easter(int yr, bool gregorian, int &mo, int &dy);
        bool is_dst(double jd);
        bool is_leap_year(int yr, bool gregorian = true);
        void jd_to_cal(double jd, bool gregorian, int &yr, int &mo, double &day);
        int jd_to_day_of_week(double jd);
        double jd_to_jcent(double jd);
		std::string lt_to_str(double jd, const std::string &zone = "", const std::string &level = "second");
        double sidereal_time_greenwich(double jd);
        void ut_to_lt(double jd, double &lt, std::string &zone);
        };

    namespace dicts {        
        // dicts
        class StringToPlanet {
            public:
                StringToPlanet();
                const vPlanets &operator[](const std::string &name) const;
            private:
                std::map<std::string, vPlanets> pmap;
            };

        class StringToCoord {
            public:
                StringToCoord();
                const Coords &operator[](const std::string &name) const;
            private:
                std::map<std::string, Coords> pmap;
            };

        class MonthToString {
            public:
                MonthToString();
                const std::string &operator[](int month) const;
            private:
                std::map<int, std::string> pmap;
            };

        class SeasonToString {
            public:
                SeasonToString();
                const std::string &operator[](Season season) const;
            private:
                std::map<Season, std::string> pmap;
            };

        class PlanetToString {
            public:
                PlanetToString();
                const std::string &operator[](vPlanets planet) const;
            private:
                std::map<vPlanets, std::string> pmap;
            };

        extern const StringToPlanet stringToPlanet;
        extern const StringToCoord stringToCoord;
        extern const MonthToString monthToString;
        extern const SeasonToString seasonToString;
        extern const PlanetToString planetToString;
        };

    namespace dynamical {
        // dynamical
        double deltaT_seconds(double jd);
        double dt_to_ut(double jd);
        };

    namespace elp2000 {
        // elp2000
        class ELP2000 {
            public:
                void dimension3(double jd, double &longitude, double &latitude, double &radius) const;
                double dimension(double jd, Coords dim) const;
            private:
                double longitude(double jd) const;
                double latitude(double jd) const;
                double radius(double jd) const;
            };
        };

    namespace equinox {        
        // equinox
        double equinox_approx(int yr, Season season);
        double equinox_exact(double jd, Season season, double delta);
        };

    namespace nutation {
        // nutation
        double nut_in_lon(double jd);
        double nut_in_obl(double jd);
        double obliquity(double jd);
        double obliquity_hi(double jd);
        };

    namespace riseset {
        // riseset
        double rise(double jd, const std::vector<double> &raList, const std::vector<double> &decList, double h0, double delta);
        double set(double jd, const std::vector<double> &raList, const std::vector<double> &decList, double h0, double delta);
        double transit(double jd, const std::vector<double> &raList, double delta);
        double moon_rst_altitude(double r);
        };
        
    namespace util {
        // util
        void d_to_dms(double x, int &deg, int &mn, double &sec);
        double d_to_r(double d);
        double diff_angle(double a, double b);
        double dms_to_d(int deg, int min, double sec);
        void ecl_to_equ(double longitude, double latitude, double obliquity, double &ra, double &dec);
        void equ_to_horiz(double H, double decl, double &A, double &h);
        void equ_to_ecl(double ra, double dec, double obliquity, double &longitude, double &latitude);
        void fday_to_hms(double day, int &hour, int &min, int &sec);
        double hms_to_fday(int hr, int mn, double sec);
        std::string int_to_string(int i);
        double interpolate3(double n, const std::vector<double> &y);
        double interpolate_angle3(double n, const std::vector<double> &y);
        void load_params();
        std::string lower(const std::string &str);
        double modpi2(double x);
        double polynomial(const std::vector<double> &terms, double x);
        double r_to_d(double r);
	std::vector<std::string> split(const std::string &str);
        double string_to_double(const std::string &str);
        int string_to_int(const std::string &str);
        std::string strip(const std::string &str);
        std::string upper(const std::string &str);
        };

    namespace vsop87d {
        // vsop87d
        class VSOP87d {
            public:
                VSOP87d();
                void dimension3(double jd, vPlanets planet, double &longitude, double &latitude, double &radius) const;
                double dimension(double jd, vPlanets planet, Coords dim) const;
            };

        void vsop_to_fk5(double jd, double &L, double &B);
        void geocentric_planet(double jd, vPlanets planet, double deltaPsi, double epsilon, double delta, double &ra, double &dec);
        void load_vsop87d_text_db();
        };

    namespace sun {        
        // sun
        class Sun {
            public:
                void dimension3(double jd, double &longitude, double &latitude, double &radius) const;
                double dimension(double jd, Coords dim) const;
            private:
                vsop87d::VSOP87d vsop;
            };

        void longitude_radius_low(double jd, double &L, double &R);
        double apparent_longitude_low(double jd, double L);
        double aberration_low(double R);
        };
    };

#endif
