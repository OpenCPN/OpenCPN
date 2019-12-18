/******************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2019 Alec Leamas                                        *
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

#include <fstream>

#include <curl/curl.h>

#include <wx/filename.h>
#include <wx/log.h>

#include "Downloader.h"



/** Dummy curl callback on received data from remote. */
static size_t throw_cb(void *ptr, size_t size, size_t nmemb, void *data)
{
  (void)ptr;
  (void)data;
  return (size_t)(size * nmemb);
}


static unsigned write_cb(char* in, unsigned size, unsigned nmemb, void* data);
    // Forward

Downloader::Downloader(std::string url_)
    :url(url_), stream(), error_msg(""), errorcode(0)
{};


int Downloader::last_errorcode() { return errorcode; }

std::string Downloader::last_error() { return error_msg; }

void Downloader::on_chunk(const char* buff, unsigned bytes)
{
    stream->write(buff, bytes);
}


bool Downloader::download(std::ostream* stream)
{
    CURL* curl;
    char curl_errbuf[CURL_ERROR_SIZE];

    this->stream = stream;
    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curl_errbuf);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
    // FIXME -- Add correct certificates on host.
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,  write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
    int code = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    if(code != CURLE_OK) {
        wxLogWarning("Failed to get '%s' [%s]\n", url, curl_errbuf);
        errorcode = code;
        error_msg = std::string(curl_errbuf);
        return false;
    }
    return true;
}


bool Downloader::download(std::string& path)
{
    if (path == "") {
        path = wxFileName::CreateTempFileName("ocpn_dl").ToStdString();
    }
    std::ofstream stream;
    stream.open(path.c_str(),
                std::ios::out | std::ios::binary | std::ios::trunc);
    if (!stream.is_open()) {
        errorcode = CURLE_WRITE_ERROR;
        error_msg = std::string("Cannot open temporary file ") + path;
        return false;
    }
    bool ok = download(&stream);
    stream.close();
    return ok;
}


long Downloader::get_filesize()
{
    CURL* curl;
    char curl_errbuf[CURL_ERROR_SIZE];
    double filesize = 0.0;

    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
    curl_easy_setopt(curl, CURLOPT_FILETIME, 1L);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, throw_cb);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_HEADER, 0L);

    int r = curl_easy_perform(curl);
    if (r == CURLE_OK) {
        r = curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD,
                              &filesize); 
    }
    curl_easy_cleanup(curl);
    wxLogMessage("filesize %s: %d bytes\n", url.c_str(), (int) filesize);
    if (r != CURLE_OK) {
        errorcode = r;
        error_msg = std::string(curl_errbuf);
        return 0;
    }
    return (long) filesize;
}


/** Curl callback on received data from remote. */
static unsigned write_cb(char* in, unsigned size, unsigned nmemb, void* data)
{
    auto downloader = static_cast<Downloader*>(data);
    if (data == 0) {
        return 0;
    }
    downloader->on_chunk(in, size * nmemb);
    return in == NULL ? 0 : size * nmemb;
}



