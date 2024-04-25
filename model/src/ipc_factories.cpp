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
 **************************************************************************/

/** \file Local communications factories
 *
 * Factory methods which returns dbus or wxwidgets based implementations
 * of instance checkers, servers and clients.
 */

#include <stdlib.h>

#include "model/ipc_api.h"
#include "model/wx_instance_chk.h"

#if defined(__linux__)  && !defined(__ANDROID__)
#include "model/dbus_client.h"
#include "model/dbus_server.h"
#endif

static InstanceCheck& GetWxInstanceChk() {
  static WxInstanceCheck wx_check;
  return wx_check;
}

#ifdef __ANDROID__

std::unique_ptr<LocalClientApi> LocalClientApi::GetClient() {
  return std::unique_ptr<LocalClientApi>(new DummyIpcClient());
}

LocalServerApi& LocalServerApi:: GetInstance() {
  return DummyIpcServer::GetInstance();
}

void LocalServerApi::ReleaseInstance() {}

InstanceCheck& InstanceCheck::GetInstance() {
  return DummyInstanceChk::GetInstance();
}


#elif defined(__linux__)
static bool UseDbus() {
  return getenv("FLATPAK_ID") != 0 || getenv("OCPN_FORCE_DBUS");
}

std::unique_ptr<LocalClientApi> LocalClientApi::GetClient() {
  if (UseDbus()) {
    return std::unique_ptr<LocalClientApi>(new DbusLocalClient);
  } else {
    return std::unique_ptr<LocalClientApi>(new IpcClient);
  }
}

LocalServerApi& LocalServerApi:: GetInstance() {
  return UseDbus() ? DbusServer::GetInstance() : IpcConnection::GetInstance();
}

void LocalServerApi::ReleaseInstance() {}

InstanceCheck& InstanceCheck::GetInstance() {
  if (UseDbus())
    return DbusServer::GetInstance();
  else
    return GetWxInstanceChk();
}

#else  // __linux__ nor __ANDROID__
std::unique_ptr<LocalClientApi> LocalClientApi::GetClient() {
  return std::unique_ptr<LocalClientApi>(new IpcClient());
}

LocalServerApi& LocalServerApi:: GetInstance() {
  return IpcConnection::GetInstance();
}

void LocalServerApi::ReleaseInstance() { IpcConnection::ReleaseInstance(); }

InstanceCheck& InstanceCheck::GetInstance() {
  return GetWxInstanceChk();
}

#endif    // __linux__
