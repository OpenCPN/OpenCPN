/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  NMEA Data Object
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
 *
 *
 *
 *
 */


#ifndef __DATASTREAM_H__
#define __DATASTREAM_H__

#ifdef __MINGW32__
#undef IPV6STRICT    // mingw FTBS fix:  missing struct ip_mreq
#include <windows.h>
#endif

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled header


#include <wx/datetime.h>


#ifdef __WXGTK__
// newer versions of glib define its own GSocket but we unfortunately use this
// name in our own (semi-)public header and so can't change it -- rename glib
// one instead
//#include <gtk/gtk.h>
#define GSocket GlibGSocket
#include "wx/socket.h"
#undef GSocket
#else
#include "wx/socket.h"
#endif

#ifndef __WXMSW__
#include <sys/socket.h>                 // needed for (some) Mac builds
#include <netinet/in.h>
#endif

#ifdef __WXMSW__
// #include <windows.h>
// #include <dbt.h>
// #include <winioctl.h>
// #include <initguid.h>
#endif
#include <string>
#include "ConnectionParams.h"
#include "dsPortType.h"

//----------------------------------------------------------------------------
//   constants
//----------------------------------------------------------------------------
#ifndef PI
#define PI        3.1415926535897931160E0      /* pi */
#endif

#define TIMER_SOCKET   7006

//      Error codes, returned by GetLastError()
enum {
    DS_ERROR_PORTNOTFOUND
};


//      End-of-sentence types
enum {
    DS_EOS_CRLF,
    DS_EOS_LF,
    DS_EOS_CR
};

//      Serial port handshake type
enum {
    DS_HANDSHAKE_NONE,
    DS_HANDSHAKE_XON_XOFF
};

#define DS_SOCKET_ID             5001
#define DS_SERVERSOCKET_ID       5002
#define DS_ACTIVESERVERSOCKET_ID 5003

#define     MAX_RX_MESSSAGE_SIZE  4096
#define     RX_BUFFER_SIZE        4096


// Class declarations





// Class declarations
class OCP_DataStreamInput_Thread;
class DataStream;
class GarminProtocolHandler;

extern  const wxEventType wxEVT_OCPN_DATASTREAM;
extern  const wxEventType wxEVT_OCPN_THREADMSG;

bool CheckSumCheck(const std::string& sentence);

//----------------------------------------------------------------------------
// DataStream
//
//      Physical port is specified by a string in the class ctor.
//      Examples strings:
//              Serial:/dev/ttyS0               (Standard serial port)
//              Serial:COM4
//              TCP:192.168.1.1:5200            (TCP source, address and port specified)
//              GPSD:192.168.2.3:5400           (GPSD Wire protocol, address and port specified)
//
//----------------------------------------------------------------------------



class DataStream: public wxEvtHandler
{
protected:
    DataStream(wxEvtHandler *input_consumer,
               const ConnectionType conn_type,
               const wxString& Port,
               const wxString& BaudRate,
               dsPortType io_select,
               int priority,
               bool bGarmin,
               int EOS_type,
               int handshake_type);
    DataStream(wxEvtHandler *input_consumer, const ConnectionParams* params);

public:
    virtual ~DataStream();

    virtual void Close();

    bool IsOk() const { return m_bok; }
    wxString GetPort() const { return m_portstring; }
    dsPortType GetIoSelect() const { return m_io_select; }
    int GetPriority() const { return m_priority; }

    virtual bool SendSentence( const wxString &sentence );

    int GetLastError() const { return m_last_error; }

 //    Secondary thread life toggle
 //    Used to inform launching object (this) to determine if the thread can
 //    be safely called or polled, e.g. wxThread->Destroy();
    void SetSecThreadActive(void){m_bsec_thread_active = true;}
    void SetSecThreadInActive(void){m_bsec_thread_active = false;}
    bool IsSecThreadActive() const { return m_bsec_thread_active; }

    void SetChecksumCheck(bool check) { m_bchecksumCheck = check; }

    void SetInputFilter(wxArrayString filter) { m_input_filter = filter; }
    void SetInputFilterType(ListType filter_type) { m_input_filter_type = filter_type; }
    void SetOutputFilter(wxArrayString filter) { m_output_filter = filter; }
    void SetOutputFilterType(ListType filter_type) { m_output_filter_type = filter_type; }
    bool SentencePassesFilter(const wxString& sentence, FilterDirection direction);
    bool ChecksumOK(const std::string& sentence);
    bool GetGarminMode() const { return m_bGarmin_GRMN_mode; }

    wxString GetBaudRate() const { return m_BaudRate; }
    dsPortType GetPortType() const { return m_io_select; }
    wxArrayString GetInputSentenceList() const { return m_input_filter; }
    wxArrayString GetOutputSentenceList() const { return m_output_filter; }
    ListType GetInputSentenceListType() const { return m_input_filter_type; }
    ListType GetOutputSentenceListType() const { return m_output_filter_type; }
    bool GetChecksumCheck() const { return m_bchecksumCheck; }
    ConnectionType GetConnectionType() const { return m_connection_type; }
    const ConnectionParams* GetConnectionParams() const { return &m_params; }
    int                 m_Thread_run_flag;

protected:
    void SetOk(bool ok) { m_bok = ok; };

    void SetGarminProtocolHandler(GarminProtocolHandler *garminHandler) {
        m_GarminHandler = garminHandler;
    }

    void SetSecondaryThread(OCP_DataStreamInput_Thread *secondary_Thread) {
        m_pSecondary_Thread = secondary_Thread;
    }
    OCP_DataStreamInput_Thread* GetSecondaryThread() { return m_pSecondary_Thread; }
    void SetThreadRunFlag(int run) { m_Thread_run_flag = run; }

    wxEvtHandler* GetConsumer() { return m_consumer; }

    void SetConnectTime(wxDateTime time) { m_connect_time = time; }
    wxDateTime GetConnectTime() { return m_connect_time; }


private:
    virtual void Open();


    bool                m_bok;
    wxEvtHandler        *m_consumer;
    wxString            m_portstring;
    wxString            m_BaudRate;
    dsPortType          m_io_select;
    int                 m_priority;
    int                 m_handshake;

    OCP_DataStreamInput_Thread *m_pSecondary_Thread;
    bool                m_bsec_thread_active;
    int                 m_last_error;

    ConnectionType      m_connection_type;

    bool                m_bchecksumCheck;
    wxArrayString       m_input_filter;
    ListType            m_input_filter_type;
    wxArrayString       m_output_filter;
    ListType            m_output_filter_type;

    bool                m_bGarmin_GRMN_mode;
    GarminProtocolHandler *m_GarminHandler;
    wxDateTime          m_connect_time;
    ConnectionParams    m_params;

};

class InternalGPSDataStream : public DataStream {
public:
    InternalGPSDataStream(wxEvtHandler *input_consumer,
                          const ConnectionParams *params) : DataStream(input_consumer, params) {
        Open();
    }

private:
    void Open();
};

class InternalBTDataStream : public DataStream {
public:
    InternalBTDataStream(wxEvtHandler *input_consumer,
                         const ConnectionParams *params) : DataStream(input_consumer, params) {
        Open();
    }

private:
    void Open();
};

class NullDataStream : public DataStream {
public:
    NullDataStream(wxEvtHandler *input_consumer,
                   const ConnectionParams *params) : DataStream(input_consumer, params) {}

};

// Factory methods, preparation to split into subclasses with clearer
// Responsibilities

DataStream* makeDataStream(wxEvtHandler *input_consumer, const ConnectionParams* params);


#endif


