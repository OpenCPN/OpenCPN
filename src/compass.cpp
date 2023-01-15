/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  OpenCPN Main wxWidgets Program
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
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
 */
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // precompiled headers
#include "config.h"
#include "ocpn_types.h"
#include "compass.h"
#include "chcanv.h"
#include "styles.h"

#include "glChartCanvas.h"
#include "ocpn_frame.h"     // FIXME (dave) colorschemes

extern ocpnStyle::StyleManager* g_StyleManager;
extern bool bGPSValid;
extern bool g_bSatValid;
extern int g_SatsInView;
extern bool g_bopengl;

ocpnCompass::ocpnCompass(ChartCanvas* parent, bool bShowGPS) {
  m_parent = parent;
  m_bshowGPS = bShowGPS;

  ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();
  _img_compass = style->GetIcon(_T("CompassRose"));
  _img_gpsRed = style->GetIcon(_T("gpsRed"));

  m_rose_angle = -999;  // force a refresh when first used

  m_pStatBoxToolStaticBmp = NULL;

  m_rect =
      wxRect(style->GetCompassXOffset(), style->GetCompassYOffset(),
             _img_compass.GetWidth() + _img_gpsRed.GetWidth() +
                 style->GetCompassLeftMargin() * 2 + style->GetToolSeparation(),
             _img_compass.GetHeight() + style->GetCompassTopMargin() +
                 style->GetCompassBottomMargin());

#ifdef ocpnUSE_GL
  m_texobj = 0;
#endif
  m_texOK = false;

  m_scale = 1.0;
  m_cs = GLOBAL_COLOR_SCHEME_RGB;
}

ocpnCompass::~ocpnCompass() {
#ifdef ocpnUSE_GL
  if (m_texobj) {
    glDeleteTextures(1, &m_texobj);
    m_texobj = 0;
  }
#endif

  delete m_pStatBoxToolStaticBmp;
}

void ocpnCompass::Paint(ocpnDC& dc) {
  if (m_shown && m_StatBmp.IsOk()) {
#if defined(ocpnUSE_GLES) || defined(ocpnUSE_GL)
    if (!m_texobj){
      // The glContext is known active here,
      // so safe to create a texture.
      glGenTextures(1, &m_texobj);
      CreateTexture();
    }

    if (g_bopengl && m_texobj /*&& m_texOK*/) {
      glBindTexture(GL_TEXTURE_2D, m_texobj);
      glEnable(GL_TEXTURE_2D);

#if defined(USE_ANDROID_GLES2) || defined(ocpnUSE_GLSL)
      float coords[8];
      float uv[8];

      // normal uv, normalized to POT
      uv[0] = 0;
      uv[1] = 0;
      uv[2] = (float)m_image_width / m_tex_w;
      uv[3] = 0;
      uv[4] = (float)m_image_width / m_tex_w;
      uv[5] = (float)m_image_height / m_tex_h;
      uv[6] = 0;
      uv[7] = (float)m_image_height / m_tex_h;

      // pixels
      coords[0] = m_rect.x;
      coords[1] = m_rect.y;
      coords[2] = m_rect.x + m_rect.width;
      coords[3] = m_rect.y;
      coords[4] = m_rect.x + m_rect.width;
      coords[5] = m_rect.y + m_rect.height;
      coords[6] = m_rect.x;
      coords[7] = m_rect.y + m_rect.height;

      m_parent->GetglCanvas()->RenderTextures(dc, coords, uv, 4,
                                              m_parent->GetpVP());
#else

      glBegin(GL_QUADS);

      glTexCoord2f(0, 0);
      glVertex2i(m_rect.x, m_rect.y);
      glTexCoord2f((float)m_image_width / m_tex_w, 0);
      glVertex2i(m_rect.x + m_rect.width, m_rect.y);
      glTexCoord2f((float)m_image_width / m_tex_w,
                   (float)m_image_height / m_tex_h);
      glVertex2i(m_rect.x + m_rect.width, m_rect.y + m_rect.height);
      glTexCoord2f(0, (float)m_image_height / m_tex_h);
      glVertex2i(m_rect.x, m_rect.y + m_rect.height);

      glEnd();
#endif

      glDisable(GL_TEXTURE_2D);

    } else {
#ifdef __WXOSX__
      // Support MacBook Retina display
      if(g_bopengl){
        double scale = m_parent->GetContentScaleFactor();
        if(scale > 1){
          wxImage image = m_StatBmp.ConvertToImage();
          image.Rescale( image.GetWidth() * scale, image.GetHeight() * scale);
          wxBitmap bmp( image );
          dc.DrawBitmap(bmp, m_rect.x, m_rect.y, true);
        }
        else
          dc.DrawBitmap(m_StatBmp, m_rect.x, m_rect.y, true);
      }
      else
        dc.DrawBitmap(m_StatBmp, m_rect.x, m_rect.y, true);
#else
      dc.DrawBitmap(m_StatBmp, m_rect.x, m_rect.y, true);
#endif
    }

#else
    dc.DrawBitmap(m_StatBmp, m_rect.x, m_rect.y, true);
#endif
  }
}

