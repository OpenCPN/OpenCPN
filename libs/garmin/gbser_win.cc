/*
    Serial interface - Windows layer.

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

#include <windows.h>
#include <setupapi.h>

#include <cassert>
#include <cstdarg>
#include <cstdio>

struct gbser_handle {
  HANDLE          comport;
  DWORD           timeout;
  unsigned long   magic;

  unsigned char   inbuf[BUFSIZE];
  unsigned        inbuf_used;
};

#define DEV_PREFIX "\\\\.\\\\"

/* Wrapper to safely cast a void * into a gbser_handle */
static gbser_handle* gbser__get_handle(void* p)
{
  gbser_handle* h = (gbser_handle*) p;
  assert(h->magic == MYMAGIC);
  return h;
}

DWORD mkspeed(unsigned br)
{
  switch (br) {
  case   1200:
    return CBR_1200;
  case   2400:
    return CBR_2400;
  case   4800:
    return CBR_4800;
  case   9600:
    return CBR_9600;
  case  19200:
    return CBR_19200;
  case  38400:
    return CBR_38400;
  case  57600:
    return CBR_57600;
  case 115200:
    return CBR_115200;
  case 230400:
  // Per https://msdn.microsoft.com/en-us/library/windows/desktop/aa363214
  // "This member can be an actual baud rate value, or one of the 
  // following indexes."
  // They provide a CBR_25600, which would be programmable on a 16450 only
  // with a bizarre oscillator crystal, but don't provide a 230400, such
  // as is used as the default in skytraq.
#if defined CBR_230400
    return CBR_230400;
#else
    return 230400;
#endif
  default:
    fatal("Unsupported serial speed: %d\n", br);
    return 0;   /* keep compiler happy */
  }
}

using hp_time = LARGE_INTEGER;

static void get_time(hp_time* tv)
{
  QueryPerformanceCounter(tv);
}

static double elapsed(hp_time* tv)
{
  hp_time now;
  LARGE_INTEGER tps;

  QueryPerformanceFrequency(&tps);
  QueryPerformanceCounter(&now);

  return ((double)(now.QuadPart - tv->QuadPart) /
          (double) tps.QuadPart) * 1000;
}

static int set_rx_timeout(gbser_handle* h, DWORD timeout)
{
  if (timeout != h->timeout) {
    COMMTIMEOUTS to;

    if (!GetCommTimeouts(h->comport, &to)) {
      return gbser_ERROR;
    }

    to.ReadIntervalTimeout         = timeout;
    to.ReadTotalTimeoutMultiplier  = 0;
    to.ReadTotalTimeoutConstant    = timeout;
    to.WriteTotalTimeoutMultiplier = 0;
    to.WriteTotalTimeoutConstant   = 0;

    if (!SetCommTimeouts(h->comport, &to)) {
      return gbser_ERROR;
    } else {
      h->timeout = timeout;
      return gbser_OK;
    }
  } else {
    return gbser_OK;
  }
}

/* This isn't part of the above abstraction; it's just a helper for
 * the other serial modules in the tree.
 *
 * Windows does a weird thing with serial ports.
 * COM ports 1 - 9 are "COM1:" through "COM9:"
 * The one after that is \\.\\com10 - this function tries to plaster over
 * that.
 *
 * Worse still, Win98 and ME fail the open if you rename com1 to be \\.\\com1:
 *
 * It returns a pointer to a staticly allocated buffer and is therefore not
 * thread safe.   The buffer pointed to remains valid only until the next
 * call to this function.
 */

const char*
fix_win_serial_name_r(const char* comname, char* obuf, size_t len)
{
  if (!gbser_is_serial(comname) ||
      ((strlen(comname) == 5) && (comname[4] == ':')) ||
      ((strlen(comname) == 4) && (case_ignore_strncmp(comname, "com", 3) == 0))
     ) {
    strncpy(obuf, comname, len);
  } else {
    snprintf(obuf, len, DEV_PREFIX "%s", comname);
    size_t l = strlen(obuf);
    if (obuf[l - 1] == ':') {
      obuf[l - 1] = '\0';
    }
  }

  return obuf;
}

