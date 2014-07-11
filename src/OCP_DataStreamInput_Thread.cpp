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

#include "chart1.h"
extern MyFrame *gFrame;

#ifdef __WXMSW__
extern int g_total_NMEAerror_messages;
extern int g_nNMEADebug;
#endif

typedef enum DS_ENUM_BUFFER_STATE
{
      DS_RX_BUFFER_EMPTY,
      DS_RX_BUFFER_FULL
}_DS_ENUM_BUFFER_STATE;


OCP_DataStreamInput_Thread::OCP_DataStreamInput_Thread(DataStream *Launcher,
                                                       wxEvtHandler *MessageTarget,
                                                       const wxString& PortName,
                                                       const wxString& strBaudRate,
                                                       wxMutex *pout_mutex,
                                                       dsPortType io_select
                                                      )
{
    m_launcher = Launcher;                          // This thread's immediate "parent"

    m_pMessageTarget = MessageTarget;

    m_PortName = PortName;
    m_FullPortName = _T("Serial:") + PortName;

    m_pout_mutex = pout_mutex;
    m_io_select = io_select;

    rx_buffer = new char[DS_RX_BUFFER_SIZE + 1];
    temp_buf = new char[DS_RX_BUFFER_SIZE + 1];

    put_ptr = rx_buffer;                            // local circular queue
    tak_ptr = rx_buffer;

    m_baud = 4800;                                  // default
    long lbaud;
    if(strBaudRate.ToLong(&lbaud))
        m_baud = (int)lbaud;

    // Allocate the static output queue
    for(int i=0 ; i  < OUT_QUEUE_LENGTH ; i++){
        m_poutQueue[i] = (char *)malloc(MAX_OUT_QUEUE_MESSAGE_LENGTH);
    }
    m_takIndex = -1;
    m_putIndex = -1;

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
                    wxString str_temp_buf(temp_buf, wxConvUTF8);
                    Parse_And_Send_Posn(temp_buf);
                }

            }                   //if nl
        }                       // if newdata > 0

        //      Check for any pending output message

        if( m_pout_mutex && (wxMUTEX_NO_ERROR == m_pout_mutex->TryLock()) ){
            bool b_qdata = (m_takIndex != (-1) || m_putIndex != (-1));
            
            while(b_qdata){
                if(m_takIndex < OUT_QUEUE_LENGTH) {
                    
                    //  Take a copy of message
                    char msg[MAX_OUT_QUEUE_MESSAGE_LENGTH];
                    strncpy( msg, m_poutQueue[m_takIndex], MAX_OUT_QUEUE_MESSAGE_LENGTH-1 );
                    
                    //  Update and release the taker index
                    if(m_takIndex==m_putIndex)
                        m_takIndex=m_putIndex=(-1);
                    else if(m_takIndex == (OUT_QUEUE_LENGTH-1) )
                        m_takIndex=0;
                    else
                        m_takIndex++;
                    
                    
                    m_pout_mutex->Unlock();
                    WriteComPortPhysical(m_gps_fd, msg);
                    
                    if( wxMUTEX_NO_ERROR == m_pout_mutex->TryLock() )
                        b_qdata = (m_takIndex != (-1) || m_putIndex != (-1));
                    else
                        b_qdata = false;
                }
                else {                                  // some index error
                    m_takIndex = (-1);
                    m_putIndex = (-1);
                    b_qdata = false;
                }
                
                
            } //while b_qdata
            m_pout_mutex->Unlock();
        }
        
 bail_output:
    bool bail = true;
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

//    Entry Point
void *OCP_DataStreamInput_Thread::Entry()
{
    wxString msg;
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
        wxString msg(_T("NMEA input device open failed: "));
        msg.Append(m_PortName);
        wxString msg_error;
        msg_error.Printf(_T("...GetLastError():  %d"), GetLastError());
        msg.Append(msg_error);

        ThreadMessage(msg);
        goto thread_exit;
    }

    hSerialComm = (HANDLE)m_gps_fd;


    //  If port supports output, set a short timeout so that output polling mechanism works
    int max_timeout = 5;
    int loop_timeout = 2000;
    int n_reopen_wait = 2000;
    bool nl_found = false;

    if( (m_io_select == DS_TYPE_INPUT_OUTPUT) || (m_io_select == DS_TYPE_OUTPUT) ) {
        loop_timeout = 2;
        max_timeout = 5000;
    }

    COMMTIMEOUTS timeouts;

    timeouts.ReadIntervalTimeout = MAXDWORD;
    timeouts.ReadTotalTimeoutMultiplier = MAXDWORD;
    timeouts.ReadTotalTimeoutConstant = loop_timeout;
    timeouts.WriteTotalTimeoutMultiplier = MAXDWORD;
    timeouts.WriteTotalTimeoutConstant = MAXDWORD;

    if (!SetCommTimeouts((HANDLE)m_gps_fd, &timeouts)) // Error setting time-outs.
        goto thread_exit;

    not_done = true;

