/***************************************************************************
 *   Copyright (C) 2014  ALec Leamas                                       *
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
 ***************************************************************************/

/**
 * \file
 *
 * Implement ds_porttype.h -- port type definition and support
 */

#include <cassert>

#include "model/ds_porttype.h"

#include <unordered_map>

std::string PortDirectionToString(PortDirection pd) {
  static const std::unordered_map<PortDirection, std::string> kNameByDirection =
      {{PortDirection::kOutput, "OUT"},
       {PortDirection::kInput, "IN"},
       {PortDirection::kInOut, "IN/OUT"},
       {PortDirection::kUpload, "UPLOAD"}};
  if (static_cast<size_t>(pd) >= kNameByDirection.size()) return "???";
  return kNameByDirection.at(pd);
}
