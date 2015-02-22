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

#include "OCP_DataStreamInput_Thread.h"
#include "OCPN_DataStreamEvent.h"
#include "datastream.h"
#include "dychart.h"

#ifdef __POSIX__
	#include <sys/termios.h>
#endif

#define DS_RX_BUFFER_SIZE 4096

extern const wxEventType wxEVT_OCPN_DATASTREAM;
extern const wxEventType wxEVT_OCPN_THREADMSG;

#include "chart1.h"
extern MyFrame *gFrame;

#ifdef __WXMSW__
extern int g_total_NMEAerror_messages;
extern int g_nNMEADebug;

#include <queue>
#endif

typedef enum DS_ENUM_BUFFER_STATE
{
      DS_RX_BUFFER_EMPTY,
      DS_RX_BUFFER_FULL
}_DS_ENUM_BUFFER_STATE;


/**
 * This thread manages reading the data stream from the declared serial port.
 *
 * This thread manages reading the NMEA data stream from the declared source.
 */

OCP_DataStreamInput_Thread::OCP_DataStreamInput_Thread(DataStream *Launcher,
                                                       wxEvtHandler *MessageTarget,
                                                       const wxString& PortName,
                                                       const wxString& strBaudRate,
                                                       dsPortType io_select
                                                      )
{
    m_launcher = Launcher;                          // This thread's immediate "parent"

    m_pMessageTarget = MessageTarget;

    m_PortName = PortName;
    m_FullPortName = _T("Serial:") + PortName;

    m_io_select = io_select;

    rx_buffer = new char[DS_RX_BUFFER_SIZE + 1];
    temp_buf = new char[DS_RX_BUFFER_SIZE + 1];

    put_ptr = rx_buffer;                            // local circular queue
    tak_ptr = rx_buffer;

    m_baud = 4800;                                  // default
    long lbaud;
    if(strBaudRate.ToLong(&lbaud))
        m_baud = (int)lbaud;

    Create();
    
}

OCP_DataStreamInput_Thread::~OCP_DataStreamInput_Thread(void)
{
    delete[] rx_buffer;
    delete[] temp_buf;
}

void OCP_DataStreamInput_Thread::OnExit(void)
{
}

//      Sadly, the thread itself must implement the underlying OS serial port
//      in a very machine specific way....

#ifdef __WXQT__
#define __POSIX__
#include <termios.h>
#include <unistd.h>
#endif


