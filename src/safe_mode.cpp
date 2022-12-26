#include <cstdio>
#include <string>
#include <fstream>

#include <wx/dialog.h>
#include <wx/filename.h>
#include <wx/sizer.h>

#include "base_platform.h"
#include "ocpn_utils.h"
#include "gui_lib.h"

#include "safe_mode.h"

extern BasePlatform* g_BasePlatform;
extern bool g_bdisable_opengl;

namespace safe_mode {

static const char* LAST_RUN_ERROR_MSG =
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

static std::string check_file_path() {
  std::string path = g_BasePlatform->GetPrivateDataDir().ToStdString();
  path += SEP;
  path += "startcheck.dat";
  return path;
}

void set_mode(bool mode) {
  safe_mode = mode;
  g_bdisable_opengl = g_bdisable_opengl || mode;
}

bool get_mode() { return safe_mode; }

/**
 * Check if the last start failed, possibly invoke user dialog and set
 * safe mode state.
 */
void check_last_start() {
  std::string path = check_file_path();
  if (!ocpn::exists(path)) {
    std::ofstream dest(path, std::ios::binary);
    dest << "Internal opencpn use" << std::endl;
    dest.close();
    return;
  }
  long style = wxYES | wxNO | wxNO_DEFAULT | wxICON_QUESTION;
  auto dlg = new OCPN_TimedHTMLMessageDialog(0, LAST_RUN_ERROR_MSG,
                                             _("Safe restart"), TIMEOUT_SECONDS,
                                             style, false, wxDefaultPosition);
  int reply = dlg->ShowModal();
  safe_mode = reply == wxID_YES;
  dlg->Destroy();
}

/** Mark last run as successful. */
void clear_check() { remove(check_file_path().c_str()); }

}  // namespace safe_mode
