/**************************************************************************
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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

/**
 * \file
 *
 * Wrapper for creating a ChartCtx based on global vars
 */

#ifndef _CHART_CTX_FACTORY_H__
#define _CHART_CTX_FACTORY_H__

#include "model/config_vars.h"
#include "s52plib.h"

#ifdef ocpnUSE_GL
#include "gl_headers.h"
#endif

#ifdef ocpnUSE_GL

extern GLenum g_texture_rectangle_format;

/** Return a ChartCtx reflecting caller's opengl context */
static ChartCtx ChartCtxFactory() {
  return ChartCtx(g_bopengl, g_texture_rectangle_format);
}
#else

/** Return a ChartCtx reflecting caller's context not using opengl */
static ChartCtx ChartCtxFactory() { return ChartCtx(g_bopengl); }
#endif  // ocpnUSE_GL

#endif  //  _CHART_CTX_FACTORY_H__
