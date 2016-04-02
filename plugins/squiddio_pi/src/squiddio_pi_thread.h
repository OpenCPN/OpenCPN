/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Squidd.io Plugin background thread
 * Author:   Pavel Kalian
 *
 ***************************************************************************
 *   Copyright (C) 2015 by Mauro Calvi                                     *
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
 ***************************************************************************
 */

#ifndef _SQUIDDIOPITHREAD_H_
#define _SQUIDDIOPITHREAD_H_

#include <wx/wxprec.h>

#ifndef  WX_PRECOMP
  #include <wx/wx.h>
#endif //precompiled headers

class squiddio_pi;

BEGIN_DECLARE_EVENT_TYPES()
	DECLARE_EVENT_TYPE( myEVT_MYEVENT, 1 )
END_DECLARE_EVENT_TYPES()

class SquiddioEvent : public wxEvent
{
public:
    SquiddioEvent();
    ~SquiddioEvent();
    wxEvent *Clone() const;
    int event_type;
    
    DECLARE_DYNAMIC_CLASS(SquiddioEvent)
};

#define EVT_MYEVENT(func)                      \
	DECLARE_EVENT_TABLE_ENTRY( myEVT_MYEVENT,  \
		-1,                                    \
		-1,                                    \
		(wxObjectEventFunction)                \
		(myEventFunction) & func,      \
		(wxObject *) NULL ),

class SquiddioThread : public wxThread
{
public:
    SquiddioThread( squiddio_pi * handler );
    ~SquiddioThread();
    void *Entry();
    
    bool IsWorking() { return m_bIsWorking; }
    void GetData();
    void CheckIsOnline();
    void ReportPosition( double lat, double lon, double sog, double cog );
protected:
    squiddio_pi *m_pHandler;

private:
    bool m_bIsWorking;
    bool m_bCheckOnline;
    bool m_bPositionReport;
    int m_QueryRegion;
    double m_reportlat;
    double m_reportlon;
    double m_reportsog;
    double m_reportcog;
    bool m_getdata;
};

#endif
