/***************************************************************************
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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

/**
 * \file
 *
 * AIS decoding functions
 */

#ifndef COMM_AIS_H_
#define COMM_AIS_H_

#include <wx/string.h>

#include "model/ais_target_data.h"
#include "model/ais_bitstring.h"
#include "model/ais_defs.h"
#include "model/ocpn_types.h"

bool Parse_VDXBitstring(AisBitstring *bstr, AisTargetData *ptd);

AisError DecodeSingleVDO(const wxString &str, GenericPosDatEx *pos);

bool NMEA_AISCheckSumOK(const wxString &str);

#endif  // COMM_AIS_H_
