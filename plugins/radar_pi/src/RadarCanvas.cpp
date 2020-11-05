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

#include "RadarCanvas.h"
#include "RadarInfo.h"
#include "TextureFont.h"
#include "drawutil.h"

PLUGIN_BEGIN_NAMESPACE

BEGIN_EVENT_TABLE(RadarCanvas, wxGLCanvas)
EVT_MOVE(RadarCanvas::OnMove)
EVT_SIZE(RadarCanvas::OnSize)
EVT_PAINT(RadarCanvas::Render)
EVT_MOUSEWHEEL(RadarCanvas::OnMouseWheel)
EVT_LEFT_DOWN(RadarCanvas::OnMouseClickDown)
EVT_MOTION(RadarCanvas::OnMouseMotion)
EVT_LEFT_UP(RadarCanvas::OnMouseClickUp)
END_EVENT_TABLE()

static int attribs[] = {WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16, WX_GL_STENCIL_SIZE, 8, 0};

RadarCanvas::RadarCanvas(radar_pi *pi, RadarInfo *ri, wxWindow *parent, wxSize size)
    : wxGLCanvas(parent, wxID_ANY, attribs, wxDefaultPosition, size, wxFULL_REPAINT_ON_RESIZE | wxBG_STYLE_CUSTOM, _T("")) {
  m_parent = parent;
  m_pi = pi;
  m_ri = ri;
  m_context = new wxGLContext(this);
  m_zero_context = new wxGLContext(this);
  m_cursor_texture = 0;
  m_last_mousewheel_zoom_in = 0;
  m_last_mousewheel_zoom_out = 0;

  LOG_VERBOSE(wxT("radar_pi: %s create OpenGL canvas"), m_ri->m_name.c_str());
  Refresh(false);
}

RadarCanvas::~RadarCanvas() {
  LOG_VERBOSE(wxT("radar_pi: %s destroy OpenGL canvas"), m_ri->m_name.c_str());
  delete m_context;
  delete m_zero_context;
  if (m_cursor_texture) {
    glDeleteTextures(1, &m_cursor_texture);
    m_cursor_texture = 0;
  }
}

void RadarCanvas::OnSize(wxSizeEvent &evt) {
  wxSize parentSize = m_parent->GetSize();
  LOG_DIALOG(wxT("radar_pi: %s resize OpenGL canvas to %d, %d"), m_ri->m_name.c_str(), parentSize.x, parentSize.y);
  Refresh(false);
  if (GetSize() != parentSize) {
    SetSize(parentSize);
  }
}

void RadarCanvas::OnMove(wxMoveEvent &evt) {
  wxPoint pos = m_parent->GetPosition();
  LOG_DIALOG(wxT("radar_pi: %s move OpenGL canvas to %d, %d"), m_ri->m_name.c_str(), pos.x, pos.y);
}

wxSize RadarCanvas::GetScaledSize(wxSize size)
{
#ifdef __WXGTK3__
  return size * GetContentScaleFactor();
#else
  return size;
#endif
}

int RadarCanvas::GetScaledSize(int size)
{
#ifdef __WXGTK3__
  return size * GetContentScaleFactor();
#else
  return size;
#endif
}

