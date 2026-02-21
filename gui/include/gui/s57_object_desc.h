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
 **************************************************************************/

/**
 * \file
 *
 * S57 object encapsulation
 */

#ifndef S57OBJECTDESC_H_
#define S57OBJECTDESC_H_

#include <wx/string.h>

/**
 * Describes an S57 object (feature) in an Electronic Navigational Chart.
 * Objects represent various maritime features such as buoys, lights, depth
 * areas, land areas, etc. Each object has a specific class and set of
 * attributes that define its characteristics.
 */
class S57ObjectDesc {
public:
  wxString S57ClassName;
  wxString S57ClassDesc;
  wxString Attributes;
};

#endif
