/***************************************************************************
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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 ***************************************************************************/

/**
 * \file
 *
 * Catalog options dialog, by default disabled.
 */

#ifndef CATALOG_MGR_H__
#define CATALOG_MGR_H__

#include <wx/dialog.h>
#include <wx/frame.h>

/** Catalog options GUI.  */
class AdvancedCatalogDialog : public wxFrame {
public:
  /**
   * Invoke the advanced catalog dialog after a status check.
   */
  AdvancedCatalogDialog(wxWindow* parent);
};

class SimpleCatalogDialog : public wxDialog {
public:
  /**
   * Perform a simple catalog update without options.
   */
  SimpleCatalogDialog(wxWindow* parent);
};

#endif  // CATALOG_MGR_H__
