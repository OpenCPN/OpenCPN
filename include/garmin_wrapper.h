/*
    Garmin Jeeps Interface Wrapper.

    Copyright (C) 2010 David S Register

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02111 USA

 */

#ifndef garmin_wrapper_h
#define garmin_wrapper_h

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // precompiled header

#include <wx/string.h>

#include "route.h"
//#include "navutil.h"
#include "comm_n0183_output.h"

/*  Wrapped interface from higher level objects   */
int Garmin_GPS_Init(const wxString &port_name);
int Garmin_GPS_Open(wxString &port_name);
int Garmin_GPS_PVT_On(wxString &port_name);
int Garmin_GPS_PVT_Off(wxString &port_name);
int Garmin_GPS_GetPVT(void *pvt);
void Garmin_GPS_ClosePortVerify(void);

wxString Garmin_GPS_GetSaveString();

int Garmin_GPS_SendWaypoints(const wxString &port_name, RoutePointList *wplist);
int Garmin_GPS_SendRoute(const wxString &port_name, Route *pr,
                         N0183DlgCtx dlg_ctx);

wxString GetLastGarminError(void);

int Garmin_USB_On(void);
int Garmin_USB_Off(void);

#endif /* garmin_wrapper_h */
