/******************************************************************************
 * $Id: compass.h, v1.0 2010/08/05 SethDart Exp $
 *
 * Project:  OpenCPN
 * Purpose:  Dashboard Plugin
 * Author:   Jean-Eudes Onfray
 *           (Inspired by original work from Andreas Heiming)
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

#ifndef __Compass_H__
#define __Compass_H__

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

#include "dial.h"

//+------------------------------------------------------------------------------
//|
//| CLASS:
//|    DashboardInstrument_Compass
//|
//| DESCRIPTION:
//|    This class creates a compass style control
//|
//+------------------------------------------------------------------------------
class DashboardInstrument_Compass: public DashboardInstrument_Dial
{
      public:
            DashboardInstrument_Compass(wxWindow *parent, wxWindowID id, wxString title, int cap_flag);

            ~DashboardInstrument_Compass(void){}

            void SetData(int, double, wxString);

      private:

      protected:
            void DrawBackground(wxGCDC* dc);
            void DrawForeground(wxGCDC* dc);
};

#endif // __Compass_H__

