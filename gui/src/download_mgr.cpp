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
 * Implement download_mgr.h -- generic GUI downloads tool.
 */

#include "gl_headers.h"  // Must come before anything including GL stuff

#include "config.h"
#include "download_mgr.h"

#include <fstream>
#include <set>
#include <sstream>

#include <wx/bitmap.h>
#include <wx/button.h>
#include <wx/debug.h>
#include <wx/file.h>
#include <wx/image.h>
#include <wx/log.h>
#include <wx/panel.h>
#include <wx/progdlg.h>
#include <wx/sizer.h>
#include <wx/statline.h>
#include <wx/uri.h>

#include "model/downloader.h"
#include "model/plugin_cache.h"
#include "model/plugin_handler.h"
#include "model/semantic_vers.h"
#include "model/svg_utils.h"

#include "catalog_mgr.h"
#include "expand_icon.h"
#include "ocpn_platform.h"
#include "picosha2.h"
#include "pluginmanager.h"
#include "styles.h"

#undef major  // Work around gnu's major() and minor() macros.
#undef minor

// Main window reload event
wxDEFINE_EVENT(EVT_PLUGINS_RELOAD, wxCommandEvent);

/**
 * Check if sha256sum of a tarball matches checksum in metadata.
 */
static bool checksum_ok(const std::string& path,
                        const PluginMetadata& metadata) {
  wxLogDebug("Checksum test on %s", metadata.name.c_str());
  if (metadata.checksum == "") {
    wxLogDebug("No metadata checksum, aborting check,");
    return true;
  }
  const size_t pos = metadata.checksum.find(':');
  std::string checksum(metadata.checksum);
  if (pos == std::string::npos) {
    checksum = std::string("sha256:") + checksum;
  }
  std::ifstream f(path, std::ios::binary);
  picosha2::hash256_one_by_one hasher;
  while (!f.eof()) {
    char buff[2048];
    f.read(buff, sizeof(buff));
    const std::string block(buff, f.gcount());
    hasher.process(block.begin(), block.end());
  }
  hasher.finish();
  std::string tarball_hash =
      std::string("sha256:") + picosha2::get_hash_hex_string(hasher);

  if (tarball_hash == checksum) {
    wxLogDebug("Checksum ok: %s", tarball_hash.c_str());
    return true;
  }
  wxLogMessage("Checksum fail on %s, tarball: %s, metadata: %s",
               metadata.name.c_str(), tarball_hash.c_str(), checksum.c_str());
  return false;
}

/**
 * Return index in ArrayOfPlugins for plugin with given name,
 * or -1 if not found.
 */
static ssize_t PlugInIxByName(const std::string name,
                              const ArrayOfPlugIns* plugins) {
  for (unsigned i = 0; i < plugins->GetCount(); i += 1) {
    if (name == plugins->Item(i)->m_common_name.Lower().ToStdString()) {
      return i;
    }
  }
  return -1;
}

/** Add progress and final message dialogs to the basic Downloader. */
GuiDownloader::GuiDownloader(wxWindow* parent, PluginMetadata plugin)
    : Downloader(plugin.tarball_url),
      m_downloaded(0),
      m_dialog(0),
      m_plugin(plugin),
      m_parent(parent) {}

std::string GuiDownloader::run(wxWindow* parent, bool remove_current) {
  bool ok;
  bool downloaded = false;
  std::string path = ocpn::lookup_tarball(m_plugin.tarball_url.c_str());
  if (!path.size()) {
    long size = get_filesize();
    std::string label(_("Downloading "));
    label += url;
    m_dialog =
        new wxProgressDialog(_("Downloading"), label.c_str(), size, parent,
                             wxPD_AUTO_HIDE | wxPD_APP_MODAL | wxPD_CAN_ABORT);
#ifdef __ANDROID__
    m_dialog->SetBackgroundColour(wxColour(0x7c, 0xb0, 0xe9));  // light blue
#endif
    ok = download(path);
    g_Platform->HideBusySpinner();

    if (!ok) {
      delete m_dialog;
      showErrorDialog("Download error");
      return "";
    }

    // Download aborted?
    if (m_dialog == 0) {
      showErrorDialog("Download aborted");
      return "";
    } else {
      delete m_dialog;
    }
    if (!checksum_ok(path, m_plugin)) {
      showErrorDialog("Checksum error");
      return "";
    }
    m_dialog = 0;  // make sure that on_chunk() doesn't misbehave.
    downloaded = true;
  }

  auto pluginHandler = PluginHandler::GetInstance();
  if (remove_current) {
    wxLogMessage("Uninstalling %s", m_plugin.name.c_str());
    pluginHandler->Uninstall(m_plugin.name);
  }
  ok = pluginHandler->InstallPlugin(m_plugin, path);
  if (!ok) {
    showErrorDialog("Installation error");
    return "";
  }

  if (downloaded) {
    // Cache the tarball from the tmp location to the plugin cache.
    wxURI uri(wxString(m_plugin.tarball_url.c_str()));
    wxFileName fn(uri.GetPath());
    auto basename = fn.GetFullName().ToStdString();
    if (ocpn::store_tarball(path.c_str(), basename.c_str())) {
      wxLogMessage("Copied %s to local cache at %s", path.c_str(),
                   basename.c_str());
    }
  }

  wxMessageDialog* dlg = new wxMessageDialog(
      m_parent,
      m_plugin.name + " " + m_plugin.version + _(" successfully installed"),
      _("Installation complete"), wxOK | wxCENTRE | wxICON_INFORMATION);
  dlg->ShowModal();
  return path;
}

void GuiDownloader::on_chunk(const char* buff, unsigned bytes) {
  Downloader::on_chunk(buff, bytes);
  m_downloaded += bytes;
  if (m_dialog && !m_dialog->Update(m_downloaded)) {
    // User pushed Cancel button
    delete m_dialog;
    m_dialog = 0;
  }
}

void GuiDownloader::showErrorDialog(const char* msg) {
  auto dlg = new wxMessageDialog(m_parent, "", _("Installation error"),
                                 wxOK | wxICON_ERROR);
  auto last_error_msg = last_error();
  std::string text = msg;
  if (last_error_msg != "") {
    text = text + ": " + error_msg;
  }
  text = text + "\nPlease check system log for more info.";
  dlg->SetMessage(text);
  dlg->ShowModal();
  dlg->Destroy();
}
