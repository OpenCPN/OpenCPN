/******************************************************************************
 * $Id: depth.cpp, v1.0 2010/08/30 SethDart Exp $
 *
 * Project:  OpenCPN
 * Purpose:  Dashboard Plugin
 * Author:   Jean-Eudes Onfray
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

#include "depth.h"
#include "wx28compat.h"
extern int g_iDashDepthUnit;


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

DashboardInstrument_Depth::DashboardInstrument_Depth( wxWindow *parent, wxWindowID id, wxString title) :
      DashboardInstrument(parent, id, title, OCPN_DBP_STC_DPT | OCPN_DBP_STC_TMP)
{
      m_MaxDepth = 0;
      m_Depth = 0;
      m_DepthUnit = getUsrDistanceUnit_Plugin( g_iDashDepthUnit );
      m_Temp = _T("--");
      for (int idx = 0; idx < DEPTH_RECORD_COUNT; idx++)
      {
            m_ArrayDepth[idx] = 0;
      }
}

wxSize DashboardInstrument_Depth::GetSize( int orient, wxSize hint )
{
      wxClientDC dc(this);
      int w;
      dc.GetTextExtent(m_title, &w, &m_TitleHeight, 0, 0, g_pFontTitle);
      if( orient == wxHORIZONTAL ) {
          return wxSize( DefaultWidth, wxMax(m_TitleHeight+140, hint.y) );
      } else {
          return wxSize( wxMax(hint.x, DefaultWidth), m_TitleHeight+140 );
      }
}

void DashboardInstrument_Depth::SetData(int st, double data, wxString unit)
{
      if (st == OCPN_DBP_STC_DPT)
      {
            m_Depth = data;

            for (int idx = 1; idx < DEPTH_RECORD_COUNT; idx++)
            {
                m_ArrayDepth[idx - 1] = m_ArrayDepth[idx];
            }
            m_ArrayDepth[DEPTH_RECORD_COUNT - 1] = data;
            m_DepthUnit = unit;
      }
      else if (st == OCPN_DBP_STC_TMP)
      {
            m_Temp = wxString::Format(_T("%.1f"), data)+DEGREE_SIGN+unit;
      }
}

void DashboardInstrument_Depth::Draw(wxGCDC* dc)
{
      DrawBackground(dc);
      DrawForeground(dc);
}

void DashboardInstrument_Depth::DrawBackground(wxGCDC* dc)
{
      wxSize size = GetClientSize();
      wxColour cl;

      GetGlobalColor(_T("DASHL"), &cl);
      dc->SetTextForeground(cl);

      wxPen pen;
      pen.SetStyle(wxPENSTYLE_SOLID);
      GetGlobalColor(_T("DASHF"), &cl);
      pen.SetColour(cl);
      pen.SetWidth(1);
      dc->SetPen(pen);

      dc->DrawLine(3, 50, size.x-3, 50);
      dc->DrawLine(3, 140, size.x-3, 140);

#ifdef __WXMSW__      
      pen.SetStyle(wxPENSTYLE_SHORT_DASH);
#else
      pen.SetStyle(wxPENSTYLE_DOT);
      pen.SetWidth(1);
#endif      
      
      dc->SetPen(pen);
      dc->DrawLine(3, 65, size.x-3, 65);
      dc->DrawLine(3, 90, size.x-3, 90);
      dc->DrawLine(3, 115, size.x-3, 115);

      dc->SetFont(*g_pFontSmall);

      m_MaxDepth = 0;
      for (int idx = 0; idx < DEPTH_RECORD_COUNT; idx++)
      {
            if (m_ArrayDepth[idx] > m_MaxDepth)
                  m_MaxDepth = m_ArrayDepth[idx];
      }
      // Increase MaxDepth slightly for nicer display
      m_MaxDepth *= 1.2;

      wxString label;
      label.Printf(_T("%.0f ")+m_DepthUnit, 0.0);
      int width, height;
      dc->GetTextExtent(label, &width, &height, 0, 0, g_pFontSmall);
      dc->DrawText(label, size.x-width-1, 40-height);

      label.Printf(_T("%.0f ")+m_DepthUnit, m_MaxDepth);
      dc->GetTextExtent(label, &width, &height, 0, 0, g_pFontSmall);
      dc->DrawText(label, size.x-width-1, size.y-height);
}

void DashboardInstrument_Depth::DrawForeground(wxGCDC* dc)
{
      wxSize size = GetClientSize();
      wxColour cl;

      GetGlobalColor(_T("DASHL"), &cl);
      wxBrush brush;
      brush.SetStyle(wxBRUSHSTYLE_SOLID);
      brush.SetColour(cl);
      dc->SetBrush(brush);
      dc->SetPen(*wxTRANSPARENT_PEN);

      double ratioH = 100.0 / m_MaxDepth; // 140-40=100
      double ratioW = double(size.x-6) / (DEPTH_RECORD_COUNT-1);
      wxPoint points[DEPTH_RECORD_COUNT+2];
      for (int idx = 0; idx < DEPTH_RECORD_COUNT; idx++)
      {
            points[idx].x = idx * ratioW + 3;
            if (m_ArrayDepth[idx])
                  points[idx].y = 40 + m_ArrayDepth[idx] * ratioH;
            else
                  points[idx].y = 140;
      }
      points[DEPTH_RECORD_COUNT].x = size.x-3;
      points[DEPTH_RECORD_COUNT].y = 140;
      points[DEPTH_RECORD_COUNT+1].x = 3;
      points[DEPTH_RECORD_COUNT+1].y = 140;
      dc->DrawPolygon(DEPTH_RECORD_COUNT+2, points);
      
      GetGlobalColor(_T("DASHF"), &cl);
      dc->SetTextForeground( cl );
      dc->SetFont(*g_pFontData);
      if (m_DepthUnit != _T("-") & m_Depth != NAN) { //Watchdog
          dc->DrawText(wxString::Format(_T("%.1f "), m_Depth) + m_DepthUnit, 10, m_TitleHeight); 
      } else
          dc->DrawText(_T("---"), 10, m_TitleHeight);

      dc->SetFont(*g_pFontLabel);
      int width, height;
      dc->GetTextExtent(m_Temp, &width, &height, 0, 0, g_pFontLabel);
      dc->DrawText(m_Temp, 0, size.y-height);
}

