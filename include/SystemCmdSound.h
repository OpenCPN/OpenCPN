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

#ifndef SYSTEM_CMD_SOUND_H__
#define SYSTEM_CMD_SOUND_H__

#include "OCPN_Sound.h"
#include "config.h"

/**
 * Sound backend based on running a external CLI tool using system(3). 
 * Supports synchronous and asynchronous mode. The command line tools
 * also typically supports a wide range of audio formats; the exact
 * list is platform dependent.
 */

class SystemCmdSound: public OcpnSound
{

    public:
        SystemCmdSound(const char* cmd = SYSTEM_SOUND_CMD)
            :m_path(""), m_cmd(cmd), m_isPlaying(false) {};
        ~SystemCmdSound() {};

        bool Load(const char* path, int deviceIndex = -1) override;
        bool Play() override;
        bool Stop() override;

    private:
        void worker();
        bool canPlay();
        bool m_isPlaying;
        std::string m_cmd;
        std::string m_path;
};


#endif // __WX_SOUND_H__
