/***************************************************************************
 *   Copyright (C) 2007 - 2010 by David S. Register, Richard M Smith       *
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
 * MacOS hardware probing functions
 */

#ifndef MACUTILS_H_INCLUDED_H_
#define MACUTILS_H_INCLUDED_H_

#include "config.h"

#ifdef __WXOSX__

#define MAX_SERIAL_PORTS 10

extern "C" int FindSerialPortNames(char** pNames, int iMaxNames);
extern "C" bool ValidateSerialPortName(const char* pPortName,
                                       int iMaxNamesToSearch);
extern "C" int GetMacMonitorSize();

extern "C" int ProcessIsTranslated();
extern "C" int IsAppleSilicon();

#endif  // __WXOSX__

#endif  // MACUTILS_H_INCLUDED_H__
