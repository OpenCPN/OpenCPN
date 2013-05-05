/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  OpenCPN Main wxWidgets Program
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
 **************************************************************************/

class ocpnFloatingCompassWindow : public wxDialog
{
public:
      ocpnFloatingCompassWindow( wxWindow *parent );
      ~ocpnFloatingCompassWindow();
      void OnPaint( wxPaintEvent& event );
      wxBitmap CreateBmp( bool bnew = false );
      void UpdateStatus( bool newColorScheme = false );

      void OnClose( wxCloseEvent& event );
      void OnToolLeftClick( wxCommandEvent& event );
      void MouseEvent( wxMouseEvent& event );
      void SetColorScheme( ColorScheme cs );
      int GetXOffset(void) const { return m_xoffset; }
      int GetYOffset(void) const { return m_yoffset; }

private:
      wxBitmap m_StatBmp;
      wxBitmap m_MaskBmp;
      wxStaticBitmap *m_pStatBoxToolStaticBmp;

      wxWindow *m_pparent;
      wxBoxSizer *m_topSizer;
      wxString m_lastgpsIconName;
      double m_rose_angle;

      wxBitmap _img_compass;
      wxBitmap _img_gpsRed;
      int m_xoffset;
      int m_yoffset;

      DECLARE_EVENT_TABLE()
};


