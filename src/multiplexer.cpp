/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  NMEA Data Multiplexer Object
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
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
 ***************************************************************************/

#include "wx/wx.h"

#include "multiplexer.h"

extern PlugInManager    *g_pi_manager;

Multiplexer::Multiplexer()
{
    m_aisconsumer = NULL;
    m_gpsconsumer = NULL;
    Connect(wxEVT_OCPN_DATASTREAM, (wxObjectEventFunction)(wxEventFunction)&Multiplexer::OnEvtStream);
    m_pdatastreams = new wxArrayOfDataStreams();
}

Multiplexer::~Multiplexer()
{
    ClearStreams();
    delete m_pdatastreams;
}

void Multiplexer::AddStream(DataStream *stream)
{
    m_pdatastreams->Add(stream);
}

void Multiplexer::StopAllStreams()
{
    for (size_t i = 0; i < m_pdatastreams->Count(); i++)
    {
        m_pdatastreams->Item(i)->Close();
    }
}

void Multiplexer::ClearStreams()
{
    for (size_t i = 0; i < m_pdatastreams->Count(); i++)
    {
        m_pdatastreams->Item(i)->Close();
    }
    m_pdatastreams->Clear();
}

void Multiplexer::SendNMEAMessage( wxString &msg )
{
    //Send to all the outputs
    for (size_t i = 0; i < m_pdatastreams->Count(); i++)
    {
        DataStream* s = m_pdatastreams->Item(i);
        if ( s->IsOk() && (s->GetIoSelect() == DS_TYPE_INPUT_OUTPUT || s->GetIoSelect() == DS_TYPE_OUTPUT) )
            s->SendSentence(msg);
    }
    //Send to plugins
    if ( g_pi_manager )
        g_pi_manager->SendNMEASentenceToAllPlugIns( msg );
}

void Multiplexer::SetAISHandler(wxEvtHandler *handler)
{
    m_aisconsumer = handler;
}

void Multiplexer::SetGPSHandler(wxEvtHandler *handler)
{
    m_gpsconsumer = handler;
}

void Multiplexer::OnEvtStream(OCPN_DataStreamEvent& event)
{
    wxString message = event.GetNMEAString();
    wxString ds = event.GetDataSource();

    if( !message.IsEmpty() )
    {
        //Send to all the other outputs
        for (size_t i = 0; i < m_pdatastreams->Count(); i++)
        {
            DataStream* s = m_pdatastreams->Item(i);
            if ( ds != s->GetPort() )
                if ( s->IsOk() )
                    if ( s->GetIoSelect() == DS_TYPE_INPUT_OUTPUT || s->GetIoSelect() == DS_TYPE_OUTPUT )
                        s->SendSentence(message);
        }
        //Send to core consumers
        if( message.Mid(3,3).IsSameAs(_T("VDM")) ||
                message.Mid(3,3).IsSameAs(_T("VDO")) ||
                message.Mid(1,5).IsSameAs(_T("FRPOS")) ||
                message.Mid(1,2).IsSameAs(_T("CD")) )
        {
            if( m_aisconsumer )
                m_aisconsumer->AddPendingEvent(event);
        }
        else
        {
            if( m_gpsconsumer )
                m_gpsconsumer->AddPendingEvent(event);
        }
        //Send to plugins
        if ( g_pi_manager )
            g_pi_manager->SendNMEASentenceToAllPlugIns( message );
    }
}

