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

#include <fstream>
#include <sstream>
#include <thread>

#include <wx/button.h>
#include <wx/debug.h>
#include <wx/filename.h>
#include <wx/log.h>
#include <wx/msgdlg.h>
#include <wx/panel.h>
#include <wx/progdlg.h>
#include <wx/sizer.h>
#include <wx/statline.h>

#include "catalog_handler.h"
#include "catalog_mgr.h"
#include "Downloader.h"
#include "ocpn_utils.h"
#include "OCPNPlatform.h"
#include "PluginHandler.h"
#include "download_mgr.h"

extern OCPNPlatform*            g_Platform;


/** Posted by Worker() in CatalogLoad, consumed by CatalogLoad::DialogGrid. */
wxDEFINE_EVENT(CHANNELS_DL_DONE, wxCommandEvent);
wxDEFINE_EVENT(CHANNELS_PARSE_DONE, wxCommandEvent);
wxDEFINE_EVENT(CATALOG_DL_DONE, wxCommandEvent);
wxDEFINE_EVENT(CATALOG_PARSE_DONE, wxCommandEvent);


/** Posted by  CatalogUpdate on close. */
wxDEFINE_EVENT(CATALOG_DLG_CLOSE, wxCommandEvent);

#ifdef _WIN32
static const std::string SEP("\\");
#else
static const std::string SEP("/");
#endif

namespace catalog_mgr
{

class Helpers
{
    public:
        Helpers(wxWindow* _parent): parent(_parent)
        {}

        wxStaticText* staticText(const char* text)
        {
            return new wxStaticText(parent, wxID_ANY, text);
        }

        wxButton* makeButton(const char* label, int id = wxID_ANY)
        {
            return new wxButton(parent, wxID_ANY, label);
        }

    private:
        wxWindow* parent;
};


/** The advanced dialog showing channels, possible updates, rollback etc. */
class CatalogUpdate: public wxDialog, Helpers
{
    protected:
        class UrlEdit; //forward
        class ActiveCatalogGrid; //forward

    public:
        CatalogUpdate(wxWindow* parent)
            :wxDialog(parent, wxID_ANY, _("Manage Plugin Catalog"),
                      wxDefaultPosition , wxDefaultSize,
                      wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
            Helpers(this),
            m_show_edit(true)
        {
            auto sizer = new wxBoxSizer(wxVERTICAL);
            auto flags = wxSizerFlags().Expand().Border();

            m_catalog_grid = new ActiveCatalogGrid(this);
            sizer->Add(new UrlStatus(this), flags);
            sizer->Add(new UrlChannel(this, m_catalog_grid), flags);
            sizer->Add(new wxStaticLine(this), flags);

            sizer->Add(m_catalog_grid, flags);
            sizer->Add(new wxStaticLine(this), flags);

            m_advanced  = new wxStaticText(this, wxID_ANY, "");
            m_advanced->Bind(wxEVT_LEFT_DOWN,
                             [=](wxMouseEvent& e) { toggleUrlEdit(); } );
            sizer->Add(m_advanced, flags);

            m_url_box = new wxBoxSizer(wxVERTICAL);
            m_url_edit = new UrlEdit(this);
            m_url_box->Add(m_url_edit, flags);
            m_url_box->Add(new Buttons(this, m_catalog_grid),
                           wxSizerFlags().Border().Right());
            sizer->Add(m_url_box, flags);
            sizer->Add(new wxStaticLine(this), flags);

            auto done = makeButton(_("Done"));
            sizer->Add(done, wxSizerFlags().Border().Right());
            done->Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=] (wxCommandEvent e) {
                EndModal(wxID_OK);
                wxCommandEvent evt(CATALOG_DLG_CLOSE);
                wxPostEvent(GetParent(), evt);
                e.Skip();
            });
            toggleUrlEdit();

            Bind(wxEVT_CLOSE_WINDOW, [=](wxCloseEvent& e) {
                EndModal(wxID_OK);
                wxCommandEvent cmd_evt(CATALOG_DLG_CLOSE);
                wxPostEvent(GetParent(), cmd_evt);
                e.Skip();
            });

            SetSizer(sizer);
            auto size = getWindowSize();
            size.SetHeight(1);
            SetMinClientSize(size);
            parent->Hide();
            Fit();
            ShowModal();
        }

