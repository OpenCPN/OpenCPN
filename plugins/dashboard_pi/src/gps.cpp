/******************************************************************************
 * $Id: gps.cpp, v1.0 2010/08/26 SethDart Exp $
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

#include "gps.h"

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

// Required deg2rad
#include "dial.h"

DashboardInstrument_GPS::DashboardInstrument_GPS( wxWindow *parent, wxWindowID id, wxString title) :
      DashboardInstrument(parent, id, title, OCPN_DBP_STC_GPS)
{
      m_cx = 35;
      m_cy = 57;
      m_radius = 35;

      m_SatCount = 0;
      for (int idx = 0; idx < 12; idx++)
      {
            m_SatInfo[idx].SatNumber = 0;
            m_SatInfo[idx].ElevationDegrees = 0;
            m_SatInfo[idx].AzimuthDegreesTrue = 0;
            m_SatInfo[idx].SignalToNoiseRatio = 0;
      }

      SetInstrumentWidth(200);
}

void DashboardInstrument_GPS::SetInstrumentWidth(int width)
{
      wxClientDC dc(this);
      int w;
      dc.GetTextExtent(m_title, &w, &m_TitleHeight, 0, 0, g_pFontTitle);
      m_cx = width/2;
      m_width = width;
      m_height = m_TitleHeight+140;
      SetMinSize(wxSize(m_width, m_height));
      Refresh(false);
}

void DashboardInstrument_GPS::SetSatInfo(int cnt, int seq, SAT_INFO sats[4])
{
      m_SatCount = cnt;
      // Some GPS receivers may emit more than 12 sats info
      if (seq < 1 || seq > 3)
            return;

      int lidx = (seq-1)*4;
      for (int idx = 0; idx < 4; idx++)
      {
            m_SatInfo[lidx+idx].SatNumber = sats[idx].SatNumber;
            m_SatInfo[lidx+idx].ElevationDegrees = sats[idx].ElevationDegrees;
            m_SatInfo[lidx+idx].AzimuthDegreesTrue = sats[idx].AzimuthDegreesTrue;
            m_SatInfo[lidx+idx].SignalToNoiseRatio = sats[idx].SignalToNoiseRatio;
      }

      Refresh(false);
}

void DashboardInstrument_GPS::Draw(wxBufferedDC* dc)
{
      DrawFrame(dc);
      DrawBackground(dc);
      DrawForeground(dc);
}

void DashboardInstrument_GPS::DrawFrame(wxBufferedDC* dc)
{
      wxRect rect = GetClientRect();
      wxColour cl;

      GetGlobalColor(_T("DILG1"), &cl);
      dc->SetTextBackground(cl);
      dc->SetBackgroundMode(wxSOLID);
      GetGlobalColor(_T("BLUE2"), &cl);
      dc->SetTextForeground(cl);
      dc->SetBrush(*wxTRANSPARENT_BRUSH);

      wxPen pen;
      pen.SetStyle(wxSOLID);
      GetGlobalColor(_T("BLUE2"), &cl);
      pen.SetColour(cl);
      dc->SetPen(pen);

      dc->DrawCircle(m_cx, m_cy, m_radius);

      dc->SetFont(*g_pFontSmall);
      dc->DrawText(_("N"), m_cx-3, m_cy-m_radius-6);
      dc->DrawText(_("E"), m_cx+m_radius-4, m_cy-5);
      dc->DrawText(_("S"), m_cx-3, m_cy+m_radius-6);
      dc->DrawText(_("W"), m_cx-m_radius-4, m_cy-5);

      dc->SetBackgroundMode(wxTRANSPARENT);

      dc->DrawLine(3, 100, rect.width-3, 100);
      dc->DrawLine(3, 140, rect.width-3, 140);

      pen.SetStyle(wxDOT);
      dc->SetPen(pen);
      dc->DrawCircle(m_cx, m_cy, m_radius * sin(deg2rad(45)));
      dc->DrawCircle(m_cx, m_cy, m_radius * sin(deg2rad(20)));

      pen.SetStyle(wxSHORT_DASH);
      dc->SetPen(pen);
      dc->DrawLine(3, 110, rect.width-3, 110);
      dc->DrawLine(3, 120, rect.width-3, 120);
      dc->DrawLine(3, 130, rect.width-3, 130);
}

void DashboardInstrument_GPS::DrawBackground(wxBufferedDC* dc)
{
      dc->SetFont(*g_pFontSmall);
      // Draw SatID
      for (int idx = 0; idx < 12; idx++)
      {
            //if (m_SatInfo[idx].SignalToNoiseRatio)
            if (m_SatInfo[idx].SatNumber)
                  dc->DrawText(wxString::Format(_T("%02d"), m_SatInfo[idx].SatNumber), idx*16+5, 142);
      }
}

void DashboardInstrument_GPS::DrawForeground(wxBufferedDC* dc)
{
      wxColour cl;
      GetGlobalColor(_T("BLUE1"), &cl);
      wxBrush brush;
      brush.SetStyle(wxSOLID);
      brush.SetColour(cl);
      dc->SetBrush(brush);
      dc->SetPen(*wxTRANSPARENT_PEN);
      dc->SetTextBackground(cl);
      GetGlobalColor(_T("BLUE2"), &cl);
      dc->SetTextForeground(cl);
      dc->SetBackgroundMode(wxSOLID);

      wxString label;
      wxFont font = dc->GetFont();
      for (int idx = 0; idx < 12; idx++)
      {
            if (m_SatInfo[idx].SignalToNoiseRatio)
            {
                  dc->DrawRectangle(idx*16+5, 140, 14, m_SatInfo[idx].SignalToNoiseRatio * -0.4);
                  label.Printf(_T("%02d"), m_SatInfo[idx].SatNumber);
                  int width, height;
                  dc->GetTextExtent(label, &width, &height, 0, 0, &font);
                  int posx = m_cx + m_radius * cos(deg2rad(m_SatInfo[idx].AzimuthDegreesTrue-ANGLE_OFFSET)) * sin(deg2rad(ANGLE_OFFSET-m_SatInfo[idx].ElevationDegrees)) - width/2;
                  int posy = m_cy + m_radius * sin(deg2rad(m_SatInfo[idx].AzimuthDegreesTrue-ANGLE_OFFSET)) * sin(deg2rad(ANGLE_OFFSET-m_SatInfo[idx].ElevationDegrees)) - height/2;
                  dc->DrawText(label, posx, posy);
            }
      }
}

