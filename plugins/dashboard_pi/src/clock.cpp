/******************************************************************************
 * $Id: clock.cpp, v1.0 2011/05/15 nohal Exp $
 *
 * Project:  OpenCPN
 * Purpose:  Dashboard Plugin
 * Author:   Pavel Kalian
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

#include "clock.h"

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

DashboardInstrument_Clock::DashboardInstrument_Clock( wxWindow *parent, wxWindowID id, wxString title, int cap_flag, wxString format ) :
      DashboardInstrument_Single( parent, id, title, cap_flag, format )
{
}

wxSize DashboardInstrument_Clock::GetSize( int orient, wxSize hint )
{
      wxClientDC dc(this);
      int w;
      dc.GetTextExtent(m_title, &w, &m_TitleHeight, 0, 0, g_pFontTitle);
      dc.GetTextExtent(_T("00:00:00 UTC"), &w, &m_DataHeight, 0, 0, g_pFontData);

      if( orient== wxHORIZONTAL ) {
          return wxSize( DefaultWidth, wxMax(m_TitleHeight+m_DataHeight, hint.y) );
      } else {
          return wxSize( wxMax(hint.x, DefaultWidth), m_TitleHeight+m_DataHeight );
      }
}

void DashboardInstrument_Clock::SetData( int, double, wxString )
{
// Nothing to do here but we want to override the default
}

void DashboardInstrument_Clock::SetUtcTime( wxDateTime data )
{
    if (data.IsValid())
    {
        m_data = data.FormatISOTime().Append(_T(" UTC"));
    }
}

DashboardInstrument_Moon::DashboardInstrument_Moon( wxWindow *parent, wxWindowID id, wxString title ) :
      DashboardInstrument_Clock( parent, id, title, OCPN_DBP_STC_CLK|OCPN_DBP_STC_LAT, _T("%i/4 %c") )
{
    m_phase = -1;
    m_radius = 14;
    m_hemisphere = _T("");
}

wxSize DashboardInstrument_Moon::GetSize( int orient, wxSize hint )
{
      wxClientDC dc(this);
      int w;
      dc.GetTextExtent(m_title, &w, &m_TitleHeight, 0, 0, g_pFontTitle);

      if( orient== wxHORIZONTAL ) {
          return wxSize( DefaultWidth, wxMax(m_TitleHeight+10+m_radius*2, hint.y) );
      } else {
          return wxSize( wxMax(hint.x, DefaultWidth), m_TitleHeight+10+m_radius*2 );
      }
}

void DashboardInstrument_Moon::SetData( int st, double value, wxString format )
{
    if( st == OCPN_DBP_STC_LAT ) {
        m_hemisphere = (value < 0 ? _T("S") : _T("N") );
    }
}

void DashboardInstrument_Moon::Draw(wxGCDC* dc)
{
    if ( m_phase == -1 || m_hemisphere == _T("") ) return;

    wxSize sz = GetClientSize();
    wxColour cl0, cl1, cl2;

    dc->SetPen( *wxTRANSPARENT_PEN );
    GetGlobalColor(_T("DASHL"), &cl0);
    dc->SetBrush(cl0);
    wxPoint points[3];
    points[0].x = 5;
    points[0].y = m_TitleHeight+m_radius*2+6;
    points[1].x = sz.x/2;
    points[1].y = m_TitleHeight+10;
    points[2].x = sz.x-5;
    points[2].y = m_TitleHeight+m_radius*2+6;
    dc->DrawPolygon(3, points, 0, 0);

    int x = 2+m_radius+(sz.x-m_radius-2)/8*m_phase;
    int y = m_TitleHeight+m_radius+5;

    /* Moon phases are seen upside-down on the southern hemisphere */
    int startangle = ( m_hemisphere == _("N") ? -90 : 90 );

    GetGlobalColor(_T("DASH2"), &cl0);
    GetGlobalColor(_T("DASH1"), &cl1);
    GetGlobalColor(_T("DASHF"), &cl2);

    dc->SetBrush(cl0);
    dc->DrawCircle( x, y, m_radius );
    dc->SetBrush(cl1);

    switch ( m_phase ) {
    case 0:
        dc->SetPen( cl2 );
        dc->SetBrush( *wxTRANSPARENT_BRUSH );
        dc->DrawCircle( x, y, m_radius );
    break;
    case 1:
        dc->DrawEllipticArc( x-m_radius, m_TitleHeight+5, m_radius*2, m_radius*2, startangle, startangle+180 );
        dc->SetBrush( cl0 );
        dc->DrawEllipticArc( x-m_radius/2, m_TitleHeight+5, m_radius, m_radius*2, startangle, startangle+180 );
    break;
    case 2:
        dc->SetBrush( cl1 );
        dc->DrawEllipticArc( x-m_radius, m_TitleHeight+5, m_radius*2, m_radius*2, startangle, startangle+180 );
    break;
    case 3:
        //if( m_hemisphere == _("N") ) {
        dc->DrawEllipticArc( x-m_radius/2, m_TitleHeight+5, m_radius, m_radius*2, -startangle, 180-startangle );
        dc->DrawEllipticArc( x-m_radius, m_TitleHeight+5, m_radius*2, m_radius*2, startangle, startangle+180 );
    break;
    case 4:
        dc->DrawCircle( x, y, m_radius );
    break;
    case 5:
        dc->DrawEllipticArc( x-m_radius, m_TitleHeight+5, m_radius*2, m_radius*2, -startangle, 180-startangle );
        dc->DrawEllipticArc( x-m_radius/2, m_TitleHeight+5, m_radius, m_radius*2, startangle, startangle+180 );
    break;
    case 6:
        dc->DrawEllipticArc( x-m_radius, m_TitleHeight+5, m_radius*2, m_radius*2, -startangle, 180-startangle );
    break;
    case 7:
        dc->DrawEllipticArc( x-m_radius, m_TitleHeight+5, m_radius*2, m_radius*2, -startangle, 180-startangle );
        dc->SetBrush( cl0 );
        dc->DrawEllipticArc( x-m_radius/2, m_TitleHeight+5, m_radius, m_radius*2, -startangle, 180-startangle );
    break;
    }
    dc->SetPen( cl2 );
    dc->SetBrush( *wxTRANSPARENT_BRUSH );
    dc->DrawCircle( x, y, m_radius );
}

