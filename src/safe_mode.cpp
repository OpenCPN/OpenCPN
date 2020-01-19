#include <cstdio>
#include <string>
#include <fstream>

#include <wx/dialog.h>

#include "OCPNPlatform.h"
#include "ocpn_utils.h"

#include "safe_mode.h"

extern OCPNPlatform*            g_Platform;
extern bool                     g_bdisable_opengl;

namespace safe_mode {

static bool safe_mode = false;

bool get_mode() { return safe_mode; }

void set_mode(bool mode)
{
    safe_mode = mode;
    g_bdisable_opengl = g_bdisable_opengl || mode;
}

static const char* LAST_RUN_ERROR_MSG = \
    "The last opencpn run seems to have failed. Do you want to run\n"
    "in safe mode without plugins and other possibly problematic\n"
    "features?";


#ifdef _WIN32
static std::string SEP("\\");
#else
static std::string SEP("/");
#endif


static std::string check_file_path() 
{
    std::string path = g_Platform->GetPrivateDataDir().ToStdString();
    path += SEP;
    path += "startcheck.dat";
    return path;
}


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
    auto dlg = new wxMessageDialog(0,  _(LAST_RUN_ERROR_MSG), "",
                                   wxYES_NO | wxCENTRE | wxICON_QUESTION);
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
