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
#ifndef CATALOG_PARSER_H__
#define CATALOG_PARSER_H__

#include <memory>
#include <string>
#include <vector>

#include <expat.h>

#if defined(__MINGW32__) && defined(Yield)
#undef Yield                 // from win.h, conflicts with mingw headers
#endif

#include "ocpn_utils.h"

#ifdef XML_LARGE_SIZE
#  if defined(XML_USE_MSC_EXTENSIONS) && _MSC_VER < 1400
#    define XML_FMT_INT_MOD "I64"
#  else
#    define XML_FMT_INT_MOD "ll"
#  endif
#else
#  define XML_FMT_INT_MOD "l"
#endif

#ifdef XML_UNICODE_WCHAR_T
# include <wchar.h>
# define XML_FMT_STR "ls"
#else
# define XML_FMT_STR "s"
#endif

struct PluginMetadata {
    std::string name;
    std::string version;
    std::string release;
    std::string summary;

    std::string api_version;
    std::string author;
    std::string description;
    std::string git_commit ;
    std::string git_date;
    std::string source;
    std::string tarball_url;
    std::string target;
    std::string target_version;
    std::string info_url;

    bool openSource;

    bool readonly;                // Can plugin be removed?
    int ix;                       // Index in list of installed or available.
    void clear() { *this = PluginMetadata(); }
    PluginMetadata() :  readonly(true), ix(-1) {}
};


/**
 * The result from parsing the xml catalog i. e., ocpn-plugins.xml.
 */
struct catalog_ctx {
    std::vector<PluginMetadata> plugins;
    std::string version;
    std::string date;

    // Internal data used while parsing, undefined on exit.
    std::unique_ptr<PluginMetadata> plugin;    
    std::string buff;
    int depth;
};


bool ParseCatalog(const std::string xml, catalog_ctx& ctx);

#endif  // CATALOG_PARSER_H__
