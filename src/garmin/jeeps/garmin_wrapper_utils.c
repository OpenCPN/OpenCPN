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

#include "garmin_wrapper_utils.h"
#include <ctype.h>
#include <stdio.h>


/*    Some function stubs sprinkled throughout jeeps  */
void fatal(const char *p, ...)
{}



void *
xrealloc(void *p, size_t s)

{
	char *o = (char *) realloc(p,s);

	if (!o) {
		fatal("gpsbabel: Unable to realloc %ld bytes of memory.\n", (unsigned long) s);
	}

	return o;
}

void
xfree( void *mem )
{
    free(mem);
}

void *
xcalloc(size_t nmemb, size_t size)
{
	void *obj = calloc(nmemb, size);

	if (!obj) {
		fatal("gpsbabel: Unable to allocate %ld units of %ld bytes of memory.\n", (unsigned long) nmemb, (unsigned long) size);
	}

	return obj;
}

int
case_ignore_strcmp(const char *s1, const char *s2)
{
	for(;toupper(*s1) == toupper(*s2); ++ s1, ++s2) {
		if (*s1 == 0)
			return 0;
	}
	return (toupper(*s1) < toupper(*s2)) ? -1 : +1;

}

int
case_ignore_strncmp(const char *s1, const char *s2, int n)
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

char *
xstrdup(const char *s)

{
	char *o = s ? strdup(s) : strdup("");

	if (!o) {
		fatal("gpsbabel: Unable to allocate %ld bytes of memory.\n", (unsigned long) strlen(s));
	}

	return o;
}


void
            le_write16(void *addr, const unsigned value)
{
      unsigned char *p = (unsigned char *)addr;
      p[0] = value;
      p[1] = value >> 8;

}

void
            le_write32(void *addr, const unsigned value)
{
      unsigned char *p = (unsigned char *)addr;
      p[0] = value;
      p[1] = value >> 8;
      p[2] = value >> 16;
      p[3] = value >> 24;
}

signed int
            le_read16(const void *addr)
{
      const unsigned char *p = (const unsigned char *)addr;
      return p[0] | (p[1] << 8);
}

signed int
            le_read32(const void *addr)
{
      const unsigned char *p = (const unsigned char *)addr;
      return p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
}


#define DEV_PREFIX "\\\\.\\\\"

/* Return true if a port name seems to refer to a serial port.
 * On Windows this tests the filename (against the regex
 * /^(\\\\\.\\\\)?com\d+:?$/i). On Posix it returns the value of
 * isatty()
 */

int gbser_is_serial(const char *port_name) {
      const char *pfx = DEV_PREFIX;
      size_t pfx_l = strlen(pfx);
      const char *com = "COM";
      size_t com_l = strlen(com);
      unsigned digits;

      if (NULL == port_name) {
            return 0;
      }

      /* Skip any prefix */
      if (memcmp(port_name, pfx, pfx_l) == 0) {
            port_name += pfx_l;
      }

      if (case_ignore_strncmp(port_name, com, com_l) != 0) {
            return 0;
      }

      port_name += com_l;
      for (digits = 0; isdigit(*port_name); port_name++, digits++) {
            /* do nothing */
      }

      if (digits == 0) {
            return 0;
      }

      if (*port_name == ':') {
            port_name++;
      }

      if (*port_name != '\0') {
            return 0;
      }

      /* Success! */
      return 1;
}


const char *
fix_win_serial_name_r(const char *comname, char *obuf, size_t len)
{
      if (!gbser_is_serial(comname) ||
           ((strlen(comname) == 5) && (comname[4] == ':')) ||
           ((strlen(comname) == 4) && (case_ignore_strncmp(comname, "com", 3) == 0))
         ) {
            strncpy(obuf, comname, len);
         } else {
               size_t l;
               snprintf(obuf, len, DEV_PREFIX "%s", comname);
               l = strlen(obuf);
               if (obuf[l - 1] == ':') {
                     obuf[l - 1] = '\0';
               }
         }

         return obuf;
}

static char gb_com_buffer[100];

const char *fix_win_serial_name(const char *comname)
{
      return fix_win_serial_name_r(comname, gb_com_buffer, sizeof(gb_com_buffer));
}

