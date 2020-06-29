/*
    Garmin Jeeps Interface Wrapper.

    Copyright (C) 2010 David S Register

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02111 USA

 */

#include <ctype.h>
#include <stdio.h>

#include "../jeeps/gps.h"
#include "garmin_wrapper_utils.h"

gpsdevh* g_gps_devh;

#define LAST_ERROR_SIZE 256

char last_error[256];

const char* GetDeviceLastError(void)
{
      return last_error;
}

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
#

void* xrealloc(void *p, size_t s)

{
	char *o = (char *) realloc(p,s);

	if (!o) {
		fatal("gpsbabel: Unable to realloc %ld bytes of memory.\n", (unsigned long) s);
	}

	return o;
}

void xfree(void* mem)
{
    free(mem);
}

void* xmalloc(size_t size)
{
  void* obj = malloc(size);

  if (!obj) {
    fatal("gpsbabel: Unable to allocate %ld bytes of memory.\n", (unsigned long) size);
  }

  return obj;
}


void* xcalloc(size_t nmemb, size_t size)
{
	void *obj = calloc(nmemb, size);

	if (!obj) {
		fatal("gpsbabel: Unable to allocate %ld units of %ld bytes of memory.\n", (unsigned long) nmemb, (unsigned long) size);
	}

	return obj;
}

int case_ignore_strcmp(const char *s1, const char *s2)
{
	for(;toupper(*s1) == toupper(*s2); ++ s1, ++s2) {
		if (*s1 == 0)
			return 0;
	}
	return (toupper(*s1) < toupper(*s2)) ? -1 : +1;

}

int case_ignore_strncmp(const char *s1, const char *s2, int n)
{
	int rv = 0;

	while (n && ((rv = toupper(*s1) - toupper(*s2)) == 0)
		&& *s1) {
		s1++;
		s2++;
		n--;
	}
	return rv;
}

char* xstrdup(const char *s)
{
	char *o = s ? strdup(s) : strdup("");

	if (!o) {
		fatal("gpsbabel: Unable to allocate %ld bytes of memory.\n", (unsigned long) strlen(s));
	}

	return o;
}

namespace garmin {

void le_write16(void *addr, const unsigned value)
{
      unsigned char *p = (unsigned char *)addr;
      p[0] = value;
      p[1] = value >> 8;

}

void le_write32(void *addr, const unsigned value)
{
      unsigned char *p = (unsigned char *)addr;
      p[0] = value;
      p[1] = value >> 8;
      p[2] = value >> 16;
      p[3] = value >> 24;
}

signed int le_read16(const void *addr)
{
      const unsigned char *p = (const unsigned char *)addr;
      return p[0] | (p[1] << 8);
}

signed int le_read32(const void *addr)
{
      const unsigned char *p = (const unsigned char *)addr;
      return p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
}

}  // namespace garmin


#define DEV_PREFIX "\\\\.\\\\"


static char gb_com_buffer[100];
