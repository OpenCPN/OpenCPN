/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  S57 Chart Object
 * Author:   David Register
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
 **************************************************************************/

#ifndef __S57SECTOR_H__
#define __S57SECTOR_H__

#include <wx/wx.h>

/**
 * Represents a sector of a light in an S57 chart. Contains information about a
 * light sector, including its position, angular range, visibility range, color,
 * and type.
 */
typedef struct {
  wxPoint2DDouble pos;
  double sector1, sector2;
  double range;
  wxColor color;
  bool iswhite;
  bool isleading;
} s57Sector_t;

#endif