void RadarCanvas::RenderTexts(const wxSize &loc) {
  int x, y;
  int menu_x;
  wxString s;
  RadarState state = (RadarState)m_ri->m_state.GetValue();

#define MENU_ROUNDING 4
#define MENU_BORDER 8
#define MENU_EXTRA_WIDTH 32

  // Draw Menu in the top right

  s = _("Menu");
  if (m_pi->m_settings.dock_radar[m_ri->m_radar]) {
    s = _("Menu ") + m_ri->m_name;
  }
  m_FontMenu.GetTextExtent(s, &x, &y);
  menu_x = x;

  // Calculate the size of the rounded rect, this is also where you can 'click'...
  m_menu_size.x = x + 2 * (MENU_BORDER + MENU_EXTRA_WIDTH);
  m_menu_size.y = y + 2 * (MENU_BORDER);

  if (state != RADAR_OFF) {
    glColor4ub(40, 40, 100, 128);

    DrawRoundRect(loc.GetWidth() - m_menu_size.x, 0, m_menu_size.x, m_menu_size.y, 4);

    glColor4ub(100, 255, 255, 255);
    // The Menu text is slightly inside the rect
    m_FontMenu.RenderString(s, loc.GetWidth() - m_menu_size.x + MENU_BORDER + MENU_EXTRA_WIDTH, MENU_BORDER);

    // Draw - + in mid bottom

    s = wxT("  -   + ");
    m_FontMenuBold.GetTextExtent(s, &x, &y);

    // Calculate the size of the rounded rect, this is also where you can 'click'...
    m_zoom_size.x = x + 2 * (MENU_BORDER);
    m_zoom_size.y = y + 2 * (MENU_BORDER);

    glColor4ub(80, 80, 80, 128);

    DrawRoundRect(loc.GetWidth() / 2 - m_zoom_size.x / 2, loc.GetHeight() - m_zoom_size.y + MENU_ROUNDING, m_zoom_size.x,
                  m_zoom_size.y, MENU_ROUNDING);

    glColor4ub(200, 200, 200, 255);
    // The -+ text is slightly inside the rect
    m_FontMenuBold.RenderString(s, loc.GetWidth() / 2 - m_zoom_size.x / 2 + MENU_BORDER,
                                loc.GetHeight() - m_zoom_size.y + MENU_BORDER);
  }

  glColor4ub(200, 255, 200, 255);
  s = m_ri->GetCanvasTextTopLeft();
  m_FontBig.RenderString(s, 0, 0);

  s = m_ri->GetCanvasTextBottomLeft();
  if (s.length()) {
    m_FontBig.GetTextExtent(s, &x, &y);
    m_FontBig.RenderString(s, 0, loc.GetHeight() - y);
  }

  s = m_ri->GetCanvasTextCenter();
  if (s.length()) {
    m_FontBig.GetTextExtent(s, &x, &y);
    m_FontBig.RenderString(s, (loc.GetWidth() - x) / 2, (loc.GetHeight() - y) / 2);
  }

  if (state != RADAR_OFF) {
    wxSize i;
    i.x = loc.GetWidth() - 5 - menu_x / 2;
    i.y = loc.GetHeight() - 5;

    i = RenderControlItem(i, m_ri->m_rain, CT_RAIN, _("Rain"));
    i.y -= 5;
    i = RenderControlItem(i, m_ri->m_sea, CT_SEA, _("Sea"));
    i.y -= 5;
    i = RenderControlItem(i, m_ri->m_gain, CT_GAIN, _("Gain"));
  }
}

/*
 * Receives bottom mid part of canvas to draw, returns back top mid
 */
wxSize RadarCanvas::RenderControlItem(const wxSize &loc, RadarControlItem &item, ControlType ct, const wxString &name) {
  if (!m_ri->m_control_dialog || m_ri->m_control_dialog->m_ctrl[ct].type == CT_NONE) {
    return loc;
  }

  ControlInfo ci = m_ri->m_control_dialog->m_ctrl[ct];
  int tx, ty;
  int state = item.GetState();
  int value = item.GetValue();
  wxString label;

  switch (item.GetState()) {
    case RCS_OFF:
      glColor4ub(100, 100, 100, 255);  // Grey
      label << _("Off");
      value = -1;
      break;

    case RCS_MANUAL:
      glColor4ub(255, 100, 100, 255);  // Reddish
      label.Printf(wxT("%d"), value);
      break;

    default:
      glColor4ub(200, 255, 200, 255);  // Greenish
      if (ci.autoNames && state > RCS_MANUAL && state <= RCS_MANUAL + ci.autoValues) {
        label
            << ci.autoNames[state - RCS_AUTO_1];  // A little shorter than in the control, but here we have colour to indicate Auto.
      } else {
        label << _("Auto");
      }
      if (!m_ri->m_showManualValueInAuto) {
        value = -1;
      }
      break;
  }

  m_FontNormal.GetTextExtent(label, &tx, &ty);
  wxSize where = loc;
  where.y -= ty;
  m_FontNormal.RenderString(label, loc.GetWidth() - tx / 2, where.y);

  m_FontNormal.GetTextExtent(name, &tx, &ty);
  where.y -= ty;
  m_FontNormal.RenderString(name, loc.GetWidth() - tx / 2, where.y);

  // Draw a semi circle, 270 degrees when 100%
  if (value > 0) {
    glLineWidth(2.0);
    DrawArc(where.x, where.y + ty, ty + 3, (float)deg2rad(-225), (float)deg2rad(value * 270. / ci.maxValue), value / 2);
  }
  return where;
}

