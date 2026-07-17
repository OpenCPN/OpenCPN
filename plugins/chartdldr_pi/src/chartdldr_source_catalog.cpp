/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#include "chartdldr_source_catalog.h"

#include "pugixml.hpp"

#include <cstring>

namespace {

bool ChildTextNonEmpty(const pugi::xml_node& node, const char* child_name) {
  const pugi::xml_node child = node.child(child_name);
  if (!child) {
    return false;
  }
  const char* text = child.child_value();
  return text != nullptr && text[0] != '\0';
}

/** A usable predefined source needs a display name and a fetch location. */
bool HasPublishableCatalog(const pugi::xml_node& node) {
  for (pugi::xml_node element = node.first_child(); element;
       element = element.next_sibling()) {
    if (!std::strcmp(element.name(), "catalog")) {
      if (ChildTextNonEmpty(element, "name") &&
          ChildTextNonEmpty(element, "location")) {
        return true;
      }
    }
    if (HasPublishableCatalog(element)) {
      return true;
    }
  }
  return false;
}

}  // namespace

bool ChartDldrSourceCatalogXmlIsPublishable(const wxString& path) {
  if (path.IsEmpty()) {
    return false;
  }

  pugi::xml_document doc;
  const pugi::xml_parse_result result = doc.load_file(path.mb_str());
  if (!result) {
    return false;
  }

  const pugi::xml_node root = doc.first_child();
  if (!root) {
    return false;
  }

  // The predefined tree hangs off a <sections> element; require it so a
  // structurally valid but unrelated XML document cannot be published.
  bool has_sections = false;
  for (pugi::xml_node element = root.first_child(); element;
       element = element.next_sibling()) {
    if (!std::strcmp(element.name(), "sections")) {
      has_sections = true;
      break;
    }
  }
  if (!has_sections) {
    return false;
  }

  return HasPublishableCatalog(root);
}
