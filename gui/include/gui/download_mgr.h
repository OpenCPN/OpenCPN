/**************************************************************************
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
 **************************************************************************/

/**
 * \file
 *
 * Generic GUI downloads tool
 */

#ifndef DOWNLOAD_MGR_H__
#define DOWNLOAD_MGR_H__

#include <wx/dialog.h>
#include <wx/progdlg.h>
#include <wx/window.h>

#include "model/downloader.h"
#include "model/catalog_parser.h"

// Accepted by PluginDownloadDialog, reloads plugin list.
wxDECLARE_EVENT(EVT_PLUGINS_RELOAD, wxCommandEvent);

/**
 * Dialog for downloading plugins.
 */
class PluginDownloadDialog : public wxDialog {
public:
  PluginDownloadDialog(wxWindow* parent);

  wxWindow* GetRealParent() { return m_parent; }
};

/**
 * Downloader with progress and final message dialogs.
 */
class GuiDownloader : public Downloader {
private:
  long m_downloaded;
  wxProgressDialog* m_dialog;
  PluginMetadata m_plugin;
  wxWindow* m_parent;

public:
  GuiDownloader(wxWindow* parent, PluginMetadata plugin);
  std::string run(wxWindow* parent, bool remove_current);
  void on_chunk(const char* buff, unsigned bytes) override;
  void showErrorDialog(const char* msg);
};

#endif  // DOWNLOAD_MGR_H__
