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

#ifndef SEMANTIC_VERSION_H_GUARD
#define SEMANTIC_VERSION_H_GUARD

#include "config.h"
#include <sstream>

#undef major                // walk around gnu's major() and minor() macros.
#undef minor


/**
 * Versions are simplified semantic versioning: major.minor.revision-tag+build
 * for example 1.2.6+1-alfa+deadbee. The values major, minor and revision should
 * be integers. The tag is a pre-release marker, the build is build info.
 *
 * Parsing and comparing follows the spec besides the pre-release tag which 
 * is sorted strictly lexically (no dotted parts or numeric ordering support).
 *
 * See: https://semver.org/
 */
struct SemanticVersion
{
    int major;
    int minor;
    int patch;
    std::string pre;    // Pre-release tag like alfa
    std::string build;  // Build info

    /** Construct a "0.0.0" version. */ 
    SemanticVersion();

    /** Parse a version string, sets major == -1 on errors. */
    SemanticVersion(std::string version_release);

    SemanticVersion(int major, int minor, int rev = 0,
                    std::string pre = "",  std::string build = "");

    bool operator <  (const SemanticVersion& other);
    bool operator == (const SemanticVersion& other);
    bool operator >  (const SemanticVersion& other);
    bool operator <= (const SemanticVersion& other);
    bool operator >= (const SemanticVersion& other);
    bool operator != (const SemanticVersion& other);

    /** Return printable representation. */
    std::string to_string();
};

/** Dump version string. */
std::ostream& operator << (std::ostream& s, const SemanticVersion& v);


#endif  // SEMANTIC_VERSION_H_GUARD
