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
