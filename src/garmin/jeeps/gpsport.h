/********************************************************************
** @source JEEPS
**
** @author Copyright (C) 1999 Alan Bleasby
** @version 1.0
** @modified Dec 28 1999 Alan Bleasby. First version
** @modified Copyright (C) 2004, 2005, 2006 Robert Lipe
** @modified Copyright (C) 2007 Achim Schumacher
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
/*
 *  For portability any '32' type must be 32 bits
 *                  and '16' type must be 16 bits
 */

/* Since GPSBabel already has an integer size abstraction layer and
 * defs.h includes gbtypes.h before this file, just use that.
 */

/* dsr  Include the top level typedefs inline   */

/*
 *  If this is a problem and any interesting system doesn't have the C99-ism
 *  of <stdint.h> we'll come up with something more clever that'll likely
 *  include a gross collection of  __STDC_VERSION >= 199901L  || __GNUC__
 */

#if defined(_MSC_VER)

typedef unsigned long gbuint32;
typedef unsigned short gbuint16;
typedef long gbint32;
typedef short gbint16;

#ifndef snprintf
  #define snprintf  _snprintf
#endif

#else

# if defined (__FreeBSD__)
#  include <inttypes.h>
# else
#  include <stdint.h>
# endif

typedef uint32_t	gbuint32;
typedef uint16_t	gbuint16;
typedef  int32_t 	 gbint32;
typedef  int16_t	 gbint16;

#endif /* defined(_MSC_VER) */

typedef gbuint32	gbsize_t;
typedef unsigned char	gbuint8;
typedef signed char	gbint8;



typedef unsigned char UC;
typedef gbuint16      US;
typedef gbuint16      uint16;
typedef gbint16       int16;
typedef gbuint32      uint32;
typedef gbint32       int32;
