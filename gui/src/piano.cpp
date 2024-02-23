/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Chart Bar Window
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
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
 ***************************************************************************
 *
 *
 */

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // precompiled headers
#include "dychart.h"

#include "model/config_vars.h"
#include "model/cutil.h"
#include "model/wx28compat.h"

#include "chcanv.h"
#include "piano.h"
#include "chartdb.h"
#include "chartbase.h"
#include "styles.h"
#include "ocpndc.h"
#include "OCPNPlatform.h"
#include "color_handler.h"
#include "ocpn_plugin.h"
#include "ocpn_frame.h"

#ifdef __OCPN__ANDROID__
#include "qdebug.h"
#include "androidUTIL.h"
#endif

#ifdef ocpnUSE_GL
#include "glChartCanvas.h"
#endif

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(RectArray);

//------------------------------------------------------------------------------
//    External Static Storage
//------------------------------------------------------------------------------
extern ChartDB *ChartData;
extern ocpnStyle::StyleManager *g_StyleManager;
extern int g_GUIScaleFactor;
extern bool g_bopengl;

extern OCPNPlatform *g_Platform;
extern MyFrame *gFrame;
extern BasePlatform *g_BasePlatform;

//------------------------------------------------------------------------------
//          Piano Window Implementation
//------------------------------------------------------------------------------
BEGIN_EVENT_TABLE(Piano, wxEvtHandler)
EVT_TIMER(PIANO_EVENT_TIMER, Piano::onTimerEvent)
END_EVENT_TABLE()

// Define a constructor
Piano::Piano(ChartCanvas *parent) {
  m_parentCanvas = parent;

  m_index_last = -1;
  m_iactive = -1;

  m_hover_icon_last = -1;
  m_hover_last = -1;
  m_brounded = false;
  m_bBusy = false;
  m_gotPianoDown = false;

  m_nRegions = 0;
  m_width = 0;

  //>    SetBackgroundStyle( wxBG_STYLE_CUSTOM ); // on WXMSW, this prevents
  //flashing on color scheme change

  m_pVizIconBmp = NULL;
  m_pInVizIconBmp = NULL;
  m_pPolyIconBmp = NULL;
  m_pSkewIconBmp = NULL;
  m_pTmercIconBmp = NULL;

  SetColorScheme(GLOBAL_COLOR_SCHEME_RGB);  // default

  m_eventTimer.SetOwner(this, PIANO_EVENT_TIMER);

  m_tex = m_tex_piano_height = 0;
  m_piano_mode = PIANO_MODE_LEGACY;
}

Piano::~Piano() {
  if (m_pInVizIconBmp) delete m_pInVizIconBmp;
  if (m_pPolyIconBmp) delete m_pPolyIconBmp;
  if (m_pSkewIconBmp) delete m_pSkewIconBmp;
  if (m_pTmercIconBmp) delete m_pTmercIconBmp;
  if (m_pVizIconBmp) delete m_pVizIconBmp;
}

void Piano::Paint(int y, wxDC &dc, wxDC *shapeDC) {
  ocpnDC odc(dc);
  Paint(y, odc, shapeDC);
}

