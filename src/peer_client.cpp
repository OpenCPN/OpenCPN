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

#include "peer_client.h"

#include "wx/curl/http.h"
#include "wx/curl/thread.h"
#include "nav_object_database.h"
#include <wx/json_defs.h>
#include <wx/jsonreader.h>
#include "REST_server.h"
#include "gui_lib.h"

extern wxString g_hostname;


wxString GetErrorText(int result){
  switch (result) {
    case RESTServerResult::RESULT_GENERIC_ERROR:
      return _("Server Generic Error");
    case RESTServerResult::RESULT_OBJECT_REJECTED:
      return _("Peer rejected object");
    case RESTServerResult::RESULT_DUPLICATE_REJECTED:
      return _("Peer rejected duplicate object");
    case RESTServerResult::RESULT_ROUTE_INSERT_ERROR:
       return _("Peer internal error (insert)");
    default:
      return _("Server Unknown Error");
  }
}

size_t wxcurl_string_write_UTF8(void* ptr, size_t size, size_t nmemb, void* pcharbuf)
{
    size_t iRealSize = size * nmemb;
    wxCharBuffer* pStr = (wxCharBuffer*) pcharbuf;

    if(pStr)
    {
#ifdef __WXMSW__
        wxString str1a = wxString(*pStr);
        wxString str2 = wxString((const char*)ptr, wxConvUTF8, iRealSize);
        *pStr = (str1a + str2).mb_str();
#else
        wxString str = wxString(*pStr, wxConvUTF8) + wxString((const char*)ptr, wxConvUTF8, iRealSize);
        *pStr = str.mb_str(wxConvUTF8);
#endif
    }

    return iRealSize;
}


class wxCurlHTTPNoZIP : public wxCurlHTTP
{
public:
    wxCurlHTTPNoZIP(const wxString& szURL = wxEmptyString,
               const wxString& szUserName = wxEmptyString,
               const wxString& szPassword = wxEmptyString,
               wxEvtHandler* pEvtHandler = NULL, int id = wxID_ANY,
               long flags = wxCURL_DEFAULT_FLAGS);

   ~wxCurlHTTPNoZIP();

  bool Post(const char *url, const char *body_data);
  std::string GetResponseBody() const;

protected:
    void SetCurlHandleToDefaults(const wxString& relativeURL);

};

wxCurlHTTPNoZIP::wxCurlHTTPNoZIP(const wxString& szURL /*= wxEmptyString*/,
                       const wxString& szUserName /*= wxEmptyString*/,
                       const wxString& szPassword /*= wxEmptyString*/,
                       wxEvtHandler* pEvtHandler /*= NULL*/,
                       int id /*= wxID_ANY*/,
                       long flags /*= wxCURL_DEFAULT_FLAGS*/)
: wxCurlHTTP(szURL, szUserName, szPassword, pEvtHandler, id, flags)

{
}

wxCurlHTTPNoZIP::~wxCurlHTTPNoZIP()
{
    ResetPostData();
}

void wxCurlHTTPNoZIP::SetCurlHandleToDefaults(const wxString& relativeURL)
{
    wxCurlBase::SetCurlHandleToDefaults(relativeURL);

    SetOpt(CURLOPT_ENCODING, "identity");               // No encoding, plain ASCII

    if(m_bUseCookies)
    {
        SetStringOpt(CURLOPT_COOKIEJAR, m_szCookieFile);
    }
}

bool wxCurlHTTPNoZIP::Post(const char *url, const char *body_data)
{
    curl_off_t iSize = 0;

    if(m_pCURL )
    {
        SetOpt(CURLOPT_ENCODING, "identity");               // No encoding, plain ASCII

        SetHeaders();

        SetOpt(CURLOPT_URL, url);

        iSize = strlen(body_data);
        if(iSize == (~(ssize_t)0))      // wxCurlHTTP does not know how to upload unknown length streams.
            return false;

        SetOpt(CURLOPT_POSTFIELDSIZE, iSize);
        SetOpt(CURLOPT_POSTFIELDS, body_data);


        SetOpt(CURLOPT_POST, TRUE);
        SetOpt(CURLOPT_POSTFIELDSIZE_LARGE, iSize);
        //SetStreamReadFunction(buffer);

        //  Use a private data write trap function to handle UTF8 content
        SetOpt(CURLOPT_WRITEFUNCTION, wxcurl_string_write_UTF8);         // private function
        SetOpt(CURLOPT_WRITEDATA, (void*)&m_szResponseBody);

        SetOpt(CURLOPT_CAINFO, "cert.pem");
        SetOpt(CURLOPT_SSL_VERIFYPEER, 0);

        //curl_easy_setopt(m_pCURL, CURLOPT_XFERINFOFUNCTION, xferinfo);
        curl_easy_setopt(m_pCURL, CURLOPT_NOPROGRESS, 0L);

        if(Perform())
        {
            ResetHeaders();
            return IsResponseOk();
        }
    }

    return false;
}

