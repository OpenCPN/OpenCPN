/**************************************************************************
 *   Copyright (C) 2024 by David S. Register                               *
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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

/**
 * \file
 *
 * ocpn_plugin.h HostApi122 implementation
 */

#include <wx/app.h>

#include "ocpn_plugin.h"

// FIXME (leamas) find new home.
std::unique_ptr<HostApi> GetHostApi() {
  auto impl = dynamic_cast<Api122Impl*>(wxTheApp);
  assert(impl && "wxTheApp does not implement Api122Impl");
  return std::make_unique<HostApi122>(HostApi122(impl));
}

void HostApi122::RegisterApiEventCallback(
    const std::string& plugin_name, std::function<void(EventType)> callback) {
  auto impl = dynamic_cast<Api122Impl*>(wxTheApp);
  assert(impl && "wxTheApp does not implement Api122Impl");
  impl->RegisterApiEventCallback(plugin_name, callback);
}
