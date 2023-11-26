/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Peer-peer data sharing.
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2022 by David Register                                  *
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
#include <sstream>

#include <curl/curl.h>

#include "peer_client.h"

#include <wx/fileconf.h>
#include <wx/json_defs.h>
#include <wx/jsonreader.h>
#include <wx/tokenzr.h>

#include "config_vars.h"
#include "FontMgr.h"
#include "gui_lib.h"
#include "nav_object_database.h"
#include "rest_server.h"

extern MyFrame* gFrame;

wxString GetErrorText(RestServerResult result) {
  switch (result) {
    case RestServerResult::GenericError:
      return _("Server Generic Error");
    case RestServerResult::ObjectRejected:
      return _("Peer rejected object");
    case RestServerResult::DuplicateRejected:
      return _("Peer rejected duplicate object");
    case RestServerResult::RouteInsertError:
      return _("Peer internal error (insert)");
    default:
      return _("Server Unknown Error");
  }
}

size_t wxcurl_string_write_UTF8(void* ptr, size_t size, size_t nmemb,
                                void* pcharbuf) {
  size_t iRealSize = size * nmemb;
  wxCharBuffer* pStr = (wxCharBuffer*)pcharbuf;

  if (pStr) {
#ifdef __WXMSW__
    wxString str1a = wxString(*pStr);
    wxString str2 = wxString((const char*)ptr, wxConvUTF8, iRealSize);
    *pStr = (str1a + str2).mb_str();
#else
    wxString str = wxString(*pStr, wxConvUTF8) +
                   wxString((const char*)ptr, wxConvUTF8, iRealSize);
    *pStr = str.mb_str(wxConvUTF8);
#endif
  }

  return iRealSize;
}

struct MemoryStruct {
  char* memory;
  size_t size;
};

static size_t WriteMemoryCallback(void* contents, size_t size, size_t nmemb,
                                  void* userp) {
  size_t realsize = size * nmemb;
  struct MemoryStruct* mem = (struct MemoryStruct*)userp;

  char* ptr = (char*)realloc(mem->memory, mem->size + realsize + 1);
  if (!ptr) {
    /* out of memory! */
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }

  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  return realsize;
}

int navobj_transfer_progress;

int xfer_callback(void* clientp, curl_off_t dltotal, curl_off_t dlnow,
                  curl_off_t ultotal, curl_off_t ulnow) {
  if (ultotal == 0) {
    navobj_transfer_progress = 0;
  } else {
    navobj_transfer_progress = 100 * ulnow / ultotal;
  }
  wxYield();
  return 0;
}