void Piano::Paint(int y, ocpnDC &dc, wxDC *shapeDC) {
  if (shapeDC) {
    shapeDC->SetBackground(*wxBLACK_BRUSH);
    shapeDC->SetBrush(*wxWHITE_BRUSH);
    shapeDC->SetPen(*wxWHITE_PEN);
    shapeDC->Clear();
  }

  ocpnStyle::Style *style = g_StyleManager->GetCurrentStyle();
  if (!style->chartStatusWindowTransparent) {
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.SetBrush(m_backBrush);
    dc.DrawRectangle(0, y, m_parentCanvas->GetClientSize().x, GetHeight());
  }

  //    Create the Piano Keys

  int nKeys = m_composite_array.size();

  wxPen ppPen(GetGlobalColor(_T("CHBLK")), 1, wxPENSTYLE_SOLID);
  dc.SetPen(ppPen);

  for (int i = 0; i < nKeys; i++) {
    int chart_family = m_composite_array[i].chart_family;
    int chart_type = m_composite_array[i].chart_type;

    bool selected = IsAnyActiveChartInPianoKeyElement(m_composite_array[i]);

    if (chart_type == CHART_TYPE_CM93 ||
        chart_type == CHART_TYPE_CM93COMP) {
      if (selected)
        dc.SetBrush(m_scBrush);
      else
        dc.SetBrush(m_cBrush);
    } else if (chart_type == CHART_TYPE_MBTILES) {
      if (selected)
        dc.SetBrush(m_tileBrush);
      else
        dc.SetBrush(m_utileBrush);
    } else if (chart_family ==  CHART_FAMILY_VECTOR) {
      if (selected)
        dc.SetBrush(m_svBrush);
      else
        dc.SetBrush(m_vBrush);
    } else {  // Raster Chart
      if (selected)
        dc.SetBrush(m_srBrush);
      else
        dc.SetBrush(m_rBrush);
    }

    if (m_bBusy) dc.SetBrush(m_unavailableBrush);

    wxRect box = KeyRect[i];
    box.y += y;

    if (m_brounded) {
      dc.DrawRoundedRectangle(box.x, box.y, box.width, box.height,
                              box.height / 5);
      if (shapeDC)
        shapeDC->DrawRoundedRectangle(box.x, box.y, box.width, box.height,
                                      box.height / 5);
    } else {
      dc.DrawRectangle(box.x, box.y, box.width, box.height);
      if (shapeDC) shapeDC->DrawRectangle(box);
    }

    if (IsAllEclipsedChartInPianoKeyElement(m_composite_array[i])) {
        //if (InArray(m_eclipsed_index_array, key_db_index)) {
      dc.SetBrush(m_backBrush);
      int w = 3;
      dc.DrawRoundedRectangle(box.x + w, box.y + w, box.width - (2 * w),
                              box.height - (2 * w), box.height / 5 - 1);
    }

#if 0
    //    Look in the current noshow array for this index
    if (InArray(m_noshow_index_array, key_db_index) && m_pInVizIconBmp &&
        m_pInVizIconBmp->IsOk())
      dc.DrawBitmap(ConvertTo24Bit(dc.GetBrush().GetColour(), *m_pInVizIconBmp),
                    box.x + 4, box.y + 3, false);

    //    Look in the current skew array for this index
    if (InArray(m_skew_index_array, key_db_index) && m_pSkewIconBmp &&
        m_pSkewIconBmp->IsOk())
      dc.DrawBitmap(ConvertTo24Bit(dc.GetBrush().GetColour(), *m_pSkewIconBmp),
                    box.x + box.width - m_pSkewIconBmp->GetWidth() - 4,
                    box.y + 2, false);

    //    Look in the current tmerc array for this index
    if (InArray(m_tmerc_index_array, key_db_index) && m_pTmercIconBmp &&
        m_pTmercIconBmp->IsOk())
      dc.DrawBitmap(ConvertTo24Bit(dc.GetBrush().GetColour(), *m_pTmercIconBmp),
                    box.x + box.width - m_pTmercIconBmp->GetWidth() - 4,
                    box.y + 2, false);

    //    Look in the current poly array for this index
    if (InArray(m_poly_index_array, key_db_index) && m_pPolyIconBmp &&
        m_pPolyIconBmp->IsOk())
      dc.DrawBitmap(ConvertTo24Bit(dc.GetBrush().GetColour(), *m_pPolyIconBmp),
                    box.x + box.width - m_pPolyIconBmp->GetWidth() - 4,
                    box.y + 2, false);
#endif
  }
}

#if 0
static void SetColor(unsigned char color[4], const wxBrush &brush)
{
    const wxColour &c = brush.GetColour();
    color[0] = c.Red(), color[1] = c.Green(), color[2] = c.Blue(), color[3] = 255;
}
#endif

