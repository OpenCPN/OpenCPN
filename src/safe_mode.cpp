#include <cstdio>
#include <string>
#include <fstream>

#include <wx/dialog.h>
#include <wx/filename.h>
#include <wx/sizer.h>

#include "OCPNPlatform.h"
#include "ocpn_utils.h"

#include "safe_mode.h"

extern OCPNPlatform*            g_Platform;
extern bool                     g_bdisable_opengl;

extern wxImage LoadSVGIcon(wxString filename, int width, int height);


namespace safe_mode {

static const char* LAST_RUN_ERROR_MSG = \
    _("The last opencpn run seems to have failed. Do you want to run\n"
    "in safe mode without plugins and other possibly problematic\n"
    "features?\n");

#ifdef _WIN32
static std::string SEP("\\");
#else
static std::string SEP("/");
#endif


static const int TIMEOUT_SECONDS = 15;


static bool safe_mode = false;


static std::string check_file_path()
{
    std::string path = g_Platform->GetPrivateDataDir().ToStdString();
    path += SEP;
    path += "startcheck.dat";
    return path;
}


static void LoadIcon(const char* plugin_name, wxBitmap& bitmap, int size=32)
{
    wxFileName path(g_Platform->GetSharedDataDir(), plugin_name);
    path.AppendDir("uidata");
    path.AppendDir("markicons");
    bool ok = false;
    path.SetExt("svg");
    if (path.IsFileReadable()) {
        wxImage img = LoadSVGIcon(path.GetFullPath(), size, size);
        bitmap = wxBitmap(img);
        ok = bitmap.IsOk();
    }
    if (!ok) {
        wxLogMessage("Cannot load Symbol-Question-Black.svg icon");
    }
}


class SafeModeDialog: public wxDialog
{

    public:
        SafeModeDialog() :
            wxDialog(0, wxID_ANY, "Safe restart") 
        {
            timer.Bind(wxEVT_TIMER, [&](wxTimerEvent) { EndModal(wxID_NO); });
            timer.StartOnce(TIMEOUT_SECONDS * 1000);

            wxBitmap bitmap;
            LoadIcon("Symbol-Question-Black.svg", bitmap, 64);
            auto static_bitmap = new wxStaticBitmap(this, wxID_ANY, bitmap);

            auto hbox = new wxBoxSizer(wxHORIZONTAL);
            hbox->Add(static_bitmap);
            hbox->Add(new wxStaticText(this, wxID_ANY,  LAST_RUN_ERROR_MSG),
                      wxSizerFlags().Expand());

            auto no_btn = new wxButton(this, wxID_NO);
            no_btn->SetFocus();
            no_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED,
                         [&](wxCommandEvent&) { EndModal(wxID_NO); } );

            auto yes_btn = new wxButton(this, wxID_YES);
            yes_btn->Bind(wxEVT_COMMAND_BUTTON_CLICKED,
                          [&](wxCommandEvent&) { EndModal(wxID_YES); } );

            auto buttons = new wxStdDialogButtonSizer();
            buttons->Add(no_btn, wxSizerFlags().Border());
            buttons->Add(yes_btn, wxSizerFlags().Border());

            auto vbox = new wxBoxSizer(wxVERTICAL);
            vbox->Add(hbox);
            vbox->Add(buttons);

            SetSizer(vbox);
            auto size = GetTextExtent(
                "in safe mode without plugins and other possibly problematic");
            SetMinClientSize(wxSize(size.GetWidth() * 12 / 10,
                             size.GetHeight() * 8));
            SetMaxClientSize(wxSize(-1, size.GetHeight() * 8));
            Layout();
            Show();
        };

    private:
        wxTimer timer;
};


void set_mode(bool mode)
{
    safe_mode = mode;
    g_bdisable_opengl = g_bdisable_opengl || mode;
}


bool get_mode() { return safe_mode; }


/** 
 * Check if the last start failed, possibly invoke user dialog and set
 * safe mode state.
 */
void check_last_start()
{
    std::string path = check_file_path();
    if (!ocpn::exists(path)) {
        std::ofstream dest(path, std::ios::binary);
        dest << "Internal opencpn use" << std::endl;
        dest.close();
        return;
    }
    auto dlg = new SafeModeDialog();
    int reply = dlg->ShowModal();
    safe_mode = reply == wxID_YES;
    dlg->Destroy();
}


/** Mark last run as successful. */
void clear_check()
{
    remove(check_file_path().c_str());
}


}  // namespace safe_mode
