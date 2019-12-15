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

/**
 *  Datatypes and a single method to parse ocpn-plugins.xml XML data.
 */

/** Overall metadata for the set of plugins used. */
struct CatalogData {
    std::string version;
    std::string date;
    bool undef;
    CatalogData() : undef(true) {}
};


/** Plugin metadata, reflects the xml format directly. */
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
    catalog_ctx(): depth(0) {}
};


bool ParseCatalog(const std::string xml, catalog_ctx* ctx);

#endif  // CATALOG_PARSER_H__
