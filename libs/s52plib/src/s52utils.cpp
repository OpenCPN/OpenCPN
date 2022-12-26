/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  S52 Utility Library
 * Author:   David Register, Sylvain Duclos
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
 *                                                                         *
 *   Copyright (C) 2000-2004  Sylvain Duclos                               *
 *   sduclos@users.sourceforgue.net                                        *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************
 *
 */


#include "s52utils.h"
//#include <stdio.h>        // FILE
//#include <string.h>       // strncmp()

//#include <stdlib.h>       // exit()

// configuration file
#define CONF_NAME "./s52test.conf"

#define TRUE 1
#define FALSE 0

//#define CPATH_MAX 255
///////////////////////////////////////////////////////////////////
//
//   GLOBAL MARINER PARAMETER (will move out)
//
// NOTE: value for Chart No 1 found in README
//
// Soundings      ON
// Text           ON
// Depth Shades   4
// Safety Contour 10 m
// Safety Depth   7 m
// Shallow        5 m
// Deep           30 m

/* A) value for Chart No 1 */
/*
double SHOW_TEXT       = TRUE;    // view group 23
double TWO_SHADES      = FALSE;   // flag indicating selection of two depth
shades (on/off) [default ON] double SAFETY_CONTOUR  = 10.0;    // selected
safety contour (meters) [IMO PS 3.6] double SAFETY_DEPTH    = 7.0;     //
selected safety depth (meters) [IMO PS 3.7] double SHALLOW_CONTOUR = 5.0;     //
selected shallow water contour (meters) (optional) double DEEP_CONTOUR = 30.0;
// selected deepwatercontour (meters) (optional)
*/

/* B) value for testing */
/*
//gboolean TWO_SHADES      = TRUE;     // flag indicating selection of two depth
shades (on/off) [default ON] gboolean TWO_SHADES      = FALSE;    // flag
indicating selection of two depth shades (on/off) [default ON] gboolean
SHOW_TEXT       = TRUE;     // view group 23
//double    SAFETY_DEPTH    = 30.0;    // selected safety depth (meters) [IMO
PS 3.7]
//double    SHALLOW_CONTOUR = 2.0;     // selected shallow water contour
(meters) (optional) double    SAFETY_DEPTH    = 15.0;    // selected safety
depth (meters) [IMO PS 3.7] double    SHALLOW_CONTOUR = 5.0;     // selected
shallow water contour (meters) (optional)
//double    SAFETY_CONTOUR  = 30.0;    // selected safety contour (meters) [IMO
PS 3.6]
//double    DEEP_CONTOUR    = 30.0;    // selected deepwatercontour (meters)
(optional)
//double    SAFETY_CONTOUR  = 5.0;     // selected safety contour (meters) [IMO
PS 3.6]
//double    DEEP_CONTOUR    = 10.0;    // selected deepwatercontour (meters)
(optional) double    SAFETY_CONTOUR  = 10.0;    // selected safety contour
(meters) [IMO PS 3.6] double    DEEP_CONTOUR    = 15.0;    // selected
deepwatercontour (meters) (optional)
*/

/* param needed for certain conditional symbology */
/*
gboolean SHALLOW_PATTERN = FALSE;    // flag indicating selection of shallow
water highlight (on/off)(optional) [default OFF] gboolean SHIPS_OUTLINE   =
FALSE;    // flag indicating selection of ship scale symbol (on/off) [IMO
PS 8.4] double   DISTANCE_TAGS   = 0.0;      // selected spacing of "distance to
run" tags at a route (nm) double   TIME_TAGS       = 0.0;      // selected
spacing of time tags at the pasttrack (min) gboolean FULL_SECTORS    = TRUE; //
show full length light sector lines gboolean SYMBOLIZED_BND  = TRUE;     //
symbolized area boundaries
*/

