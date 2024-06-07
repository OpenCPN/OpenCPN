/******************************************************************************
 * $Id: instrument.cpp, v1.0 2010/08/30 SethDart Exp $
 *
 * Project:  OpenCPN
 * Purpose:  Dashboard Plugin
 * Author:   Jean-Eudes Onfray
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

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // precompiled headers
#include <cmath>

#include "instrument.h"

#ifdef __OCPN__ANDROID__
#include "qdebug.h"
#endif

 // ColorScheme ... Try only make a simple darker in Night mode. this could be made better.
extern PI_ColorScheme aktuellColorScheme;

wxColour GetColourSchemeBackgroundColour(wxColour co)
{
    wxColour ret_val = co;
    #if wxCHECK_VERSION(3, 1, 6)
    unsigned int red = co.GetRed();
    unsigned int Green = co.GetGreen();
    unsigned int Blue = co.GetBlue();
    #else
    unsigned int red = co.Red();
    unsigned int Green = co.Green();
    unsigned int Blue = co.Blue();
    #endif
    switch (aktuellColorScheme) {
    case PI_GLOBAL_COLOR_SCHEME_RGB:
        break;
    case PI_GLOBAL_COLOR_SCHEME_DAY:
        break;
    case PI_GLOBAL_COLOR_SCHEME_DUSK:
        red *= .8;
        Green *= .8;
        Blue *= .8;
        ret_val = wxColour(red, Green, Blue);
        break;
    case PI_GLOBAL_COLOR_SCHEME_NIGHT:
        red *= .5;
        Green *= .5;
        Blue *= .5;
        ret_val = wxColour(red, Green, Blue);
        break;
    default: break;
    }
    return ret_val;
}


wxColour GetColourSchemeFont(wxColour co)
{
    wxColour ret_val = co;
    #if wxCHECK_VERSION(3, 1, 6)
    unsigned int red = co.GetRed();
    unsigned int Green = co.GetGreen();
    unsigned int Blue = co.GetBlue();
    #else
    unsigned int red = co.Red();
    unsigned int Green = co.Green();
    unsigned int Blue = co.Blue();
    #endif
    switch (aktuellColorScheme) {
    case PI_GLOBAL_COLOR_SCHEME_RGB:
        break;
    case PI_GLOBAL_COLOR_SCHEME_DAY:
        break;
    case PI_GLOBAL_COLOR_SCHEME_DUSK:
        red *= .8;
        Green *= .8;
        Blue *= .8;
        //if (red + Green + Blue < 10) {
        //  red = Green = Blue = 50;
        //}
        ret_val = wxColour(red, Green, Blue);
        break;
    case PI_GLOBAL_COLOR_SCHEME_NIGHT:
        red *= .5;
        Green *= .5;
        Blue *= .5;
        if (red + Green + Blue < 10) {
          red = Green = Blue = 50;
        }
        ret_val = wxColour(red, Green, Blue);
        break;
    default: break;
    }
    return ret_val;
}


//----------------------------------------------------------------
//
//    Generic DashboardInstrument Implementation
//
//----------------------------------------------------------------


DashboardInstrument::DashboardInstrument(wxWindow* pparent, wxWindowID id,
                                         wxString title, DASH_CAP cap_flag,
                                         InstrumentProperties* Properties)
    : wxControl(pparent, id, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE) {
  m_title = title;
  m_Properties = Properties;
  m_cap_flag.set(cap_flag);

  SetBackgroundStyle(wxBG_STYLE_CUSTOM);
  SetDrawSoloInPane(false);
  wxClientDC dc(this);
  int width;
  wxFont f;
  if (m_Properties) {
    f = m_Properties->m_TitleFont.GetChosenFont();
    dc.GetTextExtent(m_title, &width, &m_TitleHeight, 0, 0, &f);
  } else {
    f = g_pFontTitle->GetChosenFont();
    dc.GetTextExtent(m_title, &width, &m_TitleHeight, 0, 0, &f);
  }
  Connect(wxEVT_ERASE_BACKGROUND,
          wxEraseEventHandler(DashboardInstrument::OnEraseBackground));
  Connect(wxEVT_PAINT, wxPaintEventHandler(DashboardInstrument::OnPaint));

  //  On OSX, there is an orphan mouse event that comes from the automatic
  //  exEVT_CONTEXT_MENU synthesis on the main wxWindow mouse handler.
  //  The event goes to an instrument window (here) that may have been deleted
  //  by the preferences dialog.  Result is NULL deref. Solution:  Handle
  //  right-click here, and DO NOT skip() Strangely, this does not work for
  //  GTK... See: http://trac.wxwidgets.org/ticket/15417

#if defined(__WXOSX__) || defined(__WXQT__)
  Connect(wxEVT_RIGHT_DOWN,
          wxMouseEventHandler(DashboardInstrument::MouseEvent), NULL, this);
#endif
}

void DashboardInstrument::MouseEvent(wxMouseEvent& event) {
  if (event.GetEventType() == wxEVT_RIGHT_DOWN) {
    wxContextMenuEvent evtCtx(wxEVT_CONTEXT_MENU, this->GetId(),
                              this->ClientToScreen(event.GetPosition()));
    evtCtx.SetEventObject(this);
    GetParent()->GetEventHandler()->AddPendingEvent(evtCtx);
  }
}

CapType DashboardInstrument::GetCapacity() { return m_cap_flag; }
void DashboardInstrument::SetDrawSoloInPane(bool value) {
  m_drawSoloInPane = value;
}
void DashboardInstrument::OnEraseBackground(wxEraseEvent& WXUNUSED(evt)) {
  // intentionally empty
}

void DashboardInstrument::OnPaint(wxPaintEvent& WXUNUSED(event)) {
    wxAutoBufferedPaintDC pdc(this);
    if (!pdc.IsOk()) {
        wxLogMessage(
            _T("DashboardInstrument::OnPaint() fatal: ")
            _T("wxAutoBufferedPaintDC.IsOk() false."));
        return;
    }

    wxSize size = GetClientSize();
    if (size.x == 0 || size.y == 0) {
        wxLogMessage(_T("DashboardInstrument::OnPaint() fatal: Zero size DC."));
        return;
    }

#if wxUSE_GRAPHICS_CONTEXT
    wxGCDC dc(pdc);
#else
    wxDC& dc(pdc);
#endif
    wxColour cl;
    if (m_Properties)
    {
        dc.SetBackground(GetColourSchemeBackgroundColour(m_Properties->m_DataBackgroundColour));
    }
    else
    {
        GetGlobalColor(_T("DASHB"), &cl);
        dc.SetBackground(cl);
    }
#ifdef __WXGTK__
  dc.SetBrush(cl);
  dc.SetPen(*wxTRANSPARENT_PEN);
  dc.DrawRectangle(0, 0, size.x, size.y);
#endif
  dc.Clear();

  Draw(&dc);

  if (!m_drawSoloInPane) {
      wxPen pen;
      pen.SetStyle(wxPENSTYLE_SOLID);
      if (m_Properties)
      {
          pen.SetColour(GetColourSchemeBackgroundColour(m_Properties->m_TitleBackgroundColour));
          dc.SetPen(pen);
          dc.SetBrush(GetColourSchemeBackgroundColour(m_Properties->m_TitleBackgroundColour));
      }
      else
      {
          GetGlobalColor(_T("DASHL"), &cl);
          pen.SetColour(cl);
          dc.SetPen(pen);
          dc.SetBrush(cl);
      }

    dc.DrawRoundedRectangle(0, m_TitleTop, size.x, m_TitleHeight, 3);
    wxFont f;
    wxString Title=m_title;
    if (m_Properties)
    {
        f = m_Properties->m_TitleFont.GetChosenFont();
        dc.SetFont(f);
        dc.SetTextForeground(GetColourSchemeFont(m_Properties->m_TitleFont.GetColour()));
        dc.SetTextBackground(GetColourSchemeBackgroundColour(m_Properties->m_TitleBackgroundColour));
        if ( !m_Properties->m_Title.IsEmpty() ) Title=m_Properties->m_Title;
    }
    else
    {
        f = g_pFontTitle->GetChosenFont();
        dc.SetFont(f);
        dc.SetTextForeground(GetColourSchemeFont(g_pFontTitle->GetColour()));
        GetGlobalColor(_T("DASHL"), &cl);
        dc.SetTextBackground(cl);
    }
    // GetGlobalColor(_T("DASHF"), &cl);
    //dc.SetTextForeground(cl);
    if (m_TitleRightAlign) {
      int w,h;
      dc.GetTextExtent(Title, &w, &h, 0, 0, &f);
      dc.DrawText(Title, GetClientSize().GetWidth()-w-g_iTitleMargin, m_TitleTop);
    } else {
      dc.DrawText(Title, g_iTitleMargin, m_TitleTop);
    }
  }
}

//----------------------------------------------------------------
//
//    DashboardInstrument_Simple Implementation
//
//----------------------------------------------------------------

DashboardInstrument_Single::DashboardInstrument_Single(wxWindow* pparent,
                                                       wxWindowID id,
                                                       wxString title,
                                                       InstrumentProperties* Properties,
                                                       DASH_CAP cap_flag,
                                                       wxString format)
    : DashboardInstrument(pparent, id, title, cap_flag, Properties) {
  m_format = format;
  m_data = _T("---");
  m_DataHeight = 0;
  m_Properties = Properties;
  m_DataTop = 0;
  m_TitleTop = 0;
  m_DataRightAlign = false;
  m_TitleRightAlign = false;
}

wxSize DashboardInstrument_Single::GetSize(int orient, wxSize hint) {
  wxClientDC dc(this);
  int w;
  wxFont f;
  m_DataRightAlign = (g_DataAlignment & wxALIGN_RIGHT) != 0;
  int InstrumentSpacing = g_iInstrumentSpacing;

  if (m_Properties)
  {
      f = m_Properties->m_TitleFont.GetChosenFont();
      dc.GetTextExtent(m_title, &w, &m_TitleHeight, 0, 0, &f);
      f = m_Properties->m_DataFont.GetChosenFont();
      dc.GetTextExtent(_T("000"), &w, &m_DataHeight, 0, 0, &f);

      if ( m_Properties->m_DataAlignment!=wxALIGN_INVALID ) m_DataRightAlign = (m_Properties->m_DataAlignment & wxALIGN_RIGHT) != 0;
      if ( m_Properties->m_InstrumentSpacing >= 0 ) InstrumentSpacing = m_Properties->m_InstrumentSpacing;
  }
  else
  {
      f = g_pFontTitle->GetChosenFont();
      dc.GetTextExtent(m_title, &w, &m_TitleHeight, 0, 0, &f);
      f = g_pFontData->GetChosenFont();
      dc.GetTextExtent(_T("000"), &w, &m_DataHeight, 0, 0, &f);

  }

  m_TitleRightAlign = (g_TitleAlignment & wxALIGN_RIGHT) != 0;
  m_TitleTop = m_DataHeight * g_TitleVerticalOffset;
  int h=m_TitleTop + m_TitleHeight + m_DataHeight + InstrumentSpacing;
  m_DataTop = m_TitleHeight;
  if ( (g_TitleAlignment & wxALIGN_BOTTOM) != 0 ) {
    m_TitleTop = m_DataHeight * (1 + g_TitleVerticalOffset);
    h=m_TitleTop + m_TitleHeight + InstrumentSpacing;
    m_DataTop = 0;
  }

  if (orient == wxHORIZONTAL) {
    return wxSize(DefaultWidth, wxMax(hint.y, h));
  } else {
    return wxSize(wxMax(hint.x, DefaultWidth), h);
  }
}

void DashboardInstrument_Single::Draw(wxGCDC* dc) {
  wxColour cl;
  wxFont f;
  int dataMargin=g_iDataMargin;
  if (m_Properties)
  {
      f = m_Properties->m_DataFont.GetChosenFont();
      dc->SetFont((f));
      dc->SetTextForeground(GetColourSchemeFont(m_Properties->m_DataFont.GetColour()));
      if ( m_Properties->m_DataMargin>=0 ) dataMargin = m_Properties->m_DataMargin;
  }
  else
  {
      f = g_pFontData->GetChosenFont();
      dc->SetFont((f));
      dc->SetTextForeground(GetColourSchemeFont(g_pFontData->GetColour()));
  }
  //GetGlobalColor(_T("DASHF"), &cl);
  //dc->SetTextForeground(cl);

  int x1;
  x1=dataMargin;

  if ( m_DataRightAlign ) {
    int w,h;
    dc->GetTextExtent(m_data, &w, &h, 0, 0, &f);
    x1=GetClientSize().GetWidth() - w - dataMargin;
  }

  dc->DrawText(m_data, x1, m_DataTop);
}

void DashboardInstrument_Single::SetData(DASH_CAP st, double data,
                                         wxString unit) {
  if (m_cap_flag.test(st)) {
    if (!std::isnan(data)) {
      bool showUnit = (m_Properties ? (m_Properties->m_ShowUnit==1) : g_bShowUnit);
      wxString format = (m_Properties && m_Properties->m_Format != "" ? m_Properties->m_Format : m_format);
      if (unit == _T("C"))
        m_data = wxString::Format(format, data) + (showUnit ? DEGREE_SIGN + _T("C"):"");
      else if (unit == _T("\u00B0"))
        m_data = wxString::Format(format, data) + (showUnit ? DEGREE_SIGN : "");
      else if (unit == _T("\u00B0T"))
        m_data = wxString::Format(format, data) + (showUnit ? DEGREE_SIGN + _(" true") : "");
      else if (unit == _T("\u00B0M"))
        m_data = wxString::Format(format, data) + (showUnit ? DEGREE_SIGN + _(" mag") : "");
      else if (unit == _T("\u00B0L"))
        m_data = _T(">") + wxString::Format(format, data) + (showUnit ? DEGREE_SIGN : "");
      else if (unit == _T("\u00B0R"))
        m_data = wxString::Format(format, data) + (showUnit ? DEGREE_SIGN + _T("<") : "");
      else if (unit == _T("N"))  // Knots
        m_data = wxString::Format(format, data) + (showUnit ? _T(" Kts") : _T(""));
      /* maybe in the future ...
                      else if (unit == _T("M")) // m/s
                        m_data = wxString::Format(m_format, data)+_T(" m/s");
                      else if (unit == _T("K")) // km/h
                        m_data = wxString::Format(m_format, data)+_T(" km/h");
       ... to be completed
       */
      else
        m_data = wxString::Format(format, data) + (showUnit ? _T(" ") + unit : "");
    } else
      m_data = _T("---");

    Refresh();
  }
}