#ifdef __POSIX__
//    Entry Point
void *OCP_DataStreamInput_Thread::Entry()
{

    bool not_done = true;
    bool nl_found = false;
    wxString msg;


    //    Request the com port from the comm manager
    if ((m_gps_fd = OpenComPortPhysical(m_PortName, m_baud)) < 0)
    {
        wxString msg(_T("NMEA input device open failed: "));
        msg.Append(m_PortName);
        ThreadMessage(msg);
        goto thread_exit;
    }

    m_launcher->SetSecThreadActive();               // I am alive

//    The main loop

    while((not_done) && (m_launcher->m_Thread_run_flag > 0))
    {
        if(TestDestroy())
            not_done = false;                               // smooth exit

      //    Blocking, timeout protected read of one character at a time
      //    Timeout value is set by c_cc[VTIME]
      //    Storing incoming characters in circular buffer
      //    And watching for new line character
      //     On new line character, send notification to parent
        char next_byte = 0;
        ssize_t newdata;
        newdata = read(m_gps_fd, &next_byte, 1);            // read of one char
                                                            // return (-1) if no data available, timeout

#ifdef __WXOSX__
        if (newdata < 0 )
            wxThread::Sleep(100) ;
#endif


      // Fulup patch for handling hot-plug or wakeup events
      // from serial port drivers
        {
              static int maxErrorLoop;

              if (newdata > 0)
              {
        // we have data, so clear error
                    maxErrorLoop =0;
              }
              else
              {
        // no need to retry every 1ms when on error
                    sleep (1);

        // if we have more no character for 5 second then try to reopen the port
                    if (maxErrorLoop++ > 5)
                    {

        // do not retry for the next 5s
                          maxErrorLoop = 0;

        // free old unplug current port
                          CloseComPortPhysical(m_gps_fd);
        //    Request the com port from the comm manager
                          if ((m_gps_fd = OpenComPortPhysical(m_PortName, m_baud)) < 0)  {
//                                wxString msg(_T("NMEA input device open failed (will retry): "));
//                                msg.Append(m_PortName);
//                                ThreadMessage(msg);
                          } else {
//                                wxString msg(_T("NMEA input device open on hotplug OK: "));
//                                msg.Append(m_PortName);
//                                ThreadMessage(msg);
                          }
                    }
              }
        }

        //  And process any character

        if(newdata > 0)
        {
            nl_found = false;
            *put_ptr++ = next_byte;
            if((put_ptr - rx_buffer) > DS_RX_BUFFER_SIZE)
            put_ptr = rx_buffer;

            if(0x0a == next_byte)
            nl_found = true;


            //    Found a NL char, thus end of message?
            if(nl_found)
            {
                char *tptr;
                char *ptmpbuf;

                //    Copy the message into a temporary _buffer

                tptr = tak_ptr;
                ptmpbuf = temp_buf;

                while((*tptr != 0x0a) && (tptr != put_ptr))
                {
                    *ptmpbuf++ = *tptr++;

                    if((tptr - rx_buffer) > DS_RX_BUFFER_SIZE)
                    tptr = rx_buffer;

                    wxASSERT_MSG((ptmpbuf - temp_buf) < DS_RX_BUFFER_SIZE, (const wxChar *)"temp_buf overrun1");

                }
                if((*tptr == 0x0a) && (tptr != put_ptr))    // well formed sentence
                {
                    *ptmpbuf++ = *tptr++;
                    if((tptr - rx_buffer) > DS_RX_BUFFER_SIZE)
                    tptr = rx_buffer;

                    wxASSERT_MSG((ptmpbuf - temp_buf) < DS_RX_BUFFER_SIZE, (const wxChar *)"temp_buf overrun2");

                    *ptmpbuf = 0;

                    tak_ptr = tptr;

                //    Message is ready to parse and send out
                //    Messages may be coming in as <blah blah><lf><cr>.
                //    One example device is KVH1000 heading sensor.    
                //    If that happens, the first character of a new captured message will the <cr>,
                //    and we need to discard it.    
                //    This is out of spec, but we should handle it anyway   
                    if(temp_buf[0] == '\r')
                        Parse_And_Send_Posn(&temp_buf[1]);
                    else
                        Parse_And_Send_Posn(temp_buf);
                    
                }

            }                   //if nl
        }                       // if newdata > 0

        //      Check for any pending output message

        m_outCritical.Enter();
        {
            bool b_qdata = !out_que.empty();
            
            while(b_qdata){
                    
                    //  Take a copy of message
                    char *qmsg = out_que.front();
                    char msg[MAX_OUT_QUEUE_MESSAGE_LENGTH];
                    strncpy( msg, qmsg, MAX_OUT_QUEUE_MESSAGE_LENGTH-1 );
                    out_que.pop();
                    free(qmsg);

                    m_outCritical.Leave();
                    WriteComPortPhysical(m_gps_fd, msg);
                    m_outCritical.Enter();
                    
                    b_qdata = !out_que.empty();
                
                
            } //while b_qdata
        }
        m_outCritical.Leave();
        
        
    }                          // the big while...

//          Close the port cleanly
    CloseComPortPhysical(m_gps_fd);

thread_exit:
    m_launcher->SetSecThreadInActive();             // I am dead
    m_launcher->m_Thread_run_flag = -1;

    return 0;
}


#endif          //__POSIX__


#ifdef __WXMSW__
#define SERIAL_OVERLAPPED

