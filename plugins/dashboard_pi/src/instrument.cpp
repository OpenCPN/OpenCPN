/******************************************************************************
 * $Id: instrument.cpp, v1.0 2010/08/30 SethDart Exp $
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

DashboardInstrument::DashboardInstrument(wxWindow *pparent, wxWindowID id, wxString title, int cap_flag)
      :wxWindow(pparent, id, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE, title)
{
      m_title = title;
      m_width = 10;
      m_height = 10;
      m_cap_flag = cap_flag;

      wxClientDC dc(this);
      int width;
      dc.GetTextExtent(m_title, &width, &m_TitleHeight, 0, 0, g_pFontTitle);

      Connect(this->GetId(), wxEVT_PAINT, wxPaintEventHandler(DashboardInstrument::OnPaint));
}

int DashboardInstrument::GetCapacity()
{
      return m_cap_flag;
}

void DashboardInstrument::OnPaint(wxPaintEvent& WXUNUSED(event))
{
      wxPaintDC dc(this);

      if(!dc.IsOk())
            return;

      wxBitmap bmp;

      // Create a double buffer to draw the plot
      // on screen to prevent flicker from occuring.
      wxBufferedDC buff_dc;
      buff_dc.Init(&dc, bmp);

      wxRect rect = GetClientRect();

      if(rect.width == 0 || rect.height == 0)
      {
            return;
      }

      wxColour cl;

      GetGlobalColor(_T("DILG1"), &cl);
      buff_dc.SetBackground(cl);
      buff_dc.Clear();

      GetGlobalColor(_T("UIBDR"), &cl);
      // With wxTRANSPARENT_PEN the borders are ugly so lets use the same color for both
      wxPen pen;
      pen.SetStyle(wxSOLID);
      pen.SetColour(cl);
      buff_dc.SetPen(pen);
      buff_dc.SetBrush(cl);
      buff_dc.DrawRoundedRectangle(0, 0, rect.width, m_TitleHeight, 3);

      buff_dc.SetFont(*g_pFontTitle);
      //      dc->SetTextForeground(pFontMgr->GetFontColor(_T("Dashboard Label")));
      GetGlobalColor(_T("DILG3"), &cl);
      buff_dc.SetTextForeground(cl);
      buff_dc.DrawText(m_title, 5, 0);

      Draw(&buff_dc);
}

//----------------------------------------------------------------
//
//    DashboardInstrument_Simple Implementation
//
//----------------------------------------------------------------

DashboardInstrument_Single::DashboardInstrument_Single(wxWindow *pparent, wxWindowID id, wxString title, int cap_flag, wxString format)
      :DashboardInstrument(pparent, id, title, cap_flag)
{
      m_format = format;

      m_data = _T("---");
      SetInstrumentWidth(200);
}

void DashboardInstrument_Single::SetInstrumentWidth(int width)
{
      wxClientDC dc(this);
      int w;
      dc.GetTextExtent(m_title, &w, &m_TitleHeight, 0, 0, g_pFontTitle);
      dc.GetTextExtent(_T("000"), &w, &m_DataHeight, 0, 0, g_pFontData);

      m_width = width;
      m_height = m_TitleHeight+m_DataHeight;
      SetMinSize(wxSize(m_width, m_height));
      Refresh(false);
}

void DashboardInstrument_Single::Draw(wxBufferedDC* dc)
{
      wxColour cl;

      dc->SetFont(*g_pFontData);
      //dc.SetTextForeground(pFontMgr->GetFontColor(_T("Dashboard Data")));
      GetGlobalColor(_T("BLUE2"), &cl);
      dc->SetTextForeground(cl);

      dc->DrawText(m_data, 10, m_TitleHeight);
}

void DashboardInstrument_Single::SetData(int st, double data, wxString unit)
{
      if (m_cap_flag & st)
      {
            m_data = wxString::Format(m_format, data);

            Refresh(false);
      }
}

//----------------------------------------------------------------
//
//    DashboardInstrument_Position Implementation
//
//----------------------------------------------------------------

DashboardInstrument_Position::DashboardInstrument_Position(wxWindow *pparent, wxWindowID id, wxString title, int cap_flag1, int cap_flag2)
      :DashboardInstrument(pparent, id, title, cap_flag1 | cap_flag2)
{

      m_data1 = _T("---");
      m_data2 = _T("---");
      m_cap_flag1 = cap_flag1;
      m_cap_flag2 = cap_flag2;
      SetInstrumentWidth(200);
}

void DashboardInstrument_Position::SetInstrumentWidth(int width)
{
      wxClientDC dc(this);
      int w;
      dc.GetTextExtent(m_title, &w, &m_TitleHeight, 0, 0, g_pFontTitle);
      dc.GetTextExtent(_T("000"), &w, &m_DataHeight, 0, 0, g_pFontData);

      m_width = width;
      m_height = m_TitleHeight+m_DataHeight*2;
      SetMinSize(wxSize(m_width, m_height));
      Refresh(false);
}

void DashboardInstrument_Position::Draw(wxBufferedDC* dc)
{
      wxColour cl;

      dc->SetFont(*g_pFontData);
      //dc.SetTextForeground(pFontMgr->GetFontColor(_T("Dashboard Data")));
      GetGlobalColor(_T("BLUE2"), &cl);
      dc->SetTextForeground(cl);

      dc->DrawText(m_data1, 10, m_TitleHeight);
      dc->DrawText(m_data2, 10, m_TitleHeight+m_DataHeight);
}

void DashboardInstrument_Position::SetData(int st, double data, wxString unit)
{
      if (st == m_cap_flag1)
      {
            m_data1 = toSDMM(1, data);
      }
      else if (st == m_cap_flag2)
      {
            m_data2 = toSDMM(2, data);
      }
      else return;

      Refresh(false);
}

/**************************************************************************/
/*          Some assorted utilities                                       */
/**************************************************************************/

wxString toSDMM ( int NEflag, double a )
{
      short neg = 0;
      int d;
      long m;

      if ( a < 0.0 )
      {
            a = -a;
            neg = 1;
      }
      d = ( int ) a;
      m = ( long ) ( ( a - ( double ) d ) * 60000.0 );

      if ( neg )
            d = -d;

      wxString s;

      if ( !NEflag )
            s.Printf ( _T ( "%d %02ld.%03ld'" ), d, m / 1000, m % 1000 );
      else
      {
            if ( NEflag == 1 )
            {
                  char c = 'N';

                  if ( neg )
                  {
                        d = -d;
                        c = 'S';
                  }

                  s.Printf ( _T ( "%03d %02ld.%03ld %c" ), d, m / 1000, ( m % 1000 ), c );
            }
            else if ( NEflag == 2 )
            {
                  char c = 'E';

                  if ( neg )
                  {
                        d = -d;
                        c = 'W';
                  }
                  s.Printf ( _T ( "%03d %02ld.%03ld %c" ), d, m / 1000, ( m % 1000 ), c );
            }
      }
      return s;
}

