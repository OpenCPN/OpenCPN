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

#ifndef __WX_SOUND_H__
#define __WX_SOUND_H__

#include <wx/sound.h>

#include "OCPN_Sound.h"

/**
 * Sound backend based on wxWidget's wxSound class. On Linux, this seems
 * to be broken -- at a minimum it requires the old OSS API which on
 * pulseaudio platforms means to have the osspd service running. Even so,
 * there seems to be many quirks here.
 *
 * However, the backend might possibly work on MacOS and/or Windows.
 * Supports synchronous and asynchronous mode.
 *
 */

class OcpnWxSound: public OcpnSound
{

    public:
        OcpnWxSound() {};
        ~OcpnWxSound() { Stop(); };

        bool Load(const char* path, int deviceIndex = -1) override;
        void UnLoad() override {};
        bool Play() override;
        bool Stop() override;

    private:
        void worker();
        std::string m_path;
        bool m_isPlaying;
        wxSound m_sound;
};


#endif // __WX_SOUND_H__
