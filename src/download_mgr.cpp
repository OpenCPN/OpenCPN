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
#include <wx/uri.h>

#include "catalog_mgr.h"
#include "download_mgr.h"
#include "Downloader.h"
#include "OCPNPlatform.h"
#include "PluginHandler.h"
#include "pluginmanager.h"
#include "semantic_vers.h"
#include "styles.h"

extern PlugInManager*           g_pi_manager;
extern ocpnStyle::StyleManager* g_StyleManager;
extern OCPNPlatform*            g_Platform;

extern wxImage LoadSVGIcon( wxString filename, int width, int height );

#undef major                // walk around gnu's major() and minor() macros.
#undef minor

// Main window reload event
wxDEFINE_EVENT(EVT_PLUGINS_RELOAD, wxCommandEvent);

namespace download_mgr {


/**
 * Return index in ArrayOfPlugins for plugin with given name,
 * or -1 if not found.
 */
static ssize_t PlugInIxByName(const std::string name, ArrayOfPlugIns* plugins)
{
    for (unsigned i = 0; i < plugins->GetCount(); i += 1) {
        if (name == plugins->Item(i)->m_common_name.Lower().ToStdString()) {
            return i;
        }
    }
    return -1;
}


/** Return PlugInContainer with given name or 0 if not found. */
static PlugInContainer*
    PlugInByName(const std::string name, ArrayOfPlugIns* plugins)
{
    auto ix = PlugInIxByName(name, plugins);
    return ix == -1 ? 0 : plugins->Item(ix);
}


/** Load a png icon rescaled to size x size. */
static void LoadPNGIcon(const char* path, int size, wxBitmap& bitmap)
{
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


/** Load a svg icon rescaled to size x size. */
static void LoadSVGIcon(wxFileName path, int size, wxBitmap& bitmap)
{
    wxImage img = LoadSVGIcon(path.GetFullPath(), size, size);
    bitmap = wxBitmap(img);
}


/**
 * A plugin icon, scaled to about 2/3 of available space
 *
 * Load icons from .../uidata/plugins, on the form plugin.svg or
 * plugin.png. If neither exists, display a default  icon.
 */
class PluginIconPanel: public wxPanel
{
    public:
        PluginIconPanel(wxWindow* parent, std::string plugin_name)
            :wxPanel(parent), m_plugin_name(plugin_name)
        {
            auto size = GetClientSize();
            auto minsize = GetTextExtent("OpenCPN");
            SetMinClientSize(wxSize(minsize.GetWidth(), size.GetHeight()));
            Layout();
            Bind(wxEVT_PAINT, &PluginIconPanel::OnPaint, this);
        }

        void OnPaint(wxPaintEvent& event)
        {
            auto size = GetClientSize();
            int minsize = wxMin(size.GetHeight(), size.GetWidth());
            auto offset = minsize / 10;

            LoadIcon(m_plugin_name.c_str(), m_bitmap,  2 * minsize / 3);
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

        void LoadIcon(const char* plugin_name, wxBitmap& bitmap, int size=32)
        {
            wxFileName path(g_Platform->GetSharedDataDir(), plugin_name);
            path.AppendDir("uidata");
            path.AppendDir("plugins");
            path.SetExt("svg");
            bool ok = false;
            if (path.IsFileReadable()) {
                LoadSVGIcon(path, size, bitmap);
                ok = bitmap.IsOk();
            }
            if (!ok) {
                path.SetExt("png");
                if (path.IsFileReadable()) {
                    LoadPNGIcon(path.GetFullPath(), size, bitmap);
                    ok = bitmap.IsOk();
                }
            }
            if (!ok) {
                auto style = g_StyleManager->GetCurrentStyle();
                bitmap = wxBitmap(style->GetIcon( _T("default_pi")));
            }
        }
};


/** Download and install a PluginMetadata item when clicked. */
class InstallButton: public wxPanel
{
    public:
        InstallButton(wxWindow* parent, PluginMetadata metadata)
            :wxPanel(parent), m_metadata(metadata), m_remove(false)
        {
            PlugInContainer* found =
                PlugInByName(metadata.name, g_pi_manager->GetPlugInArray());
            std::string label(_("Install"));
            if (found) {
                label = getUpdateLabel(found, metadata);
                m_remove = true;
            }
            auto button = new wxButton(this, wxID_ANY, label);
            auto pluginHandler = PluginHandler::getInstance();
            button->Enable(pluginHandler->isPluginWritable(metadata.name));
            auto box = new wxBoxSizer(wxHORIZONTAL);
            box->Add(button);
            SetSizer(box);
            Bind(wxEVT_COMMAND_BUTTON_CLICKED, &InstallButton::OnClick, this);
        }

        void OnClick(wxCommandEvent& event) {
            if (m_remove) {
                wxLogMessage("Uninstalling %s", m_metadata.name.c_str());
                PluginHandler::getInstance()->uninstall(m_metadata.name);
            }
            wxLogMessage("Installing %s", m_metadata.name.c_str());
            
            auto pluginHandler = PluginHandler::getInstance();
            bool cacheResult = pluginHandler->installPluginFromCache( m_metadata );
            
            if(!cacheResult){
                auto downloader = new GuiDownloader(this, m_metadata);
                downloader->run(this);
                auto pic = PlugInByName(m_metadata.name,
                                        g_pi_manager->GetPlugInArray());
                if (!pic) {
                    wxLogMessage("Installation of %s failed",
                                m_metadata.name.c_str());
                    return;
                }
                auto upwards = GetParent()->GetParent()->GetParent();
                auto main_window = dynamic_cast<PluginDownloadDialog*>(upwards);
                wxASSERT(main_window != 0);
                auto listPanels =
                    dynamic_cast<PluginListPanel*>(main_window->GetRealParent()->GetPrevSibling());
                wxASSERT(listPanels != 0);
                listPanels->ReloadPluginPanels(g_pi_manager->GetPlugInArray());
                auto window = GetSizer()->GetItem((size_t) 0)->GetWindow();
                auto btn = dynamic_cast<wxButton*>(window);
                wxASSERT(btn != 0);
                btn->SetLabel(_("Reinstall"));
            }
        }

    private:
        PluginMetadata m_metadata;
        bool m_remove;

        const char* getUpdateLabel(PlugInContainer* pic,
                                   PluginMetadata metadata)
        {
            SemanticVersion currentVersion(pic->m_version_major,
                                       pic->m_version_minor);
            if (pic->m_version_str != "") {
                currentVersion =
                    SemanticVersion::parse(pic->m_version_str.ToStdString());
            }
            auto newVersion = SemanticVersion::parse(metadata.version);
            if (newVersion > currentVersion) {
                return _("Update");
            }
            else if (newVersion == currentVersion) {
                return _("Reinstall");
            }
            else {
                return _("Downgrade");
            }
        }
};




/** The two buttons 'install' and 'website', the latter optionally hidden. */
class CandidateButtonsPanel: public wxPanel
{
    public:

        CandidateButtonsPanel(wxWindow* parent, const PluginMetadata* plugin)
            :wxPanel(parent)
        {
            auto flags = wxSizerFlags().Border();

            auto vbox = new wxBoxSizer(wxVERTICAL);
            vbox->Add(new InstallButton(this, *plugin), 
                                        flags.DoubleBorder().Top().Right());
            vbox->Add(1, 1, 1, wxEXPAND);   // Expanding, stretchable spacer
            m_info_btn = new WebsiteButton(this, plugin->info_url.c_str());
            m_info_btn->Hide();
            vbox->Add(m_info_btn, flags.DoubleBorder().Bottom().Right());
            SetSizer(vbox);
            Fit();
        }

        void HideDetails(bool hide)
        {
            m_info_btn->Show(!hide);
            GetParent()->Layout();
        }

    private:
        WebsiteButton* m_info_btn;
};


/** Plugin name, version, summary + an optionally shown description. */
class PluginTextPanel: public wxPanel
{
    public:
        PluginTextPanel(wxWindow* parent,
                        const PluginMetadata* plugin,
                        CandidateButtonsPanel* buttons)
            : wxPanel(parent), m_descr(0), m_buttons(buttons)
        {
            auto flags = wxSizerFlags().Border();

            auto sum_hbox = new wxBoxSizer(wxHORIZONTAL);
            m_summary = staticText(plugin->summary);
            sum_hbox->Add(m_summary);
            sum_hbox->AddSpacer(10);
            m_more = staticText("");
            m_more->SetLabelMarkup(MORE);
            sum_hbox->Add(m_more, wxSizerFlags());

            auto vbox = new wxBoxSizer(wxVERTICAL);
            auto name = staticText(plugin->name + "    " + plugin->version);
            m_descr = staticText(plugin->description);
            m_descr->Hide();
            vbox->Add(name, flags);
            vbox->Add(sum_hbox, flags);
            vbox->Add(m_descr, flags.Expand());
            SetSizer(vbox);

            m_more->Bind(wxEVT_LEFT_DOWN, &PluginTextPanel::OnClick, this);
            m_descr->Bind(wxEVT_LEFT_DOWN, &PluginTextPanel::OnClick, this);
        }

        void OnClick(wxMouseEvent& event)
        {
            m_descr->Show(!m_descr->IsShown());
            m_more->SetLabelMarkup(m_descr->IsShown() ? LESS : MORE);
            m_buttons->HideDetails(!m_descr->IsShown());
            GetParent()->SendSizeEvent();
            GetParent()->GetParent()->GetParent()->Layout();
            GetParent()->GetParent()->GetParent()->Refresh(true);
            GetParent()->GetParent()->GetParent()->Update();
        }

    protected:
        const char* const MORE = _("<span foreground='blue'>More...</span>");
        const char* const LESS = _("<span foreground='blue'>Less...</span>");

        wxStaticText* staticText(const wxString& text)
        {
            return new wxStaticText(this, wxID_ANY, text, wxDefaultPosition,
                                    wxDefaultSize, wxALIGN_LEFT);
        }

        wxStaticText* m_descr;
        wxStaticText* m_more;
        wxStaticText* m_summary;
        CandidateButtonsPanel* m_buttons;
};


/**
 * Three buttons bottom-right for plugin catalog maintenance.
 */
class MainButtonsPanel: public wxPanel
{
    public:
        MainButtonsPanel(wxWindow* parent, wxWindow* victim)
            :wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(200, 32))
        {
            auto sizer = new wxBoxSizer(wxHORIZONTAL);
            auto spacing = GetTextExtent("m").GetWidth();
            sizer->Add(1, 1, 1, wxEXPAND);   // Expanding, stretchable space
            sizer->Add(new UpdateCatalogDialogBtn(this), wxSizerFlags());
            sizer->Add(spacing, 0);
            sizer->Add(new UpdateCatalogNowBtn(this), wxSizerFlags());
            sizer->Add(spacing, 0);
            sizer->Add(new wxButton(this, wxID_OK, _("Done")),
                       wxSizerFlags());
            SetSizer(sizer);
        }

    protected:
        
        /**  Invokes the simple update catalog procedure. */
        class UpdateCatalogNowBtn: public wxButton
        {
            public:

            UpdateCatalogNowBtn(wxWindow* parent)
                :wxButton(parent, wxID_ANY, _("Update plugin catalog"))
            {
                Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent&) {
                        new SimpleCatalogDialog(this);
                        wxCommandEvent evt(EVT_PLUGINS_RELOAD);
                        wxPostEvent(GetParent(), evt);
                });
            }
        };