    protected:
        const char* const HIDE =
            _("<span foreground='blue'>Hide &lt;&lt;&lt;</span>");
        const char* const ADVANCED =
            _("<span foreground='blue'>Ultra advanced &gt;&gt;&gt;</span>");

        wxBoxSizer* m_url_box;
        ActiveCatalogGrid* m_catalog_grid;
        UrlEdit* m_url_edit;
        wxStaticText* m_advanced;
        bool m_show_edit;

        /**
         * The window width  is determined by the normally hidden custom
         * url text control. Return the size of the main window including
         * the hidden part.
         */
        wxSize getWindowSize()
        {
            auto uri = CatalogHandler::getInstance()->GetDefaultUrl();
            auto size = GetTextExtent(uri);
            size.SetWidth(size.GetWidth()  * 120 / 100);
            size.SetHeight(size.GetHeight() * 130 / 100);
            return size;
        }

        void toggleUrlEdit()
        {
            m_show_edit = !m_show_edit;
            m_url_box->GetItem((size_t) 0)->GetWindow()->Show(m_show_edit);
            m_url_box->GetItem((size_t) 1)->GetWindow()->Show(m_show_edit);
            m_advanced->SetLabelMarkup(m_show_edit ? HIDE : ADVANCED);
            Fit();
        }

        /** The Url Status line at top */
        struct UrlStatus: public wxPanel, public Helpers
        {
            UrlStatus(wxWindow* parent) : wxPanel(parent), Helpers(this)
            {
                auto sizer = new wxBoxSizer(wxHORIZONTAL);
                auto flags = wxSizerFlags()
                    .Expand().Border().Align(wxALIGN_CENTER_VERTICAL);
                sizer->Add(staticText(_("Catalog URL status: ")), flags);
                auto catalog = CatalogHandler::getInstance();
                int channels = catalog->GetChannels().size();
                auto text = staticText(channels > 0? _("OK"): _("Error"));
                sizer->Add(text, flags);

                SetSizer(sizer);
                Fit();
                Show();
            }
        };

        /**
         * Active catalog: The current active, the default and latest
         * downloaded + buttons to use default or latest.
         */
        struct ActiveCatalogGrid: public wxPanel, public Helpers
        {
            ActiveCatalogGrid(wxWindow* parent)
                :wxPanel(parent), Helpers(this)
            {
                using CmdEvt = wxCommandEvent;

                auto grid = new wxFlexGridSizer(4, 0, 0);
                auto plugin_handler = PluginHandler::getInstance();
                grid->AddGrowableCol(0);
                grid->AddGrowableCol(1);
                grid->AddGrowableCol(2);
                grid->AddGrowableCol(3);
                auto flags = wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL);
                flags = flags.DoubleBorder();

                /* Cell 0..3 */
                CatalogData catalog_data =
                    CatalogHandler::getInstance()->UserCatalogData();
                grid->Add(staticText(
                            _("Current active plugin catalog")), flags);
                grid->Add(staticText(""), flags);
                grid->Add(staticText(""), flags);
                grid->Add(staticText(""), flags);

                /* Cell 4..7 */
                catalog_data =
                    CatalogHandler::getInstance()->DefaultCatalogData();
                grid->Add(staticText(_("Default catalog")), flags);
                grid->Add(staticText(""), flags);
                grid->Add(staticText(""), flags);
                auto use_default = makeButton(_("Use as active catalog"));
                grid->Add(use_default, wxSizerFlags().Border());
                use_default->Bind(wxEVT_COMMAND_BUTTON_CLICKED,
                                  [=](CmdEvt& e) { useDefaultCatalog(); });

                /* Cell 8..11 */
                catalog_data =
                    CatalogHandler::getInstance()->LatestCatalogData();
                grid->Add(staticText(_("Latest available catalog:")), flags);
                grid->Add(staticText(""), flags);
                grid->Add(staticText(""), flags);
                auto use_latest = makeButton(_("Use as active catalog"));
                use_latest->Bind(wxEVT_COMMAND_BUTTON_CLICKED,
                                 [=](CmdEvt& e) { UseLatestCatalog(); });

                grid->Add(use_latest, wxSizerFlags().Border());

