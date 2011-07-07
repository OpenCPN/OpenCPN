/******************************************************************************
 * $Id: instrument.h, v1.0 2010/08/30 SethDart Exp $
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

#ifndef _INSTRUMENT_H_
#define _INSTRUMENT_H_

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

// Required GetGlobalColor
#include "../../../include/ocpn_plugin.h"
#include <wx/dcbuffer.h>

// Zeniths for sunset/sunrise calculation
#define ZENITH_OFFICIAL (90.0 + 50.0 / 60.0)
#define ZENITH_CIVIL 96.0
#define ZENITH_NAUTICAL 102.0
#define ZENITH_ASTRONOMICAL 108.0


extern wxFont *g_pFontTitle;
extern wxFont *g_pFontData;
extern wxFont *g_pFontLabel;
extern wxFont *g_pFontSmall;

wxString toSDMM ( int NEflag, double a );
void calculateSun(double latit, double longit, wxDateTime &sunrise, wxDateTime &sunset);

class DashboardInstrument;
class DashboardInstrument_Single;
class DashboardInstrument_Position;
class DashboardInstrument_Sun;

enum
{
    OCPN_DBP_STC_LAT = 1 << 0,
    OCPN_DBP_STC_LON = 1 << 1,
    OCPN_DBP_STC_SOG = 1 << 2,
    OCPN_DBP_STC_COG = 1 << 3,
    OCPN_DBP_STC_STW = 1 << 4,
    OCPN_DBP_STC_HDM = 1 << 5,
    OCPN_DBP_STC_HDT = 1 << 6,
    OCPN_DBP_STC_HMV = 1 << 7, // Magnetic variation
    OCPN_DBP_STC_BRG = 1 << 8,
    OCPN_DBP_STC_AWA = 1 << 9,
    OCPN_DBP_STC_AWS = 1 << 10,
    OCPN_DBP_STC_TWA = 1 << 11,
    OCPN_DBP_STC_TWS = 1 << 12,
    OCPN_DBP_STC_DPT = 1 << 13,
    OCPN_DBP_STC_TMP = 1 << 14,
    OCPN_DBP_STC_VMG = 1 << 15,
    OCPN_DBP_STC_RSA = 1 << 16,
    OCPN_DBP_STC_SAT = 1 << 17,
    OCPN_DBP_STC_GPS = 1 << 18,
    OCPN_DBP_STC_PLA = 1 << 19, // Cursor latitude
    OCPN_DBP_STC_PLO = 1 << 20, // Cursor longitude
    OCPN_DBP_STC_CLK = 1 << 21,
    OCPN_DBP_STC_MON = 1 << 22
};

class DashboardInstrument : public wxWindow
{
public:
      DashboardInstrument(wxWindow *pparent, wxWindowID id, wxString title, int cap_flag);
      ~DashboardInstrument(){}

      int GetCapacity();
      virtual void SetInstrumentWidth(int width) = 0;
      virtual void OnPaint(wxPaintEvent& WXUNUSED(event));
      virtual void SetData(int st, double data, wxString unit) = 0;

private:

protected:
      int               m_cap_flag;
      int               m_TitleHeight, m_width, m_height;
      wxString          m_title;

      virtual void Draw(wxBufferedDC* dc) = 0;

};

class DashboardInstrument_Single : public DashboardInstrument
{
public:
      DashboardInstrument_Single(wxWindow *pparent, wxWindowID id, wxString title, int cap, wxString format);
      ~DashboardInstrument_Single(){}

      void SetInstrumentWidth(int width);
      void SetData(int st, double data, wxString unit);

protected:
      wxString          m_data;
      wxString          m_format;
      int               m_DataHeight;

      void Draw(wxBufferedDC* dc);

};

class DashboardInstrument_Position : public DashboardInstrument
{
public:
      DashboardInstrument_Position(wxWindow *pparent, wxWindowID id, wxString title, int cap_flag1=OCPN_DBP_STC_LAT, int cap_flag2=OCPN_DBP_STC_LON);
      ~DashboardInstrument_Position(){}

      void SetInstrumentWidth(int width);
      void SetData(int st, double data, wxString unit);

protected:
      wxString          m_data1;
      wxString          m_data2;
      int               m_cap_flag1;
      int               m_cap_flag2;
      int               m_DataHeight;

      void Draw(wxBufferedDC* dc);

};

class DashboardInstrument_Sun : public DashboardInstrument_Position
{
public:
      DashboardInstrument_Sun(wxWindow *pparent, wxWindowID id, wxString title, int cap_flag1=OCPN_DBP_STC_LAT, int cap_flag2 = OCPN_DBP_STC_LON, int cap_flag= OCPN_DBP_STC_CLK) : DashboardInstrument_Position(pparent, id, title, cap_flag1, cap_flag2) { m_lat = m_lon = 999.9; m_dt = wxDateTime::Now().ToUTC(); m_cap_flag = m_cap_flag | cap_flag; }
      ~DashboardInstrument_Sun(){}

      void SetData(int st, double data, wxString unit);
      void SetUtcTime(int st, wxDateTime value);

protected:
      double m_lat;
      double m_lon;
      wxDateTime m_dt;

private:
      void calculateSun(double latit, double longit, wxDateTime &sunrise, wxDateTime &sunset);
};

#endif

