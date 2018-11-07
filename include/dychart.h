/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Global Build Options
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************
 *
 */

//----------------------------------------------------------------------------------
//          Global Build options for opencpn
//----------------------------------------------------------------------------------

#ifndef _DYCHART_H_
#define _DYCHART_H_

//    Profiling support

//#include "/usr/include/valgrind/callgrind.h"

//  Chart cacheing policy defaults

#define CACHE_N_LIMIT_DEFAULT 20          // Cache no more than n charts

#define CACHE_MEM_LIMIT_DEFAULT 0       // Application memory useage target, kBytes



//          If defined, update the system time using GPS receiver data.
//          Time update is applied if the system time and GPS time differ
//          by more than one minute, and only once per session.
//          On Linux, this option requires root privileges, obtained by sudo.
//          Thus, the following line is required in etc/sudoers:
//
//              nav ALL=NOPASSWD:/bin/date -s *
//
//          Where "nav" is the user's user name.
//
//          Also, the opencpn configuration file must contain the key
//          [Settings]
//              SetSystemTime=1
//          For security, this option is not available on the "Options" dialog
#define ocpnUPDATE_SYSTEM_TIME


//------------------------------------------------------------------------------
//          Some private, app global type definitions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//          Various compile time options
//------------------------------------------------------------------------------

#ifdef __MSVC__
#pragma warning(disable:4114)
#pragma warning(disable:4284)             // This one is to do with "reverse iterator UDT..." Huh?


#endif

//    Following definition required by GDAL
#define notdef 1

#ifdef __MSVC__
//    __MSVC__ randomly does not link snprintf, or _snprintf
//    Replace it with a local version, code is in cutil.c
#define snprintf mysnprintf
#endif

//------------------------------------------------------------------------------
// Some Portable math definitions
//------------------------------------------------------------------------------

//    Floating Point Max/Min

#ifndef __max
      #define __max(a,b)  (((a) > (b)) ? (a) : (b))
#endif

#ifndef __min
      #define __min(a,b)  (((a) < (b)) ? (a) : (b))
#endif

#ifdef __MSVC__
      #define fmin __min
      #define fmax __max

      #define round(x) round_msvc(x)

#endif

//------------------------------------------------------------------------------
//          Some Build constants
//------------------------------------------------------------------------------

//    Home Base, used if the config file lat/lon seems bogus or missing

//#define START_LAT   35.1025              // New Bern (Ver 1.0)
//#define START_LON  -77.0342

//#define START_LAT   26.783               // Green Turtle Key  (Ver 1.2)
//#define START_LON  -77.333

//#define START_LAT   25.786               //  Miami Beach (Ver 1.2.2)
//#define START_LON  -80.148

#define START_LAT   33.358               //  Georgetown, SC (Ver 1.2.4)
#define START_LON  -79.282

//------------------------------------------------------------------------------
//          Some MSW and MSVCRT Specific Includes
//------------------------------------------------------------------------------
#ifdef __WXMSW__
    #include "wx/msw/private.h"
#endif

//------------------------------------------------------------------------------
//          Some Memory Leak Detection Code
//------------------------------------------------------------------------------



#ifdef __MSVC__
        #ifdef _DEBUG
            #define _CRTDBG_MAP_ALLOC
            #include <crtdbg.h>
            #define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__ )
            #define new DEBUG_NEW
        #endif
#endif


//----------------------------------------------------------------------------
//          Environment Access functions
//----------------------------------------------------------------------------
#ifdef __MSVC__
#define _putenv _putenv       // This is for MSVC
#else
#define _putenv putenv        // This is for other Windows compiler
#endif

//----------------------------------------------------------------------------
//              Use and compile for S57 ENCs?
//----------------------------------------------------------------------------
//      Linux ./configure utility defines USE_S57 by default

//----------------------------------------------------------------------------
//              Use the CPL Portability library only if S57 is enabled
//----------------------------------------------------------------------------

#ifdef USE_S57
#define USE_CPL
#include "mygdal/cpl_port.h"
#endif

#ifndef NULL
#define NULL 0
#endif



/***********************************************************************
 * Define __POSIX__ to imply posix thread model compatibility
 * Especially used for communication port multithreading.
 *
 * Posix thread model is available on selected platforms, see code.
 */

#ifdef __POSIX__
#undef __POSIX__
#endif

#ifdef  __WXOSX__
#define __POSIX__
#endif

#ifdef  __WXGTK__
#define __POSIX__
#endif

#ifndef OCPN_GL_INCLUDES
#define OCPN_GL_INCLUDES 1


#ifdef __WXMSW__
    #include "GL/gl.h"            // local copy for Windows
    #include "GL/glu.h"
#else
    #ifndef __OCPN__ANDROID__
        #include <GL/gl.h>
        #include <GL/glu.h>
        #include <GL/glext.h>
    #else
        #include <qopengl.h>
        #include <GL/gl_private.h>              // this is a cut-down version of gl.h
                                                // which allows use of gl functions with gles2 headers
                                                // to be included as well, and avoids colisions.
    #endif

#endif

#endif      //OCPN_GL_INCLUDES

#ifdef __OCPN__ANDROID__
#include "qdebug.h"
#endif

#endif      // __FILE__
