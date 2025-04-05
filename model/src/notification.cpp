/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Notification Object
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2025 by David S. Register                               *
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
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 **************************************************************************/
#include <cmath>
#include <memory>
#include <vector>
#include <wx/datetime.h>

#include "model/notification.h"

Notification::Notification(NotificationSeverity _severity,
                           const std::string &_message, int _timeout_secs) {
  severity = _severity;
  message = _message;
  activate_time = wxDateTime::Now().GetTicks();
  message_hash = std::hash<std::string>{}(_message);
  auto_timeout_secs = _timeout_secs;
}
