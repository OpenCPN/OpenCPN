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

/* Some classes to provide string->enum and other mappings. */

#include "astrolabe.hpp"

using std::string;
using std::map;

using astrolabe::vPlanets;
using astrolabe::Coords;
using astrolabe::util::int_to_string;

// externally visible objects
const astrolabe::dicts::StringToPlanet astrolabe::dicts::stringToPlanet;
const astrolabe::dicts::StringToCoord  astrolabe::dicts::stringToCoord;
const astrolabe::dicts::MonthToString  astrolabe::dicts::monthToString;
const astrolabe::dicts::SeasonToString  astrolabe::dicts::seasonToString;
const astrolabe::dicts::PlanetToString  astrolabe::dicts::planetToString;

//
// planet name -> vPlanets
//
astrolabe::dicts::StringToPlanet::StringToPlanet() {
    class Data {
        public:
	    Data(const string &name, vPlanets planet) : name(name), planet(planet) {};

	    string name;
	    vPlanets planet;
        };
    /*
    const Data tbl[] = {            
        {Data("Mercury", vMercury)},
        {Data("Venus", vVenus)},
        {Data("Earth", vEarth)},
        {Data("Mars", vMars)},
        {Data("Jupiter", vJupiter)},
        {Data("Saturn", vSaturn)},
        {Data("Uranus", vUranus)},
        {Data("Neptune", vNeptune)}
        };
    */
    const Data tbl[] = {            
        Data("Mercury", vMercury),
        Data("Venus", vVenus),
        Data("Earth", vEarth),
        Data("Mars", vMars),
        Data("Jupiter", vJupiter),
        Data("Saturn", vSaturn),
        Data("Uranus", vUranus),
        Data("Neptune", vNeptune)
        };
    for (const Data *p = tbl; p != tbl + ARRAY_SIZE(tbl); p++)
        pmap[p->name] = p->planet;
    }
    
const vPlanets &astrolabe::dicts::StringToPlanet::operator[](const string &name) const {
    std::map<string, vPlanets>::const_iterator p = pmap.find(name);
    if (p == pmap.end())
        throw Error("astrolabe::dicts::StringToPlanet::operator[]: unknown planet name = " + name);
    return p->second;
    }
    
//
// coordinate name -> Coords
//
astrolabe::dicts::StringToCoord::StringToCoord() {
    class Data {
        public:
	    Data(const string &name, Coords coord) : name(name), coord(coord) {};

            string name;
            Coords coord;
        };

    const Data tbl[] = {            
        Data("L", vL),
        Data("B", vB),
        Data("R", vR)
        };
    for (const Data *p = tbl; p != tbl + ARRAY_SIZE(tbl); p++)
        pmap[p->name] = p->coord;
    }
    
const Coords &astrolabe::dicts::StringToCoord::operator[](const string &name) const {
    std::map<string, Coords>::const_iterator p = pmap.find(name);
    if (p == pmap.end())
        throw Error("astrolabe::dicts::StringToCoord::operator[]: unknown coord name = " + name);
    return p->second;
    }
    
//
// integer -> month name string
//
// Month names. There must be twelve. The default is three-character
// abbreviations so that listings line up.
//
astrolabe::dicts::MonthToString::MonthToString() {
    class Data {
        public:
	    Data(int month, const string &name) : month(month), name(name) {};

            int month;
            string name;
        };

    const Data tbl[] = {            
        Data(1, "jan"),
        Data(2, "feb"),
        Data(3, "mar"),
        Data(4, "apr"),
        Data(5, "may"),
        Data(6, "jun"),
        Data(7, "jul"),
        Data(8, "aug"),
        Data(9, "sep"),
        Data(10, "oct"),
        Data(11, "nov"),
        Data(12, "dec"),
        };
    for (const Data *p = tbl; p != tbl + ARRAY_SIZE(tbl); p++)
        pmap[p->month] = p->name;
    }
    
const string &astrolabe::dicts::MonthToString::operator[](int month) const {
    std::map<int, string>::const_iterator p = pmap.find(month);
    if (p == pmap.end())
        throw Error("astrolabe::dicts::MonthToString::operator[]: month number out of range = " + int_to_string(month));
    return p->second;
    }
    
//
// Season -> season name string
//
// Month names. There must be twelve. The default is three-character
// abbreviations so that listings line up.
//
astrolabe::dicts::SeasonToString::SeasonToString() {
    class Data {
        public:
            Data(Season season, const string &name) : season(season), name(name) {};

            Season season;
            string name;
        };

    const Data tbl[] = {            
        Data(kSpring, "spring"),
        Data(kSummer, "summer"),
        Data(kAutumn, "autumn"),
        Data(kWinter, "winter")
        };
    for (const Data *p = tbl; p != tbl + ARRAY_SIZE(tbl); p++)
        pmap[p->season] = p->name;
    }
    
const string &astrolabe::dicts::SeasonToString::operator[](Season season) const {
    std::map<Season, string>::const_iterator p = pmap.find(season);
    if (p == pmap.end())
        throw Error("astrolabe::dicts::SeasonToString::operator[]: season out of range = " + int_to_string(season));
    return p->second;
    }
    
//
// vPlanets -> planet name string
//
astrolabe::dicts::PlanetToString::PlanetToString() {
    class Data {
        public:
            Data(vPlanets planet, const string &name) : planet(planet), name(name) {};

            vPlanets planet;
	    string name;
        };

    const Data tbl[] = {            
        Data(vMercury, "Mercury"),
        Data(vVenus, "Venus"),
        Data(vEarth, "Earth"),
        Data(vMars, "Mars"),
        Data(vJupiter, "Jupiter"),
        Data(vSaturn, "Saturn"),
        Data(vUranus, "Uranus"),
        Data(vNeptune, "Neptune")
        };
    for (const Data *p = tbl; p != tbl + ARRAY_SIZE(tbl); p++)
        pmap[p->planet] = p->name;
    }
    
const string &astrolabe::dicts::PlanetToString::operator[](vPlanets planet) const {
    std::map<vPlanets, string>::const_iterator p = pmap.find(planet);
    if (p == pmap.end())
        throw Error("astrolabe::dicts::PlanetToString::operator[]: planet out of range = " + int_to_string(planet));
    return p->second;
    }
    