//    Entry Point
void *OCP_DataStreamInput_Thread::Entry()
{
    wxString msg;
    OVERLAPPED osReader = {0};
    OVERLAPPED osWriter = {0};
    
    m_launcher->SetSecThreadActive();               // I am alive
    
    wxSleep(1);         //  allow Bluetooth SPP connections to re-cycle after the parent's test for existence.
                        //  In the MS Bluetooth stack, there is apparently a minimum time required
                        //  between CloseHandle() and CreateFile() on the same port.
                        // FS#1008

    bool not_done;
    HANDLE hSerialComm = (HANDLE)(-1);

       //    Request the com port from the comm manager
    if ((m_gps_fd = OpenComPortPhysical(m_PortName, m_baud)) < 0)
    {
        wxString msg(_T("NMEA input device initial open failed: "));
        msg.Append(m_PortName);
        wxString msg_error;
        msg_error.Printf(_T("...GetLastError():  %d"), GetLastError());
        msg.Append(msg_error);

        ThreadMessage(msg);
        m_gps_fd = NULL;
//        goto thread_exit;
    }

    hSerialComm = (HANDLE)m_gps_fd;

    int n_reopen_wait = 2000;
    
    COMMTIMEOUTS timeouts;

    //  Short read timeout for faster response
    timeouts.ReadIntervalTimeout = 1;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.ReadTotalTimeoutConstant = 0;
    timeouts.WriteTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant = 500; 
    
    
    if(m_gps_fd){
        if (!SetCommTimeouts(hSerialComm, &timeouts)){ // Error setting time-outs.
            CloseComPortPhysical(m_gps_fd);
            m_gps_fd = NULL;
        }            
    }

      // Create the reader overlapped event.
    osReader.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    // Create the writer overlapped event.
    osWriter.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    
      
    not_done = true;

#define READ_BUF_SIZE 200
    char szBuf[READ_BUF_SIZE];
    
#define READ_TIMEOUT      50      // milliseconds
    
    DWORD dwRead;
    DWORD dwWritten;
    DWORD dwRes;
    DWORD dwToWrite;

    int n_timeout = 0;
    int max_timeout = 100;
    
    bool fWaitingOnRead = false;
    bool fWaitingOnWrite = false;
    
    
//    The main loop

    while(not_done)
    {
        if(TestDestroy())
            not_done = false;                               // smooth exit

       //    Was port closed due to error condition?
        while(!m_gps_fd)
        {
            if((TestDestroy()) || (m_launcher->m_Thread_run_flag == 0))
                goto thread_exit;                               // smooth exit

            if(n_reopen_wait){
                int nrwd10 = wxMax(1, n_reopen_wait /10);

                while(n_reopen_wait > 0){
                    wxThread::Sleep(nrwd10);                        // stall for a bit
                    
                    if((TestDestroy()) || (m_launcher->m_Thread_run_flag == 0))
                        goto thread_exit;                               // smooth exit
                    
                    n_reopen_wait -= nrwd10;
                }
                        
                n_reopen_wait = 0;
            }


            if ((m_gps_fd = OpenComPortPhysical(m_PortName, m_baud)) > 0)
            {
                hSerialComm = (HANDLE)m_gps_fd;

                wxThread::Sleep(100);                        // stall for a bit
                
                if (!SetCommTimeouts(hSerialComm, &timeouts)){ // Error setting time-outs.
                      int errt = GetLastError();                // so just retry
                      CloseComPortPhysical(m_gps_fd);
                      m_gps_fd = NULL;
                      
                }
                
                fWaitingOnWrite = FALSE;
                fWaitingOnRead = FALSE;
                n_timeout = 0;
                
            }
            else
            {
                m_gps_fd = 0;
                
                int nwait = 2000;
                while(nwait > 0){
                    wxThread::Sleep(200);                        // stall for a bit
                    
                    if((TestDestroy()) || (m_launcher->m_Thread_run_flag == 0))
                        goto thread_exit;                               // smooth exit
                        
                    nwait -= 200;
                }
            }
        }

        if( (m_io_select == DS_TYPE_INPUT_OUTPUT) || (m_io_select == DS_TYPE_OUTPUT) ) {
                m_outCritical.Enter();
                bool b_qdata = !out_que.empty();
                    
                bool b_break = false;
                while(!b_break && b_qdata){
                    char msg[MAX_OUT_QUEUE_MESSAGE_LENGTH];
                    
//                    printf("wl %d\n", out_que.size());
                    {
                        
                        if(fWaitingOnWrite){
//                            printf("wow\n");
                            dwRes = WaitForSingleObject(osWriter.hEvent, INFINITE);
                            
                            switch(dwRes)
                            {
                                case WAIT_OBJECT_0:
                                    if (!GetOverlappedResult(hSerialComm, &osWriter, &dwWritten, FALSE)) {
                                        if (GetLastError() == ERROR_OPERATION_ABORTED){
                                            //    UpdateStatus("Write aborted\r\n");
                                        }
                                        else{
                                            b_break = true;
                                        }
                                    }
                                    
                                    if (dwWritten != dwToWrite) {
                                        //ErrorReporter("Error writing data to port (overlapped)");
                                    }
                                    else {
                                        // Delayed write completed
                                        fWaitingOnWrite = false;
//                                        printf("-wow\n");
                                    }
                                    break;
                                    
                                //                
                                // wait timed out
                                //
                                case WAIT_TIMEOUT:
                                    break;
                                    
                                case WAIT_FAILED:
                                default:
                                    break;
                            }
                            
                        }
                        if(!fWaitingOnWrite){          // not waiting on Write, OK to issue another
                        
                        //  Take a copy of message
                            char *qmsg = out_que.front();
                            strncpy( msg, qmsg, MAX_OUT_QUEUE_MESSAGE_LENGTH-1 );
                            out_que.pop();
                            free(qmsg);
 
                            dwToWrite = strlen(msg);
                            //
                            // issue write
                            //
                            n_timeout = 0;
                            
//                            printf("w\n");
                            if (!WriteFile(hSerialComm, msg, dwToWrite, &dwWritten, &osWriter)) {
                                if (GetLastError() == ERROR_IO_PENDING) { 
                                //
                                // write is delayed
                                //
                                    fWaitingOnWrite = true;
//                                    printf("+wow\n");
                                }
                                else{
                                    b_break = true;
                                }
                            }
                            else {
                            //
                            // writefile returned immediately
                            //
                            }

                            b_qdata = !out_que.empty();
                            
                        }
                    }
                    
                } //while b_qdata
                

            
                m_outCritical.Leave();
        }
        
        
        //
        // if no read is outstanding, then issue another one
        //
//        printf("r\n");
        if (!fWaitingOnRead) {
            if (!ReadFile(hSerialComm, szBuf, READ_BUF_SIZE, &dwRead, &osReader)) {
                if (GetLastError() != ERROR_IO_PENDING) {  // read not delayed?
                        CloseComPortPhysical(m_gps_fd);
                        m_gps_fd = NULL;
                        fWaitingOnRead = FALSE;
                        n_reopen_wait = 2000;
                }
                else
                    fWaitingOnRead = TRUE;
            }
            else {    // read completed immediately
                n_timeout = 0;
                
                if (dwRead)
                    HandleASuccessfulRead(szBuf, dwRead);
            }
        }
        
        //
        // wait for pending operations to complete
        //
        if ( fWaitingOnRead ) {
            dwRes = WaitForSingleObject(osReader.hEvent, READ_TIMEOUT);
            
            switch(dwRes)
            {
                //
                // read completed
                //
                case WAIT_OBJECT_0:
                    if (!GetOverlappedResult(hSerialComm, &osReader, &dwRead, FALSE)) {
                        int err = GetLastError();
                        if (GetLastError() == ERROR_OPERATION_ABORTED){
                        }
                        else{
                                //      Some other error
                            n_reopen_wait = 2000;
                            CloseComPortPhysical(m_gps_fd);
                            m_gps_fd = 0;
                        }
                    }
                    else {      // read completed successfully
                        if (dwRead)
                            HandleASuccessfulRead(szBuf, dwRead);
                    }
                    
                    fWaitingOnRead = FALSE;
                    n_timeout = 0;
                    
                    break;
                    
                case WAIT_TIMEOUT:
                    n_timeout++;
                        
                    break;                       
                    
                default:                // error of some kind with handles
                    fWaitingOnRead = FALSE;
                    break;
            }
        }
        
        if(m_launcher->m_Thread_run_flag <= 0)
            not_done = false;
        
    }           // the big while...



thread_exit:

//          Close the port cleanly
    CloseComPortPhysical(m_gps_fd);

    m_launcher->SetSecThreadInActive();             // I am dead
    m_launcher->m_Thread_run_flag = -1;

    return 0;

}

