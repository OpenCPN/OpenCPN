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
 * Route identity utility definition
 */

#ifndef OPENCPN_ROUTE_IDENT_H
#define OPENCPN_ROUTE_IDENT_H

#include <cstdint>
#include <string>
#include <vector>

class RouteBlobBuilder {
public:
  void AddUint8(uint8_t v);
  void AddBool(bool v);

  void AddInt32(int32_t v);
  void AddUint32(uint32_t v);

  void AddInt64(int64_t v);
  void AddUint64(uint64_t v);

  void AddString(const std::string& s);

  void AddLatLon(double v);
  void AddSpeed(double v);

  const std::vector<uint8_t>& GetBlob() const { return m_blob; }

private:
  std::vector<uint8_t> m_blob;
};

#endif  // OPENCPN_ROUTE_IDENT_H
