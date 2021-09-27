/**
 * Safe mode handling.
 *
 * Administrate the safe mode state which is true if openpcn should run in
 * safe mode without OpenGL, plugins, etc.
 */

#include <wx/window.h>

#include "OCPNPlatform.h"
#include "ocpn_utils.h"

namespace safe_mode {

bool get_mode();

void set_mode(bool mode);

/**
 * Check if the last start failed, possibly invoke user dialog and set
 * safe mode state.
 */
void check_last_start();

/** Mark last run as successful. */
void clear_check();

}  // namespace safe_mode
