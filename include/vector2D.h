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




class vector2D
{
public:
    vector2D() { x = 0.0; y = 0.0; }
    vector2D( double a, double b ) { x = a; y = b; }
    friend bool operator==( vector2D &a, vector2D &b ) { return a.x == b.x && a.y == b.y; }
    friend bool operator!=( vector2D &a, vector2D &b ) { return a.x != b.x || a.y != b.y; }
    friend vector2D operator-( vector2D a, vector2D b ) { return vector2D( a.x - b.x, a.y - b.y ); }
    friend vector2D operator+( vector2D a, vector2D b ) { return vector2D( a.x + b.x, a.y + b.y ); }
    friend vector2D operator*( double t, vector2D a ) { return vector2D( a.x * t, a.y * t ); }

    union{ double x; double lon; };
    union{ double y; double lat; };
};

typedef vector2D* pVector2D;

//---------------------------------------------------------------------------------
//      Vector Stuff for Hit Test Algorithm
//---------------------------------------------------------------------------------

extern "C" double vGetLengthOfNormal(pVector2D a, pVector2D b, pVector2D n);
extern "C" double vDotProduct(pVector2D v0, pVector2D v1);
extern "C" pVector2D vAddVectors(pVector2D v0, pVector2D v1, pVector2D v);
extern "C" pVector2D vSubtractVectors(pVector2D v0, pVector2D v1, pVector2D v);
extern "C" double vVectorMagnitude(pVector2D v0);
extern "C" double vVectorSquared(pVector2D v0);


#endif
