/*
    OS Abstraction for serial interface.

    Copyright (C) 2006-2014  Robert Lipe, robertlipe+source@gpsbabel.org

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

#ifndef __GBSER_H
#define __GBSER_H

#include <cstddef> // for size_t

#define gbser_OK         0
#define gbser_NOTHING   -1
#define gbser_TIMEOUT   -2
#define gbser_ERROR     -3

#if defined(__WIN32__) || defined(__CYGWIN__)
#define WINSERIAL 1
#else
#define POSIXSERIAL 1
#endif

/* Open a serial port. |port_name| is the (platform specific) name
 * of the serial device to open. Under WIN32 familiar DOS port names
 * ('com1:') are translated into the equivalent name required by
 * WIN32
 */
void* gbser_init(const char* port_name);

/* Close a serial port
 */
void gbser_deinit(void* handle);

/* Set the serial port speed.
 */
int gbser_set_speed(void* handle, unsigned speed);

/* Set the serial port speed, start, parity and stop bits */
int gbser_set_port(void* handle, unsigned speed,
                   unsigned bits,
                   unsigned parity,
                   unsigned stop);

/* Set the serial port up by parsing the supplied parameter string.
 * Valid parameter strings look like '4800,8,N,1'. Parsing is case-
 * insensitive, spaces are allowed around the commas and omitted
 * trailing fields will default to '8', 'N' and '1'
 */
int gbser_setup(void* handle, const char* spec);

/* Return true if there are characters available on the serial port
 */
int gbser_avail(void* handle);

/* Read as many bytes as are available without blocking. At most |len|
 * bytes will be read. Returns the number of bytes read or gbser_ERROR if an
 * error occurs.
 */
int gbser_read(void* handle, void* buf, unsigned len);

/* Read the specified number of bytes. Block until the requested number
 * of bytes have been read or the timeout (in ms) is exceeded.
 */
int gbser_read_wait(void* handle, void* buf, unsigned len, unsigned ms);

/* Read from the serial port until the specified |eol| character is
 * found. Any character matching |discard| will be discarded. To
 * read lines terminated by 0x0A0x0D discarding linefeeds use
 * gbser_read_line(h, buf, len, 1000, 0x0D, 0x0A);
 */
int gbser_read_line(void* handle, void* buf,
                    unsigned len, unsigned ms,
                    int eol, int discard);

/* Read a single character from the port, returning immediately if
 * none are available. TODO: Define return values
 */
int gbser_readc(void* handle);

/* Read a single character from the port, waiting up to |ms|
 * milliseconds for a character to be available.
 */
int gbser_readc_wait(void* handle, unsigned ms);

/* Discard any pending input on the serial port.
 */
int gbser_flush(void* handle);

/* Write |len| bytes from |buf| to the serial port.
 */
int gbser_write(void* handle, const void* buf, unsigned len);

/* Write a null terminated string in |str| to the serial
 * port.
 */
int gbser_print(void* handle, const char* str);

/* Write a single character to the serial port.
 */
int gbser_writec(void* handle, int c);

/* Return true if a port name seems to refer to a serial port.
 * On Windows this tests the filename (against the regex
 * /^(\\\\\.\\\\)?com\d+:?$/i). On Posix it returns the value of
 * isatty()
 */
int gbser_is_serial(const char* port_name);

/* This isn't part of the above abstraction; it's just a helper for
 * the other serial modules in the tree.
 *
 * Windows does a weird thing with serial ports.
 * COM ports 1 - 9 are "COM1:" through "COM9:"
 * The one after that is \\.\\com10 - this function tries to plaster over
 * that.
 * It returns a pointer to a statically allocated buffer and is therefore not
 * thread safe.   The buffer pointed to remains valid only until the next
 * call to this function.
 */

const char* fix_win_serial_name_r(const char* comname, char* obuf, size_t len);
const char* fix_win_serial_name(const char* comname);

#endif /* GBSER_H */