void OCP_DataStreamInput_Thread::HandleASuccessfulRead( char *szBuf, int nread )
{
    if(nread > 0)
    {
        if((g_total_NMEAerror_messages < g_nNMEADebug) && (g_nNMEADebug > 1000))
        {
            g_total_NMEAerror_messages++;
            wxString msg;
            msg.Printf(_T("NMEA activity...%d bytes"), nread);
            ThreadMessage(msg);
        }
        
        int nchar = nread;
        char *pb = szBuf;
        
        while(nchar)
        {
            if(0x0a == *pb)
                m_nl_found = true;
            
            *put_ptr++ = *pb++;
            if((put_ptr - rx_buffer) > DS_RX_BUFFER_SIZE)
                put_ptr = rx_buffer;
            
            nchar--;
        }
        if((g_total_NMEAerror_messages < g_nNMEADebug) && (g_nNMEADebug > 1000))
        {
            g_total_NMEAerror_messages++;
            wxString msg1 = _T("Buffer is: ");
            int nc = nread;
            char *pb = szBuf;
            while(nc)
            {
                msg1.Append(*pb++);
                nc--;
            }
            ThreadMessage(msg1);
        }
    }

    //    Found a NL char, thus end of message?
    if(m_nl_found)
    {
        char *tptr;
        char *ptmpbuf;
        
        bool partial = false;
        while (!partial)
        {
            
            //    Copy the message into a temp buffer
            
            tptr = tak_ptr;
            ptmpbuf = temp_buf;
            
            while((*tptr != 0x0a) && (tptr != put_ptr))
            {
                *ptmpbuf++ = *tptr++;
                
                if((tptr - rx_buffer) > RX_BUFFER_SIZE)
                    tptr = rx_buffer;
                //                    wxASSERT_MSG((ptmpbuf - temp_buf) < DS_RX_BUFFER_SIZE, "temp_buf overrun");
            }
            
            if((*tptr == 0x0a) && (tptr != put_ptr))    // well formed sentence
                    {
                        *ptmpbuf++ = *tptr++;
                        if((tptr - rx_buffer) > DS_RX_BUFFER_SIZE)
                            tptr = rx_buffer;
                        //                    wxASSERT_MSG((ptmpbuf - temp_buf) < DS_RX_BUFFER_SIZE, "temp_buf overrun");
                            
                            *ptmpbuf = 0;
                            
                            tak_ptr = tptr;
                            
                            // parse and send the message
                            //                    wxString str_temp_buf(temp_buf, wxConvUTF8);
                            if(temp_buf[0] == '\r')
                                Parse_And_Send_Posn(&temp_buf[1]);
                            else
                                Parse_And_Send_Posn(temp_buf);
                    }
                    else
                    {
                        partial = true;
                    }
        }                 // while !partial
        
    }        // nl found
}




