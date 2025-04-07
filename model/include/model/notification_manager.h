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
#ifndef _NOTIFICATION_MANAGER_H__
#define _NOTIFICATION_MANAGER_H__

#include <wx/timer.h>
#include "notification.h"
#include "observable_evtvar.h"

class NotificationManager {
public:
  static NotificationManager& GetInstance();

  NotificationManager();

  std::string AddNotification(std::shared_ptr<Notification> _notification);
  std::string AddNotification(NotificationSeverity _severity,
                              const std::string& _message,
                              int timeout_secs = -1);

  bool AcknowledgeNotification(std::string GUID);
  NotificationSeverity GetMaxSeverity();
  void OnTimer(wxTimerEvent& event);

  std::vector<std::shared_ptr<Notification>> GetNotifications() {
    return active_notifications;
  }
  size_t GetNotificationCount() { return active_notifications.size(); }
  EventVar evt_notificationlist_change;

private:
  std::vector<std::shared_ptr<Notification>> active_notifications;
  wxTimer m_timeout_timer;
};

#endif