// build a texture to hold minimum sized rectangles and icons used to render the
// chart bar this texture is only updated if the color scheme or chart bar
// height change
void Piano::BuildGLTexture() {
#ifdef ocpnUSE_GL

  // Defer building until auxiliary bitmaps have been loaded
  if (!m_pInVizIconBmp || !m_pTmercIconBmp || !m_pSkewIconBmp ||
      !m_pPolyIconBmp)
    return;

  int h = GetHeight();

  wxBrush tbackBrush;  // transparent back brush
  ocpnStyle::Style *style = g_StyleManager->GetCurrentStyle();
  if (style->chartStatusWindowTransparent)
    tbackBrush = wxColour(1, 1, 1);
  else
    tbackBrush = m_backBrush;

  wxBrush brushes[] = {m_scBrush,   m_cBrush,     m_svBrush,
                       m_vBrush,    m_srBrush,    m_rBrush,
                       m_tileBrush, m_utileBrush, m_unavailableBrush};

  m_ref = h;
  m_pad = h / 7;  // spacing between buttons
  m_radius = h / 4;
  m_texPitch = ((2 * m_ref) + (2 * m_pad));

  m_tex_piano_height = h;
  m_texw = m_texPitch * 3;

  m_texh = ((sizeof brushes) / (sizeof *brushes)) * h;
  m_texh += 4 * m_ref;  // for icons;

  m_texh = NextPow2(m_texh);
  m_texw = NextPow2(m_texw);

  if (!m_tex) glGenTextures(1, &m_tex);

  glBindTexture(GL_TEXTURE_2D, m_tex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  wxBitmap bmp(m_texw, m_texh);
  wxMemoryDC dc(bmp);

  dc.SetPen(*wxTRANSPARENT_PEN);
  dc.SetBrush(tbackBrush);
  dc.DrawRectangle(0, 0, m_texw, m_texh);

  // 0.5 mm nominal, but not less than 1 pixel
  double nominal_line_width_pix = floor(g_Platform->GetDisplayDPmm() / 2.0);
  nominal_line_width_pix *= OCPN_GetWinDIPScaleFactor();
  nominal_line_width_pix = wxMax(1.0, nominal_line_width_pix);

  // draw the needed rectangles
  wxPen ppPen(GetGlobalColor(_T("CHBLK")), nominal_line_width_pix,
              wxPENSTYLE_SOLID);
  dc.SetPen(ppPen);
  for (unsigned int b = 0; b < (sizeof brushes) / (sizeof *brushes); b++) {
    unsigned int x = 0, y = h * b;

    dc.SetBrush(brushes[b]);

    int v = 2;
    dc.DrawRectangle(x + m_pad, y + v, 2 * m_ref, h - 2 * v);

    x += m_texPitch;
    dc.DrawRoundedRectangle(x + m_pad, y + v, 2 * m_ref, h - 2 * v, m_radius);

    int w = m_ref / 6;  // border width of eclipsed chart

    x += m_texPitch;
    dc.DrawRoundedRectangle(x + m_pad, y + v, 2 * m_ref, h - 2 * v, m_radius);
    dc.SetBrush(m_backBrush);
    dc.DrawRoundedRectangle(x + m_pad + w, y + v + w, (2 * m_ref) - (2 * w),
                            h - 2 * v - 2 * w,
                            m_radius * (h - 2 * v - 2 * w) /
                                (h - 2 * v));  // slightly smaller radius
  }

  dc.SelectObject(wxNullBitmap);

  wxImage image = bmp.ConvertToImage();

  unsigned char *data = new unsigned char[4 * m_texw * m_texh], *d = data,
                *e = image.GetData(), *a = image.GetAlpha();
  for (unsigned int i = 0; i < m_texw * m_texh; i++) {
    if (style->chartStatusWindowTransparent && e[0] == 1 && e[1] == 1 &&
        e[2] == 1)
      d[3] = 0;
    else
      d[3] = 255;

    memcpy(d, e, 3), d += 4, e += 3;
  }

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_texw, m_texh, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, data);
  delete[] data;

  // put the bitmaps in below
  wxBitmap *bitmaps[] = {m_pInVizIconBmp, m_pTmercIconBmp, m_pSkewIconBmp,
                         m_pPolyIconBmp};

  for (unsigned int i = 0; i < 4; i++) {
    int iw = bitmaps[i]->GetWidth(), ih = bitmaps[i]->GetHeight();

    wxImage im = bitmaps[i]->ConvertToImage();
    unsigned char *data = new unsigned char[4 * iw * ih], *d = data,
                  *e = im.GetData(), *a = im.GetAlpha();
    for (int j = 0; j < iw * ih; j++) {
      memcpy(d, e, 3), d += 3, e += 3;
      *d = *a, d++, a++;
    }

    int off = ((sizeof brushes) / (sizeof *brushes)) * h + m_ref * i;
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, off, iw, ih, GL_RGBA, GL_UNSIGNED_BYTE,
                    data);
    delete[] data;
  }
#endif
}

