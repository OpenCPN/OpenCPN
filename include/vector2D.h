/******************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************
 */

#ifndef __VECTOR2D_H__
#define __VECTOR2D_H__

#include "RoutePoint.h"

class vector2D
{
public:
    vector2D() { x = 0.0; y = 0.0; }
    vector2D( double a, double b ) { x = a; y = b; }
    void Set( RoutePoint* p ) { lat = p->m_lat; lon = p->m_lon; }
    friend bool operator==( vector2D &a, vector2D &b ) { return a.x == b.x && a.y == b.y; }
    friend bool operator!=( vector2D &a, vector2D &b ) { return a.x != b.x || a.y != b.y; }
    friend vector2D operator-( vector2D a, vector2D b ) { return vector2D( a.x - b.x, a.y - b.y ); }
    friend vector2D operator+( vector2D a, vector2D b ) { return vector2D( a.x + b.x, a.y + b.y ); }
    friend vector2D operator*( double t, vector2D a ) { return vector2D( a.x * t, a.y * t ); }

    union{ double x; double lon; };
    union{ double y; double lat; };
};

typedef vector2D* pVector2D;

#endif
