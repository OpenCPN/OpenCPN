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
        SystemCmdSound(const char* cmd = OCPN_SOUND_CMD)
            :m_isPlaying(false), m_cmd(cmd), m_path("")  {};
        ~SystemCmdSound() {};

        bool Load(const char* path, int deviceIndex = -1) override;
        void UnLoad() override {};
        bool Play() override;
        bool Stop() override;
        /**
         * Set system command string in case program wants to change from
         * default string.
         */
        void SetCmd(const char* cmd ) { m_cmd = cmd; };

    private:
        void worker();
        bool canPlay();
        bool m_isPlaying;
        std::string m_cmd;
        std::string m_path;
};

const unsigned maxPlayTime = 200;   // maximum stall time time is 200mS

#endif // SYSTEM_CMD_SOUND_H__
