/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  OpenCPN Android support utilities
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2015 by David S. Register                               *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 **************************************************************************/

#ifndef ANDROIDUTIL_H
#define ANDROIDUTIL_H

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif //precompiled headers


#define GPS_OFF                         0
#define GPS_ON                          1
#define GPS_PROVIDER_AVAILABLE          2
#define GPS_SHOWPREFERENCES             3

extern bool androidUtilInit( void );

extern bool androidGetMemoryStatus( int *mem_total, int *mem_used );
extern double GetAndroidDisplaySize();
extern wxSize getAndroidDisplayDimensions( void );
extern bool LoadQtStyleSheet(wxString &sheet_file);

extern void androidShowBusyIcon();
extern void androidHideBusyIcon();


extern bool androidStartNMEA(wxEvtHandler *consumer);
extern bool androidStopNMEA();
extern wxString androidGPSService(int parm);
extern bool androidDeviceHasGPS();

extern bool androidDeviceHasBlueTooth();
extern bool androidStartBluetoothScan();
extern bool androidStopBluetoothScan();
extern wxArrayString androidGetBluetoothScanResults();
extern bool androidStartBT(wxEvtHandler *consumer, wxString mac_address );


#endif   //guard