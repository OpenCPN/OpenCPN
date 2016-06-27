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
 */



#include <vector>
#include <utility>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "georef.h"
#include "cutil.h"


#ifdef __MSVC__
#define snprintf mysnprintf
#endif

#if !defined(NAN)
static const long long lNaN = 0xfff8000000000000;
#define NAN (*(double*)&lNaN)
#endif



//  ellipsoid: index into the gEllipsoid[] array, in which
//             a is the ellipsoid semimajor axis
//             invf is the inverse of the ellipsoid flattening f
//  dx, dy, dz: ellipsoid center with respect to WGS84 ellipsoid center
//    x axis is the prime meridian
//    y axis is 90 degrees east longitude
//    z axis is the axis of rotation of the ellipsoid

// The following values for dx, dy and dz were extracted from the output of
// the GARMIN PCX5 program. The output also includes values for da and df, the
// difference between the reference ellipsoid and the WGS84 ellipsoid semi-
// major axis and flattening, respectively. These are replaced by the
// data contained in the structure array gEllipsoid[], which was obtained from
// the Defence Mapping Agency document number TR8350.2, "Department of Defense
// World Geodetic System 1984."

struct DATUM const gDatum[] = {
//     name               ellipsoid   dx      dy       dz
      { "Adindan",                5,   -162,    -12,    206 },    // 0
      { "Afgooye",               15,    -43,   -163,     45 },    // 1
      { "Ain el Abd 1970",       14,   -150,   -251,     -2 },    // 2
      { "Anna 1 Astro 1965",      2,   -491,    -22,    435 },    // 3
      { "Arc 1950",               5,   -143,    -90,   -294 },    // 4
      { "Arc 1960",               5,   -160,     -8,   -300 },    // 5
      { "Ascension Island  58",  14,   -207,    107,     52 },    // 6
      { "Astro B4 Sorol Atoll",  14,    114,   -116,   -333 },    // 7
      { "Astro Beacon  E ",      14,    145,     75,   -272 },    // 8
      { "Astro DOS 71/4",        14,   -320,    550,   -494 },    // 9
      { "Astronomic Stn  52",    14,    124,   -234,    -25 },    // 10
      { "Australian Geod  66",    2,   -133,    -48,    148 },    // 11
      { "Australian Geod  84",    2,   -134,    -48,    149 },    // 12
      { "Bellevue (IGN)",        14,   -127,   -769,    472 },    // 13
      { "Bermuda 1957",           4,    -73,    213,    296 },    // 14
      { "Bogota Observatory",    14,    307,    304,   -318 },    // 15
      { "Campo Inchauspe",       14,   -148,    136,     90 },    // 16
      { "Canton Astro 1966",     14,    298,   -304,   -375 },    // 17
      { "Cape",                   5,   -136,   -108,   -292 },    // 18
      { "Cape Canaveral",         4,     -2,    150,    181 },    // 19
      { "Carthage",               5,   -263,      6,    431 },    // 20
      { "CH-1903",                3,    674,     15,    405 },    // 21
      { "Chatham 1971",          14,    175,    -38,    113 },    // 22
      { "Chua Astro",            14,   -134,    229,    -29 },    // 23
      { "Corrego Alegre",        14,   -206,    172,     -6 },    // 24
      { "Djakarta (Batavia)",     3,   -377,    681,    -50 },    // 25
      { "DOS 1968",              14,    230,   -199,   -752 },    // 26
      { "Easter Island 1967",    14,    211,    147,    111 },    // 27
      { "European 1950",         14,    -87,    -98,   -121 },    // 28
      { "European 1979",         14,    -86,    -98,   -119 },    // 29
      { "Finland Hayford",       14,    -78,   -231,    -97 },    // 30
      { "Gandajika Base",        14,   -133,   -321,     50 },    // 31
      { "Geodetic Datum  49",    14,     84,    -22,    209 },    // 32
      { "Guam 1963",              4,   -100,   -248,    259 },    // 33
      { "GUX 1 Astro",           14,    252,   -209,   -751 },    // 34
      { "Hermannskogel Datum",    3,    682,   -203,    480 },    // 35
      { "Hjorsey 1955",          14,    -73,     46,    -86 },    // 36
      { "Hong Kong 1963",        14,   -156,   -271,   -189 },    // 37
      { "Indian Bangladesh",      6,    289,    734,    257 },    // 38
      { "Indian Thailand",        6,    214,    836,    303 },    // 39
      { "Ireland 1965",           1,    506,   -122,    611 },    // 40
      { "ISTS 073 Astro  69",    14,    208,   -435,   -229 },    // 41
      { "Johnston Island",       14,    191,    -77,   -204 },    // 42
      { "Kandawala",              6,    -97,    787,     86 },    // 43
      { "Kerguelen Island",      14,    145,   -187,    103 },    // 44
      { "Kertau 1948",            7,    -11,    851,      5 },    // 45
      { "L.C. 5 Astro",           4,     42,    124,    147 },    // 46
      { "Liberia 1964",           5,    -90,     40,     88 },    // 47
      { "Luzon Mindanao",         4,   -133,    -79,    -72 },    // 48
      { "Luzon Philippines",      4,   -133,    -77,    -51 },    // 49
      { "Mahe 1971",              5,     41,   -220,   -134 },    // 50
      { "Marco Astro",           14,   -289,   -124,     60 },    // 51
      { "Massawa",                3,    639,    405,     60 },    // 52
      { "Merchich",               5,     31,    146,     47 },    // 53
      { "Midway Astro 1961",     14,    912,    -58,   1227 },    // 54
      { "Minna",                  5,    -92,    -93,    122 },    // 55
      { "NAD27 Alaska",           4,     -5,    135,    172 },    // 56
      { "NAD27 Bahamas",          4,     -4,    154,    178 },    // 57
      { "NAD27 Canada",           4,    -10,    158,    187 },    // 58
      { "NAD27 Canal Zone",       4,      0,    125,    201 },    // 59
      { "NAD27 Caribbean",        4,     -7,    152,    178 },    // 60
      { "NAD27 Central",          4,      0,    125,    194 },    // 61
      { "NAD27 CONUS",            4,     -8,    160,    176 },    // 62
      { "NAD27 Cuba",             4,     -9,    152,    178 },    // 63
      { "NAD27 Greenland",        4,     11,    114,    195 },    // 64
      { "NAD27 Mexico",           4,    -12,    130,    190 },    // 65
      { "NAD27 San Salvador",     4,      1,    140,    165 },    // 66
      { "NAD83",                 11,      0,      0,      0 },    // 67
      { "Nahrwn Masirah Ilnd",    5,   -247,   -148,    369 },    // 68
      { "Nahrwn Saudi Arbia",     5,   -231,   -196,    482 },    // 69
      { "Nahrwn United Arab",     5,   -249,   -156,    381 },    // 70
      { "Naparima BWI",          14,     -2,    374,    172 },    // 71
      { "Observatorio 1966",     14,   -425,   -169,     81 },    // 72
      { "Old Egyptian",          12,   -130,    110,    -13 },    // 73
      { "Old Hawaiian",           4,     61,   -285,   -181 },    // 74
      { "Oman",                   5,   -346,     -1,    224 },    // 75
      { "Ord Srvy Grt Britn",     0,    375,   -111,    431 },    // 76
      { "Pico De Las Nieves",    14,   -307,    -92,    127 },    // 77
      { "Pitcairn Astro 1967",   14,    185,    165,     42 },    // 78
      { "Prov So Amrican  56",   14,   -288,    175,   -376 },    // 79
      { "Prov So Chilean  63",   14,     16,    196,     93 },    // 80
      { "Puerto Rico",            4,     11,     72,   -101 },    // 81
      { "Qatar National",        14,   -128,   -283,     22 },    // 82
      { "Qornoq",                14,    164,    138,   -189 },    // 83
      { "Reunion",               14,     94,   -948,  -1262 },    // 84
      { "Rome 1940",             14,   -225,    -65,      9 },    // 85
      { "RT 90",                  3,    498,    -36,    568 },    // 86
      { "Santo (DOS)",           14,    170,     42,     84 },    // 87
      { "Sao Braz",              14,   -203,    141,     53 },    // 88
      { "Sapper Hill 1943",      14,   -355,     16,     74 },    // 89
      { "Schwarzeck",            21,    616,     97,   -251 },    // 90
      { "South American  69",    16,    -57,      1,    -41 },    // 91
      { "South Asia",             8,      7,    -10,    -26 },    // 92
      { "Southeast Base",        14,   -499,   -249,    314 },    // 93
      { "Southwest Base",        14,   -104,    167,    -38 },    // 94
      { "Timbalai 1948",          6,   -689,    691,    -46 },    // 95
      { "Tokyo",                  3,   -128,    481,    664 },    // 96
      { "Tristan Astro 1968",    14,   -632,    438,   -609 },    // 97
      { "Viti Levu 1916",         5,     51,    391,    -36 },    // 98
      { "Wake-Eniwetok  60",     13,    101,     52,    -39 },    // 99
      { "WGS 72",                19,      0,      0,      5 },    // 100
      { "WGS 84",                20,      0,      0,      0 },    // 101
      { "Zanderij",              14,   -265,    120,   -358 },    // 102
      { "WGS_84",                20,      0,      0,      0 },    // 103
      { "WGS-84",                20,      0,      0,      0 },    // 104
      { "EUROPEAN DATUM 1950",   14,    -87,    -98,   -121 }, 
      { "European 1950 (Norway Finland)", 14, -87, -98, -121}, 
      { "ED50",                  14,    -87,    -98,   -121 }, 
      { "RT90 (Sweden)",          3,    498,    -36,    568 }, 
      { "Monte Mario 1940",      14,   -104,    -49,     10 },
      { "Ord Surv of Gr Britain 1936", 0, 375, -111,    431 }, 
      { "South American 1969",  16,     -57,      1,    -41 },
      { "PULKOVO 1942 (2)",     15,      25,   -141,    -79 },
      { "EUROPEAN DATUM",       14,     -87,    -98,   -121 }, 
      { "BERMUDA DATUM 1957",    4,     -73,    213,    296 },
      { "COA",                  14,    -206,    172,     -6 },
      { "COABR",                14,    -206,    172,     -6 },
      { "Roma 1940",            14,    -225,    -65,      9 },
      { "ITALIENISCHES LANDESNETZ",14,  -87,    -98,   -121 },
      { "HERMANSKOGEL DATUM",     3,    682,   -203,    480 },
      { "AGD66",                  2 ,  -128,    -52,    153 },
      { "ED",                    14,    -87,    -98,   -121 },
      { "EUROPEAN 1950 (SPAIN AND PORTUGAL)",14,-87,-98,-121},
      { "ED-50",                 14,    -87,    -98,   -121 },
      { "EUROPEAN",              14,    -87,    -98,   -121 },
      { "POTSDAM",               14,    -87,    -98,   -121 },
      { "GRACIOSA SW BASE DATUM",14,   -104,    167,    -38 }, 
      { "WGS 1984",              20,      0,      0,      0 },
      { "WGS 1972",              19,      0,      0,      5 },
      { "WGS",                   19,      0,      0,      5 }

      
};

struct ELLIPSOID const gEllipsoid[] = {
//      name                               a        1/f
      {  "Airy 1830",                  6377563.396, 299.3249646   },    // 0
      {  "Modified Airy",              6377340.189, 299.3249646   },    // 1
      {  "Australian National",        6378160.0,   298.25        },    // 2
      {  "Bessel 1841",                6377397.155, 299.1528128   },    // 3
      {  "Clarke 1866",                6378206.4,   294.9786982   },    // 4
      {  "Clarke 1880",                6378249.145, 293.465       },    // 5
      {  "Everest (India 1830)",       6377276.345, 300.8017      },    // 6
      {  "Everest (1948)",             6377304.063, 300.8017      },    // 7
      {  "Modified Fischer 1960",      6378155.0,   298.3         },    // 8
      {  "Everest (Pakistan)",         6377309.613, 300.8017      },    // 9
      {  "Indonesian 1974",            6378160.0,   298.247       },    // 10
      {  "GRS 80",                     6378137.0,   298.257222101 },    // 11
      {  "Helmert 1906",               6378200.0,   298.3         },    // 12
      {  "Hough 1960",                 6378270.0,   297.0         },    // 13
      {  "International 1924",         6378388.0,   297.0         },    // 14
      {  "Krassovsky 1940",            6378245.0,   298.3         },    // 15
      {  "South American 1969",        6378160.0,   298.25        },    // 16
      {  "Everest (Malaysia 1969)",    6377295.664, 300.8017      },    // 17
      {  "Everest (Sabah Sarawak)",    6377298.556, 300.8017      },    // 18
      {  "WGS 72",                     6378135.0,   298.26        },    // 19
      {  "WGS 84",                     6378137.0,   298.257223563 },    // 20
      {  "Bessel 1841 (Namibia)",      6377483.865, 299.1528128   },    // 21
      {  "Everest (India 1956)",       6377301.243, 300.8017      },    // 22
      {  "Struve 1860",                6378298.3,   294.73        }     // 23
      
};

