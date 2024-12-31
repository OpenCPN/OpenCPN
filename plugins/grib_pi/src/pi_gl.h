/***************************************************************************
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
 ***************************************************************************/
/**
 * \file
 * OpenGL Platform Abstraction Layer.
 *
 * Provides consistent OpenGL header inclusion and function access across
 * multiple platforms.
 *
 * This layer ensures consistent OpenGL functionality across different
 * operating systems while handling platform-specific initialization
 * and extensions.
 */
//----------------------------------------------------------------------------------
//          OpenGL Build options for S52PLIB
//----------------------------------------------------------------------------------
#ifndef _PIGL_H_
#define _PIGL_H_

#if defined(__OCPN__ANDROID__)
#include <qopengl.h>
#include <GL/gl_private.h>  // this is a cut-down version of gl.h
#include <GLES2/gl2.h>
#elif defined(__MSVC__)
#include "glew.h"
#elif defined(__WXOSX__)
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
typedef void (*_GLUfuncptr)();
#define GL_COMPRESSED_RGB_FXT1_3DFX 0x86B0
#elif defined(__WXQT__) || defined(__WXGTK__)
#include <GL/glew.h>
#include <GL/glu.h>
#endif

#endif  // __FILE__