bool ocpnCompass::MouseEvent(wxMouseEvent& event) {
  if (!m_shown || !m_rect.Contains(event.GetPosition())) return false;

  if (event.LeftDown()) {
    if (m_parent->GetUpMode() == NORTH_UP_MODE)
      m_parent->SetUpMode(COURSE_UP_MODE);
    else if (m_parent->GetUpMode() == COURSE_UP_MODE)
      m_parent->SetUpMode(HEAD_UP_MODE);
    else
      m_parent->SetUpMode(NORTH_UP_MODE);
  }

  return true;
}

void ocpnCompass::SetColorScheme(ColorScheme cs) {
  m_cs = cs;
  UpdateStatus(true);
}

void ocpnCompass::UpdateStatus(bool bnew) {
  if (bnew)
    m_lastgpsIconName.Clear();  // force an update to occur

  CreateBmp(bnew);

#ifdef ocpnUSE_GL
  if (g_bopengl && m_texobj)
    CreateTexture();
#endif

}

void ocpnCompass::SetScaleFactor(float factor) {
  ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();

  if (factor > 0.1)
    m_scale = factor;
  else
    m_scale = 1.0;

  //  Precalculate the background sizes to get m_rect width/height
  wxBitmap compassBg, gpsBg;
  int orient = style->GetOrientation();
  style->SetOrientation(wxTB_HORIZONTAL);
  if (style->HasBackground()) {
    compassBg = style->GetNormalBG();
    style->DrawToolbarLineStart(compassBg);
    compassBg = style->SetBitmapBrightness(compassBg, m_cs);
    gpsBg = style->GetNormalBG();
    style->DrawToolbarLineEnd(gpsBg);
    gpsBg = style->SetBitmapBrightness(gpsBg, m_cs);
  }

  if (fabs(m_scale - 1.0) > 0.1) {
    wxImage bg_img = compassBg.ConvertToImage();
    bg_img.Rescale(compassBg.GetWidth() * m_scale,
                   compassBg.GetHeight() * m_scale, wxIMAGE_QUALITY_NORMAL);
    compassBg = wxBitmap(bg_img);

    bg_img = gpsBg.ConvertToImage();
    bg_img.Rescale(gpsBg.GetWidth() * m_scale, gpsBg.GetHeight() * m_scale,
                   wxIMAGE_QUALITY_NORMAL);
    gpsBg = wxBitmap(bg_img);
  }

  int width =
      compassBg.GetWidth() + gpsBg.GetWidth() + style->GetCompassLeftMargin();
  if (!style->marginsInvisible)
    width += style->GetCompassLeftMargin() + style->GetToolSeparation();

  m_rect = wxRect(style->GetCompassXOffset(), style->GetCompassYOffset(), width,
                  compassBg.GetHeight() + style->GetCompassTopMargin() +
                      style->GetCompassBottomMargin());
}

