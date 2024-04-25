/***************************************************************************
 *   Copyright (C) 2023 Alec Leamas                                        *
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
 ***************************************************************************
 */

#ifndef DBUS_LOCAL_API_H__
#define DBUS_LOCAL_API_H__

#include <memory>
#include <string>
#include <thread>
#include <utility>

#include "model/dbus_server.h"
#include "model/local_api.h"

/** Implement LocalClientApi on linux using Dbus mechanisms.  */
class DbusLocalClient : public LocalClientApi {
public:
  DbusLocalClient() = default;
  virtual ~DbusLocalClient() = default;

  LocalApiResult SendRaise();

  LocalApiResult SendOpen(const char* path);

  LocalApiResult SendQuit();

  LocalApiResult GetRestEndpoint();
};

#endif  // DBUS_LOCAL_API_H__
