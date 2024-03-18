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

/**
 * \file
 *
 * The local API has a server side handling commands and a client part
 * issuing commands.
 *
 * The commands supported are:
 *
 *   - Raise:  Bring OpenCPN to top of the GUI stack.
 *   - Open <filename>: Open a GPX waypoint or route file.
 *   - Quit: Exit server application.
 *   - GetRestEndpoint: Returns an address/port tuple string like
 *     128.0.0.1/1503 giving the adress/port used by the REST server.
 *
 * Besides GetRestAddress all commands returns a std::pair with a boolean and
 * a string. The boolean indicates successful completion, the string is either
 * an error message or, for GetRestEndpoint, the address/port endpoint.
 */

#ifndef LOCAL_SERVER_API__
#define LOCAL_SERVER_API__

#include <wx/cmdline.h>

#include "observable_evtvar.h"

using LocalApiResult = std::pair<bool, std::string>;

enum class CmdlineAction { Raise, Quit, Open, GetRestEndpoint, Fail, Skip };


class LocalApiException :  public std::exception {
public:
  LocalApiException(const std::string why) : reason(why) {}

  const char* str() { return reason.c_str(); }
private:
  std::string reason;
};


/** Base interface for local server command handling. */
class LocalServerApi {
public:

  /** @return Reference to a LocalServerApi implementation. */
  static LocalServerApi& GetInstance();

  /** Release Instance */
  static void ReleaseInstance();

  /** Notified on the Raise command. */
  EventVar on_raise;

  /** Notified on the Quit command. */
  EventVar on_quit;

  /** Callback invoked on open command with a file path argument. */
  std::function<bool(const std::string&)> open_file_cb;

  /** Set callback returning the rest server root endpoint. */
  virtual void SetGetRestApiEndpointCb(std::function<std::string()> cb) {
    get_rest_api_endpoint_cb = cb;
  }

  std::function<std::string()> get_rest_api_endpoint_cb;

protected:
  LocalServerApi()
    : get_rest_api_endpoint_cb([](){ return "0.0.0.0/1024"; }) {}

};

/** Base interface for local clients. */
class LocalClientApi {
public:
  static std::unique_ptr<LocalClientApi> GetClient();

  LocalClientApi() = default;
  virtual ~LocalClientApi() = default;


  virtual LocalApiResult HandleCmdline(const wxCmdLineParser& parser);
  virtual LocalApiResult HandleCmdline(CmdlineAction action, const std::string& arg);

  virtual LocalApiResult SendRaise() = 0;
  virtual LocalApiResult SendOpen(const char* path) = 0;
  virtual LocalApiResult SendQuit() = 0;
  virtual LocalApiResult GetRestEndpoint() = 0;

protected:
  CmdlineAction ParseArgs(const wxCmdLineParser& parser, std::string& arg);
};

#endif   //  LOCAL_SERVER_API__
