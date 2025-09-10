/**************************************************************************
 *   Copyright (C) 2022 Alec Leamas                                        *
 *   Copyright (C) 2022 David Register                                     *
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

/**
 * \file
 * Communication drivers factory and support
 */

#ifndef _COMM_DRV_FACTORY_H
#define _COMM_DRV_FACTORY_H

#include "model/conn_params.h"
#include "model/comm_driver.h"

/**
 * DriverListener which handles incoming N0183 data. By default
 * messages are just handed to NavmsgBus and thus becomes
 * available for both upper layers and plugins.
 *
 * Messages classified as junk, filtered or with bad checksum
 * are handed to the evt_dropped_msg EventVar on
 * CommDrvRegistry to be available for the Data Monitor.
 */
class N0183Listener;

/** Create and register a driver for given connection. */
void MakeCommDriver(const ConnectionParams* params);

/** Create and register the loopback driver. */
void MakeLoopbackDriver();

void initIXNetSystem();
void uninitIXNetSystem();

#endif  // _COMM_DRV_FACTORY_H
