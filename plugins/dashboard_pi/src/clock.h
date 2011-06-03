/******************************************************************************
 * $Id: clock.h, v1.0 2011/05/15 nohal Exp $
 *
 * Project:  OpenCPN
 * Purpose:  Dashboard Plugin
 * Author:   Pavel Kalian
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
 *   $EMAIL$   *
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

#ifndef __CLOCK_H__
#define __CLOCK_H__

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

int moon_phase(int y, int m, int d);

class DashboardInstrument_Clock: public DashboardInstrument_Single
{
      public:
            DashboardInstrument_Clock( wxWindow *parent, wxWindowID id, wxString title);

            ~DashboardInstrument_Clock(void){}

            void SetInstrumentWidth(int width);
            void SetData(int, double, wxString) {};
            void SetUtcTime(int st, wxDateTime value);
};

class DashboardInstrument_Moon : public DashboardInstrument_Single
{
public:
      DashboardInstrument_Moon( wxWindow *parent, wxWindowID id, wxString title);
      ~DashboardInstrument_Moon(){}

      void SetUtcTime(int st, wxDateTime value);
};

#endif // __CLOCK_H__

