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
#include <iostream>
#include <string>

#include "model/local_api.h"
#include "model/ocpn_utils.h"
#include "model/logger.h"

#ifdef __ANDROID__

CmdlineAction LocalClientApi::ParseArgs(const wxCmdLineParser& parser,
                                        std::string& arg) {
  return CmdlineAction::Skip;
}

#else
CmdlineAction LocalClientApi::ParseArgs(const wxCmdLineParser& parser,
                                        std::string& arg) {
  CmdlineAction result = CmdlineAction::Fail;
  arg = "";
  if (parser.GetParamCount() == 0) {
    result = CmdlineAction::Raise;
  } else if (parser.GetParamCount() == 1) {
    if (parser.GetParam(0) == "raise") {
      result = CmdlineAction::Raise;
    } else if (parser.GetParam(0) == "quit") {
      result = CmdlineAction::Quit;
    } else if (parser.GetParam(0) == "get_rest_endpoint") {
      result = CmdlineAction::GetRestEndpoint;
    } else if (ocpn::exists(parser.GetParam(0).ToStdString().c_str())) {
      result = CmdlineAction::Open;
      arg = parser.GetParam(0).ToStdString();
    }
  } else if (parser.GetParamCount() == 2) {
    if (parser.GetParam(0) == "open") {
      result = CmdlineAction::Open;
      arg = parser.GetParam(1).ToStdString();
    }
  }
  return result;
}
#endif   // __ANDROID__

LocalApiResult LocalClientApi::HandleCmdline(const wxCmdLineParser& parser) {
  std::string arg;
  auto action = ParseArgs(parser, arg);
  return HandleCmdline(action, arg);
}

LocalApiResult LocalClientApi::HandleCmdline(CmdlineAction action,
                                             const std::string& arg) {
  switch (action) {
    case CmdlineAction::Fail:
        MESSAGE_LOG << "IpcClient: Cannot parse command line (ignored)";
        return LocalApiResult(false, "Cannot parse command line");
    case CmdlineAction::Quit: {
          auto result = SendQuit();
          if (!result.first) {
            MESSAGE_LOG << "Error running remote quit cmd: " << result.second;
          }
          return result;
        }
        break;
    case CmdlineAction::Raise: {
          auto result = SendRaise();
          if (!result.first) {
            MESSAGE_LOG << "Error running remote raise cmd: " << result.second;
          }
          return result;
        }
        break;
    case CmdlineAction::DumpStats: {
          auto result = SendDumpStats();
          if (!result.first) {
            MESSAGE_LOG << "Error running remote dump_stats cmd: "
                        << result.second;
          }
          return result;
        }
        break;
    case CmdlineAction::Open: {
          auto result = SendOpen(arg.c_str());
          if (!result.first) {
            MESSAGE_LOG << "Error running remote open of file \"" << arg
                    << "\": " << result.second;
          }
          return result;
        }
        break;
    case CmdlineAction::GetRestEndpoint: {
          auto result = GetRestEndpoint();
          if (result.first)
            std::cout << result.second << "\n" << std::flush;
          else
            std::cout << "Error getting remote endpoint: " << result.second
                << "\n" << std::flush;
          return result;
        }
    case CmdlineAction::Skip:
      return LocalApiResult(true, "Unknown command CmdlineAction::Skip");
  }
  wxLogMessage("Strange code path!");
  return LocalApiResult(false, "Internal error");
}
