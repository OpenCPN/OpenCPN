/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2013 by David S. Register                               *
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

#ifndef __OCP_DATASTREAMINPUT_THREAD_H__
#define __OCP_DATASTREAMINPUT_THREAD_H__

#include <wx/thread.h>
#include <wx/string.h>
#include <wx/event.h>
#include <wx/arrstr.h>

#include <mutex>                // std::mutex
#include <queue>                // std::queue

#ifdef __WXMSW__
	#include <windows.h>
	#include <initguid.h>
	#include "setupapi.h"                   // presently stored in opencpn/src
#endif

#include "config.h"

#include "dsPortType.h"

#ifdef OCPN_USE_NEWSERIAL
#include "serial/serial.h"
#endif

#define OUT_QUEUE_LENGTH                20
#define MAX_OUT_QUEUE_MESSAGE_LENGTH    100

class DataStream;

template<typename T>
class atomic_queue
{
public:
    size_t size()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queque.size();
    }
    
    bool empty()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queque.empty();
    }
    
    const T& front()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queque.front();
    }
    
    void push( const T& value )
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queque.push(value);
    }
    
    void pop()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queque.pop();
    }
    
private:
    std::queue<T> m_queque;
    mutable std::mutex m_mutex;
};



/**
 * This thread manages reading the data stream from the declared serial port.
 *
 * This thread manages reading the NMEA data stream from the declared source.
 */
class OCP_DataStreamInput_Thread: public wxThread
{
public:

    OCP_DataStreamInput_Thread(DataStream *Launcher,
                                  wxEvtHandler *MessageTarget,
                                  const wxString& PortName,
                                  const wxString& strBaudRate,
                                  dsPortType io_select
                              );

    ~OCP_DataStreamInput_Thread(void);
    void *Entry();
    bool SetOutMsg(const wxString &msg);
    void OnExit(void);


private:
#ifdef OCPN_USE_NEWSERIAL
    serial::Serial m_serial;
    void ThreadMessage(const wxString &msg);
    bool OpenComPortPhysical(const wxString &com_name, int baud_rate);
    void CloseComPortPhysical();
    void Parse_And_Send_Posn(const char *s);
    size_t WriteComPortPhysical(char *msg);
    size_t WriteComPortPhysical(const wxString& string);
#else
    void ThreadMessage(const wxString &msg);
    void Parse_And_Send_Posn(const char *s);
    int OpenComPortPhysical(const wxString &com_name, int baud_rate);
    int CloseComPortPhysical(int fd);
    int WriteComPortPhysical(int port_descriptor, const wxString& string);
    int WriteComPortPhysical(int port_descriptor, char *msg);
    int ReadComPortPhysical(int port_descriptor, int count, unsigned char *p);
    bool CheckComPortPhysical(int port_descriptor);
    
    void HandleASuccessfulRead( char *buf, int nread );
    wxCriticalSection       m_outCritical;
#endif
    wxEvtHandler            *m_pMessageTarget;
    DataStream              *m_launcher;
    wxString                m_PortName;
    wxString                m_FullPortName;

    dsPortType              m_io_select;

    char                    *put_ptr;
    char                    *tak_ptr;

    char                    *rx_buffer;
    char                    *temp_buf;

    unsigned long           error;

    int                     m_gps_fd;
    int                     m_baud;
    int                     m_n_timeout;

    //int                     m_takIndex;
    //int                     m_putIndex;
    //char                    *m_poutQueue[OUT_QUEUE_LENGTH];
    
    atomic_queue<char *>  out_que;
    

#ifdef __WXMSW__
    HANDLE                  m_hSerialComm;
    bool                    m_nl_found;
#endif

};

#endif
