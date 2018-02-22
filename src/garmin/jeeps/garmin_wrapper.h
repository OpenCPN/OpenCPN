/*
    Garmin Jeeps Interface Wrapper.

    Copyright (C) 2010 David S Register, bdbcat@yahoo.com

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111 USA

 */


#ifndef garmin_wrapper_h
#define garmin_wrapper_h

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled header

#include "navutil.h"

//    Fwd definitions
class ComPortManager;



/*  Wrapped interface from higher level objects   */
int Garmin_GPS_Init(ComPortManager *pPortMan, wxString &port_name);
wxString Garmin_GPS_GetSaveString();

int Garmin_GPS_SendWaypoints(ComPortManager *pPortMan, wxString &port_name, RoutePointList *wplist);
int Garmin_GPS_SendRoute(ComPortManager *pPortMan, wxString &port_name, Route *pr, wxGauge *pProgress);

wxString GetLastGarminError(void);

int Garmin_USB_On(void);
int Garmin_USB_Off(void);



#endif      /* garmin_wrapper_h */

