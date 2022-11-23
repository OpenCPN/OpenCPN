// S52utils.h: utility
//
// Project:  OpENCview

/*
    This file is part of the OpENCview project, a viewer of ENC
    Copyright (C) 2000-2004  Sylvain Duclos sduclos@users.sourceforgue.net

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
   USA.
*/

#ifndef __S52UTILS_H__
#define __S52UTILS_H__

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // precompiled headers

inline int roundint (double x)
{
#ifdef __WXOSX__
    return wxRound(x);     //FS#1278
#else
  int tmp = static_cast<int>(x);
  tmp += (x - tmp >= .5) - (x - tmp <= -.5);
  return tmp;
#endif
}

#ifdef SOLARIS
// well should be cc
#define PRINTF printf(__FILE__ ":%i: : ", __LINE__), printf
#else
#define PRINTF printf(__FILE__ ":%i: %s(): ", __LINE__, __FUNCTION__), printf
#endif

// valid label in .conf file
#define CONF_CATALOG "CATALOG"
#define CONF_PLIB "PLIB"
#define CONF_CHART "CHART"
#define CONF_RGB "DUMMY_RGB_BRIGHT"

// global parameter for mariners' selection
typedef enum _S52_MAR_param_t {
  S52_MAR_NONE = 0,       // default
  S52_MAR_SHOW_TEXT = 1,  // view group 23
  S52_MAR_TWO_SHADES =
      2,  // flag indicating selection of two depth shades (on/off) [default ON]
  S52_MAR_SAFETY_CONTOUR = 3,  // selected safety contour (meters) [IMO PS 3.6]
  S52_MAR_SAFETY_DEPTH = 4,    // selected safety depth (meters) [IMO PS 3.7]
  S52_MAR_SHALLOW_CONTOUR =
      5,  // selected shallow water contour (meters) (optional)
  S52_MAR_DEEP_CONTOUR = 6,     // selected deepwatercontour (meters) (optional)
  S52_MAR_SHALLOW_PATTERN = 7,  // flag indicating selection of shallow water
                                // highlight (on/off)(optional) [default OFF]
  S52_MAR_SHIPS_OUTLINE = 8,  // flag indicating selection of ship scale symbol
                              // (on/off) [IMO PS 8.4]
  S52_MAR_DISTANCE_TAGS =
      9,  // selected spacing of "distance to run" tags at a route (nm)
  S52_MAR_TIME_TAGS =
      10,  // selected spacing of time tags at the pasttrack (min)
  S52_MAR_FULL_SECTORS = 11,    // show full length light sector lines
  S52_MAR_SYMBOLIZED_BND = 12,  // symbolized area boundaries

  S52_MAR_SYMPLIFIED_PNT = 13,  // simplified point

  S52_MAR_DISP_CATEGORY = 14,  // display category

  S52_MAR_COLOR_PALETTE = 15,  // color palette

  S52_MAR_NUM = 16  // number of parameters
} S52_MAR_param_t;

extern double S52_getMarinerParam(S52_MAR_param_t param);
extern int S52_setMarinerParam(S52_MAR_param_t param, double val);
#endif
