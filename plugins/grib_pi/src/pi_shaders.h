/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2017 by David S. Register                               *
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
 **************************************************************************/

#ifndef __PISHADERS_H__
#define __PISHADERS_H__

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // precompiled headers

#ifdef USE_ANDROID_GLES2
#include <GLES2/gl2.h>
#endif

#include <GL/glew.h>
#include <GL/gl.h>
//#include <GL/glu.h>

extern GLint pi_color_tri_shader_program;
extern GLint pi_colorv_tri_shader_program;
extern GLint pi_texture_2D_shader_program;
extern GLint pi_circle_filled_shader_program;

bool pi_loadShaders();
void configureShaders(float width, float height);

#endif
