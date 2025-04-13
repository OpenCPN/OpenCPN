/**************************************************************************
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

/**
 * \file
 * Class NotificationManager.
 */

#ifndef _NOTIFICATION_MANAGER_H__
#define _NOTIFICATION_MANAGER_H__

#include <wx/timer.h>
#include "notification.h"
#include "observable_evtvar.h"

/** The global list of user notifications, a singleton. */
class NotificationManager {
public:
  static NotificationManager& GetInstance();

  NotificationManager(const NotificationManager&) = delete;
  NotificationManager& operator=(const NotificationManager&) = delete;

  std::string AddNotification(std::shared_ptr<Notification> _notification);

  /**
   * Add a new notification.
   * @param severity Notification type.
   * @param message string, possibly multiline.
   * @oaram timeout_secs If given, the notifications is automatically
   *     removed after timeout_secs seconds if still existing.
   */
  std::string AddNotification(NotificationSeverity _severity,
                              const std::string& _message,
                              int timeout_secs = -1);
  /**
   * User ack on a notification which eventually will remove it
   * @param guid  Guid obtained from AddNotification() or GetUUID().
   * @return true if the notification corresponding to guid exists.
   */
  bool AcknowledgeNotification(const std::string& guid);

  /** Return max severity among current active notifications. */
  NotificationSeverity GetMaxSeverity();

  /** Return current active notifications. */
  const std::vector<std::shared_ptr<Notification>>& GetNotifications() const {
    return active_notifications;
  }

  size_t GetNotificationCount() const { return active_notifications.size(); }

  /** Notified without data when a notification is added or removed. */
  EventVar evt_notificationlist_change;

private:
  NotificationManager();
  void PersistNotificationAsFile(
      const std::shared_ptr<Notification> _notification);

  void OnTimer(wxTimerEvent& event);

  std::vector<std::shared_ptr<Notification>> active_notifications;
  wxTimer m_timeout_timer;
};

#endif