#define READ_BUF_SIZE 200
    char szBuf[READ_BUF_SIZE];

    DWORD dwRead;
    DWORD dwOneRead;
    char  chRead;
    int ic;
    int n_timeout;

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

            if(n_reopen_wait)
            {
                wxThread::Sleep(n_reopen_wait);                        // stall for a bit
                n_reopen_wait = 0;
            }


            if ((m_gps_fd = OpenComPortPhysical(m_PortName, m_baud)) > 0)
            {
                hSerialComm = (HANDLE)m_gps_fd;

                if(!SetCommMask((HANDLE)m_gps_fd, EV_RXCHAR)) // Setting Event Type
                    goto thread_exit;

                COMMTIMEOUTS timeouts;
                timeouts.ReadIntervalTimeout = MAXDWORD;
                timeouts.ReadTotalTimeoutMultiplier = MAXDWORD;
                timeouts.ReadTotalTimeoutConstant = loop_timeout;
                timeouts.WriteTotalTimeoutMultiplier = MAXDWORD;
                timeouts.WriteTotalTimeoutConstant = MAXDWORD;

                if (!SetCommTimeouts((HANDLE)m_gps_fd, &timeouts)) // Error setting time-outs.
                    goto thread_exit;
            }
            else
            {
                m_gps_fd = 0;
                wxThread::Sleep(2000);                        // stall for a bit
            }
        }

        bool b_inner = true;
        bool b_sleep = false;
        dwRead = 0;
        n_timeout = 0;
        wxDateTime now = wxDateTime::Now();
        int t = now.GetTicks();                 // set a separate timer not controlled by serial port
        ic=0;
        while( b_inner ) {
            if( m_pout_mutex && (wxMUTEX_NO_ERROR == m_pout_mutex->TryLock()) ){
                bool b_qdata = (m_takIndex != (-1) || m_putIndex != (-1));
                
                while(b_qdata){
                    if(m_takIndex < OUT_QUEUE_LENGTH) {
                        
                        //  Take a copy of message
                        char msg[MAX_OUT_QUEUE_MESSAGE_LENGTH];
                        strncpy( msg, m_poutQueue[m_takIndex], MAX_OUT_QUEUE_MESSAGE_LENGTH-1 );
                        
                        //  Update and release the taker index
                        if(m_takIndex==m_putIndex)
                            m_takIndex=m_putIndex=(-1);
                        else if(m_takIndex == (OUT_QUEUE_LENGTH-1) )
                            m_takIndex=0;
                        else
                            m_takIndex++;
                        
                        
                        m_pout_mutex->Unlock();
                        WriteComPortPhysical(m_gps_fd, msg);
                        
                        if( wxMUTEX_NO_ERROR == m_pout_mutex->TryLock() )
                            b_qdata = (m_takIndex != (-1) || m_putIndex != (-1));
                        else
                            b_qdata = false;
                    }
                    else {                                  // some index error
                    m_takIndex = (-1);
                    m_putIndex = (-1);
                    b_qdata = false;
                    }
                    
                    
                } //while b_qdata
                m_pout_mutex->Unlock();
            }           // Mutex lock
            

            if( b_sleep )                       // we need a sleep if the serial port does not honor commtimeouts
                wxSleep(1);
            if(ReadFile((HANDLE)m_gps_fd, &chRead, 1, &dwOneRead, NULL))
            {
                if(1 == dwOneRead) {
                    b_sleep = false;
                    szBuf[ic] = chRead;
                    dwRead++;
                    if(++ic > READ_BUF_SIZE - 1)
                        goto HandleASuccessfulRead;
                    if(chRead == 0x0a)
                        goto HandleASuccessfulRead;
                }
                else {                          // timed out
                    n_timeout++;;
                    if( n_timeout > max_timeout ) {
                        
                        //  If nothing has been input from the port for a long time (10 sec), it may be broken.
                        //  So we probably want to reset the port.
                        
                        //  There are 2 cases to consider on timing
                        //  1.  Read-only port
                        //      COMMTIMEOUT(loop_timeout) is 2 seconds, max_timeout=5
                        //   2.  Read/Write port
                        //      COMMTIMEOUT(loop_timeout) is 2 msec, max_timeout=5000
                        
                        //  Some virtual port emulators (XPort, especially) do not honor COMMTIMEOUTS
                        //  So, we must check the real time (in seconds) of the inactivity time.
                        //  If it is much shorter than 10 secs, assume that COMMTIMEOUTS are not working
                        //  In this case, abort the port reset, reset the counters,
                        //  and signal a need for a Sleep() between read commands to avoid CPU saturation.
                        
                        bool b_need_reset = true;
                        
                        wxDateTime then = wxDateTime::Now();
                        int dt = then.GetTicks() - t;
                        if( (dt) <  ((max_timeout * loop_timeout)/1000)/2 ) {
                            b_need_reset = false;
                            n_timeout = 0;
                            b_sleep = true;
                        }
                        
                        if( b_need_reset ) {
                            b_inner = false;
                            CloseComPortPhysical(m_gps_fd);
                            m_gps_fd = 0;
                            dwRead = 0;
                            nl_found = false;
                            n_reopen_wait = 2000;
                        }
                    }
                    else if((TestDestroy()) || (m_launcher->m_Thread_run_flag == 0)) {
                        goto thread_exit;                               // smooth exit
                    }
                }
            }
            else {                     //      ReadFile Erorr
                b_inner = false;
                CloseComPortPhysical(m_gps_fd);
                m_gps_fd = 0;
                dwRead = 0;
                nl_found = false;
                n_reopen_wait = 2000;
            }
        }