void Piano::DrawGL(int off) {
  return DrawGLSL(off);
}
void Piano::DrawGLSL(int off) {
#ifdef ocpnUSE_GL
  unsigned int w = m_parentCanvas->GetClientSize().x *
                   m_parentCanvas->GetContentScaleFactor();
  int h = GetHeight();
  unsigned int endx = 0;

  if (static_cast<int>(m_tex_piano_height) != h) BuildGLTexture();

  if (static_cast<int>(m_tex_piano_height) != h) return;

  int y1 = off, y2 = y1 + h;

  int nKeys = m_composite_array.size();

  // we could cache the coordinates and recompute only when the piano hash
  // changes, but the performance is already fast enough at this point
  float *texcoords = new float[(nKeys * 3 + 1) * 4 * 2],
        *coords = new float[(nKeys * 3 + 1) * 4 * 2];

  int tc = 0, vc = 0;

  // draw the keys
  for (int i = 0; i < nKeys; i++) {
    int b;
    int chart_family = m_composite_array[i].chart_family;
    int chart_type = m_composite_array[i].chart_type;

    if (chart_type == CHART_TYPE_CM93 || chart_type == CHART_TYPE_CM93COMP)
      b = 0;
    else if (chart_type == CHART_TYPE_MBTILES)
      b = 6;
    else if (chart_family == CHART_FAMILY_VECTOR)
      b = 2;
    else  // Raster Chart
      b = 4;

    if (!IsAnyActiveChartInPianoKeyElement(m_composite_array[i]))
      b++;  // render color inactive

    wxRect box = KeyRect[i];
    float y = h * b, v1 = (y + .5) / m_texh, v2 = (y + h - .5) / m_texh;

    // texcord contains the texture pixel coordinates in the texture for the
    // three rectangle parts
    const float texcord[6] = {0,
                              (float)m_ref - 1,
                              (float)m_ref,
                              (float)m_ref,
                              (float)m_ref + 1,
                              (float)m_texPitch - 1};

    int uindex;
    if (m_brounded) {
      if (IsAllEclipsedChartInPianoKeyElement(m_composite_array[i]))
        uindex = 2;
      else
        uindex = 1;
    } else
      uindex = 0;

    // if the chart is too narrow.. we maybe render the "wrong" rectangle
    // because it can be thinner
    int x1 = box.x, x2 = x1 + box.width, w = 2 * uindex + 1;
    while (x1 + w > x2 - w && uindex > 0) uindex--, w -= 2;

    // the minimal width rectangles are texture mapped to the
    // width needed by mapping 3 quads: left middle and right
    int x[6] = {x1 - 3, x1 + m_ref, x2 - m_ref, x2 + 3};

    // adjust for very narrow keys
    if (x[1] > x[2]) {
      int avg = (x[1] + x[2]) / 2;
      x[1] = x[2] = avg;
    }

    for (int i = 0; i < 3; i++) {
      float u1 = ((uindex * m_texPitch) + texcord[2 * i] + .5) / m_texw,
            u2 = ((uindex * m_texPitch) + texcord[2 * i + 1] + .5) / m_texw;
      int x1 = x[i], x2 = x[i + 1];
      texcoords[tc++] = u1, texcoords[tc++] = v1, coords[vc++] = x1,
      coords[vc++] = y1;
      texcoords[tc++] = u2, texcoords[tc++] = v1, coords[vc++] = x2,
      coords[vc++] = y1;
      texcoords[tc++] = u2, texcoords[tc++] = v2, coords[vc++] = x2,
      coords[vc++] = y2;
      texcoords[tc++] = u1, texcoords[tc++] = v2, coords[vc++] = x1,
      coords[vc++] = y2;
    }
    endx = x[3];
  }

  // if not transparent, fill the rest of the chart bar with the background
  ocpnStyle::Style *style = g_StyleManager->GetCurrentStyle();
  if (!style->chartStatusWindowTransparent && endx < w) {
    texcoords[tc++] = 0, texcoords[tc++] = 0, coords[vc++] = endx,
    coords[vc++] = y1;
    texcoords[tc++] = 0, texcoords[tc++] = 0, coords[vc++] = w,
    coords[vc++] = y1;
    texcoords[tc++] = 0, texcoords[tc++] = 0, coords[vc++] = w,
    coords[vc++] = y2;
    texcoords[tc++] = 0, texcoords[tc++] = 0, coords[vc++] = endx,
    coords[vc++] = y2;
  }

  glBindTexture(GL_TEXTURE_2D, m_tex);

  glEnable(GL_TEXTURE_2D);

  glEnable(GL_BLEND);
  m_parentCanvas->GetglCanvas()->RenderTextures(
      m_parentCanvas->GetglCanvas()->m_gldc, coords, texcoords, vc / 2,
      m_parentCanvas->GetpVP());
  glDisable(GL_BLEND);

  glDisable(GL_BLEND);

  // draw the bitmaps, if any
  if (GetPianoMode() == PIANO_MODE_LEGACY) {
    vc = tc = 0;
    for (int i = 0; i < nKeys; i++) {
      int key_db_index = m_composite_array[i].dbindex_list[0];

      if (-1 == key_db_index) continue;

      wxRect box = KeyRect[i];

      wxBitmap *bitmaps[] = {m_pInVizIconBmp, m_pTmercIconBmp, m_pSkewIconBmp,
                             m_pPolyIconBmp};
      int index;
      if (InArray(m_noshow_index_array, key_db_index))
        index = 0;
      else {
        if (InArray(m_skew_index_array, key_db_index))
          index = 2;
        else if (InArray(m_tmerc_index_array, key_db_index))
          index = 1;
        else if (InArray(m_poly_index_array, key_db_index))
          index = 3;
        else
          continue;
      }

      int x1, y1, iw = bitmaps[index]->GetWidth(),
                  ih = bitmaps[index]->GetHeight();
      if (InArray(m_noshow_index_array, key_db_index))
        x1 = box.x + 4, y1 = box.y + 3;
      else
        x1 = box.x + box.width - iw - 4, y1 = box.y + 2;

      y1 += off;
      int x2 = x1 + iw, y2 = y1 + ih;

      wxBrush brushes[] = {m_scBrush,   m_cBrush,     m_svBrush,
                           m_vBrush,    m_srBrush,    m_rBrush,
                           m_tileBrush, m_utileBrush, m_unavailableBrush};

      float yoff = ((sizeof brushes) / (sizeof *brushes)) * h + 16 * index;
      float u1 = 0, u2 = (float)iw / m_texw;
      float v1 = yoff / m_texh, v2 = (yoff + ih) / m_texh;

      texcoords[tc++] = u1, texcoords[tc++] = v1, coords[vc++] = x1,
      coords[vc++] = y1;
      texcoords[tc++] = u2, texcoords[tc++] = v1, coords[vc++] = x2,
      coords[vc++] = y1;
      texcoords[tc++] = u2, texcoords[tc++] = v2, coords[vc++] = x2,
      coords[vc++] = y2;
      texcoords[tc++] = u1, texcoords[tc++] = v2, coords[vc++] = x1,
      coords[vc++] = y2;

      glEnable(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, m_tex);
      glEnable(GL_BLEND);

      m_parentCanvas->GetglCanvas()->RenderTextures(
          m_parentCanvas->GetglCanvas()->m_gldc, coords, texcoords, vc / 2,
          m_parentCanvas->GetpVP());
    }
  }

  glDisable(GL_BLEND);
  glDisable(GL_TEXTURE_2D);
  delete[] texcoords;
  delete[] coords;
#endif    //gl
}

