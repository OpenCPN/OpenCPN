/******************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2013 by David S. Register                               *
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

#include <wx/log.h>

#include "SndfileSoundLoader.h"


bool SndfileSoundLoader::Load(const char* path)
{
    memset(&m_sfinfo, 0, sizeof(m_sfinfo));
    m_sndfile = sf_open(path, SFM_READ, &m_sfinfo);
    if (m_sndfile == NULL) {
        const char* err = sf_strerror(NULL);
        wxLogWarning("Cannot open file %s: %s", path, err);
        return false;
    }
    wxLogMessage("Using libsndfile sound loader.");
    return true;
}

void SndfileSoundLoader::UnLoad()
{
    if (m_sndfile) sf_close(m_sndfile);
    m_sndfile = 0;
}

bool SndfileSoundLoader::Reset()
{
    if (!m_sndfile) {
        wxLogWarning("SndfileLoader: Attempt to reset unloaded file.");
        return false;
    }
    return sf_seek(m_sndfile, 0L, SEEK_SET) != -1;
}

size_t SndfileSoundLoader::Get(void* buff, size_t length)
{
    short* dest = static_cast<short*>(buff);
    size_t done = sf_read_short(m_sndfile, dest, length/2);
    return done * 2;
}


unsigned SndfileSoundLoader::GetChannelCount()  const
{
    return m_sfinfo.channels;
}

unsigned SndfileSoundLoader::GetSamplingRate() const
{
    return m_sfinfo.samplerate;
}

SndfileSoundLoader::~SndfileSoundLoader()
{
    if (m_sndfile) sf_close(m_sndfile);
}
