/*
    Serial interface

    Copyright (C) 2006  Robert Lipe, robertlipe+source@gpsbabel.org

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

 */

#include "defs.h"
#include "gbser.h"
#include "gbser_private.h"

#include <cassert>
#include <cstdarg>
#include <cstdio>

void gbser__db(int l, const char* msg, ...)
{
  va_list ap;
  va_start(ap, msg);
  if (global_opts.debug_level >= l) {
    vprintf(msg, ap);
  }
  va_end(ap);
}

/* Set the serial port speed.
 */
int gbser_set_speed(void* handle, unsigned speed)
{
  return gbser_set_port(handle, speed, 8, 0, 1);
}

static int parity_letter(char c)
{
  switch (c) {
  case 'N':
  case 'n':
    return 0;
  case 'O':
  case 'o':
    return 1;
  case 'E':
  case 'e':
    return 2;
  default:
    return -1;
  }
}

/* Set the serial port up by parsing the supplied parameter string.
 * Valid parameter strings look like '4800,8,N,1'. Parsing is case-
 * insensitive, spaces are allowed around the commas and omitted
 * trailing fields will default to '8', 'N' and '1'
 */
int gbser_setup(void* handle, const char* spec)
{
  unsigned arg[] = { 4800, 8, 0, 1 };

  for (unsigned int ap = 0; ap < sizeof(arg) / sizeof(arg[0]); ap++) {
    unsigned t = 0;
    int pl;
    while (isspace(*spec)) {
      spec++;
    }
    /* Allow 'N', 'O' or 'E' as the parity spec */
    if (ap == 2 && (pl = parity_letter(*spec), pl >= 0)) {
      t = pl;
      spec++;
    } else {
      if (!isdigit(*spec)) {
        break;
      }
      while (isdigit(*spec)) {
        t = t * 10 + *spec++ - '0';
      }
    }
    arg[ap] = t;
    while (isspace(*spec)) {
      spec++;
    }
    if (*spec != ',') {
      break;
    }
    spec++;
  }

  if (*spec != '\0') {
    return gbser_ERROR;
  }

  return gbser_set_port(handle, arg[0], arg[1], arg[2], arg[3]);
}

/* Return true if there are characters available on the serial port
 */
int gbser_avail(void* handle)
{
  return gbser__fill_buffer(handle, 1, nullptr);
}

/* Read as many bytes as are available without blocking. At most |len|
 * bytes will be read. Returns the number of bytes read or gbser_ERROR if an
 * error occurs.
 */
int gbser_read(void* handle, void* buf, unsigned len)
{
  int got = 0;

  while (len > 0) {
    int rc = gbser__fill_buffer(handle, len, nullptr);
    if (rc < 0) {
      /* error */
      return rc;
    } else if (rc == 0) {
      /* nothing available */
      break;
    }
    got += gbser__read_buffer(handle, &buf, &len);
  }

  return got;
}

/* Read the specified number of bytes. Block until the requested number
 * of bytes have been read or the timeout (in ms) is exceeded.
 */
int gbser_read_wait(void* handle, void* buf, unsigned len, unsigned ms)
{
  int got = 0;

  while (len > 0 && ms != 0) {
    int rc;
    if (rc = gbser__fill_buffer(handle, len, &ms), rc < 0) {
      return rc;
    }
    got += gbser__read_buffer(handle, &buf, &len);
  }

  return got;
}

/* Read a single character from the port, returning immediately if
 * none are available.
 */
int gbser_readc(void* handle)
{
  unsigned char buf;

  int rc = gbser_read(handle, &buf, 1);
  if (rc > 0) {
    return buf;
  } else if (rc == 0) {
    return gbser_NOTHING;
  } else {
    return gbser_ERROR;
  }
}

/* Read a single character from the port, waiting up to |ms|
 * milliseconds for a character to be available.
 */
int gbser_readc_wait(void* handle, unsigned ms)
{
  unsigned char buf;

  int rc = gbser_read_wait(handle, &buf, 1, ms);
  if (rc > 0) {
    return buf;
  } else if (rc == 0) {
    return gbser_NOTHING;
  } else {
    return gbser_ERROR;
  }
}

/* Write a null terminated string in |str| to the serial
 * port.
 */
int gbser_print(void* handle, const char* str)
{
  return gbser_write(handle, str, (unsigned) strlen(str));
}

/* Write a single character to the serial port.
 */
int gbser_writec(void* handle, int c)
{
  return gbser_write(handle, &c, 1);
}