void ocpnCompass::CreateBmp(bool newColorScheme) {
  //if (!m_shown) return;

  wxString gpsIconName;
  ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();

  // In order to draw a horizontal compass window when the toolbar is vertical,
  // we need to save away the sizes and backgrounds for the two icons.

  static wxBitmap compassBg, gpsBg;
  static wxSize toolsize;
  static int topmargin, leftmargin, radius;

  if (!compassBg.IsOk() || newColorScheme) {
    int orient = style->GetOrientation();
    style->SetOrientation(wxTB_HORIZONTAL);
    if (style->HasBackground()) {
      compassBg = style->GetNormalBG();
      style->DrawToolbarLineStart(compassBg);
      compassBg = style->SetBitmapBrightness(compassBg, m_cs);
      gpsBg = style->GetNormalBG();
      style->DrawToolbarLineEnd(gpsBg);
      gpsBg = style->SetBitmapBrightness(gpsBg, m_cs);
    }

    if (fabs(m_scale - 1.0) > 0.1) {
      wxImage bg_img = compassBg.ConvertToImage();
      bg_img.Rescale(compassBg.GetWidth() * m_scale,
                     compassBg.GetHeight() * m_scale, wxIMAGE_QUALITY_NORMAL);
      compassBg = wxBitmap(bg_img);

      bg_img = gpsBg.ConvertToImage();
      bg_img.Rescale(gpsBg.GetWidth() * m_scale, gpsBg.GetHeight() * m_scale,
                     wxIMAGE_QUALITY_NORMAL);
      gpsBg = wxBitmap(bg_img);
    }

    leftmargin = style->GetCompassLeftMargin();
    topmargin = style->GetCompassTopMargin();
    radius = style->GetCompassCornerRadius();

    if (orient == wxTB_VERTICAL) style->SetOrientation(wxTB_VERTICAL);
  }

  bool b_need_refresh = false;

  if (bGPSValid) {
    if (g_bSatValid) {
      gpsIconName = _T("gps3Bar");
      if (g_SatsInView <= 8) gpsIconName = _T("gps2Bar");
      if (g_SatsInView <= 4) gpsIconName = _T("gps1Bar");
      if (g_SatsInView < 0) gpsIconName = _T("gpsGry");

    } else
      gpsIconName = _T("gpsGrn");
  } else
    gpsIconName = _T("gpsRed");

  if (m_lastgpsIconName != gpsIconName) b_need_refresh = true;

  double rose_angle = -999.;

  if ((fabs(m_parent->GetVPRotation()) > .01) ||
      (fabs(m_parent->GetVPSkew()) > .01)) {
    rose_angle = -m_parent->GetVPRotation();
  } else
    rose_angle = 0.;

  if (fabs(m_rose_angle - rose_angle) > .1) b_need_refresh = true;

  //if (!b_need_refresh) return;

  int width = compassBg.GetWidth();
  if (m_bshowGPS) width += gpsBg.GetWidth() + leftmargin;

  if (!style->marginsInvisible)
    width += leftmargin + style->GetToolSeparation();

  m_StatBmp.Create(width, compassBg.GetHeight() + topmargin +
                              style->GetCompassBottomMargin());

  m_rect.width = m_StatBmp.GetWidth();
  m_rect.height = m_StatBmp.GetHeight();

  //if (!m_StatBmp.IsOk()) return;

  m_MaskBmp = wxBitmap(m_StatBmp.GetWidth(), m_StatBmp.GetHeight());
  if (style->marginsInvisible) {
    wxMemoryDC sdc(m_MaskBmp);
    sdc.SetBackground(*wxWHITE_BRUSH);
    sdc.Clear();
    sdc.SetBrush(*wxBLACK_BRUSH);
    sdc.SetPen(*wxBLACK_PEN);
    wxSize maskSize = wxSize(m_MaskBmp.GetWidth() - leftmargin,
                             m_MaskBmp.GetHeight() - (2 * topmargin));
    sdc.DrawRoundedRectangle(wxPoint(leftmargin, topmargin), maskSize, radius);
    sdc.SelectObject(wxNullBitmap);
  } else if (radius) {
    wxMemoryDC sdc(m_MaskBmp);
    sdc.SetBackground(*wxWHITE_BRUSH);
    sdc.Clear();
    sdc.SetBrush(*wxBLACK_BRUSH);
    sdc.SetPen(*wxBLACK_PEN);
    sdc.DrawRoundedRectangle(0, 0, m_MaskBmp.GetWidth(), m_MaskBmp.GetHeight(),
                             radius);
    sdc.SelectObject(wxNullBitmap);
  }
#if !defined(USE_ANDROID_GLES2) && !defined(ocpnUSE_GLSL)
  m_StatBmp.SetMask(new wxMask(m_MaskBmp, *wxWHITE));
#endif

  wxMemoryDC mdc;
  mdc.SelectObject(m_StatBmp);
  mdc.SetBackground(wxBrush(GetGlobalColor(_T("COMP1")), wxBRUSHSTYLE_SOLID));
  mdc.Clear();

  mdc.SetPen(wxPen(GetGlobalColor(_T("UITX1")), 1));
  mdc.SetBrush(wxBrush(GetGlobalColor(_T("UITX1")), wxBRUSHSTYLE_TRANSPARENT));

  if (!style->marginsInvisible)
    mdc.DrawRoundedRectangle(0, 0, m_StatBmp.GetWidth(), m_StatBmp.GetHeight(),
                             radius);

  wxPoint offset(leftmargin, topmargin);

  //    Build Compass Rose, rotated...
  wxBitmap BMPRose;
  wxPoint after_rotate;

  int cwidth = style->GetToolSize().x * m_scale;
  int cheight = style->GetToolSize().y * m_scale;
  cheight = wxMin(cheight, compassBg.GetHeight());
  cwidth = wxMin(cwidth, cheight);
  cheight = cwidth;

  if (m_parent->GetUpMode() == COURSE_UP_MODE)
    BMPRose = style->GetIcon(_T("CompassRose"), cwidth, cheight);
  else if (m_parent->GetUpMode() == HEAD_UP_MODE)
    BMPRose = style->GetIcon(_T("CompassRoseMag"), cwidth, cheight);
  else
    BMPRose = style->GetIcon(_T("CompassRoseBlue"), cwidth, cheight);
  if ((fabs(m_parent->GetVPRotation()) > .01) ||
      (fabs(m_parent->GetVPSkew()) > .01)) {
    wxImage rose_img = BMPRose.ConvertToImage();
    wxPoint rot_ctr(cwidth / 2, cheight / 2);
    wxImage rot_image =
        rose_img.Rotate(rose_angle, rot_ctr, true, &after_rotate);
    BMPRose = wxBitmap(rot_image).GetSubBitmap(
        wxRect(-after_rotate.x, -after_rotate.y, cwidth, cheight));
  }

  wxBitmap iconBm;

  if (style->HasBackground()) {
    iconBm = MergeBitmaps(compassBg, BMPRose, wxSize(0, 0));
  } else {
    iconBm = BMPRose;
  }

  iconBm = ConvertTo24Bit(wxColor(0, 0, 0), iconBm);

  mdc.DrawBitmap(iconBm, offset);
  offset.x += iconBm.GetWidth();
  offset.x += style->GetToolSeparation();

  m_rose_angle = rose_angle;

  if (m_bshowGPS) {
    //  GPS Icon
    int twidth = style->GetToolSize().x * m_scale;
    int theight = style->GetToolSize().y * m_scale;
    theight = wxMin(cheight, compassBg.GetHeight());
    int swidth = wxMax(twidth, theight);
    int sheight = wxMin(twidth, theight);

    //  Sometimes, the SVG renderer gets the size wrong due to some internal
    //  rounding error. If so found, it seems to work OK by just reducing the
    //  requested size by one pixel....
    wxBitmap gicon = style->GetIcon(gpsIconName, swidth, sheight);
    if (gicon.GetHeight() != sheight)
      gicon = style->GetIcon(gpsIconName, swidth - 1, sheight - 1, true);

    if (style->HasBackground()) {
      iconBm = MergeBitmaps(gpsBg, gicon, wxSize(0, 0));
    } else {
      iconBm = gicon;
    }

    iconBm = ConvertTo24Bit(wxColor(0, 0, 0), iconBm);
    mdc.DrawBitmap(iconBm, offset);
    mdc.SelectObject(wxNullBitmap);

    m_lastgpsIconName = gpsIconName;
  }
}

