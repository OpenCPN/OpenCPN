/******************************************************************************
 *
 * Project:  OpenCP
 * Purpose:  S52 Utility Library
 * Author:   David Register, Sylvain Duclos
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
 *   bdbcat@yahoo.com   *
 *
 *   Copyright (C) 2000-2004  Sylvain Duclos
 *   sduclos@users.sourceforgue.net
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************
 *
 * $Log: s52utils.cpp,v $
 * Revision 1.9  2010/04/27 01:43:31  bdbcat
 * Build 426
 *
 * Revision 1.8  2009/11/18 01:26:13  bdbcat
 * 1.3.5 Beta 1117
 *
 * Revision 1.7  2009/08/03 03:14:38  bdbcat
 * Cleanup for MSVC
 *
 * Revision 1.6  2008/10/24 16:18:43  bdbcat
 * Remove/inline  isblank()
 *
 * Revision 1.5  2008/08/09 23:58:40  bdbcat
 * Numerous revampings....
 *
 * Revision 1.4  2008/03/30 22:28:43  bdbcat
 * Cleanup
 *
 * Revision 1.3  2008/01/10 03:38:08  bdbcat
 * Update for Mac OSX
 *
 * Revision 1.2  2007/05/03 13:23:56  dsr
 * Major refactor for 1.2.0
 *
 * Revision 1.1.1.1  2006/08/21 05:52:19  dsr
 * Initial import as opencpn, GNU Automake compliant.
 *
 * Revision 1.3  2006/08/04 11:42:02  dsr
 * no message
 *
 * Revision 1.2  2006/05/19 19:25:39  dsr
 * Change Mariner Selections for critical depths
 *
 * Revision 1.1.1.1  2006/04/19 03:23:28  dsr
 * Rename/Import to OpenCPN
 *
 * Revision 1.4  2006/04/19 00:49:48  dsr
 * *** empty log message ***
 *
 * Revision 1.3  2006/03/16 03:08:24  dsr
 * Cleanup tabs
 *
 * Revision 1.2  2006/02/23 01:48:36  dsr
 * Cleanup
 *
 *
 *
 */

//#include "dychart.h"              // comment out because MSVC CRT Mem debug breaks compile???

#include "s52utils.h"
//#include <stdio.h>        // FILE
//#include <string.h>       // strncmp()

//#include <stdlib.h>       // exit()

// configuration file
#define CONF_NAME   "./s52test.conf"



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
double TWO_SHADES      = FALSE;   // flag indicating selection of two depth shades (on/off) [default ON]
double SAFETY_CONTOUR  = 10.0;    // selected safety contour (meters) [IMO PS 3.6]
double SAFETY_DEPTH    = 7.0;     // selected safety depth (meters) [IMO PS 3.7]
double SHALLOW_CONTOUR = 5.0;     // selected shallow water contour (meters) (optional)
double DEEP_CONTOUR    = 30.0;    // selected deepwatercontour (meters) (optional)
*/

/* B) value for testing */
/*
//gboolean TWO_SHADES      = TRUE;     // flag indicating selection of two depth shades (on/off) [default ON]
gboolean TWO_SHADES      = FALSE;    // flag indicating selection of two depth shades (on/off) [default ON]
gboolean SHOW_TEXT       = TRUE;     // view group 23
//double    SAFETY_DEPTH    = 30.0;    // selected safety depth (meters) [IMO PS 3.7]
//double    SHALLOW_CONTOUR = 2.0;     // selected shallow water contour (meters) (optional)
double    SAFETY_DEPTH    = 15.0;    // selected safety depth (meters) [IMO PS 3.7]
double    SHALLOW_CONTOUR = 5.0;     // selected shallow water contour (meters) (optional)
//double    SAFETY_CONTOUR  = 30.0;    // selected safety contour (meters) [IMO PS 3.6]
//double    DEEP_CONTOUR    = 30.0;    // selected deepwatercontour (meters) (optional)
//double    SAFETY_CONTOUR  = 5.0;     // selected safety contour (meters) [IMO PS 3.6]
//double    DEEP_CONTOUR    = 10.0;    // selected deepwatercontour (meters) (optional)
double    SAFETY_CONTOUR  = 10.0;    // selected safety contour (meters) [IMO PS 3.6]
double    DEEP_CONTOUR    = 15.0;    // selected deepwatercontour (meters) (optional)
*/

/* param needed for certain conditional symbology */
/*
gboolean SHALLOW_PATTERN = FALSE;    // flag indicating selection of shallow water highlight (on/off)(optional) [default OFF]
gboolean SHIPS_OUTLINE   = FALSE;    // flag indicating selection of ship scale symbol (on/off) [IMO PS 8.4]
double   DISTANCE_TAGS   = 0.0;      // selected spacing of "distance to run" tags at a route (nm)
double   TIME_TAGS       = 0.0;      // selected spacing of time tags at the pasttrack (min)
gboolean FULL_SECTORS    = TRUE;     // show full length light sector lines
gboolean SYMBOLIZED_BND  = TRUE;     // symbolized area boundaries
*/

