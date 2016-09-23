/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Support XZ compressed charts
 * Author:   Sean D'Epagnier
 *
 ***************************************************************************
 *   Copyright (C) 2016 by David S. Register                               *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.             *
 ***************************************************************************
 *
 */

#ifdef USE_LZMA
#include <lzma.h>

// this implements a non-seekable input stream of xz compressed file
class wxCompressedFFileInputStream : public wxInputStream
{
public:
    wxCompressedFFileInputStream(const wxString& fileName);
    virtual ~wxCompressedFFileInputStream();

    virtual bool IsOk() const { return wxStreamBase::IsOk() && m_file->IsOpened(); }
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
class ChartDataNonSeekableInputStream : public wxInputStream
{
public:
    ChartDataNonSeekableInputStream(const wxString& fileName);
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
class ChartDataInputStream : public wxInputStream
{
public:
    ChartDataInputStream(const wxString& fileName);
    virtual ~ChartDataInputStream();

    virtual bool IsOk() const { return m_stream->IsOk(); }
    bool IsSeekable() const { return m_stream->IsSeekable(); }

    wxString TempFileName() const { return m_tempfilename; }

protected:
    size_t OnSysRead(void *buffer, size_t size);
    wxFileOffset OnSysSeek(wxFileOffset pos, wxSeekMode mode);
    wxFileOffset OnSysTell() const;

private:
    wxString  m_tempfilename;
    wxInputStream *m_stream;

    wxDECLARE_NO_COPY_CLASS(ChartDataInputStream);
};

#else

typedef wxFFileInputStream ChartDataInputStream;
typedef wxFFileInputStream ChartDataNonSeekableInputStream;

#endif // USE_LZMA

bool DecompressXZFile(const wxString &input_path, const wxString &output_path);
