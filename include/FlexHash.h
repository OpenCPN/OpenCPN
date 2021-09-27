
/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Hash of arbitrary length
 * Author:   Anton Samsonov
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
 *
 */

#ifndef __FLEXHASH_H__
#define __FLEXHASH_H__

#include <vector>
#include "ssl/sha1.h"

class FlexHash {
public:
  FlexHash(size_t output_octets);

  void Reset(void);
  void Update(const void* input, size_t input_octets);
  void Finish(void);
  void Receive(void* output);

  void Compute(const void* input, size_t input_octets, void* output);
  static void Compute(const void* input, size_t input_octets, void* output,
                      size_t output_octets);

  static bool Test(void);

protected:
  sha1_context m_Context;
  std::vector<unsigned char> m_Output;
};

#endif
