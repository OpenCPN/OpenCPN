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

#include <sstream>

#include <wx/log.h>

#if defined(__MINGW32__) && defined(Yield)
#undef Yield  // from win.h, conflicts with mingw headers
#endif

#include "model/catalog_parser.h"
#include "model/ocpn_utils.h"
#include "model/catalog_handler.h"
#include "pugixml.hpp"

static void add_node(pugi::xml_node root, const std::string& name,
                     const std::string& value) {
  auto child = root.append_child(name.c_str());
  child.append_child(pugi::node_pcdata).set_value(value.c_str());
}

std::string PluginMetadata::to_string() {
  pugi::xml_document doc;
  auto root = doc.append_child("plugin");
  root.append_attribute("version").set_value("1");

  add_node(root, "name", name);
  add_node(root, "version", version);
  add_node(root, "release", release);
  add_node(root, "summary", summary);
  add_node(root, "api-version", api_version);
  add_node(root, "open-source", openSource ? "true" : "false");
  add_node(root, "is-imported", is_imported ? "true" : "false");
  add_node(root, "author", author);
  add_node(root, "source", author);
  add_node(root, "info-url", info_url);
  add_node(root, "description", description);
  add_node(root, "target", target);
  add_node(root, "target-version", target_version);
  add_node(root, "target-arch", target_arch);
  add_node(root, "tarball-url", tarball_url);
  add_node(root, "tarball-checksum", checksum);

  std::ostringstream ss;
  doc.save(ss, "    ");
  return ss.str();
}

static void ParseValue(pugi::xml_node node, const std::string& name,
                       std::string& value) {
  auto child = node.child(name.c_str());
  if (child) value = ocpn::trim(child.first_child().value());
}

static void ParseBool(pugi::xml_node node, const std::string& name,
                      bool& value) {
  auto child = node.child(name.c_str());
  if (child) {
    auto text = ocpn::trim(child.first_child().value());
    value = text == "yes" || text == "true";
  }
}

bool ParsePlugin(pugi::xml_node root, PluginMetadata& plugin) {
  ParseValue(root, "name", plugin.name);
  ParseValue(root, "version", plugin.version);
  ParseValue(root, "release", plugin.release);
  ParseValue(root, "summary", plugin.summary);
  ParseValue(root, "api-version", plugin.api_version);
  ParseValue(root, "author", plugin.author);
  ParseValue(root, "description", plugin.description);
  ParseValue(root, "source", plugin.source);
  ParseValue(root, "tarball-url", plugin.tarball_url);
  ParseValue(root, "info-url", plugin.info_url);
  ParseValue(root, "target", plugin.target);
  ParseValue(root, "target-version", plugin.target_version);
  ParseValue(root, "target-arch", plugin.target_arch);
  ParseValue(root, "target-checksum", plugin.checksum);
  ParseBool(root, "open-source", plugin.openSource);
  ParseBool(root, "is-imported", plugin.is_imported);
  return root.child("name") ? true : false;
}

bool ParsePlugin(const std::string& xml, PluginMetadata& plugin) {
  pugi::xml_document doc;
  doc.load_string(xml.c_str());
  auto node = doc.child("plugin");
  if (!node) {
    wxLogWarning("Trying to parse plugin without root node");
    return false;
  }
  return ParsePlugin(doc.child("plugin"), plugin);
}

bool ParseCatalog(const std::string xml, CatalogCtx* ctx) {
  bool ok = true;
  PluginMetadata* plugin = 0;

  pugi::xml_document doc;
  doc.load_string(xml.c_str());

  pugi::xml_node nodes = doc.child("plugins");
  for (pugi::xml_node node = nodes.first_child(); node;
       node = node.next_sibling()) {
    if (!strcmp(node.name(), "version") && ctx->version == "") {
      ctx->version = ocpn::trim(node.first_child().value());
    } else if (strcmp(node.name(), "date") == 0 && ctx->date == "") {
      ctx->date = ocpn::trim(node.first_child().value());
    } else if (strcmp(node.name(), "meta-url") == 0) {
      ctx->meta_url = ocpn::trim(node.first_child().value());
    } else if (!strcmp(node.name(), "plugin")) {
      if (ctx->meta_url != "") {
        ctx->meta_urls.push_back(ctx->meta_url);
        ctx->meta_url = "";
      } else {
        if (plugin) {
          ctx->plugins.push_back(*plugin);
          delete plugin;
        }
        plugin = new PluginMetadata;
      }
      auto meta_url = node.child("meta-url");
      if (meta_url) {
        auto url = ocpn::trim(meta_url.first_child().value());
        ctx->meta_url = url;
      }
      ParsePlugin(node, *plugin);
    }
  }
  // capture last plugin
  if (plugin) {
    ctx->plugins.push_back(*plugin);
    delete plugin;
  } else {
    if (ctx->meta_url != "") {
      ctx->meta_urls.push_back(ctx->meta_url);
      ctx->meta_url = "";
    }
  }
  return true;
}