void ocpnCompass::CreateTexture() {
#if defined(USE_ANDROID_GLES2) || defined(ocpnUSE_GLSL)
  // GLES does not do ocpnDC::DrawBitmap(), so use
                           // texture
  if (g_bopengl) {
    wxImage image = m_StatBmp.ConvertToImage();
    unsigned char* imgdata = image.GetData();
    unsigned char* imgalpha = image.GetAlpha();
    m_tex_w = image.GetWidth();
    m_tex_h = image.GetHeight();
    m_image_width = m_tex_w;
    m_image_height = m_tex_h;

    // Make it POT
    int width_pot = m_tex_w;
    int height_pot = m_tex_h;

    int xp = image.GetWidth();
    if (((xp != 0) && !(xp & (xp - 1))))  // detect POT
      width_pot = xp;
    else {
      int a = 0;
      while (xp) {
        xp = xp >> 1;
        a++;
      }
      width_pot = 1 << a;
    }

    xp = image.GetHeight();
    if (((xp != 0) && !(xp & (xp - 1))))
      height_pot = xp;
    else {
      int a = 0;
      while (xp) {
        xp = xp >> 1;
        a++;
      }
      height_pot = 1 << a;
    }

    m_tex_w = width_pot;
    m_tex_h = height_pot;

    GLuint format = GL_RGBA;
    GLuint internalformat = GL_RGBA8; //format;
    int stride = 4;

    if (imgdata) {
      unsigned char* teximage =
          (unsigned char*)malloc(stride * m_tex_w * m_tex_h);

      for (int i = 0; i < m_image_height; i++) {
        for (int j = 0; j < m_image_width; j++) {
          int s = (i * 3 * m_image_width) + (j * 3);
          int d = (i * stride * m_tex_w) + (j * stride);

          teximage[d + 0] = imgdata[s + 0];
          teximage[d + 1] = imgdata[s + 1];
          teximage[d + 2] = imgdata[s + 2];
          teximage[d + 3] = 255;
        }
      }

      glBindTexture(GL_TEXTURE_2D, m_texobj);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                      GL_NEAREST);  // No mipmapping
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

      glTexImage2D(GL_TEXTURE_2D, 0, internalformat, m_tex_w, m_tex_h, 0,
                   format, GL_UNSIGNED_BYTE, teximage);

      free(teximage);
      glBindTexture(GL_TEXTURE_2D, 0);
      m_texOK = true;
    }
  }
