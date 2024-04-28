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

/** Updates install and optional selection dialog. */

#include "config.h"

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

#include "catalog_mgr.h"
#include "update_mgr.h"
#include "model/plugin_loader.h"
#include "model/downloader.h"
#include "OCPNPlatform.h"
#include "model/plugin_handler.h"
#include "pluginmanager.h"
#include "model/semantic_vers.h"
#include "styles.h"
#include "options.h"
#include "svg_utils.h"

#ifdef __ANDROID__
#include "androidUTIL.h"
#endif

extern PlugInManager* g_pi_manager;
extern ocpnStyle::StyleManager* g_StyleManager;
extern OCPNPlatform* g_Platform;
extern options* g_options;

#undef major  // walk around gnu's major() and minor() macros.
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

//    HardBreakWrapper wrapper(win, text, widthMax);
//    return wrapper.GetWrapped();

// namespace update_mgr {

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

/** Load a png icon rescaled to size x size. */
static void LoadPNGIcon(const char* path, int size, wxBitmap& bitmap) {
  wxPNGHandler handler;
  if (!wxImage::FindHandler(handler.GetName())) {
    wxImage::AddHandler(new wxPNGHandler());
  }
  auto img = new wxImage();
  bool ok = img->LoadFile(path, wxBITMAP_TYPE_PNG);
  if (!ok) {
    bitmap = wxBitmap();
    return;
  }
  img->Rescale(size, size);
  bitmap = wxBitmap(*img);
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

  void OnPaint(wxPaintEvent& event) {
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
      bitmap = wxBitmap(style->GetIcon(_T("default_pi"), size, size));
      wxLogMessage("Icon: %s not found.", path.GetFullPath());
    }

    /*
                wxFileName path(g_Platform->GetSharedDataDir(), plugin_name);
                path.AppendDir("uidata");
                bool ok = false;
                path.SetExt("png");
                if (path.IsFileReadable()) {
                    LoadPNGIcon(path.GetFullPath(), size, bitmap);
                    ok = bitmap.IsOk();
                }
                if (!ok) {
                    auto style = g_StyleManager->GetCurrentStyle();
                    bitmap = wxBitmap(style->GetIcon( _T("default_pi")));
                }
    */
  }
};

/** Download and install a PluginMetadata item when clicked. */
class InstallButton : public wxPanel {
public:
  InstallButton(wxWindow* parent, PluginMetadata metadata)
      : wxPanel(parent), m_metadata(metadata), m_remove(false) {
    auto loader = PluginLoader::getInstance();
    PlugInContainer* found =
        PlugInByName(metadata.name, loader->GetPlugInArray());
    std::string label(_("Install"));
    if (found &&
        (found->m_version_major > 0 || found->m_version_minor > 0)) {
      label = getUpdateLabel(found, metadata);
      m_remove = true;
    }
    auto button = new wxButton(this, wxID_ANY, label);
    auto pluginHandler = PluginHandler::getInstance();
    auto box = new wxBoxSizer(wxHORIZONTAL);
    box->Add(button);
    SetSizer(box);
    Bind(wxEVT_COMMAND_BUTTON_CLICKED, &InstallButton::OnClick, this);
  }