                SetSizer(grid);
                UpdateVersions();
                Fit();
                Show();
            }

            void UpdateVersion(wxSizer* grid, CatalogData data, size_t ix)
            {
                auto version = dynamic_cast<wxStaticText*>(
                        grid->GetItem(ix)->GetWindow());
                version->SetLabel(data.version);
                auto date = dynamic_cast<wxStaticText*>(
                        grid->GetItem(ix + 1)->GetWindow());
                date->SetLabel(data.date);
            }

            /**
             * Update version and date for default, latest and active
             * catalog.
             */
            void UpdateVersions()
            {
                CatalogData data =
                    CatalogHandler::getInstance()->UserCatalogData();
                auto grid = dynamic_cast<wxSizer*>(GetSizer());
                UpdateVersion(grid, data, 1);
                data = CatalogHandler::getInstance()->DefaultCatalogData();
                UpdateVersion(grid, data, 5);
                data = CatalogHandler::getInstance()->LatestCatalogData();
                UpdateVersion(grid, data, 9);
                Refresh(true);
                Update();
            }

            std::string GetDefaultCatalogPath()
            {
                std::string path
                    = g_Platform->GetSharedDataDir().ToStdString();
                path += SEP ;
                path += "ocpn-plugins.xml";
                return path;
            }

            void ReloadAvailableVersion(const std::string url)
            {
                auto handler = CatalogHandler::getInstance();
                std::ostringstream xml;
                auto status = handler->DownloadCatalog(&xml);
                std::string message;
                if (status != CatalogHandler::ServerStatus::OK) {
                    message = _("Cannot download data from url");
                }
                status = handler->ParseCatalog(xml.str(), true);
                if (status != CatalogHandler::ServerStatus::OK) {
                    message = _("Cannot parse downloaded data");
                }
                if (message != "") {
                    wxMessageBox(message,
                                 _("Catalog update problem"),
                                 wxICON_ERROR);
                }
                else {
                    UpdateVersions();
                }
            }

            std::string GetPrivateCatalogPath()
            {
                auto plugin_handler = PluginHandler::getInstance();
                std::string path =
                    g_Platform->GetPrivateDataDir().ToStdString();
                path += SEP + "ocpn-plugins.xml";
                return path;
            }

            void useDefaultCatalog()
            {
                auto src = GetDefaultCatalogPath();
                auto dest = GetPrivateCatalogPath();
                ocpn::copy_file(src, dest );
                CatalogHandler::getInstance()->ClearCatalogData();
                UpdateVersions();
            }

