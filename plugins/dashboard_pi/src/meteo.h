/******************************************************************************
 * $Id: meteo.h, v1.0 2010/08/30 SethDart Exp $
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

#ifndef __METEO_H__
#define __METEO_H__

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

// Warn: div by 0 if count == 1

#define METEO_GRAPH_HOURS 12
#define METEO_GRAPH_UPDATE_NUM_PER_HOUR 6
#define METEO_RECORD_COUNT METEO_GRAPH_HOURS*METEO_GRAPH_UPDATE_NUM_PER_HOUR

#include "instrument.h"

class DashboardInstrument_Meteo: public DashboardInstrument
{
      public:
            DashboardInstrument_Meteo( wxWindow *parent, wxWindowID id, wxString title);

            ~DashboardInstrument_Meteo(void){}

            void SetInstrumentWidth(int width);
            void SetData(int, double, wxString);

      private:

      protected:
	    double TimeLapCount, Temp;
            double m_ArrayTemp[METEO_RECORD_COUNT];
            double m_DataTemp;
            double m_ArrayPress[METEO_RECORD_COUNT];
            double m_DataPress;
            void Draw(wxBufferedDC* dc);
            void DrawBackground(wxBufferedDC* dc);
            void DrawForeground(wxBufferedDC* dc);
	    void DrawGraph(wxBufferedDC* dc, double dataArray[], wxString colour, wxString unit, long style = wxALIGN_NOT);
};

#endif // __METEO_H__

