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

#ifndef __OCPNSOUNDDATA_H__
#define __OCPNSOUNDDATA_H__

#ifdef OCPN_USE_PORTAUDIO

#include "wx/wxprec.h"
#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif //precompiled headers

/// Sound data, as loaded from .wav file:
class OCPNSoundData
{
public:
    OCPNSoundData();
    ~OCPNSoundData();

    // .wav header information:
    unsigned m_channels;       // num of channels (mono:1, stereo:2)
    unsigned m_samplingRate;
    unsigned m_bitsPerSample;  // if 8, then m_data contains unsigned 8bit
    // samples (wxUint8), if 16 then signed 16bit
    // (wxInt16)
    unsigned m_samples;        // length in samples:

    // wave data:
    size_t   m_dataBytes;
    wxUint8 *m_data;           // m_dataBytes bytes of data

    wxUint8 *m_dataWithHeader; // ditto, but prefixed with .wav header
};

#endif

#endif
