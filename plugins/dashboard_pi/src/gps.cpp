/******************************************************************************
 * $Id: gps.cpp, v1.0 2010/08/26 SethDart Exp $
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

#include "gps.h"
#include "wx28compat.h"

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

DashboardInstrument_GPS::DashboardInstrument_GPS(wxWindow* parent,
                                                 wxWindowID id, wxString title)
    : DashboardInstrument(parent, id, title, OCPN_DBP_STC_GPS) {
  m_refDim = GetCharHeight() * 90 / 100;

  m_cx = 35;
  m_cy = m_refDim * 35 / 10;
  m_radius = m_refDim * 2;
  m_scaleDelta = m_refDim / 2;
  m_scaleBase = (m_radius * 2) + (2 * m_refDim);

  for (int idx = 0; idx < 12; idx++) {
    m_SatInfo[idx].SatNumber = 0;
    m_SatInfo[idx].ElevationDegrees = 0;
    m_SatInfo[idx].AzimuthDegreesTrue = 0;
    m_SatInfo[idx].SignalToNoiseRatio = 0;
  }
  m_SatCount = 0;
  talkerID = wxEmptyString;
  for (int i = 0; i < GNSS_SYSTEM; i++) {
    m_Gtime[i] = 10000;
  }
  m_lastShift = wxDateTime::Now();
  b_shift = false;
  s_gTalker = wxEmptyString;
  m_iMaster = 1;  // Start with the GPS system
  m_MaxSatCount = 0;
}

wxSize DashboardInstrument_GPS::GetSize(int orient, wxSize hint) {
  wxClientDC dc(this);
  int w;
  dc.GetTextExtent(m_title, &w, &m_TitleHeight, 0, 0, g_pFontTitle);
  w = (12 * m_refDim);  // Max 12 vertical bars
  if (orient == wxHORIZONTAL) {
    m_cx = w / 2;
    return wxSize(w, wxMax(hint.y, m_TitleHeight + (m_refDim * 84 / 10)));
  } else {
    w = wxMax(hint.x, w);
    m_cx = w / 2;
    return wxSize(w, m_TitleHeight + (m_refDim * 84 / 10));
  }
}

void DashboardInstrument_GPS::SetSatInfo(int cnt, int seq, wxString talk,
                                         SAT_INFO sats[4]) {
  m_SatCount = cnt;
  talkerID = talk;

  /* Some GNSS receivers may emit more than (3*4)=12 sats info.
     We read the three first only since our graphic is
     is not adapted for more than 12 satellites*/
  if (seq < 1 || seq > 3) return;

  if (talkerID != wxEmptyString) {
    /* Switch view between the six GNSS system
       mentioned in NMEA0183, when available.
       Show each system for 30 seconds.
       Time to shift now? */
    wxDateTime now = wxDateTime::Now();
    wxTimeSpan sinceLastShift = now - m_lastShift;
    if (sinceLastShift.GetSeconds() > 30) {
      b_shift = true;
      m_lastShift = now;
    }
    if (b_shift) {
      // Who's here and in turn to show up next
      bool secondturn = false;
      int im = m_iMaster == GNSS_SYSTEM - 1 ? 0 : m_iMaster + 1;
      for (int i = im; i < GNSS_SYSTEM; i++) {
        wxTimeSpan lastUpdate = now - m_Gtime[i];
        if (lastUpdate.GetSeconds() < 6) {
          m_iMaster = i;
          b_shift = false;
          m_MaxSatCount = 0;
          break;
        }
        if (i == 5 && !secondturn) {
          i = -1;
          secondturn = true;
        }
      }
    }
    if (talkerID == _T("GP")) {
      m_Gtime[1] = now;
      if (m_iMaster != 1) return;
      // If two groups of messages in this sequence 
      // show only the first one with most(12) satellites
      if (m_MaxSatCount > m_SatCount) return;
      else m_MaxSatCount = m_SatCount;
      s_gTalker = wxString::Format(_T("GPS\n%d"), m_SatCount);
    } else if (talkerID == _T("GL")) {
      m_Gtime[2] = now;
      if (m_iMaster != 2) return;
      // See "GP" above
      if (m_MaxSatCount > m_SatCount) return;
      else m_MaxSatCount = m_SatCount;
      s_gTalker = wxString::Format(_T("GLONASS\n%d"), m_SatCount);
    } else if (talkerID == _T("GA")) {
      m_Gtime[3] = now;
      if (m_iMaster != 3) return;
      // See "GP" above
      if (m_MaxSatCount > m_SatCount) return;
      else m_MaxSatCount = m_SatCount;
      s_gTalker = wxString::Format(_T("Galileo\n%d"), m_SatCount);
    } else if (talkerID == _T("GB")) {  // BeiDou  BDS
      m_Gtime[4] = now;
      if (m_iMaster != 4) return;
      // See "GP" above
      if (m_MaxSatCount > m_SatCount) return;
      else m_MaxSatCount = m_SatCount;
      s_gTalker = wxString::Format(_T("BeiDou\n%d"), m_SatCount);
    } else if (talkerID == _T("GI")) {
      m_Gtime[5] = now;
      if (m_iMaster != 5) return;
      // See "GP" above
      if (m_MaxSatCount > m_SatCount) return;
      else m_MaxSatCount = m_SatCount;
      s_gTalker = wxString::Format(_T("NavIC\n%d"), m_SatCount);
    } else if (talkerID == _T("GQ")) {
      m_Gtime[0] = now;
      if (m_iMaster != 0) return;
      // See "GP" above
      if (m_MaxSatCount > m_SatCount) return;
      else m_MaxSatCount = m_SatCount;
      s_gTalker = wxString::Format(_T("QZSS\n%d"), m_SatCount);
    }
    else {
      // Would be a not known N2k PGP type like "Combined GPS/GLONASS"
      s_gTalker = wxString::Format(_T("%s\n%d"), talkerID , m_SatCount);
    }
  }

  int lidx = (seq - 1) * 4;
  for (int idx = 0; idx < 4; idx++) {
    m_SatInfo[lidx + idx].SatNumber = sats[idx].SatNumber;
    m_SatInfo[lidx + idx].ElevationDegrees = sats[idx].ElevationDegrees;
    m_SatInfo[lidx + idx].AzimuthDegreesTrue = sats[idx].AzimuthDegreesTrue;
    m_SatInfo[lidx + idx].SignalToNoiseRatio = sats[idx].SignalToNoiseRatio;
  }
  // Clean out possible leftovers.
  for (int idx = m_SatCount; idx < 12; idx++) {
    m_SatInfo[idx].SatNumber = 0;
    m_SatInfo[idx].SignalToNoiseRatio = 0;
  }
}

