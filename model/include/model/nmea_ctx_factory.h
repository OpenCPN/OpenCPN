/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Wrapper for creating an NmeaContext based on global vars
 * Author:   Alec Leamas
 *
 ***************************************************************************
 *   Copyright (C) 2023 by Alec Leamas
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
 **************************************************************************/

#ifndef _NMEA_CTX_FACTORY_H__
#define _NMEA_CTX_FACTORY_H__

#include "nmea0183.h"
#include "model/config_vars.h"

static NmeaContext NmeaCtxFactory() {
  NmeaContext ctx;
  ctx.get_talker_id = []() { return  g_TalkerIdText; };
  ctx.get_apb_precision = []() { return g_NMEAAPBPrecision; };
  return ctx;
}

#endif