void RadarCanvas::RenderRangeRingsAndHeading(const wxSize &clientSize, float r) {
  // Max range ringe
  // Size of rendered string in pixels
  glPushMatrix();
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  double heading = 180.;
  if (m_pi->GetHeadingSource() != HEADING_NONE) {
    switch (m_ri->GetOrientation()) {
      case ORIENTATION_HEAD_UP:
        heading += 0.;
        m_ri->m_predictor = 0.;  // predictor in the direction of the line on the radar window
        break;
      case ORIENTATION_STABILIZED_UP:
        heading += m_ri->m_course;
        m_ri->m_predictor = m_pi->GetHeadingTrue() - m_ri->m_course;
        break;
      case ORIENTATION_NORTH_UP:
        m_ri->m_predictor = m_pi->GetHeadingTrue();
        break;
      case ORIENTATION_COG_UP:
        heading += m_pi->GetCOG();
        m_ri->m_predictor = m_pi->GetHeadingTrue() - heading - 180.;
        break;
    }
  } else {
    m_ri->m_predictor = 0.;
  }

  glTranslated(m_ri->m_off_center.x + m_ri->m_drag.x, m_ri->m_off_center.y + m_ri->m_drag.y, 0.);
  glColor3ub(0, 126, 29);  // same color as HDS
  glLineWidth(1.0);

  int meters = m_ri->m_range.GetValue();
  int rings = 1;

  if (meters > 0) {
    // Instead of computing various modulo we just check which ranges
    // result in a non-empty range string.
    // We try 3/4th, 2/3rd, 1/2, falling back to 1 ring = no subrings

    for (rings = 4; rings > 1; rings--) {
      wxString s = m_ri->GetDisplayRangeStr(meters * (rings - 1) / rings, false);
      if (s.length() > 0) {
        break;
      }
    }
  }

  float x = sinf((float)(0.25 * PI)) * r / (double)rings;
  float y = cosf((float)(0.25 * PI)) * r / (double)rings;
  float x1 = 0;
  float y1 = 0;
  if (m_ri->m_off_center.y > 10) {
    y = -y;    // position text opposite the direction of off-center
    y1 = -16;  // additional offset to position text outside the ring
  }
  if (m_ri->m_off_center.x > 10) {
    x = -x;
    x1 = -16;  // additional offset to position text outside the ring
  }
  // Position of the range texts
  float center_x = clientSize.GetWidth() / 2.0;
  float center_y = clientSize.GetHeight() / 2.0;
  int px;
  int py;

  for (int i = 1; i <= rings; i++) {
    DrawArc(center_x, center_y, r * i / (double)rings, 0.0, 2.0 * (float)PI, 360);
    if (meters != 0) {
      wxString s = m_ri->GetDisplayRangeStr(meters * i / rings, false);
      if (s.length() > 0) {
        m_FontNormal.RenderString(s, center_x + x1 + x * i, center_y + y1 + y * i);
      }
    }
  }

  // if (m_pi->GetHeadingSource() != HEADING_NONE) {

  x = sinf((float)deg2rad(m_ri->m_predictor));
  y = -cosf((float)deg2rad(m_ri->m_predictor));
  glLineWidth(1.0);

  glBegin(GL_LINES);
  glVertex2f(center_x, center_y);
  glVertex2f(center_x + x * r * 2, center_y + y * r * 2);

  for (int i = 0; i < 360; i += 10) {
    x = -sinf(deg2rad(i - heading)) * (r * 1.00);
    y = cosf(deg2rad(i - heading)) * (r * 1.00);

    // draw a little 'tick' outward from the outermost range circle (which is already drawn)
    glVertex2f(center_x + x, center_y + y);
    glVertex2f(center_x + x * 1.02, center_y + y * 1.02);
  }
  glEnd();
  for (int i = 0; i < 360; i += 30) {
    x = -sinf(deg2rad(i - heading)) * (r * 1.00 - 1);
    y = cosf(deg2rad(i - heading)) * (r * 1.00 - 1);

    wxString s;
    if (i % 90 == 0 && (m_pi->GetHeadingSource() != HEADING_NONE)) {
      static char nesw[4] = {'N', 'E', 'S', 'W'};
      s = wxString::Format(wxT("%c"), nesw[i / 90]);
    } else {
      s = wxString::Format(wxT("%u"), i);
    }

    m_FontNormal.GetTextExtent(s, &px, &py);
    if (x > 0) {
      x -= px;
    }
    if (y > 0) {
      y -= py;
    }
    m_FontNormal.RenderString(s, center_x + x, center_y + y);
  }
  //}

  glPopAttrib();
  glPopMatrix();
}

