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

// Calculate the times of solstice and equinox events for Earth

#include <cmath>
#include "astrolabe.hpp"

using std::vector;
using std::map;

using astrolabe::calendar::jd_to_jcent;
using astrolabe::constants::pi;
using astrolabe::constants::pi2;
using astrolabe::Error;
using astrolabe::kAutumn;
using astrolabe::kSpring;
using astrolabe::kSummer;
using astrolabe::kWinter;
using astrolabe::nutation::nut_in_lon;
using astrolabe::Season;
using astrolabe::sun::aberration_low;
using astrolabe::sun::Sun;
using astrolabe::util::diff_angle;
using astrolabe::util::d_to_r;
using astrolabe::util::int_to_string;
using astrolabe::util::polynomial;
using astrolabe::vsop87d::vsop_to_fk5;

enum Epoch {pre1000, post1000};

class Key {
    public:
        Key(Epoch epoch, Season season) :
            epoch(epoch),
            season(season) {};
        bool operator<(const Key &rhs) const {
            if (epoch < rhs.epoch)
                return true;
            if (epoch > rhs.epoch)
                return false;
            return (season < rhs.season);
            };
        
        const Epoch epoch;
        const Season season;
    };

class Value {
    public:
        Value() {};
        Value(double d0, double d1, double d2, double d3, double d4) {
            series.push_back(d0);
            series.push_back(d1);
            series.push_back(d2);
            series.push_back(d3);
            series.push_back(d4);
            };
        
        vector<double> series; // should be const
    };
    
struct Data {
    Epoch epoch;
    Season season;
    double d0, d1, d2, d3, d4;
    };

namespace {
    const Data _tbl[] = {
    //
    // Meeus-1998 Table 27.A
    //
        {pre1000, kSpring, 1721139.29189, 365242.13740,  0.06134,  0.00111, -0.00071},
        {pre1000, kSummer, 1721233.25401, 365241.72562, -0.05323,  0.00907, -0.00025},
        {pre1000, kAutumn, 1721325.70455, 365242.49558, -0.11677, -0.00297,  0.00074},
        {pre1000, kWinter, 1721414.39987, 365242.88257, -0.00769, -0.00933, -0.00006},
    //
    // Meeus-1998 Table 27.B
    //
        {post1000, kSpring, 2451623.80984, 365242.37404,  0.05169, -0.00411, -0.00057},
        {post1000, kSummer, 2451716.56767, 365241.62603,  0.00325,  0.00888, -0.00030},
        {post1000, kAutumn, 2451810.21715, 365242.01767, -0.11575,  0.00337,  0.00078},
        {post1000, kWinter, 2451900.05952, 365242.74049, -0.06223, -0.00823,  0.00032}
        };
    };
        
class Table {
    public:
        Table(const Data *begin, const Data *end) {
            for (const Data *p = begin; p != end; p++)
                table[Key(p->epoch, p->season)] = Value(p->d0, p->d1, p->d2, p->d3, p->d4);
            };      
        map<Key, Value> table; // should be const
    };

namespace {
    Table table(_tbl, _tbl + ARRAY_SIZE(_tbl));
    };

//
// Meeus-1998 Table 27.C
//
struct Term {
    int A;
    double B, C;
    };
    
namespace {
    const Term _terms[] = {
        {485, d_to_r(324.96),  d_to_r(  1934.136)},
        {203, d_to_r(337.23),  d_to_r( 32964.467)},
        {199, d_to_r(342.08),  d_to_r(    20.186)},
        {182, d_to_r( 27.85),  d_to_r(445267.112)},
        {156, d_to_r( 73.14),  d_to_r( 45036.886)},
        {136, d_to_r(171.52),  d_to_r( 22518.443)},
        { 77, d_to_r(222.54),  d_to_r( 65928.934)},
        { 74, d_to_r(296.72),  d_to_r(  3034.906)},
        { 70, d_to_r(243.58),  d_to_r(  9037.513)},
        { 58, d_to_r(119.81),  d_to_r( 33718.147)},
        { 52, d_to_r(297.17),  d_to_r(   150.678)},
        { 50, d_to_r( 21.02),  d_to_r(  2281.226)},
        { 45, d_to_r(247.54),  d_to_r( 29929.562)},
        { 44, d_to_r(325.15),  d_to_r( 31555.956)},
        { 29, d_to_r( 60.93),  d_to_r(  4443.417)},
        { 18, d_to_r(155.12),  d_to_r( 67555.328)},
        { 17, d_to_r(288.79),  d_to_r(  4562.452)},
        { 16, d_to_r(198.04),  d_to_r( 62894.029)},
        { 14, d_to_r(199.76),  d_to_r( 31436.921)},
        { 12, d_to_r( 95.39),  d_to_r( 14577.848)},
        { 12, d_to_r(287.11),  d_to_r( 31931.756)},
        { 12, d_to_r(320.81),  d_to_r( 34777.259)},
        {  9, d_to_r(227.73),  d_to_r(  1222.114)},
        {  8, d_to_r( 15.45),  d_to_r( 16859.074)}
        };

