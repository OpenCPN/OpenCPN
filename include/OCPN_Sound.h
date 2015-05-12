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

#ifndef __OCPN_SOUND_H__
#define __OCPN_SOUND_H__

#include <wx/sound.h>

#ifdef OCPN_USE_PORTAUDIO
    #include "OCPNSoundData.h"
    #include "portaudio.h"
#endif

class OCPN_Sound: public wxSound
{
public:
    OCPN_Sound();
    ~OCPN_Sound();

    static int DeviceCount();

    bool IsOk() const;
    bool Create(const wxString& fileName, int deviceIndex=-1, bool isResource = false);
    bool Play(unsigned flags = wxSOUND_ASYNC) const;
    bool IsPlaying() const;
    void Stop();
    void UnLoad(void);

private:
    bool m_OK;

#ifdef OCPN_USE_PORTAUDIO
    bool LoadWAV(const wxUint8 *data, size_t length, bool copyData);
    void FreeMem(void);

    OCPNSoundData *m_osdata;
    PaStream *m_stream;
#endif

};

#endif
