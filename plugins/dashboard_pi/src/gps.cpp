/**************************************************************************
 *   Copyright (C) 2010 by Jean-Eudes Onfray                               *
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

/**
 * \file
 *
 * Dashboard GPS instrument implementation
 */

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "gps.h"

// Required deg2rad
#include "dial.h"

DashboardInstrument_GPS::DashboardInstrument_GPS(
    wxWindow* parent, wxWindowID id, wxString title,
    InstrumentProperties* Properties)
    : DashboardInstrument(parent, id, title, OCPN_DBP_STC_GPS, Properties) {
  m_ref_dim = wxWindow::GetCharHeight() * 80 / 100;
  m_ref_dim *= OCPN_GetWinDIPScaleFactor() < 1.0
                   ? 2.0 * OCPN_GetWinDIPScaleFactor()
                   : 1.0;  // 1.5

  m_cx = 35;
  m_cy = static_cast<int>(m_ref_dim * 35 / 10);
  m_radius = m_ref_dim * 2;
  m_scale_delta = m_ref_dim / 2;
  m_scale_base = (m_radius * 2) + (2 * m_ref_dim);

  for (int idx = 0; idx < 12; idx++) {
    m_sat_info[idx].SatNumber = 0;
    m_sat_info[idx].ElevationDegrees = 0;
    m_sat_info[idx].AzimuthDegreesTrue = 0;
    m_sat_info[idx].SignalToNoiseRatio = 0;
  }
  m_sat_count = 0;
  talker_id = wxEmptyString;
  for (int i = 0; i < GNSS_SYSTEM; i++) {
    m_gtime[i] = 10000;
  }
  m_last_shift = wxDateTime::Now();
  b_shift = false;
  m_talker = wxEmptyString;
  m_master = 1;  // Start with the GPS system
  m_max_sat_count = 0;
}

wxSize DashboardInstrument_GPS::GetSize(int orient, wxSize hint) {
  wxClientDC dc(this);
  int w;
  wxFont f;
  if (m_Properties)
    f = m_Properties->m_TitleFont.GetChosenFont();
  else
    f = g_pFontTitle->GetChosenFont();
  dc.GetTextExtent(m_title, &w, &m_TitleHeight, nullptr, nullptr, &f);
  w = (12 * m_ref_dim);  // Max 12 vertical bars
  if (orient == wxHORIZONTAL) {
    m_cx = w / 2;
    return wxSize(w, wxMax(hint.y, m_TitleHeight + (m_ref_dim * 84 / 10)));
  } else {
    w = wxMax(hint.x, w);
    m_cx = w / 2;
    return wxSize(w, m_TitleHeight + (m_ref_dim * 84 / 10));
  }
}

