/***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
 *   Copyright (C) 2025 by NoCodeHummel                                    *
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
 * Implement printout_chart.h -- Print chart canvas mix-in
 */

#include <wx/dc.h>
#include <wx/utils.h>

#include "model/config_vars.h"

#include "chcanv.h"
#include "gl_chart_canvas.h"
#include "printout_chart.h"
#include "top_frame.h"

void ChartPrintout::DrawPage(wxDC* dc, int page) {
  // Get the Size of the Chart Canvas
  int sx, sy;
  top_frame::Get()->GetAbstractPrimaryCanvas()->GetClientSize(&sx, &sy);
  // of the canvas

  float max_x = sx;
  float max_y = sy;

  // Let's have at least some device units margin
  float margin_x = 50;
  float margin_y = 50;

  // Add the margin to the graphic size
  max_x += (2 * margin_x);
  max_y += (2 * margin_y);

  // Get the size of the DC in pixels
  int w, h;
  dc->GetSize(&w, &h);

  // Calculate a suitable scaling factor
  float scale_x = (float)(w / max_x);
  float scale_y = (float)(h / max_y);

  // Use x or y scaling factor, whichever fits on the DC
  float actual_scale = wxMin(scale_x, scale_y);

  // Calculate the position on the DC for centring the graphic
  float pos_x = (float)((w - (max_x * actual_scale)) / 2.0);
  float pos_y = (float)((h - (max_y * actual_scale)) / 2.0);

  pos_x = wxMax(pos_x, margin_x);
  pos_y = wxMax(pos_y, margin_y);

  // Set the scale and origin
  dc->SetUserScale(actual_scale, actual_scale);
  dc->SetDeviceOrigin((long)pos_x, (long)pos_y);

  //  Get the latest bitmap as rendered by the ChartCanvas

  if (g_bopengl) {
#ifdef ocpnUSE_GL
    if (m_gl_bmp.IsOk()) {
      wxMemoryDC mdc;
      mdc.SelectObject(m_gl_bmp);
      dc->Blit(0, 0, m_gl_bmp.GetWidth(), m_gl_bmp.GetHeight(), &mdc, 0, 0);
      mdc.SelectObject(wxNullBitmap);
    }
#endif
  } else {
    //  And Blit/scale it onto the Printer DC
    wxMemoryDC mdc;
    wxBitmap* scratch_bitmap =
        top_frame::Get()->GetAbstractPrimaryCanvas()->GetScratchBitmap();
    mdc.SelectObject(*scratch_bitmap);
    dc->Blit(0, 0, scratch_bitmap->GetWidth(), scratch_bitmap->GetHeight(),
             &mdc, 0, 0);

    mdc.SelectObject(wxNullBitmap);
  }
}

void ChartPrintout::GenerateGLbmp() {
  if (g_bopengl) {
#ifdef ocpnUSE_GL
    int gsx = top_frame::Get()->GetWxGlCanvas()->GetSize().x;
    int gsy = top_frame::Get()->GetWxGlCanvas()->GetSize().y;

    unsigned char* buffer = (unsigned char*)malloc(gsx * gsy * 4);
    glReadPixels(0, 0, gsx, gsy, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

    unsigned char* e = (unsigned char*)malloc(gsx * gsy * 3);

    if (buffer && e) {
      for (int p = 0; p < gsx * gsy; p++) {
        e[3 * p + 0] = buffer[4 * p + 0];
        e[3 * p + 1] = buffer[4 * p + 1];
        e[3 * p + 2] = buffer[4 * p + 2];
      }
    }
    free(buffer);

    wxImage image(gsx, gsy);
    image.SetData(e);
    wxImage mir_imag = image.Mirror(false);
    m_gl_bmp = wxBitmap(mir_imag);
#endif
  }
}
