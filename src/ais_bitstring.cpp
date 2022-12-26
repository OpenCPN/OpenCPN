/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
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
 */

#include <cstring>

#include "ais_bitstring.h"

AisBitstring::AisBitstring(const char *str) {
  byte_length = strlen(str);

  for (int i = 0; i < byte_length; i++) {
    bitbytes[i] = to_6bit(str[i]);
  }
}

int AisBitstring::GetBitCount() { return byte_length * 6; }

//  Convert printable characters to IEC 6 bit representation
//  according to rules in IEC AIS Specification
unsigned char AisBitstring::to_6bit(const char c) {
  if (c < 0x30) return (unsigned char)-1;
  if (c > 0x77) return (unsigned char)-1;
  if ((0x57 < c) && (c < 0x60)) return (unsigned char)-1;

  unsigned char cp = c;
  cp += 0x28;

  if (cp > 0x80)
    cp += 0x20;
  else
    cp += 0x28;

  return (unsigned char)(cp & 0x3f);
}

int AisBitstring::GetInt(int sp, int len, bool signed_flag) {
  int acc = 0;
  int s0p = sp - 1;  // to zero base

  int cp, cx, c0;

  for (int i = 0; i < len; i++) {
    acc = acc << 1;
    cp = (s0p + i) / 6;
    cx = bitbytes[cp];  // what if cp >= byte_length?
    c0 = (cx >> (5 - ((s0p + i) % 6))) & 1;
    if (i == 0 && signed_flag &&
        c0)  // if signed value and first bit is 1, pad with 1's
      acc = ~acc;
    acc |= c0;
  }

  return acc;
}

int AisBitstring::GetStr(int sp, int bit_len, char *dest, int max_len) {
  // char temp_str[85];
  char *temp_str = dest;

  char acc = 0;
  int s0p = sp - 1;  // to zero base

  int k = 0;
  int cp, cx, c0, cs;

  int i = 0;
  while (i < bit_len && k < max_len) {
    acc = 0;
    for (int j = 0; j < 6; j++) {
      acc = acc << 1;
      cp = (s0p + i) / 6;
      cx = bitbytes[cp];  // what if cp >= byte_length?
      cs = 5 - ((s0p + i) % 6);
      c0 = (cx >> cs) & 1;
      acc |= c0;

      i++;
    }
    temp_str[k] = (char)(acc & 0x3f);

    if (acc < 32) temp_str[k] += 0x40;
    k++;
  }

  temp_str[k] = 0;

  return k;
}
