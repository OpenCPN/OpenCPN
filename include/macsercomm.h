//////////////////////////////////////////////////////////////////////
//
// Serial port comm class for the Mac OSX.
//
// adapted from sercomm.cpp
//
//////////////////////////////////////////////////////////////////////

// begin rms
#ifdef __WXOSX__

#if !defined(AFX_SYNCSERIALCOMM_H__D1CAB621_DF4B_4729_82AB_31D5B9EFE8A9__INCLUDED_)
#define AFX_SYNCSERIALCOMM_H__D1CAB621_DF4B_4729_82AB_31D5B9EFE8A9__INCLUDED_


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <paths.h>
#include <termios.h>
#include <sysexits.h>
#include <sys/param.h>
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <AvailabilityMacros.h>

#include <CoreFoundation/CoreFoundation.h>

#include <IOKit/IOKitLib.h>
#include <IOKit/serial/IOSerialKeys.h>
#if defined(MAC_OS_X_VERSION_10_3) && (MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_3)
#include <IOKit/serial/ioss.h>
#endif
#include <IOKit/IOBSD.h>

#include "macutils.h"

//////////////////////////////////////////////////////////////////////
// Name: CSyncSerialComm
// Version: 1.0
//////////////////////////////////////////////////////////////////////

class CSyncSerialComm
{
public:
      int Flush(int dwFlag);
      int Write(const char *pszBuf, int dwSize);
      int Read(char **ppszBuf, int &dwSize);
      int ConfigPort(int dwBaudRate = 9600, int dwTimeOutInSec = 5);
      int Close();
      int Open();

      CSyncSerialComm(const char *pszPortName);
      virtual ~CSyncSerialComm();

private:
	char* m_pszPortName ;
	// file despcriptor for the port
	int m_fileDescriptor ;
	// Hold the original termios attributes so we can reset them
	struct termios m_OriginalTTYAttrs ;
};

#endif // !defined(AFX_SYNCSERIALCOMM_H__D1CAB621_DF4B_4729_82AB_31D5B9EFE8A9__INCLUDED_)

#endif            //__WXOSX__
// end rms
