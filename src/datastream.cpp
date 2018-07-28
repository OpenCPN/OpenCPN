/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  NMEA Data Stream Object
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
 ***************************************************************************

 ***************************************************************************
 *  Parts of this file were adapted from source code found in              *
 *  John F. Waers (jfwaers@csn.net) public domain program MacGPS45         *
 ***************************************************************************
 *
 */
#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

#include <wx/datetime.h>

#include <stdlib.h>
#include <math.h>
#include <time.h>

#ifndef __WXMSW__
#include <arpa/inet.h>
#include <netinet/tcp.h>
#endif

#include "dychart.h"

#include "datastream.h"
#include "NetworkDataStream.h"
#include "SerialDataStream.h"

#include "OCPN_DataStreamEvent.h"
#include "OCP_DataStreamInput_Thread.h"
// #include "garmin/jeeps/garmin_wrapper.h"
#include "GarminProtocolHandler.h"

#ifdef __OCPN__ANDROID__
#include "androidUTIL.h"
#endif

#include <vector>
#include <wx/socket.h>
#include <wx/log.h>
#include <wx/memory.h>
#include <wx/chartype.h>
#include <wx/wx.h>
#include <wx/sckaddr.h>

#if !defined(NAN)
static const long long lNaN = 0xfff8000000000000;
#define NAN (*(double*)&lNaN)
#endif

const wxEventType wxEVT_OCPN_DATASTREAM = wxNewEventType();

extern bool g_benableUDPNullHeader;

#define N_DOG_TIMEOUT   5

#ifdef __WXMSW__
// {2C9C45C2-8E7D-4C08-A12D-816BBAE722C0}
DEFINE_GUID(GARMIN_GUID1, 0x2c9c45c2L, 0x8e7d, 0x4c08, 0xa1, 0x2d, 0x81, 0x6b, 0xba, 0xe7, 0x22, 0xc0);
#endif

#ifdef __OCPN__ANDROID__
#include <netdb.h>
int gethostbyaddr_r(const char *, int, int, struct hostent *, char *, size_t, struct hostent **, int *)
{
    wxLogMessage(_T("Called stub gethostbyaddr_r()"));
    return 0;
}
#endif


bool CheckSumCheck(const std::string& sentence)
{
    size_t check_start = sentence.find('*');
    if(check_start == wxString::npos || check_start > sentence.size() - 3)
        return false; // * not found, or it didn't have 2 characters following it.
        
    std::string check_str = sentence.substr(check_start+1,2);
    unsigned long checksum;
    //    if(!check_str.ToULong(&checksum,16))
    if(!(checksum = strtol(check_str.c_str(), 0, 16)))
        return false;
    
    unsigned char calculated_checksum = 0;
    for(std::string::const_iterator i = sentence.begin()+1; i != sentence.end() && *i != '*'; ++i)
        calculated_checksum ^= static_cast<unsigned char> (*i);
    
    return calculated_checksum == checksum;
    
}


DataStream* makeDataStream(wxEvtHandler *input_consumer, const ConnectionParams* params)
{
    wxLogMessage( wxString::Format(_T("makeSerialDataStream %s"),
            params->GetDSPort().c_str()) );
    switch (params->Type) {
        case SERIAL:
            return new SerialDataStream(input_consumer, params);
        case NETWORK:
            return new NetworkDataStream(input_consumer, params);
        case INTERNAL_GPS:
            return new InternalGPSDataStream(input_consumer, params);
        case INTERNAL_BT:
            return new InternalBTDataStream(input_consumer, params);
        default:
            return new NullDataStream(input_consumer, params);
    }
}


//------------------------------------------------------------------------------
//    DataStream Implementation
//------------------------------------------------------------------------------


// constructor
DataStream::DataStream(wxEvtHandler *input_consumer,
             const ConnectionType conn_type,         
             const wxString& Port,
             const wxString& BaudRate,
             dsPortType io_select,
             int priority,
             bool bGarmin,
             int EOS_type,
             int handshake_type)
    :
    m_Thread_run_flag(-1),
    m_bok(false),
    m_consumer(input_consumer),
    m_portstring(Port),
    m_BaudRate(BaudRate),
    m_io_select(io_select),
    m_priority(priority),
    m_handshake(handshake_type),
    m_pSecondary_Thread(NULL),
    m_connection_type(conn_type),
    m_bGarmin_GRMN_mode(bGarmin),
    m_GarminHandler(NULL),
    m_params()
{
    wxLogMessage( _T("Classic CTOR"));

    SetSecThreadInActive();

    Open();
}

