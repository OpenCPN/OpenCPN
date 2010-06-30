//////////////////////////////////////////////////////////////////////
//
// Serial port comm class for the Mac OSX.
//
// adapted from sercomm.cpp
//
//////////////////////////////////////////////////////////////////////
// begin rms
#include "wx/wxprec.h"
#ifdef __WXOSX__

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

#include "wx/print.h"
#include "wx/printdlg.h"
#include "wx/artprov.h"

#include "dychart.h"
#include "macsercomm.h"

CPL_CVSID("$Id: macsercomm.cpp,v 1.1 2008/03/30 23:33:03 bdbcat Exp $");

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSyncSerialComm::CSyncSerialComm(const char *pszPortName)
{

//    rewrite this thing with better license????
            assert(pszPortName);

      m_pszPortName = new char[strlen(pszPortName) +1];
      strcpy(m_pszPortName, pszPortName);
}

CSyncSerialComm::~CSyncSerialComm()
{
      if(m_pszPortName)
            delete[] m_pszPortName;

      Close();
}

//////////////////////////////////////////////////////////////////////
// Name: Open
// Version: 1.0
// Return: true if port was opened
// Comment: This function is used open a connection with a serial port.
// Uses non-overlapped i/o, and allows for reading & writing to the
// port.
//////////////////////////////////////////////////////////////////////
int CSyncSerialComm::Open()
{
    int				handshake;
    struct termios	options;
    m_fileDescriptor = -1;
    
    // Open the serial port read/write, with no controlling terminal, and don't wait for a connection.
    // The O_NONBLOCK flag also causes subsequent I/O on the device to be non-blocking.
    // See open(2) ("man 2 open") for details.
    
    m_fileDescriptor = open(m_pszPortName, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (m_fileDescriptor == -1)
    {
        wxLogMessage("Error opening serial port %s - %s(%d).\n",
               m_pszPortName, strerror(errno), errno);
        return false ;
    }

    // Note that open() follows POSIX semantics: multiple open() calls to the same file will succeed
    // unless the TIOCEXCL ioctl is issued. This will prevent additional opens except by root-owned
    // processes.
    // See tty(4) ("man 4 tty") and ioctl(2) ("man 2 ioctl") for details.
    
    if (ioctl(m_fileDescriptor, TIOCEXCL) == -1)
    {
        wxLogMessage("Error setting TIOCEXCL on %s - %s(%d).\n",
            m_pszPortName, strerror(errno), errno);
		if (m_fileDescriptor != -1)
		{
			close(m_fileDescriptor);
			m_fileDescriptor = -1 ;
		}
		return false ;
    }
    
    // Now that the device is open, clear the O_NONBLOCK flag so subsequent I/O will block.
    // See fcntl(2) ("man 2 fcntl") for details.
    
    if (fcntl(m_fileDescriptor, F_SETFL, 0) == -1)
    {
        wxLogMessage("Error clearing O_NONBLOCK %s - %s(%d).\n",
            m_pszPortName, strerror(errno), errno);
		if (m_fileDescriptor != -1)
		{
			close(m_fileDescriptor);
			m_fileDescriptor = -1 ;
		}
        return false ;
    }
    
    // Get the current options and save them so we can restore the default settings later.
    if (tcgetattr(m_fileDescriptor, &m_OriginalTTYAttrs) == -1)
    {
        wxLogMessage("Error getting tty attributes %s - %s(%d).\n",
            m_pszPortName, strerror(errno), errno);
		if (m_fileDescriptor != -1)
		{
			close(m_fileDescriptor);
			m_fileDescriptor = -1 ;
		}
        return false ;
    }
	return true ;
}

//////////////////////////////////////////////////////////////////////
// Name: Close
// Version: 1.0
// Return: HRESULT
// Comment: This function is used to close the serial port connection
// Note: This function is called with the destructor
//////////////////////////////////////////////////////////////////////

int CSyncSerialComm::Close()
{
	if (m_fileDescriptor != -1)
    {
        close(m_fileDescriptor);
    // Block until all written output has been sent from the device.
    // Note that this call is simply passed on to the serial device driver. 
	// See tcsendbreak(3) ("man 3 tcsendbreak") for details.
    if (tcdrain(m_fileDescriptor) == -1)
    {
        wxLogMessage("Error waiting for drain - %s(%d).\n",
            strerror(errno), errno);
    }
    
    // Traditionally it is good practice to reset a serial port back to
    // the state in which you found it. This is why the original termios struct
    // was saved.
    if (tcsetattr(m_fileDescriptor, TCSANOW, &m_OriginalTTYAttrs) == -1)
    {
        wxLogMessage("Error resetting tty attributes - %s(%d).\n",
            strerror(errno), errno);
    }
    close(m_fileDescriptor);
	m_fileDescriptor = -1 ;
    }	
}

//////////////////////////////////////////////////////////////////////
// Name: ConfigPort
// Version: 1.0
// Parameter: dwBaudRate - This must be set to the baud rate of the
// serial port connection otherwise invalid reads occur.
// dwTimeOutInSec - Specifies the timeout for read and write of the serial
// port connection in seconds
// Return: HRESULT
// Comment: This function is used configure the serial port connection.
//////////////////////////////////////////////////////////////////////

int CSyncSerialComm::ConfigPort(int dwBaudRate, int dwTimeOutInSec)
{
    struct termios	options;
    int				handshake;
			
    // The serial port attributes such as timeouts and baud rate are set by modifying the termios
    // structure and then calling tcsetattr() to cause the changes to take effect. Note that the
    // changes will not become effective without the tcsetattr() call.
    // See tcsetattr(4) ("man 4 tcsetattr") for details.
    
    options = m_OriginalTTYAttrs;
    
    // Print the current input and output baud rates.
    // See tcsetattr(4) ("man 4 tcsetattr") for details.
    
    wxLogMessage("Current input baud rate is %d\n", (int) cfgetispeed(&options));
    wxLogMessage("Current output baud rate is %d\n", (int) cfgetospeed(&options));
    
    // Set raw input (non-canonical) mode, with reads blocking until either a single character 
    // has been received or a one second timeout expires.
    // See tcsetattr(4) ("man 4 tcsetattr") and termios(4) ("man 4 termios") for details.
    
    cfmakeraw(&options);
    options.c_cc[VMIN] = 1;
    options.c_cc[VTIME] = 10;
        
    // The baud rate, word length, and handshake options can be set as follows:
    
    cfsetspeed(&options, dwBaudRate);		// Set 19200 baud    
    options.c_cflag |= (CS8 	   | 	// Use 7 bit words
						CCTS_OFLOW | 	// CTS flow control of output
						CRTS_IFLOW);	// RTS flow control of input
			

    // Print the new input and output baud rates. Note that the IOSSIOSPEED ioctl interacts with the serial driver 
	// directly bypassing the termios struct. This means that the following two calls will not be able to read
	// the current baud rate if the IOSSIOSPEED ioctl was used but will instead return the speed set by the last call
	// to cfsetspeed.
    
    wxLogMessage("Input baud rate changed to %d\n", (int) cfgetispeed(&options));
    wxLogMessage("Output baud rate changed to %d\n", (int) cfgetospeed(&options));
    
    // Cause the new options to take effect immediately.
    if (tcsetattr(m_fileDescriptor, TCSANOW, &options) == -1)
    {
        wxLogMessage("Error setting tty attributes %s - %s(%d).\n",
            m_pszPortName, strerror(errno), errno);
        goto error;
    }

    // To set the modem handshake lines, use the following ioctls.
    // See tty(4) ("man 4 tty") and ioctl(2) ("man 2 ioctl") for details.
    
    if (ioctl(m_fileDescriptor, TIOCSDTR) == -1) // Assert Data Terminal Ready (DTR)
    {
        wxLogMessage("Error asserting DTR %s - %s(%d).\n",
            m_pszPortName, strerror(errno), errno);
    }
    
    if (ioctl(m_fileDescriptor, TIOCCDTR) == -1) // Clear Data Terminal Ready (DTR)
    {
        wxLogMessage("Error clearing DTR %s - %s(%d).\n",
            m_pszPortName, strerror(errno), errno);
    }
    
    handshake = TIOCM_DTR | TIOCM_RTS | TIOCM_CTS | TIOCM_DSR;
    if (ioctl(m_fileDescriptor, TIOCMSET, &handshake) == -1)
    // Set the modem lines depending on the bits set in handshake
    {
        wxLogMessage("Error setting handshake lines %s - %s(%d).\n",
            m_pszPortName, strerror(errno), errno);
    }
    
    // To read the state of the modem lines, use the following ioctl.
    // See tty(4) ("man 4 tty") and ioctl(2) ("man 2 ioctl") for details.
    
    if (ioctl(m_fileDescriptor, TIOCMGET, &handshake) == -1)
    // Store the state of the modem lines in handshake
    {
        wxLogMessage("Error getting handshake lines %s - %s(%d).\n",
            m_pszPortName, strerror(errno), errno);
    }
    
    wxLogMessage("Handshake lines currently set to %d\n", handshake);
    
    // Success
    return m_fileDescriptor;
    
    // Failure path
error:
    if (m_fileDescriptor != -1)
    {
        close(m_fileDescriptor);
    }
    
    return -1;
			
}

//////////////////////////////////////////////////////////////////////
// Name: Write
// Version: 1.0
// Parameter: szBuf - The buffer holding the bytes to write to the serial
// port connection
// dwSize - The size of the buffer
// Return: HRESULT
// Comment: This function writes one byte at a time until all the bytes
// in the buffer is sent out
//////////////////////////////////////////////////////////////////////

int CSyncSerialComm::Write(const char *pszBuf, int dwSize)
{
	int numBytes = 0 ;
	int iTries = 10 ;
	while ((numBytes < dwSize) && (iTries > 0 ))
	{
		iTries--;
        // Send an AT command to the modem
        numBytes = write(m_fileDescriptor, pszBuf, dwSize-numBytes);
        
		if (numBytes == -1)
		{
			wxLogMessage("Error writing to modem - %s(%d).\n", strerror(errno), errno);
		}
		else
		{
			wxLogMessage("Wrote %ld bytes\n", numBytes);
		}
	}
	return bool(numBytes == dwSize) ;
}

//////////////////////////////////////////////////////////////////////
// Name: Read
// Version: 1.0
// Parameter: ppszBuf - The buffer that will have the value that was
// read in from the serial port.
// dwSize - The size of the buffer
// Return: HRESULT
// Comment: This function sets an event that will be signalled if the
// any byte is buffered internally. Once this occurs, the function keeps
// reading multiple a single byte at a time until there is no more furthur
// byte to read from the input stream
//////////////////////////////////////////////////////////////////////

int CSyncSerialComm::Read(char **ppszBuf, int &dwSize)
{
      return false ;

}

#endif            //__WXOSX__
// end rms
