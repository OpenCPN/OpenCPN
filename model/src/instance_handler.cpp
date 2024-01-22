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

#include <wx/filename.h>
#include <wx/ipc.h>
#include <wx/log.h>
#include <wx/snglinst.h>
#include <wx/string.h>

#include "model/base_platform.h"

#include "bbox.h"
#include "model/nav_object_database.h"

class StConnection : public wxConnection {
public:
  StConnection() {}
  ~StConnection() {}
  bool OnExec(const wxString &topic, const wxString &data);

  /** Update RouteManagerDialog callback. */
  std::function<void()> update_route_mgr_dlg;
    //   if (pRouteManagerDialog && pRouteManagerDialog->IsShown())
    //    pRouteManagerDialog->UpdateLists();

  /** Center global view to a given box callback. */
  std::function<void(LLBBox)> gframe_center_view;
    // gFrame->CenterView(gFrame->GetPrimaryCanvas(), box);

  /** Raise current OPenCPN main window to top of GUI application stack. */
  std::function<void()> raise;
   //  gFrame->InvalidateAllGL();
   //  gFrame->RefreshAllCanvas(false);
   //  gFrame->Raise();
};

// Client class, to be used by subsequent instances in OnInit
class StClient : public wxClient {
public:
  StClient(){};
  wxConnectionBase *OnMakeConnection() { return new StConnection; }
};


// Opens a file passed from another instance
bool StConnection::OnExec(const wxString &topic, const wxString &data) {
  // not setup yet
  // if (!gFrame) return false;

  wxString path(data);
  if (path.IsEmpty()) {
    raise();
  } else {
    NavObjectCollection1 *pSet = new NavObjectCollection1;
    pSet->load_file(path.fn_str());
    int wpt_dups;
    // Import with full vizibility of names and objects
    pSet->LoadAllGPXObjects(!pSet->IsOpenCPN(), wpt_dups, true);
    update_route_mgr_dlg();
    LLBBox box = pSet->GetBBox();
    if (box.GetValid()) {
      gframe_center_view(box);
    }
    delete pSet;
    return true;
  }
  return true;
}

// Server class, for listening to connection requests
class StServer : public wxServer {
public:
  wxConnectionBase *OnAcceptConnection(const wxString &topic);
};

/** Return true if there is a top-level modal window. */
static bool IsToplevelModal() {
  for (auto w = wxTopLevelWindows.GetFirst(); w; w = w->GetNext()) {
    wxDialog *dlg = dynamic_cast<wxDialog*>(w->GetData());
    if (dlg && dlg->IsModal()) {
      return true;
    }
  }
  return false;
}

// Accepts a connection from another instance
wxConnectionBase *StServer::OnAcceptConnection(const wxString& topic) {
  if (topic.Lower() == "opencpn" && !IsToplevelModal()) {
    return new StConnection();
  }
  return 0;
}



class InstanceHandler : public wxSingleInstanceChecker {
public:
  bool Init(const std::vector<std::string>& params) {
    if (wxSingleInstanceChecker::IsAnotherRunning()) {
      wxChar separator = wxFileName::GetPathSeparator();
      wxString service_name =
          g_BasePlatform->GetPrivateDataDir() + separator + _T("opencpn-ipc");

      auto checker = new wxSingleInstanceChecker(_T("_OpenCPN_SILock"),
                                              g_BasePlatform->GetPrivateDataDir());
      if (!checker->IsAnotherRunning()) {
        StServer *m_server = new StServer;
        if (!m_server->Create(service_name)) {
          wxLogDebug(wxT("Failed to create an IPC service."));
          return false;
        }
      } else {
        wxLogNull logNull;
        StClient *client = new StClient;
        // ignored under DDE, host name in TCP/IP based classes
        wxString hostName = wxT("localhost");
        // Create the connection service, topic
        wxConnectionBase *connection =
            client->MakeConnection(hostName, service_name, _T("OpenCPN"));
        if (connection) {
          // Ask the other instance to open a file or raise itself
          if (params.empty()) {
            for (size_t n = 0; n < params.size(); n++) {
              wxString path(params[n]);
              if (::wxFileExists(path)) {
                connection->Execute(path);
              }
            }
          }
          connection->Execute(wxT(""));
          connection->Disconnect();
          delete connection;
        } else {
          //  If we get here, it means that the wxWidgets single-instance-detect
          //  logic found the lock file, And so thinks another instance is
          //  running. But that instance is not reachable, for some reason. So,
          //  the safe thing to do is delete the lockfile, and exit.  Next start
          //  will proceed normally. This may leave a zombie OpenCPN, but at
          //  least O starts.
          wxString lockFile = wxString(g_BasePlatform->GetPrivateDataDir() +
                                       separator + _T("_OpenCPN_SILock"));
          if (wxFileExists(lockFile)) wxRemoveFile(lockFile);

          wxMessageBox(
              _("Sorry, an existing instance of OpenCPN may be too busy "
                "to respond.\nPlease retry."),
              wxT("OpenCPN"), wxICON_INFORMATION | wxOK);
        }
        delete client;
        return false;  // exit quietly
      }
    }
    return true;
  }
};
