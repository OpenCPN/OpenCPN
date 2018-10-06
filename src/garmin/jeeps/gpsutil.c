/********************************************************************
** @source JEEPS utility functions
**
** @author Copyright (C) 1999 Alan Bleasby
** @version 1.0
** @modified Dec 28 1999 Alan Bleasby. First version
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
#include <stdlib.h>
#include <fcntl.h>
#include <stdarg.h>

static int32 gps_endian_called=0;
static int32 GPS_Little=0;

int32 gps_warning = 0;
int32 gps_error   = 0;
int32 gps_user    = 0;
int32 gps_show_bytes = 0;
int32 gps_errno = 0;

char last_error[256];


char * GetDeviceLastError(void)
{
      return last_error;
}


/* @func GPS_Util_Little ***********************************************
**
** Determine endian nature of host
**
** @return [int32] true if little-endian
************************************************************************/

int32 GPS_Util_Little(void)
{
    static union lb
    {
	char chars[sizeof(int32)];
	int32 i;
    }
    data;

    if(!gps_endian_called)
    {
	gps_endian_called = 1;
	data.i = 0;
	*data.chars = '\1';
	if(data.i == 1)
	    GPS_Little = 1;
	else
	    GPS_Little = 0;
    }

    return GPS_Little;
}


/* @func GPS_Util_Get_Short ********************************************
**
** Get a short from a string
**
** @return [US] value
************************************************************************/

US GPS_Util_Get_Short(const UC *s)
{
    static US ret;
    UC *p;

    p = (UC *)&ret;

    if(!GPS_Little)
    {
	*p++ = *(s+1);
	*p = *s;
    }
    else
    {
	*p++ = *s;
	*p = *(s+1);
    }

    return ret;
}



/* @func GPS_Util_Put_Short ********************************************
**
** Put a short to a string
**
** @param [w] s [UC *] string to write to
** @param [r] v [const US] short to write
**
** @return [void]
************************************************************************/

void GPS_Util_Put_Short(UC *s, const US v)
{
    UC *p;

    p = (UC *)&v;

    if(!GPS_Little)
    {
	*s++ = *(p+1);
	*s = *p;
    }
    else
    {
	*s++ = *p;
	*s = *(p+1);
    }

    return;
}



/* @func GPS_Util_Get_Double ********************************************
**
** Get a double from a string
**
** @return [double] value
************************************************************************/

double GPS_Util_Get_Double(const UC *s)
{
    double ret;
    UC *p;
    int32 i;

    p = (UC *)&ret;


    if(!GPS_Little)
	for(i=sizeof(double)-1;i>-1;--i)
	    *p++ = s[i];
    else
	for(i=0;i<(int32)sizeof(double);++i)
	    *p++ = s[i];

    return ret;
}



/* @func GPS_Util_Put_Double ********************************************
**
** Put a double to a string
**
** @param [w] s [UC *] string to write to
** @param [r] v [const double] double to write
**
** @return [void]
************************************************************************/

void GPS_Util_Put_Double(UC *s, const double v)
{
    UC *p;
    int32 i;

    p = (UC *)&v;

    if(!GPS_Little)
	for(i=sizeof(double)-1;i>-1;--i)
	    s[i] = *p++;
    else
	for(i=0;i<(int32)sizeof(double);++i)
	    s[i] = *p++;

    return;
}




/* @func GPS_Util_Get_Int ********************************************
**
** Get an int from a string
**
** @return [int32] value
************************************************************************/

int32 GPS_Util_Get_Int(const UC *s)
{
    int32 ret;
    UC *p;
    int32 i;

    p = (UC *)&ret;


    if(!GPS_Little)
	for(i=sizeof(int32)-1;i>-1;--i)
	    *p++ = s[i];
    else
	for(i=0;i<(int32)sizeof(int32);++i)
	    *p++ = s[i];

    return ret;
}



/* @func GPS_Util_Put_Int ********************************************
**
** Put a int to a string
**
** @param [w] s [UC *] string to write to
** @param [r] v [const int32] int to write
**
** @return [void]
************************************************************************/

void GPS_Util_Put_Int(UC *s, const int32 v)
{
    UC *p;
    int32 i;

    p = (UC *)&v;

    if(!GPS_Little)
	for(i=sizeof(int32)-1;i>-1;--i)
	    s[i] = *p++;
    else
	for(i=0;i<(int32)sizeof(int32);++i)
	    s[i] = *p++;

    return;
}



