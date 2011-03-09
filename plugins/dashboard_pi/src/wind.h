/******************************************************************************
 * $Id: wind.h, v1.0 2010/08/05 SethDart Exp $
 *
 * Project:  OpenCPN
 * Purpose:  Dashboard Plugin
 * Author:   Jean-Eudes Onfray
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

#ifndef __Wind_H__
#define __Wind_H__

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
//|    DashboardInstrument_Wind
//|
//| DESCRIPTION:
//|    This class creates a wind style control
//|
//+------------------------------------------------------------------------------
class DashboardInstrument_Wind: public DashboardInstrument_Dial
{
      public:
            DashboardInstrument_Wind( wxWindow *parent, wxWindowID id, wxString title, int cap_flag);

            ~DashboardInstrument_Wind(void){}

      private:

      protected:
            void DrawBackground(wxBufferedDC* dc);
};

class DashboardInstrument_WindCompass: public DashboardInstrument_Dial
{
      public:
            DashboardInstrument_WindCompass( wxWindow *parent, wxWindowID id, wxString title, int cap_flag);

            ~DashboardInstrument_WindCompass(void){}

      private:

      protected:
            void DrawBackground(wxBufferedDC* dc);
};

#endif // __Wind_H__

