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

#include <string>
#include <vector>

/**
 *  Datatypes and  methods to parse ocpn-plugins.xml XML data,
 *  either complete catalog or a single plugin.
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
  std::string git_commit;
  std::string git_date;
  std::string source;
  std::string tarball_url;
  std::string target;
  std::string build_gtk;
  std::string target_version;
  std::string target_arch;
  std::string info_url;
  std::string meta_url;
  std::string checksum;
  bool is_imported;
  bool is_orphan;

  bool openSource;

  bool readonly;  ///< Can plugin be removed?
  int ix;         ///< Index in list of installed or available.
  void clear() { *this = PluginMetadata(); }
  std::string key() const {
    return name + version + release + target + target_version;
  }

  std::string to_string(); ///< Return printable XML representation.

  PluginMetadata()
      : is_imported(false), is_orphan(false), openSource(true),
        readonly(true), ix(-1) {}
};

/**
 * The result from parsing the xml catalog i. e., ocpn-plugins.xml.
 */
struct CatalogCtx {
  // list of plugins parsed
  std::vector<PluginMetadata> plugins;

  // list meta-urls found when parsing last plugin.
  std::vector<std::string> meta_urls;
  std::vector<std::string> parsed_metas;

  std::string version;
  std::string date;

  // Internal data used while parsing, undefined on exit.
  std::string buff;
  std::string meta_url;
  int depth;
  CatalogCtx() : depth(0) {}
};

bool ParseCatalog(const std::string xml, CatalogCtx* ctx);

bool ParsePlugin(const std::string& xml, PluginMetadata& metadata);

#endif  // CATALOG_PARSER_H__
