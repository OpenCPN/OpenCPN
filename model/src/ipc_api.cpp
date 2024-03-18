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
 ***************************************************************************/

#include <sstream>

#include <wx/filename.h>

#include "model/ipc_api.h"
#include "model/base_platform.h"
#include "model/logger.h"
#include "model/ocpn_utils.h"

IpcServer* IpcConnection::s_instance = nullptr;

// FIXME (leamas) Bad name
std::string GetSocketPath() {
  auto const static sep = static_cast<char>(wxFileName::GetPathSeparator());
  auto dirpath = g_BasePlatform->GetPrivateDataDir();
  if (!wxFileName::DirExists(dirpath)) wxFileName::Mkdir(dirpath);
  return dirpath.ToStdString() + sep + "opencpn-ipc";
}

IpcClient::IpcClient(const std::string& path) {
  connection = MakeConnection("localhost", path, "OpenCPN");
  if (!connection)
    throw LocalApiException(std::string("Cannot connect to: ") + path);
};


LocalApiResult IpcClient::SendQuit() {
  if (connection->Execute(wxString("quit"))) {
    return LocalApiResult(true, "");
  } else {
    return LocalApiResult(false, "Server error running quit command");
  }
}


LocalApiResult IpcClient::SendRaise() {
  if (connection->Execute(wxString("raise"))) {
    return LocalApiResult(true, "");
  } else {
    return LocalApiResult(false, "Server error running raise command");
  }
}


LocalApiResult IpcClient::SendOpen(const char* path) {
  const  void* reply = connection->Request(wxString("open " ) + path);
  if (reply) return LocalApiResult(true, static_cast<const char*>(reply));
  return LocalApiResult(false, "");
}


LocalApiResult IpcClient::GetRestEndpoint() {
  const  void* reply = connection->Request("get_rest_endpoint");
  if (reply) {
    return LocalApiResult(true, static_cast<const char*>(reply));
  }
  return LocalApiResult(false, "Server error running get_rest_endpoint");
}

LocalServerApi& IpcConnection::GetInstance() {
  if (!s_instance) s_instance = new IpcServer(GetSocketPath());
  return *s_instance;
}

void IpcConnection::ReleaseInstance() {
  if (s_instance) {
    delete s_instance;
    s_instance = nullptr;
  }
}

bool IpcConnection::OnExec(const wxString&, const wxString& data) {
  if (data == "quit") {
    server.on_quit.Notify();
    return true;
  } else if (data == "raise") {
    server.on_raise.Notify();
    return true;
  } else {
    return false;
  }
}

const void* IpcConnection::OnRequest(const wxString& topic,
                                 const wxString& item, size_t* size,
                                 wxIPCFormat format) {
  if (format != wxIPC_TEXT) return 0;

  std::string line = item.ToStdString();
  if (ocpn::startswith(line, "get_rest_endpoint")) {
    buffer = server.get_rest_api_endpoint_cb();
    if (size) *size = buffer.size();
    return buffer.c_str();
  } else if( ocpn::startswith(line, "open")) {
    auto words = ocpn::split(line.c_str(), " ");
    if (words.size() != 2) {
      wxLogWarning("Illegal open cmd line: %s", line.c_str());
      return 0;
    }
    bool ok = server.open_file_cb(words[1]);
    const char* reply = ok ? "ok" : "fail";
    if (size) *size = strlen(reply);
    return reply;
  } else {
    wxLogWarning("Illegal cmd line: %s", line.c_str());
    return 0;
  }
}
