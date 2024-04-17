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

#ifndef _IPC_API_H__
#define _IPC_API_H__

#include <wx/cmdline.h>
#include <wx/ipc.h>
#include <wx/log.h>

#include "model/local_api.h"

/**
 * \file  ipc-api.h
 * Server and client CLI api implementations.
 */

std::string GetSocketPath();

/**
 * Implement LocalClientApi using a filesystem fifo/socket.
 */
class IpcClientConnection : public  wxConnection {
friend class IpcClient;

public:


private:
  IpcClientConnection() : wxConnection() {}
};


class IpcClient : public wxClient, public LocalClientApi {
public:

  IpcClient(const std::string& path);

  IpcClient() : IpcClient(GetSocketPath()) {}

  LocalApiResult SendRaise();
  LocalApiResult SendOpen(const char* path);
  LocalApiResult SendQuit();
  LocalApiResult GetRestEndpoint();
  wxConnectionBase* OnMakeConnection() { return new IpcClientConnection; }

private:
  wxConnectionBase* connection;
};


class IpcServer;  // forward

/**
 * Started by IpcServer on filesystem fifo/socket connects.
 */
class IpcConnection: public wxConnection {
friend  class IpcServer;

public:
  static LocalServerApi& GetInstance();
  static void ReleaseInstance();

  IpcConnection(IpcConnection&) = delete;
  void operator= (const IpcConnection&) = delete;


  IpcServer& server;

  /** Handle commands without reply: quit and raise. */
  bool OnExec(const wxString&, const wxString& data);

  /**
   * Handle commands with a reply
   *   - open <path>, returns "ok" or "fail"
   *   - get_api_endpoint, returns endpoint string.
   */
  const void* OnRequest(const wxString& topic, const wxString& item,
                        size_t* size, wxIPCFormat format) ;

protected:
  IpcConnection(IpcServer& s) : server(s) {}

private:
  std::string buffer;
  static IpcServer* s_instance;
};

/**
 * Implement LocalServerApi using a filesystem fifo/socket
 */
class IpcServer : public wxServer, public LocalServerApi {
public:
   const bool is_connected;

   IpcServer(const std::string& path)
       : wxServer(), is_connected(Create(path)) {}

   IpcServer() : IpcServer(GetSocketPath()) {}

   wxConnectionBase* OnAcceptConnection(const wxString& topic) {
     return new IpcConnection(*this);
   }

   /** void, we are serving as long as there is a ServerFactory. */
   void Serve() {}
};

/**
 * Useless place holder for LocalServerApi
 */
class DummyIpcServer : public LocalServerApi {
public:
   static DummyIpcServer& GetInstance() {
     static DummyIpcServer server;
     return server;
   }

   DummyIpcServer() {}
   DummyIpcServer(const std::string& path) {}

   wxConnectionBase* OnAcceptConnection(const wxString& topic) {
     assert(false && "OnAcceptConnection called in DummyIpcServer");
     return nullptr;   // not reachable, for the compiler
   }

   void Serve() {}
};

class DummyIpcClient :  public LocalClientApi {
public:

  DummyIpcClient(const std::string& path) {}

  DummyIpcClient() {}

  LocalApiResult SendRaise() {
    return LocalApiResult(false, "raise command not implemented");
  }

  LocalApiResult SendOpen(const char* path) {
    return LocalApiResult(false, "open command not implemented");
  }

  LocalApiResult SendQuit() {
    return LocalApiResult(false, "quit command not implemented");
  }

  LocalApiResult GetRestEndpoint() {
    return LocalApiResult(false, "get_rest_endpoint command not implemented");
  }

  wxConnectionBase* OnMakeConnection() {
    assert(false && "OnMakeConnection called in DummyIpcServer");
    return nullptr;   // not reachable, for the compiler
  }
};



#endif  // _IPC_API_H__
