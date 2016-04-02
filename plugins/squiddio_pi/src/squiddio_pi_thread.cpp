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

#include "squiddio_pi_thread.h"
#include "squiddio_pi.h"

IMPLEMENT_DYNAMIC_CLASS( SquiddioEvent, wxEvent )
DEFINE_EVENT_TYPE( myEVT_MYEVENT )

SquiddioEvent::SquiddioEvent()
{
    SetEventType( myEVT_MYEVENT );
}

SquiddioEvent::~SquiddioEvent() {}

wxEvent* SquiddioEvent::Clone() const
{
    SquiddioEvent *newevent = new SquiddioEvent(*this);
    return newevent;
}

/** \brief Worker method of the Squidd.io server communication thread
 *
 * \return void*
 *
 */   
void *SquiddioThread::Entry()
{
    wxMilliSleep(500); //Give everything some time to settle down before we try to do our work
    m_pHandler->SetThreadRunning(true);
    while (!TestDestroy())
    {
        if( !m_bIsWorking )
        {
            m_bIsWorking = true;
            if ( m_bCheckOnline )
            {
                m_pHandler->CheckIsOnline();
                m_bCheckOnline = false;
            }
            else if ( m_bPositionReport )
            {
                //TODO: Separate from logwindow first.
                m_bPositionReport = false;
            }
            m_bIsWorking = false;
        }
        wxMilliSleep(250);
    }
    // signal the event handler that this thread is going to be destroyed
    // NOTE: here we assume that using the m_pHandler pointer is safe,
    // (in this case this is assured by the MyFrame destructor)
    //    wxQueueEvent(m_pHandler, new wxThreadEvent(wxEVT_COMMAND_DBTHREAD_COMPLETED));
    //return (wxThread::ExitCode)0; // success

    return 0;
}

/** \brief Squidd.io server communication worker thread constructor
 *
 * \param handler squiddio_pi* Pointer to the plug-in
 *
 */   
SquiddioThread::SquiddioThread(squiddio_pi * handler) : wxThread()
{
    m_pHandler = handler;
    m_bIsWorking = false;
    m_bCheckOnline = false;
    m_bPositionReport = false;
    m_getdata = false;
    Create();
}

/** \brief Squidd.io server communication worker thread destructor
 */
SquiddioThread::~SquiddioThread()
{
    wxCriticalSectionLocker enter( m_pHandler->m_pThreadCS );
    m_pHandler->m_pThread = NULL;
    m_pHandler->SetThreadRunning( false );
}

/** \brief Entry point to invoke the retrieval of POI list from the Squidd.io server on the background
 *
 * \param lat double
 * \param lon double
 * \return void
 *
 */       
void SquiddioThread::GetData()
{
    m_getdata = true;
}

/** \brief Checks whether Internet connection is available
 *
 * \return void
 *
 */   
void SquiddioThread::CheckIsOnline()
{
    m_bCheckOnline = true;
}

/** \brief Sends a position report
 *
 * \param lat double
 * \param lon double
 * \param sog double
 * \param cog double
 * \return void
 *
 */        
void SquiddioThread::ReportPosition( double lat, double lon, double sog, double cog )
{
    m_reportlat = lat;
    m_reportlon = lon;
    m_reportsog = sog;
    m_reportcog = cog;
    m_bPositionReport = true;
}