    const VECTOR(terms, Term);
    };
        
    
double astrolabe::equinox::equinox_approx(int yr, Season season) {
    /* Returns the approximate time of a solstice or equinox event.
    
    The year must be in the range -1000...3000. Within that range the
    the error from the precise instant is at most 2.16 minutes.
    
    Parameters:
        yr     : year
        season : one of ("spring", "summer", "autumn", "winter")
    
    Returns:
        Julian Day of the event in dynamical time
    
    */
    if (yr < -1000 || yr > 3000) 
        throw Error("astrolabe::equinox::equinox_approx: year is out of range = " + int_to_string(yr));
    if (season < kSpring || season > kWinter)
        throw Error("astrolabe::equinox::equinox_approx: unknown season = " + int_to_string(season));

    double Y; 
    Epoch epoch;   
    if (-1000 <= yr && yr <= 1000) {
        Y = yr / 1000.0;
        epoch = pre1000;
        }
    else {
        Y = (yr - 2000) / 1000.0;
        epoch = post1000;
        }

    const Key key(epoch, season);
    const vector<double> &poly = table.table[key].series;
    double jd = polynomial(poly, Y);
    
    const double T = jd_to_jcent(jd);
    const double W = d_to_r(35999.373 * T - 2.47);
    const double delta_lambda = 1 + 0.0334 * cos(W) + 0.0007 * cos(2 * W);
    double S = 0.0;
    for (std::vector<Term>::const_iterator p = terms.begin(); p != terms.end(); p++) 
        S += p->A * cos(p->B + p->C * T);
    jd += 0.00001 * S / delta_lambda;
    return jd;
    }

namespace {
    class SeasonToCircle {
        public:
            SeasonToCircle() {
                class Data {
		    public:
			Data(Season season, double circle) : season(season), circle(circle) {};

                        Season season;
			double circle;
                    };

                const Data tbl[] = {            
                    Data(kSpring, 0),
                    Data(kSummer, pi * 0.5),
                    Data(kAutumn, pi),
                    Data(kWinter, pi * 1.5)
                    };
                for (const Data *p = tbl; p != tbl + ARRAY_SIZE(tbl); p++)
                    pmap[p->season] = p->circle;
                };

            const double &operator[](Season season) const {
	        std::map<Season, double>::const_iterator p = pmap.find(season);
                if (p == pmap.end())
                    throw Error("astrolabe::equinox::SeasonToCircle::const double &operator[]: season out of range = " + int_to_string(season));
                return p->second;
                };

        private:
	    map<Season, double> pmap;
        };

    const SeasonToCircle seasonToCircle;
    };
    
double astrolabe::equinox::equinox_exact(double jd, Season season, double delta) {
    /* Return the precise moment of an equinox or solstice event on Earth.
    
    Parameters:
        jd     : Julian Day of an approximate time of the event in dynamical time
        season : one of ("spring", "summer", "autumn", "winter")
        model  : the solar position model, which must be an object that responds that
            to the dimension3() method. Actually, we presume this is an
            instance of astrolabe.sun.Sun() because we use the VSOP coordinate
            correction function. 
        delta  : the required precision in days. Times accurate to a second are
            reasonable when using the VSOP model.
        
    Returns:
        Julian Day of the event in dynamical time

    */
    //
    // If we knew that the starting approximate time was close enough
    // to the actual time, we could pull nut_in_lon() and the aberration
    // out of the loop and save some calculating.
    //
    static const double _k_sun_motion = 365.25 / pi2;
    const double circ = seasonToCircle[season];
    Sun sun;
    for (int i = 0; i < 20; i++) {
        const double jd0 = jd;
        double L, B, R;
        sun.dimension3(jd, L, B, R);
        L += nut_in_lon(jd) + aberration_low(R);
        vsop_to_fk5(jd, L, B);
        // Meeus uses jd + 58 * sin(diff(...))
        jd += diff_angle(L, circ) * _k_sun_motion;
        if (fabs(jd - jd0) < delta)
            return jd;
        }
    throw Error("astrolabe::equinox::equinox_exact: bailout");
    }
