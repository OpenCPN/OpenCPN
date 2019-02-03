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
#ifdef __cplusplus
extern "C"
{
#endif


#ifndef garmin_wrapper_utils_h
#define garmin_wrapper_utils_h


#include <time.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>



/*    Some function stubs sprinkled throughout jeeps  */
void fatal(const char *, ...);
void *xrealloc(void *p, size_t s);
void *xcalloc(size_t nmemb, size_t size);
int case_ignore_strcmp(const char *s1, const char *s2);
int case_ignore_strncmp(const char *s1, const char *s2, int n);
void xfree(void *mem);
char *xstrdup(const char *s);


#ifdef __WIN32__
/* This isn't part of the above abstraction; it's just a helper for
 * the other serial modules in the tree.
 *
 * Windows does a weird thing with serial ports.
 * COM ports 1 - 9 are "COM1:" through "COM9:"
 * The one after that is \\.\\com10 - this function tries to plaster over
 * that.
 * It returns a pointer to a staticly allocated buffer and is therefore not
 * thread safe.   The buffer pointed to remains valid only until the next
 * call to this function.
 */
#ifndef snprintf
  #define snprintf  _snprintf
#endif

const char *fix_win_serial_name_r(const char *comname, char *obuf, size_t len);
const char *fix_win_serial_name(const char *comname);
#endif



void le_write16(void *addr, const unsigned value);
void le_write32(void *addr, const unsigned value);
signed int le_read16(const void *addr);
signed int le_read32(const void *addr);

#endif      /* garmin_wrapper_utils_h */

#ifdef __cplusplus
}
#endif
