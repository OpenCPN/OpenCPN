/**************************************************************************
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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 ***************************************************************************/

/**
 * \file
 *
 * Implement idx_entry.h -- tidal data container
 */

#include "idx_entry.h"

#include <string.h>
#include <stdlib.h>
#include <wx/arrimpl.cpp>

WX_DEFINE_OBJARRAY(ArrayOfIDXEntry);

IDX_entry::IDX_entry() { memset(this, 0, sizeof(IDX_entry)); }

IDX_entry::~IDX_entry() { free(IDX_tzname); }
