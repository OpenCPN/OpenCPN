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

#ifndef __MSW_SOUND_H__
#define __MSW_SOUND_H__

#include "OCPN_Sound.h"

/**
 * Sound backend on the windows PlaySound() API.
 */

class MswSound: public OcpnSound
{

    public:
        MswSound() {};
        ~MswSound() { Stop(); };

        bool Load(const char* path, int deviceIndex = -1) override;
        bool Play() override;
        bool Stop() override;

    private:
        void worker();
        std::wstring m_path;
        bool m_isPlaying;
};


#endif // __MSW_SOUND_H__
