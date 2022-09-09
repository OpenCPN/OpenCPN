/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2022 by David S. Register                               *
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

#ifndef _COMM_AIS_H__
#define _COMM_AIS_H__

#include "ais_target_data.h"
#include "ais_bitstring.h"
#include "ais_defs.h"
#include "ocpn_types.h"

bool Parse_VDXBitstring(AisBitstring *bstr, AisTargetData *ptd);

AisError DecodeSingleVDO(const wxString &str, GenericPosDatEx *pos);

bool NMEA_AISCheckSumOK(const wxString& str);

#endif    // _COMM_AIS_H__