short nDatums = sizeof(gDatum)/sizeof(struct DATUM);



/* define constants */

void datumParams(short datum, double *a, double *es)
{
    extern struct DATUM const gDatum[];
    extern struct ELLIPSOID const gEllipsoid[];

    
    if( datum < nDatums){
        double f = 1.0 / gEllipsoid[gDatum[datum].ellipsoid].invf;    // flattening
        if(es)
            *es = 2 * f - f * f;                                          // eccentricity^2
        if(a)
            *a = gEllipsoid[gDatum[datum].ellipsoid].a;                   // semimajor axis
    }
    else{
        double f = 1.0 / 298.257223563;    // WGS84
        if(es)
            *es = 2 * f - f * f;              
        if(a)
            *a = 6378137.0;                   
    }
}

static int datumNameCmp(const char *n1, const char *n2)
{
	while(*n1 || *n2)
	{
		if (*n1 == ' ')
			n1++;
		else if (*n2 == ' ')
			n2++;
		else if (toupper(*n1) == toupper(*n2))
			n1++, n2++;
		else
			return 1;	// No string match
	}
	return 0;	// String match
}

static int isWGS84(int i)
{
    // DATUM_INDEX_WGS84 is an optimization
    // but there's more than on in gDatum table
    if (i == DATUM_INDEX_WGS84)
        return i;

    if (gDatum[i].ellipsoid != gDatum[DATUM_INDEX_WGS84].ellipsoid)
        return i;

    if (gDatum[i].dx != gDatum[DATUM_INDEX_WGS84].dx)
        return i;

    if (gDatum[i].dy != gDatum[DATUM_INDEX_WGS84].dy)
        return i;

    if (gDatum[i].dz != gDatum[DATUM_INDEX_WGS84].dz)
        return i;
        
    return DATUM_INDEX_WGS84;
}

int GetDatumIndex(const char *str)
{
      int i = 0;
      while (i < (int)nDatums)
      {
            if(!datumNameCmp(str, gDatum[i].name))
            {
                  return isWGS84(i);
            }
            i++;
      }

      return -1;
}

/****************************************************************************/
/* Convert degrees to dd mm'ss.s" (DMS-Format)                              */
/****************************************************************************/
void toDMS(double a, char *bufp, int bufplen)
{
    bool neg = a < 0.0;
    a = fabs(a);
    int n = (int) ((a - (int) a) * 36000.0);
    int m = n / 600;
    int s = n % 600;
    sprintf(bufp, "%d%02d'%02d.%01d\"", (int) (neg ? -a : a), m, s / 10, s % 10);
}


/****************************************************************************/
/* Convert dd mm'ss.s" (DMS-Format) to degrees.                             */
/****************************************************************************/

double fromDMS(char *dms)
{
    int d = 0, m = 0;
    double s = 0.0;
    char buf[20] = {'\0'};

    sscanf(dms, "%d%[ ]%d%[ ']%lf%[ \"NSWEnswe]", &d, buf, &m, buf, &s, buf);

    s = (double) (abs(d)) + ((double) m + s / 60.0) / 60.0;

    if (d >= 0 && strpbrk(buf, "SWsw") == NULL)
      return s;

    return -s;
}


/****************************************************************************/
/* Convert degrees to dd mm.mmm' (DMM-Format)                               */
/****************************************************************************/

void todmm(int flag, double a, char *bufp, int bufplen)
{
    bool bNeg = a < 0.0;
    a = fabs(a);

    int m = (int) ((a - (int) a) * 60000.0);

    if(!flag)
        snprintf(bufp, bufplen, "%d %02d.%03d'", (int) a, m / 1000, m % 1000);
    else
    {
        if(flag == 1)
        {
            snprintf(bufp, bufplen, "%02d %02d.%03d %c", (int) a, m / 1000, (m % 1000), bNeg ? 'S' : 'N');
        }
        else if(flag == 2)
        {
            snprintf(bufp, bufplen, "%03d %02d.%03d %c", (int) a, m / 1000, (m % 1000), bNeg ? 'W' : 'E');
        }
    }
}


void toDMM(double a, char *bufp, int bufplen)
{
    todmm(0, a, bufp, bufplen);
    return;
}


/* --------------------------------------------------------------------------------- */
/****************************************************************************/
/* Convert Lat/Lon <-> Simple Mercator                                      */
/****************************************************************************/
void toSM(double lat, double lon, double lat0, double lon0, double *x, double *y)
{
    double xlon = lon;

/*  Make sure lon and lon0 are same phase */

    if((lon * lon0 < 0.) && (fabs(lon - lon0) > 180.))
    {
        lon < 0.0 ? xlon += 360.0 : xlon -= 360.0;
    }

    const double z = WGS84_semimajor_axis_meters * mercator_k0;

    *x = (xlon - lon0) * DEGREE * z;

     // y =.5 ln( (1 + sin t) / (1 - sin t) )
    const double s = sin(lat * DEGREE);
    const double y3 = (.5 * log((1 + s) / (1 - s))) * z;

    const double s0 = sin(lat0 * DEGREE);
    const double y30 = (.5 * log((1 + s0) / (1 - s0))) * z;
    *y = y3 - y30;
}

double toSMcache_y30(double lat0)
{
    const double z = WGS84_semimajor_axis_meters * mercator_k0;
    const double s0 = sin(lat0 * DEGREE);
    const double y30 = (.5 * log((1 + s0) / (1 - s0))) * z;
    return y30;
}

void toSMcache(double lat, double lon, double y30, double lon0, double *x, double *y)
{
    double xlon = lon;

/*  Make sure lon and lon0 are same phase */

    if((lon * lon0 < 0.) && (fabs(lon - lon0) > 180.))
    {
        lon < 0.0 ? xlon += 360.0 : xlon -= 360.0;
    }

    const double z = WGS84_semimajor_axis_meters * mercator_k0;

    *x = (xlon - lon0) * DEGREE * z;

     // y =.5 ln( (1 + sin t) / (1 - sin t) )
    const double s = sin(lat * DEGREE);
    const double y3 = (.5 * log((1 + s) / (1 - s))) * z;

    *y = y3 - y30;
}

void
fromSM(double x, double y, double lat0, double lon0, double *lat, double *lon)
{
      const double z = WGS84_semimajor_axis_meters * mercator_k0;

// lat = arcsin((e^2(y+y0) - 1)/(e^2(y+y0) + 1))
/*
      double s0 = sin(lat0 * DEGREE);
      double y0 = (.5 * log((1 + s0) / (1 - s0))) * z;

      double e = exp(2 * (y0 + y) / z);
      double e11 = (e - 1)/(e + 1);
      double lat2 =(atan2(e11, sqrt(1 - e11 * e11))) / DEGREE;
*/
//    which is the same as....

      const double s0 = sin(lat0 * DEGREE);
      const double y0 = (.5 * log((1 + s0) / (1 - s0))) * z;

      *lat = (2.0 * atan(exp((y0+y)/z)) - PI/2.) / DEGREE;

      // lon = x + lon0
      *lon = lon0 + (x / (DEGREE * z));
}

void toSM_ECC(double lat, double lon, double lat0, double lon0, double *x, double *y)
{
      const double f = 1.0 / WGSinvf;       // WGS84 ellipsoid flattening parameter
      const double e2 = 2 * f - f * f;      // eccentricity^2  .006700
      const double e = sqrt(e2);

      const double z = WGS84_semimajor_axis_meters * mercator_k0;

      *x = (lon - lon0) * DEGREE * z;

// y =.5 ln( (1 + sin t) / (1 - sin t) )
      const double s = sin(lat * DEGREE);
      const double y3 = (.5 * log((1 + s) / (1 - s))) * z;

      const double s0 = sin(lat0 * DEGREE);
      const double y30 = (.5 * log((1 + s0) / (1 - s0))) * z;
      const double y4 = y3 - y30;

    //Add eccentricity terms

      const double falsen =  z *log(tan(PI/4 + lat0 * DEGREE / 2)*pow((1. - e * s0)/(1. + e * s0), e/2.));
      const double test =    z *log(tan(PI/4 + lat  * DEGREE / 2)*pow((1. - e * s )/(1. + e * s ), e/2.));
      *y = test - falsen;
}

void fromSM_ECC(double x, double y, double lat0, double lon0, double *lat, double *lon)
{
      const double f = 1.0 / WGSinvf;       // WGS84 ellipsoid flattening parameter
      const double es = 2 * f - f * f;      // eccentricity^2  .006700
      const double e = sqrt(es);

      const double z = WGS84_semimajor_axis_meters * mercator_k0;

      *lon = lon0 + (x / (DEGREE * z));

      const double s0 = sin(lat0 * DEGREE);

      const double falsen = z *log(tan(PI/4 + lat0 * DEGREE / 2)*pow((1. - e * s0)/(1. + e * s0), e/2.));
      const double t = exp((y + falsen) / (z));
      const double xi = (PI / 2.) - 2.0 * atan(t);

      //    Add eccentricity terms

      double esf = (es/2. + (5*es*es/24.) + (es*es*es/12.) + (13.0 *es*es*es*es/360.)) * sin( 2 * xi);
      esf += ((7.*es*es/48.) + (29.*es*es*es/240.) + (811.*es*es*es*es/11520.)) * sin (4. * xi);
      esf += ((7.*es*es*es/120.) + (81*es*es*es*es/1120.) + (4279.*es*es*es*es/161280.)) * sin(8. * xi);


     *lat = -(xi + esf) / DEGREE;

}

#define TOL 1e-10
#define CONV      1e-10
#define N_ITER    10
#define I_ITER 20
#define ITOL 1.e-12

void
toPOLY(double lat, double lon, double lat0, double lon0, double *x, double *y)
{
      const double z = WGS84_semimajor_axis_meters * mercator_k0;

      if (fabs((lat - lat0) * DEGREE) <= TOL)
      {
            *x = (lon - lon0) * DEGREE * z;
            *y = 0.;

      }
      else
      {
          const double E = (lon - lon0) * DEGREE * sin(lat * DEGREE);
          const double cot = 1. / tan(lat * DEGREE);
          *x = sin(E) * cot;
          *y = (lat * DEGREE) - (lat0 * DEGREE) + cot * (1. - cos(E));

          *x *= z;
          *y *= z;
      }


/*
      if (fabs(lp.phi) <= TOL) { xy.x = lp.lam; xy.y = P->ml0; }
      else {
            cot = 1. / tan(lp.phi);
            xy.x = sin(E = lp.lam * sin(lp.phi)) * cot;
            xy.y = lp.phi - P->phi0 + cot * (1. - cos(E));
      }
*/
}



void
fromPOLY(double x, double y, double lat0, double lon0, double *lat, double *lon)
{
      const double z = WGS84_semimajor_axis_meters * mercator_k0;

      double yp = y - (lat0 * DEGREE * z);
      if(fabs(yp) <= TOL)
      {
            *lon = lon0 + (x / (DEGREE * z));
            *lat = lat0;
      }
      else
      {
            yp = y / z;
            const double xp = x / z;

            double lat3 = yp;
            const double B = (xp * xp) + (yp * yp);
            int i = N_ITER;
            double dphi;
            do {
                  double tp = tan(lat3);
                  dphi = (yp * (lat3 * tp + 1.) - lat3 - .5 * ( lat3 * lat3 + B) * tp) / ((lat3 - yp) / tp - 1.);
                  lat3 -= dphi;
            } while (fabs(dphi) > CONV && --i);
            if (! i)
            {
                  *lon = 0.;
                  *lat = 0.;
            }
            else
            {
                  *lon = asin(xp * tan(lat3)) / sin(lat3);
                  *lon /= DEGREE;
                  *lon += lon0;

                  *lat = lat3 / DEGREE;
            }
      }
}

/****************************************************************************/
/* Convert Lat/Lon <-> Transverse Mercator                                                                  */
/****************************************************************************/



//converts lat/long to TM coords.  Equations from USGS Bulletin 1532
//East Longitudes are positive, West longitudes are negative.
//North latitudes are positive, South latitudes are negative
//Lat and Long are in decimal degrees.
      //Written by Chuck Gantz- chuck.gantz@globalstar.com
      //Adapted for opencpn by David S. Register