        /**  Button invoking the advanced  catalog dialog. */
        class UpdateCatalogDialogBtn: public wxButton
        {
            public:

            UpdateCatalogDialogBtn(wxWindow* parent)
                :wxButton(parent, wxID_ANY, _("Advanced catalog update..."))
            {
                 Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent&) {
                      new AdvancedCatalogDialog(this);
                 });
            }
        };
};


/**
 * The list of download candidates in a scrolled window + OK and
 * Settings  button.
 */
class OcpnScrolledWindow : public wxScrolledWindow
{
    public:
        OcpnScrolledWindow(wxWindow* parent)
            :wxScrolledWindow(parent),
            m_grid(new wxFlexGridSizer(3, 0, 0))
        {
            auto box = new wxBoxSizer(wxVERTICAL);
            populateGrid(m_grid);
            box->Add(m_grid, wxSizerFlags().Proportion(1).Expand());
            auto button_panel = new MainButtonsPanel(this, parent);
            box->Add(button_panel, wxSizerFlags().Right().Border().Expand());
            Bind(EVT_PLUGINS_RELOAD, [&](wxCommandEvent& ev) { Reload(); });

            SetSizer(box);
            FitInside();
            // TODO: Compute size using wxWindow::GetEffectiveMinSize()
            SetScrollRate(1, 1);
        };

