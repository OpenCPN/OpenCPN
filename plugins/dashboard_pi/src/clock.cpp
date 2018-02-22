/******************************************************************************
 * $Id: gps.cpp, v1.0 2011/05/15 nohal Exp $
 *
 * Project:  OpenCPN
 * Purpose:  Dashboard Plugin
 * Author:   Pavel Kalian
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
 *   bdbcat@yahoo.com                                                      *
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

#include "clock.h"

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

DashboardInstrument_Clock::DashboardInstrument_Clock( wxWindow *parent, wxWindowID id, wxString title) :
      DashboardInstrument_Single(parent, id, title, OCPN_DBP_STC_CLK, _T("%02i:%02i:%02i UTC"))
{
}

void DashboardInstrument_Clock::SetInstrumentWidth(int width)
{
      wxClientDC dc(this);
      int w;
      dc.GetTextExtent(m_title, &w, &m_TitleHeight, 0, 0, g_pFontTitle);
      dc.GetTextExtent(_T("00:00:00 UTC"), &w, &m_DataHeight, 0, 0, g_pFontData);

      m_width = width;
      m_height = m_TitleHeight+m_DataHeight;
      SetMinSize(wxSize(m_width, m_height));
      Refresh(false);
}

void DashboardInstrument_Clock::SetUtcTime(int st, wxDateTime data)
{
      if (m_cap_flag & st)
      {
            if (data.IsValid())
            {
                  m_data = data.ToUTC().FormatISOTime().Append(_T(" UTC"));
            }

            Refresh(false);
      }
}

DashboardInstrument_Moon::DashboardInstrument_Moon( wxWindow *parent, wxWindowID id, wxString title) :
      DashboardInstrument_Single(parent, id, title, OCPN_DBP_STC_MON, _T("%i/4 %c"))
{
}

void DashboardInstrument_Moon::SetUtcTime(int st, wxDateTime data)
{
      if (m_cap_flag & st)
      {
            if (data.IsValid())
            {
                  int phase = moon_phase(data.ToUTC().GetYear(), data.ToUTC().GetMonth() + 1, data.ToUTC().GetDay());
                  char sign = ' ';
                  if (phase > 0 && phase < 4)
                        sign = '+';
                  else if (phase > 4)
                  {
                        sign = '-';
                        phase = 8 - phase;
                  }
                  m_data = wxString::Format(m_format, phase, sign);
            }

            Refresh(false);
      }
}

// Moon phase calculation
int moon_phase(int y, int m, int d)
{
    /*
      calculates the moon phase (0-7), accurate to 1 segment.
      0 = > new moon.
      4 => full moon.
      */

    int c,e;
    double jd;
    int b;

    if (m < 3) {
        y--;
        m += 12;
    }
    ++m;
    c = 365.25*y;
    e = 30.6*m;
    jd = c+e+d-694039.09;  /* jd is total days elapsed */
    jd /= 29.53;           /* divide by the moon cycle (29.53 days) */
    b = jd;		   /* int(jd) -> b, take integer part of jd */
    jd -= b;		   /* subtract integer part to leave fractional part of original jd */
    b = jd*8 + 0.5;	   /* scale fraction from 0-8 and round by adding 0.5 */
    b = b & 7;		   /* 0 and 8 are the same so turn 8 into 0 */
    return b;
}