void RadarCanvas::FillCursorTexture() {
#define CURSOR_WIDTH 16
#define CURSOR_HEIGHT 16

  // clang-format off
  const static char *cursor[CURSOR_HEIGHT] = {
    "................",
    "......*****.....",
    "......*---*.....",
    "......*---*.....",
    "......*---*.....",
    "..*****---*****.",
    "..*-----------*.",
    "..*-----------*.",
    "..*-----------*.",
    "..*****---*****.",
    "......*---*.....",
    "......*---*.....",
    "......*---*.....",
    "......*****.....",
    "................",
    "................",
  };
  // clang-format on

  GLubyte cursorTexture[CURSOR_HEIGHT][CURSOR_WIDTH][4];
  GLubyte *loc;
  int s, t;

  /* Setup RGB image for the texture. */
  loc = (GLubyte *)cursorTexture;
  for (t = 0; t < CURSOR_HEIGHT; t++) {
    for (s = 0; s < CURSOR_WIDTH; s++) {
      switch (cursor[t][s]) {
        case '*': /* White */
          loc[0] = 0xff;
          loc[1] = 0xff;
          loc[2] = 0xff;
          loc[3] = 0xff;
          break;
        case '-': /* black */
          loc[0] = 0x00;
          loc[1] = 0x00;
          loc[2] = 0x00;
          loc[3] = 0xff;
          break;
        default: /* transparent */
          loc[0] = 0x00;
          loc[1] = 0x00;
          loc[2] = 0x00;
          loc[3] = 0x00;
      }
      loc += 4;
    }
  }

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, cursorTexture);
}

void RadarCanvas::RenderCursor(const wxSize &clientSize, float radius) {
  glColor3f(1.0f, 1.0f, 1.0f);

  if (!isnan(m_ri->m_mouse_vrm)) {
    RenderCursor(clientSize, radius, m_ri->m_mouse_vrm * 1852., m_ri->m_mouse_ebl[m_ri->GetOrientation()]);
  } else {
    RenderCursor(clientSize, radius, m_ri->m_mouse_pos);
  }
}

void RadarCanvas::RenderChartCursor(const wxSize &clientSize, float radius) {
  glColor3f(0.0f, 1.0f, 1.0f);

  RenderCursor(clientSize, radius, m_pi->m_cursor_pos);
}

void RadarCanvas::RenderCursor(const wxSize &clientSize, float radius, const GeoPosition &cursor) {
  float distance;
  float bearing;
  GeoPosition pos;

  if (isnan(cursor.lat) || isnan(cursor.lon) || !m_ri->GetRadarPosition(&pos)) {
    return;
  }
  // Can't compute this upfront, ownship may move...
  distance = local_distance(pos, cursor) * 1852.;
  bearing = local_bearing(pos, cursor);
  if (m_ri->GetOrientation() != ORIENTATION_NORTH_UP) {
    bearing -= m_pi->GetHeadingTrue();
  }
  RenderCursor(clientSize, radius, distance, bearing);
}

