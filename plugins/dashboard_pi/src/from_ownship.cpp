/******************************************************************************
 * $Id: from_ownship.cpp
 *
 * Project:  OpenCPN
 * Purpose:  Dashboard Plugin
 * Author:   Pavel Kalian
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
 */

#include "from_ownship.h"

extern int g_iDashDistanceUnit;

//----------------------------------------------------------------
//
//    DashboardInstrument_FromOwnship Implementation
//
//----------------------------------------------------------------
DashboardInstrument_FromOwnship::DashboardInstrument_FromOwnship(wxWindow *pparent, wxWindowID id, wxString title, int cap_flag1, int cap_flag2 ,int cap_flag3,int cap_flag4)
    :DashboardInstrument(pparent, id, title, cap_flag1 | cap_flag2 | cap_flag3 | cap_flag4)
{
    m_data1 =_T("---");
    m_data2 =_T("---");
    m_cap_flag1 = cap_flag1;
    m_cap_flag2 = cap_flag2;
    m_cap_flag3 = cap_flag3;
    m_cap_flag4 = cap_flag4;
    s_lat = 99999999;
    s_lon = 99999999;
}

void DashboardInstrument_FromOwnship::Draw(wxGCDC* dc)
{
    wxColour cl;

    dc->SetFont(*g_pFontData);
    //dc.SetTextForeground(pFontMgr->GetFontColor(_T("Dashboard Data")));
    GetGlobalColor(_T("BLUE2"), &cl);
    dc->SetTextForeground(cl);

    dc->DrawText(m_data1, 10, m_TitleHeight);
    dc->DrawText(m_data2, 10, m_TitleHeight + m_DataHeight);
}

void DashboardInstrument_FromOwnship::SetData(int st, double data, wxString unit)
{
    if (st == m_cap_flag1)
    {
	      c_lat = data;
    }
    else if (st == m_cap_flag2)
    {
	      c_lon = data;
    }
	  else if (st == m_cap_flag3)
    {
	      s_lat = data;
    }
    else if (st == m_cap_flag4)
    {
          s_lon = data;
    }
    else
        return;
    if ( s_lat < 99999999 && s_lon < 99999999 )
    {
        double brg,dist;
        DistanceBearingMercator_Plugin(c_lat, c_lon, s_lat, s_lon, &brg, &dist);
        m_data1.Printf(_T("%03d ") + DEGREE_SIGN,(int)brg);
        m_data2.Printf(_T("%3.2f %s"), toUsrDistance_Plugin(dist, g_iDashDistanceUnit), getUsrDistanceUnit_Plugin(g_iDashDistanceUnit).c_str());
    }
	  	
    Refresh(false);
}

wxSize DashboardInstrument_FromOwnship::GetSize( int orient, wxSize hint )
{
      wxClientDC dc(this);
      int w;
      dc.GetTextExtent(m_title, &w, &m_TitleHeight, 0, 0, g_pFontTitle);
      dc.GetTextExtent(_T("000.00 NMi"), &w, &m_DataHeight, 0, 0, g_pFontData);

      if( orient == wxHORIZONTAL ) {
          return wxSize( w+10, wxMax(hint.y, m_TitleHeight+m_DataHeight*2) );
      } else {
          return wxSize( wxMax(hint.x, w+10), m_TitleHeight+m_DataHeight*2 );
      }
      
}