//----------------------------------------------------------------
//
//    DashboardInstrument_Position Implementation
//
//----------------------------------------------------------------

DashboardInstrument_Position::DashboardInstrument_Position(wxWindow* pparent,
                                                           wxWindowID id,
                                                           wxString title,
                                                           InstrumentProperties* Properties,
                                                           DASH_CAP cap_flag1,
                                                           DASH_CAP cap_flag2)
    : DashboardInstrument(pparent, id, title, cap_flag1, Properties) {
  m_cap_flag.set(cap_flag2);

  m_data1 = _T("---");
  m_data2 = _T("---");
  m_cap_flag1 = cap_flag1;
  m_cap_flag2 = cap_flag2;
  m_DataHeight = 0;
}

wxSize DashboardInstrument_Position::GetSize(int orient, wxSize hint) {
  wxClientDC dc(this);
  int w;
  wxFont f;
  int dataMargin=g_iDataMargin;
  m_DataRightAlign = (g_DataAlignment & wxALIGN_RIGHT) != 0;
  int InstrumentSpacing = g_iInstrumentSpacing;

  if (m_Properties)
  {
      f = m_Properties->m_TitleFont.GetChosenFont();
      dc.GetTextExtent(m_title, &w, &m_TitleHeight, 0, 0, &f);
      f = m_Properties->m_DataFont.GetChosenFont();
      dc.GetTextExtent(_T("000  00.0000 W"), &w, &m_DataHeight, 0, 0, &f);

      if ( m_Properties->m_DataMargin>=0 ) dataMargin = m_Properties->m_DataMargin;
      if ( m_Properties->m_DataAlignment!=wxALIGN_INVALID ) m_DataRightAlign = (m_Properties->m_DataAlignment & wxALIGN_RIGHT) != 0;
      if ( m_Properties->m_InstrumentSpacing >= 0 ) InstrumentSpacing = m_Properties->m_InstrumentSpacing;
  }
  else
  {
      f = g_pFontTitle->GetChosenFont();
      dc.GetTextExtent(m_title, &w, &m_TitleHeight, 0, 0, &f);
      f = g_pFontData->GetChosenFont();
      dc.GetTextExtent(_T("000  00.0000 W"), &w, &m_DataHeight, 0, 0, &f);
  }

  m_TitleRightAlign = (g_TitleAlignment & wxALIGN_RIGHT) != 0;
  m_TitleTop = m_DataHeight*g_TitleVerticalOffset;
  int h=m_TitleTop + m_TitleHeight + m_DataHeight * 2 + InstrumentSpacing;
  m_DataTop = m_TitleHeight;
  if ( (g_TitleAlignment & wxALIGN_BOTTOM) != 0 ) {
    m_TitleTop = m_DataHeight * 2 + m_DataHeight*g_TitleVerticalOffset;
    h=m_TitleTop + m_TitleHeight + InstrumentSpacing;
    m_DataTop = 0;
  }

  if (orient == wxHORIZONTAL) {
    return wxSize(w + dataMargin, wxMax(hint.y, h));
  } else {
    return wxSize(wxMax(hint.x, w + dataMargin), h);
  }
}