void DashboardInstrument_Moon::SetUtcTime( wxDateTime data )
{
    if (data.IsValid())
    {
        m_phase = moon_phase(data.GetYear(), data.GetMonth() + 1, data.GetDay());
    }
}

// Moon phase calculation
int DashboardInstrument_Moon::moon_phase(int y, int m, int d)
{
    /*
      calculates the moon phase (0-7), accurate to 1 segment.
      0 => new moon.
      4 => full moon.
      */

    int c,e;
    double jd;
    int b;

    if (m < 3) {
        y--;
        m += 12;
    }
    ++m;
    c = 365.25*y;
    e = 30.6*m;
    jd = c+e+d-694039.09;  /* jd is total days elapsed */
    jd /= 29.53;           /* divide by the moon cycle (29.53 days) */
    b = jd;		   /* int(jd) -> b, take integer part of jd */
    jd -= b;		   /* subtract integer part to leave fractional part of original jd */
    b = jd*8 + 0.5;	   /* scale fraction from 0-8 and round by adding 0.5 */
    b = b & 7;		   /* 0 and 8 are the same so turn 8 into 0 */
    return b;
}

#include <math.h>
#include <time.h>

#ifndef PI
      #define PI        3.1415926535897931160E0      /* pi */
#endif
#define DEGREE    (PI/180.0)
#define RADIAN    (180.0/PI)
#define TPI       (2*PI)

// Zeniths for sunset/sunrise calculation
#define ZENITH_OFFICIAL (90.0 + 50.0 / 60.0)
#define ZENITH_CIVIL 96.0
#define ZENITH_NAUTICAL 102.0
#define ZENITH_ASTRONOMICAL 108.0

// Convert decimal hours in hours and minutes
wxDateTime convHrmn(double dhr) {
      int hr,mn;
      hr = (int) dhr;
      mn = (dhr - (double) hr)*60;
      return wxDateTime(hr, mn);
};

DashboardInstrument_Sun::DashboardInstrument_Sun( wxWindow *parent, wxWindowID id, wxString title ) :
    DashboardInstrument_Clock( parent, id, title, OCPN_DBP_STC_LAT|OCPN_DBP_STC_LON|OCPN_DBP_STC_CLK )
{
    m_lat = m_lon = 999.9;
    m_dt = wxDateTime::Now().ToUTC();
    m_sunrise = _T("---");
    m_sunset = _T("---");
}

wxSize DashboardInstrument_Sun::GetSize( int orient, wxSize hint )
{
      wxClientDC dc(this);
      int w;
      dc.GetTextExtent(m_title, &w, &m_TitleHeight, 0, 0, g_pFontTitle);
      dc.GetTextExtent(_T("00:00:00 UTC"), &w, &m_DataHeight, 0, 0, g_pFontData);

      if( orient== wxHORIZONTAL ) {
          return wxSize( DefaultWidth, wxMax(m_TitleHeight+m_DataHeight*2, hint.y) );
      } else {
          return wxSize( wxMax(hint.x, DefaultWidth), m_TitleHeight+m_DataHeight*2 );
      }
}

void DashboardInstrument_Sun::Draw(wxGCDC* dc)
{
      wxColour cl;

      dc->SetFont(*g_pFontData);
      GetGlobalColor(_T("DASHF"), &cl);
      dc->SetTextForeground(cl);

      dc->DrawText(m_sunrise, 10, m_TitleHeight);
      dc->DrawText(m_sunset, 10, m_TitleHeight+m_DataHeight);
}

void DashboardInstrument_Sun::SetUtcTime( wxDateTime data )
{
    if (data.IsValid())
    {
        m_dt = data;
        wxDateTime sunrise, sunset;
        calculateSun(m_lat, m_lon, sunrise, sunset);
        if (sunrise.GetYear() != 999)
            m_sunrise = sunrise.FormatISOTime().Append(_T(" UTC"));
        else
            m_sunrise = _T("---");
        if (sunset.GetYear() != 999)
            m_sunset = sunset.FormatISOTime().Append(_T(" UTC"));
        else
            m_sunset = _T("---");
    }
}

void DashboardInstrument_Sun::SetData( int st, double data, wxString unit )
{
      if( st == OCPN_DBP_STC_LAT )
      {
            m_lat = data;
      }
      else if( st == OCPN_DBP_STC_LON )
      {
            m_lon = data;
      }
      else return;

      if (m_lat == 999.9 || m_lon == 999.9)
            return;

      wxDateTime sunset, sunrise;
      calculateSun(m_lat, m_lon, sunrise, sunset);
      if (sunrise.GetYear() != 999)
            m_sunrise = sunrise.FormatISOTime().Append(_T(" UTC"));
      else
            m_sunrise = _T("---");
      if (sunset.GetYear() != 999)
            m_sunset = sunset.FormatISOTime().Append(_T(" UTC"));
      else
            m_sunset = _T("---");
}

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