  void OnClick(wxCommandEvent& event) {
    wxLogMessage("Selected update: %s", m_metadata.name.c_str());
    auto top_parent = GetParent()->GetParent()->GetParent();
    auto dialog = dynamic_cast<UpdateDialog*>(top_parent);
    wxASSERT(dialog != 0);
    dialog->SetUpdate(m_metadata);
    dialog->EndModal(wxID_OK);
  }

private:
  PluginMetadata m_metadata;
  bool m_remove;

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
      : wxPanel(parent), m_descr(0), m_buttons(buttons) {
    auto flags = wxSizerFlags().Border();
    m_isDesc = false;

    MORE = "<span foreground=\'blue\'>";
    MORE += _("More");
    MORE += "...</span>";
    LESS = "<span foreground=\'blue\'>";
    LESS += _("Less");
    LESS += "...</span>";

    //  For small displays, skip the "More" text.
    if (g_Platform->getDisplaySize().x < 80 * GetCharWidth())
      MORE = "";

    auto sum_hbox = new wxBoxSizer(wxHORIZONTAL);
    m_widthDescription = g_options->GetSize().x *4 / 10;

    // m_summary = staticText(plugin->summary);
    m_summary = new wxStaticText(
        this, wxID_ANY, _T(""), wxDefaultPosition,
        wxSize(m_widthDescription, -1) /*, wxST_NO_AUTORESIZE*/);
    m_summaryText = wxString(plugin->summary.c_str());
    m_summary->SetLabel(m_summaryText);
    m_summary->Wrap(m_widthDescription);

    HardBreakWrapper wrapper(this, m_summaryText, m_widthDescription);
    m_summaryLineCount = wrapper.GetLineCount() + 1;

    sum_hbox->Add(m_summary);
    sum_hbox->AddSpacer(10);
    m_more = staticText("4 Chars");
    m_more->SetLabelMarkup(MORE);
    sum_hbox->Add(m_more, wxSizerFlags());

    auto vbox = new wxBoxSizer(wxVERTICAL);
    SetSizer(vbox);

    std::string name_reduced = plugin->name;
    if(plugin->name.size() * GetCharWidth() > (size_t)m_widthDescription * 7 / 10){
      int nc = m_widthDescription *7 / 10 / GetCharWidth();
      if (nc > 3){
        name_reduced = plugin->name.substr(0, nc-3) + "...";
      }
    }

    wxString nameText(name_reduced + "  " + plugin->version);
    if (bshowTuple) nameText += "   " + plugin->target;

    auto name = staticText(nameText);

    m_descr = new wxStaticText(
        this, wxID_ANY, _T(""), wxDefaultPosition,
        wxSize(m_widthDescription, -1) /*, wxST_NO_AUTORESIZE*/);
    m_descText = wxString(plugin->description.c_str());
    m_descr->SetLabel(m_descText);
    m_descr->Wrap(m_widthDescription);
    m_descr->Hide();
    vbox->Add(name, flags);
    vbox->Add(sum_hbox, flags);
    vbox->Add(m_descr, 0);
    Fit();

    m_more->Bind(wxEVT_LEFT_DOWN, &PluginTextPanel::OnClick, this);
    m_descr->Bind(wxEVT_LEFT_DOWN, &PluginTextPanel::OnClick, this);
  }

  void OnClick(wxMouseEvent& event) {
    m_descr->Show(!m_descr->IsShown());
    m_descr->SetLabel(_T(""));
    m_descr->SetLabel(m_descText);
    m_descr->Wrap(m_widthDescription);
    Layout();
    wxSize asize = GetEffectiveMinSize();

    m_more->SetLabelMarkup(m_descr->IsShown() ? LESS : MORE);
    m_buttons->HideDetails(!m_descr->IsShown());

    UpdateDialog* swin = wxDynamicCast(GetGrandParent(), UpdateDialog);
    if (swin) {
      swin->RecalculateSize();
    }
  }

  int m_summaryLineCount;
  bool m_isDesc;

protected:
  wxString MORE, LESS;

  wxStaticText* staticText(const wxString& text) {
    return new wxStaticText(this, wxID_ANY, text, wxDefaultPosition,
                            wxDefaultSize, wxALIGN_LEFT);
  }

  wxStaticText* m_descr;
  wxStaticText* m_more;
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
  };

  void populateGrid(wxFlexGridSizer* grid) {
    /** Compare two PluginMetadata objects, a named c++ requirement. */
    struct metadata_compare {
      bool operator()(const PluginMetadata& lhs,
                      const PluginMetadata& rhs) const {
        return lhs.key() < rhs.key();
      }
    };

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

//}  // namespace update_mgr

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
  int calcHeight = 0;
  int calcWidth = 0;
  wxWindowList& kids = m_scrwin->GetChildren();
  for (unsigned int i = 0; i < kids.GetCount(); i++) {
    wxWindowListNode* node = kids.Item(i);
    wxWindow* win = node->GetData();

    if (win && win->IsKindOf(CLASSINFO(PluginTextPanel))) {
      PluginTextPanel* panel = (PluginTextPanel*)win;
      if (panel->m_isDesc) {
        wxSize tsize = win->GetEffectiveMinSize();
        calcHeight += tsize.y + GetCharHeight();
        calcWidth = tsize.x * 2;
      }
    }
  }

  calcHeight += 3 * GetCharHeight();  // "dismiss" button
  calcWidth = wxMin(calcWidth, g_Platform->getDisplaySize().x);

  m_scrwin->SetMinSize(wxSize(calcWidth, calcHeight));

#ifdef __OCPN__ANDROID__
  SetMinSize(g_Platform->getDisplaySize());
#endif


  Fit();
  SetMaxSize(g_Platform->getDisplaySize());
  Layout();
}