void DashboardInstrument_Position::Draw(wxGCDC* dc) {
  wxColour cl;
  wxFont f;
  int dataMargin=g_iDataMargin;
  if (m_Properties)
  {
      f = m_Properties->m_DataFont.GetChosenFont();
      dc->SetFont((f));
      dc->SetTextForeground(GetColourSchemeFont(m_Properties->m_DataFont.GetColour()));
      if ( m_Properties->m_DataMargin>=0 ) dataMargin = m_Properties->m_DataMargin;
  }
  else
  {
    f = g_pFontData->GetChosenFont();
      dc->SetFont((f));
      dc->SetTextForeground(GetColourSchemeFont(g_pFontData->GetColour()));
  }
  //GetGlobalColor(_T("DASHF"), &cl);
  //dc->SetTextForeground(cl);

  int x1,x2;
  x1=x2=dataMargin;

  if ( m_DataRightAlign ) {
    int w,h;
    dc->GetTextExtent(m_data1, &w, &h, 0, 0, &f);
    x1=GetClientSize().GetWidth() - w - dataMargin;
    dc->GetTextExtent(m_data2, &w, &h, 0, 0, &f);
    x2=GetClientSize().GetWidth() - w - dataMargin;
  }

  dc->DrawText(m_data1, x1, m_DataTop);
  dc->DrawText(m_data2, x2, m_DataTop + m_DataHeight);
}