// WARNING: must be in sync with S52_MAR_param_t
/*
static char *_MARparamNm[]  = {
    "S52_MAR_NONE",             //= 0,    // default
    "S52_MAR_SHOW_TEXT",        //= 1,    // view group 23
    "S52_MAR_TWO_SHADES",       //= 2,    // flag indicating selection of two depth shades (on/off) [default ON]
    "S52_MAR_SAFETY_CONTOUR",   //= 3,    // selected safety contour (meters) [IMO PS 3.6]
    "S52_MAR_SAFETY_DEPTH",     //= 4,    // selected safety depth (meters) [IMO PS 3.7]
    "S52_MAR_SHALLOW_CONTOUR",  //= 5,    // selected shallow water contour (meters) (optional)
    "S52_MAR_DEEP_CONTOUR",     //= 6,    // selected deepwatercontour (meters) (optional)
    "S52_MAR_SHALLOW_PATTERN",  //= 7,    // flag indicating selection of shallow water highlight (on/off)(optional) [default OFF]
    "S52_MAR_SHIPS_OUTLINE",    //= 8,    // flag indicating selection of ship scale symbol (on/off) [IMO PS 8.4]
    "S52_MAR_DISTANCE_TAGS",    //= 9,    // selected spacing of "distance to run" tags at a route (nm)
    "S52_MAR_TIME_TAGS",        //= 10,   // selected spacing of time tags at the pasttrack (min)
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
    0.0,      // NONE
    TRUE,     // SHOW_TEXT
    TRUE,     // TWO_SHADES

    8.0,     // SAFETY_CONTOUR
    //0.0,     // SAFETY_CONTOUR  --to test DEPCNT02 selection (GL) in CA49995A.000
    //0.5,     // SAFETY_CONTOUR  --to test DEPCNT02 selection (GL) in CA49995A.000

    5.0,      // SAFETY_DEPTH
    //5.0,      // SAFETY_DEPTH

    3.0,      // SHALLOW_CONTOUR
    10.0,     // DEEP_CONTOUR

    //FALSE,    // SHALLOW_PATTERN
    TRUE,    // SHALLOW_PATTERN

    FALSE,    // SHIPS_OUTLINE
    0.0,      // DISTANCE_TAGS
    0.0,      // TIME_TAGS
    TRUE,     // FULL_SECTORS
    TRUE,     // SYMBOLIZED_BND

    TRUE,     // SYMPLIFIED_PNT

//    'D',      // S52_MAR_DISP_CATEGORY --DISPLAYBASE
//    'S',      // S52_MAR_DISP_CATEGORY --STANDARD
    'O',      // S52_MAR_DISP_CATEGORY --OTHER

//    0,        // S52_MAR_COLOR_PALETTE --DAY_BRIGHT
    1,        // S52_MAR_COLOR_PALETTE --DAY_BLACKBACK
//    2,        // S52_MAR_COLOR_PALETTE --DAY_WHITEBACK
//    3,        // S52_MAR_COLOR_PALETTE --DUSK
//    4,        // S52_MAR_COLOR_PALETTE --NIGHT

    16.0      // NUM
};

//////////////////////////////////////////////////////////////////
#if 0
int S52_getConfig(const char *label, valueBuf *vbuf)
// return TRUE and string value in vbuf for label, FLASE if fail
{
   FILE *fp;
//   int ret = 0;
   int  ret = 1;
   char lbuf[CPATH_MAX] = {'#'};
   char tmp [CPATH_MAX];
   char frmt[CPATH_MAX];

   fp = fopen(CONF_NAME, "r");
   if (NULL == fp) {
//       PRINTF("ERROR opening " CONF_NAME "\n");
       return 0;
   }

   // prevent buffer overflow
   sprintf(frmt, "%s%i%s", "%s %", MAXL-1, "[^\n]s");
   //printf("frmt:%s\n", frmt);

   while (ret > 0) {
       if (lbuf[0] != '#') {
           if (0 == strncmp(lbuf, label, strlen(label))) {
               //sscanf(tmp, "%255s", *vbuf);
               char *c = tmp;
               while (*c == ' ' || *c == 0x09) c++;
               sscanf(c, "%255[^\n]", *vbuf);
//               PRINTF("label:%s value:%s \n", lbuf, *vbuf);
               fclose(fp);
               return 1;
           }
       }


       //ret = fscanf(fp, frmt, lbuf, vbuf);
       ret = fscanf(fp, "%s%255[^\n]\n", lbuf, tmp);
       //printf("label:%s \n", lbuf);
       //printf("value:%s \n", tmp);
       //printf("ret:%i\n", ret);
   }
   fclose(fp);

   *vbuf[0] = '\0';
   return 0;

}
#endif

double S52_getMarinerParam(S52_MAR_param_t param)
// return Mariner parameter or '0.0' if fail
// FIXME: check mariner param against groups selection
{
//    valueBuf vbuf;

//      DSR
    return _MARparamVal[param];
/*
    if (S52_MAR_NONE<param && param<S52_MAR_NUM)
    {

        if (S52_getConfig(_MARparamNm[param], &vbuf))
            return (float)atof(vbuf);
        else
            return (float)_MARparamVal[param];
    }

    return (float)0.0;
*/
}

int    S52_setMarinerParam(S52_MAR_param_t param, double val)
{
    if (S52_MAR_NONE<param && param<S52_MAR_NUM)
        _MARparamVal[param] = val;
    else
        return FALSE;

    return TRUE;
}
