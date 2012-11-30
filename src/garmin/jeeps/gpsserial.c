/********************************************************************
** @source JEEPS serial port low level functions
**
** @author Copyright (C) 1999,2000 Alan Bleasby
** @version 1.0
** @modified December 28th 1999 Alan Bleasby. First version
** @modified June 29th 2000 Alan Bleasby. NMEA additions
** @modified Copyright (C) 2006 Robert Lipe
** @@
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** You should have received a copy of the GNU Library General Public
** License along with this library; if not, write to the
** Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
** Boston, MA 02110-1301,  USA.
********************************************************************/
#include "garmin_gps.h"
#include "gpsserial.h"
//dsr#include "../gbser.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>

extern char last_error[];

#if 0
#define GARMULATOR 1
char *rxdata[] = {
	"10 06 02 fe 00 fa 10 03",
	"10 ff 7d 97 00 0e 01 53 74 72 65 65 74 50 69 6c 6f 74 20 33 20 53 6f 66 74 77 61 72 65 20 56 65 72 73 69 6f 6e 20 32 2e 37 30 00 56 45 52 42 4d 41 50 20 41 6d 65 72 69 63 61 73 20 41 75 74 6f 72 6f 75 74 65 20 31 2e 30 30 00 56 45 52 41 55 44 20 45 6e 67 6c 69 73 68 20 33 2e 30 31 00 56 45 52 53 50 4c 53 43 52 4e 20 53 70 6c 61 73 68 20 53 63 72 65 65 6e 20 4d 69 73 73 69 6e 67 00 f1 10 03",
	"10 f8 0e 56 45 52 53 4d 41 50 31 20 4e 6f 6e 65 00 fb 10 03",

	/* Guessing from here down */
	"10 06 02 fe 00 fa 10 03", /* Ack the unknown packet */
	"10 fd 24 50 00 00 4c 01 00 41 0a 00 41 64 00 44 6d 00 41 c9 00 44 ca 00 44 6d 00 44 d2 00 41 2d 01 44 36 01 44 2d 01 66 10 03", /* PTR Array */
	"10 06 02 0a 00 ee 10 03", /* Ack */
	"10 0e 08 06 04 d4 07 00 17 3a 30 84 10 03", /* DATTIME */
	"10 06 02 0a 00 ee 10 03", /* Ack */
	"10 1b 02 09 00 da 10 03", /* RECORD */
	"10 06 02 0a 00 ee 10 03", /* Ack */
	"10 23 5f 01 00 ff 70 3f 20 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 a6 1b aa 19 6e 78 5c c2 00 00 00 00 51 59 04 69 00 00 00 00 00 00 00 00 ff ff ff ff 47 43 31 41 33 37 00 54 68 65 20 54 72 6f 6c 6c 20 62 79 20 61 31 38 32 70 69 6c 6f 74 20 26 20 46 61 6d 69 6c 79 00 00 00 00 00 59 10 03"
	"10 0c 02 07 00 eb 10 03" /* XFERCMP */
};
#endif

gpsdevh *g_gps_devh;

void VerifySerialPortClosed()
{
      if(g_gps_devh)
            GPS_Device_Off(g_gps_devh);
}

int Garmin_Serial_GPS_PVT_On( const char *port_name )
{
    return GPS_A800_On(port_name, &g_gps_devh);
}

int Garmin_Serial_GPS_PVT_Off( const char *port_name )
{
    return GPS_A800_Off(port_name, &g_gps_devh);
}

int GPS_Serial_Command_Pvt_Get(GPS_PPvt_Data *pvt )
{
    return GPS_Command_Pvt_Get(&g_gps_devh, pvt );
}

/*
 * termio on Cygwin is apparently broken, so we revert to Windows serial.
 */
#if defined (__WIN32__) || defined (__CYGWIN__)

#include <windows.h>

typedef struct {
        HANDLE comport;
} win_serial_data;

/*
 * Display an error from the serial subsystem.
 */
void GPS_Serial_Error(const char *mb, ...)
{
      va_list argp;
      va_start(argp, mb);
      sprintf(last_error, mb, argp);
      va_end(argp);

/*
	va_list ap;
	char msg[200];
	char *s;
	int b;

	va_start(ap, mb);
	b = vsnprintf(msg, sizeof(msg), mb, ap);
	s = msg + b;
	*s++ = ':';
	*s++ = ' ';

	FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, 0,
			GetLastError(), 0, s, sizeof(msg) - b - 2, 0 );
	GPS_Error(msg);
*/
}