void Piano::SetColorScheme(ColorScheme cs) {
  //    Recreate the local brushes

  m_backBrush = wxBrush(GetGlobalColor(_T("UIBDR")), wxBRUSHSTYLE_SOLID);

  m_rBrush = wxBrush(GetGlobalColor(_T("BLUE2")),
                     wxBRUSHSTYLE_SOLID);  // Raster Chart unselected
  m_srBrush =
      wxBrush(GetGlobalColor(_T("BLUE1")), wxBRUSHSTYLE_SOLID);  // and selected

  m_vBrush = wxBrush(GetGlobalColor(_T("GREEN2")),
                     wxBRUSHSTYLE_SOLID);  // Vector Chart unselected
  m_svBrush = wxBrush(GetGlobalColor(_T("GREEN1")),
                      wxBRUSHSTYLE_SOLID);  // and selected

  m_utileBrush = wxBrush(GetGlobalColor(_T("VIO01")),
                         wxBRUSHSTYLE_SOLID);  // MBTiles Chart unselected
  m_tileBrush =
      wxBrush(GetGlobalColor(_T("VIO02")), wxBRUSHSTYLE_SOLID);  // and selected

  m_cBrush = wxBrush(GetGlobalColor(_T("YELO2")),
                     wxBRUSHSTYLE_SOLID);  // CM93 Chart unselected
  m_scBrush =
      wxBrush(GetGlobalColor(_T("YELO1")), wxBRUSHSTYLE_SOLID);  // and selected

  m_unavailableBrush = wxBrush(GetGlobalColor(_T("UINFD")),
                               wxBRUSHSTYLE_SOLID);  // and unavailable

  m_tex_piano_height = 0;  // force texture to update
}

void Piano::ShowBusy(bool busy) {
  m_bBusy = busy;
  //    Refresh( true );
  //    Update();
}