void DashboardInstrument_GPS::SetSatInfo(int cnt, int seq, wxString talk,
                                         SAT_INFO sats[4]) {
  m_sat_count = cnt;
  talker_id = talk;

  /* Some GNSS receivers may emit more than (3*4)=12 sats info.
     We read the three first only since our graphic is
     is not adapted for more than 12 satellites*/
  if (seq < 1 || seq > 3) return;

  if (talker_id != wxEmptyString) {
    /* Switch view between the six GNSS system
       mentioned in NMEA0183, when available.
       Show each system for 20 seconds.
       Time to shift now? */
    wxDateTime now = wxDateTime::Now();
    wxTimeSpan sinceLastShift = now - m_last_shift;
    if (sinceLastShift.GetSeconds() > 20) {
      b_shift = true;
      m_last_shift = now;
    }
    if (b_shift) {
      // Who's here and in turn to show up next
      bool secondturn = false;
      int im = m_master == GNSS_SYSTEM - 1 ? 0 : m_master + 1;
      for (int i = im; i < GNSS_SYSTEM; i++) {
        wxTimeSpan lastUpdate = now - m_gtime[i];
        if (lastUpdate.GetSeconds() < 6) {
          m_master = i;
          b_shift = false;
          m_max_sat_count = 0;
          break;
        }
        if (i == 5 && !secondturn) {
          i = -1;
          secondturn = true;
        }
      }
    }
    if (talker_id == "GP") {
      m_gtime[1] = now;
      if (m_master != 1) return;
      // If two groups of messages in this sequence
      // show only the first one with most(12) satellites
      if (m_max_sat_count > m_sat_count)
        return;
      else
        m_max_sat_count = m_sat_count;
      m_talker = wxString::Format("GPS\n%d", m_sat_count);
    } else if (talker_id == "GL") {
      m_gtime[2] = now;
      if (m_master != 2) return;
      // See "GP" above
      if (m_max_sat_count > m_sat_count)
        return;
      else
        m_max_sat_count = m_sat_count;
      m_talker = wxString::Format("GLONASS\n%d", m_sat_count);
    } else if (talker_id == "GA") {
      m_gtime[3] = now;
      if (m_master != 3) return;
      // See "GP" above
      if (m_max_sat_count > m_sat_count)
        return;
      else
        m_max_sat_count = m_sat_count;
      m_talker = wxString::Format("Galileo\n%d", m_sat_count);
    } else if (talker_id == "GB" || talker_id == "BD") {  // BeiDou  BDS
      m_gtime[4] = now;
      if (m_master != 4) return;
      // See "GP" above
      if (m_max_sat_count > m_sat_count)
        return;
      else
        m_max_sat_count = m_sat_count;
      m_talker = wxString::Format("BeiDou\n%d", m_sat_count);
    } else if (talker_id == "GI") {
      m_gtime[5] = now;
      if (m_master != 5) return;
      // See "GP" above
      if (m_max_sat_count > m_sat_count)
        return;
      else
        m_max_sat_count = m_sat_count;
      m_talker = wxString::Format("NavIC\n%d", m_sat_count);
    } else if (talker_id == "GQ") {
      m_gtime[0] = now;
      if (m_master != 0) return;
      // See "GP" above
      if (m_max_sat_count > m_sat_count)
        return;
      else
        m_max_sat_count = m_sat_count;
      m_talker = wxString::Format("QZSS\n%d", m_sat_count);
    } else {
      // Would be a not known N2k PGP type like "Combined GPS/GLONASS"
      m_talker = wxString::Format("%s\n%d", talker_id, m_sat_count);
    }
  }

  int lidx = (seq - 1) * 4;
  for (int idx = 0; idx < 4; idx++) {
    m_sat_info[lidx + idx].SatNumber = sats[idx].SatNumber;
    m_sat_info[lidx + idx].ElevationDegrees = sats[idx].ElevationDegrees;
    m_sat_info[lidx + idx].AzimuthDegreesTrue = sats[idx].AzimuthDegreesTrue;
    m_sat_info[lidx + idx].SignalToNoiseRatio = sats[idx].SignalToNoiseRatio;
  }
  // Clean out possible leftovers.
  for (int idx = m_sat_count; idx < 12; idx++) {
    m_sat_info[idx].SatNumber = 0;
    m_sat_info[idx].SignalToNoiseRatio = 0;
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
  if (m_Properties)
    cb = GetColourSchemeBackgroundColour(m_Properties->m_DataBackgroundColour);
  else
    GetGlobalColor("DASHB", &cb);
  dc->SetTextBackground(cb);
  dc->SetBackgroundMode(wxSOLID);

  wxColour cl;
  if (m_Properties)
    cl = GetColourSchemeBackgroundColour(m_Properties->m_TitleBackgroundColour);
  else
    GetGlobalColor("DASHL", &cl);
  dc->SetTextForeground(cl);
  dc->SetBrush(*wxTRANSPARENT_BRUSH);

  wxPen pen;
  pen.SetStyle(wxPENSTYLE_SOLID);
  wxColour cf;
  GetGlobalColor("DASHF", &cf);
  pen.SetColour(cf);
  pen.SetWidth(1);
  dc->SetPen(pen);

  dc->DrawCircle(m_cx, m_cy, m_radius);
  if (m_Properties)
    dc->SetFont((m_Properties->m_SmallFont.GetChosenFont()));
  else
    dc->SetFont((g_pFontSmall->GetChosenFont()));

  wxScreenDC sdc;
  int height, width;
  wxFont f;
  if (m_Properties)
    f = m_Properties->m_SmallFont.GetChosenFont();
  else
    f = g_pFontSmall->GetChosenFont();
  sdc.GetTextExtent("W", &width, &height, nullptr, nullptr, &f);

  wxBitmap tbm(width, height, -1);
  wxMemoryDC tdc(tbm);
  tdc.SetBackground(cb);
  // tdc.SetTextForeground(cl);
  tdc.SetTextBackground(cb);
  tdc.SetBackgroundMode(wxSOLID);
  if (m_Properties) {
    tdc.SetFont(m_Properties->m_SmallFont.GetChosenFont());
    tdc.SetTextForeground(
        GetColourSchemeFont(m_Properties->m_SmallFont.GetColour()));
  } else {
    tdc.SetFont(g_pFontSmall->GetChosenFont());
    tdc.SetTextForeground(GetColourSchemeFont(g_pFontSmall->GetColour()));
  }
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

  dc->DrawLine(3, m_scale_base, size.x - 3, m_scale_base);
  dc->DrawLine(3, m_scale_base + 4 * m_scale_delta, size.x - 3,
               m_scale_base + 4 * m_scale_delta);

  pen.SetStyle(wxPENSTYLE_DOT);
  dc->SetPen(pen);
  dc->DrawCircle(m_cx, m_cy, m_radius * sin(deg2rad(45)));
  dc->DrawCircle(m_cx, m_cy, m_radius * sin(deg2rad(20)));

  //        wxSHORT_DASH is not supported on GTK, and it destroys the pen.
#ifndef __WXGTK__
  pen.SetStyle(wxPENSTYLE_SHORT_DASH);
  dc->SetPen(pen);
#endif
  dc->DrawLine(3, m_scale_base + 1 * m_scale_delta, size.x - 3,
               m_scale_base + 1 * m_scale_delta);
  dc->DrawLine(3, m_scale_base + 2 * m_scale_delta, size.x - 3,
               m_scale_base + 2 * m_scale_delta);
  dc->DrawLine(3, m_scale_base + 3 * m_scale_delta, size.x - 3,
               m_scale_base + 3 * m_scale_delta);
}

void DashboardInstrument_GPS::DrawBackground(wxGCDC* dc) {
  // Draw SatID
  wxFont f;
  wxScreenDC sdc;
  int height, width;
  if (m_Properties)
    f = m_Properties->m_SmallFont.GetChosenFont();
  else
    f = g_pFontSmall->GetChosenFont();
  sdc.GetTextExtent("W", &width, &height, nullptr, nullptr, &f);

  wxColour cl;
  wxBitmap tbm(dc->GetSize().x, height, -1);
  wxMemoryDC tdc(tbm);
  wxColour c2;
  if (m_Properties)
    c2 = GetColourSchemeBackgroundColour(m_Properties->m_DataBackgroundColour);
  else
    GetGlobalColor("DASHB", &c2);
  tdc.SetBackground(c2);
  tdc.Clear();

  if (m_Properties) {
    tdc.SetFont(m_Properties->m_SmallFont.GetChosenFont());
    cl = GetColourSchemeFont(m_Properties->m_SmallFont.GetColour());
  } else {
    tdc.SetFont(g_pFontSmall->GetChosenFont());
    GetGlobalColor("DASHF", &cl);
  }
  tdc.SetTextForeground(cl);
  tdc.SetTextBackground(c2);

  int pitch = m_ref_dim;
  int offset = m_ref_dim * 15 / 100;
  for (int idx = 0; idx < 12; idx++) {
    if (m_sat_info[idx].SatNumber) {
      wxString satno = wxString::Format("%02d", m_sat_info[idx].SatNumber);
      // Avoid three digit sat-number here. Especially for BeiDou(GB/BD)
      satno = satno.Right(2);
      tdc.DrawText(satno, idx * pitch + offset, 0);
    } else
      tdc.DrawText(" -", idx * pitch + offset, 0);
  }

  tdc.SelectObject(wxNullBitmap);

  int scaleDelta = m_ref_dim / 2;
  int scaleBase = (m_radius * 2) + (2 * m_ref_dim);

  dc->DrawBitmap(tbm, 0, scaleBase + (scaleDelta * 45 / 10), false);
}

void DashboardInstrument_GPS::DrawForeground(wxGCDC* dc) {
  wxColour cl;
  if (m_Properties)
    cl = GetColourSchemeFont(m_Properties->m_DataFont.GetColour());
  else
    cl = GetColourSchemeFont(g_pFontData->GetColour());
  // GetGlobalColor("DASHL", &cl);
  wxBrush brush(cl);
  dc->SetBrush(brush);
  dc->SetPen(*wxTRANSPARENT_PEN);
  dc->SetTextBackground(cl);

  wxColor cf;
  GetGlobalColor("DASHF", &cf);
  dc->SetTextForeground(cf);
  dc->SetBackgroundMode(wxSOLID);

  wxColour cb;
  if (m_Properties)
    cb = GetColourSchemeBackgroundColour(m_Properties->m_TitleBackgroundColour);
  else
    GetGlobalColor("DASHL", &cb);

  dc->SetTextBackground(cb);

  int m_scaleDelta = m_ref_dim / 2;
  int m_scaleBase = (m_radius * 2) + (2 * m_ref_dim);
  int pitch = m_ref_dim;
  int offset = m_ref_dim * 12 / 100;

  for (int idx = 0; idx < 12; idx++) {
    if (m_sat_info[idx].SignalToNoiseRatio) {
      int h = m_sat_info[idx].SignalToNoiseRatio * m_ref_dim / 24;  // 0.4;
      dc->DrawRectangle(idx * pitch + offset,
                        m_scaleBase + (4 * m_scaleDelta) - h, pitch * 60 / 100,
                        h);
    }
  }

  wxString label = "00";
  wxFont f;
  int width, height;
  wxScreenDC sdc;
  if (m_Properties)
    f = m_Properties->m_SmallFont.GetChosenFont();
  else
    f = g_pFontSmall->GetChosenFont();

  sdc.GetTextExtent(label, &width, &height, nullptr, nullptr, &f);
  dc->SetFont(f);
  dc->SetBackgroundMode(wxTRANSPARENT);

  for (int idx = 0; idx < 12; idx++) {
    if (m_sat_info[idx].SignalToNoiseRatio) {
      label.Printf("%02d", m_sat_info[idx].SatNumber);
      int posx =
          m_cx +
          m_radius *
              cos(deg2rad(m_sat_info[idx].AzimuthDegreesTrue - ANGLE_OFFSET)) *
              sin(deg2rad(ANGLE_OFFSET - m_sat_info[idx].ElevationDegrees)) -
          width / 2.0;
      int posy =
          m_cy +
          m_radius *
              sin(deg2rad(m_sat_info[idx].AzimuthDegreesTrue - ANGLE_OFFSET)) *
              sin(deg2rad(ANGLE_OFFSET - m_sat_info[idx].ElevationDegrees)) -
          height / 2.0;
      dc->DrawText(label, posx, posy);
    }
  }
  dc->SetBackgroundMode(wxSOLID);
  if (talker_id != wxEmptyString) dc->DrawText(m_talker, 1, m_ref_dim * 3 / 2);
}
