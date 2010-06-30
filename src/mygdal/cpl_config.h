#ifndef __CPL_CONFIG__
#define __CPL_CONFIG__

/* Define if you don't have vprintf but do have _doprnt.  */
#undef HAVE_DOPRNT

/* Define if you have the vprintf function.  */
/* Note that some are predefined by wxWidgets   */

#define HAVE_VPRINTF

#ifndef HAVE_VSNPRINTF
//DSR ....comment for MSW #define HAVE_VSNPRINTF
#endif

#ifdef __WXMSW__
#undef HAVE_SNPRINTF
#endif

#ifdef WINDOWS          /*  This definition comes from cpl_port.h  */
#undef HAVE_SNPRINTF
#endif

/* Define if you have the ANSI C header files.  */
#ifndef STDC_HEADERS
#  define STDC_HEADERS
#endif

/* Define if you have the <fcntl.h> header file.  */
#undef HAVE_FCNTL_H

/* Define if you have the <unistd.h> header file.  */
#undef HAVE_UNISTD_H

/* Define if you have the <stdint.h> header file.  */
#undef HAVE_STDINT_H

#undef HAVE_LIBDL

/* Define to 1 if you have the <locale.h> header file. */
#define HAVE_LOCALE_H 1

/* Define to 1 if you have the `localtime_r' function. */
#undef HAVE_LOCALTIME_R

#undef HAVE_DLFCN_H
#undef HAVE_DBMALLOC_H
#undef HAVE_LIBDBMALLOC
#undef WORDS_BIGENDIAN

#define CPL_DISABLE_DLL


#endif
