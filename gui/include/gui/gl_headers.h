/**************************************************************************
 *   Copyright (C) 2025 Alec Leamas                                        *
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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

/**
 * \file
 *
 * Platform independent GL includes
 */

#if defined(__ANDROID__)
#include <qopengl.h>
#include <GL/gl_private.h>  // this is a cut-down version of gl.h
#include <GLES2/gl2.h>

#elif defined(ocpnUSE_GL)
#if defined(_WIN32)
#include <glew.h>

#elif __APPLE__
typedef void (*_GLUfuncptr)();
#define GL_COMPRESSED_RGB_FXT1_3DFX 0x86B0
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>

#elif defined(__linux__)
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>

#else
#error platform not supported.
#endif  // _WIN32
#endif  // ocpnUSE_GL