void DashboardInstrument_GPS::Draw(wxGCDC* dc) {
  DrawFrame(dc);
  DrawBackground(dc);
  DrawForeground(dc);
}

void DashboardInstrument_GPS::DrawFrame(wxGCDC* dc) {
  wxSize size = GetClientSize();
  wxColour cb;

  GetGlobalColor(_T("DASHB"), &cb);
  dc->SetTextBackground(cb);
  dc->SetBackgroundMode(wxSOLID);

  wxColour cl;
  GetGlobalColor(_T("DASHL"), &cl);
  dc->SetTextForeground(cl);
  dc->SetBrush(*wxTRANSPARENT_BRUSH);

  wxPen pen;
  pen.SetStyle(wxPENSTYLE_SOLID);
  wxColour cf;
  GetGlobalColor(_T("DASHF"), &cf);
  pen.SetColour(cf);
  pen.SetWidth(1);
  dc->SetPen(pen);

  dc->DrawCircle(m_cx, m_cy, m_radius);

  dc->SetFont(*g_pFontSmall);

  wxScreenDC sdc;
  int height, width;
  sdc.GetTextExtent(_T("W"), &width, &height, NULL, NULL, g_pFontSmall);

  wxBitmap tbm(width, height, -1);
  wxMemoryDC tdc(tbm);
  tdc.SetBackground(cb);
  tdc.SetTextForeground(cl);
  tdc.SetTextBackground(cb);
  tdc.SetBackgroundMode(wxSOLID);
  tdc.SetFont(*g_pFontSmall);

  tdc.Clear();
  tdc.DrawText(_("N"), 0, 0);
  dc->Blit(m_cx - 3, m_cy - m_radius - 6, width, height, &tdc, 0, 0);

  tdc.Clear();
  tdc.DrawText(_("E"), 0, 0);
  dc->Blit(m_cx + m_radius - 4, m_cy - 5, width, height, &tdc, 0, 0);

  tdc.Clear();
  tdc.DrawText(_("S"), 0, 0);
  dc->Blit(m_cx - 3, m_cy + m_radius - 6, width, height, &tdc, 0, 0);

  tdc.Clear();
  tdc.DrawText(_("W"), 0, 0);
  dc->Blit(m_cx - m_radius - 4, m_cy - 5, width, height, &tdc, 0, 0);

  tdc.SelectObject(wxNullBitmap);

  dc->SetBackgroundMode(wxTRANSPARENT);

  dc->DrawLine(3, m_scaleBase, size.x - 3, m_scaleBase);
  dc->DrawLine(3, m_scaleBase + 4 * m_scaleDelta, size.x - 3,
               m_scaleBase + 4 * m_scaleDelta);

  pen.SetStyle(wxPENSTYLE_DOT);
  dc->SetPen(pen);
  dc->DrawCircle(m_cx, m_cy, m_radius * sin(deg2rad(45)));
  dc->DrawCircle(m_cx, m_cy, m_radius * sin(deg2rad(20)));

  //        wxSHORT_DASH is not supported on GTK, and it destroys the pen.
#ifndef __WXGTK__
  pen.SetStyle(wxPENSTYLE_SHORT_DASH);
  dc->SetPen(pen);
#endif
  dc->DrawLine(3, m_scaleBase + 1 * m_scaleDelta, size.x - 3,
               m_scaleBase + 1 * m_scaleDelta);
  dc->DrawLine(3, m_scaleBase + 2 * m_scaleDelta, size.x - 3,
               m_scaleBase + 2 * m_scaleDelta);
  dc->DrawLine(3, m_scaleBase + 3 * m_scaleDelta, size.x - 3,
               m_scaleBase + 3 * m_scaleDelta);
}