        void populateGrid(wxFlexGridSizer* grid)
        {
            /** Compare two PluginMetadata objects, a named c++ requirement. */
            struct metadata_compare{
                bool operator() (const PluginMetadata& lhs,
                                 const PluginMetadata& rhs) const
                {
                    return lhs.key() < rhs.key();
                }
            };

            auto flags = wxSizerFlags();
            grid->SetCols(3);
            grid->AddGrowableCol(2);
            auto available = PluginHandler::getInstance()->getAvailable();
            std::set<PluginMetadata, metadata_compare> unique_plugins;
            for (auto plugin: PluginHandler::getInstance()->getAvailable()) {
                unique_plugins.insert(plugin);
            }
            for (auto plugin: unique_plugins) {
                if (!PluginHandler::isCompatible(plugin)) {
                    continue;
                }
                grid->Add(new PluginIconPanel(this, plugin.name), flags.Expand());
                auto buttons = new CandidateButtonsPanel(this, &plugin);
                grid->Add(new PluginTextPanel(this, &plugin, buttons),
                          flags.Proportion(1).Right());
                grid->Add(buttons, flags.DoubleBorder());
                grid->Add(new wxStaticLine(this), wxSizerFlags(0).Expand());
                grid->Add(new wxStaticLine(this), wxSizerFlags(0).Expand());
                grid->Add(new wxStaticLine(this), wxSizerFlags(0).Expand());
            }
        }