void  toTM(float lat, float lon, float lat0, float lon0, double *x, double *y)
{

// constants for WGS-84
      const double f = 1.0 / WGSinvf;       /* WGS84 ellipsoid flattening parameter */
      const double a = WGS84_semimajor_axis_meters;
      const double k0 = 1.;                /*  Scaling factor    */

      const double eccSquared = 2 * f - f * f;
      const double eccPrimeSquared = (eccSquared)/(1-eccSquared);
      const double LatRad = lat*DEGREE;
      const double LongOriginRad = lon0 * DEGREE;
      const double LongRad = lon*DEGREE;

      const double N = a/sqrt(1-eccSquared*sin(LatRad)*sin(LatRad));
      const double T = tan(LatRad)*tan(LatRad);
      const double C = eccPrimeSquared*cos(LatRad)*cos(LatRad);
      const double A = cos(LatRad)*(LongRad-LongOriginRad);

      const double MM = a*((1   - eccSquared/4          - 3*eccSquared*eccSquared/64  - 5*eccSquared*eccSquared*eccSquared/256)*LatRad
                  - (3*eccSquared/8 + 3*eccSquared*eccSquared/32  + 45*eccSquared*eccSquared*eccSquared/1024)*sin(2*LatRad)
                  + (15*eccSquared*eccSquared/256 + 45*eccSquared*eccSquared*eccSquared/1024)*sin(4*LatRad)
                  - (35*eccSquared*eccSquared*eccSquared/3072)*sin(6*LatRad));

      *x = (k0*N*(A+(1-T+C)*A*A*A/6 + (5-18*T+T*T+72*C-58*eccPrimeSquared)*A*A*A*A*A/120));

      *y = (k0*(MM+N*tan(LatRad)*(A*A/2+(5-T+9*C+4*C*C)*A*A*A*A/24
                  + (61-58*T+T*T+600*C-330*eccPrimeSquared)*A*A*A*A*A*A/720)));

}

/* --------------------------------------------------------------------------------- */

//converts TM coords to lat/long.  Equations from USGS Bulletin 1532
//East Longitudes are positive, West longitudes are negative.
//North latitudes are positive, South latitudes are negative
//Lat and Long are in decimal degrees
      //Written by Chuck Gantz- chuck.gantz@globalstar.com
      //Adapted for opencpn by David S. Register

void fromTM (double x, double y, double lat0, double lon0, double *lat, double *lon)
{
      const double rad2deg = 1./DEGREE;
// constants for WGS-84

      const double f = 1.0 / WGSinvf;       /* WGS84 ellipsoid flattening parameter */
      const double a = WGS84_semimajor_axis_meters;
      const double k0 = 1.;                /*  Scaling factor    */

      const double eccSquared = 2 * f - f * f;

      const double eccPrimeSquared = (eccSquared)/(1-eccSquared);
      const double e1 = (1.0 - sqrt(1.0 - eccSquared)) / (1.0 + sqrt(1.0 - eccSquared));

      const double MM = y / k0;
      const double mu = MM/(a*(1-eccSquared/4-3*eccSquared*eccSquared/64-5*eccSquared*eccSquared*eccSquared/256));

      const double phi1Rad = mu      + (3*e1/2-27*e1*e1*e1/32)*sin(2*mu)
                  + (21*e1*e1/16-55*e1*e1*e1*e1/32)*sin(4*mu)
                  +(151*e1*e1*e1/96)*sin(6*mu);

      const double N1 = a/sqrt(1-eccSquared*sin(phi1Rad)*sin(phi1Rad));
      const double T1 = tan(phi1Rad)*tan(phi1Rad);
      const double C1 = eccPrimeSquared*cos(phi1Rad)*cos(phi1Rad);
      const double R1 = a*(1-eccSquared)/pow(1-eccSquared*sin(phi1Rad)*sin(phi1Rad), 1.5);
      const double D = x/(N1*k0);

      *lat = phi1Rad - (N1*tan(phi1Rad)/R1)*(D*D/2-(5+3*T1+10*C1-4*C1*C1-9*eccPrimeSquared)*D*D*D*D/24
                  +(61+90*T1+298*C1+45*T1*T1-252*eccPrimeSquared-3*C1*C1)*D*D*D*D*D*D/720);
      *lat = lat0 + (*lat * rad2deg);

      *lon = (D-(1+2*T1+C1)*D*D*D/6+(5-2*C1+28*T1-3*C1*C1+8*eccPrimeSquared+24*T1*T1)
                  *D*D*D*D*D/120)/cos(phi1Rad);
      *lon = lon0 + *lon * rad2deg;

}


/* orthographic, polar, stereographic, gnomonic and equirectangular projection routines, contributed by Sean D'Epagnier */
/****************************************************************************/
/* Convert Lat/Lon <-> Simple Polar                                         */
/****************************************************************************/
void cache_phi0(double lat0, double *sin_phi0, double *cos_phi0)
{
    double phi0 = lat0*DEGREE;
    *sin_phi0 = sin(phi0);
    *cos_phi0 = cos(phi0);
}

void toORTHO(double lat, double lon, double sin_phi0, double cos_phi0, double lon0, double *x, double *y)
{
    const double z = WGS84_semimajor_axis_meters * mercator_k0;

    double xlon = lon;
    /*  Make sure lon and lon0 are same phase */
    if((lon * lon0 < 0.) && (fabs(lon - lon0) > 180.))
        lon < 0.0 ? xlon += 360.0 : xlon -= 360.0;

    double theta = (xlon - lon0) * DEGREE;
    double phi = lat * DEGREE;
    double cos_phi = cos(phi);

    double vy = sin(phi), vz = cos(theta)*cos_phi;

    if(vy*sin_phi0 + vz*cos_phi0 < 0) { // on the far side of the earth
        *x = *y = NAN;
        return;
    }

    double vx = sin(theta)*cos_phi;
    double vw = vy*cos_phi0 - vz*sin_phi0;

    *x = vx*z;
    *y = vw*z;
}

void fromORTHO(double x, double y, double lat0, double lon0, double *lat, double *lon)
{
    const double z = WGS84_semimajor_axis_meters * mercator_k0;

    double vx = x / z;
    double vw = y / z;

    double phi0 = lat0 * DEGREE;
    double d = 1 - vx*vx - vw*vw;

    if(d < 0) { // position is outside of the earth
        *lat = *lon = NAN;
        return;
    }

    double sin_phi0 = sin(phi0), cos_phi0 = cos(phi0);
    double vy = vw*cos_phi0 + sqrt(d)*sin_phi0;
    double phi = asin(vy);

    double vz = (vy*cos_phi0 - vw) / sin_phi0;
    double theta = atan2(vx, vz);

    *lat = phi / DEGREE;
    *lon = theta / DEGREE + lon0;
}

double toPOLARcache_e(double lat0)
{
   double pole = lat0 > 0 ? 90 : -90;
   return tan((pole - lat0) * DEGREE / 2);
}

void toPOLAR(double lat, double lon, double e, double lat0, double lon0, double *x, double *y)
{
    const double z = WGS84_semimajor_axis_meters * mercator_k0;

    double xlon = lon;
    /*  Make sure lon and lon0 are same phase */
    if((lon * lon0 < 0.) && (fabs(lon - lon0) > 180.))
        lon < 0.0 ? xlon += 360.0 : xlon -= 360.0;

    double theta = (xlon - lon0) * DEGREE;
    double pole = lat0 > 0 ? 90 : -90;

    double d = tan((pole - lat) * DEGREE / 2);

    *x = fabs(d)*sin(theta)*z;
    *y = (e-d*cos(theta))*z;
}


void fromPOLAR(double x, double y, double lat0, double lon0, double *lat, double *lon)
{
    const double z = WGS84_semimajor_axis_meters * mercator_k0;
    double pole = lat0 > 0 ? 90 : -90;

    double e = tan((pole - lat0) * DEGREE / 2);

    double xn = x/z;
    double yn = e - y/z;
    double d = sqrt(xn*xn + yn*yn);
    if(pole < 0) // south polar (negative root and correct branch from cosine)
        d = -d, yn = -yn;

    *lat = pole - atan(d) * 2 / DEGREE;

    double theta = atan2(xn, yn);
    *lon = theta / DEGREE + lon0;
}

static inline void toSTEREO1(double &u, double &v, double &w, double lat, double lon,
                             double sin_phi0, double cos_phi0, double lon0)
{
    double xlon = lon;
    /*  Make sure lon and lon0 are same phase */
    if((lon * lon0 < 0.) && (fabs(lon - lon0) > 180.))
        lon < 0.0 ? xlon += 360.0 : xlon -= 360.0;

    double theta = (xlon - lon0) * DEGREE, phi = lat*DEGREE;
    double cos_phi = cos(phi), v0 = sin(phi), w0 = cos(theta)*cos_phi;

    u = sin(theta)*cos_phi;
    v = cos_phi0*v0 - sin_phi0*w0;
    w = sin_phi0*v0 + cos_phi0*w0;
}

static inline void fromSTEREO1(double *lat, double *lon, double lat0, double lon0,
                               double u, double v, double w)
{
    double phi0 = lat0*DEGREE;
    double v0 = sin(phi0)*w + cos(phi0)*v;
    double w0 = cos(phi0)*w - sin(phi0)*v;
    double phi = asin(v0);
    double theta = atan2(u, w0);

    *lat = phi / DEGREE;
    *lon = theta / DEGREE + lon0;
}

void toSTEREO(double lat, double lon, double sin_phi0, double cos_phi0, double lon0, double *x, double *y)
{
    const double z = WGS84_semimajor_axis_meters * mercator_k0;

    double u, v, w;
    toSTEREO1(u, v, w, lat, lon, sin_phi0, cos_phi0, lon0);

    double t = 2/(w+1);
    *x = u*t*z;
    *y = v*t*z;
}

void fromSTEREO(double x, double y, double lat0, double lon0, double *lat, double *lon)
{
    const double z = WGS84_semimajor_axis_meters * mercator_k0;

    x /= z, y /= z;

    double t = (x*x + y*y) / 4 + 1;

    double u = x/t;
    double v = y/t;
    double w = 2/t - 1;

    fromSTEREO1(lat, lon, lat0, lon0, u, v, w);
}

void toGNO(double lat, double lon, double sin_phi0, double cos_phi0, double lon0, double *x, double *y)
{
    const double z = WGS84_semimajor_axis_meters * mercator_k0;

    double u, v, w;
    toSTEREO1(u, v, w, lat, lon, sin_phi0, cos_phi0, lon0);

    if(w <= 0) {
        *x = *y = NAN; // far side of world
        return;
    }

    *x = u/w*z;
    *y = v/w*z;
}

void fromGNO(double x, double y, double lat0, double lon0, double *lat, double *lon)
{
    const double z = WGS84_semimajor_axis_meters * mercator_k0;

    x /= z, y /= z;

    double w = 1 / sqrt(x*x + y*y + 1);
    double u = x*w;
    double v = y*w;

    fromSTEREO1(lat, lon, lat0, lon0, u, v, w);
}

void toEQUIRECT(double lat, double lon, double lat0, double lon0, double *x, double *y)
{
    const double z = WGS84_semimajor_axis_meters * mercator_k0;
    double xlon = lon;
    /*  Make sure lon and lon0 are same phase */
    if((lon * lon0 < 0.) && (fabs(lon - lon0) > 180.))
        lon < 0.0 ? xlon += 360.0 : xlon -= 360.0;

    *x = (xlon - lon0) * DEGREE * z;
    *y = (lat - lat0) * DEGREE * z;
}

void fromEQUIRECT(double x, double y, double lat0, double lon0, double *lat, double *lon)
{
    const double z = WGS84_semimajor_axis_meters * mercator_k0;

    *lat = lat0 + (y / (DEGREE * z));
//    if(fabs(*lat) > 90) *lat = NAN;
    *lon = lon0 + (x / (DEGREE * z));
}


/* --------------------------------------------------------------------------------- *

 *Molodensky
 *In the listing below, the class GeodeticPosition has three members, lon, lat, and h.
 *They are double-precision values indicating the longitude and latitude in radians,
 * and height in meters above the ellipsoid.

 * The source code in the listing below may be copied and reused without restriction,
 * but it is offered AS-IS with NO WARRANTY.

 * Adapted for opencpn by David S. Register

 * --------------------------------------------------------------------------------- */

void MolodenskyTransform (double lat, double lon, double *to_lat, double *to_lon, int from_datum_index, int to_datum_index)
{
    double dlat = 0;
    double dlon = 0;
    
    if( from_datum_index < nDatums){
      const double from_lat = lat * DEGREE;
      const double from_lon = lon * DEGREE;
      const double from_f = 1.0 / gEllipsoid[gDatum[from_datum_index].ellipsoid].invf;    // flattening
      const double from_esq = 2 * from_f - from_f * from_f;                               // eccentricity^2
      const double from_a = gEllipsoid[gDatum[from_datum_index].ellipsoid].a;             // semimajor axis
      const double dx = gDatum[from_datum_index].dx;
      const double dy = gDatum[from_datum_index].dy;
      const double dz = gDatum[from_datum_index].dz;
      const double to_f = 1.0 / gEllipsoid[gDatum[to_datum_index].ellipsoid].invf;        // flattening
      const double to_a = gEllipsoid[gDatum[to_datum_index].ellipsoid].a;                 // semimajor axis
      const double da = to_a - from_a;
      const double df = to_f - from_f;
      const double from_h = 0;


      const double slat = sin (from_lat);
      const double clat = cos (from_lat);
      const double slon = sin (from_lon);
      const double clon = cos (from_lon);
      const double ssqlat = slat * slat;
      const double adb = 1.0 / (1.0 - from_f);  // "a divided by b"

      const double rn = from_a / sqrt (1.0 - from_esq * ssqlat);
      const double rm = from_a * (1. - from_esq) / pow ((1.0 - from_esq * ssqlat), 1.5);

      dlat = (((((-dx * slat * clon - dy * slat * slon) + dz * clat)
                  + (da * ((rn * from_esq * slat * clat) / from_a)))
                  + (df * (rm * adb + rn / adb) * slat * clat)))
            / (rm + from_h);

      dlon = (-dx * slon + dy * clon) / ((rn + from_h) * clat);

      const double dh = (dx * clat * clon) + (dy * clat * slon) + (dz * slat)
                  - (da * (from_a / rn)) + ((df * rn * ssqlat) / adb);

    } 
    
    *to_lon = lon + dlon/DEGREE;
    *to_lat = lat + dlat/DEGREE;
//
      return;
}


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

