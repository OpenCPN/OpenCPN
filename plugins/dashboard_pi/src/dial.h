/******************************************************************************
 * $Id: dial.h, v1.0 2010/08/05 SethDart Exp $
 *
 * Project:  OpenCPN
 * Purpose:  Dashboard Plugin
 * Author:   Jean-Eudes Onfray
 *           (Inspired by original work from Andreas Heiming)
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

#ifndef __Dial_H__
#define __Dial_H__

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

#include "instrument.h"

#define ANGLE_OFFSET            90      // 0 degrees are at 12 o´clock

typedef enum
{
      DIAL_LABEL_NONE,
      DIAL_LABEL_HORIZONTAL,
      DIAL_LABEL_ROTATED
} DialLabelOption;

typedef enum
{
      DIAL_MARKER_NONE,
      DIAL_MARKER_SIMPLE,
      DIAL_MARKER_REDGREEN,
      DIAL_MARKER_REDGREENBAR
} DialMarkerOption;

typedef enum
{
      DIAL_POSITION_NONE,
      DIAL_POSITION_INSIDE,
      DIAL_POSITION_TOPLEFT,
      DIAL_POSITION_TOPRIGHT,
      DIAL_POSITION_BOTTOMLEFT,
      DIAL_POSITION_BOTTOMRIGHT
} DialPositionOption;

extern double rad2deg(double angle);
extern double deg2rad(double angle);

//+------------------------------------------------------------------------------
//|
//| CLASS:
//|    DashboardInstrument_Dial
//|
//| DESCRIPTION:
//|    This class creates a speedometer style control
//|
//+------------------------------------------------------------------------------
class DashboardInstrument_Dial: public DashboardInstrument
{
      public:
            DashboardInstrument_Dial( wxWindow *parent, wxWindowID id, wxString title, int cap_flag,
                        int s_angle, int r_angle, int s_value, int e_value);

            ~DashboardInstrument_Dial(void){}

            virtual void SetInstrumentWidth(int width);
            void SetData(int, double, wxString);
            void SetOptionMarker(double step, DialMarkerOption option, int offset) { m_MarkerStep = step; m_MarkerOption = option; m_MarkerOffset = offset; }
            void SetOptionLabel(double step, DialLabelOption option, wxArrayString labels=wxArrayString()) { m_LabelStep = step; m_LabelOption = option; m_LabelArray = labels; }
            void SetOptionMainValue(wxString format, DialPositionOption option)
                        { m_MainValueFormat = format; m_MainValueOption = option; }
            void SetOptionExtraValue(int cap, wxString format, DialPositionOption option)
                        { m_ExtraValueCap = cap; m_cap_flag |= cap; m_ExtraValueFormat = format; m_ExtraValueOption = option; }

      private:

      protected:
            int m_cx, m_cy, m_radius;
            int m_AngleStart, m_AngleRange;
            double m_MainValue;
            int m_MainValueCap;
            double m_MainValueMin, m_MainValueMax;
            wxString m_MainValueFormat;
            DialPositionOption m_MainValueOption;
            double m_ExtraValue;
            int m_ExtraValueCap;
            wxString m_ExtraValueFormat;
            DialPositionOption m_ExtraValueOption;
            DialMarkerOption m_MarkerOption;
            int m_MarkerOffset;
            double m_MarkerStep, m_LabelStep;
            DialLabelOption m_LabelOption;
            wxArrayString m_LabelArray;

            virtual void Draw(wxBufferedDC* dc);
            virtual void DrawFrame(wxBufferedDC* dc);
            virtual void DrawMarkers(wxBufferedDC* dc);
            virtual void DrawLabels(wxBufferedDC* dc);
            virtual void DrawBackground(wxBufferedDC* dc);
            virtual void DrawData(wxBufferedDC* dc, double value, wxString format, DialPositionOption position);
            virtual void DrawForeground(wxBufferedDC* dc);
};

#endif // __Dial_H__