void DashboardInstrument_GPS::DrawBackground(wxGCDC* dc) {
  // Draw SatID

  wxScreenDC sdc;
  int height, width;
  sdc.GetTextExtent(_T("W"), &width, &height, NULL, NULL, g_pFontSmall);

  wxColour cl;
  wxBitmap tbm(dc->GetSize().x, height, -1);
  wxMemoryDC tdc(tbm);
  wxColour c2;
  GetGlobalColor(_T("DASHB"), &c2);
  tdc.SetBackground(c2);
  tdc.Clear();

  tdc.SetFont(*g_pFontSmall);
  GetGlobalColor(_T("DASHF"), &cl);
  tdc.SetTextForeground(cl);

  int pitch = m_refDim;
  int offset = m_refDim * 12 / 100;
  for (int idx = 0; idx < 12; idx++) {
    if (m_SatInfo[idx].SatNumber)
      tdc.DrawText(wxString::Format(_T("%02d"), m_SatInfo[idx].SatNumber),
                   idx * pitch + offset, 0);
    else
      tdc.DrawText(" -", idx * pitch + offset, 0);
  }

  tdc.SelectObject(wxNullBitmap);

  int scaleDelta = m_refDim / 2;
  int scaleBase = (m_radius * 2) + (2 * m_refDim);

  dc->DrawBitmap(tbm, 0, scaleBase + (scaleDelta * 45 / 10), false);
}

void DashboardInstrument_GPS::DrawForeground(wxGCDC* dc) {
  wxColour cl;
  GetGlobalColor(_T("DASHL"), &cl);
  wxBrush brush(cl);
  dc->SetBrush(brush);
  dc->SetPen(*wxTRANSPARENT_PEN);
  dc->SetTextBackground(cl);

  wxColor cf;
  GetGlobalColor(_T("DASHF"), &cf);
  dc->SetTextForeground(cf);
  dc->SetBackgroundMode(wxSOLID);

  wxColour cb;
  GetGlobalColor(_T("DASHB"), &cb);

  int m_scaleDelta = m_refDim / 2;
  int m_scaleBase = (m_radius * 2) + (2 * m_refDim);
  int pitch = m_refDim;
  int offset = m_refDim * 25 / 100;

  for (int idx = 0; idx < 12; idx++) {
    if (m_SatInfo[idx].SignalToNoiseRatio) {
      int h = m_SatInfo[idx].SignalToNoiseRatio * m_refDim / 24;  // 0.4;
      dc->DrawRectangle(idx * pitch + offset,
                        m_scaleBase + (4 * m_scaleDelta) - h, pitch * 60 / 100, h);
    }
  }

  wxString label;
  for (int idx = 0; idx < 12; idx++) {
    if (m_SatInfo[idx].SignalToNoiseRatio) {
      label.Printf(_T("%02d"), m_SatInfo[idx].SatNumber);
      int width, height;
      wxScreenDC sdc;
      sdc.GetTextExtent(label, &width, &height, 0, 0, g_pFontSmall);

      wxBitmap tbm(width, height, -1);
      wxMemoryDC tdc(tbm);
      tdc.SetBackground(cb);
      tdc.Clear();

      tdc.SetFont(*g_pFontSmall);
      tdc.SetTextForeground(cf);
      tdc.SetBackgroundMode(wxSOLID);
      tdc.SetTextBackground(cl);

      tdc.DrawText(label, 0, 0);
      tdc.SelectObject(wxNullBitmap);

      int posx =
          m_cx +
          m_radius *
              cos(deg2rad(m_SatInfo[idx].AzimuthDegreesTrue - ANGLE_OFFSET)) *
              sin(deg2rad(ANGLE_OFFSET - m_SatInfo[idx].ElevationDegrees)) -
          width / 2;
      int posy =
          m_cy +
          m_radius *
              sin(deg2rad(m_SatInfo[idx].AzimuthDegreesTrue - ANGLE_OFFSET)) *
              sin(deg2rad(ANGLE_OFFSET - m_SatInfo[idx].ElevationDegrees)) -
          height / 2;
      dc->DrawBitmap(tbm, posx, posy, false);
    }
  }
  if (talkerID != wxEmptyString) dc->DrawText(s_gTalker, 1, m_refDim);
}
