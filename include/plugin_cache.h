/******************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2019 Alec Leamas                                        *
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

#ifndef PLUGIN_CACHE_H__
#define PLUGIN_CACHE_H__

#include <string>

namespace ocpn {

std::string get_basename(const char* path);

/** Store metadata in metadata cache, return success/fail. */
bool store_metadata(const char* path);

/**
 * Get metadata path for a given name defaulting to ocpn-plugins.xml)
 * @return Path to cached metadata or "" if not found
 */
std::string lookup_metadata(const char* name = 0);

/** Store a tarball in tarball cache, return success/fail. */
bool store_tarball(const char* path, const char* basename);

/**
 * Get path to tarball in cache for given filename
 * @return Path to cached metadata or "" if not found
 */
std::string lookup_tarball(const char* basename);

/** Return number of files in cache */
unsigned cache_file_count();

/** Return total size of files in cache in kbytes. */
unsigned long cache_size();

/** Remove all files in cache: */
void cache_clear();

}  // namespace ocpn
#endif
