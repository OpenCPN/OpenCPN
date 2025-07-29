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

#include <wx/string.h>
#include "ais_target_data.h"

#define TRACKTYPE_DEFAULT 0
#define TRACKTYPE_ALWAYS 1
#define TRACKTYPE_NEVER 2

typedef enum AisError {
  AIS_NoError = 0,
  AIS_Partial,
  AIS_NMEAVDX_TOO_LONG,
  AIS_NMEAVDX_CHECKSUM_BAD,
  AIS_NMEAVDX_BAD,
  AIS_NO_SERIAL,
  AIS_NO_TCP,
  AIS_GENERIC_ERROR,
  AIS_INCOMPLETE_MULTIPART
} _AisError;

using AisTargetDataMap = std::unordered_map<int, AisTargetCacheData>;

#define TIMER_AIS_MSEC 998
#define TIMER_AIS_AUDIO_MSEC 2000

#endif  //  _AIS_DEFS_H__
