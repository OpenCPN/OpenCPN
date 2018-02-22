/******************************************************************************
 * $Id: depth.h, v1.0 2010/08/30 SethDart Exp $
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

#ifndef __DEPTH_H__
#define __DEPTH_H__

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
#define DEPTH_RECORD_COUNT 30

#include "instrument.h"

class DashboardInstrument_Depth: public DashboardInstrument
{
      public:
            DashboardInstrument_Depth( wxWindow *parent, wxWindowID id, wxString title);

            ~DashboardInstrument_Depth(void){}

            void SetInstrumentWidth(int width);
            void SetData(int, double, wxString);

      private:

      protected:
            double m_ArrayDepth[DEPTH_RECORD_COUNT];
            double m_MaxDepth;
            double m_Depth;
            wxString m_Temp;

            void Draw(wxBufferedDC* dc);
            void DrawBackground(wxBufferedDC* dc);
            void DrawForeground(wxBufferedDC* dc);
};

#endif // __DEPTH_H__

