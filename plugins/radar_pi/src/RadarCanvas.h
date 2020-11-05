/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Radar Plugin
 * Authors:  David Register
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

#ifndef _RADAR_CANVAS_H_
#define _RADAR_CANVAS_H_

#include "TextureFont.h"
#include "radar_pi.h"

PLUGIN_BEGIN_NAMESPACE

const double ZOOM_FACTOR_CENTER = .9;    // On how big a part of the PPI do we draw the radar picture
const double ZOOM_FACTOR_OFFSET = 1.05;  // On how big a part of the PPI do we draw the radar picture

class RadarCanvas : public wxGLCanvas {
 public:
  RadarCanvas(radar_pi* pi, RadarInfo* ri, wxWindow* parent, wxSize size);
  virtual ~RadarCanvas();

  void Render(wxPaintEvent& evt);
  void OnMove(wxMoveEvent& evt);
  void OnSize(wxSizeEvent& evt);
  void OnMouseClickDown(wxMouseEvent& event);
  void OnMouseClickUp(wxMouseEvent& event);
  void OnMouseMotion(wxMouseEvent& event);
  void OnMouseWheel(wxMouseEvent& event);

 private:
  void FillCursorTexture();
  void RenderTexts(const wxSize& location);
  void RenderRangeRingsAndHeading(const wxSize& center, float radius);
  void RenderCursor(const wxSize& clientSize, float radius, double range, double bearing);
  void RenderCursor(const wxSize& clientSize, float radius, const GeoPosition& cursor);
  void RenderCursor(const wxSize& clientSize, float radius);
  void RenderChartCursor(const wxSize& clientSize, float radius);
  void Render_EBL_VRM(const wxSize& clientSize, float radius);
  wxSize RenderControlItem(const wxSize& loc, RadarControlItem& item, ControlType ct, const wxString& name);
  wxSize GetScaledSize(wxSize size);
  int GetScaledSize(int size);

  wxWindow* m_parent;
  radar_pi* m_pi;
  RadarInfo* m_ri;

  wxGLContext* m_context;  // Our GL context
  wxGLContext* m_zero_context;

  TextureFont m_FontNormal;
  TextureFont m_FontBig;
  TextureFont m_FontMenu;
  TextureFont m_FontMenuBold;
  wxSize m_menu_size;
  wxSize m_zoom_size;
  wxPoint m_mouse_down;
  unsigned int m_cursor_texture;

  wxLongLong m_last_mousewheel_zoom_in;
  wxLongLong m_last_mousewheel_zoom_out;

  DECLARE_EVENT_TABLE();
};

PLUGIN_END_NAMESPACE

#endif