#endif            // __WXMSW__

void OCP_DataStreamInput_Thread::Parse_And_Send_Posn(const char *buf)
{
    if( m_pMessageTarget ) {
        OCPN_DataStreamEvent Nevent(wxEVT_OCPN_DATASTREAM, 0);
        Nevent.SetNMEAString( buf );
        Nevent.SetStream( m_launcher );

        m_pMessageTarget->AddPendingEvent(Nevent);
    }

    return;
}

const wxEventType wxEVT_OCPN_THREADMSG = wxNewEventType();

void OCP_DataStreamInput_Thread::ThreadMessage(const wxString &msg)
{
    //    Signal the main program thread
    OCPN_ThreadMessageEvent event(wxEVT_OCPN_THREADMSG, 0);
    event.SetSString( std::string(msg.mb_str()));
    if( gFrame )
        gFrame->GetEventHandler()->AddPendingEvent(event);
}

bool OCP_DataStreamInput_Thread::SetOutMsg(const wxString &msg)
{
    //  Assume that the caller already owns the mutex
    wxCriticalSectionLocker locker( m_outCritical );
    
    if(out_que.size() < OUT_QUEUE_LENGTH){
        wxCharBuffer buf = msg.ToUTF8();
        if(buf.data()){
            char *qmsg = (char *)malloc(strlen(buf.data()) +1);
            strcpy(qmsg, buf.data());
            out_que.push(qmsg);
            return true;
        }
    }
    
    return false;
    
#if 0
    if((m_takIndex==0 && m_putIndex==OUT_QUEUE_LENGTH-1) || (m_takIndex==(m_putIndex+1)))
    {
        return false;
    }
    else
    {
        if(m_takIndex==(-1) && m_putIndex==(-1))
        {
            m_putIndex=0;
            m_takIndex=0;
        }
        else if(m_takIndex!=0 && m_putIndex==OUT_QUEUE_LENGTH-1)
            m_putIndex=0;
        else
            m_putIndex=m_putIndex+1;

        //      Error backstop....
        if(m_putIndex <= OUT_QUEUE_LENGTH)
            strncpy(m_poutQueue[m_putIndex], msg.mb_str(), MAX_OUT_QUEUE_MESSAGE_LENGTH-1);
        else {
            m_takIndex = -1;
            m_putIndex = -1;
        }

        return true;
    }
#endif    
}