bool Piano::IsAnyActiveChartInPianoKeyElement(PianoKeyElement &pke){
  for (auto &index : m_active_index_array){
    auto found = find(pke.dbindex_list.begin(), pke.dbindex_list.end(), index);
    if (found != pke.dbindex_list.end())
      return true;
  }
  return false;
}

bool Piano::IsAllEclipsedChartInPianoKeyElement(PianoKeyElement &pke){
  bool bfound_all = true;
  for (auto &index : pke.dbindex_list){
    auto found = find(m_eclipsed_index_array.begin(), m_eclipsed_index_array.end(), index);
    if (found == m_eclipsed_index_array.end())
      bfound_all = false;
  }
  return bfound_all;
}

void Piano::SetKeyArray(std::vector<int> &center_array, std::vector<int> &full_array) {

  // Measure the available space for keys, and so decide on mode
  // To account for scaling, etc., measure in parent base character size.
  if (center_array.size()) {
    int refd = m_parentCanvas->GetCharWidth();

    int nkeys = center_array.size();
    int key_width = (m_width_avail / refd) / nkeys;
    if (key_width < 8) {
      m_piano_mode = PIANO_MODE_COMPOSITE;
      m_key_array = full_array;
    }
    else {
      m_piano_mode = PIANO_MODE_LEGACY;
      m_key_array = center_array;
    }
  }
  else
    m_piano_mode = PIANO_MODE_LEGACY;

  // Create the composite array
  m_composite_array.clear();

  if (m_piano_mode == PIANO_MODE_COMPOSITE) {
    for (size_t i = 0; i < m_key_array.size(); i++) {
      const ChartTableEntry &cte = ChartData->GetChartTableEntry(m_key_array[i]);
      int scale = cte.GetScale();
      int chart_type = cte.GetChartType();

      // Perform scale compositing only for vector-family charts.
      // All other families/types retain legacy piano keys, implemented as
      // PianoKeyElement with only one chart in the array.
      if ((cte.GetChartFamily() == CHART_FAMILY_VECTOR) ||
          (cte.GetChartFamily() == CHART_FAMILY_RASTER)) {
        auto predicate = [scale, chart_type](const PianoKeyElement &pke) {
          return ((scale == pke.chart_scale) && (chart_type == pke.chart_type));
        };
        auto found = find_if(m_composite_array.begin(), m_composite_array.end(),
                             predicate);
        if (found == m_composite_array.end()) {  // need a new PianoKeyElement
          PianoKeyElement new_pke;
          new_pke.chart_scale = scale;
          new_pke.chart_family = (ChartFamilyEnum)cte.GetChartFamily();
          new_pke.chart_type = (ChartTypeEnum)cte.GetChartType();
          new_pke.dbindex_list.push_back(m_key_array[i]);
          m_composite_array.push_back(new_pke);
        } else {
          PianoKeyElement &ex_pke = *found;
          ex_pke.dbindex_list.push_back(m_key_array[i]);
        }
      } else {
        PianoKeyElement new_pke;
        new_pke.chart_scale = scale;
        new_pke.chart_family = (ChartFamilyEnum)cte.GetChartFamily();
        new_pke.chart_type = (ChartTypeEnum)cte.GetChartType();
        new_pke.dbindex_list.push_back(m_key_array[i]);
        m_composite_array.push_back(new_pke);
      }
    }
  }
  else {
    for (size_t i = 0; i < m_key_array.size(); i++) {
      const ChartTableEntry &cte = ChartData->GetChartTableEntry(m_key_array[i]);
      int scale = cte.GetScale();
      int chart_type = cte.GetChartType();
      PianoKeyElement new_pke;
      new_pke.chart_scale = scale;
      new_pke.chart_family = (ChartFamilyEnum)cte.GetChartFamily();
      new_pke.chart_type = (ChartTypeEnum)cte.GetChartType();
      new_pke.dbindex_list.push_back(m_key_array[i]);
      m_composite_array.push_back(new_pke);
    }
  }

  // Sort the composite array
  std::sort(m_composite_array.begin(),m_composite_array.end(),
            [](PianoKeyElement &a, PianoKeyElement &b){ return a.chart_scale < b.chart_scale; });

}

void Piano::SetNoshowIndexArray(std::vector<int> array) {
  m_noshow_index_array = array;
}

void Piano::AddNoshowIndexArray(std::vector<int> array) {
  for (unsigned int i = 0; i < array.size(); i++) {
    m_noshow_index_array.push_back(array[i]);
  }
}

void Piano::SetActiveKeyArray(std::vector<int> array) {
  m_active_index_array = array;
}

