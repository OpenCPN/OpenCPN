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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

/**
 * \file
 *
 * User  notification container
 */

#ifndef NOTIFICATION_H_
#define NOTIFICATION_H_

#include <string>

enum class NotificationSeverity : int {
  kInformational = 0,
  kWarning = 1,
  kCritical = 2
};

/** User visible notification. */
class Notification {
public:
  Notification(NotificationSeverity _severity, const std::string &_message,
               int _timeout_secs = -1);
  virtual ~Notification() = default;

  std::string GetMessage() { return message; }
  NotificationSeverity GetSeverity() const { return severity; }
  time_t GetActivateTime() const { return activate_time; }
  std::string GetGuid() const { return guid; }
  size_t GetStringHash() const { return message_hash; }
  int GetTimeoutStart() const { return auto_timeout_start; }
  int GetTimeoutLeft() const { return auto_timeout_left; }
  void DecrementTimoutCount() { auto_timeout_left--; }

private:
  NotificationSeverity severity;
  const std::string message;
  const time_t activate_time;
  const std::string guid;
  const size_t message_hash;
  int auto_timeout_start;
  int auto_timeout_left;
};

#endif
