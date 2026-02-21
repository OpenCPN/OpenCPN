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
 ***************************************************************************/

/**
 * \file
 *
 * Updates install and optional selection dialog.
 */

#include "config.h"
#include "gl_headers.h"  // Must be included before anything using GL stuff

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
#include <wx/textwrapper.h>

#include "update_mgr.h"

#include "model/downloader.h"
#include "model/plugin_handler.h"
#include "model/plugin_loader.h"
#include "model/semantic_vers.h"
#include "model/svg_utils.h"

#include "catalog_mgr.h"
#include "expand_icon.h"
#include "ocpn_platform.h"
#include "options.h"
#include "pluginmanager.h"
#include "styles.h"

#ifdef __ANDROID__
#include "androidUTIL.h"
#endif

#undef major  // work around gnu's major() and minor() macros.
#undef minor

class HardBreakWrapper : public wxTextWrapper {
public:
  HardBreakWrapper(wxWindow* win, const wxString& text, int widthMax) {
    m_lineCount = 0;
    Wrap(win, text, widthMax);
  }
  wxString const& GetWrapped() const { return m_wrapped; }
  int const GetLineCount() const { return m_lineCount; }

protected:
  virtual void OnOutputLine(const wxString& line) { m_wrapped += line; }
  virtual void OnNewLine() {
    m_wrapped += '\n';
    m_lineCount++;
  }

private:
  wxString m_wrapped;
  int m_lineCount;
};

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

/** Return PlugInContainer with given name or 0 if not found. */
static PlugInContainer* PlugInByName(const std::string name,
                                     const ArrayOfPlugIns* plugins) {
  auto ix = PlugInIxByName(name, plugins);
  return ix == -1 ? 0 : plugins->Item(ix);
}

/**
 * A plugin icon, scaled to about 2/3 of available space
 *
 * Load icons from .../uidata/plugins, on the form plugin.svg or
 * plugin.png. If neither exists, display a default  icon.
 */
class PluginIconPanel : public wxPanel {
public:
  PluginIconPanel(wxWindow* parent, std::string plugin_name)
      : wxPanel(parent), m_plugin_name(plugin_name) {
    auto size = GetClientSize();
    auto minsize = GetTextExtent("OpenCPN");
    SetMinClientSize(wxSize(minsize.GetWidth(), size.GetHeight()));
    Layout();
    Bind(wxEVT_PAINT, &PluginIconPanel::OnPaint, this);
  }

  void OnPaint(wxPaintEvent&) {
    auto size = GetClientSize();
    int minsize = wxMin(size.GetHeight(), size.GetWidth());
    auto offset = minsize / 10;

    LoadIcon("packageBox.svg", m_bitmap, 2 * minsize / 3);
    wxPaintDC dc(this);
    if (!m_bitmap.IsOk()) {
      wxLogMessage("AddPluginPanel: bitmap is not OK!");
      return;
    }
    dc.DrawBitmap(m_bitmap, offset, offset, true);
  }

protected:
  wxBitmap m_bitmap;
  const std::string m_plugin_name;

  void LoadIcon(const char* plugin_name, wxBitmap& bitmap, int size = 32) {
    wxFileName path(g_Platform->GetSharedDataDir(), plugin_name);
    path.AppendDir("uidata");
    path.AppendDir("traditional");
    bool ok = false;

    if (path.IsFileReadable()) {
      bitmap = LoadSVG(path.GetFullPath(), size, size);
      ok = bitmap.IsOk();
    }

    if (!ok) {
      auto style = g_StyleManager->GetCurrentStyle();
      bitmap = wxBitmap(style->GetIcon("default_pi", size, size));
      wxLogMessage("Icon: %s not found.", path.GetFullPath());
    }
  }
};

/** Download and install a PluginMetadata item when clicked. */
class InstallButton : public wxPanel {
public:
  InstallButton(wxWindow* parent, PluginMetadata metadata)
      : wxPanel(parent), m_metadata(metadata) {
    auto loader = PluginLoader::GetInstance();
    PlugInContainer* found =
        PlugInByName(metadata.name, loader->GetPlugInArray());
    std::string label(_("Install"));
    if (found &&
        ((found->m_version_major > 0) || (found->m_version_minor > 0))) {
      label = getUpdateLabel(found, metadata);
    }
    auto button = new wxButton(this, wxID_ANY, label);
    auto box = new wxBoxSizer(wxHORIZONTAL);
    box->Add(button);
    SetSizer(box);
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &InstallButton::OnClick, this);
  }

  void OnClick(wxCommandEvent&) {
    wxLogMessage("Selected update: %s", m_metadata.name.c_str());
    auto top_parent = GetParent()->GetParent()->GetParent();
    auto dialog = dynamic_cast<UpdateDialog*>(top_parent);
    wxASSERT(dialog != 0);
    dialog->SetUpdate(m_metadata);
    dialog->EndModal(wxID_OK);
  }

