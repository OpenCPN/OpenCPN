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

#ifndef __SHADERS_H__
#define __SHADERS_H__

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // precompiled headers

#include "dychart.h"

#include <memory>
#include <vector>
#include <fstream>

enum Consts { INFOLOG_LEN = 512 };

extern GLint color_tri_shader_program;
extern GLint texture_2D_shader_program;
extern GLint fade_texture_2D_shader_program;
extern GLint circle_filled_shader_program;
extern GLint FBO_texture_2D_shader_program;
extern GLint texture_2DA_shader_program;

bool loadShaders(int index = 0);
void reConfigureShaders(int index = 0);
void unloadShaders();

#endif
