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

#include "config.h"

#include <cmath>
#include <algorithm>

//    Profiling support

//#include "/usr/include/valgrind/callgrind.h"

//  Chart cacheing policy defaults

#define CACHE_N_LIMIT_DEFAULT 20  // Cache no more than n charts

#define CACHE_MEM_LIMIT_DEFAULT 0  // Application memory useage target, kBytes

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
#pragma warning(disable : 4114)
#pragma warning( \
    disable : 4284)  // This one is to do with "reverse iterator UDT..." Huh?
#endif

//    Following definition required by GDAL
#define notdef 1

#ifdef __MSVC__
//    __MSVC__ randomly does not link snprintf, or _snprintf
//    Replace it with a local version, code is in cutil.c
#undef snprintf
#define snprintf mysnprintf
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

#define START_LAT 33.358  //  Georgetown, SC (Ver 1.2.4)
#define START_LON -79.282

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
#define DEBUG_NEW new (_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif
#endif

//----------------------------------------------------------------------------
//          Environment Access functions
//----------------------------------------------------------------------------
#ifdef __MSVC__
#define _putenv _putenv  // This is for MSVC
#else
#define _putenv putenv  // This is for other Windows compiler
#endif

//----------------------------------------------------------------------------
//              Use the CPL Portability library only if S57 is enabled
//----------------------------------------------------------------------------

#define USE_CPL
//#include "gdal/cpl_port.h"

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

#ifdef __WXOSX__
#define __POSIX__
#endif

#ifdef __WXGTK__
#define __POSIX__
#endif

#if defined(__OCPN__ANDROID__)
 //#include <GLES2/gl2.h>
 #include <qopengl.h>
 #include <GL/gl_private.h>  // this is a cut-down version of gl.h
 #include <GLES2/gl2.h>
#elif defined(__MSVC__)
 #include "glew.h"
#elif defined(__WXOSX__)
 #include <OpenGL/gl.h>
 #include <OpenGL/glu.h>
 typedef void (*  _GLUfuncptr)();
 #define GL_COMPRESSED_RGB_FXT1_3DFX       0x86B0
#elif defined(__WXQT__) || defined(__WXGTK__)
 #include <GL/glew.h>
 #include <GL/glu.h>
#endif

/*
#if defined(_WIN32) || defined(_WIN64)
#  include <gl/glew.h>
#  include <GL/gl.h>
#  include <GL/glu.h>
#elif __APPLE__
#  include "TargetConditionals.h"
#  if (TARGET_OS_IPHONE && TARGET_IPHONE_SIMULATOR) || TARGET_OS_IPHONE
#    include <OpenGLES/ES2/gl.h>
#    include <OpenGLES/ES2/glext.h>
#  else
#    include <OpenGL/gl.h>
#    include <OpenGL/glu.h>
#    include <OpenGL/glext.h>
#  endif
#elif defined(__ANDROID__) || defined(ANDROID)
#  include <GLES2/gl2.h>
#  include <GLES2/gl2ext.h>
#elif defined(__linux__) || defined(__unix__) || defined(__posix__)
#  include <GL/gl.h>
#  include <GL/glu.h>
#  include <GL/glext.h>
#else
#  error platform not supported.
#endif
*/


#ifdef __OCPN__ANDROID__
#include "qdebug.h"
#endif

#endif  // __FILE__
