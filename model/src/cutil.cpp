/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Extern C Linked Utilities
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.             *
 ***************************************************************************
 *
 */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#include "model/cutil.h"

double round_msvc(double x) { return (floor(x + 0.5)); }

#ifdef __MSVC__
#include <windows.h>
#include <float.h>  // for _clear87()

long __stdcall MyUnhandledExceptionFilter(
    struct _EXCEPTION_POINTERS *ExceptionInfo) {
  //    return EXCEPTION_EXECUTE_HANDLER ;        // terminates the app

  switch (ExceptionInfo->ExceptionRecord->ExceptionCode) {
    case EXCEPTION_FLT_DENORMAL_OPERAND:
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:
    case EXCEPTION_FLT_INEXACT_RESULT:
    case EXCEPTION_FLT_INVALID_OPERATION:
    case EXCEPTION_FLT_OVERFLOW:
    case EXCEPTION_FLT_STACK_CHECK:
    case EXCEPTION_FLT_UNDERFLOW:
      _clear87();
      return EXCEPTION_CONTINUE_EXECUTION;  // retry

    default:
      return EXCEPTION_CONTINUE_SEARCH;  // standard fatal dialog box
  }
}
#endif

/*          Replacement for __MSVC__ in absence of snprintf or _snprintf  */
#ifdef __MSVC__
int mysnprintf(char *buffer, int count, const char *format, ...) {
  int ret;

  va_list arg;
  va_start(arg, format);
  ret = _vsnprintf(buffer, count, format, arg);

  va_end(arg);
  return ret;
}
#endif

int NextPow2(int size) {
  int n = size - 1;  // compute dimensions needed as next larger power of 2
  int shift = 1;
  while ((n + 1) & n) {
    n |= n >> shift;
    shift <<= 1;
  }

  return n + 1;
}
