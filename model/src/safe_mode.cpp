#include <cstdio>
#include <string>
#include <fstream>

#include <wx/dialog.h>
#include <wx/filename.h>
#include <wx/sizer.h>

#include "model/base_platform.h"
#include "model/cmdline.h"
#include "model/ocpn_utils.h"

#include "model/safe_mode.h"

namespace safe_mode {

#ifdef _WIN32
static std::string SEP("\\");
#else
static std::string SEP("/");
#endif


bool safe_mode = false;

std::string check_file_path() {
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


/** Mark last run as successful. */
void clear_check() { remove(check_file_path().c_str()); }

}  // namespace safe_mode
