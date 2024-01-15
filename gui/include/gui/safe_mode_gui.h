/**
 * Safe mode GUI handling.
 *
 * Administrate the safe mode state which is true if openpcn should run in
 * safe mode without OpenGL, plugins, etc.
 */

#include <wx/window.h>

#include "OCPNPlatform.h"
#include "model/safe_mode.h"

namespace safe_mode {

/**
 * Check if the last start failed, possibly invoke user dialog and set
 * safe mode state.
 */
void check_last_start();

}