static char gb_com_buffer[100];

const char* fix_win_serial_name(const char* comname)
{
  return fix_win_serial_name_r(comname, gb_com_buffer, sizeof(gb_com_buffer));
}

/* Open a serial port. |port_name| is the (platform specific) name
 * of the serial device to open. Under WIN32 familiar DOS port names
 * ('com1:') are translated into the equivalent name required by
 * WIN32
 */
void* gbser_init(const char* port_name)
{
  gbser_handle* h = (gbser_handle*) xcalloc(1, sizeof(*h));
  const char* xname = fix_win_serial_name(port_name);

  gbser__db(2, "Translated port name: \"%s\"\n", xname);

  h->magic = MYMAGIC;

  HANDLE comport = CreateFileA(xname, GENERIC_READ | GENERIC_WRITE,
                               0, NULL, OPEN_EXISTING, 0, NULL);

  if (comport == INVALID_HANDLE_VALUE) {
    goto failed;
  }

  h->comport = comport;
  h->timeout = 1;
  if (gbser_set_port(h, 4800, 8, 0, 1) || set_rx_timeout(h, 0)) {
    goto failed;
  }

  return h;

failed:
  if (comport) {
    CloseHandle(h->comport);
  }
  xfree(h);

  return NULL;
}

/* Close a serial port
 */
void gbser_deinit(void* handle)
{
  gbser_handle* h = gbser__get_handle(handle);

  CloseHandle(h->comport);

  xfree(h);
}

int gbser_set_port(void* handle, unsigned speed, unsigned bits, unsigned parity, unsigned stop)
{
  gbser_handle* h = gbser__get_handle(handle);
  DCB tio;

  if (bits < 5 || bits > 8) {
    fatal("Unsupported bits setting: %d\n", bits);
  }

  if (parity > 2) {
    fatal("Unsupported parity setting: %d\n", parity);
  }

  if (stop < 1 || stop > 2) {
    fatal("Unsupported stop setting: %d\n", stop);
  }

  tio.DCBlength = sizeof(DCB);
  GetCommState(h->comport, &tio);

  tio.BaudRate            = mkspeed(speed);
  tio.fBinary             = true;
  tio.fParity             = true;
  tio.fOutxCtsFlow        = false;
  tio.fOutxDsrFlow        = false;
  tio.fDtrControl         = DTR_CONTROL_ENABLE;
  tio.fDsrSensitivity     = false;
  tio.fTXContinueOnXoff   = true;
  tio.fOutX               = false;
  tio.fInX                = false;
  tio.fErrorChar          = false;
  tio.fNull               = false;
  tio.fRtsControl         = RTS_CONTROL_ENABLE;
  tio.fAbortOnError       = false;
  tio.ByteSize            = bits;
  tio.Parity              = parity == 0 ? NOPARITY :
                            (parity == 1 ? ODDPARITY : EVENPARITY);
  tio.StopBits            = stop   == 1 ? ONESTOPBIT : TWOSTOPBITS;

  if (!SetCommState(h->comport, &tio)) {
    return gbser_ERROR;
  }
  return gbser_OK;
}

unsigned gbser__read_buffer(void* handle, void** buf, unsigned* len)
{
  gbser_handle* h = gbser__get_handle(handle);
  unsigned count = *len;
  unsigned char* cp = (unsigned char*) *buf;
  if (count > h->inbuf_used) {
    count = h->inbuf_used;
  }

  memcpy(cp, h->inbuf, count);
  memmove(h->inbuf, h->inbuf + count,
          h->inbuf_used - count);
  h->inbuf_used -= count;
  *len -= count;
  cp   += count;
  *buf = (void*) cp;
  return count;
}

/* Return when the input buffer contains at least |want| bytes or |*ms|
 * milliseconds have elapsed. |ms| may be NULL or |*ms| may be zero to
 * poll the port for available bytes and return immediately. |*ms| will
 * be updated to indicate the remaining time on exit.
 * Returns the number of bytes available (>=0) or an error code (<0).
 */
