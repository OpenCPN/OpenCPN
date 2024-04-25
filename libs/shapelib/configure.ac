dnl Process this file with autoconf to produce a configure script.

m4_define(shapelib_version_major, 1)
m4_define(shapelib_version_minor, 6)
m4_define(shapelib_version_micro, 0)

AC_PREREQ(2.62)
AC_INIT(shapelib, shapelib_version_major.shapelib_version_minor.shapelib_version_micro)
AC_CONFIG_MACRO_DIR(m4)
AC_CONFIG_SRCDIR(shapefil.h)

AM_INIT_AUTOMAKE([foreign -Wall -Wextra])

AM_SILENT_RULES([yes])

m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

dnl See http://www.gnu.org/software/libtool/manual/html_node/Updating-version-info.html
AC_SUBST([SHAPELIB_SO_VERSION], [4:0:0])

AC_PROG_CC
AC_PROG_CXX
AM_PROG_CC_C_O
AC_LANG([C])
AC_LANG([C++])

dnl ************************** Libtool initialization *************************
LT_INIT

dnl ****************************** Detect libm *******************************

AC_CHECK_LIB(m,floor,LIBM=-lm)
AC_SUBST([LIBM])

dnl ****************************** Detect Win32 *******************************
AC_MSG_CHECKING([for some Win32 platform])
case "$host" in
  *-*-mingw*|*-*-cygwin*)
    platform_win32=yes
    ;;
  *)
    platform_win32=no
    ;;
esac
AC_MSG_RESULT([$platform_win32])
AM_CONDITIONAL(PLATFORM_WIN32, test "$platform_win32" = "yes")

dnl ********************************* Summary *********************************
echo
echo "Configuration summary for $PACKAGE $VERSION:"
echo "  - Host:                  ${host}"
echo

AC_CONFIG_FILES([Makefile contrib/Makefile shapelib.pc])

AC_OUTPUT
