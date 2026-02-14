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

#ifndef UPDATE_MGR_H__
#define UPDATE_MGR_H__

#include <vector>

#include <wx/dialog.h>
#include <wx/window.h>

#include "model/catalog_parser.h"

class OcpnUpdateScrolledWindow;

/**
 * Modal dialog, displays available updates (possibly just one) and lets
 * user select and eventually confirm update.
 */
class UpdateDialog : public wxDialog {
public:
  UpdateDialog(wxWindow* parent, const std::vector<PluginMetadata>& updates);
  virtual ~UpdateDialog();

  PluginMetadata GetUpdate() { return update; };
  void SetUpdate(const PluginMetadata md) { update = md; };
  void RecalculateSize();

  OcpnUpdateScrolledWindow* m_scrwin;

private:
  PluginMetadata update;
};

#endif  // UPDATE_MGR_H__