double adjlon (double lon) {
      if (fabs(lon) <= SPI) return( lon );
      lon += ONEPI;  /* adjust to 0..2pi rad */
      lon -= TWOPI * floor(lon / TWOPI); /* remove integral # of 'revolutions'*/
      lon -= ONEPI;  /* adjust back to -pi..pi rad */
      return( lon );
}



/* --------------------------------------------------------------------------------- */
/*
// Given the lat/long of starting point, and traveling a specified distance,
// at an initial bearing, calculates the lat/long of the resulting location.
// using sphere earth model.
*/
/* --------------------------------------------------------------------------------- */
void ll_gc_ll(double lat, double lon, double brg, double dist, double *dlat, double *dlon)
{
    
    double th1,costh1,sinth1,sina12,cosa12,M,N,c1,c2,D,P,s1;
    int merid, signS;
    
    /*   Input/Output from geodesic functions   */
    double al12;           /* Forward azimuth */
    double al21;           /* Back azimuth    */
    double geod_S;         /* Distance        */
    double phi1, lam1, phi2, lam2;
    
    int ellipse;
    double geod_f;
    double geod_a;
    double es, onef, f, f64, f2, f4;
    
    /*      Setup the static parameters  */
    phi1 = lat * DEGREE;            /* Initial Position  */
    lam1 = lon * DEGREE;
    al12 = brg * DEGREE;            /* Forward azimuth */
    geod_S = dist * 1852.0;         /* Distance        */
    
    
    //void geod_pre(struct georef_state *state) 
    {
        
        /*   Stuff the WGS84 projection parameters as necessary
         *      To avoid having to include <geodesic,h>
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
        
        al12 = adjlon(al12); /* reduce to  +- 0-PI */
        signS = fabs(al12) > HALFPI ? 1 : 0;
        th1 = ellipse ? atan(onef * tan(phi1)) : phi1;
        costh1 = cos(th1);
        sinth1 = sin(th1);
        if ((merid = fabs(sina12 = sin(al12)) < MERI_TOL)) {
            sina12 = 0.;
            cosa12 = fabs(al12) < HALFPI ? 1. : -1.;
            M = 0.;
        } else {
            cosa12 = cos(al12);
            M = costh1 * sina12;
        }
        N = costh1 * cosa12;
        if (ellipse) {
            if (merid) {
                c1 = 0.;
                c2 = f4;
                D = 1. - c2;
                D *= D;
                P = c2 / D;
            } else {
                c1 = geod_f * M;
                c2 = f4 * (1. - M * M);
                D = (1. - c2)*(1. - c2 - c1 * M);
                P = (1. + .5 * c1 * M) * c2 / D;
            }
        }
        if (merid) s1 = HALFPI - th1;
        else {
            s1 = (fabs(M) >= 1.) ? 0. : acos(M);
            s1 =  sinth1 / sin(s1);
            s1 = (fabs(s1) >= 1.) ? 0. : acos(s1);
        }
    }
    
    
    //void  geod_for(struct georef_state *state)
    {
        double d,sind,u,V,X,ds,cosds,sinds,ss,de;
        
        ss = 0.;
        
        if (ellipse) {
            d = geod_S / (D * geod_a);
            if (signS) d = -d;
            u = 2. * (s1 - d);
            V = cos(u + d);
            X = c2 * c2 * (sind = sin(d)) * cos(d) * (2. * V * V - 1.);
            ds = d + X - 2. * P * V * (1. - 2. * P * cos(u)) * sind;
            ss = s1 + s1 - ds;
        } else {
            ds = geod_S / geod_a;
            if (signS) ds = - ds;
        }
        cosds = cos(ds);
        sinds = sin(ds);
        if (signS) sinds = - sinds;
        al21 = N * cosds - sinth1 * sinds;
        if (merid) {
            phi2 = atan( tan(HALFPI + s1 - ds) / onef);
            if (al21 > 0.) {
                al21 = PI;
                if (signS)
                    de = PI;
                else {
                    phi2 = - phi2;
                    de = 0.;
                }
            } else {
                al21 = 0.;
                if (signS) {
                    phi2 = - phi2;
                    de = 0;
                } else
                    de = PI;
            }
        } else {
            al21 = atan(M / al21);
            if (al21 > 0)
                al21 += PI;
            if (al12 < 0.)
                al21 -= PI;
            al21 = adjlon(al21);
            phi2 = atan(-(sinth1 * cosds + N * sinds) * sin(al21) /
            (ellipse ? onef * M : M));
            de = atan2(sinds * sina12 ,
                       (costh1 * cosds - sinth1 * sinds * cosa12));
            if (ellipse){
                if (signS)
                    de += c1 * ((1. - c2) * ds +
                    c2 * sinds * cos(ss));
                else
                    de -= c1 * ((1. - c2) * ds -
                    c2 * sinds * cos(ss));
            }
        }
        lam2 = adjlon( lam1 + de );
    }
    
    
    *dlat = phi2 / DEGREE;
    *dlon = lam2 / DEGREE;
}