std::string wxCurlHTTPNoZIP::GetResponseBody() const
{
#ifndef __arm__
    wxString s = wxString((const char *)m_szResponseBody, wxConvLibc);
    return std::string(s.mb_str());

#else
    return std::string((const char *)m_szResponseBody);
#endif

}

struct MemoryStruct {
  char *memory;
  size_t size;
};

static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  char *ptr = (char *)realloc(mem->memory, mem->size + realsize + 1);
  if(!ptr) {
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


int SendRoute(std::string dest_ip_address, Route *route, bool overwrite)
{
  if(!route)
    return -1;

  // Get XML representation of object.
  NavObjectCollection1 *pgpx = new NavObjectCollection1;
  pgpx->AddGPXRoute(route);
  std::ostringstream stream;
  pgpx->save(stream, PUGIXML_TEXT(" "));

  std::string url(dest_ip_address);
  url += "/api/rx_object";
  url += _T("?source=") + g_hostname;

#if 1
  struct MemoryStruct chunk;
  chunk.memory = (char *)malloc(1);
  chunk.size = 0;

  CURL* c = curl_easy_init();
  curl_easy_setopt(c, CURLOPT_ENCODING, "identity");               // No encoding, plain ASCII
  curl_easy_setopt(c, CURLOPT_URL, url.c_str());
  curl_easy_setopt(c, CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(c, CURLOPT_SSL_VERIFYHOST, 0L);

  int iSize =  strlen(stream.str().c_str());
  curl_easy_setopt(c, CURLOPT_POSTFIELDSIZE, iSize);
  curl_easy_setopt(c, CURLOPT_COPYPOSTFIELDS, stream.str().c_str());

  curl_easy_setopt(c, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
  curl_easy_setopt(c, CURLOPT_WRITEDATA, (void *)&chunk);

  CURLcode result = curl_easy_perform(c);
  long response_code = -1;

  if(result == CURLE_OK)
    curl_easy_getinfo(c, CURLINFO_RESPONSE_CODE, &response_code);

  curl_easy_cleanup(c);

  if(response_code == 200){
    //printf("%s\n", post.GetResponseBody().c_str());

    wxString body(chunk.memory);
    wxJSONValue  root;
    wxJSONReader reader;

    int numErrors = reader.Parse( body, &root );
    // Capture the result
    int result = root["result"].AsInt();
    if (result > 0){
      wxString error_text = GetErrorText(result);
      OCPNMessageDialog mdlg(NULL, error_text, wxString(_("OpenCPN Info")),
                         wxICON_ERROR | wxOK);
      mdlg.ShowModal();
    }
  }

#else
  wxCurlHTTPNoZIP post;
  post.SetOpt(CURLOPT_TIMEOUT, 5);
  res = post.Post( url.c_str(), stream.str().c_str() );

    // get the response code of the server
  post.GetInfo(CURLINFO_RESPONSE_CODE, &iResponseCode);
  if(iResponseCode == 200){
    printf("%s\n", post.GetResponseBody().c_str());

    wxString body(post.GetResponseBody().c_str());
    wxJSONValue  root;
    wxJSONReader reader;

    int numErrors = reader.Parse( body, &root );
    // Capture the result
    int result = root["result"].AsInt();
    if (result > 0){
      wxString error_text = GetErrorText(result);
      OCPNMessageDialog mdlg(NULL, error_text, wxString(_("OpenCPN Info")),
                         wxICON_ERROR | wxOK);
      mdlg.ShowModal();
    }
  }
#endif

  return true;
}
