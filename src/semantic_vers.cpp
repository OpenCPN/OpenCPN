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

SemanticVersion SemanticVersion::parse(std::string s)
{
    using namespace std;

    SemanticVersion vers;
    size_t pos = s.find('+');
    if (pos != string::npos) {
        vers.build = s.substr(pos + 1);
        s = s.substr(0, pos);
    }
    pos = s.find('-');
    if (pos != string::npos) {
        vers.pre = s.substr(pos + 1);
        s = s.substr(0, pos);
    }
    int r = sscanf(s.c_str(), "%d.%d.%d.%d",
                   &vers.major, &vers.minor, &vers.patch, &vers.post);
    if (r < 2) {
        vers.major = -1;
    }
    return vers;
}


SemanticVersion::SemanticVersion()
    :major(0), minor(0), patch(0), post(0), pre(""), build("")
{}


SemanticVersion::SemanticVersion(int major, int minor, int patch, int post,
                                 std::string pre, std::string build)
{
    this->major = major;
    this->minor = minor;
    this->patch = patch;
    this->post = post;
    this->pre = pre;
    this->build = build;
}

bool SemanticVersion::operator < (const SemanticVersion& other)
{
    if (major != other.major) return major < other.major;
    if (minor != other.minor) return minor < other.minor;
    if (patch != other.patch) return patch < other.patch;
    if (post != other.post) return post < other.post;
    return pre < other.pre;
}

bool SemanticVersion::operator == (const SemanticVersion& other)
{
    return major == other.major
        && minor == other.minor
        && patch == other.patch
        && post == other.post
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
    s << v.major << '.' << v.minor;
    if (v.patch != -1) {
        s << '.' << v.patch;
    }
    if (v.post != 0) {
        s << '.' << v.post;
    }
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