void DashboardInstrument_Position::SetData(DASH_CAP st, double data,
                                           wxString unit) {
  if (std::isnan(data)) return;
  if (st == m_cap_flag1) {
    m_data1 = toSDMM(1, data);
    m_data1[0] = ' ';
  } else if (st == m_cap_flag2) {
    m_data2 = toSDMM(2, data);
  } else
    return;
  Refresh();
}

/**************************************************************************/
/*          Some assorted utilities                                       */
/**************************************************************************/

wxString toSDMM(int NEflag, double a) {
  short neg = 0;
  int d;
  long m;

  if (a < 0.0) {
    a = -a;
    neg = 1;
  }
  d = (int)a;
  m = (long)((a - (double)d) * 60000.0);

  if (neg) d = -d;

  wxString s;

  if (!NEflag)
    s.Printf(_T ( "%d %02ld.%03ld'" ), d, m / 1000, m % 1000);
  else {
    if (NEflag == 1) {
      char c = 'N';

      if (neg) {
        d = -d;
        c = 'S';
      }

      s.Printf(_T ( "%03d %02ld.%03ld %c" ), d, m / 1000, (m % 1000), c);
    } else if (NEflag == 2) {
      char c = 'E';

      if (neg) {
        d = -d;
        c = 'W';
      }
      s.Printf(_T ( "%03d %02ld.%03ld %c" ), d, m / 1000, (m % 1000), c);
    }
  }
  return s;
}
