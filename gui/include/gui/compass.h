/**************************************************************************
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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/
#ifndef GUI_COMPASS_H
#define GUI_COMPASS_H

/**
 * \file
 *
 * Compass display state
 */

#include "chcanv.h"
#include "color_types.h"
#include "ocpndc.h"

class ChartCanvas;  // circular

/**
 * Represents a compass display in the OpenCPN navigation system.
 *
 * The ocpnCompass class provides a visual compass display for the OpenCPN
 * application. It shows the current heading and, optionally, GPS status.
 * This compass is typically displayed as an overlay on the main chart view.
 */
class ocpnCompass {
public:
  ocpnCompass(ChartCanvas *parent, bool bShowGPS = true);
  ~ocpnCompass();

  bool IsShown() const { return m_shown; }
  void Show(bool show) { m_shown = show; }
  void Paint(ocpnDC &dc);

  void UpdateStatus(bool newColorScheme = false);

  bool MouseEvent(wxMouseEvent &event);
  void SetColorScheme(ColorScheme cs);
  int GetXOffset(void) const { return m_xoffset; }
  int GetYOffset(void) const { return m_yoffset; }
  float GetScaleFactor() { return m_scale; }
  void SetScaleFactor(float factor);

  void Move(const wxPoint &pt) { m_rect.SetPosition(pt); }
  /**
   * Return the coordinates of the compass widget, in physical pixels relative
   * to the canvas window. Beware when comparing with data returned from
   * wxWidgets APIs, which return logical pixels.
   */
  wxRect GetRect(void) const { return m_rect; }
  /**
   * Return the coordinates of the compass widget, in logical pixels.
   * This can be compared with data returned from wxWidgets APIs.
   */
  wxRect GetLogicalRect(void) const;

private:
  void CreateBmp(bool bnew = false);
  void CreateTexture();
  void UpdateTexture();

  ChartCanvas *m_parent;
  wxBitmap m_StatBmp;
  wxBitmap m_MaskBmp;
  wxStaticBitmap *m_pStatBoxToolStaticBmp;

  wxString m_lastgpsIconName;
  double m_rose_angle;

  wxBitmap _img_compass;
  wxBitmap _img_gpsRed;
  int m_xoffset;
  int m_yoffset;
  float m_scale;

  /**
   * The coordinates of the compass widget, in physical pixels relative to the
   * canvas window.
   */
  wxRect m_rect;
  bool m_shown;
  bool m_bshowGPS;
  ColorScheme m_cs;
  bool m_texOK;

#ifdef ocpnUSE_GL
  unsigned int m_texobj;
  int m_tex_w, m_tex_h;
  int m_image_width, m_image_height;
#endif
};

#endif  //  GUI_COMPASS_H
