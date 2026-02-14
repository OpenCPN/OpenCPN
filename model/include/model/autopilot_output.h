/***************************************************************************
 *   Copyright (C) 2025 by David S. Register                               *
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
 * Autopilot output support
 */

#ifndef _AUTOPILOTOUTPUT_H__
#define _AUTOPILOTOUTPUT_H__

#include "comm_driver.h"
#include "model/route.h"

bool UpdateAutopilotN0183(Routeman &routeman);
bool UpdateAutopilotN2K(Routeman &routeman);

/** Send RMC + a faked RMB when there is no active route. */
bool SendNoRouteRmbRmc(Routeman &routeman);

bool SendPGN129283(Routeman &routeman, AbstractCommDriver *driver);
bool SendPGN129284(Routeman &routeman, AbstractCommDriver *driver);
bool SendPGN129285(Routeman &routeman, AbstractCommDriver *driver);

#endif
