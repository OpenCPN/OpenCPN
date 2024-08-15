#include <cstdio>
#include <string>
#include <fstream>

#include <wx/dialog.h>
#include <wx/filename.h>
#include <wx/sizer.h>

#include "model/cmdline.h"
#include "gui_lib.h"
#include "model/ocpn_utils.h"

#include "model/safe_mode.h"

namespace safe_mode {

static const int TIMEOUT_SECONDS = 15;

static const char* LAST_RUN_ERROR_MSG =
    _("<p>The last opencpn run seems to have failed. Do you want to run\n"
      "in safe mode without plugins and other possibly problematic\n"
      "features?\n</p><br/></br><p>You may consider visiting the <a "
      "href=\"https://github.com/OpenCPN/OpenCPN/wiki/"
      "OpenCPN-5.8-known-issues\">list of known issues</a>.</p>");

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

}  // namespace safe_mode