void ll_gc_ll_reverse(double lat1, double lon1, double lat2, double lon2,
                      double *bearing, double *dist)
{
//    double th1,costh1,sinth1,sina12,cosa12,M,N,c1,c2,D,P,s1;
//    int merid, signS;
    
    /*   Input/Output from geodesic functions   */
    double al12;           /* Forward azimuth */
    double al21;           /* Back azimuth    */
    double geod_S;         /* Distance        */
    double phi1, lam1, phi2, lam2;
    
    int ellipse;
    double geod_f;
    double geod_a;
    double es, onef, f, f64, f2, f4;
    
    /*      Setup the static parameters  */
    phi1 = lat1 * DEGREE;            /* Initial Position  */
    lam1 = lon1 * DEGREE;
    phi2 = lat2 * DEGREE;
    lam2 = lon2 * DEGREE;
    
    //void geod_inv(struct georef_state *state)
    {
        double      th1,th2,thm,dthm,dlamm,dlam,sindlamm,costhm,sinthm,cosdthm,
        sindthm,L,E,cosd,d,X,Y,T,sind,tandlammp,u,v,D,A,B;
        
        
        /*   Stuff the WGS84 projection parameters as necessary
         *      To avoid having to include <geodesic,h>
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
            if(bearing)
                *bearing = 0.;
            if(dist)
                *dist = 0.;
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
    
    if(al12 < 0)
        al12 += 2*PI;
    
    if(bearing)
        *bearing = al12 / DEGREE;
    if(dist)
        *dist = geod_S / 1852.0;
}

void PositionBearingDistanceMercator(double lat, double lon, double brg, double dist,
                                     double *dlat, double *dlon)
{
    ll_gc_ll(lat, lon, brg, dist, dlat, dlon);
}

/* --------------------------------------------------------------------------------- */
/*
// Given the lat/long of starting point and ending point,
// calculates the distance along a geodesic curve, using sphere earth model.
*/
/* --------------------------------------------------------------------------------- */

double DistGreatCircle(double slat, double slon, double dlat, double dlon)
{
//    double th1,costh1,sinth1,sina12,cosa12,M,N,c1,c2,D,P,s1;
//    int merid, signS;
    
    /*   Input/Output from geodesic functions   */
    double al12;           /* Forward azimuth */
    double al21;           /* Back azimuth    */
    double geod_S;         /* Distance        */
    double phi1, lam1, phi2, lam2;
    
    int ellipse;
    double geod_f;
    double geod_a;
    double es, onef, f, f64, f2, f4;
    
    double d5;
    phi1 = slat * DEGREE;
    lam1 = slon * DEGREE;
    phi2 = dlat * DEGREE;
    lam2 = dlon * DEGREE;
    
    //void geod_inv(struct georef_state *state)
    {
        double      th1,th2,thm,dthm,dlamm,dlam,sindlamm,costhm,sinthm,cosdthm,
        sindthm,L,E,cosd,d,X,Y,T,sind,tandlammp,u,v,D,A,B;
        
        
        /*   Stuff the WGS84 projection parameters as necessary
         *      To avoid having to include <geodesic,h>
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
            return 0.0;
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
    
    d5 = geod_S / 1852.0;
    
    return d5;
}


void DistanceBearingMercator(double lat0, double lon0, double lat1, double lon1, double *brg, double *dist)
{
      //    Calculate bearing by conversion to SM (Mercator) coordinates, then simple trigonometry

      double lon0x = lon0;
      double lon1x = lon1;

      //    Make lon points the same phase
      if((lon0x * lon1x) < 0.)
      {
          lon0x < 0.0 ? lon0x += 360.0 : lon1x += 360.0;
            //    Choose the shortest distance
            if(fabs(lon0x - lon1x) > 180.)
            {
                lon0x > lon1x ? lon0x -= 360.0 : lon1x -= 360.0;
            }

            //    Make always positive
            lon1x += 360.;
            lon0x += 360.;
      }

      //    Classic formula, which fails for due east/west courses....
      if(dist)
      {
          //    In the case of exactly east or west courses
          //    we must make an adjustment if we want true Mercator distances

          //    This idea comes from Thomas(Cagney)
          //    We simply require the dlat to be (slightly) non-zero, and carry on.
          //    MAS022210 for HamishB from 1e-4 && .001 to 1e-9 for better precision
          //    on small latitude diffs
          const double mlat0 = fabs(lat1 - lat0) < 1e-9 ? lat0 + 1e-9 : lat0;

          double east, north;
          toSM_ECC(lat1, lon1x, mlat0, lon0x, &east, &north);
          const double C = atan2(east, north);
          if(cos(C))
          {
              const double dlat = (lat1 - mlat0) * 60.;              // in minutes
              *dist = (dlat /cos(C));
          }
          else
          {
              *dist = DistGreatCircle(lat0, lon0, lat1, lon1);
          }
      }

      //    Calculate the bearing using the un-adjusted original latitudes and Mercator Sailing
      if(brg)
      {
          double east, north;
          toSM_ECC(lat1, lon1x, lat0, lon0x, &east, &north);

          const double C = atan2(east, north);
          const double brgt = 180. + (C * 180. / PI);
          if (brgt < 0)
              *brg = brgt + 360.;
          else if (brgt >= 360.)
              *brg = brgt - 360.;
          else
              *brg = brgt;
      }
}


/* --------------------------------------------------------------------------------- */
/*
 * lmfit
 *
 * Solves or minimizes the sum of squares of m nonlinear
 * functions of n variables.
 *
 * From public domain Fortran version
 * of Argonne National Laboratories MINPACK
 *     argonne national laboratory. minpack project. march 1980.
 *     burton s. garbow, kenneth e. hillstrom, jorge j. more
 * C translation by Steve Moshier
 * Joachim Wuttke converted the source into C++ compatible ANSI style
 * and provided a simplified interface
 */


//#include "lmmin.h"            // all moved to georef.h
#define _LMDIF

///=================================================================================
///     Customized section for openCPN georeferencing

double my_fit_function( double tx, double ty, int n_par, double* p )
{

    double ret = p[0] + p[1]*tx;

    if(n_par > 2)
          ret += p[2]*ty;
    if(n_par > 3)
    {
        ret += p[3]*tx*tx;
        ret += p[4]*tx*ty;
        ret += p[5]*ty*ty;
    }
    if(n_par > 6)
    {
        ret += p[6]*tx*tx*tx;
        ret += p[7]*tx*tx*ty;
        ret += p[8]*tx*ty*ty;
        ret += p[9]*ty*ty*ty;
    }

    return ret;
}

int Georef_Calculate_Coefficients_Onedir(int n_points, int n_par, double *tx, double *ty, double *y, double *p,
                                        double hintp0, double hintp1, double hintp2)
        /*
        n_points : number of sample points
        n_par :  3, 6, or 10,  6 is probably good
        tx:  sample data independent variable 1
        ty:  sample data independent variable 2
        y:   sample data dependent result
        p:   curve fit result coefficients
        */
{

    int i;
    lm_control_type control;
    lm_data_type data;

    lm_initialize_control( &control );


    for(i=0 ; i<12 ; i++)
        p[i] = 0.;

//    memset(p, 0, 12 * sizeof(double));

    //  Insert hints
    p[0] = hintp0;
    p[1] = hintp1;
    p[2] = hintp2;

    data.user_func = my_fit_function;
    data.user_tx = tx;
    data.user_ty = ty;
    data.user_y = y;
    data.n_par = n_par;
    data.print_flag = 0;

// perform the fit:

            lm_minimize( n_points, n_par, p, lm_evaluate_default, lm_print_default,
                         &data, &control );

// print results:
//            printf( "status: %s after %d evaluations\n",
//                    lm_infmsg[control.info], control.nfev );

            //      Control.info results [1,2,3] are success, other failure
            return control.info;
}

int Georef_Calculate_Coefficients(struct GeoRef *cp, int nlin_lon)
{
    //      Zero out the points
    for(int i = 0; i < 10; ++i)
        cp->pwx[i] = cp->pwy[i] = cp->wpx[i] = cp->wpy[i] = 0.0;

    int mp = 3;

    switch (cp->order)
    {
    case 1:
        mp = 3;
        break;
    case 2:
        mp = 6;
        break;
    case 3:
        mp = 10;
        break;
    default:
        mp = 3;
        break;
    }

    const int mp_lat = mp;

    //      Force linear fit for longitude if nlin_lon > 0
    const int mp_lon = nlin_lon ? 2 : mp;

    //      Make a dummy double array
    std::vector<double> pnull(cp->count, 1.0);

    //      pixel(tx,ty) to (lat,lon)
    //      Calculate and use a linear equation for p[0..2] to hint the solver

    int r1 = Georef_Calculate_Coefficients_Onedir(cp->count, mp_lon, cp->tx, cp->ty, cp->lon, cp->pwx,
                                         cp->lonmin - (cp->txmin * (cp->lonmax - cp->lonmin) /(cp->txmax - cp->txmin)),
                                         (cp->lonmax - cp->lonmin) /(cp->txmax - cp->txmin),
                                         0.);

    //      if blin_lon > 0, force cross terms in latitude equation coefficients to be zero by making lat not dependent on tx,
    double *px = nlin_lon ? &pnull[0] : cp->tx;

    int r2 = Georef_Calculate_Coefficients_Onedir(cp->count, mp_lat, px, cp->ty, cp->lat, cp->pwy,
                                         cp->latmin - (cp->tymin * (cp->latmax - cp->latmin) /(cp->tymax - cp->tymin)),
                                         0.,
                                         (cp->latmax - cp->latmin) /(cp->tymax - cp->tymin));

    //      (lat,lon) to pixel(tx,ty)
    //      Calculate and use a linear equation for p[0..2] to hint the solver

    int r3 = Georef_Calculate_Coefficients_Onedir(cp->count, mp_lon, cp->lon, cp->lat, cp->tx, cp->wpx,
                                         cp->txmin - ((cp->txmax - cp->txmin) * cp->lonmin / (cp->lonmax - cp->lonmin)),
                                         (cp->txmax - cp->txmin) / (cp->lonmax - cp->lonmin),
                                         0.0);

    //      if blin_lon > 0, force cross terms in latitude equation coefficients to be zero by making ty not dependent on lon,
    px = nlin_lon ? &pnull[0] : cp->lon;

    int r4 = Georef_Calculate_Coefficients_Onedir(cp->count, mp_lat, &pnull[0]/*cp->lon*/, cp->lat, cp->ty, cp->wpy,
                                         cp->tymin - ((cp->tymax - cp->tymin) * cp->latmin / (cp->latmax - cp->latmin)),
                                        0.0,
                                        (cp->tymax - cp->tymin) / (cp->latmax - cp->latmin));

    if((r1) && (r1 < 4) && (r2) && (r2 < 4) && (r3) && (r3 < 4) && (r4) && (r4 < 4))
        return 0;

    return 1;
}



int Georef_Calculate_Coefficients_Proj(struct GeoRef *cp)
{
      int  r1, r2, r3, r4;
      int mp;

    //      Zero out the points
      cp->pwx[6] = cp->pwy[6] = cp->wpx[6] = cp->wpy[6] = 0.;
      cp->pwx[7] = cp->pwy[7] = cp->wpx[7] = cp->wpy[7] = 0.;
      cp->pwx[8] = cp->pwy[8] = cp->wpx[8] = cp->wpy[8] = 0.;
      cp->pwx[9] = cp->pwy[9] = cp->wpx[9] = cp->wpy[9] = 0.;
      cp->pwx[3] = cp->pwy[3] = cp->wpx[3] = cp->wpy[3] = 0.;
      cp->pwx[4] = cp->pwy[4] = cp->wpx[4] = cp->wpy[4] = 0.;
      cp->pwx[5] = cp->pwy[5] = cp->wpx[5] = cp->wpy[5] = 0.;
      cp->pwx[0] = cp->pwy[0] = cp->wpx[0] = cp->wpy[0] = 0.;
      cp->pwx[1] = cp->pwy[1] = cp->wpx[1] = cp->wpy[1] = 0.;
      cp->pwx[2] = cp->pwy[2] = cp->wpx[2] = cp->wpy[2] = 0.;

      mp = 3;


    //      pixel(tx,ty) to (northing,easting)
    //      Calculate and use a linear equation for p[0..2] to hint the solver

      r1 = Georef_Calculate_Coefficients_Onedir(cp->count, mp, cp->tx, cp->ty, cp->lon, cp->pwx,
                  cp->lonmin - (cp->txmin * (cp->lonmax - cp->lonmin) /(cp->txmax - cp->txmin)),
                                (cp->lonmax - cp->lonmin) /(cp->txmax - cp->txmin),
                                 0.);



      r2 = Georef_Calculate_Coefficients_Onedir(cp->count, mp, cp->tx, cp->ty, cp->lat, cp->pwy,
                  cp->latmin - (cp->tymin * (cp->latmax - cp->latmin) /(cp->tymax - cp->tymin)),
                                0.,
                                (cp->latmax - cp->latmin) /(cp->tymax - cp->tymin));

    //      (northing/easting) to pixel(tx,ty)
    //      Calculate and use a linear equation for p[0..2] to hint the solver

      r3 = Georef_Calculate_Coefficients_Onedir(cp->count, mp, cp->lon, cp->lat, cp->tx, cp->wpx,
                  cp->txmin - ((cp->txmax - cp->txmin) * cp->lonmin / (cp->lonmax - cp->lonmin)),
                                (cp->txmax - cp->txmin) / (cp->lonmax - cp->lonmin),
                                 0.0);

      r4 = Georef_Calculate_Coefficients_Onedir(cp->count, mp, cp->lon, cp->lat, cp->ty, cp->wpy,
                  cp->tymin - ((cp->tymax - cp->tymin) * cp->latmin / (cp->latmax - cp->latmin)),
                                0.0,
                                (cp->tymax - cp->tymin) / (cp->latmax - cp->latmin));


      if((r1) && (r1 < 4) && (r2) && (r2 < 4) && (r3) && (r3 < 4) && (r4) && (r4 < 4))
            return 0;
      else
            return 1;

}


/*
 * This file contains default implementation of the evaluate and printout
 * routines. In most cases, customization of lmfit can be done by modifying
 * these two routines. Either modify them here, or copy and rename them.
 */

void lm_evaluate_default( double* par, int m_dat, double* fvec,
                          void *data, int *info )
/*
        *    par is an input array. At the end of the minimization, it contains
        *        the approximate solution vector.
        *
        *    m_dat is a positive integer input variable set to the number
        *      of functions.
        *
        *    fvec is an output array of length m_dat which contains the function
        *        values the square sum of which ought to be minimized.
        *
        *    data is a read-only pointer to lm_data_type, as specified by lmuse.h.
        *
        *      info is an integer output variable. If set to a negative value, the
        *        minimization procedure will stop.
 */
{
    lm_data_type *mydata = (lm_data_type*)data;

    for (int i=0; i<m_dat; i++)
    {
        fvec[i] = mydata->user_y[i] - mydata->user_func( mydata->user_tx[i], mydata->user_ty[i], mydata->n_par, par);
    }
    *info = *info; /* to prevent a 'unused variable' warning */
    /* if <parameters drifted away> { *info = -1; } */
}

void lm_print_default( int n_par, double* par, int m_dat, double* fvec,
                       void *data, int iflag, int iter, int nfev )
/*
        *       data  : for soft control of printout behaviour, add control
        *                 variables to the data struct
        *       iflag : 0 (init) 1 (outer loop) 2(inner loop) -1(terminated)
        *       iter  : outer loop counter
        *       nfev  : number of calls to *evaluate
 */
{
    lm_data_type *mydata = (lm_data_type*)data;

    if(mydata->print_flag)
    {
        if (iflag==2) {
            printf ("trying step in gradient direction\n");
        } else if (iflag==1) {
            printf ("determining gradient (iteration %d)\n", iter);
        } else if (iflag==0) {
            printf ("starting minimization\n");
        } else if (iflag==-1) {
            printf ("terminated after %d evaluations\n", nfev);
        }

        printf( "  par: " );
        for(int i=0; i<n_par; ++i )
            printf( " %12g", par[i] );
        printf ( " => norm: %12g\n", lm_enorm( m_dat, fvec ) );

        if ( iflag == -1 ) {
            printf( "  fitting data as follows:\n" );
            for(int i=0; i<m_dat; ++i ) {
                const double tx = (mydata->user_tx)[i];
                const double ty = (mydata->user_ty)[i];
                const double y = (mydata->user_y)[i];
                const double f = mydata->user_func( tx, ty, mydata->n_par, par );
                printf( "    tx[%2d]=%8g     ty[%2d]=%8g     y=%12g fit=%12g     residue=%12g\n",
                        i, tx, i, ty, y, f, y-f );
            }
        }
    }       // if print_flag
}





///=================================================================================

/* *********************** high-level interface **************************** */


void lm_initialize_control( lm_control_type *control )
{
    control->maxcall = 100;
    control->epsilon = 1.e-10; //1.e-14;
    control->stepbound = 100; //100.;
    control->ftol = 1.e-14;
    control->xtol = 1.e-14;
    control->gtol = 1.e-14;
}

void lm_minimize( int m_dat, int n_par, double* par,
                  lm_evaluate_ftype *evaluate, lm_print_ftype *printout,
                  void *data, lm_control_type *control )
{
    std::vector<double> fvec(m_dat);
    std::vector<double> diag(n_par);
    std::vector<double> qtf(n_par);
    std::vector<double> fjac(n_par*m_dat);
    std::vector<double> wa1(n_par);
    std::vector<double> wa2(n_par);
    std::vector<double> wa3(n_par);
    std::vector<double> wa4(m_dat);
    std::vector<int> ipvt(n_par);

    // *** perform fit.

          control->info = 0;
          control->nfev = 0;

    // this goes through the modified legacy interface:
          lm_lmdif( m_dat, n_par, par, &fvec[0], control->ftol, control->xtol, control->gtol,
                    control->maxcall*(n_par+1), control->epsilon, &diag[0], 1,
                    control->stepbound, &(control->info),
                    &(control->nfev), &fjac[0], &ipvt[0], &qtf[0], &wa1[0], &wa2[0], &wa3[0], &wa4[0],
                    evaluate, printout, data );

          (*printout)( n_par, par, m_dat, &fvec[0], data, -1, 0, control->nfev );
          control->fnorm = lm_enorm(m_dat, &fvec[0]);
          if (control->info < 0 ) control->info = 10;
}


// ***** the following messages are referenced by the variable info.

const char *lm_infmsg[] = {
    "improper input parameters",
    "the relative error in the sum of squares is at most tol",
    "the relative error between x and the solution is at most tol",
    "both errors are at most tol",
    "fvec is orthogonal to the columns of the jacobian to machine precision",
    "number of calls to fcn has reached or exceeded 200*(n+1)",
    "ftol is too small. no further reduction in the sum of squares is possible",
    "xtol too small. no further improvement in approximate solution x possible",
    "gtol too small. no further improvement in approximate solution x possible",
    "not enough memory",
    "break requested within function evaluation"
};

const char *lm_shortmsg[] = {
    "invalid input",
    "success (f)",
    "success (p)",
    "success (f,p)",
    "degenerate",
    "call limit",
    "failed (f)",
    "failed (p)",
    "failed (o)",
    "no memory",
    "user break"
};


/* ************************** implementation ******************************* */


#define BUG 0
#if BUG
#include <stdio.h>
#endif

// the following values seem good for an x86:
//#define LM_MACHEP .555e-16 /* resolution of arithmetic */
//#define LM_DWARF  9.9e-324 /* smallest nonzero number */
// the follwoing values should work on any machine:
 #define LM_MACHEP 1.2e-16
 #define LM_DWARF 1.0e-38

// the squares of the following constants shall not under/overflow:
// these values seem good for an x86:
//#define LM_SQRT_DWARF 1.e-160
//#define LM_SQRT_GIANT 1.e150
// the following values should work on any machine:
 #define LM_SQRT_DWARF 3.834e-20
 #define LM_SQRT_GIANT 1.304e19


void lm_qrfac( int m, int n, double* a, int pivot, int* ipvt,
               double* rdiag, double* acnorm, double* wa);
void lm_qrsolv(int n, double* r, int ldr, int* ipvt, double* diag,
               double* qtb, double* x, double* sdiag, double* wa);
void lm_lmpar( int n, double* r, int ldr, int* ipvt, double* diag, double* qtb,
               double delta, double* par, double* x, double* sdiag,
               double* wa1, double* wa2);

#define MIN(a,b) (((a)<=(b)) ? (a) : (b))
#define MAX(a,b) (((a)>=(b)) ? (a) : (b))
#define SQR(x)   (x)*(x)


// ***** the low-level legacy interface for full control.

void lm_lmdif( int m, int n, double* x, double* fvec, double ftol, double xtol,
               double gtol, int maxfev, double epsfcn, double* diag, int mode,
               double factor, int *info, int *nfev,
               double* fjac, int* ipvt, double* qtf,
               double* wa1, double* wa2, double* wa3, double* wa4,
               lm_evaluate_ftype *evaluate, lm_print_ftype *printout,
               void *data )
{
    /*
    *   the purpose of lmdif is to minimize the sum of the squares of
    *   m nonlinear functions in n variables by a modification of
    *   the levenberg-marquardt algorithm. the user must provide a
    *   subroutine evaluate which calculates the functions. the jacobian
    *   is then calculated by a forward-difference approximation.
    *
    *   the multi-parameter interface lm_lmdif is for users who want
    *   full control and flexibility. most users will be better off using
    *   the simpler interface lmfit provided above.
    *
    *   the parameters are the same as in the legacy FORTRAN implementation,
    *   with the following exceptions:
    *      the old parameter ldfjac which gave leading dimension of fjac has
    *        been deleted because this C translation makes no use of two-
    *        dimensional arrays;
    *      the old parameter nprint has been deleted; printout is now controlled
    *        by the user-supplied routine *printout;
    *      the parameter field *data and the function parameters *evaluate and
    *        *printout have been added; they help avoiding global variables.
    *
    *   parameters:
    *
    *    m is a positive integer input variable set to the number
    *      of functions.
    *
    *    n is a positive integer input variable set to the number
    *      of variables. n must not exceed m.
    *
    *    x is an array of length n. on input x must contain
    *      an initial estimate of the solution vector. on output x
    *      contains the final estimate of the solution vector.
    *
    *    fvec is an output array of length m which contains
    *      the functions evaluated at the output x.
    *
    *    ftol is a nonnegative input variable. termination
    *      occurs when both the actual and predicted relative
    *      reductions in the sum of squares are at most ftol.
    *      therefore, ftol measures the relative error desired
    *      in the sum of squares.
    *
    *    xtol is a nonnegative input variable. termination
    *      occurs when the relative error between two consecutive
    *      iterates is at most xtol. therefore, xtol measures the
    *      relative error desired in the approximate solution.
    *
    *    gtol is a nonnegative input variable. termination
    *      occurs when the cosine of the angle between fvec and
    *      any column of the jacobian is at most gtol in absolute
    *      value. therefore, gtol measures the orthogonality
    *      desired between the function vector and the columns
    *      of the jacobian.
    *
    *    maxfev is a positive integer input variable. termination
    *      occurs when the number of calls to lm_fcn is at least
    *      maxfev by the end of an iteration.
    *
    *    epsfcn is an input variable used in determining a suitable
    *      step length for the forward-difference approximation. this
    *      approximation assumes that the relative errors in the
    *      functions are of the order of epsfcn. if epsfcn is less
    *      than the machine precision, it is assumed that the relative
    *      errors in the functions are of the order of the machine
    *      precision.
    *
    *    diag is an array of length n. if mode = 1 (see below), diag is
    *        internally set. if mode = 2, diag must contain positive entries
    *        that serve as multiplicative scale factors for the variables.
    *
    *    mode is an integer input variable. if mode = 1, the
    *      variables will be scaled internally. if mode = 2,
    *      the scaling is specified by the input diag. other
    *      values of mode are equivalent to mode = 1.
    *
    *    factor is a positive input variable used in determining the
    *      initial step bound. this bound is set to the product of
    *      factor and the euclidean norm of diag*x if nonzero, or else
    *      to factor itself. in most cases factor should lie in the
    *      interval (.1,100.). 100. is a generally recommended value.
    *
    *    info is an integer output variable that indicates the termination
    *        status of lm_lmdif as follows:
    *
    *        info < 0  termination requested by user-supplied routine *evaluate;
    *
    *      info = 0  improper input parameters;
    *
    *      info = 1  both actual and predicted relative reductions
    *              in the sum of squares are at most ftol;
    *
    *      info = 2  relative error between two consecutive iterates
    *              is at most xtol;
    *
    *      info = 3  conditions for info = 1 and info = 2 both hold;
    *
    *      info = 4  the cosine of the angle between fvec and any
    *              column of the jacobian is at most gtol in
    *              absolute value;
    *
    *      info = 5  number of calls to lm_fcn has reached or
    *              exceeded maxfev;
    *
    *      info = 6  ftol is too small. no further reduction in
    *              the sum of squares is possible;
    *
    *      info = 7  xtol is too small. no further improvement in
    *              the approximate solution x is possible;
    *
    *      info = 8  gtol is too small. fvec is orthogonal to the
    *              columns of the jacobian to machine precision;
    *
    *    nfev is an output variable set to the number of calls to the
    *        user-supplied routine *evaluate.
    *
    *    fjac is an output m by n array. the upper n by n submatrix
    *      of fjac contains an upper triangular matrix r with
    *      diagonal elements of nonincreasing magnitude such that
    *
    *           t     t       t
    *          p *(jac *jac)*p = r *r,
    *
    *      where p is a permutation matrix and jac is the final
    *      calculated jacobian. column j of p is column ipvt(j)
    *      (see below) of the identity matrix. the lower trapezoidal
    *      part of fjac contains information generated during
    *      the computation of r.
    *
    *    ipvt is an integer output array of length n. ipvt
    *      defines a permutation matrix p such that jac*p = q*r,
    *      where jac is the final calculated jacobian, q is
    *      orthogonal (not stored), and r is upper triangular
    *      with diagonal elements of nonincreasing magnitude.
    *      column j of p is column ipvt(j) of the identity matrix.
    *
    *    qtf is an output array of length n which contains
    *      the first n elements of the vector (q transpose)*fvec.
    *
    *    wa1, wa2, and wa3 are work arrays of length n.
    *
    *    wa4 is a work array of length m.
    *
    *   the following parameters are newly introduced in this C translation:
    *
    *      evaluate is the name of the subroutine which calculates the functions.
    *        a default implementation lm_evaluate_default is provided in lm_eval.c;
    *        alternatively, evaluate can be provided by a user calling program.
    *        it should be written as follows:
    *
    *        void evaluate ( double* par, int m_dat, double* fvec,
    *                       void *data, int *info )
    *        {
    *           // for ( i=0; i<m_dat; ++i )
    *           //     calculate fvec[i] for given parameters par;
    *           // to stop the minimization,
    *           //     set *info to a negative integer.
    *        }
    *
    *      printout is the name of the subroutine which nforms about fit progress.
    *        a default implementation lm_print_default is provided in lm_eval.c;
    *        alternatively, printout can be provided by a user calling program.
    *        it should be written as follows:
    *
    *        void printout ( int n_par, double* par, int m_dat, double* fvec,
    *                       void *data, int iflag, int iter, int nfev )
    *        {
    *           // iflag : 0 (init) 1 (outer loop) 2(inner loop) -1(terminated)
    *           // iter  : outer loop counter
    *           // nfev  : number of calls to *evaluate
    *        }
    *
    *      data is an input pointer to an arbitrary structure that is passed to
    *        evaluate. typically, it contains experimental data to be fitted.
    *
    */

    *nfev = 0; // function evaluation counter

    // *** check the input parameters for errors.

    if ( (n <= 0) || (m < n) || (ftol < 0.)
        || (xtol < 0.) || (gtol < 0.) || (maxfev <= 0) || (factor <= 0.) )
    {
        *info = 0; // invalid parameter
        return;
    }

    if ( mode == 2 )  /* scaling by diag[] */
    {
        for ( int j=0; j<n; j++ )  /* check for non positive elements */
        {
            if ( diag[j] <= 0.0 )
            {
                *info = 0; // invalid parameter
                return;
            }
        }
    }
#if BUG
    printf( "lmdif\n" );
#endif

    // *** evaluate the function at the starting point and calculate its norm.

    *info = 0;
    (*evaluate)( x, m, fvec, data, info );
    (*printout)( n, x, m, fvec, data, 0, 0, ++(*nfev) );
    if ( *info < 0 ) return;

    // *** the outer loop.
    int iter = 1;  // outer loop counter
    double delta = 0.0; // just to prevent a warning (initialization within if-clause)
    double xnorm = 0.0;
    double fnorm = lm_enorm(m,fvec);
    const double eps = sqrt(MAX(epsfcn,LM_MACHEP)); // used in calculating the Jacobian by forward differences

    do {
#if BUG
        printf( "lmdif/ outer loop iter=%d nfev=%d fnorm=%.10e\n",
            iter, *nfev, fnorm );
#endif

        // O** calculate the jacobian matrix.

        for ( int j=0; j<n; j++ )
        {
            const double temp = x[j];
            const double step = eps * fabs(temp) == 0.0 ? eps : eps * fabs(temp);

            x[j] = temp + step;
            *info = 0;
            (*evaluate)( x, m, wa4, data, info );
            (*printout)( n, x, m, wa4, data, 1, iter, ++(*nfev) );
            if ( *info < 0 ) return;  // user requested break
            x[j] = temp;
            for (int i=0; i<m; i++ )
                fjac[j*m+i] = (wa4[i] - fvec[i]) / step;
        }
#if BUG>1
        // DEBUG: print the entire matrix
        for ( i=0; i<m; i++ )
        {
            for ( j=0; j<n; j++ )
                printf( "%.5e ", y[j*m+i] );
            printf( "\n" );
        }
#endif

        // O** compute the qr factorization of the jacobian.

        lm_qrfac( m, n, fjac, 1, ipvt, wa1, wa2, wa3);

        // O** on the first iteration ...

        if (iter == 1)
        {
            if (mode != 2)
                //      ... scale according to the norms of the columns of the initial jacobian.
            {
                for (int j=0; j<n; j++ )
                {
                    diag[j] = wa2[j];
                    if ( wa2[j] == 0. )
                        diag[j] = 1.;
                }
            }

            //      ... calculate the norm of the scaled x and
            //          initialize the step bound delta.

            for (int j=0; j<n; j++ )
                wa3[j] = diag[j] * x[j];

            xnorm = lm_enorm( n, wa3 );
            delta = factor*xnorm;
            if (delta == 0.)
                delta = factor;
        }

        // O** form (q transpose)*fvec and store the first n components in qtf.

        for (int i=0; i<m; i++ )
            wa4[i] = fvec[i];

        for (int j=0; j<n; j++ )
        {
            double temp3 = fjac[j*m+j];
            if (temp3 != 0.)
            {
                double sum = 0.0;
                for (int i=j; i<m; i++ )
                    sum += fjac[j*m+i] * wa4[i];
                double temp = -sum / temp3;
                for (int i=j; i<m; i++ )
                    wa4[i] += fjac[j*m+i] * temp;
            }
            fjac[j*m+j] = wa1[j];
            qtf[j] = wa4[j];
        }

        // O** compute the norm of the scaled gradient and test for convergence.

        double gnorm = 0;
        if ( fnorm != 0 )
        {
            for (int j=0; j<n; j++ )
            {
                if ( wa2[ ipvt[j] ] == 0 ) continue;

                double sum = 0.0;
                for (int i=0; i<=j; i++ )
                    sum += fjac[j*m+i] * qtf[i] / fnorm;
                gnorm = MAX( gnorm, fabs(sum/wa2[ ipvt[j] ]) );
            }
        }

        if ( gnorm <= gtol )
        {
            *info = 4;
            return;
        }

        // O** rescale if necessary.

        if ( mode != 2 )
        {
            for (int j=0; j<n; j++ )
                diag[j] = MAX(diag[j],wa2[j]);
        }

        // O** the inner loop.
        const double kP0001 = 1.0e-4;
        double ratio = 0.0;
        do {
#if BUG
            printf( "lmdif/ inner loop iter=%d nfev=%d\n", iter, *nfev );
#endif

            // OI* determine the levenberg-marquardt parameter.
            double par = 0;   // levenberg-marquardt parameter
            lm_lmpar( n,fjac,m,ipvt,diag,qtf,delta,&par,wa1,wa2,wa3,wa4 );

            // OI* store the direction p and x + p. calculate the norm of p.

            for (int j=0; j<n; j++ )
            {
                wa1[j] = -wa1[j];
                wa2[j] = x[j] + wa1[j];
                wa3[j] = diag[j]*wa1[j];
            }
            double pnorm = lm_enorm(n,wa3);

            // OI* on the first iteration, adjust the initial step bound.

            if ( *nfev<= 1+n ) // bug corrected by J. Wuttke in 2004
                delta = MIN(delta,pnorm);

            // OI* evaluate the function at x + p and calculate its norm.

            *info = 0;
            (*evaluate)( wa2, m, wa4, data, info );
            (*printout)( n, x, m, wa4, data, 2, iter, ++(*nfev) );
            if ( *info < 0 ) return;  // user requested break

            double fnorm1 = lm_enorm(m,wa4);
#if BUG
            printf( "lmdif/ pnorm %.10e  fnorm1 %.10e  fnorm %.10e"
                " delta=%.10e par=%.10e\n",
                pnorm, fnorm1, fnorm, delta, par );
#endif

            // OI* compute the scaled actual reduction.
            const double kP1 = 0.1;
            const double actred = kP1 * fnorm1 < fnorm ? 1 - SQR( fnorm1/fnorm ) : -1;

            // OI* compute the scaled predicted reduction and
            //     the scaled directional derivative.

            for (int j=0; j<n; j++ )
            {
                wa3[j] = 0;
                for (int i=0; i<=j; i++ )
                    wa3[i] += fjac[j*m+i]*wa1[ ipvt[j] ];
            }
            const double temp1 = lm_enorm(n,wa3) / fnorm;
            const double temp2 = sqrt(par) * pnorm / fnorm;
            const double prered = SQR(temp1) + 2 * SQR(temp2);
            const double dirder = - ( SQR(temp1) + SQR(temp2) );

            // OI* compute the ratio of the actual to the predicted reduction.

            ratio = prered !=0 ? actred / prered : 0.0;
#if BUG
            printf( "lmdif/ actred=%.10e prered=%.10e ratio=%.10e"
                " sq(1)=%.10e sq(2)=%.10e dd=%.10e\n",
                actred, prered, prered!=0 ? ratio : 0.,
                SQR(temp1), SQR(temp2), dirder );
#endif

            // OI* update the step bound.
            const double kP5 = 0.5;
            const double kP25 = 0.25;
            const double kP75 = 0.75;

            if (ratio <= kP25)
            {
                double temp = actred >= 0.0 ? kP5 : kP5*dirder/(dirder + kP5*actred);
                if ( kP1*fnorm1 >= fnorm || temp < kP1 )
                    temp = kP1;
                delta = temp * MIN(delta,pnorm/kP1);
                par /= temp;
            }
            else if ( par == 0. || ratio >= kP75 )
            {
                delta = pnorm/kP5;
                par *= kP5;
            }

            // OI* test for successful iteration...
            if (ratio >= kP0001)
            {

                //     ... successful iteration. update x, fvec, and their norms.

                for (int j=0; j<n; j++ )
                {
                    x[j] = wa2[j];
                    wa2[j] = diag[j]*x[j];
                }
                for (int i=0; i<m; i++ )
                    fvec[i] = wa4[i];
                xnorm = lm_enorm(n,wa2);
                fnorm = fnorm1;
                iter++;
            }
#if BUG
            else {
                printf( "ATTN: iteration considered unsuccessful\n" );
            }
#endif

            // OI* tests for convergence ( otherwise *info = 1, 2, or 3 )

            *info = 0; // do not terminate (unless overwritten by nonzero value)
            if ( fabs(actred) <= ftol && prered <= ftol && kP5*ratio <= 1 )
                *info = 1;
            if (delta <= xtol*xnorm)
                *info += 2;
            if ( *info != 0)
                return;

            // OI* tests for termination and stringent tolerances.

            if ( *nfev >= maxfev)
                *info = 5;
            if ( fabs(actred) <= LM_MACHEP &&
                prered <= LM_MACHEP && kP5*ratio <= 1 )
                *info = 6;
            if (delta <= LM_MACHEP*xnorm)
                *info = 7;
            if (gnorm <= LM_MACHEP)
                *info = 8;
            if ( *info != 0)
                return;

            // OI* end of the inner loop. repeat if iteration unsuccessful.

        } while (ratio < kP0001);

        // O** end of the outer loop.

    } while (1);

}



void lm_lmpar(int n, double* r, int ldr, int* ipvt, double* diag, double* qtb,
              double delta, double* par, double* x, double* sdiag,
              double* wa1, double* wa2)
{
    /*     given an m by n matrix a, an n by n nonsingular diagonal
    *     matrix d, an m-vector b, and a positive number delta,
    *     the problem is to determine a value for the parameter
    *     par such that if x solves the system
    *
    *        a*x = b ,       sqrt(par)*d*x = 0 ,
    *
    *     in the least squares sense, and dxnorm is the euclidean
    *     norm of d*x, then either par is 0. and
    *
    *        (dxnorm-delta) .le. 0.1*delta ,
    *
    *     or par is positive and
    *
    *        abs(dxnorm-delta) .le. 0.1*delta .
    *
    *     this subroutine completes the solution of the problem
    *     if it is provided with the necessary information from the
    *     qr factorization, with column pivoting, of a. that is, if
    *     a*p = q*r, where p is a permutation matrix, q has orthogonal
    *     columns, and r is an upper triangular matrix with diagonal
    *     elements of nonincreasing magnitude, then lmpar expects
    *     the full upper triangle of r, the permutation matrix p,
    *     and the first n components of (q transpose)*b. on output
    *     lmpar also provides an upper triangular matrix s such that
    *
    *         t       t               t
    *        p *(a *a + par*d*d)*p = s *s .
    *
    *     s is employed within lmpar and may be of separate interest.
    *
    *     only a few iterations are generally needed for convergence
    *     of the algorithm. if, however, the limit of 10 iterations
    *     is reached, then the output par will contain the best
    *     value obtained so far.
    *
    *     parameters:
    *
    *    n is a positive integer input variable set to the order of r.
    *
    *    r is an n by n array. on input the full upper triangle
    *      must contain the full upper triangle of the matrix r.
    *      on output the full upper triangle is unaltered, and the
    *      strict lower triangle contains the strict upper triangle
    *      (transposed) of the upper triangular matrix s.
    *
    *    ldr is a positive integer input variable not less than n
    *      which specifies the leading dimension of the array r.
    *
    *    ipvt is an integer input array of length n which defines the
    *      permutation matrix p such that a*p = q*r. column j of p
    *      is column ipvt(j) of the identity matrix.
    *
    *    diag is an input array of length n which must contain the
    *      diagonal elements of the matrix d.
    *
    *    qtb is an input array of length n which must contain the first
    *      n elements of the vector (q transpose)*b.
    *
    *    delta is a positive input variable which specifies an upper
    *      bound on the euclidean norm of d*x.
    *
    *    par is a nonnegative variable. on input par contains an
    *      initial estimate of the levenberg-marquardt parameter.
    *      on output par contains the final estimate.
    *
    *    x is an output array of length n which contains the least
    *      squares solution of the system a*x = b, sqrt(par)*d*x = 0,
    *      for the output par.
    *
    *    sdiag is an output array of length n which contains the
    *      diagonal elements of the upper triangular matrix s.
    *
    *    wa1 and wa2 are work arrays of length n.
    *
    */

#if BUG
    printf( "lmpar\n" );
#endif

    // *** compute and store in x the gauss-newton direction. if the
    //     jacobian is rank-deficient, obtain a least squares solution.

    int nsing = n;
    for (int j=0; j<n; j++ )
    {
        wa1[j] = qtb[j];
        if ( r[j*ldr+j] == 0 && nsing == n )
            nsing = j;
        if (nsing < n)
            wa1[j] = 0;
    }
#if BUG
    printf( "nsing %d ", nsing );
#endif
    for (int j=nsing-1; j>=0; j-- )
    {
        wa1[j] = wa1[j]/r[j+ldr*j];
        const double temp = wa1[j];
        for (int i=0; i<j; i++ )
            wa1[i] -= r[j*ldr+i]*temp;
    }

    for (int j=0; j<n; j++ )
        x[ ipvt[j] ] = wa1[j];

    // *** initialize the iteration counter.
    //     evaluate the function at the origin, and test
    //     for acceptance of the gauss-newton direction.

    int iter = 0;
    for (int j=0; j<n; j++ )
        wa2[j] = diag[j]*x[j];

    double dxnorm = lm_enorm(n,wa2);
    double fp = dxnorm - delta;
    const double kP1 = 0.1;
    if (fp <= kP1*delta)
    {
#if BUG
        printf( "lmpar/ terminate (fp<delta/10\n" );
#endif
        *par = 0;
        return;
    }

    // *** if the jacobian is not rank deficient, the newton
    //     step provides a lower bound, parl, for the 0. of
    //     the function. otherwise set this bound to 0..

    double parl = 0.0;
    if (nsing >= n)
    {
        for (int j=0; j<n; j++ )
            wa1[j] = diag[ ipvt[j] ] * wa2[ ipvt[j] ] / dxnorm;

        for (int j=0; j<n; j++ )
        {
            double sum = 0.0;
            for (int i=0; i<j; i++ )
                sum += r[j*ldr+i]*wa1[i];
            wa1[j] = (wa1[j] - sum)/r[j+ldr*j];
        }
        const double temp = lm_enorm(n,wa1);
        parl = fp/delta/temp/temp;
    }

    // *** calculate an upper bound, paru, for the 0. of the function.

    for (int j=0; j<n; j++ )
    {
        double sum = 0;
        for (int i=0; i<=j; i++ )
            sum += r[j*ldr+i]*qtb[i];
        wa1[j] = sum/diag[ ipvt[j] ];
    }
    const double gnorm = lm_enorm(n,wa1);
    double paru = gnorm/delta == 0.0 ? LM_DWARF/MIN(delta,kP1) : gnorm / delta;

    // *** if the input par lies outside of the interval (parl,paru),
    //     set par to the closer endpoint.

    *par = MAX( *par,parl);
    *par = MIN( *par,paru);
    if ( *par == 0.)
        *par = gnorm/dxnorm;
#if BUG
    printf( "lmpar/ parl %.4e  par %.4e  paru %.4e\n", parl, *par, paru );
#endif

    // *** iterate.

    for ( ; ; iter++ ) {

        // *** evaluate the function at the current value of par.
        const double kP001 = 0.001;
        if ( *par == 0.)
            *par = MAX(LM_DWARF,kP001*paru);
        double temp = sqrt( *par );
        for (int j=0; j<n; j++ )
            wa1[j] = temp*diag[j];
        lm_qrsolv( n, r, ldr, ipvt, wa1, qtb, x, sdiag, wa2);
        for (int j=0; j<n; j++ )
            wa2[j] = diag[j]*x[j];
        dxnorm = lm_enorm(n,wa2);
        const double fp_old = fp;
        fp = dxnorm - delta;

        // ***       if the function is small enough, accept the current value
        //     of par. also test for the exceptional cases where parl
        //     is 0. or the number of iterations has reached 10.

        if ( fabs(fp) <= kP1*delta
            || (parl == 0. && fp <= fp_old && fp_old < 0.)
            || iter == 10 )
            break; // the only exit from this loop

        // *** compute the Newton correction.

        for (int j=0; j<n; j++ )
            wa1[j] = diag[ ipvt[j] ] * wa2[ ipvt[j] ] / dxnorm;

        for (int j=0; j<n; j++ )
        {
            wa1[j] = wa1[j]/sdiag[j];
            for (int i=j+1; i<n; i++ )
                wa1[i] -= r[j*ldr+i]*wa1[j];
        }
        temp = lm_enorm( n, wa1);
        double parc = fp/delta/temp/temp;

        // *** depending on the sign of the function, update parl or paru.

        if (fp > 0)
            parl = MAX(parl, *par);
        else if (fp < 0)
            paru = MIN(paru, *par);
        // the case fp==0 is precluded by the break condition

        // *** compute an improved estimate for par.

        *par = MAX(parl, *par + parc);

    }

}



void lm_qrfac(int m, int n, double* a, int pivot, int* ipvt,
              double* rdiag, double* acnorm, double* wa)
{
    /*
    *     this subroutine uses householder transformations with column
    *     pivoting (optional) to compute a qr factorization of the
    *     m by n matrix a. that is, qrfac determines an orthogonal
    *     matrix q, a permutation matrix p, and an upper trapezoidal
    *     matrix r with diagonal elements of nonincreasing magnitude,
    *     such that a*p = q*r. the householder transformation for
    *     column k, k = 1,2,...,min(m,n), is of the form
    *
    *                    t
    *        i - (1/u(k))*u*u
    *
    *     where u has 0.s in the first k-1 positions. the form of
    *     this transformation and the method of pivoting first
    *     appeared in the corresponding linpack subroutine.
    *
    *     parameters:
    *
    *    m is a positive integer input variable set to the number
    *      of rows of a.
    *
    *    n is a positive integer input variable set to the number
    *      of columns of a.
    *
    *    a is an m by n array. on input a contains the matrix for
    *      which the qr factorization is to be computed. on output
    *      the strict upper trapezoidal part of a contains the strict
    *      upper trapezoidal part of r, and the lower trapezoidal
    *      part of a contains a factored form of q (the non-trivial
    *      elements of the u vectors described above).
    *
    *    pivot is a logical input variable. if pivot is set true,
    *      then column pivoting is enforced. if pivot is set false,
    *      then no column pivoting is done.
    *
    *    ipvt is an integer output array of length lipvt. ipvt
    *      defines the permutation matrix p such that a*p = q*r.
    *      column j of p is column ipvt(j) of the identity matrix.
    *      if pivot is false, ipvt is not referenced.
    *
    *    rdiag is an output array of length n which contains the
    *      diagonal elements of r.
    *
    *    acnorm is an output array of length n which contains the
    *      norms of the corresponding columns of the input matrix a.
    *      if this information is not needed, then acnorm can coincide
    *      with rdiag.
    *
    *    wa is a work array of length n. if pivot is false, then wa
    *      can coincide with rdiag.
    *
    */

    // *** compute the initial column norms and initialize several arrays.

    for (int j=0; j<n; j++ )
    {
        acnorm[j] = lm_enorm(m, &a[j*m]);
        rdiag[j] = acnorm[j];
        wa[j] = rdiag[j];
        if ( pivot )
            ipvt[j] = j;
    }
#if BUG
    printf( "qrfac\n" );
#endif

    // *** reduce a to r with householder transformations.

    const int minmn = MIN(m,n);
    for (int j=0; j<minmn; j++ )
    {
        int kmax = j, k;
        if ( !pivot ) goto pivot_ok;

        // *** bring the column of largest norm into the pivot position.

        for (k=j+1; k<n; k++ )
            if (rdiag[k] > rdiag[kmax])
                kmax = k;
        if (kmax == j) goto pivot_ok; // bug fixed in rel 2.1

        for (int i=0; i<m; i++ )
        {
            std::swap(a[j*m+i], a[kmax*m+i]);
            //const double temp = a[j*m+i];
            //a[j*m+i]    = a[kmax*m+i];
            //a[kmax*m+i] = temp;
        }
        rdiag[kmax] = rdiag[j];
        wa[kmax] = wa[j];
        std::swap(ipvt[j], ipvt[kmax]);
        //k = ipvt[j];
        //ipvt[j] = ipvt[kmax];
        //ipvt[kmax] = k;

pivot_ok:

        // *** compute the Householder transformation to reduce the
        //     j-th column of a to a multiple of the j-th unit vector.

        double ajnorm = lm_enorm( m-j, &a[j*m+j] );
        if (ajnorm == 0.)
        {
            rdiag[j] = 0;
            continue;
        }

        if (a[j*m+j] < 0.)
            ajnorm = -ajnorm;
        for (int i=j; i<m; i++ )
            a[j*m+i] /= ajnorm;
        a[j*m+j] += 1;

        // *** apply the transformation to the remaining columns
        //     and update the norms.

        for ( k=j+1; k<n; k++ )
        {
            double sum = 0;

            for (int i=j; i<m; i++ )
                sum += a[j*m+i]*a[k*m+i];

            double temp = sum/a[j+m*j];

            for (int i=j; i<m; i++ )
                a[k*m+i] -= temp * a[j*m+i];

            if ( pivot && rdiag[k] != 0. )
            {
                temp = a[m*k+j]/rdiag[k];
                temp = MAX( 0., 1-temp*temp );
                rdiag[k] *= sqrt(temp);
                temp = rdiag[k]/wa[k];
                const double kP05 = 0.05;
                if ( kP05*SQR(temp) <= LM_MACHEP )
                {
                    rdiag[k] = lm_enorm( m-j-1, &a[m*k+j+1]);
                    wa[k] = rdiag[k];
                }
            }
        }

        rdiag[j] = -ajnorm;
    }
}



void lm_qrsolv(int n, double* r, int ldr, int* ipvt, double* diag,
               double* qtb, double* x, double* sdiag, double* wa)
{
    /*
    *     given an m by n matrix a, an n by n diagonal matrix d,
    *     and an m-vector b, the problem is to determine an x which
    *     solves the system
    *
    *        a*x = b ,       d*x = 0 ,
    *
    *     in the least squares sense.
    *
    *     this subroutine completes the solution of the problem
    *     if it is provided with the necessary information from the
    *     qr factorization, with column pivoting, of a. that is, if
    *     a*p = q*r, where p is a permutation matrix, q has orthogonal
    *     columns, and r is an upper triangular matrix with diagonal
    *     elements of nonincreasing magnitude, then qrsolv expects
    *     the full upper triangle of r, the permutation matrix p,
    *     and the first n components of (q transpose)*b. the system
    *     a*x = b, d*x = 0, is then equivalent to
    *
    *             t     t
    *        r*z = q *b ,  p *d*p*z = 0 ,
    *
    *     where x = p*z. if this system does not have full rank,
    *     then a least squares solution is obtained. on output qrsolv
    *     also provides an upper triangular matrix s such that
    *
    *         t       t           t
    *        p *(a *a + d*d)*p = s *s .
    *
    *     s is computed within qrsolv and may be of separate interest.
    *
    *     parameters
    *
    *    n is a positive integer input variable set to the order of r.
    *
    *    r is an n by n array. on input the full upper triangle
    *      must contain the full upper triangle of the matrix r.
    *      on output the full upper triangle is unaltered, and the
    *      strict lower triangle contains the strict upper triangle
    *      (transposed) of the upper triangular matrix s.
    *
    *    ldr is a positive integer input variable not less than n
    *      which specifies the leading dimension of the array r.
    *
    *    ipvt is an integer input array of length n which defines the
    *      permutation matrix p such that a*p = q*r. column j of p
    *      is column ipvt(j) of the identity matrix.
    *
    *    diag is an input array of length n which must contain the
    *      diagonal elements of the matrix d.
    *
    *    qtb is an input array of length n which must contain the first
    *      n elements of the vector (q transpose)*b.
    *
    *    x is an output array of length n which contains the least
    *      squares solution of the system a*x = b, d*x = 0.
    *
    *    sdiag is an output array of length n which contains the
    *      diagonal elements of the upper triangular matrix s.
    *
    *    wa is a work array of length n.
    *
    */

    // *** copy r and (q transpose)*b to preserve input and initialize s.
    //     in particular, save the diagonal elements of r in x.

    for (int j=0; j<n; j++ )
    {
        for (int i=j; i<n; i++ )
            r[j*ldr+i] = r[i*ldr+j];
        x[j] = r[j*ldr+j];
        wa[j] = qtb[j];
    }
#if BUG
    printf( "qrsolv\n" );
#endif

    // *** eliminate the diagonal matrix d using a givens rotation.

    for (int j=0; j<n; j++ )
    {

        // ***       prepare the row of d to be eliminated, locating the
        //     diagonal element using p from the qr factorization.

        double qtbpj = 0.0;

        if (diag[ ipvt[j] ] == 0.)
            goto L90;
        for (int k=j; k<n; k++ )
            sdiag[k] = 0.;
        sdiag[j] = diag[ ipvt[j] ];

        // ***       the transformations to eliminate the row of d
        //     modify only a single element of (q transpose)*b
        //     beyond the first n, which is initially 0..

        for (int k=j; k<n; k++ )
        {
            const double p25 = 0.25;
            const double p5 = 0.5;

            //        determine a givens rotation which eliminates the
            //        appropriate element in the current row of d.

            if (sdiag[k] == 0.)
                continue;
            const int kk = k + ldr * k; // <! keep this shorthand !>
            double sin, cos; // these are local variables, not functions

            if ( fabs(r[kk]) < fabs(sdiag[k]) )
            {
                const double cotan = r[kk]/sdiag[k];
                sin = p5/sqrt(p25+p25*SQR(cotan));
                cos = sin*cotan;
            }
            else
            {
                const double tan = sdiag[k]/r[kk];
                cos = p5/sqrt(p25+p25*SQR(tan));
                sin = cos*tan;
            }

            // ***          compute the modified diagonal element of r and
            //        the modified element of ((q transpose)*b,0).

            r[kk] = cos*r[kk] + sin*sdiag[k];
            double temp = cos*wa[k] + sin*qtbpj;
            qtbpj = -sin*wa[k] + cos*qtbpj;
            wa[k] = temp;

            // *** accumulate the transformation in the row of s.

            for (int i=k+1; i<n; i++ )
            {
                temp = cos*r[k*ldr+i] + sin*sdiag[i];
                sdiag[i] = -sin*r[k*ldr+i] + cos*sdiag[i];
                r[k*ldr+i] = temp;
            }
        }
L90:

        // *** store the diagonal element of s and restore
        //     the corresponding diagonal element of r.

        sdiag[j] = r[j*ldr+j];
        r[j*ldr+j] = x[j];
    }

    // *** solve the triangular system for z. if the system is
    //     singular, then obtain a least squares solution.

    int nsing = n;
    for (int j=0; j<n; j++ )
    {
        if ( sdiag[j] == 0. && nsing == n )
            nsing = j;
        if (nsing < n)
            wa[j] = 0;
    }

    for (int j=nsing-1; j>=0; j-- )
    {
        double sum = 0.0;
        for (int i=j+1; i<nsing; i++ )
            sum += r[j*ldr+i]*wa[i];
        wa[j] = (wa[j] - sum)/sdiag[j];
    }

    // *** permute the components of z back to components of x.

    for (int j=0; j<n; j++ )
        x[ ipvt[j] ] = wa[j];
}



double lm_enorm( int n, double *x )
{
    /*     given an n-vector x, this function calculates the
    *     euclidean norm of x.
    *
    *     the euclidean norm is computed by accumulating the sum of
    *     squares in three different sums. the sums of squares for the
    *     small and large components are scaled so that no overflows
    *     occur. non-destructive underflows are permitted. underflows
    *     and overflows do not occur in the computation of the unscaled
    *     sum of squares for the intermediate components.
    *     the definitions of small, intermediate and large components
    *     depend on two constants, LM_SQRT_DWARF and LM_SQRT_GIANT. the main
    *     restrictions on these constants are that LM_SQRT_DWARF**2 not
    *     underflow and LM_SQRT_GIANT**2 not overflow.
    *
    *     parameters
    *
    *    n is a positive integer input variable.
    *
    *    x is an input array of length n.
    */

    double s1 = 0.0, s2 = 0.0, s3 = 0.0;
    double x1max = 0.0, x3max = 0.0;
    const double agiant = LM_SQRT_GIANT/( (double) n);

    for (int i=0; i<n; i++ )
    {
        double xabs = fabs(x[i]);
        if ( xabs > LM_SQRT_DWARF && xabs < agiant )
        {
            // **  sum for intermediate components.
            s2 += xabs*xabs;
            continue;
        }

        if ( xabs >  LM_SQRT_DWARF )
        {
            // **  sum for large components.
            if (xabs > x1max)
            {
                s1 = 1 + s1*SQR(x1max/xabs);
                x1max = xabs;
            }
            else
            {
                s1 += SQR(xabs/x1max);
            }
            continue;
        }
        // **  sum for small components.
        if (xabs > x3max)
        {
            s3 = 1 + s3*SQR(x3max/xabs);
            x3max = xabs;
        }
        else
        {
            if (xabs != 0.)
            {
                s3 += SQR(xabs/x3max);
            }
        }
    }

    // *** calculation of norm.

    if (s1 != 0)
        return x1max*sqrt(s1 + (s2/x1max)/x1max);
    if (s2 != 0)
    {
        if (s2 >= x3max)
            return sqrt( s2*(1+(x3max/s2)*(x3max*s3)) );
        return sqrt( x3max*((s2/x3max)+(x3max*s3)) );
    }

    return x3max*sqrt(s3);
}


double lat_gc_crosses_meridian( double lat1, double lon1, double lat2, double lon2, double lon )
{
    /*    
    Calculates a latitude at which a GC route between two points crosses a given meridian
    */

    double dlon = lon * DEGREE;
    double dlat1 = lat1 * DEGREE;
    double dlat2 = lat2 * DEGREE;
    double dlon1 = lon1 * DEGREE;
    double dlon2 = lon2 * DEGREE;
    
    return RADIAN * atan((sin(dlat1) * cos(dlat2) * sin(dlon-dlon2)
              - sin(dlat2) * cos(dlat1) * sin(dlon-dlon1)) / (cos(dlat1) * cos(dlat2) * sin(dlon1-dlon2)));
}

double lat_rl_crosses_meridian( double lat1, double lon1, double lat2, double lon2, double lon )
{
    /*
    Calculates a latitude at which a loxodromic route between two points crosses a given meridian
    */
    
    double brg;
    
    DistanceBearingMercator( lat2, lon2, lat1, lon1, &brg, NULL );

    double x1, y1, x;
    toSM( lat1, lon1, 0., lon, &x1, &y1 );
    toSM( lat1, lon, 0., lon, &x, &y1 );
    
    double dir = 1.0;
    if ( brg >= 270.0 )
    {
        brg -= 270.0;
    }
    else if ( brg >= 180. )
    {
        brg = 270.0 - brg;
        dir = -1.0;
    }
    else if ( brg >= 90. )
    {
        brg -= 90.0;
        dir = -1.0;
    }
    else
    {
        brg = 90.0 - brg;
    }

    double ydelta = fabs( x1 ) * tan( brg * DEGREE );
    
    double crosslat, crosslon;
    fromSM(x, y1 + dir * ydelta, 0., lon, &crosslat, &crosslon);

    return crosslat;
}
