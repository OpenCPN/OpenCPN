/*
    Serial interface for POSIX tty handling.

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
#include <cerrno>
#include <cstdarg>
#include <cstdio>
#include <fcntl.h>
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>

struct gbser_handle {
  struct termios  old_tio;
  struct termios  new_tio;
  int             fd;
  unsigned        vmin, vtime;
  unsigned long   magic;

  unsigned char   inbuf[BUFSIZE];
  unsigned        inbuf_used;
};

/* Wrapper to safely cast a void * into a gbser_handle */
static gbser_handle* gbser__get_handle(void* p)
{
  auto* h = (gbser_handle*) p;
  assert(h->magic == MYMAGIC);
  return h;
}

speed_t mkspeed(unsigned br)
{
  switch (br) {
  case   1200:
    return  B1200;
  case   2400:
    return  B2400;
  case   4800:
    return  B4800;
  case   9600:
    return  B9600;
  case  19200:
    return  B19200;
  case  38400:
    return  B38400;
#if defined B57600
  case  57600:
    return  B57600;
#endif
#if defined B115200
  case 115200:
    return B115200;
#endif
#if defined B230400
  case 230400:
    return B230400;
#endif
  default:
    fatal("Unsupported serial speed: %d\n", br);
    return 0;   /* keep compiler happy */
  }
}

using hp_time = struct timeval;

static void get_time(hp_time* tv)
{
  gettimeofday(tv, nullptr);
}

static double elapsed(hp_time* tv)
{
  hp_time now;
  double ot = (double) tv->tv_sec  * 1000 +
              (double) tv->tv_usec / 1000;
  double nt;
  gettimeofday(&now, nullptr);
  nt = (double) now.tv_sec  * 1000 +
       (double) now.tv_usec / 1000;
  /*printf("elapsed -> %f\n", nt - ot);*/
  return nt - ot;
}

static int set_rx_timeout(gbser_handle* h, unsigned vmin, unsigned vtime)
{
  if (vmin  > 255) {
    vmin  = 255;
  }
  if (vtime > 255) {
    vtime = 255;
  }
  if (vmin != h->vmin || vtime != h->vtime) {
    h->vmin  = h->new_tio.c_cc[VMIN]  = vmin;
    h->vtime = h->new_tio.c_cc[VTIME] = vtime;

    /*printf("VMIN=%d, VTIME=%d\n", h->vmin, h->vtime);*/

    return tcsetattr(h->fd, TCSANOW, &h->new_tio) ? gbser_ERROR : gbser_OK;
  } else {
    return 0;
  }
}

/* Open a serial port. |port_name| is the (platform specific) name
 * of the serial device to open. Under WIN32 familiar DOS port names
 * ('com1:') are translated into the equivalent name required by
 * WIN32
 */
void* gbser_init(const char* port_name)
{
  gbser_handle* h;

  gbser__db(4, "gbser_init(\"%s\")\n", port_name);

  h = (gbser_handle*) xcalloc(sizeof *h, 1);
  h->magic = MYMAGIC;
  h->vmin = h->vtime = 0;

  if (0 == strcmp(port_name, "-")) {
    h->fd = 0;
    return h;
  } else if (h->fd = open(port_name, O_RDWR | O_NOCTTY), h->fd == -1) {
    warning("Failed to open port (%s)\n", strerror(errno));
    goto failed;
  }

  if (!isatty(h->fd)) {
    warning("%s is not a TTY\n", port_name);
    goto failed;
  }

  if (gbser_set_port(h, 4800, 8, 0, 1)) {
    warning("gbser_set_port() failed\n");
    goto failed;
  }

  return h;

failed:
  if (h->fd != -1) {
    close(h->fd);
  }

  xfree(h);

  return nullptr;
}

/* Close a serial port
 */
void gbser_deinit(void* handle)
{
  gbser_handle* h = gbser__get_handle(handle);

  tcsetattr(h->fd, TCSAFLUSH, &h->old_tio);
  close(h->fd);

  xfree(h);
}

int gbser_set_port(void* handle, unsigned speed, unsigned bits, unsigned parity, unsigned stop)
{
  gbser_handle* h = gbser__get_handle(handle);
  speed_t s;

  static unsigned bit_flags[] = {
    0, 0, 0, 0, 0, CS5, CS6, CS7, CS8
  };

  if (bits < 5 || bits > 8) {
    fatal("Unsupported bits setting: %d\n", bits);
  }

  if (parity > 2) {
    fatal("Unsupported parity setting: %d\n", parity);
  }

  if (stop < 1 || stop > 2) {
    fatal("Unsupported stop setting: %d\n", stop);
  }

  s = mkspeed(speed);

  /* TODO: We don't /fully/ initialise the port's stat here... */

  tcgetattr(h->fd, &h->old_tio);

  h->new_tio = h->old_tio;

  /* clear bits */
//	cfmakeraw(&h->new_tio);
  h->new_tio.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP
                          |INLCR|IGNCR|ICRNL|IXON);
  h->new_tio.c_oflag &= ~OPOST;
  h->new_tio.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
  h->new_tio.c_cflag &= ~(CSIZE|PARENB);
  h->new_tio.c_cflag |= CS8;

  h->new_tio.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP |
                          INLCR  | IGNCR  | IXON);
  h->new_tio.c_cflag &= ~(CSIZE  | PARENB | PARODD | CSTOPB);

  /* set data bits, */
  h->new_tio.c_cflag |= bit_flags[bits];

  /* stop bits and... */
  if (stop == 2) {
    h->new_tio.c_cflag |= CSTOPB;
  }

  /* parity */
  if (parity != 0) {
    h->new_tio.c_cflag |= PARENB;
    if (parity == 1) {
      h->new_tio.c_cflag |= PARODD;
    }
  }

  h->new_tio.c_oflag = 0;
  h->new_tio.c_lflag = 0;

  h->new_tio.c_cc[VMIN]  = h->vmin;
  h->new_tio.c_cc[VTIME] = h->vtime;

  cfsetospeed(&h->new_tio, s);
  cfsetispeed(&h->new_tio, s);

  return tcsetattr(h->fd, TCSADRAIN, &h->new_tio) ? gbser_ERROR : gbser_OK;
}

