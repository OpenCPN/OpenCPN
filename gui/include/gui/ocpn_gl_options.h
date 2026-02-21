/***************************************************************************
 *   Copyright (C) 2013 by David S. Register                               *
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
 * OpenGL runtime options
 */

#ifndef _GUI_OCPN_GL_OPTIONS_H_
#define _GUI_OCPN_GL_OPTIONS_H_

class ocpnGLOptions {
public:
  bool m_bUseAcceleratedPanning;

  /**
   * Controls OpenGL canvas hardware-accelerated panning mode.
   *
   * When true, uses an OpenGL optimization where the chart is rendered to a
   * texture (via FBO - Frame Buffer Object) during panning operations.
   * This texture is then translated/moved using GPU operations rather than
   * redrawing the entire chart for each pan event, significantly improving
   * performance during chart panning.
   *
   * This is separate from standard chart panning which is enabled by default
   * and uses left-click + drag. This flag only controls whether that panning
   * uses hardware acceleration.
   */
  bool m_bUseCanvasPanning;

  bool m_bTextureCompression;
  bool m_bTextureCompressionCaching;

  int m_iTextureDimension;
  int m_iTextureMemorySize;

  bool m_GLPolygonSmoothing;
  bool m_GLLineSmoothing;
};

extern ocpnGLOptions g_GLOptions;  // global instance

#endif  // _GUI_OCPN_GL_OPTIONS_H_
