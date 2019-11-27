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

#include <sstream>

#include <wx/bitmap.h>
#include <wx/button.h>
#include <wx/debug.h>
#include <wx/image.h>
#include <wx/log.h>
#include <wx/panel.h>
#include <wx/progdlg.h>
#include <wx/sizer.h>
#include <wx/statline.h>

#include "catalog_mgr.h"
#include "download_mgr.h"
#include "Downloader.h"
#include "OCPNPlatform.h"
#include "PluginHandler.h"
#include "pluginmanager.h"
#include "styles.h"

extern PlugInManager*           g_pi_manager;
extern ocpnStyle::StyleManager* g_StyleManager;
extern OCPNPlatform*            g_Platform;

extern wxImage LoadSVGIcon( wxString filename, int width, int height );

#undef major                // walk around gnu's major() and minor() macros.
#undef minor

namespace download_mgr {

/**
 * Used to compare plugin versions. Versions are basically semantic
 * versioning: major.minor.revision.build for example 1.2.6.1-deadbee. The
 * values major, minor and revision should be integers. The build is a
 * free-format string sorted lexically.
 *
 * Note: The version installed is saved in text files since it's not
 * available in the plugin interface besides major.minor. See
 * https://github.com/OpenCPN/OpenCPN/issues/1443
 */
struct OcpnVersion
{
    int major;
    int minor;
    int revision;
    std::string build;

    OcpnVersion()
        :major(0), minor(0), revision(0), build("")
    {}

    OcpnVersion(std::string version_release)
    {
        char buff[255] = {0};
        std::sscanf(version_release.c_str(),
                    "%d.%d.%d.%s", &major, &minor, &revision, buff);
        build = std::string(buff);
    }

    OcpnVersion(int major, int minor, int revision=0, std::string build = "")
    {
        this->major = major;
        this->minor = minor;
        this->revision = revision;
        this->build = build;
    }

    bool operator < (const OcpnVersion& other)
    {
        if (major < other.major) return true;
        if (minor < other.minor) return true;
        if (revision < other.revision) return true;
        if (build < other.build) return true;
        return false;
    }

    bool operator == (const OcpnVersion& other)
    {
        return major == other.major
            && minor == other.minor
            && revision == other.revision
            && build == other.build;
    }

    bool operator > (const OcpnVersion& other)
    {
        return !(*this == other) && !(*this < other);
    }

    bool operator <= (const OcpnVersion& other)
    {
        return (*this == other) || (*this < other);
    }

    bool operator >= (const OcpnVersion& other)
    {
        return (*this == other) || (*this > other);
    }

    bool operator != (const OcpnVersion& other)
    {
        return !(*this == other);
    }

    friend std::ostream& operator << (std::ostream& s, const OcpnVersion& v)
    {
        s << v.major << '.' << v.minor << '.' << v.revision;
        if (v.build != "" ) {
            s << '.' << v.build;
        }
        return s;
    }

    std::string to_string()
    {
        std::ostringstream os;
        os << *this;
        return os.str();
    }
};


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