/* @func GPS_Util_Get_Uint ********************************************
**
** Get an unsigned int from a string
**
** @return [uint32] value
************************************************************************/

uint32 GPS_Util_Get_Uint(const UC *s)
{
    uint32 ret;
    UC     *p;
    int32  i;

    p = (UC *)&ret;


    if(!GPS_Little)
	for(i=sizeof(uint32)-1;i>-1;--i)
	    *p++ = s[i];
    else
	for(i=0;i<(int32)sizeof(uint32);++i)
	    *p++ = s[i];

    return ret;
}



/* @func GPS_Util_Put_Uint ********************************************
**
** Put an unisgned int to a string
**
** @param [w] s [UC *] string to write to
** @param [r] v [const uint32] unsigned int to write
**
** @return [void]
************************************************************************/

void GPS_Util_Put_Uint(UC *s, const uint32 v)
{
    UC    *p;
    int32 i;

    p = (UC *)&v;

    if(!GPS_Little)
	for(i=sizeof(uint32)-1;i>-1;--i)
	    s[i] = *p++;
    else
	for(i=0;i<(int32)sizeof(uint32);++i)
	    s[i] = *p++;

    return;
}



/* @func GPS_Util_Get_Float ********************************************
**
** Get a float from a string
**
** @return [float] value
************************************************************************/

float GPS_Util_Get_Float(const UC *s)
{
    float ret;
    UC *p;
    int32 i;

    p = (UC *)&ret;


    if(!GPS_Little)
	for(i=sizeof(float)-1;i>-1;--i)
	    *p++ = s[i];
    else
	for(i=0;i<(int32)sizeof(float);++i)
	    *p++ = s[i];

    return ret;
}



/* @func GPS_Util_Put_Float ********************************************
**
** Put a float to a string
**
** @param [w] s [UC *] string to write to
** @param [r] v [const float] float to write
**
** @return [void]
************************************************************************/

void GPS_Util_Put_Float(UC *s, const float v)
{
    UC *p;
    int32 i;

    p = (UC *)&v;

    if(!GPS_Little)
	for(i=sizeof(float)-1;i>-1;--i)
	    s[i] = *p++;
    else
	for(i=0;i<(int32)sizeof(float);++i)
	    s[i] = *p++;

    return;
}

#if 0
/* @func GPS_Util_Canon  ****************************************************
**
** Sets or unsets canonical mode
** NB: Must have called this with True before calling with False
** NB: Remember to trun it off (false) eventually
**
** @param [r] state [int32] state=true->raw state=false->normal
** @return [void]
** @@
****************************************************************************/

void GPS_Util_Canon(int32 state)
{
    static struct termios tty;
    static struct termios sv;


    if(state)
    {
	tcgetattr(1,&sv);
	tcgetattr(1, &tty);
	tty.c_cc[VMIN]='\1';
	tty.c_cc[VTIME]='\0';
	tcsetattr(1,TCSANOW,&tty);
	tty.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(1, TCSANOW, &tty);
    }
    else
	tcsetattr(1, TCSANOW, &sv);

    return;
}
#endif

#if 0
/* @func GPS_Util_Block  ****************************************************
**
** Sets or unsets blocking
** @modified 13-01-2000 to return an int
**
** @param [r] fd [int32] file descriptor
** @param [r] state [int32] state=true->block state=false->non-block
**
** @return [int32] success
** @@
****************************************************************************/

int32 GPS_Util_Block(int32 fd, int32 state)
{
    static int32 notcalled=1;
    static int32 block;
    static int32 noblock;
    int32    f;

    gps_errno = HARDWARE_ERROR;

    if(notcalled)
    {
	notcalled = 0;
	if((f=fcntl(fd,F_GETFL,0))==-1)
	{
	    GPS_Error("Util_Block: FCNTL error");
	    return 0;
	}
	block = f & ~O_NDELAY;
	noblock = f |  O_NDELAY;
    }

    if(state)
    {
	if(fcntl(fd,F_SETFL,block)==-1)
	{
	    GPS_Error("Util_Block: Error blocking");
	    return 0;
	}
    }
    else
    {
	if(fcntl(fd,F_SETFL,noblock)==-1)
	{
	    GPS_Error("Util_Block: Error unblocking");
	    return 0;
	}
    }

    return 1;
}
#endif