// WARNING: must be in sync with S52_MAR_param_t
/*
static char *_MARparamNm[]  = {
    "S52_MAR_NONE",             //= 0,    // default
    "S52_MAR_SHOW_TEXT",        //= 1,    // view group 23
    "S52_MAR_TWO_SHADES",       //= 2,    // flag indicating selection of two
depth shades (on/off) [default ON] "S52_MAR_SAFETY_CONTOUR",   //= 3,    //
selected safety contour (meters) [IMO PS 3.6] "S52_MAR_SAFETY_DEPTH",     //= 4,
// selected safety depth (meters) [IMO PS 3.7] "S52_MAR_SHALLOW_CONTOUR",  //=
5,    // selected shallow water contour (meters) (optional)
    "S52_MAR_DEEP_CONTOUR",     //= 6,    // selected deepwatercontour (meters)
(optional) "S52_MAR_SHALLOW_PATTERN",  //= 7,    // flag indicating selection of
shallow water highlight (on/off)(optional) [default OFF]
    "S52_MAR_SHIPS_OUTLINE",    //= 8,    // flag indicating selection of ship
scale symbol (on/off) [IMO PS 8.4] "S52_MAR_DISTANCE_TAGS",    //= 9,    //
selected spacing of "distance to run" tags at a route (nm) "S52_MAR_TIME_TAGS",
//= 10,   // selected spacing of time tags at the pasttrack (min)
    "S52_MAR_FULL_SECTORS",     //= 11,   // show full length light sector lines
    "S52_MAR_SYMBOLIZED_BND",   //= 12,   // symbolized area boundaries
    "S52_MAR_SYMPLIFIED_PNT",   //= 13,   // simplified point
    "S52_MAR_DISP_CATEGORY",    //= 14,   // display category
    "S52_MAR_COLOR_PALETTE",    //= 15,   // color palette

    "S52_MAR_NUM"               //= 16    // number of parameters
};
*/

// WARNING: must be in sync with _MARparamNm
static double _MARparamVal[] = {
    0.0,    // NONE
    TRUE,   // SHOW_TEXT
    FALSE,  // TWO_SHADES

    3.0,  // SAFETY_CONTOUR
    // 0.0,     // SAFETY_CONTOUR  --to test DEPCNT02 selection (GL) in
    // CA49995A.000 0.5,     // SAFETY_CONTOUR  --to test DEPCNT02 selection (GL)
    // in CA49995A.000

    3.0,  // SAFETY_DEPTH
    // 5.0,      // SAFETY_DEPTH

    2.0,  // SHALLOW_CONTOUR
    6.0,  // DEEP_CONTOUR

    // FALSE,    // SHALLOW_PATTERN
    TRUE,  // SHALLOW_PATTERN

    FALSE,  // SHIPS_OUTLINE
    0.0,    // DISTANCE_TAGS
    0.0,    // TIME_TAGS
    TRUE,   // FULL_SECTORS
    FALSE,  // SYMBOLIZED_BND

    FALSE,  // SYMPLIFIED_PNT

    //    'D',      // S52_MAR_DISP_CATEGORY --DISPLAYBASE
    //    'S',      // S52_MAR_DISP_CATEGORY --STANDARD
    'O',  // S52_MAR_DISP_CATEGORY --OTHER

    //    0,        // S52_MAR_COLOR_PALETTE --DAY_BRIGHT
    1,  // S52_MAR_COLOR_PALETTE --DAY_BLACKBACK
        //    2,        // S52_MAR_COLOR_PALETTE --DAY_WHITEBACK
        //    3,        // S52_MAR_COLOR_PALETTE --DUSK
        //    4,        // S52_MAR_COLOR_PALETTE --NIGHT

    16.0  // NUM
};

double S52_getMarinerParam(S52_MAR_param_t param)
// return Mariner parameter or '0.0' if fail
// FIXME: check mariner param against groups selection
{
  //      DSR
  return _MARparamVal[param];
}

int S52_setMarinerParam(S52_MAR_param_t param, double val) {
  if (S52_MAR_NONE < param && param < S52_MAR_NUM)
    _MARparamVal[param] = val;
  else
    return FALSE;

  return TRUE;
}
