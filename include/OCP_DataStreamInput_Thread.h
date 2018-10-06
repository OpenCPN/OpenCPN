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

#include <queue>                // std::queue

#ifdef __WXMSW__
	#include <windows.h>
	#include <winioctl.h>
	#include <initguid.h>
	#include "setupapi.h"                   // presently stored in opencpn/src
#endif

#include "dsPortType.h"

#define OUT_QUEUE_LENGTH                20
#define MAX_OUT_QUEUE_MESSAGE_LENGTH    100

class DataStream;

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
    
    std::queue<char *>  out_que;
    

#ifdef __WXMSW__
    HANDLE                  m_hSerialComm;
    bool                    m_nl_found;
#endif

};

#endif
