/**************************************************************************
 *   Copyright (C) 2022 Alec Leamas                                        *
 *   Copyright (C) 2022 David Register                                     *
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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

/**
 * \file
 *
 * JSON event definition used in internal communications to/from plugins.
 */

#ifndef JSON_EVT_H_
#define JSON_EVT_H_

#include <memory>
#include <string>

#include <wx/event.h>

#include "observable_evtvar.h"

/** Handle sending of wxJson messages to all plugins. */
class JsonEvent {
public:
  static JsonEvent& getInstance() {
    static JsonEvent instance;
    return instance;
  }

  void Notify(const std::string& message, std::shared_ptr<void> msg_root) {
    event.Notify(msg_root, message, 0);
  }

private:
  EventVar event;
};

#endif
