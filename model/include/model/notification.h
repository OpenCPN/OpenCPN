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
#ifndef _NOTIFICATION_H__
#define _NOTIFICATION_H__

#include <string>

enum class NotificationSeverity : int {
  kInformational = 0,
  kWarning = 1,
  kCritical = 2
};

class Notification {
public:
  Notification(NotificationSeverity _severity, const std::string &_message,
               int _timeout_secs = -1);
  virtual ~Notification() {}

  std::string GetMessage() { return message; }
  NotificationSeverity GetSeverity() { return severity; }
  time_t GetActivateTime() { return activate_time; }
  std::string GetGuid() { return guid; }
  void SetGuid(std::string _guid) { guid = _guid; }
  size_t GetStringHash() { return message_hash; }
  int GetTimeoutCount() { return auto_timeout_secs; }
  void DecrementTimoutCount() { auto_timeout_secs--; }

private:
  NotificationSeverity severity;
  std::string message;
  std::string guid;
  time_t activate_time;
  size_t message_hash;
  int auto_timeout_secs;
};
#endif
