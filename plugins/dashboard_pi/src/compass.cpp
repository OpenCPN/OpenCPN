/******************************************************************************
 * $Id: compass.cpp, v1.0 2010/08/05 SethDart Exp $
 *
 * Project:  OpenCPN
 * Purpose:  Dashboard Plugin
 * Author:   Jean-Eudes Onfray
 *           (Inspired by original work from Andreas Heiming)
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.             *
 ***************************************************************************
 */

#include "compass.h"

// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers)
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

DashboardInstrument_Compass::DashboardInstrument_Compass( wxWindow *parent, wxWindowID id, wxString title, int cap_flag) :
      DashboardInstrument_Dial( parent, id, title, cap_flag, 0, 360, 0, 360)
{
      SetOptionMarker(5, DIAL_MARKER_SIMPLE, 2);
      SetOptionLabel(20, DIAL_LABEL_ROTATED);
      SetOptionMainValue( _T("%.0f"), DIAL_POSITION_INSIDE);
}

void DashboardInstrument_Compass::SetData(int st, double data, wxString unit)
{
      if (st == m_MainValueCap)
      {
            // Rotate the rose
            m_AngleStart = -data;
            // Required to display data
            m_MainValue = data;
            m_MainValueUnit = unit;
      }
      else if (st == m_ExtraValueCap)
      {
            m_ExtraValue = data;
            m_ExtraValueUnit = unit;
      }
}

void DashboardInstrument_Compass::DrawBackground(wxGCDC* dc)
{
    DrawBoat( dc, m_cx, m_cy, m_radius );
    DrawCompassRose( dc, m_cx, m_cy, 0.7 * m_radius, m_AngleStart, true );
}

void DashboardInstrument_Compass::DrawForeground(wxGCDC* dc)
{
      // We dont want the default foreground (arrow) drawn
}