void Piano::SetEclipsedIndexArray(std::vector<int> array) {
  m_eclipsed_index_array = array;
}

void Piano::SetSkewIndexArray(std::vector<int> array) {
  m_skew_index_array = array;
}

void Piano::SetTmercIndexArray(std::vector<int> array) {
  m_tmerc_index_array = array;
}

void Piano::SetPolyIndexArray(std::vector<int> array) {
  m_poly_index_array = array;
}

bool Piano::InArray(std::vector<int> &array, int key) {
  for (unsigned int ino = 0; ino < array.size(); ino++)
    if (array[ino] == key) return true;
  return false;
}

wxString Piano::GetStateHash() {
  wxString hash;

  for (unsigned int i = 0; i < m_key_array.size(); i++) {
    wxString a;
    a.Printf(_T("%dK"), m_key_array[i]);
    hash += a;
  }
  for (unsigned int i = 0; i < m_noshow_index_array.size(); i++) {
    wxString a;
    a.Printf(_T("%dN"), m_noshow_index_array[i]);
    hash += a;
  }
  for (unsigned int i = 0; i < m_active_index_array.size(); i++) {
    wxString a;
    a.Printf(_T("%dA"), m_active_index_array[i]);
    hash += a;
  }
  for (unsigned int i = 0; i < m_eclipsed_index_array.size(); i++) {
    wxString a;
    a.Printf(_T("%dE"), m_eclipsed_index_array[i]);
    hash += a;
  }
  for (unsigned int i = 0; i < m_skew_index_array.size(); i++) {
    wxString a;
    a.Printf(_T("%dW"), m_skew_index_array[i]);
    hash += a;
  }
  for (unsigned int i = 0; i < m_tmerc_index_array.size(); i++) {
    wxString a;
    a.Printf(_T("%dM"), m_tmerc_index_array[i]);
    hash += a;
  }
  for (unsigned int i = 0; i < m_poly_index_array.size(); i++) {
    wxString a;
    a.Printf(_T("%dP"), m_poly_index_array[i]);
    hash += a;
  }

  return hash;
}

wxString &Piano::GenerateAndStoreNewHash() {
  m_hash = GetStateHash();
  return m_hash;
}

wxString &Piano::GetStoredHash() { return m_hash; }

void Piano::FormatKeys(void) {
  ocpnStyle::Style *style = g_StyleManager->GetCurrentStyle();
  int width = m_parentCanvas->GetClientSize().x;
#ifdef __WXOSX__
  if(g_bopengl)
    width *= m_parentCanvas->GetContentScaleFactor();
#endif
  int height = GetHeight();
  width *= g_btouch ? 0.98f : 0.6f;

  // Max width available
  m_width_avail = width;

  int nKeys = m_composite_array.size();
  int kw = style->chartStatusIconWidth;
  if (nKeys) {
    if (!kw) kw = width / nKeys;

    kw = wxMin(kw, (width * 3 / 4) / nKeys);
    kw = wxMax(kw, 6);

    //    Build the Key Regions

    KeyRect.clear();
    m_width = 0;
    for (int i = 0; i < nKeys; i++) {
      wxRect r((i * kw) + 3, 2, kw - 6, height - 4);
      KeyRect.push_back(r);
      m_width = r.x + r.width;
    }
  }
  m_nRegions = nKeys;
}

wxPoint Piano::GetKeyOrigin(int key_index) {
  if ((key_index >= 0) && (key_index <= (int)m_key_array.size() - 1)) {
    wxRect box = KeyRect[key_index];
    return wxPoint(box.x, box.y);
  } else
    return wxPoint(-1, -1);
}