DataStream::DataStream(wxEvtHandler *input_consumer,
             const ConnectionParams* params)
    :
    m_Thread_run_flag(-1),
    m_bok(false),
    m_consumer(input_consumer),
    m_portstring(params->GetDSPort()),
    m_io_select(params->IOSelect),
    m_priority(params->Priority),
    m_handshake(DS_HANDSHAKE_NONE),
    m_pSecondary_Thread(NULL),
    m_connection_type(params->Type),
    m_bGarmin_GRMN_mode(params->Garmin),
    m_GarminHandler(NULL),
    m_params(*params)
{
    m_BaudRate = wxString::Format(wxT("%i"), params->Baudrate),
    SetSecThreadInActive();
    
    wxLogMessage( _T("ConnectionParams CTOR"));

    // Open();

    SetInputFilter(params->InputSentenceList);
    SetInputFilterType(params->InputSentenceListType);
    SetOutputFilter(params->OutputSentenceList);
    SetOutputFilterType(params->OutputSentenceListType);
    SetChecksumCheck(params->ChecksumCheck);
}

void DataStream::Open(void)
{
    //  Open a port
    wxLogMessage( wxString::Format(_T("Opening NMEA Datastream %s"), m_portstring.c_str()) );
    SetOk(false);
    m_connect_time = wxDateTime::Now();
}

void InternalBTDataStream::Open(void)
{
#ifdef __OCPN__ANDROID__
    SetOk(androidStartBT(m_consumer, m_portstring ));
#endif
}

void InternalGPSDataStream::Open(void)
{
#ifdef __OCPN__ANDROID__
    androidStartNMEA(m_consumer);
    SetOk(true)
#endif

}


DataStream::~DataStream()
{
    Close();
}

void DataStream::Close()
{
    wxLogMessage( wxString::Format(_T("Closing NMEA Datastream %s"), m_portstring.c_str()) );
    
//    Kill off the Secondary RX Thread if alive
    if(m_pSecondary_Thread)
    {
        if(m_bsec_thread_active)              // Try to be sure thread object is still alive
        {
            wxLogMessage(_T("Stopping Secondary Thread"));

            m_Thread_run_flag = 0;
            int tsec = 10;
            while((m_Thread_run_flag >= 0) && (tsec--))
                wxSleep(1);

            wxString msg;
            if(m_Thread_run_flag < 0)
                  msg.Printf(_T("Stopped in %d sec."), 10 - tsec);
            else
                 msg.Printf(_T("Not Stopped after 10 sec."));
            wxLogMessage(msg);
        }

        m_pSecondary_Thread = NULL;
        m_bsec_thread_active = false;
    }

    //  Kill off the Garmin handler, if alive
    if(m_GarminHandler) {
        m_GarminHandler->Close();
        delete m_GarminHandler;
    }
    

    if(m_connection_type == INTERNAL_GPS){
#ifdef __OCPN__ANDROID__
        androidStopNMEA();
#endif
    }
    else if(m_connection_type == INTERNAL_BT){
#ifdef __OCPN__ANDROID__
        androidStopBT();
#endif
    }
    
        
}

bool DataStream::SentencePassesFilter(const wxString& sentence, FilterDirection direction)
{
    wxArrayString filter;
    bool listype = false;

    if (direction == FILTER_INPUT)
    {
        filter = m_input_filter;
        if (m_input_filter_type == WHITELIST)
            listype = true;
    }
    else
    {
        filter = m_output_filter;
        if (m_output_filter_type == WHITELIST)
            listype = true;
    }
    if (filter.Count() == 0) //Empty list means everything passes
        return true;

    wxString fs;
    for (size_t i = 0; i < filter.Count(); i++)
    {
        fs = filter.Item(i);
        switch (fs.Length())
        {
            case 2:
                if (fs == sentence.Mid(1, 2))
                    return listype;
                break;
            case 3:
                if (fs == sentence.Mid(3, 3))
                    return listype;
                break;
            case 5:
                if (fs == sentence.Mid(1, 5))
                    return listype;
                break;
        }
    }
    return !listype;
}

bool DataStream::ChecksumOK( const std::string &sentence )
{
    if (!m_bchecksumCheck)
        return true;

    return CheckSumCheck(sentence);
    
}


bool DataStream::SendSentence( const wxString &sentence )
{
    wxString payload = sentence;
    if( !sentence.EndsWith(_T("\r\n")) )
        payload += _T("\r\n");


    return true;
}

