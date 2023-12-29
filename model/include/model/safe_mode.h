/**
 * Safe mode handling.
 *
 * Administrate the safe mode state which is true if openpcn should run in
 * safe mode without OpenGL, plugins, etc.
 */

#include <wx/window.h>

#include "ocpn_utils.h"

namespace safe_mode {

extern bool safe_mode;

std::string check_file_path();

bool get_mode();

void set_mode(bool mode);

/** Mark last run as successful. */
void clear_check();

}  // namespace safe_mode
