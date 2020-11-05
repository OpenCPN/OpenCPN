/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Radar Plugin
 * Author:   David Register
 *           Dave Cowell
 *           Kees Verruijt
 *           Douwe Fokkema
 *           Sean D'Epagnier
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register              bdbcat@yahoo.com *
 *   Copyright (C) 2012-2013 by Dave Cowell                                *
 *   Copyright (C) 2012-2016 by Kees Verruijt         canboat@verruijt.net *
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
 */

#include "RadarDraw.h"
#include "RadarDrawShader.h"
#include "RadarDrawVertex.h"

PLUGIN_BEGIN_NAMESPACE

// Factory to generate a particular draw implementation
RadarDraw* RadarDraw::make_Draw(RadarInfo* ri, int draw_method) {
  switch (draw_method) {
    case 0:
      return new RadarDrawVertex(ri);
    case 1:
      return new RadarDrawShader(ri);
    default:
      wxLogError(wxT("radar_pi: unsupported draw method %d"), draw_method);
  }
  return 0;
}

RadarDraw::~RadarDraw() {}

void RadarDraw::GetDrawingMethods(wxArrayString& methods) {
  wxString m[] = {_("Vertex Array"), _("Shader")};

  methods = wxArrayString(ARRAY_SIZE(m), m);
}

PLUGIN_END_NAMESPACE