            void UseLatestCatalog()
            {
               auto catalog = CatalogHandler::getInstance();
               std::ofstream dest(GetPrivateCatalogPath());
               catalog->DownloadCatalog(&dest);
               catalog->ClearCatalogData();
               UpdateVersions();
            }
        };

        /** The buttons below custom url: Use Default and Update. */
        struct Buttons: public wxPanel, public Helpers
        {
            Buttons(wxWindow* parent, ActiveCatalogGrid* catalog_grid)
                : wxPanel(parent), Helpers(this),
                m_catalog_grid(catalog_grid),
                m_parent(dynamic_cast<CatalogUpdate*>(GetParent()))
            {
                auto sizer = new wxBoxSizer(wxHORIZONTAL);
                auto flags = wxSizerFlags().Right().Bottom().Border();

                auto clear = makeButton(_("Clear"));
                clear->Bind(wxEVT_COMMAND_BUTTON_CLICKED,
                            [=](wxCommandEvent& ev) { clearUrl(); });
                sizer->Add(clear, flags);

                auto use_default = makeButton(_("Use default location"));
                use_default->Bind(wxEVT_COMMAND_BUTTON_CLICKED,
                                  [=](wxCommandEvent& ev) { useDefaultUrl(); });
                sizer->Add(use_default, flags);

                auto update = makeButton(_("Save"));
                sizer->Add(update, flags);
                update->Bind(wxEVT_COMMAND_BUTTON_CLICKED,
                             [=](wxCommandEvent& ev) { updateUrl(); });

                SetSizer(sizer);
                Fit();
                Show();
            }

            void useDefaultUrl()
            {
                auto url = CatalogHandler::getInstance()->GetDefaultUrl();
                m_parent->m_url_edit->setText(url);
            }

            void clearUrl()
            {
                m_parent->m_url_edit->clear();
            }

            void updateUrl()
            {
                auto text =  m_parent->m_url_edit->getText();
                CatalogHandler::getInstance()->SetCustomUrl(text.c_str());
                m_catalog_grid->ReloadAvailableVersion(text);
            }

            ActiveCatalogGrid* m_catalog_grid;
            CatalogUpdate* m_parent;
        };


        /** Combobox where user selects active catalog channel. */
        struct UrlChannel: public wxPanel, public Helpers
        {
            UrlChannel(wxWindow* parent, ActiveCatalogGrid* catalog_grid)
                : wxPanel(parent), Helpers(this), m_catalog_grid(catalog_grid)
            {
                auto sizer = new wxBoxSizer(wxHORIZONTAL);
                auto flags = wxSizerFlags()
                    .Expand().Border().Align(wxALIGN_CENTER_VERTICAL);
                sizer->Add(staticText(_("Catalog channel: ")), flags);
                auto catalog = CatalogHandler::getInstance();
                wxArrayString channel_list;
                for (auto channel: catalog->GetChannels()) {
                    channel_list.Add(channel.c_str());
                }
                auto channels = new wxChoice(this, wxID_ANY,
                                             wxDefaultPosition,
                                             wxDefaultSize,
                                             channel_list);
                auto current =
                    CatalogHandler::getInstance()->GetActiveChannel();
                int ix = channels->FindString(current.c_str());
                channels->SetSelection(ix != wxNOT_FOUND ? ix : 0);
                channels->Bind(
                        wxEVT_CHOICE,
                       [=](wxCommandEvent& e) { onChannelChange(e); });
                sizer->Add(channels, flags);
                SetSizer(sizer);
                Fit();
                Show();
            }

            void onChannelChange(wxCommandEvent& ev)
            {
                auto url = ev.GetString().ToStdString().c_str();
                CatalogHandler::getInstance()->SetActiveChannel(url);
                m_catalog_grid->ReloadAvailableVersion(url);
            };

            std::string m_active_channel;
            ActiveCatalogGrid* m_catalog_grid;
        };


        /** Custom url edit control, a text line. */
        struct UrlEdit: public wxPanel, public Helpers
        {
            UrlEdit(wxWindow* parent):  wxPanel(parent), Helpers(this)
            {
                auto sizer = new wxBoxSizer(wxVERTICAL);
                auto url_location = new wxBoxSizer(wxHORIZONTAL);
                auto flags = wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL);
                url_location->Add(staticText(_("Custom catalog URL: ")),
                                  flags);
                flags = wxSizerFlags().Expand().Border();
                sizer->Add(url_location, flags);

                auto url_edit = new wxBoxSizer(wxHORIZONTAL);
                auto uri = CatalogHandler::getInstance()->GetCustomUrl();
                m_url_ctrl = new wxTextCtrl(this, wxID_ANY, uri);
                auto the_parent = dynamic_cast<CatalogUpdate*>(GetParent());
                m_url_ctrl->SetMinClientSize(the_parent->getWindowSize());
                url_edit->Add(m_url_ctrl, wxSizerFlags().Expand());

                sizer->Add(url_edit, flags);
                SetSizer(sizer);
                Fit();
                Show();
            }

            void setText(std::string text)
            {
                m_url_ctrl->Clear();
                *m_url_ctrl  << text;
                m_url_ctrl->Refresh();
            }

            void clear() { m_url_ctrl->Clear(); }

            std::string getText()
            {
                return std::string(m_url_ctrl->GetLineText(0).ToStdString());
            }

            wxTextCtrl* m_url_ctrl;
        };
};


/** Download status/progress window, always shown at start-up. */
class CatalogLoad: public wxPanel, public Helpers
{
    public:

        class DialogGrid; // forward

