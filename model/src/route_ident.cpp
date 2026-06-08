/***************************************************************************
 *   Copyright (C) 2026 by David S. Register                               *
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
 * Route identity utility implementation
 */

#include <cmath>
#include "model/route_ident.h"

static constexpr int64_t LATLON_SCALE = 10000000;
static constexpr int64_t SPEED_SCALE = 1000;

void RouteBlobBuilder::AddUint8(uint8_t v) { m_blob.push_back(v); }

void RouteBlobBuilder::AddUint32(uint32_t v) {
  for (int i = 0; i < 4; i++) m_blob.push_back((v >> (8 * i)) & 0xff);
}

void RouteBlobBuilder::AddInt32(int32_t v) {
  uint32_t u = static_cast<uint32_t>(v);
  AddUint32(u);
}

void RouteBlobBuilder::AddUint64(uint64_t v) {
  for (int i = 0; i < 8; i++) m_blob.push_back((v >> (8 * i)) & 0xff);
}

void RouteBlobBuilder::AddInt64(int64_t v) {
  uint64_t u = static_cast<uint64_t>(v);
  AddUint64(u);
}

void RouteBlobBuilder::AddBool(bool v) { m_blob.push_back(v ? 1 : 0); }

void RouteBlobBuilder::AddLatLon(double v) {
  int64_t q = llround(v * LATLON_SCALE);
  AddInt64(q);
}

void RouteBlobBuilder::AddSpeed(double v) {
  int64_t q = llround(v * SPEED_SCALE);
  AddInt64(q);
}
void RouteBlobBuilder::AddString(const std::string& s) {
  AddUint32(static_cast<uint32_t>(s.size()));

  m_blob.insert(m_blob.end(), s.begin(), s.end());
}
