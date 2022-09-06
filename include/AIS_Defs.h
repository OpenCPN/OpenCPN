/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
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

#ifndef _AIS_DEFS_H__
#define _AIS_DEFS_H__

#include <wx/hashmap.h>
#include <wx/string.h>

typedef enum AIS_Error {
  AIS_NoError = 0,
  AIS_Partial,
  AIS_NMEAVDX_TOO_LONG,
  AIS_NMEAVDX_CHECKSUM_BAD,
  AIS_NMEAVDX_BAD,
  AIS_NO_SERIAL,
  AIS_NO_TCP,
  AIS_GENERIC_ERROR,
  AIS_INCOMPLETE_MULTIPART
} _AIS_Error;

WX_DECLARE_HASH_MAP(int, wxString, wxIntegerHash, wxIntegerEqual,
                    AIS_Target_Name_Hash);

// Global definition for window, timer and other ID's as needed.
enum {
  ID_NMEA_WINDOW = wxID_HIGHEST,
  ID_AIS_WINDOW,
  INIT_TIMER,
  FRAME_TIMER_1,
  FRAME_TIMER_2,
  TIMER_AIS1,
  TIMER_DSC,
  TIMER_AISAUDIO,
  AIS_SOCKET_Isa,
  FRAME_TIMER_DOG,
  FRAME_TC_TIMER,
  FRAME_COG_TIMER,
  MEMORY_FOOTPRINT_TIMER,
  BELLS_TIMER,
  ID_NMEA_THREADMSG,
  RESIZE_TIMER,
  TOOLBAR_ANIMATE_TIMER,
  RECAPTURE_TIMER,
  WATCHDOG_TIMER
};


enum { TIME_TYPE_UTC = 1, TIME_TYPE_LMT, TIME_TYPE_COMPUTER };

#
#define TIMER_AIS_MSEC 998
#define TIMER_AIS_AUDIO_MSEC 2000

#endif  //  _AIS_DEFS_H__
