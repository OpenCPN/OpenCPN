/**************************************************************************
 *   Copyright (C) 2016  David S. Register                                 *
 *   Copyright (C) 2016  Sean D'Epagnier                                   *
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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

/**
 * \file
 *
 *  XZ compressed charts support
 */

#include "config.h"
#include <wx/ffile.h>
#include <wx/string.h>
#include <wx/wfstream.h>

#ifdef OCPN_USE_LZMA
#include <lzma.h>

/**
 *  A non-seekable compressed xz file input stream
 */
class wxCompressedFFileInputStream : public wxInputStream {
public:
  wxCompressedFFileInputStream(const wxString &fileName);
  virtual ~wxCompressedFFileInputStream();

  virtual bool IsOk() const {
    return wxStreamBase::IsOk() && m_file->IsOpened();
  }
  bool IsSeekable() const { return false; }

protected:
  size_t OnSysRead(void *buffer, size_t size);
  wxFileOffset OnSysSeek(wxFileOffset pos, wxSeekMode mode);
  wxFileOffset OnSysTell() const;

  wxFFile *m_file;
  lzma_stream strm;

private:
  void init_lzma();

  uint8_t inbuf[BUFSIZ];

  wxDECLARE_NO_COPY_CLASS(wxCompressedFFileInputStream);
};

// non-seekable stream for either non-compressed or compressed files
class ChartDataNonSeekableInputStream : public wxInputStream {
public:
  ChartDataNonSeekableInputStream(const wxString &fileName);
  virtual ~ChartDataNonSeekableInputStream();

  virtual bool IsOk() const { return m_stream->IsOk(); }
  bool IsSeekable() const { return false; }

protected:
  size_t OnSysRead(void *buffer, size_t size);
  wxFileOffset OnSysSeek(wxFileOffset pos, wxSeekMode mode);
  wxFileOffset OnSysTell() const;

private:
  wxInputStream *m_stream;

  wxDECLARE_NO_COPY_CLASS(ChartDataNonSeekableInputStream);
};

// seekable stream for either non-compressed or compressed files
// it must decompress the file to a temporary file to make it seekable
class ChartDataInputStream : public wxInputStream {
public:
  ChartDataInputStream(const wxString &fileName);
  virtual ~ChartDataInputStream();

  virtual bool IsOk() const { return m_stream->IsOk(); }
  bool IsSeekable() const { return m_stream->IsSeekable(); }

  wxString TempFileName() const { return m_tempfilename; }

protected:
  size_t OnSysRead(void *buffer, size_t size);
  wxFileOffset OnSysSeek(wxFileOffset pos, wxSeekMode mode);
  wxFileOffset OnSysTell() const;

private:
  wxString m_tempfilename;
  wxInputStream *m_stream;

  wxDECLARE_NO_COPY_CLASS(ChartDataInputStream);
};

#else

typedef wxFFileInputStream ChartDataInputStream;
typedef wxFFileInputStream ChartDataNonSeekableInputStream;

#endif  // OCPN_USE_LZMA

bool DecompressXZFile(const wxString &input_path, const wxString &output_path);
