/********************************************************************
** @source JEEPS projection functions
**
** @author Copyright (C) 1999 Alan Bleasby
** @version 1.0
** @modified Dec 28 1999 Alan Bleasby. First version
** @modified Copyright (C) 2004, 2005, 2006 Robert Lipe
** @modified Copyright (C) 2007 Achim Schumacher
** @@
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** You should have received a copy of the GNU Library General Public
** License along with this library; if not, write to the
** Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
** Boston, MA 02110-1301,  USA.
********************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

#ifndef gpsproj_h
#define gpsproj_h


#include "garmin_gps.h"

void GPS_Math_Albers_LatLon_To_EN(double phi, double lambda, double *E,
				  double *N, double phi1, double phi2,
				  double phi0, double M0, double E0,
				  double N0, double a, double b);
void GPS_Math_Albers_EN_To_LatLon(double E, double N, double *phi,
				  double *lambda, double phi1, double phi2,
				  double phi0, double M0, double E0,
				  double N0, double a, double b);


void GPS_Math_LambertCC_LatLon_To_EN(double phi, double lambda, double *E,
				     double *N, double phi1, double phi2,
				     double phi0, double M0, double E0,
				     double N0, double a, double b);
void GPS_Math_LambertCC_EN_To_LatLon(double E, double N, double *phi,
				     double *lambda, double phi1, double phi2,
				     double phi0, double M0, double E0,
				     double N0, double a, double b);

void GPS_Math_Miller_LatLon_To_EN(double phi, double lambda, double *E,
				  double *N, double M0, double E0,
				  double N0, double a, double b);
void GPS_Math_Miller_EN_To_LatLon(double E, double N, double *phi,
				  double *lambda, double M0, double E0,
				  double N0, double a, double b);

void GPS_Math_Bonne_LatLon_To_EN(double phi, double lambda, double *E,
				 double *N, double phi0, double M0, double E0,
				 double N0, double a, double b);
void GPS_Math_Bonne_EN_To_LatLon(double E, double N, double *phi,
				 double *lambda, double phi0, double M0,
				 double E0, double N0, double a, double b);

void GPS_Math_Cassini_LatLon_To_EN(double phi, double lambda, double *E,
				   double *N, double phi0, double M0,
				   double E0, double N0, double a, double b);
void GPS_Math_Cassini_EN_To_LatLon(double E, double N, double *phi,
				   double *lambda, double phi0, double M0,
				   double E0, double N0, double a, double b);

void GPS_Math_Cylea_LatLon_To_EN(double phi, double lambda, double *E,
				 double *N, double phi0, double M0,
				 double E0, double N0, double a, double b);
void GPS_Math_Cylea_EN_To_LatLon(double E, double N, double *phi,
				 double *lambda, double phi0, double M0,
				 double E0, double N0, double a, double b);

void GPS_Math_EckertIV_LatLon_To_EN(double phi, double lambda, double *E,
				    double *N, double M0, double E0, double N0,
				    double a, double b);
void GPS_Math_EckertIV_EN_To_LatLon(double E, double N, double *phi,
				    double *lambda, double M0, double E0,
				    double N0, double a, double b);

void GPS_Math_EckertVI_LatLon_To_EN(double phi, double lambda, double *E,
				    double *N, double M0, double E0, double N0,
				    double a, double b);
void GPS_Math_EckertVI_EN_To_LatLon(double E, double N, double *phi,
				    double *lambda, double M0, double E0,
				    double N0, double a, double b);

void GPS_Math_Cyled_LatLon_To_EN(double phi, double lambda, double *E,
				 double *N, double phi0, double M0, double E0,
				 double N0, double a, double b);
void GPS_Math_Cyled_EN_To_LatLon(double E, double N, double *phi,
				 double *lambda, double phi0, double M0,
				 double E0, double N0, double a, double b);

void GPS_Math_VderGrinten_LatLon_To_EN(double phi, double lambda, double *E,
				       double *N, double M0, double E0,
				       double N0, double a, double b);
void GPS_Math_VderGrinten_EN_To_LatLon(double E, double N, double *phi,
				       double *lambda, double M0, double E0,
				       double N0, double a, double b);

void GPS_Math_PolarSt_LatLon_To_EN(double phi, double lambda, double *E,
				   double *N, double phi1, double lambda1,
				   double E0, double N0, double a, double b);
void GPS_Math_PolarSt_EN_To_LatLon(double E, double N, double *phi,
				   double *lambda, double phi1, double lambda1,
				   double E0, double N0, double a, double b);

void GPS_Math_Mollweide_LatLon_To_EN(double phi, double lambda, double *E,
				     double *N, double M0, double E0,
				     double N0, double a, double b);
void GPS_Math_Mollweide_EN_To_LatLon(double E, double N, double *phi,
				     double *lambda, double M0, double E0,
				     double N0, double a, double b);

void GPS_Math_Orthog_LatLon_To_EN(double phi, double lambda, double *E,
				  double *N, double phi0, double lambda0,
				  double E0, double N0, double a, double b);
void GPS_Math_Orthog_EN_To_LatLon(double E, double N, double *phi,
				  double *lambda, double phi0, double lambda0,
				  double E0, double N0, double a, double b);

void GPS_Math_Polycon_LatLon_To_EN(double phi, double lambda, double *E,
				   double *N, double phi0, double M0,
				   double E0, double N0, double a, double b);
void GPS_Math_Polycon_EN_To_LatLon(double E, double N, double *phi,
				   double *lambda, double phi0, double M0,
				   double E0, double N0, double a, double b);

void GPS_Math_Sinusoid_LatLon_To_EN(double phi, double lambda, double *E,
				    double *N, double M0, double E0,
				    double N0, double a, double b);
void GPS_Math_Sinusoid_EN_To_LatLon(double E, double N, double *phi,
				    double *lambda, double M0, double E0,
				    double N0, double a, double b);

void GPS_Math_TCylEA_LatLon_To_EN(double phi, double lambda, double *E,
				  double *N, double phi0, double M0, double E0,
				  double N0, double a, double b);
void GPS_Math_TCylEA_EN_To_LatLon(double E, double N, double *phi,
				  double *lambda, double phi0, double M0,
				  double E0, double N0, double a, double b);

void GPS_Math_Mercator_LatLon_To_EN(double phi, double lambda, double *E,
				    double *N, double phi0, double lambda0,
				    double E0, double N0, double a, double b);
void GPS_Math_Mercator_EN_To_LatLon(double E, double N, double *phi,
				    double *lambda, double phi0,
				    double lambda0, double E0, double N0,
				    double a, double b);

void GPS_Math_TMerc_LatLon_To_EN(double phi, double lambda, double *E,
				 double *N, double phi0, double lambda0,
				 double E0, double N0, double F0,
				 double a, double b);
void GPS_Math_TMerc_EN_To_LatLon(double E, double N, double *phi,
				 double *lambda, double phi0, double lambda0,
				 double E0, double N0, double F0,
				 double a, double b);

void GPS_Math_Swiss_LatLon_To_EN(double phi, double lambda, double *E,
				 double *N,double phi0,double lambda0,
				 double E0, double N0, double a, double b);
void GPS_Math_Swiss_EN_To_LatLon(double E, double N, double *phi,
				 double *lambda, double phi0, double lambda0,
				 double E0, double N0, double a, double b);

#endif

#ifdef __cplusplus
}
#endif