void RadarCanvas::RenderCursor(const wxSize &clientSize, float radius, double distance, double bearing) {
  int display_range = m_ri->GetDisplayRange();
  double range = distance * radius / display_range;

#define CURSOR_SCALE 1

  double center_x = clientSize.GetWidth() / 2.0;
  double center_y = clientSize.GetHeight() / 2.0;
  double angle = deg2rad(bearing);
  double x = center_x + sin(angle) * range - CURSOR_WIDTH * CURSOR_SCALE / 2;
  double y = center_y - cos(angle) * range - CURSOR_WIDTH * CURSOR_SCALE / 2;

  if (!m_cursor_texture) {
    glGenTextures(1, &m_cursor_texture);
    glBindTexture(GL_TEXTURE_2D, m_cursor_texture);
    FillCursorTexture();
    LOG_DIALOG(wxT("radar_pi: generated cursor texture # %u"), m_cursor_texture);
  }

  // Drawing in existing color
  glBindTexture(GL_TEXTURE_2D, m_cursor_texture);
  glBegin(GL_QUADS);
  glTexCoord2i(0, 0);
  glVertex2i(x, y);
  glTexCoord2i(1, 0);
  glVertex2i(x + CURSOR_SCALE * CURSOR_WIDTH, y);
  glTexCoord2i(1, 1);
  glVertex2i(x + CURSOR_SCALE * CURSOR_WIDTH, y + CURSOR_SCALE * CURSOR_HEIGHT);
  glTexCoord2i(0, 1);
  glVertex2i(x, y + CURSOR_SCALE * CURSOR_HEIGHT);
  glEnd();
}

void RadarCanvas::Render_EBL_VRM(const wxSize &clientSize, float radius) {
  static const uint8_t rgb[BEARING_LINES][3] = {{22, 129, 154}, {45, 255, 254}};

  float center_x = clientSize.GetWidth() / 2.0;
  float center_y = clientSize.GetHeight() / 2.0;
  int display_range = m_ri->GetDisplayRange();
  int orientation = m_ri->GetOrientation();

  glPushMatrix();
  glTranslated(m_ri->m_off_center.x + m_ri->m_drag.x, m_ri->m_off_center.y + m_ri->m_drag.y, 0.);

  for (int b = 0; b < BEARING_LINES; b++) {
    float x, y;
    glColor3ubv(rgb[b]);
    glLineWidth(1.0);
    if (!isnan(m_ri->m_vrm[b])) {
      float scale = m_ri->m_vrm[b] * 1852.0 * radius / display_range;
      if (m_ri->m_ebl[orientation][b] != nanl("")) {
        float angle = (float)deg2rad(m_ri->m_ebl[orientation][b]);
        x = center_x + sinf(angle) * radius * 2.;
        y = center_y - cosf(angle) * radius * 2.;
        glBegin(GL_LINES);
        glVertex2f(center_x, center_y);
        glVertex2f(x, y);
        glEnd();
      }
      DrawArc(center_x, center_y, scale, 0.f, 2.f * (float)PI, 360);
    }
  }
  glPopMatrix();
}

static void ResetGLViewPort(const wxSize &size) {
  glViewport(0, 0, size.GetWidth(), size.GetHeight());
  glMatrixMode(GL_PROJECTION);  // Next two operations on the project matrix stack
  glLoadIdentity();             // Reset projection matrix stack
  glOrtho(0, size.GetWidth(), size.GetHeight(), 0, -1, 1);
  glMatrixMode(GL_MODELVIEW);  // Reset matrick stack target back to GL_MODELVIEW
}