private:
  PluginMetadata m_metadata;

  const char* getUpdateLabel(PlugInContainer* pic, PluginMetadata metadata) {
    SemanticVersion currentVersion(pic->m_version_major, pic->m_version_minor);
    if (pic->m_version_str != "") {
      currentVersion = SemanticVersion::parse(pic->m_version_str.ToStdString());
    }
    auto newVersion = SemanticVersion::parse(metadata.version);
    if (newVersion > currentVersion) {
      return _("Update");
    } else if (newVersion == currentVersion) {
      return _("Reinstall");
    } else {
      return _("Downgrade");
    }
  }
};

/** Invokes client browser on plugin info_url when clicked. */
class UpdateWebsiteButton : public wxPanel {
public:
  UpdateWebsiteButton(wxWindow* parent, const char* url)
      : wxPanel(parent), m_url(url) {
    auto vbox = new wxBoxSizer(wxVERTICAL);
    auto button = new wxButton(this, wxID_ANY, _("Website"));
    button->Enable(strlen(url) > 0);
    vbox->Add(button);
    SetSizer(vbox);
    Bind(wxEVT_COMMAND_BUTTON_CLICKED,
         [=](wxCommandEvent&) { wxLaunchDefaultBrowser(m_url); });
  }

protected:
  const std::string m_url;
};

/** The two buttons 'install' and 'website', the latter optionally hidden. */
class CandidateButtonsPanel : public wxPanel {
public:
  CandidateButtonsPanel(wxWindow* parent, const PluginMetadata* plugin)
      : wxPanel(parent) {
    auto flags = wxSizerFlags().Border();

    auto vbox = new wxBoxSizer(wxVERTICAL);
    vbox->Add(new InstallButton(this, *plugin),
              flags.DoubleBorder().Top().Right());
    vbox->Add(1, 1, 1, wxEXPAND);  // Expanding, stretchable spacer
    m_info_btn = new UpdateWebsiteButton(this, plugin->info_url.c_str());
    m_info_btn->Hide();
    vbox->Add(m_info_btn, flags.DoubleBorder().Right());
    SetSizer(vbox);
    Fit();
  }

  void HideDetails(bool hide) {
    m_info_btn->Show(!hide);
    GetParent()->Layout();
  }

private:
  UpdateWebsiteButton* m_info_btn;
};

/** Plugin name, version, summary + an optionally shown description. */
class PluginTextPanel : public wxPanel {
public:
  PluginTextPanel(wxWindow* parent, const PluginMetadata* plugin,
                  CandidateButtonsPanel* buttons, bool bshowTuple = false)
      : wxPanel(parent),
        m_isDesc(false),
        m_descr(0),
        m_more(new ExpandableIcon(this,
                                  [&](bool collapsed) { OnClick(collapsed); })),
        m_buttons(buttons) {
    auto flags = wxSizerFlags().Border();
    auto sum_hbox = new wxBoxSizer(wxHORIZONTAL);
    m_widthDescription = g_options->GetSize().x * 4 / 10;

    // m_summary = staticText(plugin->summary);
    m_summary = new wxStaticText(
        this, wxID_ANY, "", wxDefaultPosition,
        wxSize(m_widthDescription, -1) /*, wxST_NO_AUTORESIZE*/);
    m_summaryText = wxString(plugin->summary.c_str());
    m_summary->SetLabel(m_summaryText);
    m_summary->Wrap(m_widthDescription);

    HardBreakWrapper wrapper(this, m_summaryText, m_widthDescription);

    sum_hbox->Add(m_summary);
    sum_hbox->AddSpacer(10);
    sum_hbox->Add(m_more, wxSizerFlags());

    auto vbox = new wxBoxSizer(wxVERTICAL);
    SetSizer(vbox);

    std::string name_reduced = plugin->name;
    if (plugin->name.size() * GetCharWidth() >
        (size_t)m_widthDescription * 7 / 10) {
      int nc = (m_widthDescription * 7 / 10) / GetCharWidth();
      if (nc > 3) {
        name_reduced = plugin->name.substr(0, nc - 3) + "...";
      }
    }

    wxString nameText(name_reduced + "  " + plugin->version);
    if (bshowTuple) nameText += "   " + plugin->target;

    auto name = staticText(nameText);

    m_descr = new wxStaticText(
        this, wxID_ANY, "", wxDefaultPosition,
        wxSize(m_widthDescription, -1) /*, wxST_NO_AUTORESIZE*/);
    m_descText = wxString(plugin->description.c_str());
    m_descr->SetLabel(m_descText);
    m_descr->Wrap(m_widthDescription);
    m_descr->Hide();
    vbox->Add(name, flags);
    vbox->Add(sum_hbox, flags);
    vbox->Add(m_descr, 0);
    Fit();
  }

