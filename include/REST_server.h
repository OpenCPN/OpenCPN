/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:
 * Author:   David Register, Alec Leamas
 *
 ***************************************************************************
 *   Copyright (C) 2022 by David Register, Alec Leamas                     *
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

#ifndef _RESTSERVER_H
#define _RESTSERVER_H

#include <string>
#include <unordered_map>
#include <fstream>

#include <wx/event.h>

typedef enum RESTServerResult {
  RESULT_NO_ERROR = 0,
  RESULT_GENERIC_ERROR,
  RESULT_OBJECT_REJECTED,
  RESULT_DUPLICATE_REJECTED,
  RESULT_ROUTE_INSERT_ERROR,
  RESULT_NEW_PIN_REQUESTED
} _RESTServerResult;

enum {
  ORS_START_OF_SESSION,
  ORS_CHUNK_N,
  ORS_CHUNK_LAST
};

class RESTServerThread;  // Internal
class RESTServerEvent;  // Internal
class PINCreateDialog;

class RESTServer : public wxEvtHandler {
public:
  RESTServer();

  virtual ~RESTServer();

  bool StartServer(std::string certificate_location);
  void StopServer();

  void HandleServerMessage(RESTServerEvent& event);

  //    Secondary thread life toggle
  //    Used to inform launching object (this) to determine if the thread can
  //    be safely called or polled, e.g. wxThread->Destroy();
  void SetSecThreadActive(void) { m_bsec_thread_active = true; }
  void SetSecThreadInActive(void) { m_bsec_thread_active = false; }
  bool IsSecThreadActive() const { return m_bsec_thread_active; }

  void SetSecondaryThread(RESTServerThread* secondary_Thread) {
    m_pSecondary_Thread = secondary_Thread;
  }
  RESTServerThread* GetSecondaryThread() {
    return m_pSecondary_Thread;
  }
  void SetThreadRunFlag(int run) { m_Thread_run_flag = run; }

  std::string GetCertificateDirectory(){ return m_certificate_directory; }
  int m_Thread_run_flag;

  std::string m_cert_file;
  std::string m_key_file;

private:
  bool LoadConfig( void );
  bool SaveConfig( void );

  RESTServerThread* m_pSecondary_Thread;
  bool m_bsec_thread_active;
  std::string m_certificate_directory;
  std::unordered_map<std::string, std::string> m_key_map;
  PINCreateDialog *m_PINCreateDialog;
  wxString m_sPIN;
  int m_dPIN;
  bool m_b_overwrite;
  std::string m_tempUploadFilePath;
  std::ofstream m_ul_stream;


};

#endif  // guard
