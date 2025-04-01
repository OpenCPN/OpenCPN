/**************************************************************************
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
 **************************************************************************/

/**
 * \file
 * Miscellaneous utilities, many of which string related.
 */

#ifndef _OCPN_UTILS_H__
#define _OCPN_UTILS_H__

#include <string>
#include <vector>

/** Standard, mostly strings utilities. */

namespace ocpn {

/** Return true if s ends with given suffix */
bool endswith(const std::string& s, const std::string& suffix);

/** Return true if s starts with given prefix. */
bool startswith(const std::string& s, const std::string& prefix);

/**
 * Strip possibly leading space characters in s
 * @return new string with leading  non-printable characters removed
 */
std::string ltrim(const std::string& s);

/**
 * Strip possibly trailing  space characters in s
 * @return string with trailing non-printable characters removed
 */
std::string rtrim(const std::string& s);

/**
 * Strip possibly trailing  and/or leading space characters in s
 * @return string with trailing and leading non-printable characters removed
 */
std::string trim(std::string s);

/**
 * Return a single string being the concatenation of all elements in v with
 * character c in between.
 */
std::string join(std::vector<std::string> v, char c);

/** Return copy of s with all characters converted to lower case. */
std::string tolower(const std::string& s);

/** Return vector of items in s separated by delimiter*/
std::vector<std::string> split(const char* s, const std::string& delimiter);

/** Return vector of items in s separated by delimiter*/
std::vector<std::string> split(const std::string& s,
                               const std::string& delimiter);

/** @deprecated Using std::filesystem instead. */
bool exists(const std::string& path);

/** @deprecated Using std::filesystem instead. */
void mkdir(const std::string path);

/**
 * Perform in place substitution in str, replacing "from" with "to"
 * @return true if any replacement is done.
 */
bool replace(std::string& str, const std::string& from, const std::string& to);

/** Copy file contents in path src_path to dest_path. */
void copy_file(const std::string& src_path, const std::string& dest_path);

/**
 * Check if checksum in a NMEA0183 sentence is correct
 * @param sentence complete NMEA01832 message
 * @return true if checksum is OK, else false.
 */
bool N0183CheckSumOk(const std::string& sentence);

/** Return copy of str with non-printable chars replaced by hex like "<0D>" */
std::string printable(const std::string& str);

}  // namespace ocpn

#endif  //  _OCPN_UTILS_H__