int gbser__fill_buffer(void* handle, unsigned want, unsigned* ms)
{
  int rc;
  gbser_handle* h = gbser__get_handle(handle);

  if (want > BUFSIZE) {
    want = BUFSIZE;
  }

  /* Already got enough bytes? */
  if (h->inbuf_used >= want) {
    return h->inbuf_used;
  }

  if (NULL == ms || 0 == *ms) {
    DWORD err, nread;
    COMSTAT stat;
    ClearCommError(h->comport, &err, &stat);
    if (stat.cbInQue > 0) {
      DWORD count = want - h->inbuf_used;
      if (count > stat.cbInQue) {
        count = stat.cbInQue;
      }
      if (rc = set_rx_timeout(h, 1), rc) {
        return rc;
      }
      if (!ReadFile(h->comport, h->inbuf + h->inbuf_used,
                    count, &nread, NULL)) {
        err = GetLastError();
        if (err != ERROR_COUNTER_TIMEOUT && err != ERROR_TIMEOUT) {
          return gbser_ERROR;
        }
      }
      h->inbuf_used += nread;
    }
  } else {
    hp_time tv;
    DWORD nread;
    get_time(&tv);
    if (rc = set_rx_timeout(h, *ms), rc) {
      return rc;
    }
    if (!ReadFile(h->comport, h->inbuf + h->inbuf_used,
                  want - h->inbuf_used,
                  &nread, NULL)) {
      DWORD err = GetLastError();
      if (err != ERROR_COUNTER_TIMEOUT && err != ERROR_TIMEOUT) {
        return gbser_ERROR;
      }
    }
    h->inbuf_used += nread;
    double time_left = *ms - elapsed(&tv);
    *ms = time_left < 0 ? 0 : (unsigned) time_left;
  }

  return h->inbuf_used;
}

/* Discard any pending input on the serial port.
 */
int gbser_flush(void* handle)
{
  gbser_handle* h = gbser__get_handle(handle);
  h->inbuf_used = 0;
  if (!PurgeComm(h->comport, PURGE_RXCLEAR)) {
    return gbser_ERROR;
  }
  return gbser_OK;
}

/* Write |len| bytes from |buf| to the serial port.
 */
int gbser_write(void* handle, const void* buf, unsigned len)
{
  gbser_handle* h = gbser__get_handle(handle);
  DWORD nwritten;
  const char* bp = (const char*) buf;
  /* Not sure we need to spin here - but this'll work even if we don't */
  while (len > 0) {
    if (!WriteFile(h->comport, bp, len, &nwritten, NULL)) {
      return gbser_ERROR;
    }
    len -= nwritten;
    bp  += nwritten;
  }
  return gbser_OK;
}

/* Return true if a port name seems to refer to a serial port.
 * On Windows this tests the filename (against the regex
 * /^(\\\\\.\\\\)?com\d+:?$/i). On Posix it returns the value of
 * isatty()
 */

int gbser_is_serial(const char* port_name)
{
  const char* pfx = DEV_PREFIX;
  size_t pfx_l = strlen(pfx);
  const char* com = "COM";
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

/* Read from the serial port until the specified |eol| character is
 * found. Any character matching |discard| will be discarded. To
 * read lines terminated by 0x0A0x0D discarding linefeeds use
 * gbser_read_line(h, buf, len, 1000, 0x0D, 0x0A);
 */
int gbser_read_line(void* handle, void* buf,
                    unsigned len, unsigned ms,
                    int eol, int discard)
{
  char* bp = (char*) buf;
  unsigned pos = 0;
  hp_time tv;
  get_time(&tv);
  bp[pos] = '\0';
  for (;;) {
    signed time_left = ms - elapsed(&tv);
    if (time_left <= 0) {
      return gbser_TIMEOUT;
    }
    int c = gbser_readc_wait(handle, time_left);
    if (c == gbser_ERROR) {
      return c;
    } else if (c == eol) {
      return gbser_OK;
    }
    if (c != gbser_NOTHING && c != discard && pos < len - 1) {
      bp[pos++] = c;
      bp[pos]   = '\0';
    }
  }
}
