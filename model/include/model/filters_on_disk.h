/***************************************************************************
 *   Copyright (C) 2025 Alec Leamas                                        *
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
 * Data Monitor filter storage routines. Handles both user defined filters
 * and the readonly system ones shipped with application.
 *
 * All methods uses plain names as arguments and return values. Actual
 * filenames and paths are hidden by the interface.
 */

#include <string>
#include <vector>

#include "model/navmsg_filter.h"

namespace filters_on_disk {

/** Return list of filters, possibly including also the system ones. */
std::vector<std::string> List(bool include_system = false);

/**
 *  Return true iff filter with given name exists, either system or user
 *  defined.
 */
bool Exists(const std::string& name);

/** Remove a filter, return ok if no errors. */
bool Remove(const std::string& name);

/** Write contents for given filter to disk. */
bool Write(const NavmsgFilter& filter, const std::string& name);

/** Read filter with given name from disk. */
NavmsgFilter Read(const std::string& name);

/** Rename old_name on disk to new. */
bool Rename(const std::string& old_name, const std::string& new_name);

}  // namespace filters_on_disk