long PostSendObjectMessage(std::string url, std::ostringstream& body,
                           MemoryStruct* response) {
  long response_code = -1;
  navobj_transfer_progress = 0;

  CURL* c = curl_easy_init();
  curl_easy_setopt(c, CURLOPT_ENCODING,
                   "identity");  // No encoding, plain ASCII
  curl_easy_setopt(c, CURLOPT_URL, url.c_str());
  curl_easy_setopt(c, CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(c, CURLOPT_SSL_VERIFYHOST, 0L);

  int iSize = strlen(body.str().c_str());
  curl_easy_setopt(c, CURLOPT_POSTFIELDSIZE, iSize);
  curl_easy_setopt(c, CURLOPT_COPYPOSTFIELDS, body.str().c_str());

  curl_easy_setopt(c, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
  curl_easy_setopt(c, CURLOPT_WRITEDATA, (void*)response);
  curl_easy_setopt(c, CURLOPT_NOPROGRESS, 0);
  curl_easy_setopt(c, CURLOPT_XFERINFOFUNCTION, xfer_callback);

  CURLcode result = curl_easy_perform(c);
  navobj_transfer_progress = 0;
  if (result == CURLE_OK)
    curl_easy_getinfo(c, CURLINFO_RESPONSE_CODE, &response_code);

  curl_easy_cleanup(c);

  return response_code;
}

std::string GetClientKey(std::string& server_name) {
  if (TheBaseConfig()) {
    TheBaseConfig()->SetPath("/Settings/RESTClient");

    wxString key_string;

    TheBaseConfig()->Read("ServerKeys", &key_string);
    wxStringTokenizer st(key_string, _T(";"));
    while (st.HasMoreTokens()) {
      wxString s1 = st.GetNextToken();
      wxString server_name_persisted = s1.BeforeFirst(':');
      wxString server_key = s1.AfterFirst(':');

      if (!server_name_persisted.ToStdString().compare(server_name))
        return server_key.ToStdString();
    }
  }
  return "1";
}

void SaveClientKey(std::string& server_name, std::string key) {
  if (TheBaseConfig()) {
    TheBaseConfig()->SetPath("/Settings/RESTClient");

    wxArrayString array;
    wxString key_string;
    TheBaseConfig()->Read("ServerKeys", &key_string);
    wxStringTokenizer st(key_string, _T(";"));
    while (st.HasMoreTokens()) {
      wxString s1 = st.GetNextToken();
      array.Add(s1);
    }

    bool b_updated = false;
    for (unsigned int i = 0; i < array.GetCount(); i++) {
      wxString s1 = array[i];
      wxString server_name_persisted = s1.BeforeFirst(':');
      wxString server_key = s1.AfterFirst(':');
      if (server_name_persisted.IsSameAs(server_name.c_str())) {
        array[i] = server_name_persisted + ":" + key.c_str();
        b_updated = true;
        break;
      }
    }

    if (!b_updated) {
      wxString new_entry = server_name.c_str() + wxString(":") + key.c_str();
      array.Add(new_entry);
    }

    wxString key_string_updated;
    for (unsigned int i = 0; i < array.GetCount(); i++) {
      wxString s1 = array[i];
      key_string_updated += s1;
      key_string_updated += ";";
    }

    TheBaseConfig()->Write("ServerKeys", key_string_updated);
  }
  return;
}

int SendNavobjects(std::string dest_ip_address, std::string server_name,
                   std::vector<Route*> route,
                   std::vector<RoutePoint*> routepoint,
                   std::vector<Track*> track, bool overwrite) {
  if (route.empty() && routepoint.empty() && track.empty()) return -1;
  bool apikey_ok = false;
  bool b_cancel = false;
  std::ostringstream stream;
  std::string api_key;

  while (!apikey_ok && b_cancel == false) {
    api_key = GetClientKey(server_name);

    std::string url(dest_ip_address);
    url += "/api/ping";
    url += std::string("?source=") + g_hostname;
    url += std::string("&apikey=") + api_key;

    struct MemoryStruct chunk;
    chunk.memory = (char*)malloc(1);
    chunk.size = 0;

    long response_code = PostSendObjectMessage(url, stream, &chunk);

    if (response_code == 200) {
      wxString body(chunk.memory);
      wxJSONValue root;
      wxJSONReader reader;

      int numErrors = reader.Parse(body, &root);
      // Capture the result
      int result = root["result"].AsInt();
      if (result > 0) {
        if (result == static_cast<int>(RestServerResult::NewPinRequested)) {
          // Show the dialog asking for PIN
          PINConfirmDialog dlg(
              (wxWindow*)gFrame, wxID_ANY, _("OpenCPN Server Message"), "",
              wxDefaultPosition, wxDefaultSize, SYMBOL_PCD_STYLE);

          wxString hmsg(_("The server "));
          hmsg += _("needs a PIN.\nPlease enter the PIN number from ");
          hmsg += _("the server to pair with this device.\n");

          dlg.SetMessage(hmsg);
          dlg.SetText1Message("");

          dlg.ShowModal();
          if (dlg.GetReturnCode() == ID_PCD_OK) {
            wxString PIN_tentative = dlg.GetText1Value().Trim().Trim(false);
            unsigned int dPIN = atoi(PIN_tentative.ToStdString().c_str());
            std::string new_api_key = PintoRandomKeyString(dPIN);
            ;

            SaveClientKey(server_name, new_api_key);
          } else
            b_cancel = true;
        } else if (result == static_cast<int>(RestServerResult::GenericError))
          apikey_ok = true;
      } else
        apikey_ok = true;
    } else {
      wxString err_msg;
      err_msg.Printf("Server HTTP response is: %ld", response_code);
      OCPNMessageDialog mdlg(NULL, err_msg, wxString(_("OpenCPN Info")),
                             wxICON_ERROR | wxOK);
      mdlg.ShowModal();

      b_cancel = true;
    }
  }
  if (!apikey_ok || b_cancel) {
    return false;
  }
  // Get XML representation of object.
  NavObjectCollection1* pgpx = new NavObjectCollection1;
  navobj_transfer_progress = 0;
  int total = route.size() + track.size() + routepoint.size();
  int gpxgen = 0;
  for (auto r : route) {
    gpxgen++;
    pgpx->AddGPXRoute(r);
    navobj_transfer_progress = 100 * gpxgen / total;
    wxYield();
  }
  for (auto r : routepoint) {
    gpxgen++;
    pgpx->AddGPXWaypoint(r);
    navobj_transfer_progress = 100 * gpxgen / total;
    wxYield();
  }
  for (auto r : track) {
    gpxgen++;
    pgpx->AddGPXTrack(r);
    navobj_transfer_progress = 100 * gpxgen / total;
    wxYield();
  }
  pgpx->save(stream, PUGIXML_TEXT(" "));

  while (b_cancel == false) {
    std::string api_key = GetClientKey(server_name);

    std::string url(dest_ip_address);
    url += "/api/rx_object";
    url += std::string("?source=") + g_hostname;
    url += std::string("&apikey=") + api_key;

    struct MemoryStruct chunk;
    chunk.memory = (char*)malloc(1);
    chunk.size = 0;
    long response_code = PostSendObjectMessage(url, stream, &chunk);

    if (response_code == 200) {
      wxString body(chunk.memory);
      wxJSONValue root;
      wxJSONReader reader;

      int numErrors = reader.Parse(body, &root);
      // Capture the result
      int result = root["result"].AsInt();
      if (result > 0) {
        wxString error_text =
            GetErrorText(static_cast<RestServerResult>(result));
        OCPNMessageDialog mdlg(NULL, error_text, wxString(_("OpenCPN Info")),
                               wxICON_ERROR | wxOK);
        mdlg.ShowModal();
        b_cancel = true;
      } else {
        OCPNMessageDialog mdlg(
            NULL, _("Objects successfully sent to peer OpenCPN instance."),
            wxString(_("OpenCPN Info")), wxICON_INFORMATION | wxOK);
        mdlg.ShowModal();
        b_cancel = true;
      }
    } else {
      wxString err_msg;
      err_msg.Printf("Server HTTP response is: %ld", response_code);
      OCPNMessageDialog mdlg(NULL, err_msg, wxString(_("OpenCPN Info")),
                             wxICON_ERROR | wxOK);
      mdlg.ShowModal();

      b_cancel = true;
    }
  }
  return true;
}

IMPLEMENT_DYNAMIC_CLASS(PINConfirmDialog, wxDialog)

BEGIN_EVENT_TABLE(PINConfirmDialog, wxDialog)
EVT_BUTTON(ID_PCD_CANCEL, PINConfirmDialog::OnCancelClick)
EVT_BUTTON(ID_PCD_OK, PINConfirmDialog::OnOKClick)
END_EVENT_TABLE()

PINConfirmDialog::PINConfirmDialog() {
  m_OKButton = NULL;
  m_CancelButton = NULL;
  premtext = NULL;
}

PINConfirmDialog::PINConfirmDialog(wxWindow* parent, wxWindowID id,
                                   const wxString& caption,
                                   const wxString& hint, const wxPoint& pos,
                                   const wxSize& size, long style) {
  wxFont* pif = FontMgr::Get().GetFont(_T("Dialog"));
  SetFont(*pif);
  Create(parent, id, caption, hint, pos, size, style);
}

PINConfirmDialog::~PINConfirmDialog() {
  delete m_OKButton;
  delete m_CancelButton;
}

bool PINConfirmDialog::Create(wxWindow* parent, wxWindowID id,
                              const wxString& caption, const wxString& hint,
                              const wxPoint& pos, const wxSize& size,
                              long style) {
  SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
  wxDialog::Create(parent, id, caption, pos, size, style);

  CreateControls(hint);
  GetSizer()->Fit(this);
  GetSizer()->SetSizeHints(this);
  Centre();

  return TRUE;
}

void PINConfirmDialog::CreateControls(const wxString& hint) {
  PINConfirmDialog* itemDialog1 = this;

  wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
  SetSizer(itemBoxSizer2);

  //    Add a reminder text box
  itemBoxSizer2->AddSpacer(20);

  premtext = new wxStaticText(
      this, -1, "A loooooooooooooooooooooooooooooooooooooooooooooong line\n");
  itemBoxSizer2->Add(premtext, 0, wxEXPAND | wxALL, 10);

  m_pText1 = new wxTextCtrl(this, wxID_ANY, "        ", wxDefaultPosition,
                            wxDefaultSize, wxTE_CENTRE);
  itemBoxSizer2->Add(m_pText1, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 10);

  //    OK/Cancel/etc.
  wxBoxSizer* itemBoxSizer16 = new wxBoxSizer(wxHORIZONTAL);
  itemBoxSizer2->Add(itemBoxSizer16, 0, wxALIGN_RIGHT | wxALL, 5);

  m_CancelButton = new wxButton(itemDialog1, ID_PCD_CANCEL, _("Cancel"),
                                wxDefaultPosition, wxDefaultSize, 0);
  itemBoxSizer16->Add(m_CancelButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

  m_OKButton = new wxButton(itemDialog1, ID_PCD_OK, "OK", wxDefaultPosition,
                            wxDefaultSize, 0);
  itemBoxSizer16->Add(m_OKButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
  m_OKButton->SetDefault();
}

void PINConfirmDialog::SetMessage(const wxString& msg) {
  if (premtext) {
    premtext->SetLabel(msg);
    premtext->Refresh(true);
  }
}

void PINConfirmDialog::SetText1Message(const wxString& msg) {
  m_pText1->ChangeValue(msg);
  m_pText1->Show();
  GetSizer()->Fit(this);
}

void PINConfirmDialog::OnOKClick(wxCommandEvent& event) {
  SetReturnCode(ID_PCD_OK);
  EndModal(ID_PCD_OK);
}

void PINConfirmDialog::OnCancelClick(wxCommandEvent& event) {
  EndModal(ID_PCD_CANCEL);
}