        void Reload()
        {
            Hide();
            m_grid->Clear();
            populateGrid(m_grid);
            Layout();
            Show();
            FitInside();
            Refresh(true);
        }

    private:
        wxFlexGridSizer* m_grid;
};

}  // namespace download_mgr

/** Top-level install plugins dialog. */
PluginDownloadDialog::PluginDownloadDialog(wxWindow* parent)
    :wxDialog(parent, wxID_ANY, _("Plugin Manager"),
              wxDefaultPosition , wxDefaultSize,
              wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
    auto vbox = new wxBoxSizer(wxVERTICAL);
    auto scrwin = new download_mgr::OcpnScrolledWindow(this);
    vbox->Add(scrwin, wxSizerFlags(1).Expand());

    // The list has no natural height. Allocate 20 lines of text so some
    // items are displayed initially in Layout()
    int min_height = GetTextExtent("abcdefghijklmnopqrst").GetHeight() * 20;

    // There seem to be no way have dynamic, wrapping text:
    // https://forums.wxwidgets.org/viewtopic.php?f=1&t=46662
    int width = GetParent()->GetClientSize().GetWidth();
    SetMinClientSize(wxSize(width, min_height));

    SetSizer(vbox);
    Fit();
    Layout();
}

/** Add progress and final message dialogs to the basic Downloader. */
GuiDownloader::GuiDownloader(wxWindow* parent, PluginMetadata plugin)
            :Downloader(plugin.tarball_url),
            m_downloaded(0), m_dialog(0), m_plugin(plugin), m_parent(parent)
            { }

        