unsigned gbser__read_buffer(void* handle, void** buf, unsigned* len)
{
  gbser_handle* h = gbser__get_handle(handle);
  unsigned count = *len;
  auto* cp = (unsigned char*) *buf;
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

  if (nullptr == ms || 0 == *ms) {
    if ((rc = set_rx_timeout(h, 0, 0), rc < 0) ||
        (rc = read(h->fd, h->inbuf + h->inbuf_used,
                   want - h->inbuf_used), rc < 0)) {
      return gbser_ERROR;
    }
    h->inbuf_used += rc;
    /*printf("Got %d bytes\n", rc);*/
  } else {
    double time_left = *ms;
    hp_time tv;
    get_time(&tv);

    for (;;) {
      fd_set rec;
      struct timeval t;

      time_left = *ms - elapsed(&tv);
      if (time_left <= 0 || h->inbuf_used >= want) {
        break;
      }

      FD_ZERO(&rec);
      FD_SET(h->fd, &rec);

      t.tv_sec  = (time_t) time_left / 1000;
      t.tv_usec = ((unsigned) time_left % 1000) * 1000;

      if (select(h->fd + 1, &rec, nullptr, nullptr, &t) < 0) {
        return gbser_ERROR;
      }

      time_left = *ms - elapsed(&tv);

      if (FD_ISSET(h->fd, &rec)) {
#if 0
        // See below comment.

        unsigned vmin = 0, vtime = 0;
        if (time_left >= 100) {
          vmin  = want - h->inbuf_used;
          vtime = (unsigned) time_left / 100;
        }
#endif
        // The commented out call to set_rx_timeout here is totally
        // legal by POSIX standards but does result in a flurry of
        // of tcsetattrs that slightly tweak VMIN/VTIME while there
        // is incoming data.   This has been shown to trigger driver
        // bugs in the Prolific drivers for Mac and in certain Linux
        // kernels, thought the latter has since been fixed.
        // So although removing this means that the timeout behaviour
        // is actually different on POSIX and WIN32, it triggers
        // fewer buts this way.  2/12/2008 RJL
        if (/* (rc = set_rx_timeout(h, vmin, vtime), rc < 0) || */
          (rc = read(h->fd, h->inbuf + h->inbuf_used,
                     want - h->inbuf_used), rc < 0)) {
          return gbser_ERROR;
        }
        h->inbuf_used += rc;
        /*printf("Got %d bytes\n", rc);*/
      }
    }
    *ms = (time_left < 0) ? 0 : time_left;
  }

  return h->inbuf_used;
}

/* Discard any pending input on the serial port.
 */
int gbser_flush(void* handle)
{
  gbser_handle* h = gbser__get_handle(handle);
  h->inbuf_used = 0;
  if (tcflush(h->fd, TCIFLUSH)) {
    return gbser_ERROR;
  }

  return gbser_OK;
}

/* Write |len| bytes from |buf| to the serial port.
 */
int gbser_write(void* handle, const void* buf, unsigned len)
{
  gbser_handle* h = gbser__get_handle(handle);
  const char* bp = (const char*) buf;
  int rc;
  while (len > 0) {
    /*printf("write(%d, %p, %d)\n", h->fd, bp, len);*/
    if (rc = write(h->fd, bp, len), rc < 0) {
      printf("rc = %d, errno = %d (%s)\n", rc, errno, strerror(errno));
      return gbser_ERROR;
    }
    len -= rc;
    bp  += rc;
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
  int fd;
  int is_port = 0;

  if (fd = open(port_name, O_RDWR | O_NOCTTY), fd == -1) {
    gbser__db(1, "Failed to open port (%s) to check its type\n", strerror(errno));
    return 0;
  }

  is_port = isatty(fd);

  close(fd);

  return is_port;
}

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

const char* fix_win_serial_name_r(const char* comname, char* obuf, size_t len)
{
  strncpy(obuf, comname, len);
  return obuf;
}

static char gb_com_buffer[100];

const char* fix_win_serial_name(const char* comname)
{
  return fix_win_serial_name_r(comname, gb_com_buffer, sizeof(gb_com_buffer));
}

/* Read from the serial port until the specified |eol| character is
 * found. Any character matching |discard| will be discarded. To
 * read lines terminated by 0x0A, 0x0D discarding linefeeds use
 * gbser_read_line(h, buf, len, 1000, 0x0D, 0x0A);
 * The terminating character and any discarded characters are not
 * stored in the buffer.
 */
int gbser_read_line(void* handle, void* buf, unsigned len, unsigned ms, int eol, int discard)
{
  char* bp = (char*) buf;
  unsigned pos = 0;
  hp_time tv;
  get_time(&tv);
  bp[pos] = '\0';
  for (;;) {
    signed time_left = ms - elapsed(&tv);
    int c;

    if (time_left <= 0) {
      return gbser_TIMEOUT;
    }
    c = gbser_readc_wait(handle, time_left);
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
