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
#include <algorithm>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string.h>
#include <sys/stat.h>

#ifdef __MSVC__
#include <io.h>
#include <stdlib.h>
#else
#include <unistd.h>
#endif

#include "ocpn_utils.h"

namespace ocpn {

bool endswith(const std::string& str, const std::string& suffix)
{
    return str.size() >= suffix.size()
        && 0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix);
}


bool startswith(const std::string& str, const std::string& prefix)
{
    return prefix.size() <= str.size()
        && strncmp(str.c_str(), prefix.c_str(), prefix.size()) == 0;
}


/** split s on first occurrence of delim, or return s in first result. */
static std::vector<std::string> split(const std::string& s, const std::string& delim)
{
    std::vector<std::string> result;
    size_t pos = s.find(delim);
    if (pos == std::string::npos) {
        result.push_back(s);
        return result;
    }
    result.push_back(s.substr(0, pos));
    result.push_back(s.substr(pos + delim.length()));
    return result;
}


bool exists(const std::string& name)
{
#ifdef __MSVC__
    return (_access(name.c_str(), 0) != -1);
#else
    return ( access( name.c_str(), F_OK ) != -1 );
#endif
}


void mkdir(const std::string path)
{
#if defined(_WIN32) && !defined(__MINGW32__)
    mkdir(path.c_str());
#elif defined(__MINGW32__)
    ::mkdir(path.c_str());
#else
    ::mkdir(path.c_str(), 0755);
#endif
}


std::string ltrim(std::string s) {
    using namespace std;

    s.erase(s.begin(),
            find_if(s.begin(), s.end(),
                    [](int ch) { return !isspace(ch); }));
    return s;
}


std::string rtrim(std::string s) {
    using namespace std;

    s.erase(find_if(s.rbegin(), s.rend(),
                    [](int ch) { return !isspace(ch); }).base(),
            s.end());
    return s;
}


std::string trim(std::string s) {
    s = ltrim(s);
    s = rtrim(s);
    return s;
}


std::string join(std::vector<std::string> v, char c)
{
    std::string s;
    for (auto p = v.begin(); p != v.end(); p++) {
        s += *p;
        if (p != v.end() - 1) {
            s += c;
        }
    }
    return s;
}

bool replace(std::string& str, const std::string& from, const std::string& to)
{
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}


void copy_file(const std::string& src_path, const std::string& dest_path)
{
    std::ifstream source(src_path, std::ios::binary);
    std::ofstream dest(dest_path, std::ios::binary);

    dest << source.rdbuf();

    source.close();
    dest.close();
}

}  // namespace ocpn
