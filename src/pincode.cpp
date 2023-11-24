

/***************************************************************************
 *   Copyright (C) 2023 Alec Leamas                     *
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
 **************************************************************************/

#include "pincode.h"

#include <algorithm>
#include <iomanip>
#include <sstream>

#include "picosha2.h"

Pincode Pincode::Create() {
  srand(time(0));
  return Pincode(std::min(rand() % 10000 + 1, 9999));
}

uint64_t Pincode::Get() const { return m_value; }

std::string Pincode::ToString() const {
  std::stringstream ss;
  ss << std::setw(4) << std::setfill('0') << m_value;
  return ss.str();
}

std::string Pincode::Hash() const {
  std::string hash_hex_str;
  picosha2::hash256_hex_string(ToString(), hash_hex_str);
  return hash_hex_str;
}