#ifdef __POSIX__

int OCP_DataStreamInput_Thread::OpenComPortPhysical(const wxString &com_name, int baud_rate)
{

    // Declare the termios data structures
    termios ttyset_old;
    termios ttyset;

    // Open the serial port.
    int com_fd;
    if ((com_fd = open(com_name.mb_str(), O_RDWR|O_NONBLOCK|O_NOCTTY)) < 0)
        return com_fd;

    speed_t baud_parm;
    switch(baud_rate)
    {
        case     50: baud_parm =     B50; break;
        case     75: baud_parm =     B75; break;
        case    110: baud_parm =    B110; break;
        case    134: baud_parm =    B134; break;
        case    150: baud_parm =    B150; break;
        case    200: baud_parm =    B200; break;
        case    300: baud_parm =    B300; break;
        case    600: baud_parm =    B600; break;
        case   1200: baud_parm =   B1200; break;
        case   1800: baud_parm =   B1800; break;
        case   2400: baud_parm =   B2400; break;
        case   4800: baud_parm =   B4800; break;
        case   9600: baud_parm =   B9600; break;
        case  19200: baud_parm =  B19200; break;
        case  38400: baud_parm =  B38400; break;
        case  57600: baud_parm =  B57600; break;
        case 115200: baud_parm = B115200; break;
        
        default: baud_parm = B4800; break;
    }

    if (isatty(com_fd) != 0)
    {
        /* Save original terminal parameters */
        if (tcgetattr(com_fd,&ttyset_old) != 0)
            return -128;

        memcpy(&ttyset, &ttyset_old, sizeof(termios));

    //  Build the new parms off the old

    //  Baud Rate
        cfsetispeed(&ttyset, baud_parm);
        cfsetospeed(&ttyset, baud_parm);

        tcsetattr(com_fd, TCSANOW, &ttyset);

    // Set blocking/timeout behaviour
        memset(ttyset.c_cc,0,sizeof(ttyset.c_cc));
        ttyset.c_cc[VTIME] = 5;                        // 0.5 sec timeout
        fcntl(com_fd, F_SETFL, fcntl(com_fd, F_GETFL) & !O_NONBLOCK);

    // No Flow Control

        ttyset.c_cflag &= ~(PARENB | PARODD | CRTSCTS);
        ttyset.c_cflag |= CREAD | CLOCAL;
        ttyset.c_iflag = ttyset.c_oflag = ttyset.c_lflag = (tcflag_t) 0;

        int stopbits = 1;
        char parity = 'N';
        ttyset.c_iflag &=~ (PARMRK | INPCK);
        ttyset.c_cflag &=~ (CSIZE | CSTOPB | PARENB | PARODD);
        ttyset.c_cflag |= (stopbits==2 ? CS7|CSTOPB : CS8);
        switch (parity)
        {
            case 'E':
                ttyset.c_iflag |= INPCK;
                ttyset.c_cflag |= PARENB;
                break;
            case 'O':
                ttyset.c_iflag |= INPCK;
                ttyset.c_cflag |= PARENB | PARODD;
                break;
        }
        ttyset.c_cflag &=~ CSIZE;
        ttyset.c_cflag |= (CSIZE & (stopbits==2 ? CS7 : CS8));
        if (tcsetattr(com_fd, TCSANOW, &ttyset) != 0)
            return -129;

        tcflush(com_fd, TCIOFLUSH);
    }

    return com_fd;
}


