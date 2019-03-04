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
#include "config.h"
#include "SoundFactory.h"


#ifdef HAVE_PORTAUDIO
#include "PortAudioSound.h"

OcpnSound* SoundFactory(void) { return new PortAudioSound(); }
bool IsSoundFactorySynchronous( void ) { return true; }

#elif defined(__OCPN__ANDROID__)
#include "AndroidSound.h"

OcpnSound* SoundFactory(void) { return new AndroidSound(); }
bool IsSoundFactorySynchronous( void ) { return false; }

#elif defined(HAVE_SYSTEM_CMD_SOUND)
#include "SystemCmdSound.h"

OcpnSound* SoundFactory(void) { return new SystemCmdSound(SYSTEM_SOUND_CMD); }
#if defined(__WXMAC__)
bool IsSoundFactorySynchronous( void ) { return false; }
#elif defined(_WINDOWS)
bool IsSoundFactorySynchronous( void ) { return true; }
#else
bool IsSoundFactorySynchronous( void ) { return false; }        // Probably rPI
#endif

#else
#include  "OcpnWxSound.h"

OcpnSound* SoundFactory(void) { return new OcpnWxSound(); }
bool IsSoundFactorySynchronous( void ) { return true; }

#endif
