/******************************************************************************
 * $Id: meteo.cpp, v1.0 2010/08/30 SethDart Exp $
 *
 * Project:  OpenCPN
 * Purpose:  Dashboard Plugin
 * Author:   Dirk Smits
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

#include "meteo.h"

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

DashboardInstrument_Meteo::DashboardInstrument_Meteo( wxWindow *parent, wxWindowID id, wxString title) :
    DashboardInstrument(parent, id, title, OCPN_DBP_STC_MET_AIT | OCPN_DBP_STC_MET_AIP | OCPN_DBP_STC_MET_AIS )
{
    m_DataTemp = -1000; //the value -1000 is used as non valid data
    m_DataPress = -1000; //the value -1000 is used as non valid data

    for (int idx = 0; idx < METEO_RECORD_COUNT; idx++)
    {
	m_ArrayTemp[idx] = -1000;
	m_ArrayPress[idx] = -1000;
    }

    SetInstrumentWidth(200);
}

void DashboardInstrument_Meteo::SetInstrumentWidth(int width)
{
      wxClientDC dc(this);
      int w;
      dc.GetTextExtent(m_title, &w, &m_TitleHeight, 0, 0, g_pFontTitle);
      m_width = width;
      m_height = m_TitleHeight+140;
      SetMinSize(wxSize(m_width, m_height));
      Refresh(false);
}

void DashboardInstrument_Meteo::SetData(int st, double data, wxString unit)
{
    double fractpart;  
    if (st == OCPN_DBP_STC_MET_AIT)
    {
	m_DataTemp = data;
    }    
    else if (st == OCPN_DBP_STC_MET_AIP)
    {
	m_DataPress = data * 1000 ; //convert to Bar to mBars
	
	    // Update data to graphs not every time data is coming in, so you will have a long time graph
	time_t seconds;
	seconds = time (NULL); //seconds since 1-1-1970
	fractpart = modf ((seconds / (3600 / METEO_GRAPH_UPDATE_NUM_PER_HOUR)) , &Temp);//every vew min 1 count to graph
	if (TimeLapCount > Temp)
	{
	    TimeLapCount = Temp;
	}
	if (Temp > TimeLapCount)
	{
	    for (int idx = 1; idx < METEO_RECORD_COUNT; idx++)
		{
		    m_ArrayTemp[idx-1] = m_ArrayTemp[idx];
		}
		m_ArrayTemp[METEO_RECORD_COUNT-1] = m_DataTemp;
	    

	    for (int idx = 1; idx < METEO_RECORD_COUNT; idx++)
		{
		    m_ArrayPress[idx-1] = m_ArrayPress[idx];
		}
		m_ArrayPress[METEO_RECORD_COUNT-1] = m_DataPress;
	    TimeLapCount = Temp;
	}
	Refresh(false);
    }
}

void DashboardInstrument_Meteo::Draw(wxBufferedDC* dc)
{
      DrawBackground(dc);
      DrawForeground(dc);
}

void DashboardInstrument_Meteo::DrawBackground(wxBufferedDC* dc)
{
      wxRect rect = GetClientRect();
      wxColour cl;

      GetGlobalColor(_T("BLUE2"), &cl);
      dc->SetTextForeground(cl);

      wxPen pen;
      pen.SetStyle(wxSOLID);
      pen.SetColour(cl);
      dc->SetPen(pen);

      dc->DrawLine(3, 40, rect.width-3, 40);
      dc->DrawLine(3, 140, rect.width-3, 140);

      pen.SetStyle(wxSHORT_DASH);
      dc->SetPen(pen);
      dc->DrawLine(3, 65, rect.width-3, 65);
      dc->DrawLine(3, 90, rect.width-3, 90);
      dc->DrawLine(3, 115, rect.width-3, 115);

      dc->SetFont(*g_pFontSmall);

      
}

void DashboardInstrument_Meteo::DrawForeground(wxBufferedDC* dc)
{
    wxRect rect = GetClientRect();
    wxColour cl;
    // Draw Temperature line 
    DrawGraph(dc, m_ArrayTemp,_T("BLUE1"), _T(" °C"), wxALIGN_RIGHT);
    // Draw Presure line 
    DrawGraph(dc, m_ArrayPress,_T("GREEN4"), _T(" hPa"), wxALIGN_LEFT);
    
    GetGlobalColor( _T("BLUE2"), &cl);
    dc->SetTextForeground(cl);
    dc->SetFont(*g_pFontData);
    dc->DrawText(wxString::Format(_T("%5.1f °C"), m_DataTemp), 10, rect.height/2+10 - m_TitleHeight -5);// Current Temperature
    dc->DrawText(wxString::Format(_T("%5.1f hPa"), m_DataPress), 10, rect.height/2 + 15);// Current Pressure
}

void DashboardInstrument_Meteo::DrawGraph(wxBufferedDC* dc, double dataArray[], wxString colour, wxString unit, long style )
{
    wxRect rect = GetClientRect();
    double fractpart, intpart, m_Delta, m_Max = 0, m_Min = 999999;
    wxBrush brush;
    wxColour cl;
    int width, height, labelleft, idx2;
      
    for (int idx = 0; idx < METEO_RECORD_COUNT; idx++)
    {
	if (dataArray[idx] > m_Max)
		m_Max = dataArray[idx];
	if ((dataArray[idx] < m_Min) && (dataArray[idx] != -1000))
		m_Min = dataArray[idx];
    }
    
    m_Delta = (m_Max - m_Min)/4; // find scale for temp in 4 degr steps to fit nice in scale
    fractpart = modf (m_Delta , &intpart);
    m_Delta= intpart * 4 + 4;
    fractpart = modf (m_Min , &intpart);
    m_Min = intpart; 
    m_Max = m_Min + m_Delta;
    
    			// set colour
    GetGlobalColor(colour, &cl);
    dc->SetTextForeground(cl);
    dc->SetFont(*g_pFontSmall);        //Set Font labels

    brush.SetStyle(wxSOLID);
    brush.SetColour(cl);
    dc->SetBrush(brush);
    
    wxString label;
    label.Printf(_T("%1.0f"), m_Max);  // Set label Max Value
    label=label+unit;
    
    dc->GetTextExtent(label, &width, &height, 0, 0, g_pFontSmall); // get size label Max Value
    if (style & wxALIGN_RIGHT)
	labelleft = rect.width-width;
    else if (style & wxALIGN_LEFT)
	labelleft = 0;	
    dc->DrawText(label, labelleft, 40-height);              // Draw label max value

    label.Printf(_T("%1.0f"), m_Min);  // Set label Min Value
    label=label+unit;
    dc->GetTextExtent(label, &width, &height, 0, 0, g_pFontSmall); // get size label Min Value
    if (style & wxALIGN_RIGHT)
	labelleft = rect.width-width;
    else if (style & wxALIGN_LEFT)
	labelleft = 0;	
    dc->DrawText(label, labelleft, rect.height-height);              // Draw label min value
  
    
    dc->SetPen(*wxTRANSPARENT_PEN);
    double ratioH = 100.0 / (m_Max - m_Min); // 140-40=100
    double ratioW = double(rect.width - 6) / (METEO_RECORD_COUNT-1);
    wxPoint points[METEO_RECORD_COUNT*2];
    for (int idx = 0; idx < METEO_RECORD_COUNT; idx++)
    {
	idx2 = (METEO_RECORD_COUNT * 2) - idx -1;  
	points[idx].x = idx * ratioW + 3;
	points[idx2].x = points[idx].x;
	if (dataArray[idx] != -1000)
	{
		points[idx].y = 140 - (-m_Min+dataArray[idx]) * ratioH;
		points[idx2].y = 138 - (-m_Min+dataArray[idx]) * ratioH;
	}
	else
	{
		points[idx].y = 140;
		points[idx2].y = 138;
	}
    }     
    dc->DrawPolygon(METEO_RECORD_COUNT*2, points);

}