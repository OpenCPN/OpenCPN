/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Decode/encode binary data.
 * Author:   Taru Karttunen
 *
 ***************************************************************************
 *   Copyright (C) 2019 by Taru Karttunen                                  *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************
 *
 *
 */


#ifndef _OCPN_BITCAST_H_
#define _OCPN_BITCAST_H_

#include <string.h>

// decode_native<T> decodes native representation of type T and returns it.
// Good for reading e.g. floating point numbers from unaligned memory buffers.
// Modern compilers are good enough to optimize this and it works on all platforms.
template<typename T>
T decode_native(const void *ptr) {
  T ret;
  memcpy(&ret, ptr, sizeof(T));
  return ret;
}


// encode_native<T> encode native representation of type T into the given pointer.
// Good for e.g. encoding floating point numbers into unaligned memory buffers.
// Modern compilers are good enough to optimize this and it works on all platforms.
template<typename T>
void encode_native(void *ptr, T val) {
  memcpy(ptr, &val, sizeof(T));
}

#endif