  void OnClick() {
    m_descr->SetLabel(m_descText);
    m_descr->Wrap(m_widthDescription);
    GetParent()->Layout();
    m_buttons->HideDetails(!m_descr->IsShown());

    auto swin = dynamic_cast<UpdateDialog*>(GetGrandParent());
    if (swin) {
      swin->RecalculateSize();
    }
  }

  void OnClick(wxMouseEvent&) {
    m_descr->Show(!m_descr->IsShown());
    OnClick();
  }

  void OnClick(bool collapsed) {
    m_descr->Show(!collapsed);
    m_isDesc = !collapsed;
    OnClick();
  }
  bool m_isDesc;

protected:
  wxStaticText* staticText(const wxString& text) {
    return new wxStaticText(this, wxID_ANY, text, wxDefaultPosition,
                            wxDefaultSize, wxALIGN_LEFT);
  }

  wxStaticText* m_descr;
  ExpandableIcon* m_more;
  wxStaticText* m_summary;
  CandidateButtonsPanel* m_buttons;
  int m_widthDescription;
  wxString m_descText;
  wxString m_summaryText;
};

/**
 * The list of download candidates in a scrolled window + OK and
 * Settings  button.
 */
class OcpnUpdateScrolledWindow : public wxScrolledWindow {
public:
  OcpnUpdateScrolledWindow(wxWindow* parent,
                           const std::vector<PluginMetadata>& updates)
      : wxScrolledWindow(parent),
        m_updates(updates),
        m_grid(new wxFlexGridSizer(3, 0, 0)) {
    auto box = new wxBoxSizer(wxVERTICAL);
    populateGrid(m_grid);
    box->Add(m_grid, wxSizerFlags().Proportion(0).Expand());
    auto butt_box = new wxBoxSizer(wxHORIZONTAL);
    auto cancel_btn = new wxButton(this, wxID_CANCEL, _("Dismiss"));
    butt_box->Add(1, 1, 1, wxEXPAND);  // Expanding, stretchable spacer
    butt_box->Add(cancel_btn, wxSizerFlags().Border());
    box->Add(butt_box, wxSizerFlags().Proportion(0).Expand());

    SetSizer(box);
    SetMinSize(GetEffectiveMinSize());
    SetScrollRate(1, 1);
    SetAutoLayout(true);
  };

  void populateGrid(wxFlexGridSizer* grid) {
    auto flags = wxSizerFlags();
    grid->SetCols(3);
    grid->AddGrowableCol(2);
    for (auto plugin : m_updates) {
      grid->Add(new PluginIconPanel(this, plugin.name), flags.Expand());
      auto buttons = new CandidateButtonsPanel(this, &plugin);
      bool b_show_tuple = false;
      if (g_Platform->getDisplaySize().x > 80 * GetCharWidth())
        b_show_tuple = m_updates.size() > 1;
      PluginTextPanel* tpanel =
          new PluginTextPanel(this, &plugin, buttons, b_show_tuple);
      tpanel->m_isDesc = true;
      grid->Add(tpanel, flags.Proportion(1).Right());
      grid->Add(buttons, flags.DoubleBorder());
      grid->Add(new wxStaticLine(this), wxSizerFlags(0).Expand());
      grid->Add(new wxStaticLine(this), wxSizerFlags(0).Expand());
      grid->Add(new wxStaticLine(this), wxSizerFlags(0).Expand());
    }
  }

private:
  const std::vector<PluginMetadata> m_updates;
  wxFlexGridSizer* m_grid;
};

/** Top-level install plugins dialog. */
UpdateDialog::UpdateDialog(wxWindow* parent,
                           const std::vector<PluginMetadata>& updates)
    : wxDialog(parent, wxID_ANY, _("Plugin Manager"), wxDefaultPosition,
               wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER) {
  auto vbox = new wxBoxSizer(wxVERTICAL);
  SetSizer(vbox);

  m_scrwin = new OcpnUpdateScrolledWindow(this, updates);
  vbox->Add(m_scrwin, wxSizerFlags(1).Expand());

  RecalculateSize();

  Center();
#ifdef __ANDROID__
  androidDisableRotation();
#endif
}

UpdateDialog::~UpdateDialog() {
#ifdef __ANDROID__
  androidEnableRotation();
#endif
}

void UpdateDialog::RecalculateSize() {
  m_scrwin->SetMinClientSize(m_scrwin->GetSizer()->GetMinSize());
#ifdef __ANDROID__
  SetMinSize(g_Platform->getDisplaySize());
#endif
  SetMaxSize(g_Platform->getDisplaySize());
  Fit();
}