/* @func GPS_Warning ********************************************************
**
** Prints warning if gps_warning is true
**
** @param [r] s [char *] warning
**
** @return [void]
** @@
****************************************************************************/

void GPS_Warning(char *s)
{
    if(!gps_warning)
	return;

    fprintf(stderr,"[WARNING] %s\n",s);
    fflush(stderr);

    return;
}


/* @func GPS_Fatal ********************************************************
**
** Always prints error and exits program
** Bad thing for a library so the library doesn't call it.
**
** @param [r] s [char *] fatal error
**
** @return [void]
** @@
****************************************************************************/

void GPS_Fatal(char *s)
{

    fprintf(stderr,"[FATAL] %s\n",s);
    exit(0);
    return;
}



/* @func GPS_Error **********************************************************
**
** Prints Error if gps_error is true
**
** @param [r] s [char *] error
**
** @return [void]
** @@
****************************************************************************/

void GPS_Error(char *fmt, ...)
{
    va_list argp;
    va_start(argp, fmt);

    if(!gps_error)
	return;


    fprintf(stderr, "[ERROR] ");
    vfprintf(stderr, fmt, argp);
    fprintf(stderr, "\n");

    sprintf(last_error, fmt, argp);

    va_end(argp);
    return;
}


/* @func GPS_Enable_Error ***************************************************
**
** Enable error message printing
**
** @return [void]
** @@
****************************************************************************/

void GPS_Enable_Error(void)
{
    gps_error = 1;
    return;
}



/* @func GPS_Enable_Warning ***************************************************
**
** Enable warning message printing
**
** @return [void]
** @@
****************************************************************************/

void GPS_Enable_Warning(void)
{
    gps_warning = 1;
    return;
}



/* @func GPS_Disable_Error ***************************************************
**
** Disable error message printing
**
** @return [void]
** @@
****************************************************************************/

void GPS_Disable_Error(void)
{
    gps_error = 0;
    return;
}



/* @func GPS_Disable_Warning ***********************************************
**
** Disable warning message printing
**
** @return [void]
** @@
****************************************************************************/

void GPS_Disable_Warning(void)
{
    gps_warning = 0;
    return;
}



/* @func GPS_User ********************************************************
**
** Prints a message if gps_user is true
**
** @param [r] s [char *] message
**
** @return [void]
** @@
****************************************************************************/

void GPS_User(const char *fmt, ...)
{
    va_list  argp;
    va_start (argp, fmt);

    if (gps_user) {
	vfprintf(stdout, fmt, argp);
	fflush(stdout);
    }

    va_end(argp);
}

/* @func GPS_Disable_User ***********************************************
**
** Disable message printing
**
** @return [void]
** @@
****************************************************************************/

void GPS_Disable_User(void)
{
    gps_user = 0;
    return;
}


/* @func GPS_Enable_User ***********************************************
**
** Disable warning message printing
**
** @return [void]
** @@
****************************************************************************/

void GPS_Enable_User(void)
{
    gps_user = 1;
    return;
}


/* @func GPS_Diagnose ********************************************************
**
** Prints bytes read from gps if gps_show_bytes is set
**
** @param [r] cs [int32] byte read
**
** @return [void]
** @@
****************************************************************************/

void GPS_Diagnose(int32 c)
{
    if(!gps_show_bytes)
	return;

    fprintf(stdout,"%d\n",(int)c);
    fflush(stdout);

    return;
}

void GPS_Diag(const char *fmt, ...)
{
    va_list argp;
    va_start(argp, fmt);

    if(gps_show_bytes) {
         vfprintf(stdout, fmt, argp);
    }
    va_end(argp);
    return;

}

/* @func GPS_Enable_Diagnose ***********************************************
**
** Enable diagnosis mode
**
** @return [void]
** @@
****************************************************************************/

void GPS_Enable_Diagnose(void)
{
    gps_show_bytes = 1;
    return;
}



/* @func GPS_Disble_Diagnose ***********************************************
**
** Disable diagnosis mode
**
** @return [void]
** @@
****************************************************************************/

void GPS_Disable_Diagnose(void)
{
    gps_show_bytes = 0;
    return;
}
