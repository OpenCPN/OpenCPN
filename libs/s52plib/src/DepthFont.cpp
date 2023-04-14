/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  texture OpenGL text rendering built from wxFont
 * Author:   Sean D'Epagnier
 *
 ***************************************************************************
 *   Copyright (C) 2014 Sean D'Epagnier                                    *
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

#include <wx/wx.h>

#include "s52plibGL.h"

#include "DepthFont.h"
#include "ocpn_plugin.h"

DepthFont::DepthFont() {
  texobj = 0;
  m_built = false;
  m_scaleFactor = 0;
  m_ContentScaleFactor = 1.0;
}

DepthFont::~DepthFont() { Delete(); }

void DepthFont::Build(wxFont *font, double scale, double dip_factor) {
  /* avoid rebuilding if the parameters are the same */
  if (m_built && (*font == m_font)) return;

  m_font = *font;
  m_scaleFactor = scale;
  m_dip_factor = dip_factor;

  m_maxglyphw = 0;
  m_maxglyphh = 0;

  double scaler = m_ContentScaleFactor * dip_factor;

  wxFont *scaled_font =
          FindOrCreateFont_PlugIn(font->GetPointSize() / scaler,
                                  font->GetFamily(), font->GetStyle(),
                                  font->GetWeight(), false,
                                  font->GetFaceName());
  wxScreenDC sdc;
  sdc.SetFont(*scaled_font);

  for (int i = 0; i < 10; i++) {
    wxCoord gw, gh;
    wxString text;
    text = wxString::Format(_T("%d"), i);
    wxCoord descent, exlead;
    sdc.GetTextExtent(text, &gw, &gh, &descent, &exlead,
                      scaled_font);  // measure the text

    tgi[i].width = gw;
    tgi[i].height = gh;  // - descent;

    tgi[i].advance = gw;
    tgi[i].advance *= dip_factor;
    if (i == 1)
      m_width_one = gw;

    m_maxglyphw = wxMax(tgi[i].width + tgi[i].advance, m_maxglyphw);
    m_maxglyphh = wxMax(tgi[i].height, m_maxglyphh);
  }

  int w = 10 * m_maxglyphw;
  int h = m_maxglyphh;

  /* make power of 2 */

  for (tex_w = 1; tex_w < w; tex_w *= 2)
    ;
  for (tex_h = 1; tex_h < h; tex_h *= 2)
    ;

  wxBitmap tbmp(tex_w, tex_h);
  wxMemoryDC dc;
  dc.SelectObject(tbmp);
  dc.SetFont(*scaled_font);

  /* fill bitmap with black */
  dc.SetBackground(wxBrush(wxColour(0, 0, 0)));
  dc.Clear();

  /* draw the text white */
  dc.SetTextForeground(wxColour(255, 255, 255));

  /*    wxPen pen(wxColour( 255, 255, 255 ));
      wxBrush brush(wxColour( 255, 255, 255 ), wxTRANSPARENT);
      dc.SetPen(pen);
      dc.SetBrush(brush);
   */
  int row = 0, col = 0;
  for (int i = 0; i < 10; i++) {
    tgi[i].x = col * m_maxglyphw;
    tgi[i].y = row * m_maxglyphh;

    wxString text;
    text = wxString::Format(_T("%d"), i);

    dc.DrawText(text, tgi[i].x, tgi[i].y);

    col++;
  }

  dc.SelectObject(wxNullBitmap);

  wxImage image = tbmp.ConvertToImage();

  GLuint format, internalformat;
  int stride;

  format = GL_ALPHA;
  internalformat = format;
  stride = 1;

  unsigned char *imgdata = image.GetData();

  if (imgdata) {
    unsigned char *teximage = (unsigned char *)malloc(stride * tex_w * tex_h);

    for (int j = 0; j < tex_w * tex_h; j++)
      for (int k = 0; k < stride; k++)
        teximage[j * stride + k] = imgdata[3 * j];

    glGenTextures(1, &texobj);
    glBindTexture(GL_TEXTURE_2D, texobj);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                    GL_NEAREST /*GL_LINEAR*/);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, internalformat, tex_w, tex_h, 0, format,
                 GL_UNSIGNED_BYTE, teximage);

    free(teximage);
  }

  m_built = true;
}

void DepthFont::Delete() {
  if (texobj) {
    glDeleteTextures(1, &texobj);
    texobj = 0;
  }
  m_built = false;
  m_scaleFactor = 0;
}

bool DepthFont::GetGLTextureRect(wxRect &texrect, int symIndex) {
  if (symIndex < 10) {
    texrect.x = tgi[symIndex].x;
    texrect.y = tgi[symIndex].y;
    texrect.width = tgi[symIndex].width + (m_width_one / 10);
    texrect.height = tgi[symIndex].height;
    return true;
  } else {
    texrect.x = tgi[0].x;
    texrect.y = tgi[0].y;
    texrect.width = tgi[0].width;
    texrect.height = tgi[0].height;
    return false;
  }
}