        CatalogLoad(wxWindow* parent, bool use_latest = false)
            :wxPanel(parent), Helpers(this), m_simple(use_latest)
        {
            auto sizer = new wxBoxSizer(wxVERTICAL);
            auto flags = wxSizerFlags().Expand().Border() ;
            m_grid = new DialogGrid(this);
            sizer->Add(m_grid, flags);
            sizer->Add(1, 1, 1, wxEXPAND);   // Expanding spacer
            if (m_simple) {
                m_buttons = new Buttons(this);
                sizer->Add(m_buttons, flags.Bottom().Right());
            }
            auto size = GetTextExtent(_("Check latest release..."));
            size.SetHeight(size.GetHeight() * 10);
            size.SetWidth(size.GetWidth() * 5 / 2);
            SetMinClientSize(size);
            std::thread worker([=]() { Worker(); });
            worker.detach();

            SetSizer(sizer);
            Fit();
            Show();

            Bind(CHANNELS_DL_DONE,
                 [=](wxCommandEvent& ev) { m_grid->CellDone(ev, 1); });
            Bind(CHANNELS_PARSE_DONE,
                 [=](wxCommandEvent& ev) { m_grid->CellDone(ev, 3); });
            Bind(CATALOG_DL_DONE,
                 [=](wxCommandEvent& ev) { m_grid->CellDone(ev, 5); });
            Bind(CATALOG_PARSE_DONE,
                 [=](wxCommandEvent& ev) { workerDone(ev); });
        }

        void PostEvent(int evt_id, catalog_status status, std::string message)
        {
            wxCommandEvent evt(evt_id);
            evt.SetInt(static_cast<int>(status));
            evt.SetString(message);
            wxPostEvent(this, evt);
        }

        /** Last part of initial Worker() call, in main thread. */
        void workerDone(wxCommandEvent& ev)
        {
            m_grid->CellDone(ev, 5);
            if (m_simple) {
                std::string message =_("Catalog updated").ToStdString();
                std::string path =
                    g_Platform->GetPrivateDataDir().ToStdString();
                path += SEP;
                path += "ocpn-plugins.xml";
                std::ofstream stream;
                stream.open(path, std::ios::out);
                if (stream.is_open()) {
                    stream << m_xml.str();
                    stream.close();
                    ev.SetString("Catalog updated");
                    ev.SetInt(static_cast<int>(catalog_status::OK_MSG));
                }
                else {
                    ev.SetString("Update error (cannot writé to file)");
                    ev.SetInt(static_cast<int>(catalog_status::OS_ERROR));
                    wxLogMessage("Update error: Cannot write to %s",
                                 path.c_str());
                }
                m_grid->CellDone(ev, 6);
                m_buttons->ActivateOk();
            }
            else {
                CatalogData catalog_data;
                auto handler = CatalogHandler::getInstance();
                catalog_data = handler->LatestCatalogData();
                new CatalogUpdate(this);
            }
        }

        /** Runs in separate, detached thread, started from ctor. */
        void Worker()
        {
            auto catalog = CatalogHandler::getInstance();
            std::ostringstream json;

            auto status = catalog->LoadChannels(&json);
            PostEvent(CHANNELS_DL_DONE, status, catalog->LastErrorMsg());

            status = catalog->LoadChannels(json.str());
            PostEvent(CHANNELS_PARSE_DONE, status, catalog->LastErrorMsg());

            if (status == catalog_status::OK) {
                auto channel = catalog->GetActiveChannel();
                status = catalog_status::OK_MSG;
                PostEvent(CHANNELS_PARSE_DONE, status, channel);
            }
            m_xml.clear();
            status = catalog->DownloadCatalog(&m_xml);
            PostEvent(CATALOG_DL_DONE, status, catalog->LastErrorMsg());

            status = catalog->ParseCatalog(m_xml.str(), true);
            if (status == catalog_status::OK) {
                PostEvent(CATALOG_PARSE_DONE,
                          catalog_status::OK_MSG,
                          catalog->LatestCatalogData().version);
            }
            else {
                PostEvent(CATALOG_PARSE_DONE,
                          status,
                          catalog->LastErrorMsg());
            }
        }