#endif
}

void ocpnCompass::UpdateTexture() {
#if defined(USE_ANDROID_GLES2) || defined(ocpnUSE_GLSL)
  // GLES does not do ocpnDC::DrawBitmap(), so use
                           // texture
  if (g_bopengl) {
    wxImage image = m_StatBmp.ConvertToImage();
    unsigned char* imgdata = image.GetData();
    unsigned char* imgalpha = image.GetAlpha();
    m_tex_w = image.GetWidth();
    m_tex_h = image.GetHeight();
    m_image_width = m_tex_w;
    m_image_height = m_tex_h;

    // Make it POT
    int width_pot = m_tex_w;
    int height_pot = m_tex_h;

    int xp = image.GetWidth();
    if (((xp != 0) && !(xp & (xp - 1))))  // detect POT
      width_pot = xp;
    else {
      int a = 0;
      while (xp) {
        xp = xp >> 1;
        a++;
      }
      width_pot = 1 << a;
    }

    xp = image.GetHeight();
    if (((xp != 0) && !(xp & (xp - 1))))
      height_pot = xp;
    else {
      int a = 0;
      while (xp) {
        xp = xp >> 1;
        a++;
      }
      height_pot = 1 << a;
    }

    m_tex_w = width_pot;
    m_tex_h = height_pot;

    GLuint format = GL_RGBA;
    GLuint internalformat = GL_RGBA8; //format;
    int stride = 4;

    if (imgdata) {
      unsigned char* teximage =
          (unsigned char*)malloc(stride * m_tex_w * m_tex_h);

      for (int i = 0; i < m_image_height; i++) {
        for (int j = 0; j < m_image_width; j++) {
          int s = (i * 3 * m_image_width) + (j * 3);
          int d = (i * stride * m_tex_w) + (j * stride);

          teximage[d + 0] = imgdata[s + 0];
          teximage[d + 1] = imgdata[s + 1];
          teximage[d + 2] = imgdata[s + 2];
          teximage[d + 3] = 255;
        }
      }

      glBindTexture(GL_TEXTURE_2D, m_texobj);
      glEnable(GL_TEXTURE_2D);

      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_tex_w, m_tex_h, format, GL_UNSIGNED_BYTE, teximage);

      free(teximage);
      glBindTexture(GL_TEXTURE_2D, 0);
    }
  }
#endif
}
