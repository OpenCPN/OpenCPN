/******************************************************************************
 * $Id: wind_history.h, v1.0 2010/08/30 tom-r Exp $
 *
 * Project:  OpenCPN
 * Purpose:  Dashboard Plugin
 * Author:   Thomas Rauch
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

#ifndef __WIND_HISTORY_H__
#define __WIND_HISTORY_H__

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
#define WIND_RECORD_COUNT 3000

#include "instrument.h"
#include "dial.h"



class DashboardInstrument_WindDirHistory: public DashboardInstrument
{
      public:
            DashboardInstrument_WindDirHistory( wxWindow *parent, wxWindowID id, wxString title);

            ~DashboardInstrument_WindDirHistory(void){}

            void SetData(int, double, wxString);
   wxSize GetSize( int orient, wxSize hint );


      private:
    int m_soloInPane ;
    int    m_SpdRecCnt, m_DirRecCnt, m_SpdStartVal,m_DirStartVal;
    int m_isNULL;
    int m_WindDirShift;

      protected:
      double alpha;
            double m_ArrayWindDirHistory[WIND_RECORD_COUNT];
            double m_ArrayWindSpdHistory[WIND_RECORD_COUNT];
            double m_ExpSmoothArrayWindSpd[WIND_RECORD_COUNT];
   double m_ExpSmoothArrayWindDir[WIND_RECORD_COUNT];
   wxDateTime m_ArrayRecTime[WIND_RECORD_COUNT];
            
            double m_MaxWindDir;
            double m_MinWindDir;
   double m_WindDirRange;
            double m_MaxWindSpd;  //...in array
            double m_TotalMaxWindSpd; // since O is started
            double m_WindDir;
            double m_WindSpd;
   double m_MaxWindSpdScale;
            double m_ratioW;
   double m_oldDirVal;
   bool m_IsRunning;
   int m_SampleCount;

            wxRect m_WindowRect;
            wxRect m_DrawAreaRect; //the coordinates of the real darwing area
            int m_DrawingWidth,m_TopLineHeight,m_DrawingHeight;
   int m_width,m_height;
            int m_LeftLegend, m_RightLegend;
            int m_currSec,m_lastSec,m_SpdCntperSec,m_DirCntperSec;
            double m_cntSpd,m_cntDir,m_avgSpd,m_avgDir;

            void Draw(wxGCDC* dc);
            void DrawBackground(wxGCDC* dc);
            void DrawForeground(wxGCDC* dc);
            void SetMinMaxWindScale();
   void DrawWindDirScale(wxGCDC* dc);
   void DrawWindSpeedScale(wxGCDC* dc);
   wxString GetWindDirStr(wxString WindDir);
};



#endif // __WIND_HISTORY_H__

