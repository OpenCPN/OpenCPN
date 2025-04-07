/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Notification Manager
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

#include "model/navutil_base.h"
#include "model/notification.h"
#include "model/notification_manager.h"

NotificationManager& NotificationManager::GetInstance() {
  static NotificationManager instance;
  return instance;
}

NotificationManager::NotificationManager() {
  m_timeout_timer.Bind(wxEVT_TIMER, &NotificationManager::OnTimer, this,
                       m_timeout_timer.GetId());
  m_timeout_timer.Start(1000, wxTIMER_CONTINUOUS);
}

void NotificationManager::OnTimer(wxTimerEvent& event) {
  for (auto note : active_notifications) {
    if (note->GetTimeoutCount() > 0) {
      note->DecrementTimoutCount();
    }
  }
  for (auto note : active_notifications) {
    if (note->GetTimeoutCount() == 0) {
      AcknowledgeNotification(note->GetGuid());
      note->DecrementTimoutCount();
      break;
    }
  }
}

NotificationSeverity NotificationManager::GetMaxSeverity() {
  int rv = 0;
  for (auto note : active_notifications) {
    int severity = static_cast<int>(note.get()->GetSeverity());
    if (severity > rv) rv = severity;
  }
  return static_cast<NotificationSeverity>(rv);
}

std::string NotificationManager::AddNotification(
    std::shared_ptr<Notification> _notification) {
  _notification.get()->SetGuid(GpxDocument::GetUUID().ToStdString());
  active_notifications.push_back(_notification);
  evt_notificationlist_change.Notify();
  return _notification.get()->GetGuid();
}

std::string NotificationManager::AddNotification(NotificationSeverity _severity,
                                                 const std::string& _message,
                                                 int _timeout_secs) {
  auto notification =
      std::make_shared<Notification>(_severity, _message, _timeout_secs);
  notification.get()->SetGuid(GpxDocument::GetUUID().ToStdString());
  active_notifications.push_back(notification);
  evt_notificationlist_change.Notify();
  return notification.get()->GetGuid();
}

bool NotificationManager::AcknowledgeNotification(std::string GUID) {
  if (!GUID.length()) return false;

  size_t target_message_hash = 0;
  for (auto it = active_notifications.begin();
       it != active_notifications.end();) {
    if ((*it)->GetGuid() == GUID) {
      target_message_hash = (*it)->GetStringHash();
      break;
    } else
      ++it;
  }
  if (!target_message_hash) return false;

  // erase multiple notifications with identical message_hash
  bool rv = false;
  bool done = false;
  while (!done && active_notifications.size()) {
    for (auto it = active_notifications.begin();
         it != active_notifications.end();) {
      if ((*it)->GetStringHash() == target_message_hash) {
        active_notifications.erase(it);
        rv = true;
        break;
      } else
        ++it;
      if (it == active_notifications.end()) done = true;
    }
  }

  if (rv) evt_notificationlist_change.Notify();

  return rv;
}
