/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Wrapper for creating a ChartCtx based on global vars
 * Author:   Alec Leamas
 *
 ***************************************************************************
 *   Copyright (C) 2023 by Alec Leamas
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
#ifndef _CHART_CTX_FACTORY_H__
#define _CHART_CTX_FACTORY_H__

#include "s52plib.h"

extern bool g_bopengl;

#ifdef ocpnUSE_GL
extern GLenum g_texture_rectangle_format;

/** Return a ChartCtx reflecting caller's opengl context */
static ChartCtx ChartCtxFactory() {
  return ChartCtx(g_bopengl, g_texture_rectangle_format);
}
#else

/** Return a ChartCtx reflecting caller's context not using opengl */
static ChartCtx ChartCtxFactory() { return ChartCtx(g_bopengl); }
#endif

#endif  //  _CHART_CTX_FACTORY_H__