        /** Grid with  Server is Reachable..., Check channel... etc. */
        struct DialogGrid: public wxPanel, public Helpers
        {
            DialogGrid(wxWindow* parent): wxPanel(parent), Helpers(this)
            {
                auto grid = new wxFlexGridSizer(2, 0, 0);
                auto flags = wxSizerFlags()
                    .Expand().Border().Align(wxALIGN_CENTER_VERTICAL);
                grid->Add(staticText(_("Server is reachable...")), flags);
                grid->Add(staticText(_("")), flags);
                grid->Add(staticText(_("Check channel...")), flags);
                grid->Add(staticText(_("")), flags);
                grid->Add(staticText(_("Check latest release...")), flags);
                grid->Add(staticText(_("")), flags);
                grid->Add(staticText(_("")), flags);
                grid->Add(staticText(_("")), flags);
                auto url = CatalogHandler::getInstance()->GetCustomUrl();
                if (url != "") {
                    grid->Add(staticText(_("Custom URL")), flags);
                    grid->Add(staticText(url.c_str()), flags);
                }
                SetSizer(grid);
                Fit();
                Show();
            }

            /* Update status values in grid. */
            void CellDone(const wxCommandEvent& event, size_t index)
            {
                //wxLogMessage("CellDone: event %d", event.GetInt());
                auto cell = GetSizer()->GetItem(index)->GetWindow();
                auto code = static_cast<catalog_status>(event.GetInt());
                if (code == catalog_status::OK) {
                    cell->SetLabel(_("OK"));
                }
                else if (code == catalog_status::OK_MSG) {
                    cell->SetLabel(event.GetString().ToStdString());
                }
                else {
                    auto msg = std::string(_("Fail: "))
                        + event.GetString().ToStdString();
                    cell->SetLabel(_(msg.c_str()));
                }
                Fit();
            }
        };

        /** OK/Cancel buttons for the download progress window. */
        struct Buttons: public wxPanel
        {
            Buttons(wxWindow* parent) : wxPanel(parent)
            {
                using CmdEvt = wxCommandEvent;

                auto sizer = new wxBoxSizer(wxHORIZONTAL);
                auto flags = wxSizerFlags().Right().Bottom().Border();
                sizer->Add(1, 1, 100, wxEXPAND);   // Expanding spacer
                auto cancel = new wxButton(this, wxID_CANCEL, _("Cancel"));
                sizer->Add(cancel, flags);
                m_ok = new wxButton(this, wxID_OK, _("OK"));
                m_ok->Enable(false);
                sizer->Add(m_ok, flags);
                SetSizer(sizer);
                Fit();
                SetFocus();
                Show();
            }

            void ActivateOk()
            {
                m_ok->Enable(true);
            }

            wxButton* m_ok;
        };

        std::ostringstream m_xml;
        DialogGrid* m_grid;
        Buttons* m_buttons;
        const bool m_simple;   // Simple means just install, no advanced dialog
};


}     // namespace catalog_mgr


/** Top-level plugin catalog dialog. */
AdvancedCatalogDialog::AdvancedCatalogDialog(wxWindow* parent)
    :wxFrame(parent, wxID_ANY, _("Catalog Manager"),
              wxDefaultPosition , wxDefaultSize,
              wxDEFAULT_FRAME_STYLE | wxRESIZE_BORDER)
{
    auto vbox = new wxBoxSizer(wxHORIZONTAL);
    vbox->Add(new catalog_mgr::CatalogLoad(this, false),
              wxSizerFlags(1).Expand());
    Bind(wxEVT_CLOSE_WINDOW, [this](wxCloseEvent& e) { Destroy(); });
    Bind(CATALOG_DLG_CLOSE, [this](wxCommandEvent& ev) { Destroy(); });
    SetSizer(vbox);

    Fit();
    Center();
    Show();
}

SimpleCatalogDialog::SimpleCatalogDialog(wxWindow* parent)
    :wxDialog(parent, wxID_ANY, _("Catalog Manager"),
              wxDefaultPosition , wxDefaultSize,
              wxDEFAULT_FRAME_STYLE | wxRESIZE_BORDER)
{
    auto vbox = new wxBoxSizer(wxHORIZONTAL);
    vbox->Add(new catalog_mgr::CatalogLoad(this, true),
              wxSizerFlags(1).Expand());
    Bind(wxEVT_CLOSE_WINDOW, [&](wxCloseEvent& e) { EndModal(wxID_OK); });
    Bind(CATALOG_DLG_CLOSE, [&](wxCommandEvent& ev) { EndModal(wxID_OK); });
    SetSizer(vbox);

    Fit();
    ShowModal();
    Destroy();
}
