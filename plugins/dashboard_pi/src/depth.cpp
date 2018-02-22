/******************************************************************************
 * $Id: depth.cpp, v1.0 2010/08/30 SethDart Exp $
 *
 * Project:  OpenCPN
 * Purpose:  Dashboard Plugin
 * Author:   Jean-Eudes Onfray
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
 *   bdbcat@yahoo.com   *
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

#include "depth.h"

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
      m_Temp = _T("--");
      for (int idx = 0; idx < DEPTH_RECORD_COUNT; idx++)
      {
            m_ArrayDepth[idx] = 0;
      }

      SetInstrumentWidth(200);
}

void DashboardInstrument_Depth::SetInstrumentWidth(int width)
{
      wxClientDC dc(this);
      int w;
      dc.GetTextExtent(m_title, &w, &m_TitleHeight, 0, 0, g_pFontTitle);
      m_width = width;
      m_height = m_TitleHeight+140;
      SetMinSize(wxSize(m_width, m_height));
      Refresh(false);
}

void DashboardInstrument_Depth::SetData(int st, double data, wxString unit)
{
      if (st == OCPN_DBP_STC_DPT)
      {
            m_Depth = data;

            for (int idx = 1; idx < DEPTH_RECORD_COUNT; idx++)
            {
                  m_ArrayDepth[idx-1] = m_ArrayDepth[idx];
            }
            m_ArrayDepth[DEPTH_RECORD_COUNT-1] = data;

            Refresh(false);
      }
      else if (st == OCPN_DBP_STC_TMP)
      {
            m_Temp.Printf(_T("%2.1f°"), data);
            m_Temp.Append(unit);
      }
}

void DashboardInstrument_Depth::Draw(wxBufferedDC* dc)
{
      DrawBackground(dc);
      DrawForeground(dc);
}

void DashboardInstrument_Depth::DrawBackground(wxBufferedDC* dc)
{
      wxRect rect = GetClientRect();
      wxColour cl;

      GetGlobalColor(_T("BLUE2"), &cl);
      dc->SetTextForeground(cl);

      wxPen pen;
      pen.SetStyle(wxSOLID);
      pen.SetColour(cl);
      dc->SetPen(pen);

      dc->DrawLine(3, 40, rect.width-3, 40);
      dc->DrawLine(3, 140, rect.width-3, 140);

      pen.SetStyle(wxSHORT_DASH);
      dc->SetPen(pen);
      dc->DrawLine(3, 65, rect.width-3, 65);
      dc->DrawLine(3, 90, rect.width-3, 90);
      dc->DrawLine(3, 115, rect.width-3, 115);

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
      label.Printf(_T("%5.0f m"), 0.0);
      int width, height;
      dc->GetTextExtent(label, &width, &height, 0, 0, g_pFontSmall);
      dc->DrawText(label, rect.width-width, 40-height);

      label.Printf(_T("%5.0f m"), m_MaxDepth);
      dc->GetTextExtent(label, &width, &height, 0, 0, g_pFontSmall);
      dc->DrawText(label, rect.width-width, rect.height-height);
}

void DashboardInstrument_Depth::DrawForeground(wxBufferedDC* dc)
{
      wxRect rect = GetClientRect();
      dc->SetFont(*g_pFontData);
      dc->DrawText(wxString::Format(_T("%5.1f m"), m_Depth), 10, m_TitleHeight);

      dc->SetFont(*g_pFontLabel);
      int width, height;
      dc->GetTextExtent(m_Temp, &width, &height, 0, 0, g_pFontLabel);
      dc->DrawText(m_Temp, 0, rect.height-height);

      wxColour cl;
      GetGlobalColor(_T("BLUE1"), &cl);
      wxBrush brush;
      brush.SetStyle(wxSOLID);
      brush.SetColour(cl);
      dc->SetBrush(brush);
      dc->SetPen(*wxTRANSPARENT_PEN);

      double ratioH = 100.0 / m_MaxDepth; // 140-40=100
      double ratioW = double(rect.width - 6) / (DEPTH_RECORD_COUNT-1);
      wxPoint points[DEPTH_RECORD_COUNT+2];
      for (int idx = 0; idx < DEPTH_RECORD_COUNT; idx++)
      {
            points[idx].x = idx * ratioW + 3;
            if (m_ArrayDepth[idx])
                  points[idx].y = 40 + m_ArrayDepth[idx] * ratioH;
            else
                  points[idx].y = 140;
      }
      points[DEPTH_RECORD_COUNT].x = rect.width - 3;
      points[DEPTH_RECORD_COUNT].y = 140;
      points[DEPTH_RECORD_COUNT+1].x = 3;
      points[DEPTH_RECORD_COUNT+1].y = 140;
      dc->DrawPolygon(DEPTH_RECORD_COUNT+2, points);
}