bool Piano::MouseEvent(wxMouseEvent &event) {
  int x, y;
  event.GetPosition(&x, &y);
#ifdef __WXOSX__
  if (g_bopengl){
    x *= OCPN_GetDisplayContentScaleFactor();
    y *= OCPN_GetDisplayContentScaleFactor();
  }
#endif

  int ytop = m_parentCanvas->GetCanvasHeight() - GetHeight();
#ifdef __WXOSX__
  if (!g_bopengl)
    ytop = m_parentCanvas->GetClientSize().y - GetHeight();
#endif

  if (event.Leaving() || y < ytop) {
    if (m_bleaving) return false;
    m_bleaving = true;
  } else
    m_bleaving = false;

  //    Check the regions

  int sel_index = -1;
  int sel_dbindex = -1;

  for (int i = 0; i < m_nRegions; i++) {
    if (KeyRect[i].Contains(x, 6)) {
      sel_index = i;
      sel_dbindex = m_key_array[i];
      break;
    }
  }

  if (g_btouch) {
    if (event.LeftDown()) {
      if (-1 != sel_index) {
        m_action = DEFERRED_KEY_CLICK_DOWN;
        ShowBusy(true);
        m_eventTimer.Start(10, wxTIMER_ONE_SHOT);
      }
    }
    if (event.LeftUp()) {
      if (-1 != sel_index) {
        m_click_sel_index = sel_index;
        if (!m_eventTimer.IsRunning()) {
          m_action = DEFERRED_KEY_CLICK_UP;
          m_eventTimer.Start(10, wxTIMER_ONE_SHOT);
        }
      }
    } else if (event.RightDown()) {
      if (sel_index != m_hover_last) {
        if (sel_index >= 0)
          m_parentCanvas->HandlePianoRollover(sel_index,
                                            m_composite_array[sel_index].dbindex_list,
                                              m_composite_array[sel_index].dbindex_list.size(),
                                              m_composite_array[sel_index].chart_scale  );
        m_hover_last = sel_index;

        //                m_action = INFOWIN_TIMEOUT;
        //                m_eventTimer.Start(3000, wxTIMER_ONE_SHOT);
      }
    } else if (event.ButtonUp()) {
      m_parentCanvas->ClearPianoRollover();
      ResetRollover();
    }
  } else {
    if (m_bleaving) {
      m_parentCanvas->ClearPianoRollover();
      ResetRollover();
    } else if (event.LeftDown()) {
      if (-1 != sel_index) {
        m_parentCanvas->HandlePianoClick(sel_index,
                                            m_composite_array[sel_index].dbindex_list);

        m_parentCanvas->Raise();
      } else
        return false;
    } else if (event.RightDown()) {
      if (-1 != sel_index) {
        m_parentCanvas->HandlePianoRClick(x, y, sel_index,
                                          m_composite_array[sel_index].dbindex_list);
        m_parentCanvas->Raise();
      } else
        return false;
    } else if (!event.ButtonUp()) {
      if (sel_index != m_hover_last) {
        if (sel_index >= 0)
          m_parentCanvas->HandlePianoRollover(sel_index,
                                              m_composite_array[sel_index].dbindex_list,
                                              m_composite_array[sel_index].dbindex_list.size(),
                                              m_composite_array[sel_index].chart_scale );

        m_hover_last = sel_index;
      }
    }
  }

  return true;

  /*
   Todo:
   Could do something like this to better encapsulate the pianowin
   Allows us to get rid of global statics...

   wxCommandEvent ev(MyPianoEvent);    // Private event
   ..set up event to specify action...SelectChart,  etc
   ::PostEvent(pEventReceiver, ev);    // event receiver passed to ctor

   */
}

void Piano::ResetRollover(void) {
  m_index_last = -1;
  m_hover_icon_last = -1;
  m_hover_last = -1;
  m_gotPianoDown = false;
}

int Piano::GetHeight() {
  int height = 22 ;
#ifdef __WXOSX__
  if (g_bopengl)
    height *= m_parentCanvas->GetContentScaleFactor();
#endif
  if (g_btouch) {
    double size_mult = exp(g_GUIScaleFactor * 0.0953101798043);  // ln(1.1)
    height *= size_mult;
    height = wxMin(height, 100);  // absolute boundaries
    height = wxMax(height, 10);
  }
  height *= g_Platform->GetDisplayDensityFactor();

#ifdef __OCPN__ANDROID__
  height = wxMax(height, 4 * g_Platform->GetDisplayDPmm());
#endif

  height /= g_BasePlatform->GetDisplayDIPMult(gFrame);

  return height;
}

int Piano::GetWidth() { return m_width; }

void Piano::onTimerEvent(wxTimerEvent &event) {
  switch (m_action) {
    case DEFERRED_KEY_CLICK_DOWN:
      m_gotPianoDown = true;
      break;
    case DEFERRED_KEY_CLICK_UP:
      ShowBusy(false);
      if ((m_hover_last >= 0) || !m_gotPianoDown) {  // turn it off, and return
        m_parentCanvas->ClearPianoRollover();
        ResetRollover();
      } else {
        m_parentCanvas->HandlePianoClick(m_click_sel_index,
                                         m_composite_array[m_click_sel_index].dbindex_list);
        m_gotPianoDown = false;
      }
      break;
    case INFOWIN_TIMEOUT:
      m_parentCanvas->ClearPianoRollover();
      ResetRollover();
      break;
    default:
      break;
  }
}
