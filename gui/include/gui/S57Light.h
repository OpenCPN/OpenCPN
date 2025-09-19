/**************************************************************************
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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 **************************************************************************/

/**
 * \file
 *
 * S57 light encapsulation
 */

#ifndef S57LIGHT_H_
#define S57LIGHT_H_

#include <wx/arrstr.h>
#include <wx/string.h>

/**
 * Represents a light feature in an S57 chart. Encapsulates its attributes
 * including position, attributes, and sector information.
 */
class S57Light {
public:
  wxArrayString attributeNames;
  wxArrayString attributeValues;
  wxString position;
  bool hasSectors;
};

#endif