        ~PluginIconPanel()
        {
            Unbind(wxEVT_PAINT, &PluginIconPanel::OnPaint, this);
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


/** Add progress and final message dialogs to the basic Downloader. */
class GuiDownloader: public Downloader
{
    private:
        long m_downloaded;
        wxProgressDialog* m_dialog;
        PluginMetadata m_plugin;

    public:
        GuiDownloader(PluginMetadata plugin)
            :Downloader(plugin.tarball_url),
            m_downloaded(0), m_dialog(0), m_plugin(plugin)
        {}

        void run(wxWindow* parent)
        {
            auto pluginHandler = PluginHandler::getInstance();
            long size = get_filesize();
            std::string label(_("Downloading "));
            label += url;
            m_dialog = new wxProgressDialog(
                            _("Downloading"), label.c_str(), size, parent,
                            wxPD_AUTO_HIDE | wxPD_APP_MODAL | wxPD_CAN_ABORT);
            std::string path("");
            bool ok = download(path);
            if (m_dialog == 0) {
                ok = false;
            } else {
                delete m_dialog;
            }
            m_dialog = 0;    // make sure that on_chunk() doesn't misbehave.
            wxMessageDialog* dlg = 0;
            ok = ok ? pluginHandler->install(m_plugin, path) : false;
            auto pic = PlugInByName(m_plugin.name,
                                    g_pi_manager->GetPlugInArray());
            ok = pic ? ok : false;
            if (ok) {
                dlg = new wxMessageDialog(
                        parent,
                        m_plugin.name + " " + m_plugin.version
                              + _(" successfully installed"),
                        _("Installation complete"),
                        wxOK | wxCENTRE | wxICON_INFORMATION);
            }
            else {
                dlg = new wxMessageDialog(
                        parent,
                        _("Download or install errors, check logs"),
                        _("Installation error"),
                        wxOK | wxCENTRE | wxICON_ERROR);
            }
            dlg->ShowModal();
        }

        void on_chunk(const char* buff, unsigned bytes) override
        {
            Downloader::on_chunk(buff, bytes);
            m_downloaded += bytes;
            if (m_dialog && !m_dialog->Update(m_downloaded)) {
                // User pushed Cancel button
                delete m_dialog;
                m_dialog = 0;
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

        ~InstallButton()
        {
            Unbind(wxEVT_COMMAND_BUTTON_CLICKED,
                   &InstallButton::OnClick, this);
        }

        void OnClick(wxCommandEvent& event) {
            if (m_remove) {
                wxLogMessage("Uninstalling %s", m_metadata.name.c_str());
                PluginHandler::getInstance()->uninstall(m_metadata.name);
            }
            wxLogMessage("Installing %s", m_metadata.name.c_str());
            auto downloader = new GuiDownloader(m_metadata);
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
            listPanels->ReloadPlugins(g_pi_manager->GetPlugInArray());
            auto window = GetSizer()->GetItem((size_t) 0)->GetWindow();
            auto btn = dynamic_cast<wxButton*>(window);
            wxASSERT(btn != 0);
            btn->SetLabel(_("Reinstall"));
        }

    private:
        PluginMetadata m_metadata;
        bool m_remove;

        const char* getUpdateLabel(PlugInContainer* pic,
                                   PluginMetadata metadata)
        {
            OcpnVersion currentVersion(pic->m_version_major,
                                       pic->m_version_minor);
            if (pic->m_version_str != "") {
                currentVersion =
                    OcpnVersion(pic->m_version_str.ToStdString());
            }
            OcpnVersion newVersion(metadata.version);
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
class WebsiteButton: public wxPanel
{
    public:
        WebsiteButton(wxWindow* parent, const char* url)
            :wxPanel(parent), m_url(url)
        {
            auto vbox = new wxBoxSizer(wxVERTICAL);
            auto button = new wxButton(this, wxID_ANY, _("Website"));
            button->Enable(strlen(url) > 0);
            vbox->Add(button);
            SetSizer(vbox);
            Bind(wxEVT_COMMAND_BUTTON_CLICKED,
                 [=](wxCommandEvent&) {wxLaunchDefaultBrowser(m_url);});
        }

        ~WebsiteButton()
        {
            Unbind(wxEVT_COMMAND_BUTTON_CLICKED,
                   [=](wxCommandEvent&) {wxLaunchDefaultBrowser(m_url);});
        }

    protected:
        const std::string m_url;
};


/** The two buttons 'install' and 'website'; the latter optionally hidden. */
class CandidateButtonsPanel: public wxPanel
{
    public:

        CandidateButtonsPanel(wxWindow* parent, const PluginMetadata* plugin)
            :wxPanel(parent), m_parent(parent)
        {
            auto flags = wxSizerFlags().Border();

            auto vbox = new wxBoxSizer(wxVERTICAL);
            vbox->Add(new InstallButton(this, *plugin), flags);
            vbox->Add(1, 1, 1, wxEXPAND);   // Expanding, stretchable spacer
            m_info_btn = new WebsiteButton(this, plugin->info_url.c_str());
            m_info_btn->Hide();
            vbox->Add(m_info_btn, flags.Bottom());
            SetSizer(vbox);
            Fit();
        }

        void HideDetails(bool hide)
        {
            m_info_btn->Show(!hide);
            m_parent->Layout();
        }

    private:
        wxWindow* m_parent;
        WebsiteButton* m_info_btn;
};


/** Plugin name, version, summary + an optionally shown description. */
class PluginTextPanel: public wxPanel
{
    public:
        PluginTextPanel(wxWindow* parent,
                        const PluginMetadata* plugin,
                        CandidateButtonsPanel* buttons)
            :wxPanel(parent), m_descr(0), m_parent(parent), m_buttons(buttons)
        {
            auto flags = wxSizerFlags().Border();

            auto sum_hbox = new wxBoxSizer(wxHORIZONTAL);
            m_more = staticText("");
            m_more->SetLabelMarkup(MORE);
            sum_hbox->Add(staticText(plugin->summary));
            sum_hbox->AddSpacer(10);
            sum_hbox->Add(m_more, wxSizerFlags());

            auto vbox = new wxBoxSizer(wxVERTICAL);
            auto name = staticText(plugin->name + "    " + plugin->version);
            m_descr = staticText(plugin->description);
            m_descr->Hide();
            vbox->Add(name, flags);
            vbox->Add(sum_hbox, flags);
            vbox->Add(m_descr, flags);
            SetSizer(vbox);

            m_more->Bind(wxEVT_LEFT_DOWN, &PluginTextPanel::OnClick, this);
            m_descr->Bind(wxEVT_LEFT_DOWN, &PluginTextPanel::OnClick, this);
        }

        ~PluginTextPanel()
        {
            m_more->Unbind(wxEVT_LEFT_DOWN, &PluginTextPanel::OnClick, this);
            m_descr->Unbind(wxEVT_LEFT_DOWN, &PluginTextPanel::OnClick, this);
        }

        void OnClick(wxMouseEvent& event)
        {
            m_descr->Show(!m_descr->IsShown());
            m_more->SetLabelMarkup(m_descr->IsShown() ? LESS : MORE);
            m_parent->Layout();
            m_buttons->HideDetails(!m_descr->IsShown());
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
        wxWindow* m_parent;
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
        
        /**  Invokes the update catalog procedure. */
        class UpdateCatalogNowBtn: public wxButton
        {
            public:

            UpdateCatalogNowBtn(wxWindow* parent)
                :wxButton(parent, wxID_ANY, _("Update plugin catalog"))
            {
                Bind(wxEVT_COMMAND_BUTTON_CLICKED,
                   [=](wxCommandEvent&) {new CatalogDialog(GetParent(), true);
                });
            }
        
        };

        /**  Button invoking the update catalog dialog. */
        class UpdateCatalogDialogBtn: public wxButton
        {
            public:

            UpdateCatalogDialogBtn(wxWindow* parent)
                :wxButton(parent, wxID_ANY, _("Advanced catalog update..."))
            {
                 Bind(wxEVT_COMMAND_BUTTON_CLICKED,
                     [=](wxCommandEvent&) {new CatalogDialog(GetParent(), false); });
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
            :wxScrolledWindow(parent)
        {
            auto grid = new wxFlexGridSizer(3, 0, 0);
            grid->AddGrowableCol(0);
            grid->AddGrowableCol(1);
            grid->AddGrowableCol(2);
            auto flags = wxSizerFlags().Proportion(1).Expand();
            auto box = new wxBoxSizer(wxVERTICAL);
            for (auto plugin: PluginHandler::getInstance()->getAvailable()) {
                if (plugin.target != PKG_TARGET) {
                    continue;
                }
                grid->Add(new PluginIconPanel(this, plugin.name), flags);
                auto buttons = new CandidateButtonsPanel(this, &plugin);
                grid->Add(new PluginTextPanel(this, &plugin, buttons), flags);
                grid->Add(buttons, flags);
                grid->Add(new wxStaticLine(this), flags);
                grid->Add(new wxStaticLine(this), flags);
                grid->Add(new wxStaticLine(this), flags);
            }
            box->Add(grid, wxSizerFlags().Proportion(1).Expand());
            auto button_panel = new MainButtonsPanel(this, parent);
            box->Add(button_panel, wxSizerFlags().Right().Border().Expand());
            SetSizer(box);
            //FitInside();
            SetScrollRate(0, 1);
        };
};

}  // namespace download_mgr

/** Top-level install plugins dialog. */
PluginDownloadDialog::PluginDownloadDialog(wxWindow* parent)
    :wxDialog(parent, wxID_ANY, _("Plugin Manager"),
              wxDefaultPosition , wxDefaultSize,
              wxDEFAULT_DIALOG_STYLE),
    m_parent(parent)
{
    auto vbox = new wxBoxSizer(wxVERTICAL);
    auto scrwin = new download_mgr::OcpnScrolledWindow(this);
    vbox->Add(scrwin, wxSizerFlags().Proportion(1).Expand());

    // At least GTK has bad defaults, widgets are not realized. Try to
    // compute a reasonable minimum size:
    wxSize minsize = GetTextExtent("abcdefghijklmnopqrst");
    minsize = wxSize(5 * minsize.GetWidth(), 20 * minsize.GetHeight());
    SetMinClientSize(minsize);

    Fit();
    Center();
}
