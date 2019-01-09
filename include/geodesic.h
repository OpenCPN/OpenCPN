#ifndef GEODESIC_H_
#define GEODESIC_H_

#define _USE_MATH_DEFINES
//#include <cmath>
//#include <math.h>

#ifndef M_PI
#define M_PI        3.1415926535897931160E0      /* pi */
#endif

#define GEODESIC_WGS84_SEMI_MAJORAXIS 6378137.0
#define GEODESIC_WGS84_SEMI_MINORAXIS 6356752.3142
#define GEODESIC_DEG2RAD(d) ((d)*(M_PI/180.0))
#define GEODESIC_RAD2DEG(r) ((r)*(180.0/M_PI))
#define GEODESIC_METERS2NM(m) ((m)*0.000539956803)
#define GEODESIC_NM2METERS(nm) ((nm)/0.000539956803)
#define GEODESIC_METERS2FT(m) ((m)*3.2808399)
#define GEODESIC_FT2METERS(ft) ((ft)/3.2808399)
#define GEODESIC_MPERS2KT(mpers) ((mpers)*1.9438445)
#define GEODESIC_KT2MPERS(mpers) ((mpers)/1.9438445)

class Geodesic {
    /* Find the distance (meters) and bearings between two Lon/Lat pairs (given in degrees)
     * Results are in meters and degrees as appropriate
     */
public:
    static double GreatCircleDistBear( double Lon1, double Lat1, double Lon2, double Lat2,
            double *Dist = NULL, double *Bear1 = NULL, double *Bear2 = NULL );
    static void GreatCircleTravel( double Lon1, double Lat1, double Dist, double Bear1,
            double *Lon2 = NULL, double *Lat2 = NULL, double *Bear2 = NULL );
};

#endif /*GEODESIC_H_*/