int OCP_DataStreamInput_Thread::CloseComPortPhysical(int fd)
{
    close(fd);

    return 0;
}


int OCP_DataStreamInput_Thread::WriteComPortPhysical(int port_descriptor, const wxString& string)
{
    ssize_t status;
    status = write(port_descriptor, string.mb_str(), string.Len());

    return status;
}

int OCP_DataStreamInput_Thread::WriteComPortPhysical(int port_descriptor, char *msg)
{
    ssize_t status;
    status = write(port_descriptor, msg, strlen(msg));
    return status;
}

int OCP_DataStreamInput_Thread::ReadComPortPhysical(int port_descriptor, int count, unsigned char *p)
{
//    Blocking, timeout protected read of one character at a time
//    Timeout value is set by c_cc[VTIME]

    return read(port_descriptor, p, count);            // read of (count) characters
}


bool OCP_DataStreamInput_Thread::CheckComPortPhysical(int port_descriptor)
{
    fd_set rec;
    struct timeval t;
//      posix_serial_data *psd = (posix_serial_data *)port_descriptor;
//      int fd = psd->fd;

    int fd = port_descriptor;
    FD_ZERO(&rec);
    FD_SET(fd,&rec);

    t.tv_sec  = 0;
    t.tv_usec = 1000;
    (void) select(fd+1,&rec,NULL,NULL,&t);
    if(FD_ISSET(fd,&rec))
        return true;

    return false;
}


#endif            // __POSIX__

