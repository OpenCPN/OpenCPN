/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  OpenCPN Georef utility
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register   *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.             *
 ***************************************************************************

 ***************************************************************************
 *  Parts of this file were adapted from source code found in              *
 *  John F. Waers (jfwaers@csn.net) public domain program MacGPS45         *
 ***************************************************************************

 */

#ifndef     __GEOREF_H__
#define     __GEOREF_H__

#include <stdio.h>
#include <string.h>
//#include <math.h>
#include <ctype.h>

//------------------------
struct DATUM {
        const char *name;
        short ellipsoid;
        double dx;
        double dy;
        double dz;
};

struct ELLIPSOID {
        const char *name;             // name of ellipsoid
        double a;               // semi-major axis, meters
        double invf;            // 1/f
};

struct GeoRef {
  int status;
  int count;
  int order;
  double *tx;
  double *ty;
  double *lon;
  double *lat;
  double *pwx;
  double *pwy;
  double *wpx;
  double *wpy;
  int    txmax;
  int    tymax;
  int    txmin;
  int    tymin;
  double lonmax;
  double lonmin;
  double latmax;
  double latmin;

};


#ifndef PI
      #define PI        3.1415926535897931160E0      /* pi */
#endif
#define DEGREE    (PI/180.0)
#define RADIAN    (180.0/PI)

#define DATUM_INDEX_WGS84     101
#define DATUM_INDEX_UNKNOWN   -1


static const double WGS84_semimajor_axis_meters       = 6378137.0;           // WGS84 semimajor axis
static const double mercator_k0                       = 0.9996;
static const double WGSinvf                           = 298.257223563;       /* WGS84 1/f */


void datumParams(short datum, double *a, double *es);


//      Make these conversion routines useable by C or C++


#ifdef __cplusplus
extern "C" void toDMS(double a, char *bufp, int bufplen);
extern "C" void toDMM(double a, char *bufp, int bufplen);
extern "C" void todmm(int flag, double a, char *bufp, int bufplen);

extern "C" void toTM(float lat, float lon, float lat0, float lon0, double *x, double *y);
extern "C" void fromTM(double x, double y, double lat0, double lon0, double *lat, double *lon);

extern "C" void toSM(double lat, double lon, double lat0, double lon0, double *x, double *y);
extern "C" double toSMcache_y30(double lat0);
extern "C" void toSMcache(double lat, double lon, double y30, double lon0, double *x, double *y);
extern "C" void fromSM(double x, double y, double lat0, double lon0, double *lat, double *lon);

extern "C" void toSM_ECC(double lat, double lon, double lat0, double lon0, double *x, double *y);
extern "C" void fromSM_ECC(double x, double y, double lat0, double lon0, double *lat, double *lon);

extern "C" void toPOLY(double lat, double lon, double lat0, double lon0, double *x, double *y);
extern "C" void fromPOLY(double x, double y, double lat0, double lon0, double *lat, double *lon);

extern "C" void cache_phi0(double lat0, double *sin_phi0, double *cos_phi0);

extern "C" void toORTHO(double lat, double lon, double sin_phi0, double cos_phi0, double lon0, double *x, double *y);
extern "C" void fromORTHO(double x, double y, double lat0, double lon0, double *lat, double *lon);

extern "C" double toPOLARcache_e(double lat0);
extern "C" void toPOLAR(double lat, double lon, double e, double lat0, double lon0, double *x, double *y);
extern "C" void fromPOLAR(double x, double y, double lat0, double lon0, double *lat, double *lon);

extern "C" void toSTEREO(double lat, double lon, double sin_phi0, double cos_phi0, double lon0, double *x, double *y);
extern "C" void fromSTEREO(double x, double y, double lat0, double lon0, double *lat, double *lon);

extern "C" void toGNO(double lat, double lon, double sin_phi0, double cos_phi0, double lon0, double *x, double *y);
extern "C" void fromGNO(double x, double y, double lat0, double lon0, double *lat, double *lon);

extern "C" void toEQUIRECT(double lat, double lon, double lat0, double lon0, double *x, double *y);
extern "C" void fromEQUIRECT(double x, double y, double lat0, double lon0, double *lat, double *lon);

/// distance in nautical miles
extern "C" void ll_gc_ll(double lat, double lon, double crs, double dist, double *dlat, double *dlon);
extern "C" void ll_gc_ll_reverse(double lat1, double lon1, double lat2, double lon2,
                                double *bearing, double *dist);


