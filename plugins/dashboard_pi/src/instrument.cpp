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
#include <math.h>
#include <time.h>

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

void DashboardInstrument_Sun::SetData(int st, double data, wxString unit)
{
      if (st == m_cap_flag1)
      {
            m_lat = data;
      }
      else if (st == m_cap_flag2)
      {
            m_lon = data;
      }
      else return;

      if (m_lat == 999.9 || m_lon == 999.9)
            return;
      
      wxDateTime sunset, sunrise;
      calculateSun(m_lat, m_lon, sunrise, sunset);
      m_data1 = sunrise.FormatISOTime().Append(_T(" UTC"));
      m_data2 = sunset.FormatISOTime().Append(_T(" UTC"));

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

// Sunset/sunrise calculation
// Modified from code at http://www.sci.fi/~benefon/rscalc_cpp.html
// C++ program calculating the sunrise and sunset for
// the current date and a fixed location(latitude,longitude)
// Jarmo Lammi 1999 - 2000
// Last update January 6th, 2000

#ifndef PI
      #define PI        3.1415926535897931160E0      /* pi */
#endif
#define DEGREE    (PI/180.0)
#define RADIAN    (180.0/PI)
#define TPI       (2*PI)
#define SUNDIA    0.53
#define AIREFR    34.0/60.0 // athmospheric refraction degrees //

double L,g;
//double SunDia = 0.53;     // Sunradius degrees

//   Get the days to J2000
//   h is UT in decimal hours
//   FNday only works between 1901 to 2099 - see Meeus chapter 7
double FNday (int y, int m, int d, float h) {
      int luku = - 7 * (y + (m + 9)/12)/4 + 275*m/9 + d;
      // type casting necessary on PC DOS and TClite to avoid overflow
      luku+= (long int)y*367;
      return (double)luku - 730531.5 + h/24.0;
};

//   the function below returns an angle in the range
//   0 to 2*pi
double FNrange (double x) {
      double b = x / TPI;
      double a = TPI * (b - (long)(b));
      if (a < 0) a = TPI + a;
      return a;
};

// Calculating the hourangle
//
double f0(double lat, double declin) {
      double fo,dfo;
      // Correction: different sign at S HS
      dfo = DEGREE*(0.5*SUNDIA + AIREFR); if (lat < 0.0) dfo = -dfo;
      fo = tan(declin + dfo) * tan(lat*DEGREE);
      if (fo>0.99999) fo=1.0; // to avoid overflow //
      fo = asin(fo) + PI/2.0;
      return fo;
};


//   Find the ecliptic longitude of the Sun
double FNsun (double d) {

      //   mean longitude of the Sun
      L = FNrange(280.461 * DEGREE + .9856474 * DEGREE * d);

      //   mean anomaly of the Sun
      g = FNrange(357.528 * DEGREE + .9856003 * DEGREE * d);
      //   Ecliptic longitude of the Sun

      return FNrange(L + 1.915 * DEGREE * sin(g) + .02 * DEGREE * sin(2 * g));
};

// Convert decimal hours in hours and minutes
wxDateTime convHrmn(double dhr) {
      int hr,mn;
      hr = (int) dhr;
      mn = (dhr - (double) hr)*60;
      return wxDateTime(hr, mn);
};

void calculateSun(double latit, double longit, wxDateTime &sunrise, wxDateTime &sunset){
      double y,m,day,h;

      time_t sekunnit;
      struct tm *p;

      //  get the date and time from the user
      // read system date and extract the year
      // FIXME: we should probably also get it from GPS...

      /** First get time **/
      time(&sekunnit);

      /** Next get localtime **/

      p=localtime(&sekunnit);

      y = p->tm_year;
      // this is Y2K compliant method
      y+= 1900;
      m = p->tm_mon + 1;

      day = p->tm_mday;

      h = 12;

      //double tzone=2.0;

      double d = FNday(y, m, day, h);

      //   Use FNsun to find the ecliptic longitude of the
      //   Sun

      double lambda = FNsun(d);

      //   Obliquity of the ecliptic

      double obliq = 23.439 * DEGREE - .0000004 * DEGREE * d;

      //   Find the RA and DEC of the Sun

      double alpha = atan2(cos(obliq) * sin(lambda), cos(lambda));
      double delta = asin(sin(obliq) * sin(lambda));

      // Find the Equation of Time
      // in minutes
      // Correction suggested by David Smith
      double LL = L - alpha;
      if (L < PI) LL += TPI;
      double equation = 1440.0 * (1.0 - LL / TPI);
      double ha = f0(latit,delta);

      double riset = 12.0 - 12.0 * ha/PI /*+ tzone*/ - longit/15.0 + equation/60.0;
      double settm = 12.0 + 12.0 * ha/PI /*+ tzone*/ - longit/15.0 + equation/60.0;

      sunrise = convHrmn(riset);
      sunset = convHrmn(settm);
}