HandleASuccessfulRead:

        if(dwRead > 0)
        {
              if((g_total_NMEAerror_messages < g_nNMEADebug) && (g_nNMEADebug > 1000))
              {
                    g_total_NMEAerror_messages++;
                    wxString msg;
                    msg.Printf(_T("NMEA activity...%d bytes"), dwRead);
                    ThreadMessage(msg);
              }

              int nchar = dwRead;
              char *pb = szBuf;

              while(nchar)
              {
                    if(0x0a == *pb)
                          nl_found = true;

                    *put_ptr++ = *pb++;
                    if((put_ptr - rx_buffer) > DS_RX_BUFFER_SIZE)
                          put_ptr = rx_buffer;

                    nchar--;
              }
              if((g_total_NMEAerror_messages < g_nNMEADebug) && (g_nNMEADebug > 1000))
              {
                    g_total_NMEAerror_messages++;
                    wxString msg1 = _T("Buffer is: ");
                    int nc = dwRead;
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
        if(nl_found)
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
                    wxString str_temp_buf(temp_buf, wxConvUTF8);
                    Parse_And_Send_Posn(temp_buf);
                }
                else
                {
                    partial = true;
                }
            }                 // while !partial

        }        // nl found
        
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


void OCP_DataStreamInput_Thread::ThreadMessage(const wxString &msg)
{
    //    Signal the main program thread
    wxCommandEvent event( EVT_THREADMSG,  GetId());
    event.SetEventObject( (wxObject *)this );
    event.SetString(msg);
    if( gFrame )
        gFrame->GetEventHandler()->AddPendingEvent(event);
}

bool OCP_DataStreamInput_Thread::SetOutMsg(const wxString & msg)
{
    //  Assume that the caller already owns the mutex

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

    DWORD open_flags = 0;

    HANDLE hSerialComm = CreateFile(xcom_name.fn_str(),      // Port Name
                             GENERIC_READ | GENERIC_WRITE,     // Desired Access
                             0,                               // Shared Mode
                             NULL,                            // Security
                             OPEN_EXISTING,             // Creation Disposition
                             open_flags,
                             NULL);

    if(hSerialComm == INVALID_HANDLE_VALUE)
    {
        ThreadMessage(_T("Error:Invalid Handle"));
        return (0 - abs((int)::GetLastError()));
    }

    DWORD err;
    COMSTAT cs;
    ClearCommError(hSerialComm, &err, &cs);

    if(!SetupComm(hSerialComm, 1024, 1024))
    {
        ThreadMessage(_T("Error:SetupComm"));
//        return (0 - abs((int)::GetLastError()));
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
        ThreadMessage(_T("Error:GetCommState"));
//        return (0 - abs((int)::GetLastError()));
    }

    if(!SetCommState(hSerialComm, &dcbConfig))
    {
        ThreadMessage(_T("Error:SetCommState"));
//        return (0 - abs((int)::GetLastError()));
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
        ThreadMessage(_T("Error:SetCommTimeouts"));
//        return (0 - abs((int)::GetLastError()));
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