void RadarCanvas::Render(wxPaintEvent &evt) {
  if (!IsShown() || !m_pi->IsInitialized()) {
    return;
  }

  const wxSize clientSize = GetScaledSize(GetClientSize());
  wxPaintDC(this);  // only to be used in paint events. use wxClientDC to paint
                    // outside the paint event

  if (!m_pi->IsOpenGLEnabled()) {
    return;
  }
  LOG_VERBOSE(wxT("radar_pi: %s render OpenGL canvas %d by %d "), m_ri->m_name.c_str(), clientSize.GetWidth(),
              clientSize.GetHeight());
  double look_forward_dist = (double)wxMax(clientSize.GetWidth(), clientSize.GetHeight()) * ZOOM_FACTOR_OFFSET / 4.;

  if (m_ri->m_view_center.GetValue() == CENTER_VIEW) {
    m_ri->m_panel_zoom = ZOOM_FACTOR_CENTER;  // if off center with the button, fixed zoom
  }
  if (m_ri->m_view_center.GetValue() > CENTER_VIEW) {
    m_ri->m_panel_zoom = ZOOM_FACTOR_OFFSET;  // if off center with the button, fixed zoom
  }
  float radar_radius = (float)wxMax(clientSize.GetWidth(), clientSize.GetHeight()) * (float)m_ri->m_panel_zoom / 2.0;

  m_ri->m_radar_radius = radar_radius;

  SetCurrent(*m_context);

  glPushMatrix();
  glPushAttrib(GL_ALL_ATTRIB_BITS);

  wxFont font = GetOCPNGUIScaledFont_PlugIn(_T("StatusBar"));
  font.SetPointSize(GetScaledSize(font.GetPointSize()));
  m_FontNormal.Build(font);

  font = GetOCPNGUIScaledFont_PlugIn(_T("Dialog"));
  font.SetPointSize(GetScaledSize(font.GetPointSize() + 2));
  font.SetWeight(wxFONTWEIGHT_NORMAL);
  m_FontMenu.Build(font);
  font.SetWeight(wxFONTWEIGHT_BOLD);
  m_FontBig.Build(font);
  font.SetPointSize(font.GetPointSize() * 2); // Zoom text is really big
  m_FontMenuBold.Build(font);

  wxColour bg = M_SETTINGS.ppi_background_colour;
  glClearColor(bg.Red() / 256.0, bg.Green() / 256.0, bg.Blue() / 256.0, bg.Alpha() / 256.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  // Clear the canvas
  glEnable(GL_TEXTURE_2D);                             // Enable textures
  glEnable(GL_COLOR_MATERIAL);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  switch (m_ri->m_view_center.GetValue()) {
    case 0:
      break;

    case 1:  // center view
      m_ri->m_off_center.x = 0;
      m_ri->m_off_center.y = 0;
      break;

    case 2:  // forward view
      m_ri->m_off_center.x = -look_forward_dist * sin(deg2rad(m_ri->m_predictor));
      m_ri->m_off_center.y = look_forward_dist * cos(deg2rad(m_ri->m_predictor));
      break;

    case 3:  // aft view
      m_ri->m_off_center.x = look_forward_dist * sin(deg2rad(m_ri->m_predictor));
      m_ri->m_off_center.y = -look_forward_dist * cos(deg2rad(m_ri->m_predictor));
      break;

    default:
      break;
  }

  // LAYER 1 - RANGE RINGS AND HEADINGS
  ResetGLViewPort(clientSize);
  RenderRangeRingsAndHeading(clientSize, radar_radius);

  PlugIn_ViewPort vp;
  GeoPosition pos;

  if (m_pi->GetHeadingSource() != HEADING_NONE && m_ri->GetRadarPosition(&pos) && m_ri->m_target_on_ppi.GetValue() > 0) {
    // LAYER 2 - AIS AND ARPA TARGETS

    ResetGLViewPort(clientSize);
    glPushMatrix();
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    glTranslated(m_ri->m_off_center.x + m_ri->m_drag.x, m_ri->m_off_center.y + m_ri->m_drag.y, 0.);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    vp.m_projection_type = 4;  // Orthographic projection
    int display_range = m_ri->GetDisplayRange();

    switch (m_ri->GetOrientation()) {
      case ORIENTATION_HEAD_UP:
      case ORIENTATION_STABILIZED_UP:
        vp.rotation = deg2rad(-m_pi->GetHeadingTrue());
        break;
      case ORIENTATION_NORTH_UP:
        vp.rotation = 0.;
        break;
      case ORIENTATION_COG_UP:
        vp.rotation = deg2rad(-m_pi->GetCOG());
        break;
    }

    vp.view_scale_ppm = radar_radius / display_range;
    vp.skew = 0.;
    vp.pix_width = clientSize.GetWidth();
    vp.pix_height = clientSize.GetHeight();
    vp.clat = pos.lat;
    vp.clon = pos.lon;

    wxString aisTextFont = _("AIS Target Name");
    wxFont *aisFont = GetOCPNScaledFont_PlugIn(aisTextFont, 12);
    wxColour aisFontColor = GetFontColour_PlugIn(aisTextFont);

    if (aisFont) {
      wxColour newFontColor = M_SETTINGS.ais_text_colour;
      PlugInSetFontColor(aisTextFont, newFontColor);
      newFontColor = GetFontColour_PlugIn(aisTextFont);
    }
    PlugInAISDrawGL(this, vp);
    if (aisFont) {
      PlugInSetFontColor(aisTextFont, aisFontColor);
    }

    glPopAttrib();
    glPopMatrix();
  }

  // LAYER 3 - EBL & VRM

  ResetGLViewPort(clientSize);
  Render_EBL_VRM(clientSize, radar_radius);

  // LAYER 4 - RADAR RETURNS
  glViewport(0, 0, clientSize.GetWidth(), clientSize.GetHeight());
  glMatrixMode(GL_PROJECTION);  // Next two operations on the project matrix stack
  glLoadIdentity();             // Reset projection matrix stack
  if (clientSize.GetWidth() >= clientSize.GetHeight()) {
    glScaled(1.0, (float)-clientSize.GetWidth() / clientSize.GetHeight(), 1.0);
  } else {
    glScaled((float)clientSize.GetHeight() / clientSize.GetWidth(), -1.0, 1.0);
  }
  glMatrixMode(GL_MODELVIEW);  // Reset matrick stack target back to GL_MODELVIEW

  m_ri->RenderRadarImage1(wxPoint(0, 0), m_ri->m_panel_zoom / m_ri->m_range.GetValue(), 0.0, false);

  // LAYER 5 - TEXTS & CURSOR
  ResetGLViewPort(clientSize);
  RenderTexts(clientSize);
  glPushMatrix();

  glTranslated(m_ri->m_off_center.x + m_ri->m_drag.x, m_ri->m_off_center.y + m_ri->m_drag.y, 0.);
  RenderCursor(clientSize, radar_radius);
  RenderChartCursor(clientSize, radar_radius);  // dynamic cursor coupled to chart cursor
  glPopMatrix();

  glPopAttrib();
  glPopMatrix();

  // Do the swapbuffers first, before restoring the context. If we don't then various artifacts
  // occur on MacOS with the radar PPI window getting completely distorted.
  // Also it seems much more logical to call SwapBuffers() *before* going back to the OpenCPN
  // context.
  SwapBuffers();

  // Restore the OpenGL context, so that AIS rollover doesn't break.
  // Apparently this is executed in a timer on Windows and Linux in such
  // a way that it is using the radar's OpenGL context.
  //
  // This used to break on Mac, where timing is different and we are not
  // allowed to set the context from a class that is derived from a window
  // where the context is for a different window, but with wx 3.1.2 + OpenCPN
  // patches this seems to be fixed.
  wxGLContext *chart_context = m_pi->GetChartOpenGLContext();
  if (chart_context) {
    SetCurrent(*chart_context);
  } else {
    SetCurrent(*m_zero_context);  // Make sure OpenCPN -at least- doesn't overwrite our context info
  }
}

void RadarCanvas::OnMouseMotion(wxMouseEvent &event) {
  int x, y;
  if (event.Dragging()) {
    event.GetPosition(&x, &y);
    m_ri->m_drag.x = (x - m_mouse_down.x);
    m_ri->m_drag.y = (y - m_mouse_down.y);
  }
  event.Skip();
}

void RadarCanvas::OnMouseClickUp(wxMouseEvent &event) {
  int x, y, w, h;
  event.GetPosition(&x, &y);
  if (abs(x - m_mouse_down.x) > 10 || abs(y - m_mouse_down.y) > 10) {
    m_ri->m_off_center.x += (x - m_mouse_down.x);
    m_ri->m_off_center.y += (y - m_mouse_down.y);
    m_ri->m_drag.x = 0;
    m_ri->m_drag.y = 0;
    m_ri->m_view_center.Update(0);
  } else {
    x = m_mouse_down.x;
    y = m_mouse_down.y;
    GetClientSize(&w, &h);

    int center_x = w / 2;
    int center_y = h / 2;

    // double offset = (double)wxMax(w, h) * m_ri->m_radar_radius / 2.;  // half of the radar_radius
    center_x += (m_ri->m_off_center.x + m_ri->m_drag.x);  // horizontal
    center_y += (m_ri->m_off_center.y + m_ri->m_drag.y);
    LOG_DIALOG(wxT("radar_pi: %s Mouse clicked at %d, %d"), m_ri->m_name.c_str(), x, y);
    if (x > 0 && x < w && y > 0 && y < h) {
      if (x >= w - m_menu_size.x && y < m_menu_size.y) {
        m_pi->ShowRadarControl(m_ri->m_radar, true);
      } else if ((x >= center_x - m_zoom_size.x / 2) && (x <= center_x + m_zoom_size.x / 2) &&
                 (y > h - m_zoom_size.y + MENU_ROUNDING)) {
        if (x < center_x) {
          m_ri->AdjustRange(+1);
        } else {
          m_ri->AdjustRange(-1);
        }

      } else {
        double delta_x = x - center_x;
        double delta_y = y - center_y;

        double distance = sqrt(delta_x * delta_x + delta_y * delta_y);
        int display_range = m_ri->GetDisplayRange();

        double angle = fmod(rad2deg(atan2(delta_y, delta_x)) + 720. + 90., 360.0);

        double full_range = m_ri->m_panel_zoom * wxMax(w, h) / 2.0;

        double range = distance / (1852.0 * full_range / display_range);

        LOG_VERBOSE(wxT("radar_pi: cursor in PPI at angle=%.1fdeg range=%.2fnm"), angle, range);
        m_ri->SetMouseVrmEbl(range, angle);
      }
    }
  }
  event.Skip();
}

void RadarCanvas::OnMouseClickDown(wxMouseEvent &event) {
  event.GetPosition(&m_mouse_down.x, &m_mouse_down.y);
  event.Skip();
}

#define ZOOM_TIME_RANGE 333  // 3 zooms per second
#define ZOOM_TIME_LOCAL 50   // 20 zooms per second
#define ZOOM_SENSITIVITY 0   // Increase to make less sensitive

void RadarCanvas::OnMouseWheel(wxMouseEvent &event) {
  // int delta = event.GetWheelDelta();
  int rotation = event.GetWheelRotation();

  wxLongLong now = wxGetUTCTimeMillis();

  //  LOG_INFO(wxT("radar_pi: %s Mouse range wheel %d / %d"), m_ri->m_name.c_str(), rotation, delta);

  if (rotation) {
    if (m_pi->m_settings.reverse_zoom) {
      rotation = -rotation;
    }
    double zoom_time = m_ri->m_view_center.GetValue() ? ZOOM_TIME_RANGE : ZOOM_TIME_LOCAL;
    if (rotation > ZOOM_SENSITIVITY && m_last_mousewheel_zoom_in < now - zoom_time) {
      LOG_VERBOSE(wxT("radar_pi: %s Mouse zoom in"), m_ri->m_name.c_str());
      if (m_ri->m_view_center.GetValue()) {
        m_ri->AdjustRange(+1);
      } else {
        m_ri->m_panel_zoom *= 1.1;
        m_ri->m_off_center.x *= 1.1;
        m_ri->m_off_center.y *= 1.1;
      }
      m_last_mousewheel_zoom_in = now;
    } else if (rotation < -1 * ZOOM_SENSITIVITY && m_last_mousewheel_zoom_out < now - zoom_time) {
      LOG_INFO(wxT("radar_pi: %s Mouse zoom out"), m_ri->m_name.c_str());
      if (m_ri->m_view_center.GetValue()) {
        m_ri->AdjustRange(-1);
      } else {
        m_ri->m_panel_zoom /= 1.1;
        m_ri->m_off_center.x /= 1.1;
        m_ri->m_off_center.y /= 1.1;
      }
      m_last_mousewheel_zoom_out = now;
    }
  }
}

PLUGIN_END_NAMESPACE
