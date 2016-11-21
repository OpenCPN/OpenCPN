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

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
  #include "wx/wx.h"
#endif //precompiled headers

#include "wx/filename.h"
#include "wx/wfstream.h"

#include "ChartDataInputStream.h"

#ifdef USE_LZMA

wxCompressedFFileInputStream::wxCompressedFFileInputStream(const wxString& fileName)
{
    init_lzma();
    m_file = new wxFFile(fileName, "rb");
}

wxCompressedFFileInputStream::~wxCompressedFFileInputStream()
{
    delete m_file;
    lzma_end(&strm);
}

size_t wxCompressedFFileInputStream::OnSysRead(void *buffer, size_t size)
{
    lzma_action action = LZMA_RUN;

    strm.next_out = (uint8_t*)buffer;
    strm.avail_out = size;
    
    for(;;) {
        if (strm.avail_in == 0) {
            if(!m_file->Eof()) {
                strm.next_in = inbuf;
                strm.avail_in = m_file->Read(inbuf, sizeof inbuf);
                
                if(m_file->Error())
                    return 0;
                
            } else
                action = LZMA_FINISH;
        }
        
        lzma_ret ret = lzma_code(&strm, action);
        
        if (strm.avail_out == 0 || ret == LZMA_STREAM_END)
            return size - strm.avail_out;
        
        if(ret != LZMA_OK) {
            m_lasterror = wxSTREAM_READ_ERROR;
            return 0;
        }
    }
    return 0;
}

wxFileOffset wxCompressedFFileInputStream::OnSysSeek(wxFileOffset pos, wxSeekMode mode)
{
    // rewind to start is possible
    if(pos == 0 && mode == wxFromStart) {
        lzma_end(&strm);
        init_lzma();
        return m_file->Seek(pos, mode);
    }
    
    return wxInvalidOffset;
}

wxFileOffset wxCompressedFFileInputStream::OnSysTell() const
{
    return strm.total_out;
}

void wxCompressedFFileInputStream::init_lzma()
{
    lzma_stream s = LZMA_STREAM_INIT;
    memcpy(&strm, &s, sizeof s);
    lzma_ret ret = lzma_stream_decoder(
        &strm, UINT64_MAX, LZMA_CONCATENATED);

    if(ret != LZMA_OK)
        m_lasterror = wxSTREAM_READ_ERROR;
}



ChartDataNonSeekableInputStream::ChartDataNonSeekableInputStream(const wxString& fileName)
{
    if(fileName.Upper().EndsWith("XZ"))
        m_stream = new wxCompressedFFileInputStream(fileName);
    else
        m_stream = new wxFFileInputStream(fileName);
}

ChartDataNonSeekableInputStream::~ChartDataNonSeekableInputStream()
{
    delete m_stream;
}

size_t ChartDataNonSeekableInputStream::OnSysRead(void *buffer, size_t size)
{
    m_stream->Read(buffer, size);
    return m_stream->LastRead();
}

wxFileOffset ChartDataNonSeekableInputStream::OnSysSeek(wxFileOffset pos, wxSeekMode mode)
{
    return m_stream->SeekI(pos, mode);
}

wxFileOffset ChartDataNonSeekableInputStream::OnSysTell() const
{
    return m_stream->TellI();
}



ChartDataInputStream::ChartDataInputStream(const wxString& fileName)
{
    if(fileName.Upper().EndsWith("XZ")) {
        // decompress to temp file to allow seeking
        m_tempfilename = wxFileName::CreateTempFileName(wxFileName(fileName).GetFullName());
        wxCompressedFFileInputStream stream(fileName);
        wxFFileOutputStream tmp(m_tempfilename);

        char buffer[8192];
        int len;
        do {
            stream.Read(buffer, sizeof buffer);
            len = stream.LastRead();
            tmp.Write(buffer, len);
        } while(len == sizeof buffer);

        // do some error checking here?

        tmp.Close();
        m_stream = new wxFFileInputStream(m_tempfilename);
    } else
        m_stream = new wxFFileInputStream(fileName);
}

ChartDataInputStream::~ChartDataInputStream()
{
    // delete the temp file, how do we remove temp files if the program crashed?
    if(!m_tempfilename.empty())
        wxRemoveFile(m_tempfilename);
    delete m_stream;
}

size_t ChartDataInputStream::OnSysRead(void *buffer, size_t size)
{
    m_stream->Read(buffer, size);
    return m_stream->LastRead();
}

wxFileOffset ChartDataInputStream::OnSysSeek(wxFileOffset pos, wxSeekMode mode)
{
    return m_stream->SeekI(pos, mode);
}

wxFileOffset ChartDataInputStream::OnSysTell() const {
    return m_stream->TellI();
}

bool DecompressXZFile(const wxString &input_path, const wxString &output_path)
{
    wxCompressedFFileInputStream in(input_path);
    wxFFileOutputStream out(output_path);
    
    char buffer[8192];
    int len;
    do {
        in.Read(buffer, sizeof buffer);
        len = in.LastRead();
        out.Write(buffer, len);
    } while(len == sizeof buffer);

    return in.GetLastError() != wxSTREAM_READ_ERROR;
}

#else // USE_LZMA

bool DecompressXZFile(const wxString &input_path, const wxString &output_path)
{
    wxLogMessage(_T("Failed to decompress: ") + input_path);
    wxLogMessage(_T("OpenCPN compiled without liblzma support"));
                 
    return false;
}

#endif // USE_LZMA
