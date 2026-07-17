/***************************************************************************
 *   Copyright (C) 2026 OpenCPN Contributors                               *
 ***************************************************************************/

#ifndef CHARTDLDR_SOURCE_CATALOG_H_
#define CHARTDLDR_SOURCE_CATALOG_H_

#include <wx/string.h>

/**
 * Validate a downloaded master source-catalog (chart_sources.xml) file.
 *
 * The source catalog is the predefined tree shown in the "Add source" dialog:
 * a root element containing a `<sections>` subtree of `<section>`/`<catalog>`
 * entries. A publishable catalog must parse as XML and expose at least one
 * `<catalog>` with a non-empty `<name>` and `<location>`.
 *
 * This exists to gate the master-catalog "Update" button: an HTTP 200 that
 * actually returns an error page, a truncated transfer, or an empty
 * `<ChartCatalog/>` must never replace a good on-disk catalog.
 */
bool ChartDldrSourceCatalogXmlIsPublishable(const wxString& path);

#endif  // CHARTDLDR_SOURCE_CATALOG_H_
