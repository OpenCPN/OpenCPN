/**
 * Safe mode GUI handling.
 *
 * Administrate the safe mode state which is true if openpcn should run in
 * safe mode without OpenGL, plugins, etc.
 *
 * Copyright (c) 2023 Alec Leamas
 * License: GPL version 2 or later.
 */

#include <wx/window.h>

#include "ocpn_platform.h"
#include "model/safe_mode.h"

namespace safe_mode {

/**
 * Check if the last start failed, possibly invoke user dialog and set
 * safe mode state.
 */
void check_last_start();

}  // namespace safe_mode