std::string GuiDownloader::CheckCache()
{
    // Look in the cache
    wxURI uri( wxString(m_plugin.tarball_url.c_str()));
    wxFileName fn(uri.GetPath());
    wxString tarballFile = fn.GetFullName();
    wxString cacheDir = g_Platform->GetPrivateDataDir() + _T("/") + _T("plugins");
    wxString sep = _T("/");
    cacheDir += sep + wxString(_T("cache"));
    cacheDir += sep + wxString(_T("tarballs"));
    wxString cacheCopy = cacheDir +sep + tarballFile;
    if(wxFileExists( cacheCopy ))
        return cacheCopy.ToStdString();
    else
        return std::string("");
}
        
 

std::string GuiDownloader::run(wxWindow* parent)
{
            bool ok;
            bool downloaded = false;
            std::string path = CheckCache();
            if(!path.size()){
                long size = get_filesize();
                std::string label(_("Downloading "));
                label += url;
                m_dialog = new wxProgressDialog(
                                _("Downloading"), label.c_str(), size, parent,
                                wxPD_AUTO_HIDE | wxPD_APP_MODAL | wxPD_CAN_ABORT);
#ifdef __OCPN__ANDROID__
                m_dialog->SetBackgroundColour(wxColour(0x7c, 0xb0, 0xe9));              // light blue
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

                m_dialog = 0;    // make sure that on_chunk() doesn't misbehave.
                downloaded = true;
            }

            auto pluginHandler = PluginHandler::getInstance();
            ok = pluginHandler->installPlugin(m_plugin, path);
            if (!ok) {
                showErrorDialog("Installation error");
                return "";
            }
            
            if(downloaded){
                // Cache the tarball from the tmp location to the plugin cache.
                wxURI uri( wxString(m_plugin.tarball_url.c_str()));
                wxFileName fn(uri.GetPath());
                wxString tarballFile = fn.GetFullName();
                wxString cacheDir = g_Platform->GetPrivateDataDir() + _T("/") + _T("plugins");
                wxString sep = _T("/");
                if( !wxDirExists(cacheDir) )
                    wxMkdir( cacheDir);
                cacheDir += sep + wxString(_T("cache"));
                if( !wxDirExists(cacheDir) )
                    wxMkdir( cacheDir);
                cacheDir += sep + wxString(_T("tarballs"));
                if( !wxDirExists(cacheDir) )
                    wxMkdir( cacheDir);
        
                wxString destination = cacheDir + _T("/") + tarballFile;
                wxLogMessage(" Trying to copy %s ",  path.c_str());
                
                if(wxFileExists(wxString( path.c_str()))){
                    wxLogMessage("Copying %s to local cache",  tarballFile.c_str());
                    wxCopyFile( wxString( path.c_str()), destination);
                }
            }

            
            wxMessageDialog *dlg = new wxMessageDialog(
                        m_parent,
                        m_plugin.name + " " + m_plugin.version
                              + _(" successfully installed"),
                        _("Installation complete"),
                        wxOK | wxCENTRE | wxICON_INFORMATION);
            dlg->ShowModal();
            return path;
}

void GuiDownloader::on_chunk(const char* buff, unsigned bytes) 
{
            Downloader::on_chunk(buff, bytes);
            m_downloaded += bytes;
            if (m_dialog && !m_dialog->Update(m_downloaded)) {
                // User pushed Cancel button
                delete m_dialog;
                m_dialog = 0;
            }
}

void GuiDownloader::showErrorDialog(const char* msg)
{
            auto dlg = new wxMessageDialog(
                    m_parent,
                    "",
                    _("Installation error"),
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

