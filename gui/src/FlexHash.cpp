
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

#include <vector>
#include <memory.h>
#include "FlexHash.h"

#define FLEXHASH_INTERNAL_SIZE static_cast<size_t>(20)

FlexHash::FlexHash(size_t output_octets) : m_Output(output_octets) {}

void FlexHash::Reset(void) { sha1_starts(&this->m_Context); }

void FlexHash::Update(const void* input, size_t input_octets) {
  sha1_update(&this->m_Context, reinterpret_cast<const unsigned char*>(input),
              input_octets);
}

void FlexHash::Finish(void) {
  unsigned char output[FLEXHASH_INTERNAL_SIZE];
  sha1_finish(&this->m_Context, output);
  if (this->m_Output.size() <= FLEXHASH_INTERNAL_SIZE) {
    memcpy(&this->m_Output[0], output, this->m_Output.size());
  } else {
    memcpy(&this->m_Output[0], output, FLEXHASH_INTERNAL_SIZE);
    size_t available_octets = FLEXHASH_INTERNAL_SIZE;
    size_t remaining_octets = this->m_Output.size() - available_octets;
    while (remaining_octets) {
      size_t current_octets =
          ((remaining_octets > FLEXHASH_INTERNAL_SIZE) ? FLEXHASH_INTERNAL_SIZE
                                                       : remaining_octets);
      sha1_starts(&this->m_Context);
      sha1_update(&this->m_Context,
                  reinterpret_cast<const unsigned char*>(&this->m_Output[0]),
                  available_octets);
      sha1_finish(&this->m_Context, output);
      memcpy(&this->m_Output[available_octets], output, current_octets);
      available_octets += FLEXHASH_INTERNAL_SIZE;
      remaining_octets -= current_octets;
    }
  }
}

void FlexHash::Receive(void* output) {
  memcpy(output, &this->m_Output[0], this->m_Output.size());
}

void FlexHash::Compute(const void* input, size_t input_octets, void* output) {
  this->Reset();
  this->Update(input, input_octets);
  this->Finish();
  this->Receive(output);
}

void FlexHash::Compute(const void* input, size_t input_octets, void* output,
                       size_t output_octets) {
  FlexHash hasher(output_octets);
  hasher.Compute(input, input_octets, output);
}

bool FlexHash::Test(void) {
  // Input test vector for "The quick brown fox jumps over the lazy dog".
  static unsigned char input[] = {
      0x54, 0x68, 0x65, 0x20, 0x71, 0x75, 0x69, 0x63, 0x6b, 0x20, 0x62,
      0x72, 0x6f, 0x77, 0x6e, 0x20, 0x66, 0x6f, 0x78, 0x20, 0x6a, 0x75,
      0x6d, 0x70, 0x73, 0x20, 0x6f, 0x76, 0x65, 0x72, 0x20, 0x74, 0x68,
      0x65, 0x20, 0x6c, 0x61, 0x7a, 0x79, 0x20, 0x64, 0x6f, 0x67};
  // Output test vector for SHA-1 engine and 256-bit result.
  static unsigned char output_reference[] = {
      0x2f, 0xd4, 0xe1, 0xc6, 0x7a, 0x2d, 0x28, 0xfc, 0xed, 0x84, 0x9e,
      0xe1, 0xbb, 0x76, 0xe7, 0x39, 0x1b, 0x93, 0xeb, 0x12, 0xa4, 0xe4,
      0xd2, 0x6f, 0xd0, 0xc6, 0x45, 0x5e, 0x23, 0xe2, 0x18, 0x7c};
  char output_current[(sizeof output_reference)];
  Compute(input, (sizeof input), output_current, (sizeof output_current));
  return (memcmp(output_current, output_reference, (sizeof output_reference)) ==
          0);
}
