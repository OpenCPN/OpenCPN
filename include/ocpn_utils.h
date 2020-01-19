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
#include <string>
#include <vector>

/** Standard, mostly strings utilities. */

namespace ocpn {

bool endswith(const std::string& s, const std::string& suffix);

bool startswith(const std::string& s, const std::string& prefix);

std::string ltrim(std::string s);

std::string rtrim(std::string s);

std::string trim(std::string s); 

std::string join(std::vector<std::string> v, char c);

bool exists(const std::string& path);

void mkdir(const std::string path);

bool replace(std::string& str, const std::string& from, const std::string& to);

void copy_file(const std::string& src_path, const std::string& dest_path);

}   // namespace ocpn