int32 GPS_Serial_On(const char *port, gpsdevh **dh)
{
	DCB tio;
	COMMTIMEOUTS timeout;
	HANDLE comport;
#ifdef _UNICODE
	wchar_t    wport[80];
	LPCWSTR cstr;
#endif

	const char *xname = fix_win_serial_name(port);
	win_serial_data *wsd = xcalloc(sizeof (win_serial_data), 1);
	*dh = (gpsdevh*) wsd;
      g_gps_devh = (gpsdevh*) wsd;        // save a global copy

	GPS_Diag("Opening %s\n", xname);
#ifdef _UNICODE
	MultiByteToWideChar( 0, 0, xname, -1, wport, 80);
    cstr = wport;
	comport = CreateFile(cstr, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
#else
	comport = CreateFile(xname, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
#endif

	if (comport == INVALID_HANDLE_VALUE) {
		GPS_Serial_Error("CreateFile failed");
		gps_errno = SERIAL_ERROR;
		return 0;
	}
	tio.DCBlength = sizeof(DCB);
	GetCommState (comport, &tio);
	tio.BaudRate = CBR_9600;
	tio.fBinary = TRUE;
	tio.fParity = TRUE;
	tio.fOutxCtsFlow = FALSE;
	tio.fOutxDsrFlow = FALSE;
	tio.fDtrControl = DTR_CONTROL_ENABLE;
	tio.fDsrSensitivity = FALSE;
	tio.fTXContinueOnXoff = TRUE;
	tio.fOutX = FALSE;
	tio.fInX = FALSE;
	tio.fErrorChar = FALSE;
	tio.fNull = FALSE;
	tio.fRtsControl = RTS_CONTROL_ENABLE;
	tio.fAbortOnError = FALSE;
	tio.ByteSize = 8;
	tio.Parity = NOPARITY;
	tio.StopBits = ONESTOPBIT;

	if (!SetCommState (comport, &tio)) {
		GPS_Serial_Error("SetCommState on port '%s' failed", port);
		CloseHandle(comport);
		comport = INVALID_HANDLE_VALUE;
		gps_errno = SERIAL_ERROR;
		return 0;
	}

	/*
	 * The timeouts are kind of fictional as we always end up doing
	 * single byte reads.   At 9600bps (the default) the individual
	 * character time is 104Millisecs, so these are mostly "dead-man"
	 * (i.e. cable unplugged, unit not turned on) values.
	 */
	GetCommTimeouts (comport, &timeout);

	timeout.ReadIntervalTimeout = 1000; /*like vtime.  In MS. */
	timeout.ReadTotalTimeoutMultiplier = 1000;
	timeout.ReadTotalTimeoutConstant = 1000;
	timeout.WriteTotalTimeoutMultiplier = 1000;
	timeout.WriteTotalTimeoutConstant = 1000;
	if (!SetCommTimeouts (comport, &timeout)) {
		GPS_Serial_Error("SetCommTimeouts");
		CloseHandle (comport);
		comport = INVALID_HANDLE_VALUE;
		gps_errno = SERIAL_ERROR;
		return 0;
	}
	wsd->comport = comport;
	return 1;
}

int32 GPS_Serial_Off(gpsdevh *dh)
{
	win_serial_data *wsd = (win_serial_data*)dh;
	CloseHandle(wsd->comport);
	wsd->comport = INVALID_HANDLE_VALUE;
	xfree(wsd);
      g_gps_devh = NULL;        // save a global copy

	return 1;
}

int32 GPS_Serial_Chars_Ready(gpsdevh *dh)
{
	COMSTAT lpStat;
	DWORD lpErrors;
	win_serial_data *wsd = (win_serial_data*)dh;

	ClearCommError(wsd->comport, &lpErrors, &lpStat);
	return (lpStat.cbInQue > 0);
}

int32 GPS_Serial_Wait(gpsdevh *fd)
{
	/* Wait a short time before testing if data is ready.
	 * The GPS II, in particular, has a noticable time responding
	 * with a response to the device inquiry and if we give up on this
	 * too soon, we fail to read the response to the A001 packet and
	 * blow our state machines when it starts streaming the capabiilties
	 * response packet.
	 */
	Sleep(usecDELAY / 1000);
	return GPS_Serial_Chars_Ready(fd);
}

int32 GPS_Serial_Flush(gpsdevh *fd)
{
    unsigned char u;
    int n;
    
    GPS_Diag("Flush:");
    n=0;
    while( (GPS_Serial_Chars_Ready(fd)) && (n < 1000))
    {
        GPS_Serial_Read(fd,&u,1);
        GPS_Diag("%02x ", u);
        
        n++;
    }
    GPS_Diag("\r\n");
    return 1;
}

int32 GPS_Serial_Write(gpsdevh *dh, const void *obuf, int size)
{
	win_serial_data *wsd = (win_serial_data*)dh;
	DWORD len;

	/*
	 * Unbelievably, the Keyspan PDA serial driver 3.2, a "Windows
	 * Certified driver", will crash the OS on a write of zero bytes.
	 * We get such writes from upstream when there are zero payload
	 * bytes.  SO we trap those here to stop Keyspan & Windows from
	 * nuking the system.
	 */
	if (size == 0) {
		return 0;
	}
	WriteFile (wsd->comport, obuf, size, &len, NULL);
	if (len != (DWORD) size) {
		fatal ("Write error.   Wrote %d of %d bytes.\n", (int)len, size);
	}
	return len;
}

int32 GPS_Serial_Read(gpsdevh * dh, void *ibuf, int size)
{
	DWORD cnt  = 0;
	win_serial_data *wsd = (win_serial_data*)dh;

	ReadFile(wsd->comport, ibuf, size, &cnt, NULL);
	return cnt;
}

#else

#include <sys/ioctl.h>
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>

typedef struct {
        int fd;		/* File descriptor */
	struct termios gps_ttysave;
} posix_serial_data;

/* @func GPS_Serial_Open ***********************************************
**
** Open a serial port 8bits 1 stop bit 9600 baud
**
** @param [w] fd [int32 *] file descriptor
** @param [r] port [const char *] port e.g. ttyS1
**
** @return [int32] false upon error
************************************************************************/

int32 GPS_Serial_Open(gpsdevh *dh, const char *port)
{
    struct termios tty;
    posix_serial_data *psd = (posix_serial_data *)dh;

    /*
     * This originally had O_NDELAY | O_NOCTTY in here, but this
     * causes problems with Linux USB ttys (observed on PL2303 and MCT)
     * and the rest of the code doesn't _REALLY_ handle the partial
     * write/retry case anyway.  - robertl
     */
    if((psd->fd = open(port, O_RDWR))==-1)
    {
	GPS_Serial_Error("XSERIAL: Cannot open serial port '%s'", port);
	gps_errno = SERIAL_ERROR;
	return 0;
    }

    if(tcgetattr(psd->fd,&psd->gps_ttysave)==-1)
    {
	gps_errno = HARDWARE_ERROR;
	GPS_Serial_Error("SERIAL: tcgetattr error");
	return 0;
    }
    tty = psd->gps_ttysave;

    tty.c_cflag &= ~(CSIZE);
    tty.c_cflag |= (CREAD | CS8 | CLOCAL);
    cfsetospeed(&tty,B9600);
    cfsetispeed(&tty,B9600);

    tty.c_lflag &= 0x0;
    tty.c_iflag &= 0x0;
    tty.c_oflag &= 0x0;
    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 0;

    if(tcsetattr(psd->fd,TCSANOW|TCSAFLUSH,&tty)==-1)
    {
	GPS_Serial_Error("SERIAL: tcsetattr error");
	return 0;
    }

    return 1;
}

/*
 * Display an error from the serial subsystem.
 */
void GPS_Serial_Error(const char *mb, ...)
{
      va_list argp;
      va_start(argp, mb);

      sprintf(last_error, mb, argp);

      va_end(argp);

//      GPS_Error(mb);
/*dsr
	va_list ap;
	char msg[200];
	char *s;
	int b;

	va_start(ap, mb);
	b = vsnprintf(msg, sizeof(msg), mb, ap);
	s = msg + b;
	*s++ = ':';
	*s++ = ' ';
	*s++ = '\0';

//	FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, 0,
//			GetLastError(), 0, s, sizeof(msg) - b - 2, 0 );
        strcat(msg, strerror(errno));
	GPS_Error(msg);
*/
}

int32 GPS_Serial_Read(gpsdevh *dh, void *ibuf, int size)
{
	posix_serial_data *psd = (posix_serial_data *)dh;
#if GARMULATOR
	static int l;
	static char *rp;
	char **rxp = &rxdata[l];
	char *hex;
	char *rx = *rxp;
	char *ib = ibuf;

	if (!rp) rp = rxdata[0];

	/* Skip over nulls in our pasted strings */
	if (*rp == 0) {
	        rp = rxdata[++l];
	}

	*ib = strtoul(rp, &rp, 16);
	if (*rp) rp++;
	fprintf(stderr, ".");
	return 1;

#else
	return read(psd->fd, ibuf, size);
#endif
}

int32 GPS_Serial_Write(gpsdevh *dh, const void *obuf, int size)
{
	posix_serial_data *psd = (posix_serial_data *)dh;
	return write(psd->fd, obuf, size);
}


/* @func GPS_Serial_Flush ***********************************************
**
** Flush the serial lines
**
** @param [w] fd [int32] file descriptor
**
** @return [int32] false upon error
************************************************************************/
int32 GPS_Serial_Flush(gpsdevh *fd)
{
    posix_serial_data *psd = (posix_serial_data *)fd;

    if(tcflush(psd->fd,TCIOFLUSH))
    {
	GPS_Serial_Error("SERIAL: tcflush error");
	gps_errno = SERIAL_ERROR;
	return 0;
    }

    return 1;
}



/* @func GPS_Serial_Close ***********************************************
**
** Close serial port
**
** @param [r] fd [int32 ] file descriptor
** @param [r] port [const char *] port e.g. ttyS1
**
** @return [int32] false upon error
************************************************************************/

int32 GPS_Serial_Close(gpsdevh *fd)
{
    posix_serial_data *psd = (posix_serial_data *)fd;

    if(tcsetattr(psd->fd, TCSAFLUSH, &psd->gps_ttysave)==-1)
    {
	gps_errno = HARDWARE_ERROR;
	GPS_Serial_Error("SERIAL: tcsetattr error");
	return 0;
    }

    if(close(psd->fd)==-1)
    {
	GPS_Serial_Error("SERIAL: Error closing serial port");
	gps_errno = SERIAL_ERROR;
	return 0;
    }

    return 1;
}


/* @func GPS_Serial_Chars_Ready *****************************************
**
** Query port to see if characters are waiting to be read
**
** @param [r] fd [int32 ] file descriptor
**
** @return [int32] true if chars waiting
************************************************************************/

int32 GPS_Serial_Chars_Ready(gpsdevh *dh)
{
    fd_set rec;
    struct timeval t;
    posix_serial_data *psd = (posix_serial_data *)dh;
    int32 fd = psd->fd;

#if GARMULATOR
    static foo;
    /* Return sporadic reads just to torment the rest of the code. */
    if ((foo++ & 0xf) == 0)
    	return 1;
    else
	return 0;
#endif

    FD_ZERO(&rec);
    FD_SET(fd,&rec);

    t.tv_sec  = 0;
    t.tv_usec = 1000;
    (void) select(fd+1,&rec,NULL,NULL,&t);
    if(FD_ISSET(fd,&rec))
	return 1;

    return 0;
}



/* @func GPS_Serial_Wait ***********************************************
**
** Wait 80 milliseconds before testing for input. The GPS delay
** appears to be around 40-50 milliseconds. Doubling the value is to
** allow some leeway.
**
** @param [r] fd [int32 ] file descriptor
**
** @return [int32] true if serial chars waiting
************************************************************************/

int32 GPS_Serial_Wait(gpsdevh *dh)
{
    fd_set rec;
    struct timeval t;
    posix_serial_data *psd = (posix_serial_data *)dh;

    FD_ZERO(&rec);
    FD_SET(psd->fd,&rec);

    t.tv_sec  = 0;
    t.tv_usec = 180000;	/* Microseconds before GPS sends A001 */

    (void) select(psd->fd+1,&rec,NULL,NULL,&t);
    if(FD_ISSET(psd->fd,&rec))
	return 1;

    return 0;
}



/* @func GPS_Serial_On *****************************************
**
** Set up port
**
** @param [r] port [const char *] port
** @param [w] fd [int32 *] file descriptor
**
** @return [int32] success
************************************************************************/

int32 GPS_Serial_On(const char *port, gpsdevh **dh)
{
    posix_serial_data *psd = xcalloc(sizeof (posix_serial_data), 1);
    *dh = (gpsdevh*) psd;
    g_gps_devh = (gpsdevh*) psd;        // save a global copy

    if(!GPS_Serial_Open((gpsdevh *) psd,port))
    {
	GPS_Error("Cannot open serial port '%s'", port);
	gps_errno = SERIAL_ERROR;
	return 0;
    }

    return 1;
}



/* @func GPS_Serial_Off ***********************************************
**
** Done with port
**
** @param [r] port [const char *] port
** @param [r] fd [int32 ] file descriptor
**
** @return [int32] success
************************************************************************/

int32 GPS_Serial_Off(gpsdevh *dh)
{

    if(!GPS_Serial_Close(dh))
    {
	GPS_Error("Error Closing port");
	gps_errno = HARDWARE_ERROR;
	return 0;
    }
    dh = NULL;

    g_gps_devh = NULL;        // save a global copy

    return 1;
}

#endif /* __WIN32__ */
