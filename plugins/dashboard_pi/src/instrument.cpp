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
      if (sunrise.GetYear() != 999) 
            m_data1 = sunrise.FormatISOTime().Append(_T(" UTC"));
      else 
            m_data1 = _T("---");
      if (sunset.GetYear() != 999)
            m_data2 = sunset.FormatISOTime().Append(_T(" UTC"));
      else
            m_data2 = _T("---");

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

#ifndef PI
      #define PI        3.1415926535897931160E0      /* pi */
#endif
#define DEGREE    (PI/180.0)
#define RADIAN    (180.0/PI)
#define TPI       (2*PI)

// Convert decimal hours in hours and minutes
wxDateTime convHrmn(double dhr) {
      int hr,mn;
      hr = (int) dhr;
      mn = (dhr - (double) hr)*60;
      return wxDateTime(hr, mn);
};

void DashboardInstrument_Sun::calculateSun(double latit, double longit, wxDateTime &sunrise, wxDateTime &sunset){
/*
Source:
	Almanac for Computers, 1990
	published by Nautical Almanac Office
	United States Naval Observatory
	Washington, DC 20392

Inputs:
	day, month, year:      date of sunrise/sunset
	latitude, longitude:   location for sunrise/sunset
	zenith:                Sun's zenith for sunrise/sunset
	  offical      = 90 degrees 50'
	  civil        = 96 degrees
	  nautical     = 102 degrees
	  astronomical = 108 degrees
	
	NOTE: longitude is positive for East and negative for West
        NOTE: the algorithm assumes the use of a calculator with the
        trig functions in "degree" (rather than "radian") mode. Most
        programming languages assume radian arguments, requiring back
        and forth convertions. The factor is 180/pi. So, for instance,
        the equation RA = atan(0.91764 * tan(L)) would be coded as RA
        = (180/pi)*atan(0.91764 * tan((pi/180)*L)) to give a degree
        answer with a degree input for L.

1. first calculate the day of the year

	N1 = floor(275 * month / 9)
	N2 = floor((month + 9) / 12)
	N3 = (1 + floor((year - 4 * floor(year / 4) + 2) / 3))
	N = N1 - (N2 * N3) + day - 30
*/
      int n = m_dt.GetDayOfYear();
/*
2. convert the longitude to hour value and calculate an approximate time

	lngHour = longitude / 15
	
	if rising time is desired:
	  t = N + ((6 - lngHour) / 24)
	if setting time is desired:
	  t = N + ((18 - lngHour) / 24)
*/
      double lngHour = longit / 15;
      double tris = n + ((6 - lngHour) / 24);
      double tset = n + ((18 - lngHour) / 24);
/*

3. calculate the Sun's mean anomaly
	
	M = (0.9856 * t) - 3.289
*/
      double mris = (0.9856 * tris) - 3.289;
      double mset = (0.9856 * tset) - 3.289;
/*
4. calculate the Sun's true longitude
	
	L = M + (1.916 * sin(M)) + (0.020 * sin(2 * M)) + 282.634
	NOTE: L potentially needs to be adjusted into the range [0,360) by adding/subtracting 360
*/
      double lris = mris + (1.916 * sin(DEGREE * mris)) + (0.020 * sin(2 * DEGREE * mris)) + 282.634;
      if (lris > 360) lris -= 360;
      if (lris < 0) lris += 360;
      double lset = mset + (1.916 * sin(DEGREE * mset)) + (0.020 * sin(2 * DEGREE * mset)) + 282.634;
      if (lset > 360) lset -= 360;
      if (lset < 0) lset += 360;
/*
5a. calculate the Sun's right ascension
	
	RA = atan(0.91764 * tan(L))
	NOTE: RA potentially needs to be adjusted into the range [0,360) by adding/subtracting 360
*/
      double raris = RADIAN * atan(0.91764 * tan(DEGREE * lris));
      if (raris > 360) raris -= 360;
      if (raris < 0) raris += 360;
      double raset = RADIAN * atan(0.91764 * tan(DEGREE * lset));
      if (raset > 360) raset -= 360;
      if (raset < 0) raset += 360;
/*
5b. right ascension value needs to be in the same quadrant as L

	Lquadrant  = (floor( L/90)) * 90
	RAquadrant = (floor(RA/90)) * 90
	RA = RA + (Lquadrant - RAquadrant)
*/
      double lqris = (floor( lris/90)) * 90;
      double raqris = (floor(raris/90)) * 90;
      raris = raris + (lqris - raqris);
      double lqset = (floor( lset/90)) * 90;
      double raqset = (floor(raset/90)) * 90;
      raset = raset + (lqset - raqset);
/*
5c. right ascension value needs to be converted into hours

	RA = RA / 15
*/
      raris = raris/15;
      raset = raset/15;
/*
6. calculate the Sun's declination

	sinDec = 0.39782 * sin(L)
	cosDec = cos(asin(sinDec))
*/
      double sinDecris = 0.39782 * sin(DEGREE * lris);
      double cosDecris = cos(asin(sinDecris));
      double sinDecset = 0.39782 * sin(DEGREE * lset);
      double cosDecset = cos(asin(sinDecset));
/*
7a. calculate the Sun's local hour angle
	
	cosH = (cos(zenith) - (sinDec * sin(latitude))) / (cosDec * cos(latitude))
	
	if (cosH >  1) 
	  the sun never rises on this location (on the specified date)
	if (cosH < -1)
	  the sun never sets on this location (on the specified date)
*/
      double cosZenith = cos(DEGREE * ZENITH_OFFICIAL);
      double coshris = (cosZenith - (sinDecris * sin(DEGREE * latit))) / (cosDecris * cos(DEGREE * latit));
      double coshset = (cosZenith - (sinDecset * sin(DEGREE * latit))) / (cosDecset * cos(DEGREE * latit));
      bool neverrises = false;
      if (coshris > 1) neverrises = true;
      if (coshris < -1) neverrises = true; //nohal - it's cosine - even value lower than -1 is ilegal... correct me if i'm wrong
      bool neversets = false;
      if (coshset < -1) neversets = true;
      if (coshset > 1) neversets = true; //nohal - it's cosine - even value greater than 1 is ilegal... correct me if i'm wrong
/*
7b. finish calculating H and convert into hours
	
	if if rising time is desired:
	  H = 360 - acos(cosH)
	if setting time is desired:
	  H = acos(cosH)
	
	H = H / 15
*/
      double hris = 360 - RADIAN * acos(coshris);
      hris = hris/15;
      double hset = RADIAN * acos(coshset);
      hset = hset/15;
/*
8. calculate local mean time of rising/setting
	
	T = H + RA - (0.06571 * t) - 6.622
*/
      tris = hris + raris - (0.06571 * tris) - 6.622;
      tset = hset + raset - (0.06571 * tset) - 6.622;
/*
9. adjust back to UTC
	
	UT = T - lngHour
	NOTE: UT potentially needs to be adjusted into the range [0,24) by adding/subtracting 24
*/
      double utris = tris - lngHour;
      if (utris > 24) utris -= 24;
      if (utris <0) utris += 24;
      double utset = tset - lngHour;
      if (utset > 24) utset -= 24;
      if (utset <0) utset += 24;

      sunrise = convHrmn(utris);
      if (neverrises) sunrise.SetYear(999);
      sunset = convHrmn(utset);
      if (neversets) sunset.SetYear(999);
/*
Optional:
10. convert UT value to local time zone of latitude/longitude
	
	localT = UT + localOffset
*/
}

void DashboardInstrument_Sun::SetUtcTime(int st, wxDateTime data)
{
      if (m_cap_flag & st)
      {
            if (data.IsValid())
            {
                  m_dt = data;
                  wxDateTime sunrise, sunset;
                  calculateSun(m_lat, m_lon, sunrise, sunset);
                  if (sunrise.GetYear() != 999) 
                        m_data1 = sunrise.FormatISOTime().Append(_T(" UTC"));
                  else 
                        m_data1 = _T("---");
                  if (sunset.GetYear() != 999)
                        m_data2 = sunset.FormatISOTime().Append(_T(" UTC"));
                  else
                        m_data2 = _T("---");
            }
            Refresh(false);
      }
}
