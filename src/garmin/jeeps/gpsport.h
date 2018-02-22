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