#ifdef __WXMSW__
int OCP_DataStreamInput_Thread::OpenComPortPhysical(const wxString &com_name, int baud_rate)
{

//    Set up the serial port
    wxString xcom_name = com_name;
    xcom_name.Prepend(_T("\\\\.\\"));                  // Required for access to Serial Ports greater than COM9

#ifdef SERIAL_OVERLAPPED
    DWORD open_flags = FILE_FLAG_OVERLAPPED;
#else
    DWORD open_flags = 0;
#endif
    
    HANDLE hSerialComm = CreateFile(xcom_name.fn_str(),      // Port Name
                             GENERIC_READ | GENERIC_WRITE,     // Desired Access
                             0,                               // Shared Mode
                             NULL,                            // Security
                             OPEN_EXISTING,             // Creation Disposition
                             open_flags,
                             NULL);

    if(hSerialComm == INVALID_HANDLE_VALUE)
    {
//        ThreadMessage(_T("Error:Invalid Handle"));
        return (0 - abs((int)::GetLastError()));
    }

    DWORD err;
    COMSTAT cs;
    ClearCommError(hSerialComm, &err, &cs);

    if(!SetupComm(hSerialComm, 1024, 1024))
    {
//        ThreadMessage(_T("Error:SetupComm"));
        if(hSerialComm != INVALID_HANDLE_VALUE)
            CloseHandle( hSerialComm );
        return (0 - abs((int)::GetLastError()));
    }

    DCB dcbConfig;

    if(GetCommState(hSerialComm, &dcbConfig))           // Configuring Serial Port Settings
    {
        dcbConfig.BaudRate = baud_rate;
        dcbConfig.ByteSize = 8;
        dcbConfig.Parity = NOPARITY;
        dcbConfig.StopBits = ONESTOPBIT;
        dcbConfig.fBinary = TRUE;
        dcbConfig.fRtsControl = RTS_CONTROL_ENABLE;
        dcbConfig.fDtrControl = DTR_CONTROL_ENABLE;
        dcbConfig.fOutxDsrFlow = false;
        dcbConfig.fOutxCtsFlow = false;
        dcbConfig.fDsrSensitivity = false;
        dcbConfig.fOutX = false;
        dcbConfig.fInX = false;
        dcbConfig.fInX = false;
        dcbConfig.fInX = false;
    }

    else
    {
//        ThreadMessage(_T("Error:GetCommState"));
        if(hSerialComm != INVALID_HANDLE_VALUE)
            CloseHandle( hSerialComm );
        return (0 - abs((int)::GetLastError()));
    }

    if(!SetCommState(hSerialComm, &dcbConfig))
    {
//        ThreadMessage(_T("Error:SetCommState"));
        if(hSerialComm != INVALID_HANDLE_VALUE)
            CloseHandle( hSerialComm );
        return (0 - abs((int)::GetLastError()));
    }

    COMMTIMEOUTS commTimeout;
    int TimeOutInSec = 2;
    commTimeout.ReadIntervalTimeout = 1000*TimeOutInSec;
    commTimeout.ReadTotalTimeoutConstant = 1000*TimeOutInSec;
    commTimeout.ReadTotalTimeoutMultiplier = 0;
    commTimeout.WriteTotalTimeoutConstant = 1000*TimeOutInSec;
    commTimeout.WriteTotalTimeoutMultiplier = 0;

    if(!SetCommTimeouts(hSerialComm, &commTimeout))
    {
//        ThreadMessage(_T("Error:SetCommTimeouts"));
        if(hSerialComm != INVALID_HANDLE_VALUE)
            CloseHandle( hSerialComm );
        return (0 - abs((int)::GetLastError()));
    }

    return (int)hSerialComm;
}

int OCP_DataStreamInput_Thread::CloseComPortPhysical(int fd)
{
    if((HANDLE)fd != INVALID_HANDLE_VALUE)
        CloseHandle((HANDLE)fd);
    return 0;
}

int OCP_DataStreamInput_Thread::WriteComPortPhysical(int port_descriptor, const wxString& string)
{
    unsigned int dwSize = string.Len();
    char *pszBuf = (char *)malloc((dwSize + 1) * sizeof(char));
    strncpy(pszBuf, string.mb_str(), dwSize+1);

    DWORD dwWritten;
    int fRes;

    // Issue write.
    if (!WriteFile((HANDLE)port_descriptor, pszBuf, dwSize, &dwWritten, NULL))
        fRes = 0;         // WriteFile failed, . Report error and abort.
    else
        fRes = dwWritten;      // WriteFile completed immediately.

    free (pszBuf);


    return fRes;
}

int OCP_DataStreamInput_Thread::WriteComPortPhysical(int port_descriptor, char *msg)
{
    DWORD dwWritten;
    int fRes;
    
    // Issue write.
    if (!WriteFile((HANDLE)port_descriptor, msg, strlen(msg), &dwWritten, NULL))
        fRes = 0;         // WriteFile failed, . Report error and abort.
    else
        fRes = dwWritten;      // WriteFile completed immediately.
            
    return fRes;
}

int OCP_DataStreamInput_Thread::ReadComPortPhysical(int port_descriptor, int count, unsigned char *p)
{
    return 0;
}


bool OCP_DataStreamInput_Thread::CheckComPortPhysical(int port_descriptor)
{
    return false;
}

#endif            // __WXMSW__

