/**************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
 *   Copyright (C) 2010 Anton Samsonov                                     *
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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

/**
 * \file
 *
 * Hash of arbitrary length
 */

#ifndef __FLEXHASH_H__
#define __FLEXHASH_H__

#include <vector>
#include "ssl/sha1.h"

/**
 * Class for computing hash of arbitrary length.
 */
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
