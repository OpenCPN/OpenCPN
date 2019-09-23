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


#include "config.h"

#include <algorithm>
#include <cstdio>
#include <string>
#include <sstream>

#include "semantic_vers.h"

#undef major                // walk around gnu's major() and minor() macros.
#undef minor

SemanticVersion::SemanticVersion()
    :major(0), minor(0), patch(0), pre(""), build("")
{}


SemanticVersion::SemanticVersion(std::string s)
    :SemanticVersion()
{
    using namespace std;
    size_t pos = s.find('+');
    if (pos != string::npos) {
        build = s.substr(pos + 1);
        s = s.substr(0, pos);
    }
    pos = s.find('-');
    if (pos != string::npos) {
        pre = s.substr(pos + 1);
        s = s.substr(0, pos);
    }
    int r = sscanf(s.c_str(), "%d.%d.%d", &major, &minor, &patch);
    if (r < 2) {
        major = -1;
    }
}


SemanticVersion::SemanticVersion(
    int major, int minor, int patch, std::string pre, std::string build)
{
    this->major = major;
    this->minor = minor;
    this->patch = patch;
    this->pre = pre;
    this->build = build;
}

bool SemanticVersion::operator < (const SemanticVersion& other)
{
    if (major < other.major) return true;
    if (major > other.major) return false;
    if (minor < other.minor) return true;
    if (minor > other.minor) return false;
    if (patch < other.patch) return true;
    if (patch > other.patch) return false;
    int len = std::min(pre.length(), other.pre.length());
    if (pre.substr(0, len) != other.pre.substr(0, len)) {
        return pre.substr(0, len) < other.pre.substr(0, len);
    }
    return pre.length() > other.pre.length();
}

bool SemanticVersion::operator == (const SemanticVersion& other)
{
    return major == other.major
        && minor == other.minor
        && patch == other.patch
        && pre == other.pre;
}

bool SemanticVersion::operator > (const SemanticVersion& other)
{
    return !(*this == other) && !(*this < other);
}

bool SemanticVersion::operator <= (const SemanticVersion& other)
{
    return (*this == other) || (*this < other);
}

bool SemanticVersion::operator >= (const SemanticVersion& other)
{
    return (*this == other) || (*this > other);
}

bool SemanticVersion::operator != (const SemanticVersion& other)
{
    return !(*this == other);
}

std::ostream& operator << (std::ostream& s, const SemanticVersion& v)
{
    s << v.major << '.' << v.minor << '.' << v.patch;
    if (v.pre != "" ) { 
        s << '-' << v.pre;
    }
    if (v.build != "" ) { 
        s << '+' << v.build;
    }
    return s;
}

std::string SemanticVersion::to_string()
{
    std::ostringstream os;
    os << *this;
    return os.str();
}
