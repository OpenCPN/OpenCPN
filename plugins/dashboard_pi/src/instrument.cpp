/******************************************************************************
 * $Id: instrument.cpp, v1.0 2010/08/30 SethDart Exp $
 *
 * Project:  OpenCPN
 * Purpose:  DashBoard Plugin
 * Author:   Jean-Eudes Onfray
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
 *   $EMAIL$   *
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


#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers


#include "instrument.h"

//----------------------------------------------------------------
//
//    Generic DashboardInstrument Implementation
//
//----------------------------------------------------------------

DashboardInstrument::DashboardInstrument(wxWindow *pparent, wxWindowID id, wxString title)
      :wxWindow(pparent, id, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE, title)
{
      m_title = title;

      Connect(this->GetId(), wxEVT_PAINT, wxPaintEventHandler(DashboardInstrument::OnPaint));
}

DashboardInstrument::~DashboardInstrument()
{
}

void DashboardInstrument::OnPaint(wxPaintEvent& WXUNUSED(event))
{
      wxPaintDC dc(this);

      wxRect rect = GetClientRect();

      if(rect.width == 0 || rect.height == 0)
      {
            return;
      }

      wxBitmap bmp;

      // Create a double buffer to draw the plot
      // on screen to prevent flicker from occuring.
      wxBufferedDC buff_dc;
      buff_dc.Init(&dc, bmp);
      buff_dc.Clear();

      wxColour cl;

      GetGlobalColor(_T("DILG1"), &cl);
      buff_dc.SetBackground(cl);
      buff_dc.Clear();

      wxFont *font = OCPNGetFont(_T("DashBoard Label"), 9);
      int width;
      buff_dc.GetTextExtent(m_title, &width, &m_TitleHeight, 0, 0, font);

      GetGlobalColor(_T("UIBDR"), &cl);
      // With wxTRANSPARENT_PEN the borders are ugly so lets use the same color for both
      wxPen pen;
      pen.SetStyle(wxSOLID);
      pen.SetColour(cl);
      buff_dc.SetPen(pen);
      buff_dc.SetBrush(cl);
      buff_dc.DrawRoundedRectangle(0, 0, rect.width, m_TitleHeight, 3);

      buff_dc.SetFont(*font);
      //      dc->SetTextForeground(pFontMgr->GetFontColor(_T("DashBoard Label")));
      GetGlobalColor(_T("DILG3"), &cl);
      buff_dc.SetTextForeground(cl);
      buff_dc.DrawText(m_title, 5, 0);

      Draw(&buff_dc);
}

void DashboardInstrument::Draw(wxBufferedDC* dc)
{
}

//----------------------------------------------------------------
//
//    DashboardInstrument_Simple Implementation
//
//----------------------------------------------------------------

DashboardInstrument_Single::DashboardInstrument_Single(wxWindow *pparent, wxWindowID id, wxString title)
      :DashboardInstrument(pparent, id, title)
{
      m_data = _T("---");

      SetMinSize(wxSize(200, 50));
}

DashboardInstrument_Single::~DashboardInstrument_Single()
{
}

void DashboardInstrument_Single::Draw(wxBufferedDC* dc)
{
      wxColour cl;

      dc->SetFont(*OCPNGetFont(_T("DashBoard Data"), 16));
      //dc.SetTextForeground(pFontMgr->GetFontColor(_T("DashBoard Data")));
      GetGlobalColor(_T("BLUE2"), &cl);
      dc->SetTextForeground(cl);

      dc->DrawText(m_data, 30, 20);
}

void DashboardInstrument_Single::SetData(wxString data)
{
      m_data = data;

      Refresh(false);
}

//----------------------------------------------------------------
//
//    DashboardInstrument_Double Implementation
//
//----------------------------------------------------------------

DashboardInstrument_Double::DashboardInstrument_Double(wxWindow *pparent, wxWindowID id, wxString title)
      :DashboardInstrument(pparent, id, title)
{
      m_data1 = _T("---");
      m_data2 = _T("---");

      SetMinSize(wxSize(200, 75));
}

DashboardInstrument_Double::~DashboardInstrument_Double()
{
}

void DashboardInstrument_Double::Draw(wxBufferedDC* dc)
{
      wxColour cl;

      dc->SetFont(*OCPNGetFont(_T("DashBoard Data"), 16));
      //dc.SetTextForeground(pFontMgr->GetFontColor(_T("DashBoard Data")));
      GetGlobalColor(_T("BLUE2"), &cl);
      dc->SetTextForeground(cl);

      dc->DrawText(m_data1, 30, 20);
      dc->DrawText(m_data2, 30, 45);
}

void DashboardInstrument_Double::SetData(wxString data1, wxString data2)
{
      m_data1 = data1;
      m_data2 = data2;

      Refresh(false);
}