extern "C" void PositionBearingDistanceMercator(double lat, double lon, double brg, double dist,
                                                double *dlat, double *dlon);
extern "C" double DistGreatCircle(double slat, double slon, double dlat, double dlon);

extern "C" int GetDatumIndex(const char *str);
extern "C" void MolodenskyTransform (double lat, double lon, double *to_lat, double *to_lon, int from_datum_index, int to_datum_index);

extern "C" void DistanceBearingMercator(double lat0, double lon0, double lat1, double lon1, double *brg, double *dist);

extern "C" int Georef_Calculate_Coefficients(struct GeoRef *cp, int nlin_lon);
extern "C" int Georef_Calculate_Coefficients_Proj(struct GeoRef *cp);
extern "C" double lat_gc_crosses_meridian( double lat1, double lon1, double lat2, double lon2, double lon );
extern "C" double lat_rl_crosses_meridian( double lat1, double lon1, double lat2, double lon2, double lon );

#else
void toDMS(double a, char *bufp, int bufplen);
void toDMM(double a, char *bufp, int bufplen);
int GetDatumIndex(const char *str);
void MolodenskyTransform (double lat, double lon, double *to_lat, double *to_lon, int from_datum_index, int to_datum_index);
double lat_gc_crosses_meridian( double lat1, double lon1, double lat2, double lon2, double lon );
double lat_rl_crosses_meridian( double lat1, double lon1, double lat2, double lon2, double lon );

#endif







//--------------------

/*
 * lmfit
 *
 * Solves or minimizes the sum of squares of m nonlinear
 * functions of n variables.
 *
 * From public domain Fortran version
 * of Argonne National Laboratories MINPACK
 *     argonne national laboratory. minpack project. march 1980.
 *     burton s. garbow, kenneth e. hillstrom, jorge j. more
 * C translation by Steve Moshier
 * Joachim Wuttke converted the source into C++ compatible ANSI style
 * and provided a simplified interface
 */

// parameters for calling the high-level interface lmfit
//   ( lmfit.c provides lm_initialize_control which sets default values ):
typedef struct {
    double ftol;       // relative error desired in the sum of squares.
    double xtol;       // relative error between last two approximations.
    double gtol;       // orthogonality desired between fvec and its derivs.
    double epsilon;    // step used to calculate the jacobian.
    double stepbound;  // initial bound to steps in the outer loop.
    double fnorm;      // norm of the residue vector fvec.
    int maxcall;       // maximum number of iterations.
    int nfev;          // actual number of iterations.
    int info;          // status of minimization.
} lm_control_type;


// the subroutine that calculates fvec:
typedef void (lm_evaluate_ftype) (
        double* par, int m_dat, double* fvec, void *data, int *info );
// default implementation therof, provided by lm_eval.c:
void lm_evaluate_default (
        double* par, int m_dat, double* fvec, void *data, int *info );

// the subroutine that informs about fit progress:
typedef void (lm_print_ftype) (
        int n_par, double* par, int m_dat, double* fvec, void *data,
    int iflag, int iter, int nfev );
// default implementation therof, provided by lm_eval.c:
void lm_print_default (
        int n_par, double* par, int m_dat, double* fvec, void *data,
    int iflag, int iter, int nfev );

// compact high-level interface:
void lm_initialize_control( lm_control_type *control );
void lm_minimize ( int m_dat, int n_par, double* par,
                   lm_evaluate_ftype *evaluate, lm_print_ftype *printout,
                   void *data, lm_control_type *control );
double lm_enorm( int, double* );

// low-level interface for full control:
void lm_lmdif( int m, int n, double* x, double* fvec, double ftol, double xtol,
               double gtol, int maxfev, double epsfcn, double* diag, int mode,
               double factor, int *info, int *nfev,
               double* fjac, int* ipvt, double* qtf,
               double* wa1, double* wa2, double* wa3, double* wa4,
               lm_evaluate_ftype *evaluate, lm_print_ftype *printout,
               void *data );


#ifndef _LMDIF
extern const char *lm_infmsg[];
extern const char *lm_shortmsg[];
#endif

//      This is an opaque (to lmfit) structure set up before the call to lmfit()
typedef struct {
    double* user_tx;
    double* user_ty;
    double* user_y;
    double (*user_func)( double user_tx_point, double user_ty_point, int n_par, double* par );
    int     print_flag;
    int     n_par;
} lm_data_type;



#endif
