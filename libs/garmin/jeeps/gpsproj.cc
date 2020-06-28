/********************************************************************
** @source JEEPS projection functions
**
** @author Copyright (C) 1999 Alan Bleasby
** @version 1.0
** @modified Feb 04 2000 Alan Bleasby. First version
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
** Boston, MA  02110-1301, USA.
********************************************************************/
#include "gps.h"
#include <cmath>
#include <cstring>


/* @func GPS_Math_Albers_LatLon_To_EN **********************************
**
** Convert latitude and longitude to Albers projection easting and
** northing
**
** @param [r] phi [double] latitude (deg)
** @param [r] lambda [double] longitude (deg)
** @param [w] E [double *] easting (metre)
** @param [w] N [double *] northing (metre)
** @param [r] phi1 [double] standard latitude (parallel) 1 (deg)
** @param [r] phi2 [double] standard latitude (parallel) 2 (deg)
** @param [r] phi0 [double] latitude of origin (deg)
** @param [r] M0 [double] central meridian (deg)
** @param [r] E0 [double] false easting
** @param [r] N0 [double] false northing
** @param [r] a [double] semi-major axis
** @param [r] b [double] semi-minor axis
**
** @return [void]
************************************************************************/
void GPS_Math_Albers_LatLon_To_EN(double phi, double lambda, double* E,
                                  double* N, double phi1, double phi2,
                                  double phi0, double M0, double E0,
                                  double N0, double a, double b)

{
  double dlambda;
  double phis;
  double phic;
  double e;
  double esq;
  double esqs;
  double omesqs2;

  double a2;
  double b2;
  double q;
  double q0;
  double q1;
  double q2;
  double m1;
  double m2;
  double n;
  double phi0s;
  double phi1s;
  double phi1c;
  double phi2s;
  double phi2c;
  double ess;
  double om0;
  double m1sq;
  double C;
  double nq;
  double nq0;
  double rho;
  double rho0;
  double theta;

  phi     = GPS_Math_Deg_To_Rad(phi);
  phi0    = GPS_Math_Deg_To_Rad(phi0);
  phi1    = GPS_Math_Deg_To_Rad(phi1);
  phi2    = GPS_Math_Deg_To_Rad(phi2);
  lambda  = GPS_Math_Deg_To_Rad(lambda);
  M0      = GPS_Math_Deg_To_Rad(M0);

  dlambda = lambda - M0;
  if (dlambda > GPS_PI) {
    dlambda -= ((double)2.0 * GPS_PI);
  }
  if (dlambda < -GPS_PI) {
    dlambda += ((double)2.0 * GPS_PI);
  }

  phis = sin(phi);
  phic = cos(phi);

  a2 = a*a;
  b2 = b*b;
  esq = (a2-b2)/a2;
  e   = pow(esq,(double)0.5);


  phi0s = sin(phi0);
  ess = e * phi0s;
  om0 = ((double)1.0 - ess*ess);
  q0  = ((double)1.0 - esq) * (phi0s / om0-((double)1.0/(e+e)) *
                               log(((double)1.0-ess)/((double)1.0+ess)));
  phi1s = sin(phi1);
  phi1c = cos(phi1);
  ess = e * phi1s;
  om0 = ((double)1.0 - ess*ess);
  m1 = phi1c/pow(om0,(double)0.5);
  q1  = ((double)1.0 - esq) * (phi1s / om0-((double)1.0/(e+e)) *
                               log(((double)1.0-ess)/((double)1.0+ess)));

  m1sq = m1*m1;
  if (fabs(phi1-phi2)>1.0e-10) {
    phi2s = sin(phi2);
    phi2c = cos(phi2);
    ess   = e * phi2s;
    om0   = ((double)1.0 - ess*ess);
    m2 = phi2c/pow(om0,(double)0.5);
    q2 = ((double)1.0 - esq) * (phi2s / om0-((double)1.0/(e+e)) *
                                log(((double)1.0-ess)/((double)1.0+ess)));
    n  = (m1sq - m2*m2) / (q2-q1);
  } else {
    n  = phi1s;
  }

  C = m1sq + n*q1;
  nq0 = n * q0;
  if (C < nq0) {
    rho0 = (double)0.;
  } else {
    rho0 = (a/n) * pow(C-nq0,(double)0.5);
  }


  esqs = e * phis;
  omesqs2 = ((double)1.0 - esqs*esqs);
  q  = ((double)1.0 - esq) * (phis / omesqs2-((double)1.0/(e+e)) *
                              log(((double)1.0-esqs)/((double)1.0+esqs)));
  nq = n*q;
  if (C<nq) {
    rho = (double)0.;
  } else {
    rho = (a/n) * pow(C-nq,(double)0.5);
  }

  theta = n*dlambda;
  *E = rho * sin(theta) + E0;
  *N = rho0 - rho * cos(theta) + N0;

  return;
}




/* @func GPS_Math_Albers_EN_To_LatLon **********************************
**
** Convert Albers easting and northing to latitude and longitude
**
** @param [r] E [double] easting (metre)
** @param [r] N [double] northing (metre)
** @param [w] phi [double *] latitude (deg)
** @param [w] lambda [double *] longitude (deg)
** @param [r] phi1 [double] standard latitude (parallel) 1 (deg)
** @param [r] phi2 [double] standard latitude (parallel) 2 (deg)
** @param [r] phi0 [double] latitude of origin (deg)
** @param [r] M0 [double] central meridian (deg)
** @param [r] E0 [double] false easting
** @param [r] N0 [double] false northing
** @param [r] a [double] semi-major axis
** @param [r] b [double] semi-minor axis
**
** @return [void]
************************************************************************/
void GPS_Math_Albers_EN_To_LatLon(double E, double N, double* phi,
                                  double* lambda, double phi1, double phi2,
                                  double phi0, double M0, double E0,
                                  double N0, double a, double b)
{
  double po2;
  double rho;
  double rho0;
  double C;
  double a2;
  double b2;
  double esq;
  double e;
  double phi0s;
  double q0;
  double q1;
  double q2;
  double phi1s;
  double phi1c;
  double phi2s;
  double phi2c;
  double m1;
  double m1sq;
  double m2;
  double n;
  double nq0;

  double dx;
  double dy;
  double rhom;
  double q;
  double qc;
  double qd2;
  double rhon;
  double lat;
  double dphi;
  double phis;
  double ess;
  double om0;
  double theta;
  double tol;


  phi0    = GPS_Math_Deg_To_Rad(phi0);
  phi1    = GPS_Math_Deg_To_Rad(phi1);
  phi2    = GPS_Math_Deg_To_Rad(phi2);
  M0      = GPS_Math_Deg_To_Rad(M0);

  a2 = a*a;
  b2 = b*b;
  esq = (a2-b2)/a2;
  e   = pow(esq,(double)0.5);


  phi0s = sin(phi0);
  ess = e * phi0s;
  om0 = ((double)1.0 - ess*ess);
  q0  = ((double)1.0 - esq) * (phi0s / om0-((double)1.0/(e+e)) *
                               log(((double)1.0-ess)/((double)1.0+ess)));
  phi1s = sin(phi1);
  phi1c = cos(phi1);
  ess = e * phi1s;
  om0 = ((double)1.0 - ess*ess);
  m1 = phi1c/pow(om0,(double)0.5);
  q1  = ((double)1.0 - esq) * (phi1s / om0-((double)1.0/(e+e)) *
                               log(((double)1.0-ess)/((double)1.0+ess)));

  m1sq = m1*m1;
  if (fabs(phi1-phi2)>1.0e-10) {
    phi2s = sin(phi2);
    phi2c = cos(phi2);
    ess   = e * phi2s;
    om0   = ((double)1.0 - ess*ess);
    m2 = phi2c/pow(om0,(double)0.5);
    q2 = ((double)1.0 - esq) * (phi2s / om0-((double)1.0/(e+e)) *
                                log(((double)1.0-ess)/((double)1.0+ess)));
    n  = (m1sq - m2*m2) / (q2-q1);
  } else {
    n  = phi1s;
  }

  C = m1sq + n*q1;
  nq0 = n * q0;
  if (C < nq0) {
    rho0 = (double)0.;
  } else {
    rho0 = (a/n) * pow(C-nq0,(double)0.5);
  }


  dphi  = (double) 1.0;
  theta = (double) 0.0;
  tol   = (double) 4.85e-10;
  po2   = (double)GPS_PI / (double)2.0;

  dy   = N-N0;
  dx   = E-E0;
  rhom = rho0-dy;
  rho  = pow(dx*dx+rhom*rhom,(double)0.5);

  if (n<0.0) {
    rho  *= (double)-1.0;
    dx   *= (double)-1.0;
    dy   *= (double)-1.0;
    rhom *= (double)-1.0;
  }

  if (rho) {
    theta = atan2(dx,rhom);
  }
  rhon = rho*n;
  q    = (C - (rhon*rhon) / a2) / n;
  qc   = (double)1.0 - ((double)1.0 / (e+e)) *
         log(((double)1.0-e)/((double)1.0+e));
  if (fabs(fabs(qc)-fabs(q))>1.9e-6) {
    qd2 = q/(double)2.0;
    if (qd2>1.0) {
      *phi = po2;
    } else if (qd2<-1.0) {
      *phi = -po2;
    } else {
      lat = asin(qd2);
      if (e<1.0e-10) {
        *phi = lat;
      } else {
        while (fabs(dphi)>tol) {
          phis = sin(lat);
          ess  = e*phis;
          om0  = ((double)1.0 - ess*ess);
          dphi = (om0*om0) / ((double)2.0*cos(lat))*
                 (q/((double)1.0-esq) - phis / om0 +
                  (log(((double)1.0-ess)/((double)1.0+ess)) /
                   (e+e)));
          lat += dphi;
        }
        *phi = lat;
      }

      if (*phi > po2) {
        *phi = po2;
      } else if (*phi<-po2) {
        *phi = -po2;
      }
    }
  } else {
    if (q>=0.0) {
      *phi = po2;
    } else {
      *phi = -po2;
    }
  }

  *lambda = M0 + theta / n;
  if (*lambda > GPS_PI) {
    *lambda -= GPS_PI * (double)2.0;
  }
  if (*lambda < -GPS_PI) {
    *lambda += GPS_PI * (double)2.0;
  }
  if (*lambda>GPS_PI) {
    *lambda = GPS_PI;
  } else if (*lambda<-GPS_PI) {
    *lambda = -GPS_PI;
  }

  *phi    = GPS_Math_Rad_To_Deg(*phi);
  *lambda = GPS_Math_Rad_To_Deg(*lambda);

  return;
}



/* @func GPS_Math_LambertCC_LatLon_To_EN **********************************
**
** Convert latitude and longitude to Lambert Conformal Conic projection
** easting and northing
**
** @param [r] phi [double] latitude (deg)
** @param [r] lambda [double] longitude (deg)
** @param [w] E [double *] easting (metre)
** @param [w] N [double *] northing (metre)
** @param [r] phi1 [double] standard latitude (parallel) 1 (deg)
** @param [r] phi2 [double] standard latitude (parallel) 2 (deg)
** @param [r] phi0 [double] latitude of origin (deg)
** @param [r] M0 [double] central meridian (deg)
** @param [r] E0 [double] false easting
** @param [r] N0 [double] false northing
** @param [r] a [double] semi-major axis
** @param [r] b [double] semi-minor axis
**
** @return [void]
************************************************************************/
void GPS_Math_LambertCC_LatLon_To_EN(double phi, double lambda, double* E,
                                     double* N, double phi1, double phi2,
                                     double phi0, double M0, double E0,
                                     double N0, double a, double b)

{
  double po2;
  double po4;
  double a2;
  double b2;
  double phi0s;
  double e;
  double esq;
  double ed2;
  double ess;
  double t0;
  double t1;
  double t2;
  double m1;
  double m2;
  double phi1s;
  double phi1c;
  double phi2s;
  double phi2c;
  double n;
  double F;
  double Fa;
  double rho;
  double rho0;
  double phis;
  double t;
  double theta;
  double dphi;

  phi     = GPS_Math_Deg_To_Rad(phi);
  phi0    = GPS_Math_Deg_To_Rad(phi0);
  phi1    = GPS_Math_Deg_To_Rad(phi1);
  phi2    = GPS_Math_Deg_To_Rad(phi2);
  lambda  = GPS_Math_Deg_To_Rad(lambda);
  M0      = GPS_Math_Deg_To_Rad(M0);


  po2 = (double)GPS_PI / (double)2.0;
  po4 = (double)GPS_PI / (double)4.0;
  a2  = a*a;
  b2  = b*b;
  esq = (a2-b2)/a2;
  e   = pow(esq,(double)0.5);
  ed2 = e / (double)2.0;

  phi0s = sin(phi0);
  ess   = e * phi0s;
  t0    = tan(po4-phi0/(double)2.0) / pow(((double)1.0-ess) /
                                          ((double)1.0+ess),ed2);


  phi1s = sin(phi1);
  phi1c = cos(phi1);
  ess   = e * phi1s;
  m1    = phi1c / pow(((double)1.0-ess*ess),(double)0.5);
  t1    = tan(po4-phi1/(double)2.0) / pow(((double)1.0-ess) /
                                          ((double)1.0+ess),ed2);

  if (fabs(phi1-phi2)>1.0e-10) {
    phi2s = sin(phi2);
    phi2c = cos(phi2);
    ess   = e * phi2s;
    m2    = phi2c / pow(((double)1.0-ess*ess),(double)0.5);
    t2    = tan(po4-phi2/(double)2.0) / pow(((double)1.0-ess) /
                                            ((double)1.0+ess),ed2);
    n     = log(m1/m2) / log(t1/t2);
  } else {
    n = phi1s;
  }

  F  = m1 / (n*pow(t1,n));
  Fa = F*a;

  rho0 = pow(t0,n) * Fa;

  if (fabs(fabs(phi)-po2)>1.0e-10) {
    phis = sin(phi);
    ess  = e * phis;
    t    = tan(po4-phi/(double)2.0) / pow(((double)1.0-ess) /
                                          ((double)1.0+ess),ed2);
    rho  = pow(t,n) * Fa;
  } else {
    if ((phi*n)<=(double)0.0) {
      return;
    }
    rho = (double)0.0;
  }

  dphi = lambda - M0;
  if (dphi>GPS_PI) {
    dphi -= (double)GPS_PI * (double)2.0;
  }
  if (dphi<-GPS_PI) {
    dphi += (double)GPS_PI * (double)2.0;
  }
  theta = dphi*n;

  *E = rho * sin(theta) + E0;
  *N = rho0 - rho * cos(theta) + N0;

  return;
}




/* @func GPS_Math_LambertCC_EN_To_LatLon **********************************
**
** Convert Lambert Conformal Conic  easting and northing to latitude and
** longitude
**
** @param [r] E [double] easting (metre)
** @param [r] N [double] northing (metre)
** @param [w] phi [double *] latitude (deg)
** @param [w] lambda [double *] longitude (deg)
** @param [r] phi1 [double] standard latitude (parallel) 1 (deg)
** @param [r] phi2 [double] standard latitude (parallel) 2 (deg)
** @param [r] phi0 [double] latitude of origin (deg)
** @param [r] M0 [double] central meridian (deg)
** @param [r] E0 [double] false easting
** @param [r] N0 [double] false northing
** @param [r] a [double] semi-major axis
** @param [r] b [double] semi-minor axis
**
** @return [void]
************************************************************************/
void GPS_Math_LambertCC_EN_To_LatLon(double E, double N, double* phi,
                                     double* lambda, double phi1, double phi2,
                                     double phi0, double M0, double E0,
                                     double N0, double a, double b)
{
  double po2;
  double po4;
  double a2;
  double b2;
  double phi0s;
  double e;
  double esq;
  double ed2;
  double ess;
  double t0;
  double t1;
  double t2;
  double m1;
  double m2;
  double phi1s;
  double phi1c;
  double phi2s;
  double phi2c;
  double n;
  double F;
  double Fa;
  double rho;
  double rho0;
  double phis;
  double t;
  double theta;

  double dx;
  double dy;
  double rhom;
  double lat;
  double tlat;
  double tol;



  phi0    = GPS_Math_Deg_To_Rad(phi0);
  phi1    = GPS_Math_Deg_To_Rad(phi1);
  phi2    = GPS_Math_Deg_To_Rad(phi2);
  M0      = GPS_Math_Deg_To_Rad(M0);


  po2 = (double)GPS_PI / (double)2.0;
  po4 = (double)GPS_PI / (double)4.0;
  a2  = a*a;
  b2  = b*b;
  esq = (a2-b2)/a2;
  e   = pow(esq,(double)0.5);
  ed2 = e / (double)2.0;

  phi0s = sin(phi0);
  ess   = e * phi0s;
  t0    = tan(po4-phi0/(double)2.0) / pow(((double)1.0-ess) /
                                          ((double)1.0+ess),ed2);


  phi1s = sin(phi1);
  phi1c = cos(phi1);
  ess   = e * phi1s;
  m1    = phi1c / pow(((double)1.0-ess*ess),(double)0.5);
  t1    = tan(po4-phi1/(double)2.0) / pow(((double)1.0-ess) /
                                          ((double)1.0+ess),ed2);

  if (fabs(phi1-phi2)>1.0e-10) {
    phi2s = sin(phi2);
    phi2c = cos(phi2);
    ess   = e * phi2s;
    m2    = phi2c / pow(((double)1.0-ess*ess),(double)0.5);
    t2    = tan(po4-phi2/(double)2.0) / pow(((double)1.0-ess) /
                                            ((double)1.0+ess),ed2);
    n     = log(m1/m2) / log(t1/t2);
  } else {
    n = phi1s;
  }

  F  = m1 / (n*pow(t1,n));
  Fa = F*a;

  rho0 = pow(t0,n) * Fa;

  tlat = theta = (double)0.0;
  tol  = (double)4.85e-10;

  dx = E - E0;
  dy = N - N0;
  rhom = rho0 - dy;
  rho  = pow(dx*dx + rhom*rhom,(double)0.5);

  if (n<0.0) {
    rhom *= (double)-1.0;
    dy   *= (double)-1.0;
    dx   *= (double)-1.0;
    rho  *= (double)-1.0;
  }

  if (rho) {
    theta = atan2(dx,rhom);
    t = pow(rho/Fa,(double)1.0/n);
    lat = po2 - (double)2.0*atan(t);
    while (fabs(lat-tlat)>tol) {
      tlat = lat;
      phis = sin(lat);
      ess  = e * phis;
      lat  = po2 - (double)2.0 * atan(t*pow(((double)1.0-ess) /
                                            ((double)1.0+ess),
                                            e / (double)2.0));
    }
    *phi = lat;
    *lambda = theta/n + M0;

    if (*phi>po2) {
      *phi=po2;
    } else if (*phi<-po2) {
      *phi=-po2;
    }
    if (*lambda>GPS_PI) {
      *lambda -= (double)GPS_PI * (double)2.0;
    }
    if (*lambda<-GPS_PI) {
      *lambda += (double)GPS_PI * (double)2.0;
    }

    if (*lambda>GPS_PI) {
      *lambda = GPS_PI;
    } else if (*lambda<-GPS_PI) {
      *lambda = -GPS_PI;
    }
  } else {
    if (n>0.0) {
      *phi = po2;
    } else {
      *phi = -po2;
    }
    *lambda = M0;
  }

  *lambda = GPS_Math_Rad_To_Deg(*lambda);
  *phi    = GPS_Math_Rad_To_Deg(*phi);

  return;
}




/* @func GPS_Math_Miller_LatLon_To_EN **********************************
**
** Convert latitude and longitude to Miller Cylindrical projection easting and
** northing
**
** @param [r] phi [double] latitude (deg)
** @param [r] lambda [double] longitude (deg)
** @param [w] E [double *] easting (metre)
** @param [w] N [double *] northing (metre)
** @param [r] M0 [double] central meridian (deg)
** @param [r] E0 [double] false easting
** @param [r] N0 [double] false northing
** @param [r] a [double] semi-major axis
** @param [r] b [double] semi-minor axis
**
** @return [void]
************************************************************************/
void GPS_Math_Miller_LatLon_To_EN(double phi, double lambda, double* E,
                                  double* N, double M0, double E0,
                                  double N0, double a, double b)
{
  double a2;
  double b2;
  double R;
  double e2;
  double e4;
  double e6;
  double p2;
  double po2;
  double phis;
  double dlam;


  phi     = GPS_Math_Deg_To_Rad(phi);
  lambda  = GPS_Math_Deg_To_Rad(lambda);
  M0      = GPS_Math_Deg_To_Rad(M0);

  po2 = (double)GPS_PI / (double)2.0;
  p2  = (double)GPS_PI * (double)2.0;
  a2  = a*a;
  b2  = b*b;
  e2 = (a2-b2)/a2;
  e4 = e2*e2;
  e6 = e4*e2;

  R = a*((double)1.0-e2/(double)6.0-(double)17.0*e4/(double)360.0-
         (double)67.0*e6/(double)3024.0);

  if (M0>GPS_PI) {
    M0 -= p2;
  }

  phis = sin((double)0.8 * phi);

  dlam = lambda - M0;
  if (dlam>GPS_PI) {
    dlam-=p2;
  }
  if (dlam<-GPS_PI) {
    dlam+=p2;
  }

  *E = R*dlam+E0;
  *N = (R/(double)1.6) * log(((double)1.0+phis) / ((double)1.0-phis)) + N0;

  return;
}




/* @func GPS_Math_Miller_EN_To_LatLon **********************************
**
** Convert latitude and longitude to Miller Cylindrical projection easting and
** northing
**
** @param [r] E [double] easting (metre)
** @param [r] N [double] northing (metre)
** @param [w] phi [double *] latitude (deg)
** @param [w] lambda [double *] longitude (deg)
** @param [r] M0 [double] central meridian (deg)
** @param [r] E0 [double] false easting
** @param [r] N0 [double] false northing
** @param [r] a [double] semi-major axis
** @param [r] b [double] semi-minor axis
**
** @return [void]
************************************************************************/
void GPS_Math_Miller_EN_To_LatLon(double E, double N, double* phi,
                                  double* lambda, double M0, double E0,
                                  double N0, double a, double b)
{
  double a2;
  double b2;
  double R;
  double e;
  double e2;
  double e4;
  double e6;
  double p2;
  double po2;
  double dx;
  double dy;

  dx = E - E0;
  dy = N - N0;

  M0      = GPS_Math_Deg_To_Rad(M0);

  po2 = (double)GPS_PI / (double)2.0;
  p2  = (double)GPS_PI * (double)2.0;
  a2  = a*a;
  b2  = b*b;
  e2 = (a2-b2)/a2;
  e4 = e2*e2;
  e6 = e4*e2;
  e   = pow(e2,(double)0.5);

  R = a*((double)1.0-e2/(double)6.0-(double)17.0*e4/(double)360.0-
         (double)67.0*e6/(double)3024.0);
  if (M0>GPS_PI) {
    M0 -= p2;
  }

  *phi    = atan(sinh((double)0.8*dy/R)) / (double)0.8;
  *lambda = M0+dx/R;

  if (*phi>po2) {
    *phi=po2;
  } else if (*phi<-po2) {
    *phi=-po2;
  }

  if (*lambda>GPS_PI) {
    *lambda-=p2;
  }
  if (*lambda<-GPS_PI) {
    *lambda+=p2;
  }

  if (*lambda>GPS_PI) {
    *lambda=GPS_PI;
  } else if (*lambda<-GPS_PI) {
    *lambda=-GPS_PI;
  }

  *lambda = GPS_Math_Rad_To_Deg(*lambda);
  *phi    = GPS_Math_Rad_To_Deg(*phi);

  return;
}




/* @func GPS_Math_Bonne_LatLon_To_EN **********************************
**
** Convert latitude and longitude to Bonne pseudoconic equal area  projection
**  easting and northing
**
** @param [r] phi [double] latitude (deg)
** @param [r] lambda [double] longitude (deg)
** @param [w] E [double *] easting (metre)
** @param [w] N [double *] northing (metre)
** @param [r] phi0 [double] latitude of origin (deg)
** @param [r] M0 [double] central meridian (deg)
** @param [r] E0 [double] false easting
** @param [r] N0 [double] false northing
** @param [r] a [double] semi-major axis
** @param [r] b [double] semi-minor axis
**
** @return [void]
************************************************************************/
void GPS_Math_Bonne_LatLon_To_EN(double phi, double lambda, double* E,
                                 double* N, double phi0, double M0, double E0,
                                 double N0, double a, double b)
{
  double p2;
  double po2;
  double a2;
  double b2;
  double e2;
  double e4;
  double e6;
  double M1;
  double m1;
  double c0;
  double c1;
  double c2;
  double c3;
  double j;
  double te4;
  double E1;
  double E2;
  double E3;
  double E4;
  double x;
  double phi0s;
  double lat;
  double phi0c;
  double phi0s2;
  double phi0s4;
  double phi0s6;
  double as;

  double phis;
  double phic;
  double phis2;
  double phis4;
  double phis6;
  double dlam;
  double mm;
  double MM;
  double rho;
  double EE;
  double tol;


  lambda = GPS_Math_Deg_To_Rad(lambda);
  phi    = GPS_Math_Deg_To_Rad(phi);
  phi0   = GPS_Math_Deg_To_Rad(phi0);
  M0     = GPS_Math_Deg_To_Rad(M0);

  phi0s = sin(phi0);
  p2 = (double)GPS_PI * (double)2.0;
  po2 = (double)GPS_PI / (double)2.0;
  if (M0>GPS_PI) {
    M0 -= p2;
  }
  a2 = a*a;
  b2 = b*b;
  e2 = (a2-b2)/a2;
  e4 = e2*e2;
  e6 = e2*e4;

  j = (double)45.0*e6/(double)1024.0;
  te4 = (double)3.0 * e4;
  c0 = (double)1.0-e2/(double)4.0-te4/(double)64.0-(double)5.0*e6/
       (double)256.0;
  c1 = (double)3.0*e2/(double)8.0+te4/(double)32.0+j;
  c2 = (double)15.0*e4/(double)256.0+j;
  c3 = (double)35.0*e6/(double)3072.0;

  phi0c = cos(phi0);
  m1 = phi0c/ pow(((double)1.0-e2*phi0s*phi0s),(double)0.5);
  lat = c0 * phi0;

  phi0s2 = c1 * sin((double)2.0*phi0);
  phi0s4 = c2 * sin((double)4.0*phi0);
  phi0s6 = c3 * sin((double)6.0*phi0);
  M1 = a*(lat-phi0s2+phi0s4-phi0s6);

  x = pow((double)1.0-e2,(double)0.5);
  E1 = ((double)1.0-x) / ((double)1.0+x);
  E2 = E1*E1;
  E3 = E2*E1;
  E4 = E3*E1;

  if (!phi0s) {
    as = (double)0.0;
  } else {
    as = a*m1/phi0s;
  }


  dlam = lambda - M0;
  if (dlam>GPS_PI) {
    dlam -= p2;
  }
  if (dlam<-GPS_PI) {
    dlam += p2;
  }

  phis = sin(phi);
  phic = cos(phi);

  tol = (double)0.0001;
  if (!(phi-phi0) && (((po2-tol)<fabs(phi)) && (fabs(phi)<po2+tol))) {
    *E = *N = (double)0.0;
  } else {
    mm = phic / pow(((double)1.0-e2*phis*phis),(double)0.5);
    lat   = c0 * phi;
    phis2 = c1 * sin((double)2.0*phi);
    phis4 = c2 * sin((double)4.0*phi);
    phis6 = c3 * sin((double)6.0*phi);
    MM = a * (lat-phis2+phis4-phis6);

    rho = as + M1 - MM;
    if (!rho) {
      EE = (double)0.0;
    } else {
      EE = a * mm * dlam / rho;
    }

    *E = rho * sin(EE) + E0;
    *N = as - rho * cos(EE) + N0;
  }

  return;
}




/* @func GPS_Math_Bonne_EN_To_LatLon **********************************
**
** Convert Bonne pseudoconic equal area easting and northing projection
** to latitude and longitude
**
** @param [r] E [double] easting (metre)
** @param [r] N [double] northing (metre)
** @param [w] phi [double *] latitude (deg)
** @param [w] lambda [double *] longitude (deg)
** @param [r] phi0 [double] latitude of origin (deg)
** @param [r] M0 [double] central meridian (deg)
** @param [r] E0 [double] false easting
** @param [r] N0 [double] false northing
** @param [r] a [double] semi-major axis
** @param [r] b [double] semi-minor axis
**
** @return [void]
************************************************************************/
void GPS_Math_Bonne_EN_To_LatLon(double E, double N, double* phi,
                                 double* lambda, double phi0, double M0,
                                 double E0, double N0, double a, double b)
{
  double p2;
  double po2;
  double a2;
  double b2;
  double e2;
  double e4;
  double e6;
  double M1;
  double m1;
  double c0;
  double c1;
  double c2;
  double c3;
  double A0;
  double A1;
  double A2;
  double A3;
  double j;
  double te4;
  double E1;
  double E2;
  double E3;
  double E4;
  double x;
  double phi0s;
  double lat;
  double phi0c;
  double phi0s2;
  double phi0s4;
  double phi0s6;
  double as;

  double phis;
  double phic;
  double dx;
  double dy;
  double mu;
  double mm;
  double MM;
  double asdy;
  double rho;
  double smu2;
  double smu4;
  double smu6;
  double smu8;
  double tol;


  phi0   = GPS_Math_Deg_To_Rad(phi0);
  M0     = GPS_Math_Deg_To_Rad(M0);

  phi0s = sin(phi0);
  p2 = (double)GPS_PI * (double)2.0;
  po2 = (double)GPS_PI / (double)2.0;
  if (M0>GPS_PI) {
    M0 -= p2;
  }
  a2 = a*a;
  b2 = b*b;
  e2 = (a2-b2)/a2;
  e4 = e2*e2;
  e6 = e2*e4;

  j = (double)45.0*e6/(double)1024.0;
  te4 = (double)3.0 * e4;
  c0 = (double)1.0-e2/(double)4.0-te4/(double)64.0-(double)5.0*e6/
       (double)256.0;
  c1 = (double)3.0*e2/(double)8.0+te4/(double)32.0+j;
  c2 = (double)15.0*e4/(double)256.0+j;
  c3 = (double)35.0*e6/(double)3072.0;

  phi0c = cos(phi0);
  m1 = phi0c/ pow(((double)1.0-e2*phi0s*phi0s),(double)0.5);
  lat = c0 * phi0;

  phi0s2 = c1 * sin((double)2.0*phi0);
  phi0s4 = c2 * sin((double)4.0*phi0);
  phi0s6 = c3 * sin((double)6.0*phi0);
  M1 = a*(lat-phi0s2+phi0s4-phi0s6);

  x = pow((double)1.0-e2,(double)0.5);
  E1 = ((double)1.0-x) / ((double)1.0+x);
  E2 = E1*E1;
  E3 = E2*E1;
  E4 = E3*E1;
  A0 = (double)3.0*E1/(double)2.0-(double)27.0*E3/(double)32.0;
  A1 = (double)21.0*E2/(double)16.0-(double)55.0*E4/(double)32.0;
  A2 = (double)151.0*E3/(double)96.0;
  A3 = (double)1097.0*E4/(double)512.0;
  if (!phi0s) {
    as = (double)0.0;
  } else {
    as = a*m1/phi0s;
  }


  dx = E - E0;
  dy = N - N0;
  asdy = as - dy;
  rho = pow(dx*dx+asdy*asdy,(double)0.5);
  if (phi0<(double)0.0) {
    rho=-rho;
  }
  MM = as+M1-rho;

  mu = MM / (a*c0);
  smu2 = A0 * sin((double)2.0*mu);
  smu4 = A1 * sin((double)4.0*mu);
  smu6 = A2 * sin((double)6.0*mu);
  smu8 = A3 * sin((double)8.0*mu);
  *phi = mu+smu2+smu4+smu6+smu8;

  tol = (double)0.00001;
  if (((po2-tol)<fabs(*phi)) && (fabs(*phi)<po2+tol)) {
    *lambda = M0;
  } else {
    phic = cos(*phi);
    phis = sin(*phi);
    mm   = phic / pow(((double)1.0-e2*phis*phis),(double)0.5);
    if (phi0<(double)0.0) {
      dx = -dx;
      asdy = -asdy;
    }
    *lambda = M0 + rho * (atan2(dx,asdy)) / (a * mm);
  }

  if (*phi>po2) {
    *phi = po2;
  } else if (*phi<-po2) {
    *phi = -po2;
  }

  if (*lambda>GPS_PI) {
    *lambda -= p2;
  }
  if (*lambda<-GPS_PI) {
    *lambda += p2;
  }

  if (*lambda>GPS_PI) {
    *lambda = GPS_PI;
  } else if (*lambda<-GPS_PI) {
    *lambda=-GPS_PI;
  }

  *lambda = GPS_Math_Rad_To_Deg(*lambda);
  *phi    = GPS_Math_Rad_To_Deg(*phi);

  return;
}




/* @func GPS_Math_Cassini_LatLon_To_EN **********************************
**
** Convert latitude and longitude to Cassini transverse cylindrical projection
**  easting and northing
**
** @param [r] phi [double] latitude (deg)
** @param [r] lambda [double] longitude (deg)
** @param [w] E [double *] easting (metre)
** @param [w] N [double *] northing (metre)
** @param [r] phi0 [double] latitude of origin (deg)
** @param [r] M0 [double] central meridian (deg)
** @param [r] E0 [double] false easting
** @param [r] N0 [double] false northing
** @param [r] a [double] semi-major axis
** @param [r] b [double] semi-minor axis
**
** @return [void]
************************************************************************/
void GPS_Math_Cassini_LatLon_To_EN(double phi, double lambda, double* E,
                                   double* N, double phi0, double M0,
                                   double E0, double N0, double a, double b)
{
  double p2;
  double po2;
  double a2;
  double b2;
  double e2;
  double e4;
  double e6;
  double AM0;
  double c0;
  double c1;
  double c2;
  double c3;
  double om0;
  double A0;
  double A1;
  double A2;
  double A3;
  double j;
  double te4;
  double phi0s2;
  double phi0s4;
  double phi0s6;
  double lat;
  double x;
  double E1;
  double E2;
  double E3;
  double E4;

  double phis;
  double phic;
  double phit;
  double phis2;
  double phis4;
  double phis6;
  double RD;
  double dlam;
  double NN;
  double TT;
  double WW;
  double WW2;
  double WW3;
  double WW4;
  double WW5;
  double CC;
  double MM;


  lambda = GPS_Math_Deg_To_Rad(lambda);
  phi0   = GPS_Math_Deg_To_Rad(phi0);
  phi    = GPS_Math_Deg_To_Rad(phi);
  M0     = GPS_Math_Deg_To_Rad(M0);


  p2 = (double)GPS_PI * (double)2.;
  po2 = (double)GPS_PI / (double)2.;

  a2 = a*a;
  b2 = b*b;
  e2 = (a2-b2)/a2;
  e4 = e2*e2;
  e6 = e2*e4;

  te4 = (double)3.0 * e4;
  j   = (double)45. * e6 / (double)1024.;
  c0 = (double)1.0-e2/(double)4.-te4/(double)64.-(double)5.*e6/(double)256.;
  c1 = (double)3.*e2/(double)8.+te4/(double)32.+j;
  c2 = (double)15.*e4/(double)256.+j;
  c3 = (double)35.*e6/(double)3072.;

  lat = c0*phi0;
  phi0s2 = c1 * sin((double)2.*phi0);
  phi0s4 = c2 * sin((double)4.*phi0);
  phi0s6 = c3 * sin((double)6.*phi0);
  AM0 = a * (lat-phi0s2+phi0s4-phi0s6);

  om0 = (double)1.0 - e2;
  x = pow(om0,(double)0.5);
  E1 = ((double)1.0 - x) / ((double)1.0 + x);
  E2 = E1*E1;
  E3 = E1*E2;
  E4 = E1*E3;
  A0 = (double)3.*E1/(double)2.-(double)27.*E3/(double)32.;
  A1 = (double)21.*E2/(double)16.-(double)55.*E4/(double)32.;
  A2 = (double)151.*E3/(double)96.;
  A3 = (double)1097.*E4/(double)512.;


  dlam = lambda - M0;
  if (dlam>GPS_PI) {
    dlam -= p2;
  }
  if (dlam<-GPS_PI) {
    dlam += p2;
  }

  phis = sin(phi);
  phic = cos(phi);
  phit = tan(phi);
  RD = pow((double)1.-e2*phis*phis,(double).5);
  NN = a/RD;
  TT = phit*phit;
  WW = dlam*phic;
  WW2 = WW*WW;
  WW3 = WW*WW2;
  WW4 = WW*WW3;
  WW5 = WW*WW4;
  CC = e2*phic*phic/om0;
  lat = c0*phi;
  phis2 = c1 * sin((double)2.*phi);
  phis4 = c2 * sin((double)4.*phi);
  phis6 = c3 * sin((double)6.*phi);
  MM = a * (lat-phis2+phis4-phis6);

  *E = NN*(WW-(TT*WW3/(double)6.)-((double)8.-TT+(double)8.*CC)*
           (TT*WW5/(double)120.)) + E0;
  *N = MM-AM0+NN*phit*((WW2/(double)2.)+((double)5.-TT+(double)6.*CC) *
                       WW4/(double)24.) + N0;
  return;
}




/* @func GPS_Math_Cassini_EN_To_LatLon **********************************
**
** Convert Cassini transverse cylindrical easting and northing projection
** to latitude and longitude
**
** @param [r] E [double] easting (metre)
** @param [r] N [double] northing (metre)
** @param [w] phi [double *] latitude (deg)
** @param [w] lambda [double *] longitude (deg)
** @param [r] phi0 [double] latitude of origin (deg)
** @param [r] M0 [double] central meridian (deg)
** @param [r] E0 [double] false easting
** @param [r] N0 [double] false northing
** @param [r] a [double] semi-major axis
** @param [r] b [double] semi-minor axis
**
** @return [void]
************************************************************************/
void GPS_Math_Cassini_EN_To_LatLon(double E, double N, double* phi,
                                   double* lambda, double phi0, double M0,
                                   double E0, double N0, double a, double b)

{
  double p2;
  double po2;
  double a2;
  double b2;
  double e2;
  double e4;
  double e6;
  double AM0;
  double c0;
  double c1;
  double c2;
  double c3;
  double om0;
  double A0;
  double A1;
  double A2;
  double A3;
  double j;
  double te4;
  double phi0s2;
  double phi0s4;
  double phi0s6;
  double lat;
  double x;
  double E1;
  double E2;
  double E3;
  double E4;

  double dx;
  double dy;
  double mu;
  double mus2;
  double mus4;
  double mus6;
  double mus8;
  double M1;
  double phi1;
  double phi1s;
  double phi1c;
  double phi1t;
  double T;
  double T1;
  double N1;
  double R1;
  double RD;
  double DD;
  double D2;
  double D3;
  double D4;
  double D5;
  double tol;

  M0 = GPS_Math_Deg_To_Rad(M0);
  phi0 = GPS_Math_Deg_To_Rad(phi0);

  p2 = (double)GPS_PI * (double)2.;
  po2 = (double)GPS_PI / (double)2.;

  a2 = a*a;
  b2 = b*b;
  e2 = (a2-b2)/a2;
  e4 = e2*e2;
  e6 = e2*e4;

  te4 = (double)3.0 * e4;
  j   = (double)45. * e6 / (double)1024.;
  c0 = (double)1.0-e2/(double)4.-te4/(double)64.-(double)5.*e6/(double)256.;
  c1 = (double)3.*e2/(double)8.+te4/(double)32.+j;
  c2 = (double)15.*e4/(double)256.+j;
  c3 = (double)35.*e6/(double)3072.;

  lat = c0*phi0;
  phi0s2 = c1 * sin((double)2.*phi0);
  phi0s4 = c2 * sin((double)4.*phi0);
  phi0s6 = c3 * sin((double)6.*phi0);
  AM0 = a * (lat-phi0s2+phi0s4-phi0s6);

  om0 = (double)1.0 - e2;
  x = pow(om0,(double)0.5);
  E1 = ((double)1.0 - x) / ((double)1.0 + x);
  E2 = E1*E1;
  E3 = E1*E2;
  E4 = E1*E3;
  A0 = (double)3.*E1/(double)2.-(double)27.*E3/(double)32.;
  A1 = (double)21.*E2/(double)16.-(double)55.*E4/(double)32.;
  A2 = (double)151.*E3/(double)96.;
  A3 = (double)1097.*E4/(double)512.;



  tol = (double)1.e-5;

  dx = E - E0;
  dy = N - N0;
  M1 = AM0 + dy;
  mu = M1 / (a*c0);
  mus2 = A0 * sin((double)2.*mu);
  mus4 = A1 * sin((double)4.*mu);
  mus6 = A2 * sin((double)6.*mu);
  mus8 = A3 * sin((double)8.*mu);
  phi1 = mu + mus2 + mus4 + mus6 + mus8;

  if ((((po2-tol)<phi1)&&(phi1<(po2+tol)))) {
    *phi = po2;
    *lambda = M0;
  } else if ((((-po2-tol)<phi1)&&(phi1<(-po2+tol)))) {
    *phi = -po2;
    *lambda = M0;
  } else {
    phi1s = sin(phi1);
    phi1c = cos(phi1);
    phi1t = tan(phi1);
    T1 = phi1t*phi1t;
    RD = pow((double)1.-e2*phi1s*phi1s,(double).5);
    N1 = a/RD;
    R1 = N1 * om0 / (RD*RD);
    DD = dx/N1;
    D2 = DD*DD;
    D3 = DD*D2;
    D4 = DD*D3;
    D5 = DD*D4;
    T = (double)1. + (double)3.*T1;
    *phi = phi1-(N1*phi1t/R1)*(D2/(double)2.-T*D4/(double)24.);
    *lambda = M0+(DD-T1*D3/(double)3.+T*T1*D5/(double)15.)/phi1c;

    if (*phi>po2) {
      *phi=po2;
    } else if (*phi<-po2) {
      *phi=-po2;
    }

    if (*lambda>GPS_PI) {
      *lambda -= p2;
    }
    if (*lambda<-GPS_PI) {
      *lambda += p2;
    }

    if (*lambda>GPS_PI) {
      *lambda=GPS_PI;
    } else if (*lambda<-GPS_PI) {
      *lambda=-GPS_PI;
    }
  }

  *lambda = GPS_Math_Rad_To_Deg(*lambda);
  *phi    = GPS_Math_Rad_To_Deg(*phi);

  return;
}




/* @func GPS_Math_Cylea_LatLon_To_EN **********************************
**
** Convert latitude and longitude to Cylindrical equal area  projection
**  easting and northing
**
** @param [r] phi [double] latitude (deg)
** @param [r] lambda [double] longitude (deg)
** @param [w] E [double *] easting (metre)
** @param [w] N [double *] northing (metre)
** @param [r] phi0 [double] latitude of origin (deg)
** @param [r] M0 [double] central meridian (deg)
** @param [r] E0 [double] false easting
** @param [r] N0 [double] false northing
** @param [r] a [double] semi-major axis
** @param [r] b [double] semi-minor axis
**
** @return [void]
************************************************************************/
void GPS_Math_Cylea_LatLon_To_EN(double phi, double lambda, double* E,
                                 double* N, double phi0, double M0,
                                 double E0, double N0, double a, double b)
{
  double a2;
  double b2;
  double e;
  double e2;
  double e4;
  double e6;
  double k0;
  double ak0;
  double k2;
  double c0;
  double c1;
  double c2;
  double p2;
  double po2;
  double phi0s;
  double phi0c;

  double dlam;
  double qq;
  double x;
  double phis;

  lambda = GPS_Math_Deg_To_Rad(lambda);
  phi0   = GPS_Math_Deg_To_Rad(phi0);
  phi    = GPS_Math_Deg_To_Rad(phi);
  M0     = GPS_Math_Deg_To_Rad(M0);

  p2 = (double)GPS_PI * (double)2.;
  po2 = (double)GPS_PI / (double)2.;

  if (M0>GPS_PI) {
    M0-=p2;
  }

  a2 = a*a;
  b2 = b*b;
  e2 = (a2-b2)/a2;
  e4 = e2*e2;
  e6 = e2*e4;
  e  = pow(e2,(double).5);
  c0 = e2/(double)3.+(double)31.*e4/(double)180.+(double)517.*
       e6/(double)5040.;
  c1 = (double)23.*e4/(double)360.+(double)251.*e6/(double)3780.;
  c2 = (double)761.*e6/(double)45360.;

  phi0s = sin(phi0);
  phi0c = cos(phi0);
  k0    = phi0c / pow((double)1.-e2*phi0s*phi0s,(double).5);
  ak0   = a*k0;
  k2    = k0 * (double)2.;

  dlam = lambda - M0;
  if (dlam>GPS_PI) {
    dlam-=p2;
  }
  if (dlam<-GPS_PI) {
    dlam+=p2;
  }

  phis = sin(phi);
  x = e * phis;
  qq = ((double)1.-e2)*(phis/((double)1.-x*x)-((double)1./((double)2.*e))*
                        log(((double)1.-x)/((double)1.+x)));
  *E = ak0 * dlam + E0;
  *N = a * qq / k2 + N0;

  return;
}




/* @func GPS_Math_Cylea_EN_To_LatLon **********************************
**
** Convert Cylindrical equal area  easting and northing projection
** to latitude and longitude
**
** @param [r] E [double] easting (metre)
** @param [r] N [double] northing (metre)
** @param [w] phi [double *] latitude (deg)
** @param [w] lambda [double *] longitude (deg)
** @param [r] phi0 [double] latitude of origin (deg)
** @param [r] M0 [double] central meridian (deg)
** @param [r] E0 [double] false easting
** @param [r] N0 [double] false northing
** @param [r] a [double] semi-major axis
** @param [r] b [double] semi-minor axis
**
** @return [void]
************************************************************************/
void GPS_Math_Cylea_EN_To_LatLon(double E, double N, double* phi,
                                 double* lambda, double phi0, double M0,
                                 double E0, double N0, double a, double b)

{
  double a2;
  double b2;
  double e;
  double e2;
  double e4;
  double e6;
  double k0;
  double ak0;
  double k2;
  double c0;
  double c1;
  double c2;
  double p2;
  double po2;
  double phi0s;
  double phi0c;

  double dx;
  double dy;
  double qp;
  double bt;
  double phis;
  double i;
  double x;
  double bs2;
  double bs4;
  double bs6;


  phi0   = GPS_Math_Deg_To_Rad(phi0);
  M0     = GPS_Math_Deg_To_Rad(M0);

  p2 = (double)GPS_PI * (double)2.;
  po2 = (double)GPS_PI / (double)2.;

  if (M0>GPS_PI) {
    M0-=p2;
  }

  a2 = a*a;
  b2 = b*b;
  e2 = (a2-b2)/a2;
  e4 = e2*e2;
  e6 = e2*e4;
  e  = pow(e2,(double).5);
  c0 = e2/(double)3.+(double)31.*e4/(double)180.+(double)517.*
       e6/(double)5040.;
  c1 = (double)23.*e4/(double)360.+(double)251.*e6/(double)3780.;
  c2 = (double)761.*e6/(double)45360.;

  phi0s = sin(phi0);
  phi0c = cos(phi0);
  k0    = phi0c / pow((double)1.-e2*phi0s*phi0s,(double).5);
  ak0   = a*k0;
  k2    = k0 * (double)2.;

  dx = E - E0;
  dy = N - N0;
  phis = sin(po2);
  x = e*phis;
  qp = ((double)1.-e2)*(phis/((double)1.-x*x)-((double)1./((double)2.*e))*
                        log(((double)1.-x)/((double)1.+x)));
  i = k2*dy/(a*qp);
  if (i>(double)1.) {
    i=(double)1.;
  } else if (i<(double)-1.) {
    i=(double)-1.;
  }
  bt = asin(i);
  bs2 = c0 * sin((double)2.*bt);
  bs4 = c1 * sin((double)4.*bt);
  bs6 = c2 * sin((double)6.*bt);

  *phi = bt+bs2+bs4+bs6;
  *lambda = M0 + dx/ak0;

  if (*phi>po2) {
    *phi=po2;
  } else if (*phi<-po2) {
    *phi=-po2;
  }

  if (*lambda>GPS_PI) {
    *lambda -= p2;
  }
  if (*lambda<-GPS_PI) {
    *lambda += p2;
  }

  if (*lambda>GPS_PI) {
    *lambda=GPS_PI;
  } else if (*lambda<-GPS_PI) {
    *lambda=-GPS_PI;
  }

  *lambda = GPS_Math_Rad_To_Deg(*lambda);
  *phi    = GPS_Math_Rad_To_Deg(*phi);

  return;
}




/* @func GPS_Math_EckertIV_LatLon_To_EN **********************************
**
** Convert latitude and longitude to Eckert IV equal area elliptical
** pseudocylindrical projection easting and northing
**
** @param [r] phi [double] latitude (deg)
** @param [r] lambda [double] longitude (deg)
** @param [w] E [double *] easting (metre)
** @param [w] N [double *] northing (metre)
** @param [r] M0 [double] central meridian (deg)
** @param [r] E0 [double] false easting
** @param [r] N0 [double] false northing
** @param [r] a [double] semi-major axis
** @param [r] b [double] semi-minor axis
**
** @return [void]
************************************************************************/
void GPS_Math_EckertIV_LatLon_To_EN(double phi, double lambda, double* E,
                                    double* N, double M0, double E0, double N0,
                                    double a, double b)
{
  double a2;
  double b2;
  double e2;
  double e4;
  double e6;
  double Ra0;
  double Ra1;
  double po2;
  double p2;

  double Ra;

  double phis;
  double theta;
  double dtheta;
  double thetas;
  double thetac;
  double n;
  double dlam;
  double tol;


  lambda = GPS_Math_Deg_To_Rad(lambda);
  phi    = GPS_Math_Deg_To_Rad(phi);
  M0     = GPS_Math_Deg_To_Rad(M0);

  p2 = (double)GPS_PI * (double)2.;
  po2 = (double)GPS_PI / (double)2.;

  if (M0>GPS_PI) {
    M0-=p2;
  }


  a2 = a*a;
  b2 = b*b;
  e2 = (a2-b2) / a2;
  e4 = e2*e2;
  e6 = e2*e4;
  Ra = a*((double)1.-e2/(double)6.-(double)17.*e4/(double)360.-
          (double)67.*e6/(double)3024.);
  Ra0 = Ra * (double)0.4222382;
  Ra1 = Ra * (double)1.3265004;

  theta = phi / (double)2.;
  dtheta = (double)1.;
  tol = (double)4.85e-10;
  phis = sin(phi);

  dlam = lambda - M0;
  if (dlam>GPS_PI) {
    dlam -= p2;
  }
  if (dlam<-GPS_PI) {
    dlam += p2;
  }

  while (fabs(dtheta)>tol) {
    thetas = sin(theta);
    thetac = cos(theta);
    n = theta+thetas*thetac+(double)2.*thetas;
    dtheta = -(n-((double)2.+po2)*phis) /
             ((double)2.*thetac*((double)1.+thetac));
    theta += dtheta;
  }

  *E = Ra0*dlam*((double)1.+cos(theta))+E0;
  *N = Ra1*sin(theta)+N0;

  return;
}




/* @func GPS_Math_EckertIV_EN_To_LatLon **********************************
**
** Convert Eckert IV equal area elliptical pseudocylindrical projection
** easting and northing to latitude and longitude
**
** @param [r] E [double] easting (metre)
** @param [r] N [double] northing (metre)
** @param [w] phi [double *] latitude (deg)
** @param [w] lambda [double *] longitude (deg)
** @param [r] M0 [double] central meridian (deg)
** @param [r] E0 [double] false easting
** @param [r] N0 [double] false northing
** @param [r] a [double] semi-major axis
** @param [r] b [double] semi-minor axis
**
** @return [void]
************************************************************************/
void GPS_Math_EckertIV_EN_To_LatLon(double E, double N, double* phi,
                                    double* lambda, double M0, double E0,
                                    double N0, double a, double b)
{
  double a2;
  double b2;
  double e2;
  double e4;
  double e6;
  double Ra0;
  double Ra1;
  double po2;
  double p2;

  double Ra;
  double theta;
  double thetas;
  double thetac;
  double n;
  double dx;
  double dy;
  double i;


  M0     = GPS_Math_Deg_To_Rad(M0);

  p2 = (double)GPS_PI * (double)2.;
  po2 = (double)GPS_PI / (double)2.;

  if (M0>GPS_PI) {
    M0-=p2;
  }


  a2 = a*a;
  b2 = b*b;
  e2 = (a2-b2) / a2;
  e4 = e2*e2;
  e6 = e2*e4;
  Ra = a*((double)1.-e2/(double)6.-(double)17.*e4/(double)360.-
          (double)67.*e6/(double)3024.);
  Ra0 = Ra * (double)0.4222382;
  Ra1 = Ra * (double)1.3265004;

  dx = E - E0;
  dy = N - N0;
  i = dy/Ra1;
  if (i>(double)1.) {
    i=(double)1.;
  } else if (i<(double)-1.) {
    i=(double)-1.;
  }

  theta = asin(i);
  thetas = sin(theta);
  thetac = cos(theta);
  n = theta+thetas*thetac+(double)2.*thetas;

  *phi = asin(n/((double)2. + po2));
  *lambda = M0 + dx / (Ra0*((double)1.+thetac));

  if (*phi>po2) {
    *phi=po2;
  } else if (*phi<-po2) {
    *phi=-po2;
  }

  if (*lambda>GPS_PI) {
    *lambda -= p2;
  }
  if (*lambda<-GPS_PI) {
    *lambda += p2;
  }

  if (*lambda>GPS_PI) {
    *lambda=GPS_PI;
  } else if (*lambda<-GPS_PI) {
    *lambda=-GPS_PI;
  }

  *lambda = GPS_Math_Rad_To_Deg(*lambda);
  *phi    = GPS_Math_Rad_To_Deg(*phi);

  return;
}





/* @func GPS_Math_EckertVI_LatLon_To_EN **********************************
**
** Convert latitude and longitude to Eckert VI equal area
** pseudocylindrical projection easting and northing
**
** @param [r] phi [double] latitude (deg)
** @param [r] lambda [double] longitude (deg)
** @param [w] E [double *] easting (metre)
** @param [w] N [double *] northing (metre)
** @param [r] M0 [double] central meridian (deg)
** @param [r] E0 [double] false easting
** @param [r] N0 [double] false northing
** @param [r] a [double] semi-major axis
** @param [r] b [double] semi-minor axis
**
** @return [void]
************************************************************************/
void GPS_Math_EckertVI_LatLon_To_EN(double phi, double lambda, double* E,
                                    double* N, double M0, double E0, double N0,
                                    double a, double b)
{
  double a2;
  double b2;
  double e2;
  double e4;
  double e6;
  double Ra;
  double Rsq;
  double IRa;
  double po2;
  double p2;

  double phis;
  double theta;
  double dtheta;
  double dlam;
  double tol;


  lambda = GPS_Math_Deg_To_Rad(lambda);
  phi    = GPS_Math_Deg_To_Rad(phi);
  M0     = GPS_Math_Deg_To_Rad(M0);

  p2 = (double)GPS_PI * (double)2.;
  po2 = (double)GPS_PI / (double)2.;

  if (M0>GPS_PI) {
    M0-=p2;
  }


  a2 = a*a;
  b2 = b*b;
  e2 = (a2-b2) / a2;
  e4 = e2*e2;
  e6 = e2*e4;
  Ra = a*((double)1.-e2/(double)6.-(double)17.*e4/(double)360.-
          (double)67.*e6/(double)3024.);
  Rsq = Ra/pow((double)2.+GPS_PI,(double).5);
  IRa = (double)1./Rsq;

  phis = sin(phi);
  theta = phi;
  dtheta = (double)1.;
  tol = (double)4.85e-10;

  dlam = lambda - M0;
  if (dlam>GPS_PI) {
    dlam -= p2;
  }
  if (dlam<-GPS_PI) {
    dlam += p2;
  }

  while (fabs(dtheta)>tol) {
    dtheta = -(theta+sin(theta)-((double)1.+po2)*phis) /
             ((double)1.+cos(theta));
    theta += dtheta;
  }

  *E = Rsq*dlam*((double)1.+cos(theta))+E0;
  *N = (double)2.*Rsq*theta+N0;

  return;
}




/* @func GPS_Math_EckertVI_EN_To_LatLon **********************************
**
** Convert Eckert VI equal area pseudocylindrical projection
** easting and northing to latitude and longitude
**
** @param [r] E [double] easting (metre)
** @param [r] N [double] northing (metre)
** @param [w] phi [double *] latitude (deg)
** @param [w] lambda [double *] longitude (deg)
** @param [r] M0 [double] central meridian (deg)
** @param [r] E0 [double] false easting
** @param [r] N0 [double] false northing
** @param [r] a [double] semi-major axis
** @param [r] b [double] semi-minor axis
**
** @return [void]
************************************************************************/
void GPS_Math_EckertVI_EN_To_LatLon(double E, double N, double* phi,
                                    double* lambda, double M0, double E0,
                                    double N0, double a, double b)
{
  double a2;
  double b2;
  double e2;
  double e4;
  double e6;
  double Rsq;
  double IRa;
  double po2;
  double p2;

  double Ra;
  double theta;
  double dx;
  double dy;
  double i;


  M0     = GPS_Math_Deg_To_Rad(M0);

  p2 = (double)GPS_PI * (double)2.;
  po2 = (double)GPS_PI / (double)2.;

  if (M0>GPS_PI) {
    M0-=p2;
  }


  a2 = a*a;
  b2 = b*b;
  e2 = (a2-b2) / a2;
  e4 = e2*e2;
  e6 = e2*e4;
  Ra = a*((double)1.-e2/(double)6.-(double)17.*e4/(double)360.-
          (double)67.*e6/(double)3024.);
  Rsq = Ra/pow((double)2.+GPS_PI,(double).5);
  IRa = (double)1./Rsq;


  dx = E - E0;
  dy = N - N0;
  theta = IRa * dy / (double)2.;
  i = (theta+sin(theta)) / ((double)1.+po2);
  if (i>(double)1.) {
    *phi = po2;
  } else if (i<(double)-1.) {
    *phi = -po2;
  } else {
    *phi= asin(i);
  }
  *lambda = M0 + IRa * dx / ((double)1.+cos(theta));

  if (*phi>po2) {
    *phi=po2;
  } else if (*phi<-po2) {
    *phi=-po2;
  }

  if (*lambda>GPS_PI) {
    *lambda -= p2;
  }
  if (*lambda<-GPS_PI) {
    *lambda += p2;
  }

  if (*lambda>GPS_PI) {
    *lambda=GPS_PI;
  } else if (*lambda<-GPS_PI) {
    *lambda=-GPS_PI;
  }

  *lambda = GPS_Math_Rad_To_Deg(*lambda);
  *phi    = GPS_Math_Rad_To_Deg(*phi);

  return;
}





/* @func GPS_Math_Cyled_LatLon_To_EN **********************************
**
** Convert latitude and longitude to cylindrical equidistant projection
**  easting and northing
**
** @param [r] phi [double] latitude (deg)
** @param [r] lambda [double] longitude (deg)
** @param [w] E [double *] easting (metre)
** @param [w] N [double *] northing (metre)
** @param [r] phi0 [double] latitude of origin (deg)
** @param [r] M0 [double] central meridian (deg)
** @param [r] E0 [double] false easting
** @param [r] N0 [double] false northing
** @param [r] a [double] semi-major axis
** @param [r] b [double] semi-minor axis
**
** @return [void]
************************************************************************/
void GPS_Math_Cyled_LatLon_To_EN(double phi, double lambda, double* E,
                                 double* N, double phi0, double M0, double E0,
                                 double N0, double a, double b)
{
  double p2;
  double po2;
  double a2;
  double b2;
  double e2;
  double e4;
  double e6;
  double Ra;
  double Rac;
  double phi0c;

  double dlam;

  lambda = GPS_Math_Deg_To_Rad(lambda);
  phi    = GPS_Math_Deg_To_Rad(phi);
  phi0   = GPS_Math_Deg_To_Rad(phi0);
  M0     = GPS_Math_Deg_To_Rad(M0);

  p2 = (double)GPS_PI * (double)2.0;
  po2 = (double)GPS_PI / (double)2.0;
  if (M0>GPS_PI) {
    M0 -= p2;
  }

  a2 = a*a;
  b2 = b*b;
  e2 = (a2-b2)/a2;
  e4 = e2*e2;
  e6 = e2*e4;

  Ra = a*((double)1.-e2/(double)6.-(double)17.*e4/(double)360.-
          (double)67.*e6/(double)3024.);
  phi0c = cos(phi0);
  Rac   = Ra * phi0c;

  dlam = lambda - M0;
  if (dlam>GPS_PI) {
    dlam -= p2;
  }
  if (dlam<-GPS_PI) {
    dlam += p2;
  }

  *E = Rac * dlam + E0;
  *N = Ra * phi + N0;

  return;
}




/* @func GPS_Math_Cyled_EN_To_LatLon **********************************
**
** Convert cylindrical equidistant easting and northing projection
** to latitude and longitude
**
** @param [r] E [double] easting (metre)
** @param [r] N [double] northing (metre)
** @param [w] phi [double *] latitude (deg)
** @param [w] lambda [double *] longitude (deg)
** @param [r] phi0 [double] latitude of origin (deg)
** @param [r] M0 [double] central meridian (deg)
** @param [r] E0 [double] false easting
** @param [r] N0 [double] false northing
** @param [r] a [double] semi-major axis
** @param [r] b [double] semi-minor axis
**
** @return [void]
************************************************************************/
void GPS_Math_Cyled_EN_To_LatLon(double E, double N, double* phi,
                                 double* lambda, double phi0, double M0,
                                 double E0, double N0, double a, double b)
{
  double p2;
  double po2;
  double a2;
  double b2;
  double e2;
  double e4;
  double e6;
  double Ra;
  double Rac;
  double phi0c;

  double dx;
  double dy;


  phi0   = GPS_Math_Deg_To_Rad(phi0);
  M0     = GPS_Math_Deg_To_Rad(M0);

  p2 = (double)GPS_PI * (double)2.0;
  po2 = (double)GPS_PI / (double)2.0;
  if (M0>GPS_PI) {
    M0 -= p2;
  }

  a2 = a*a;
  b2 = b*b;
  e2 = (a2-b2)/a2;
  e4 = e2*e2;
  e6 = e2*e4;

  Ra = a*((double)1.-e2/(double)6.-(double)17.*e4/(double)360.-
          (double)67.*e6/(double)3024.);
  phi0c = cos(phi0);
  Rac   = Ra * phi0c;

  dx = E - E0;
  dy = N - N0;

  if (!Rac) {
    *lambda = (double)0.;
  } else {
    *lambda = M0 + dx / Rac;
  }

  *phi = dy/Ra;

  if (*phi>po2) {
    *phi = po2;
  } else if (*phi<-po2) {
    *phi = -po2;
  }

  if (*lambda>GPS_PI) {
    *lambda -= p2;
  }
  if (*lambda<-GPS_PI) {
    *lambda += p2;
  }

  if (*lambda>GPS_PI) {
    *lambda = GPS_PI;
  } else if (*lambda<-GPS_PI) {
    *lambda=-GPS_PI;
  }

  *lambda = GPS_Math_Rad_To_Deg(*lambda);
  *phi    = GPS_Math_Rad_To_Deg(*phi);

  return;
}




/* @func GPS_Math_VderGrinten_LatLon_To_EN **********************************
**
** Convert latitude and longitude to Van der Grinten polyconic projection
**  easting and northing
**
** @param [r] phi [double] latitude (deg)
** @param [r] lambda [double] longitude (deg)
** @param [w] E [double *] easting (metre)
** @param [w] N [double *] northing (metre)
** @param [r] M0 [double] central meridian (deg)
** @param [r] E0 [double] false easting
** @param [r] N0 [double] false northing
** @param [r] a [double] semi-major axis
** @param [r] b [double] semi-minor axis
**
** @return [void]
************************************************************************/
void GPS_Math_VderGrinten_LatLon_To_EN(double phi, double lambda, double* E,
                                       double* N, double M0, double E0,
                                       double N0, double a, double b)
{
  double po2;
  double p2;
  double a2;
  double b2;
  double e2;
  double e4;
  double e6;
  double Ra;
  double pRa;

  double gg;
  double pp;
  double pp2;
  double gm0;
  double ppa;
  double thetai;
  double theta;
  double thetas;
  double thetac;
  double qq;
  double tol;
  double aa;
  double aa2;
  double dlam;

  lambda = GPS_Math_Deg_To_Rad(lambda);
  phi    = GPS_Math_Deg_To_Rad(phi);
  M0     = GPS_Math_Deg_To_Rad(M0);

  p2 = (double)GPS_PI * (double)2.0;
  po2 = (double)GPS_PI / (double)2.0;
  if (M0>GPS_PI) {
    M0 -= p2;
  }


  a2 = a*a;
  b2 = b*b;
  e2 = (a2-b2)/a2;
  e4 = e2*e2;
  e6 = e2*e4;

  Ra = a*((double)1.-e2/(double)6.-(double)17.*e4/(double)360.-(double)67.*
          e6/(double)3024.);
  pRa = (double)GPS_PI * Ra;

  dlam = lambda - M0;
  if (dlam>GPS_PI) {
    dlam -= p2;
  }
  if (dlam<-GPS_PI) {
    dlam += p2;
  }

  tol = (double)1.0e-5;

  if (!phi) {
    *N = (double)0.0;
    *E = Ra*dlam+E0;
  } else if (!dlam || (((po2-tol)<tol)&&(phi<(po2+tol))) ||
             (((-po2-tol)<tol)&&(phi<(-po2+tol)))) {
    thetai = fabs(((double)2./(double)GPS_PI) * phi);
    if (thetai>(double)1.) {
      thetai=(double)1.;
    } else if (thetai<(double)-1.) {
      thetai=(double)-1.;
    }

    theta = asin(thetai);
    *E = 0;
    *N = pRa * tan(theta/(double)2.) * N0;
    if (phi<(double)0.0) {
      *N *= (double)-1.;
    }
  } else {
    aa = (double).5*fabs((double)GPS_PI/dlam - dlam/(double)GPS_PI);
    thetai = fabs(((double)2./(double)GPS_PI) * phi);
    if (thetai>(double)1.) {
      thetai=(double)1.;
    } else if (thetai<(double)-1.) {
      thetai=(double)-1.;
    }

    theta = asin(thetai);
    thetas = sin(theta);
    thetac = cos(theta);
    gg = thetac/(thetas+thetac-(double)1.);
    pp = gg*((double)2./thetas-(double)1.);
    aa2 = aa*aa;
    pp2 = pp*pp;
    gm0 = gg-pp2;
    ppa = pp2+aa2;
    qq = aa2+gg;
    *E = pRa*(aa*gm0+pow(aa2*gm0*gm0-ppa*(gg*gg-pp2),(double).5))/ppa+E0;
    if (dlam<(double)0.0) {
      *E *= (double)-1.;
    }
    *N = pRa*(pp*qq-aa*pow((aa2+(double)1.)*ppa-qq*qq,(double).5))/ppa+N0;
    if (phi<(double)0.0) {
      *N *= (double)-1.;
    }
  }

  return;
}




/* @func GPS_Math_VderGrinten_EN_To_LatLon **********************************
**
** Convert Van der Grinten polyconic easting and northing projection
** to latitude and longitude
**
** @param [r] E [double] easting (metre)
** @param [r] N [double] northing (metre)
** @param [w] phi [double *] latitude (deg)
** @param [w] lambda [double *] longitude (deg)
** @param [r] M0 [double] central meridian (deg)
** @param [r] E0 [double] false easting
** @param [r] N0 [double] false northing
** @param [r] a [double] semi-major axis
** @param [r] b [double] semi-minor axis
**
** @return [void]
************************************************************************/
void GPS_Math_VderGrinten_EN_To_LatLon(double E, double N, double* phi,
                                       double* lambda, double M0, double E0,
                                       double N0, double a, double b)
{
  double po2;
  double p2;
  double a2;
  double b2;
  double e2;
  double e4;
  double e6;
  double Ra;
  double pRa;

  double dx;
  double dy;
  double xx;
  double xx2;
  double yy;
  double yy2;
  double tyy2;
  double xpy;
  double c1;
  double c2;
  double c3;
  double c3c3;
  double co;
  double dd;
  double a1;
  double m1;
  double i;
  double theta;

  M0     = GPS_Math_Deg_To_Rad(M0);

  p2 = (double)GPS_PI * (double)2.0;
  po2 = (double)GPS_PI / (double)2.0;
  if (M0>GPS_PI) {
    M0 -= p2;
  }


  a2 = a*a;
  b2 = b*b;
  e2 = (a2-b2)/a2;
  e4 = e2*e2;
  e6 = e2*e4;

  Ra = a*((double)1.-e2/(double)6.-(double)17.*e4/(double)360.-(double)67.*
          e6/(double)3024.);
  pRa = (double)GPS_PI * Ra;


  dx = E - E0;
  dy = N - N0;
  xx = dx/pRa;
  yy = dy/pRa;
  xx2 = xx*xx;
  yy2 = yy*yy;
  xpy = xx2+yy2;
  tyy2 = yy2*(double)2.;

  if (!N) {
    *phi=(double)0.0;
  } else {
    c1 = -fabs(yy)*((double)1.+xpy);
    c2 = c1-tyy2+xx2;
    c3 = (double)-2.*c1+(double)1.+tyy2+xpy*xpy;
    co = c2/((double)3.*c3);
    c3c3 = c3*c3;
    dd = yy2/c3+(((double)2.*c2*c2*c2)/(c3c3*c3)-((double)9.*c1*c2)/
                 c3c3)/(double)27.;
    a1 = (c1-c2*co)/c3;
    m1 = (double)2.* pow(-((double)1./(double)3.)*a1,(double).5);
    i = (double)3.*dd/(a1*m1);
    if ((i>(double)1.)||(i<(double)-1.)) {
      *phi=po2;
    } else {
      theta = ((double)1./(double)3.)*acos((double)3.*dd/(a1*m1));
      *phi = (double)GPS_PI*(-m1*cos(theta+(double)GPS_PI/(double)3.)-
                             co);
    }
  }

  if (N<(double)0.0) {
    *phi *= (double)-1.0;
  }

  if (!xx) {
    *lambda = M0;
  } else
    *lambda = (double)GPS_PI * (xpy-(double)1.+
                                pow((double)1.+((double)2.*xx2-tyy2)+xpy*xpy,(double).5)) /
              ((double)2.*xx) + M0;
  if (*phi>po2) {
    *phi = po2;
  } else if (*phi<-po2) {
    *phi = -po2;
  }

  if (*lambda>GPS_PI) {
    *lambda -= p2;
  }
  if (*lambda<-GPS_PI) {
    *lambda += p2;
  }

  if (*lambda>GPS_PI) {
    *lambda = GPS_PI;
  } else if (*lambda<-GPS_PI) {
    *lambda=-GPS_PI;
  }

  *lambda = GPS_Math_Rad_To_Deg(*lambda);
  *phi    = GPS_Math_Rad_To_Deg(*phi);

  return;
}




/* @func GPS_Math_Bonne_LatLon_To_EN **********************************
**
** Convert latitude and longitude to Bonne pseudoconic equal area  projection
** easting and northing
**
** @param [r] phi [double] latitude (deg)
** @param [r] lambda [double] longitude (deg)
** @param [w] E [double *] easting (metre)
** @param [w] N [double *] northing (metre)
** @param [r] phi1 [double] latitude of true scale (deg)
** @param [r] lambda1 [double] longitude from pole (deg)
** @param [r] E0 [double] false easting
** @param [r] N0 [double] false northing
** @param [r] a [double] semi-major axis
** @param [r] b [double] semi-minor axis
**
** @return [void]
************************************************************************/
void GPS_Math_PolarSt_LatLon_To_EN(double phi, double lambda, double* E,
                                   double* N, double phi1, double lambda1,
                                   double E0, double N0, double a, double b)
{
  double p2;
  double po2;
  double a2;
  double b2;
  double e2;
  double e4=(double)0.;
  double e;
  double eo2;
  double sh;
  double mc;
  double tc=(double)0.;
  double amc=(double)0.;
  double ta;
  double phi1s;
  double phi1c;
  double es;
  double op;
  double om;
  double pe;
  double polat;
  double polon;

  double dlam;
  double phis;
  double t;
  double rho;


  lambda1 = GPS_Math_Deg_To_Rad(lambda1);
  phi1    = GPS_Math_Deg_To_Rad(phi1);
  lambda  = GPS_Math_Deg_To_Rad(lambda);
  phi     = GPS_Math_Deg_To_Rad(phi);


  p2 = (double)GPS_PI * (double)2.0;
  po2 = (double)GPS_PI / (double)2.0;

  ta = a * (double)2.0;
  if (lambda1>GPS_PI) {
    lambda1 -= p2;
  }
  if (phi1<(double)0.0) {
    sh=(double)1.0;
    polat = -phi1;
    polon = -lambda1;
  } else {
    sh=(double)0.0;
    polat = phi1;
    polon = lambda1;
  }

  a2 = a*a;
  b2 = b*b;
  e2 = (a2-b2)/a2;
  e  = pow(e2,(double).5);
  eo2 = e/(double)2.;

  if (fabs(fabs(polat)-po2)>(double)1.0e-10) {
    phi1s = sin(polat);
    phi1c = cos(polat);
    es    = e*phi1s;
    pe    = pow(((double)1.-es)/((double)1.+es),eo2);
    mc    = phi1c / pow((double)1.-es*es,(double).5);
    amc   = mc * a;
    tc    = tan(((double)GPS_PI/(double)4.)-polat/(double)2.) / pe;
  } else {
    op = (double)1. + e;
    om = (double)1. - e;
    e4 = pow(pow(op,op)*pow(om,om),(double).5);
  }



  if (fabs(fabs(phi)-po2)<(double)1.0e-10) {
    *E = *N = (double)0.0;
  } else {
    if (sh) {
      phi *= (double)-1.0;
      lambda *= (double)-1.0;
    }

    dlam = lambda - polon;
    if (dlam>GPS_PI) {
      dlam -= p2;
    }
    if (dlam<-GPS_PI) {
      dlam += p2;
    }
    phis = sin(phi);
    es   = e * phis;
    pe   = pow(((double)1.-es)/((double)1.+es),eo2);
    t    = tan(((double)GPS_PI/(double)4.)-phi/(double)2.) / pe;

    if (fabs(fabs(polat)-po2)>(double)1.0e-10) {
      rho = amc * t / tc;
    } else {
      rho = ta * t / e4;
    }
    *E = rho * sin(dlam) + E0;

    if (sh) {
      *E *= (double)-1.;
      *N = rho * cos(dlam) + N0;
    } else {
      *N = -rho * cos(dlam) + N0;
    }
  }

  return;
}




/* @func GPS_Math_PolarSt_EN_To_LatLon **********************************
**
** Convert Polar Stereographic easting and northing projection
** to latitude and longitude
**
** @param [r] E [double] easting (metre)
** @param [r] N [double] northing (metre)
** @param [w] phi [double *] latitude (deg)
** @param [w] lambda [double *] longitude (deg)
** @param [r] phi1 [double] latitude of true scale (deg)
** @param [r] lambda1 [double] longitude from pole (deg)
** @param [r] E0 [double] false easting
** @param [r] N0 [double] false northing
** @param [r] a [double] semi-major axis
** @param [r] b [double] semi-minor axis
**
** @return [void]
************************************************************************/
void GPS_Math_PolarSt_EN_To_LatLon(double E, double N, double* phi,
                                   double* lambda, double phi1, double lambda1,
                                   double E0, double N0, double a, double b)
{
  double p2;
  double po2;
  double a2;
  double b2;
  double e2;
  double e4=(double)0.;
  double e;
  double eo2;
  double sh;
  double mc;
  double tc=(double)0.;
  double amc=(double)0.;
  double ta;
  double phi1s;
  double phi1c;
  double es;
  double op;
  double om;
  double pe;
  double polat;
  double polon;

  double dx;
  double dy;
  double t;
  double rho;
  double PHI;
  double PHIS;
  double TPHI;


  lambda1 = GPS_Math_Deg_To_Rad(lambda1);
  phi1    = GPS_Math_Deg_To_Rad(phi1);


  p2 = (double)GPS_PI * (double)2.0;
  po2 = (double)GPS_PI / (double)2.0;

  ta = a * (double)2.0;
  if (lambda1>GPS_PI) {
    lambda1 -= p2;
  }
  if (phi1<(double)0.0) {
    sh=(double)1.0;
    polat = -phi1;
    polon = -lambda1;
  } else {
    sh=(double)0.0;
    polat = phi1;
    polon = lambda1;
  }

  a2 = a*a;
  b2 = b*b;
  e2 = (a2-b2)/a2;
  e  = pow(e2,(double).5);
  eo2 = e/(double)2.;

  if (fabs(fabs(polat)-po2)>(double)1.0e-10) {
    phi1s = sin(polat);
    phi1c = cos(polat);
    es    = e*phi1s;
    pe    = pow(((double)1.-es)/((double)1.+es),eo2);
    mc    = phi1c / pow((double)1.-es*es,(double).5);
    amc   = mc * a;
    tc    = tan(((double)GPS_PI/(double)4.)-polat/(double)2.) / pe;
  } else {
    op = (double)1. + e;
    om = (double)1. - e;
    e4 = pow(pow(op,op)*pow(om,om),(double).5);
  }


  dx = E - E0;
  dy = N - N0;
  if (!dx && !dy) {
    *phi = po2;
    *lambda = polon;
  } else {
    if (sh) {
      dx *= (double)-1.;
      dy *= (double)-1.;
    }
    rho = pow(dx*dx+dy*dy,(double).5);
    if (fabs(fabs(polat)-po2)>(double)1.0e-10) {
      t = rho * tc / amc;
    } else {
      t = rho * e4 / ta;
    }
    TPHI = (double)0.0;
    PHI  = po2 - (double)2.*atan(t);
    while (fabs(PHI-TPHI)>(double)1.0e-10) {
      TPHI=PHI;
      PHIS = sin(PHI);
      es = e * PHIS;
      pe    = pow(((double)1.-es)/((double)1.+es),eo2);
      PHI = po2 - (double)2. * atan(t*pe);
    }
    *phi = PHI;
    *lambda = polon + atan2(dx,-dy);

    if (*phi>po2) {
      *phi = po2;
    } else if (*phi<-po2) {
      *phi = -po2;
    }

    if (*lambda>GPS_PI) {
      *lambda -= p2;
    }
    if (*lambda<-GPS_PI) {
      *lambda += p2;
    }

    if (*lambda>GPS_PI) {
      *lambda = GPS_PI;
    } else if (*lambda<-GPS_PI) {
      *lambda=-GPS_PI;
    }
  }
  if (sh) {
    *phi *= (double)-1.;
    *lambda *= (double)1.;
  }

  *lambda = GPS_Math_Rad_To_Deg(*lambda);
  *phi    = GPS_Math_Rad_To_Deg(*phi);

  return;
}




/* @func GPS_Math_Mollweide_LatLon_To_EN **********************************
**
** Convert latitude and longitude to Mollweide projection easting and
** northing
**
** @param [r] phi [double] latitude (deg)
** @param [r] lambda [double] longitude (deg)
** @param [w] E [double *] easting (metre)
** @param [w] N [double *] northing (metre)
** @param [r] M0 [double] central meridian (deg)
** @param [r] E0 [double] false easting
** @param [r] N0 [double] false northing
** @param [r] a [double] semi-major axis
** @param [r] b [double] semi-minor axis
**
** @return [void]
************************************************************************/
void GPS_Math_Mollweide_LatLon_To_EN(double phi, double lambda, double* E,
                                     double* N, double M0, double E0,
                                     double N0, double a, double b)
{
  double a2;
  double b2;
  double e2;
  double e4;
  double e6;
  double p2;
  double po2;
  double Ra;
  double sRa2;
  double sRa8;

  double ps;
  double dlam;
  double theta;
  double thetap;
  double d;
  double tol;

  phi     = GPS_Math_Deg_To_Rad(phi);
  lambda  = GPS_Math_Deg_To_Rad(lambda);
  M0      = GPS_Math_Deg_To_Rad(M0);

  po2 = (double)GPS_PI / (double)2.0;
  p2  = (double)GPS_PI * (double)2.0;
  a2  = a*a;
  b2  = b*b;
  e2 = (a2-b2)/a2;
  e4 = e2*e2;
  e6 = e4*e2;

  Ra = a*((double)1.0-e2/(double)6.0-(double)17.0*e4/(double)360.0-
          (double)67.0*e6/(double)3024.0);
  sRa2 = pow((double)2.,(double).5) * Ra;
  sRa8 = pow((double)8.,(double).5) * Ra;

  if (M0>GPS_PI) {
    M0 -= p2;
  }

  ps  = sin(phi) * (double)GPS_PI;
  d   = (double)0.1745329;
  tol = (double)4.85e-10;
  thetap = phi;

  dlam = lambda - M0;
  if (dlam>GPS_PI) {
    dlam-=p2;
  }
  if (dlam<-GPS_PI) {
    dlam+=p2;
  }

  while (fabs(d)>tol) {
    d = -(thetap+sin(thetap)-ps)/((double)1.+cos(thetap));
    thetap += d;
  }
  theta = thetap / (double)2.;
  *E = (sRa8/(double)GPS_PI) * dlam * cos(theta) + E0;
  *N = sRa2 * sin(theta) + N0;

  return;
}




/* @func GPS_Math_Mollweide_EN_To_LatLon **********************************
**
** Convert latitude and longitude to Mollweide projection easting and
** northing
**
** @param [r] E [double] easting (metre)
** @param [r] N [double] northing (metre)
** @param [w] phi [double *] latitude (deg)
** @param [w] lambda [double *] longitude (deg)
** @param [r] M0 [double] central meridian (deg)
** @param [r] E0 [double] false easting
** @param [r] N0 [double] false northing
** @param [r] a [double] semi-major axis
** @param [r] b [double] semi-minor axis
**
** @return [void]
************************************************************************/
void GPS_Math_Mollweide_EN_To_LatLon(double E, double N, double* phi,
                                     double* lambda, double M0, double E0,
                                     double N0, double a, double b)
{
  double a2;
  double b2;
  double e2;
  double e4;
  double e6;
  double p2;
  double po2;
  double Ra;
  double sRa2;
  double sRa8;

  double dx;
  double dy;
  double theta=(double)0.;
  double tt;
  double i;

  M0      = GPS_Math_Deg_To_Rad(M0);

  po2 = (double)GPS_PI / (double)2.0;
  p2  = (double)GPS_PI * (double)2.0;
  a2  = a*a;
  b2  = b*b;
  e2 = (a2-b2)/a2;
  e4 = e2*e2;
  e6 = e4*e2;

  Ra = a*((double)1.0-e2/(double)6.0-(double)17.0*e4/(double)360.0-
          (double)67.0*e6/(double)3024.0);
  sRa2 = pow((double)2.,(double).5) * Ra;
  sRa8 = pow((double)8.,(double).5) * Ra;

  if (M0>GPS_PI) {
    M0 -= p2;
  }

  dx = E - E0;
  dy = N - N0;
  i  = dy/sRa2;
  if (fabs(i)>(double)1.) {
    *phi = po2;
    if (N<(double)0.0) {
      *phi *= (double)-1.;
    }
  } else {
    theta = asin(i);
    tt = theta * (double)2.;
    *phi = asin((tt+sin(tt))/(double)GPS_PI);
    if (*phi>po2) {
      *phi=po2;
    } else if (*phi<-po2) {
      *phi=-po2;
    }
  }

  if (fabs(fabs(*phi)-po2)<(double)1.0e-10) {
    *lambda = M0;
  } else {
    *lambda = M0 + (double)GPS_PI * dx / (sRa8 * cos(theta));
  }


  if (*lambda>GPS_PI) {
    *lambda-=p2;
  }
  if (*lambda<-GPS_PI) {
    *lambda+=p2;
  }

  if (*lambda>GPS_PI) {
    *lambda=GPS_PI;
  } else if (*lambda<-GPS_PI) {
    *lambda=-GPS_PI;
  }

  *lambda = GPS_Math_Rad_To_Deg(*lambda);
  *phi    = GPS_Math_Rad_To_Deg(*phi);

  return;
}




/* @func GPS_Math_Orthog_LatLon_To_EN **********************************
**
** Convert latitude and longitude to orthographic projection
**  easting and northing
**
** @param [r] phi [double] latitude (deg)
** @param [r] lambda [double] longitude (deg)
** @param [w] E [double *] easting (metre)
** @param [w] N [double *] northing (metre)
** @param [r] phi0 [double] latitude of origin (deg)
** @param [r] lambda0 [double] longitude of origin (deg)
** @param [r] E0 [double] false easting
** @param [r] N0 [double] false northing
** @param [r] a [double] semi-major axis
** @param [r] b [double] semi-minor axis
**
** @return [void]
************************************************************************/
void GPS_Math_Orthog_LatLon_To_EN(double phi, double lambda, double* E,
                                  double* N, double phi0, double lambda0,
                                  double E0, double N0, double a, double b)
{
  double p2;
  double po2;
  double a2;
  double b2;
  double e2;
  double e4;
  double e6;
  double Ra;
  double phi0s;
  double phi0c;

  double phis;
  double phic;
  double dlam;
  double clc;
  double cc;


  lambda  = GPS_Math_Deg_To_Rad(lambda);
  phi     = GPS_Math_Deg_To_Rad(phi);
  phi0    = GPS_Math_Deg_To_Rad(phi0);
  lambda0 = GPS_Math_Deg_To_Rad(lambda0);

  p2 = (double)GPS_PI * (double)2.0;
  po2 = (double)GPS_PI / (double)2.0;
  if (lambda0>GPS_PI) {
    lambda0 -= p2;
  }
  a2 = a*a;
  b2 = b*b;
  e2 = (a2-b2)/a2;
  e4 = e2*e2;
  e6 = e2*e4;
  Ra = a * ((double)1.-e2/(double)6.-(double)17.*e4/(double)360.-(double)67.*
            e6/(double)3024.);
  phi0s = sin(phi0);
  phi0c = cos(phi0);

  dlam = lambda - lambda0;
  if (dlam>GPS_PI) {
    dlam -= p2;
  }
  if (dlam<-GPS_PI) {
    dlam += p2;
  }


  phis = sin(phi);
  phic = cos(phi);
  clc = phic * cos(dlam);
  cc  = phi0s * phis + phi0c * clc;

  *E = Ra * phic * sin(dlam) + E0;
  *N = Ra * (phi0c * phis - phi0s * clc) + N0;

  return;
}




/* @func GPS_Math_Orthog_EN_To_LatLon **********************************
**
** Convert Orthogonal easting and northing projection
** to latitude and longitude
**
** @param [r] E [double] easting (metre)
** @param [r] N [double] northing (metre)
** @param [w] phi [double *] latitude (deg)
** @param [w] lambda [double *] longitude (deg)
** @param [r] phi0 [double] latitude of origin (deg)
** @param [r] lambda0 [double] longitude of origin (deg)
** @param [r] E0 [double] false easting
** @param [r] N0 [double] false northing
** @param [r] a [double] semi-major axis
** @param [r] b [double] semi-minor axis
**
** @return [void]
************************************************************************/
void GPS_Math_Orthog_EN_To_LatLon(double E, double N, double* phi,
                                  double* lambda, double phi0, double lambda0,
                                  double E0, double N0, double a, double b)
{
  double p2;
  double po2;
  double a2;
  double b2;
  double e2;
  double e4;
  double e6;
  double Ra;
  double phi0s;
  double phi0c;

  double dx;
  double dy;
  double rho;
  double adod;
  double ror;
  double cc;
  double ccs;
  double ccc;




  phi0    = GPS_Math_Deg_To_Rad(phi0);
  lambda0 = GPS_Math_Deg_To_Rad(lambda0);

  p2 = (double)GPS_PI * (double)2.0;
  po2 = (double)GPS_PI / (double)2.0;
  if (lambda0>GPS_PI) {
    lambda0 -= p2;
  }
  a2 = a*a;
  b2 = b*b;
  e2 = (a2-b2)/a2;
  e4 = e2*e2;
  e6 = e2*e4;
  Ra = a * ((double)1.-e2/(double)6.-(double)17.*e4/(double)360.-(double)67.*
            e6/(double)3024.);
  phi0s = sin(phi0);
  phi0c = cos(phi0);


  dx = E - E0;
  dy = N - N0;
  adod = atan(dx/dy);
  rho = pow(dx*dx+dy*dy,(double).5);
  if (!rho) {
    *phi = phi0;
    *lambda = lambda0;
  } else {
    ror = rho/Ra;
    if (ror>(double)1.) {
      ror=(double)1.;
    } else if (ror<(double)-1.) {
      ror=(double)-1.;
    }
    cc = asin(ror);
    ccs = sin(cc);
    ccc = cos(cc);
    *phi = asin(ccc*phi0s+(dy*ccs*phi0c/rho));
    if (phi0==po2) {
      *lambda = lambda0 - adod;
    } else if (phi0==-po2) {
      *lambda = lambda0 + adod;
    } else {
      *lambda = lambda0+atan(dx*ccs/(rho*phi0c*ccc-dy*phi0s*ccs));
    }
  }

  if (*phi>po2) {
    *phi = po2;
  } else if (*phi<-po2) {
    *phi = -po2;
  }

  if (*lambda>GPS_PI) {
    *lambda -= p2;
  }
  if (*lambda<-GPS_PI) {
    *lambda += p2;
  }

  if (*lambda>GPS_PI) {
    *lambda = GPS_PI;
  } else if (*lambda<-GPS_PI) {
    *lambda=-GPS_PI;
  }

  *lambda = GPS_Math_Rad_To_Deg(*lambda);
  *phi    = GPS_Math_Rad_To_Deg(*phi);

  return;
}




/* @func GPS_Math_Polycon_LatLon_To_EN **********************************
**
** Convert latitude and longitude to Polyconic  projection
**  easting and northing
**
** @param [r] phi [double] latitude (deg)
** @param [r] lambda [double] longitude (deg)
** @param [w] E [double *] easting (metre)
** @param [w] N [double *] northing (metre)
** @param [r] phi0 [double] latitude of origin (deg)
** @param [r] M0 [double] central meridian (deg)
** @param [r] E0 [double] false easting
** @param [r] N0 [double] false northing
** @param [r] a [double] semi-major axis
** @param [r] b [double] semi-minor axis
**
** @return [void]
************************************************************************/
void GPS_Math_Polycon_LatLon_To_EN(double phi, double lambda, double* E,
                                   double* N, double phi0, double M0,
                                   double E0, double N0, double a, double b)
{
  double p2;
  double po2;
  double a2;
  double b2;
  double e2;
  double e4;
  double e6;
  double AM0;
  double c0;
  double c1;
  double c2;
  double c3;
  double j;
  double te4;
  double phi0s2;
  double phi0s4;
  double phi0s6;

  double phis;
  double phis2;
  double phis4;
  double phis6;
  double dlam;
  double NN;
  double NNot;
  double MM;
  double EE;
  double lat;


  lambda = GPS_Math_Deg_To_Rad(lambda);
  phi    = GPS_Math_Deg_To_Rad(phi);
  phi0   = GPS_Math_Deg_To_Rad(phi0);
  M0     = GPS_Math_Deg_To_Rad(M0);

  p2 = (double)GPS_PI * (double)2.0;
  po2 = (double)GPS_PI / (double)2.0;
  if (M0>GPS_PI) {
    M0 -= p2;
  }
  a2 = a*a;
  b2 = b*b;
  e2 = (a2-b2)/a2;
  e4 = e2*e2;
  e6 = e2*e4;

  j = (double)45.0*e6/(double)1024.0;
  te4 = (double)3.0 * e4;
  c0 = (double)1.0-e2/(double)4.0-te4/(double)64.0-(double)5.0*e6/
       (double)256.0;
  c1 = (double)3.0*e2/(double)8.0+te4/(double)32.0+j;
  c2 = (double)15.0*e4/(double)256.0+j;
  c3 = (double)35.0*e6/(double)3072.0;

  lat = c0 * phi0;

  phi0s2 = c1 * sin((double)2.0*phi0);
  phi0s4 = c2 * sin((double)4.0*phi0);
  phi0s6 = c3 * sin((double)6.0*phi0);
  AM0    = a*(lat-phi0s2+phi0s4-phi0s6);



  dlam = lambda - M0;
  if (dlam>GPS_PI) {
    dlam -= p2;
  }
  if (dlam<-GPS_PI) {
    dlam += p2;
  }

  phis = sin(phi);

  if (!phi) {
    *E = a * dlam + E0;
    *N = -AM0 + N0;
  } else {
    NN = a / pow((double)1.-e2*phis*phis,(double).5);
    NNot = NN / tan(phi);
    lat = c0 * phi;
    phis2 = c1 * sin((double)2.0*phi);
    phis4 = c2 * sin((double)4.0*phi);
    phis6 = c3 * sin((double)6.0*phi);
    MM    = a*(lat-phis2+phis4-phis6);
    EE    = dlam *phis;
    *E = NNot * sin(EE) + E0;
    *N = MM - AM0 + NNot * ((double)1.-cos(EE)) + N0;
  }

  return;
}




/* @func GPS_Math_Polycon_EN_To_LatLon **********************************
**
** Convert Polyconic easting and northing projection
** to latitude and longitude
**
** @param [r] E [double] easting (metre)
** @param [r] N [double] northing (metre)
** @param [w] phi [double *] latitude (deg)
** @param [w] lambda [double *] longitude (deg)
** @param [r] phi0 [double] latitude of origin (deg)
** @param [r] M0 [double] central meridian (deg)
** @param [r] E0 [double] false easting
** @param [r] N0 [double] false northing
** @param [r] a [double] semi-major axis
** @param [r] b [double] semi-minor axis
**
** @return [void]
************************************************************************/
void GPS_Math_Polycon_EN_To_LatLon(double E, double N, double* phi,
                                   double* lambda, double phi0, double M0,
                                   double E0, double N0, double a, double b)
{
  double p2;
  double po2;
  double a2;
  double b2;
  double e2;
  double e4;
  double e6;
  double AM0;
  double c0;
  double c1;
  double c2;
  double c3;
  double j;
  double te4;
  double phi0s2;
  double phi0s4;
  double phi0s6;

  double dx;
  double dy;
  double dxoa;
  double AA;
  double BB;
  double CC=(double)0.;
  double PHIn;
  double PHId;
  double PHIs;
  double PHI;
  double PHIs2;
  double PHIs4;
  double PHIs6;
  double Mn;
  double Mnp;
  double Ma;
  double AAMa;
  double mpb;
  double AAmin;
  double tol;
  double lat;


  phi0   = GPS_Math_Deg_To_Rad(phi0);
  M0     = GPS_Math_Deg_To_Rad(M0);

  p2 = (double)GPS_PI * (double)2.0;
  po2 = (double)GPS_PI / (double)2.0;
  if (M0>GPS_PI) {
    M0 -= p2;
  }
  a2 = a*a;
  b2 = b*b;
  e2 = (a2-b2)/a2;
  e4 = e2*e2;
  e6 = e2*e4;

  j = (double)45.0*e6/(double)1024.0;
  te4 = (double)3.0 * e4;
  c0 = (double)1.0-e2/(double)4.0-te4/(double)64.0-(double)5.0*e6/
       (double)256.0;
  c1 = (double)3.0*e2/(double)8.0+te4/(double)32.0+j;
  c2 = (double)15.0*e4/(double)256.0+j;
  c3 = (double)35.0*e6/(double)3072.0;

  lat = c0 * phi0;

  phi0s2 = c1 * sin((double)2.0*phi0);
  phi0s4 = c2 * sin((double)4.0*phi0);
  phi0s6 = c3 * sin((double)6.0*phi0);
  AM0    = a*(lat-phi0s2+phi0s4-phi0s6);

  tol = (double)1.0e-12;

  dx = E - E0;
  dy = N - N0;
  dxoa = dx/a;
  if ((((-AM0-(double)1.)<dy)&&(dy<(-AM0+(double)1.)))) {
    *phi = (double)0.;
    *lambda = dxoa + M0;
  } else {
    AA = (AM0+dy) / a;
    BB = dxoa * dxoa + (AA*AA);
    PHIn = AA;
    PHId = (double)1.;

    while (fabs(PHId)>tol) {
      PHIs = sin(PHIn);
      CC = pow((double)1.-e2*PHIs*PHIs,(double).5) * tan(PHIn);
      PHI = PHIn * c0;
      PHIs2 = c1 * sin((double)2.0*PHIn);
      PHIs4 = c2 * sin((double)4.0*PHIn);
      PHIs6 = c3 * sin((double)6.0*PHIn);
      Mn    = a*(PHI-PHIs2+PHIs4-PHIs6);
      Mnp = c0 - (double)2.*c1*cos((double)2.*PHIn)+(double)4.*c2*
            cos((double)4.*PHIn)-(double)6.*c3*cos((double)6.*PHIn);
      Ma = Mn / a;
      AAMa = AA * Ma;
      mpb = Ma*Ma+BB;
      AAmin = AA - Ma;
      PHId = (AAMa*CC+AAmin-(double).5*mpb*CC)/
             (e2*PHIs2*(mpb-(double)2.*AAMa) /
              (double)4.*CC+AAmin*(CC*Mnp-(double)2./PHIs2)-Mnp);
      PHIn -= PHId;
    }
    *phi = PHIn;

    if (*phi>po2) {
      *phi = po2;
    } else if (*phi<-po2) {
      *phi = -po2;
    }

    if ((((po2-(double).00001)<fabs(*phi))&&
         (fabs(*phi)<(po2+(double).00001)))) {
      *lambda = M0;
    } else {
      *lambda = (asin(dxoa*CC)) / sin(*phi) + M0;
    }
  }

  if (*lambda>GPS_PI) {
    *lambda -= p2;
  }
  if (*lambda<-GPS_PI) {
    *lambda += p2;
  }

  if (*lambda>GPS_PI) {
    *lambda = GPS_PI;
  } else if (*lambda<-GPS_PI) {
    *lambda=-GPS_PI;
  }

  *lambda = GPS_Math_Rad_To_Deg(*lambda);
  *phi    = GPS_Math_Rad_To_Deg(*phi);

  return;
}




/* @func GPS_Math_Sinusoid_LatLon_To_EN **********************************
**
** Convert latitude and longitude to Sinusoidal projection easting and
** northing
**
** @param [r] phi [double] latitude (deg)
** @param [r] lambda [double] longitude (deg)
** @param [w] E [double *] easting (metre)
** @param [w] N [double *] northing (metre)
** @param [r] M0 [double] central meridian (deg)
** @param [r] E0 [double] false easting
** @param [r] N0 [double] false northing
** @param [r] a [double] semi-major axis
** @param [r] b [double] semi-minor axis
**
** @return [void]
************************************************************************/
void GPS_Math_Sinusoid_LatLon_To_EN(double phi, double lambda, double* E,
                                    double* N, double M0, double E0,
                                    double N0, double a, double b)
{
  double a2;
  double b2;
  double e2;
  double e4;
  double e6;
  double p2;
  double po2;
  double c0;
  double c1;
  double c2;
  double c3;
  double A1;
  double A0;
  double A2;
  double A3;
  double E1;
  double E2;
  double E3;
  double E4;
  double j;
  double om0;
  double som0;

  double phis;
  double phis2;
  double phis4;
  double phis6;
  double mm;
  double MM;
  double dlam;


  phi     = GPS_Math_Deg_To_Rad(phi);
  lambda  = GPS_Math_Deg_To_Rad(lambda);
  M0      = GPS_Math_Deg_To_Rad(M0);

  po2 = (double)GPS_PI / (double)2.0;
  p2  = (double)GPS_PI * (double)2.0;
  a2  = a*a;
  b2  = b*b;
  e2 = (a2-b2)/a2;
  e4 = e2*e2;
  e6 = e4*e2;

  j = (double)45.*e6/(double)1024.;
  c0 = (double)1.-e2/(double)4.-(double)3.*e4/(double)64.-(double)5.*
       e6/(double)256.;
  c1 = (double)3.*e2/(double)8.+(double)3.*e4/(double)32.+j;
  c2 = (double)15.*e4/(double)256.+j;
  c3 = (double)35.*e6/(double)3072.;
  om0 = (double)1. - e2;
  som0 = pow(om0,(double).5);
  E1 = ((double)1.-som0)/((double)1.+som0);
  E2 = E1*E1;
  E3 = E1*E2;
  E4 = E1*E3;
  A0 = (double)3.*E1/(double)2.-(double)27.*E3/(double)32.;
  A1 = (double)21.*E2/(double)16.-(double)55.*E4/(double)32.;
  A2 = (double)151.*E3/(double)96.;
  A3 = (double)1097.*E4/(double)512.;

  if (M0>GPS_PI) {
    M0 -= p2;
  }

  phis = sin(phi);

  dlam = lambda - M0;
  if (dlam>GPS_PI) {
    dlam-=p2;
  }
  if (dlam<-GPS_PI) {
    dlam+=p2;
  }

  mm = pow((double)1.-e2*phis*phis,(double).5);
  phis2 = c1 * sin((double)2.*phi);
  phis4 = c2 * sin((double)4.*phi);
  phis6 = c3 * sin((double)6.*phi);
  MM = a * (c0*phi-phis2+phis4-phis6);



  *E = a*dlam*cos(phi)/mm+E0;
  *N = MM + N0;

  return;
}




/* @func GPS_Math_Sinusoid_EN_To_LatLon **********************************
**
** Convert latitude and longitude to Sinusoidal projection easting and
** northing
**
** @param [r] E [double] easting (metre)
** @param [r] N [double] northing (metre)
** @param [w] phi [double *] latitude (deg)
** @param [w] lambda [double *] longitude (deg)
** @param [r] M0 [double] central meridian (deg)
** @param [r] E0 [double] false easting
** @param [r] N0 [double] false northing
** @param [r] a [double] semi-major axis
** @param [r] b [double] semi-minor axis
**
** @return [void]
************************************************************************/
void GPS_Math_Sinusoid_EN_To_LatLon(double E, double N, double* phi,
                                    double* lambda, double M0, double E0,
                                    double N0, double a, double b)
{
  double a2;
  double b2;
  double e2;
  double e4;
  double e6;
  double p2;
  double po2;
  double c0;
  double c1;
  double c2;
  double c3;
  double A1;
  double A0;
  double A2;
  double A3;
  double E1;
  double E2;
  double E3;
  double E4;
  double j;
  double om0;
  double som0;

  double dx;
  double dy;
  double mu;
  double mu2s;
  double mu4s;
  double mu6s;
  double mu8s;
  double phis;


  M0      = GPS_Math_Deg_To_Rad(M0);

  po2 = (double)GPS_PI / (double)2.0;
  p2  = (double)GPS_PI * (double)2.0;
  a2  = a*a;
  b2  = b*b;
  e2 = (a2-b2)/a2;
  e4 = e2*e2;
  e6 = e4*e2;

  j = (double)45.*e6/(double)1024.;
  c0 = (double)1.-e2/(double)4.-(double)3.*e4/(double)64.-(double)5.*
       e6/(double)256.;
  c1 = (double)3.*e2/(double)8.+(double)3.*e4/(double)32.+j;
  c2 = (double)15.*e4/(double)256.+j;
  c3 = (double)35.*e6/(double)3072.;
  om0 = (double)1. - e2;
  som0 = pow(om0,(double).5);
  E1 = ((double)1.-som0)/((double)1.+som0);
  E2 = E1*E1;
  E3 = E1*E2;
  E4 = E1*E3;
  A0 = (double)3.*E1/(double)2.-(double)27.*E3/(double)32.;
  A1 = (double)21.*E2/(double)16.-(double)55.*E4/(double)32.;
  A2 = (double)151.*E3/(double)96.;
  A3 = (double)1097.*E4/(double)512.;


  dx = E - E0;
  dy = N - N0;

  mu = dy/(c0*a);
  mu2s = A0 * sin((double)2.*mu);
  mu4s = A1 * sin((double)4.*mu);
  mu6s = A2 * sin((double)6.*mu);
  mu8s = A3 * sin((double)8.*mu);
  *phi = mu + mu2s + mu4s + mu6s + mu8s;

  if (*phi>po2) {
    *phi=po2;
  } else if (*phi<-po2) {
    *phi=-po2;
  }

  if ((((po2-(double)1.0e-8)<fabs(*phi))&&(fabs(*phi)<(po2+(double)1.0e-8)))) {
    *lambda = M0;
  } else {
    phis = sin(*phi);
    *lambda = M0 + dx*pow((double)1.0-e2*phis*phis,(double).5) /
              (a*cos(*phi));
  }

  if (*lambda>GPS_PI) {
    *lambda-=p2;
  }
  if (*lambda<-GPS_PI) {
    *lambda+=p2;
  }

  if (*lambda>GPS_PI) {
    *lambda=GPS_PI;
  } else if (*lambda<-GPS_PI) {
    *lambda=-GPS_PI;
  }

  *lambda = GPS_Math_Rad_To_Deg(*lambda);
  *phi    = GPS_Math_Rad_To_Deg(*phi);

  return;
}




/* @func GPS_Math_TCylEA_LatLon_To_EN **********************************
**
** Convert latitude and longitude to transverse cylindrical  equal area
** projection easting and northing
**
** @param [r] phi [double] latitude (deg)
** @param [r] lambda [double] longitude (deg)
** @param [w] E [double *] easting (metre)
** @param [w] N [double *] northing (metre)
** @param [r] phi0 [double] latitude of origin (deg)
** @param [r] M0 [double] central meridian (deg)
** @param [r] E0 [double] false easting
** @param [r] N0 [double] false northing
** @param [r] a [double] semi-major axis
** @param [r] b [double] semi-minor axis
**
** @return [void]
************************************************************************/
void GPS_Math_TCylEA_LatLon_To_EN(double phi, double lambda, double* E,
                                  double* N, double phi0, double M0, double E0,
                                  double N0, double a, double b)
{
  double p2;
  double po2;
  double a2;
  double b2;
  double e;
  double e2;
  double e4;
  double e6;
  double AM0;
  double qp;
  double om;
  double oo;
  double c0;
  double c1;
  double c2;
  double c3;
  double b0;
  double b1;
  double B2;
  double b3;
  double A0;
  double A1;
  double A2;
  double sf;
  double x;
  double som;
  double phis;
  double j;
  double te4;
  double lat;
  double phi0s2;
  double phi0s4;
  double phi0s6;
  double E1;
  double E2;
  double E3;
  double E4;

  double dlam;
  double qq;
  double qqo;
  double bt;
  double btc;
  double PHI;
  double PHIs2;
  double PHIs4;
  double PHIs6;
  double bts2;
  double bts4;
  double bts6;
  double PHIc;
  double PHIcs;
  double Mc;



  sf = (double)1.0; /* scale factor */

  lambda = GPS_Math_Deg_To_Rad(lambda);
  phi    = GPS_Math_Deg_To_Rad(phi);
  phi0   = GPS_Math_Deg_To_Rad(phi0);
  M0     = GPS_Math_Deg_To_Rad(M0);

  p2 = (double)GPS_PI * (double)2.0;
  po2 = (double)GPS_PI / (double)2.0;
  if (M0>GPS_PI) {
    M0 -= p2;
  }
  a2 = a*a;
  b2 = b*b;
  e2 = (a2-b2)/a2;
  e4 = e2*e2;
  e6 = e2*e4;
  e  = pow(e2,(double).5);
  om = (double)1.-e2;
  som  = pow(om,(double).5);
  oo = (double)1./((double)2.*e);

  phis = sin(po2);
  x  = e * phis;
  qp = om*(phis/((double)1.-e2*phis*phis)-oo*
           log(((double)1.-x)/((double)1.+x)));

  A0 = e2 / (double)3.+(double)31.*e4/(double)180.+(double)517.*
       e6/(double)5040.;
  A1 = (double)23.*e4/(double)360.+(double)251.*e6/(double)3780.;
  A2 = (double)761.*e6/(double)45360.;

  E1 = ((double)1.0-som) / ((double)1.0+som);
  E2 = E1*E1;
  E3 = E2*E1;
  E4 = E3*E1;

  b0 = (double)3.*E1/(double)2.-(double)27.*E3/(double)32.;
  b1 = (double)21.*E2/(double)16.-(double)55.*E4/(double)32.;
  B2 = (double)151.*E3/(double)96.;
  b3 = (double)1097.*E4/(double)512.;


  j = (double)45.0*e6/(double)1024.0;
  te4 = (double)3.0 * e4;
  c0 = (double)1.0-e2/(double)4.0-te4/(double)64.0-(double)5.0*e6/
       (double)256.0;
  c1 = (double)3.0*e2/(double)8.0+te4/(double)32.0+j;
  c2 = (double)15.0*e4/(double)256.0+j;
  c3 = (double)35.0*e6/(double)3072.0;

  lat = c0 * phi0;

  phi0s2 = c1 * sin((double)2.0*phi0);
  phi0s4 = c2 * sin((double)4.0*phi0);
  phi0s6 = c3 * sin((double)6.0*phi0);
  AM0 = a*(lat-phi0s2+phi0s4-phi0s6);


  dlam = lambda - M0;
  if (dlam>GPS_PI) {
    dlam -= p2;
  }
  if (dlam<-GPS_PI) {
    dlam += p2;
  }

  phis = sin(phi);

  if (phi==po2) {
    qq = qp;
    qqo = (double)1.;
  } else {
    x = e * phis;
    qq = om*(phis/((double)1.-e2*phis*phis)-oo*
             log(((double)1.-x)/((double)1.+x)));
    qqo = qq/qp;
  }

  if (qqo>(double)1.) {
    qqo = (double)1.;
  } else if (qqo<(double)-1.) {
    qqo = (double)-1.;
  }

  bt = asin(qqo);
  btc = atan(tan(bt)/cos(dlam));

  if ((fabs(btc)-po2)>(double)1.0e-8) {
    PHIc = btc;
  } else {
    bts2 = A0 * sin((double)2.0*btc);
    bts4 = A1 * sin((double)4.0*btc);
    bts6 = A2 * sin((double)6.0*btc);
    PHIc = btc + bts2 + bts4 + bts6;
  }

  PHIcs = sin(PHIc);
  *E = a*cos(bt)*cos(PHIc)*sin(dlam)/(sf*cos(btc)*
                                      pow((double)1.-e2*PHIcs*PHIcs,
                                          (double).5)) + E0;
  PHI = c0 * PHIc;
  PHIs2 = c1 * sin((double)2.0*PHIc);
  PHIs4 = c2 * sin((double)4.0*PHIc);
  PHIs6 = c3 * sin((double)6.0*PHIc);
  Mc = a*(PHI-PHIs2+PHIs4-PHIs6);

  *N = sf * (Mc-AM0) + N0;

  return;
}




/* @func GPS_Math_TCylEA_EN_To_LatLon **********************************
**
** Convert transverse cylindrical equal area easting and northing projection
** to latitude and longitude
**
** @param [r] E [double] easting (metre)
** @param [r] N [double] northing (metre)
** @param [w] phi [double *] latitude (deg)
** @param [w] lambda [double *] longitude (deg)
** @param [r] phi0 [double] latitude of origin (deg)
** @param [r] M0 [double] central meridian (deg)
** @param [r] E0 [double] false easting
** @param [r] N0 [double] false northing
** @param [r] a [double] semi-major axis
** @param [r] b [double] semi-minor axis
**
** @return [void]
************************************************************************/
void GPS_Math_TCylEA_EN_To_LatLon(double E, double N, double* phi,
                                  double* lambda, double phi0, double M0,
                                  double E0, double N0, double a, double b)
{
  double p2;
  double po2;
  double a2;
  double b2;
  double e;
  double e2;
  double e4;
  double e6;
  double AM0;
  double qp;
  double om;
  double oo;
  double c0;
  double c1;
  double c2;
  double c3;
  double b0;
  double b1;
  double B2;
  double b3;
  double A0;
  double A1;
  double A2;
  double sf;
  double x;
  double som;
  double phis;
  double j;
  double te4;
  double lat;
  double phi0s2;
  double phi0s4;
  double phi0s6;
  double E1;
  double E2;
  double E3;
  double E4;

  double dx;
  double dy;
  double bt;
  double btc;
  double btp;
  double btcc;
  double Mc;
  double Muc;
  double mus2;
  double mus4;
  double mus6;
  double mus8;
  double bts2;
  double bts4;
  double bts6;
  double PHIc;
  double Qc;
  double Qco;
  double t;


  sf = (double)1.0; /* scale factor */

  phi0   = GPS_Math_Deg_To_Rad(phi0);
  M0     = GPS_Math_Deg_To_Rad(M0);

  p2 = (double)GPS_PI * (double)2.0;
  po2 = (double)GPS_PI / (double)2.0;
  if (M0>GPS_PI) {
    M0 -= p2;
  }
  a2 = a*a;
  b2 = b*b;
  e2 = (a2-b2)/a2;
  e4 = e2*e2;
  e6 = e2*e4;
  e  = pow(e2,(double).5);
  om = (double)1.-e2;
  som  = pow(om,(double).5);
  oo = (double)1./((double)2.*e);

  phis = sin(po2);
  x  = e * phis;
  qp = om*(phis/((double)1.-e2*phis*phis)-oo*
           log(((double)1.-x)/((double)1.+x)));

  A0 = e2 / (double)3.+(double)31.*e4/(double)180.+(double)517.*
       e6/(double)5040.;
  A1 = (double)23.*e4/(double)360.+(double)251.*e6/(double)3780.;
  A2 = (double)761.*e6/(double)45360.;

  E1 = ((double)1.0-som) / ((double)1.0+som);
  E2 = E1*E1;
  E3 = E2*E1;
  E4 = E3*E1;

  b0 = (double)3.*E1/(double)2.-(double)27.*E3/(double)32.;
  b1 = (double)21.*E2/(double)16.-(double)55.*E4/(double)32.;
  B2 = (double)151.*E3/(double)96.;
  b3 = (double)1097.*E4/(double)512.;


  j = (double)45.0*e6/(double)1024.0;
  te4 = (double)3.0 * e4;
  c0 = (double)1.0-e2/(double)4.0-te4/(double)64.0-(double)5.0*e6/
       (double)256.0;
  c1 = (double)3.0*e2/(double)8.0+te4/(double)32.0+j;
  c2 = (double)15.0*e4/(double)256.0+j;
  c3 = (double)35.0*e6/(double)3072.0;

  lat = c0 * phi0;

  phi0s2 = c1 * sin((double)2.0*phi0);
  phi0s4 = c2 * sin((double)4.0*phi0);
  phi0s6 = c3 * sin((double)6.0*phi0);
  AM0 = a*(lat-phi0s2+phi0s4-phi0s6);



  dx = E - E0;
  dy = N - N0;
  Mc = AM0 + dy/sf;
  Muc = Mc / (c0*a);

  mus2 = b0 * sin((double)2.0*Muc);
  mus4 = b1 * sin((double)4.0*Muc);
  mus6 = B2 * sin((double)6.0*Muc);
  mus8 = b3 * sin((double)6.0*Muc);
  PHIc = Muc + mus2 + mus4 + mus6 + mus8;

  phis = sin(PHIc);
  x = e * phis;
  Qc = om*(phis/((double)1.-e2*phis*phis)-oo*
           log(((double)1.-x)/((double)1.+x)));
  Qco = Qc/qp;

  if (Qco>(double)1.) {
    Qco = (double)1.;
  } else if (Qco<(double)-1.) {
    Qco = (double)-1.;
  }

  btc = asin(Qco);
  btcc = cos(btc);
  t = sf*dx*btcc*pow((double)1.-e2*phis*phis,(double).5)/(a*cos(PHIc));
  if (t>(double)1.) {
    t=(double)1.;
  } else if (t<(double)-1.) {
    t=(double)-1.;
  }
  btp = -asin(t);
  bt = asin(cos(btp)*sin(btc));

  bts2 = A0 * sin((double)2.0*bt);
  bts4 = A1 * sin((double)4.0*bt);
  bts6 = A2 * sin((double)6.0*bt);
  *phi = bt + bts2 + bts4 + bts6;
  *lambda = M0 - atan(tan(btp)/btcc);

  if (*phi>po2) {
    *phi = po2;
  } else if (*phi<-po2) {
    *phi = -po2;
  }

  if (*lambda>GPS_PI) {
    *lambda -= p2;
  }
  if (*lambda<-GPS_PI) {
    *lambda += p2;
  }

  if (*lambda>GPS_PI) {
    *lambda = GPS_PI;
  } else if (*lambda<-GPS_PI) {
    *lambda=-GPS_PI;
  }

  *lambda = GPS_Math_Rad_To_Deg(*lambda);
  *phi    = GPS_Math_Rad_To_Deg(*phi);

  return;
}




/* @func GPS_Math_Mercator_LatLon_To_EN **********************************
**
** Convert latitude and longitude to standard Mercator projection
**  easting and northing
**
** @param [r] phi [double] latitude (deg)
** @param [r] lambda [double] longitude (deg)
** @param [w] E [double *] easting (metre)
** @param [w] N [double *] northing (metre)
** @param [r] phi0 [double] latitude of origin (deg)
** @param [r] lambda0 [double] longitude of origin (deg)
** @param [r] E0 [double] false easting
** @param [r] N0 [double] false northing
** @param [r] a [double] semi-major axis
** @param [r] b [double] semi-minor axis
**
** @return [void]
************************************************************************/
void GPS_Math_Mercator_LatLon_To_EN(double phi, double lambda, double* E,
                                    double* N, double phi0, double lambda0,
                                    double E0, double N0, double a, double b)
{
  double p2;
  double po2;
  double a2;
  double b2;
  double e2;
  double e4;
  double e3;
  double e;
  double es;
  double ab;
  double bb;
  double cb;
  double db;
  double ml;
  double phi0s;
  double sf;

  double dlam;
  double ct;
  double ex;
  double tt;
  double pt;


  lambda  = GPS_Math_Deg_To_Rad(lambda);
  phi     = GPS_Math_Deg_To_Rad(phi);
  phi0    = GPS_Math_Deg_To_Rad(phi0);
  lambda0 = GPS_Math_Deg_To_Rad(lambda0);

  ml = ((double)GPS_PI*(double)89.5)/(double)180.;

  p2 = (double)GPS_PI * (double)2.0;
  po2 = (double)GPS_PI / (double)2.0;
  if (lambda0>GPS_PI) {
    lambda0 -= p2;
  }
  a2 = a*a;
  b2 = b*b;
  es = (a2-b2)/a2;
  e2 = es*es;
  e3 = e2*es;
  e4 = e3*es;

  e  = pow(es,(double).5);
  phi0s = sin(phi0);
  sf = (double)1. / (pow((double)1.-es*phi0s*phi0s,(double).5)/cos(phi0));

  ab = es/(double)2.+(double)5.*e2/(double)24.+e3/(double)12.+(double)13.*
       e4/(double)360.;
  bb = (double)7.*e2/(double)48.+(double)29.*e3/(double)240.+
       (double)811.*e4/(double)11520.;
  cb = (double)7.*e3/(double)120.+(double)81.*e4/(double)1120.;
  db = (double)4279.*e4/(double)161280.;



  if (lambda>(double)GPS_PI) {
    lambda -= p2;
  }

  dlam = lambda - lambda0;
  if (dlam>GPS_PI) {
    dlam -= p2;
  }
  if (dlam<-GPS_PI) {
    dlam += p2;
  }


  ex = e * sin(phi);
  tt = tan((double)GPS_PI/(double)4.+phi/(double)2.);
  pt = pow((((double)1.-ex)/((double)1.+ex)),(e/(double)2.));

  ct = tt * pt;
  *N = sf * a * log(ct) + N0;
  *E = sf * a * dlam + E0;

  return;
}




/* @func GPS_Math_Mercator_EN_To_LatLon **********************************
**
** Convert standard Mercator easting and northing projection
** to latitude and longitude
**
** @param [r] E [double] easting (metre)
** @param [r] N [double] northing (metre)
** @param [w] phi [double *] latitude (deg)
** @param [w] lambda [double *] longitude (deg)
** @param [r] phi0 [double] latitude of origin (deg)
** @param [r] lambda0 [double] longitude of origin (deg)
** @param [r] E0 [double] false easting
** @param [r] N0 [double] false northing
** @param [r] a [double] semi-major axis
** @param [r] b [double] semi-minor axis
**
** @return [void]
************************************************************************/
void GPS_Math_Mercator_EN_To_LatLon(double E, double N, double* phi,
                                    double* lambda, double phi0,
                                    double lambda0, double E0, double N0,
                                    double a, double b)
{
  double p2;
  double po2;
  double a2;
  double b2;
  double e2;
  double e4;
  double e3;
  double e;
  double es;
  double ab;
  double bb;
  double cb;
  double db;
  double ml;
  double phi0s;
  double sf;

  double dx;
  double dy;
  double x;

  phi0    = GPS_Math_Deg_To_Rad(phi0);
  lambda0 = GPS_Math_Deg_To_Rad(lambda0);

  ml = ((double)GPS_PI*(double)89.5)/(double)180.;

  p2 = (double)GPS_PI * (double)2.0;
  po2 = (double)GPS_PI / (double)2.0;
  if (lambda0>GPS_PI) {
    lambda0 -= p2;
  }
  a2 = a*a;
  b2 = b*b;
  es = (a2-b2)/a2;
  e2 = es*es;
  e3 = e2*es;
  e4 = e3*es;

  e  = pow(es,(double).5);
  phi0s = sin(phi0);
  sf = (double)1. / (pow((double)1.-es*phi0s*phi0s,(double).5)/cos(phi0));

  ab = es/(double)2.+(double)5.*e2/(double)24.+e3/(double)12.+(double)13.*
       e4/(double)360.;
  bb = (double)7.*e2/(double)48.+(double)29.*e3/(double)240.+
       (double)811.*e4/(double)11520.;
  cb = (double)7.*e3/(double)120.+(double)81.*e4/(double)1120.;
  db = (double)4279.*e4/(double)161280.;

  dx = E - E0;
  dy = N - N0;
  *lambda = lambda0 + dx / (sf*a);
  x = (double)GPS_PI / (double)2. -
      (double)2.*atan((double)1./exp(dy/(sf*a)));
  *phi = x+ab*sin((double)2.*x)+bb*sin((double)4.*x)+cb*sin((double)6.*x)
         + db*sin((double)8.*x);

  if (*phi>po2) {
    *phi = po2;
  } else if (*phi<-po2) {
    *phi = -po2;
  }

  if (*lambda>GPS_PI) {
    *lambda -= p2;
  }
  if (*lambda<-GPS_PI) {
    *lambda += p2;
  }

  if (*lambda>GPS_PI) {
    *lambda = GPS_PI;
  } else if (*lambda<-GPS_PI) {
    *lambda=-GPS_PI;
  }

  *lambda = GPS_Math_Rad_To_Deg(*lambda);
  *phi    = GPS_Math_Rad_To_Deg(*phi);

  return;
}




/* @func GPS_Math_TMerc_LatLon_To_EN **********************************
**
** Convert latitude and longitude to transverse Mercator projection
**  easting and northing
**
** @param [r] phi [double] latitude (deg)
** @param [r] lambda [double] longitude (deg)
** @param [w] E [double *] easting (metre)
** @param [w] N [double *] northing (metre)
** @param [r] phi0 [double] latitude of origin (deg)
** @param [r] lambda0 [double] longitude of origin (deg)
** @param [r] E0 [double] false easting
** @param [r] N0 [double] false northing
** @param [r] F0 [double] scale factor
** @param [r] a [double] semi-major axis
** @param [r] b [double] semi-minor axis
**
** @return [void]
************************************************************************/
void GPS_Math_TMerc_LatLon_To_EN(double phi, double lambda, double* E,
                                 double* N, double phi0, double lambda0,
                                 double E0, double N0, double F0,
                                 double a, double b)
{
  GPS_Math_LatLon_To_EN(E,N,phi,lambda,N0,E0,phi0,lambda0,F0,a,b);

  return;
}




/* @func GPS_Math_TMerc_EN_To_LatLon **********************************
**
** Convert transverse Mercator easting and northing projection
** to latitude and longitude
**
** @param [r] E [double] easting (metre)
** @param [r] N [double] northing (metre)
** @param [w] phi [double *] latitude (deg)
** @param [w] lambda [double *] longitude (deg)
** @param [r] phi0 [double] latitude of origin (deg)
** @param [r] lambda0 [double] longitude of origin (deg)
** @param [r] E0 [double] false easting
** @param [r] N0 [double] false northing
** @param [r] F0 [double] scale factor
** @param [r] a [double] semi-major axis
** @param [r] b [double] semi-minor axis
**
** @return [void]
************************************************************************/
void GPS_Math_TMerc_EN_To_LatLon(double E, double N, double* phi,
                                 double* lambda, double phi0, double lambda0,
                                 double E0, double N0, double F0,
                                 double a, double b)
{
  GPS_Math_EN_To_LatLon(E,N,phi,lambda,N0,E0,phi0,lambda0,F0,a,b);

  return;
}




/* @func GPS_Math_Swiss_LatLon_To_EN ***********************************
**
** Convert latitude and longitude to Swiss grid easting and northing
**
** @param [r] phi [double] latitude (deg)
** @param [r] lambda [double] longitude (deg)
** @param [w] E [double *] easting (metre)
** @param [w] N [double *] northing (metre)
** @param [r] phi0 [double] latitude origin (deg)     [normally 46.95240556]
** @param [r] lambda0 [double] longitude origin (deg) [normally  7.43958333]
** @param [r] E0 [double] false easting (metre)       [normally 600000.0]
** @param [r] N0 [double] false northing (metre)      [normally 200000.0]
** @param [r] a [double] semi-major axis              [normally 6377397.000]
** @param [r] b [double] semi-minor axis              [normally 6356078.823]
**
** @return [void]
***************************************************************************/
void GPS_Math_Swiss_LatLon_To_EN(double phi, double lambda, double* E,
                                 double* N,double phi0,double lambda0,
                                 double E0, double N0, double a, double b)

{
  double a2;
  double b2;
  double esq;
  double e;
  double c;
  double ephi0p;
  double phip;
  double sphip;
  double phid;
  double slambda2;
  double lambda1;
  double lambda2;
  double K;
  double po4;
  double w;
  double R;

  lambda0 = GPS_Math_Deg_To_Rad(lambda0);
  phi0    = GPS_Math_Deg_To_Rad(phi0);
  lambda  = GPS_Math_Deg_To_Rad(lambda);
  phi     = GPS_Math_Deg_To_Rad(phi);

  po4=GPS_PI/(double)4.0;

  a2 = a*a;
  b2 = b*b;
  esq = (a2-b2)/a2;
  e   = pow(esq,(double)0.5);

  c = sqrt(1+((esq*pow(cos(phi0),(double)4.))/((double)1.-esq)));

  ephi0p = asin(sin(phi0)/c);

  K = log(tan(po4+ephi0p/(double)2.)) - c*(log(tan(po4+phi0/(double)2.)) -
      e/(double)2. * log(((double)1.+e*sin(phi0)) /
                         ((double)1.-e*sin(phi0))));
  lambda1 = c*(lambda-lambda0);
  w = c*(log(tan(po4+phi/(double)2.)) - e/(double)2. *
         log(((double)1.+e*sin(phi)) / ((double)1.-e*sin(phi)))) + K;


  phip = (double)2. * (atan(exp(w)) - po4);

  sphip = cos(ephi0p) * sin(phip) - sin(ephi0p) * cos(phip) * cos(lambda1);
  phid  = asin(sphip);

  slambda2 = cos(phip)*sin(lambda1) / cos(phid);
  lambda2  = asin(slambda2);

  R = a*sqrt((double)1.-esq) / ((double)1.-esq*sin(phi0) * sin(phi0));

  *N = R*log(tan(po4 + phid/(double)2.)) + N0;
  *E = R*lambda2 + E0;
  return;
}




/* @func GPS_Math_Swiss_EN_To_LatLon ************************************
**
** Convert Swiss Grid easting and northing to latitude and longitude
**
** @param [r] E [double] easting (metre)
** @param [r] N [double] northing (metre)
** @param [w] phi [double *] latitude (deg)
** @param [w] lambda [double *] longitude (deg)
** @param [r] phi0 [double] latitude origin (deg)     [normally 46.95240556]
** @param [r] lambda0 [double] longitude origin (deg) [normally  7.43958333]
** @param [r] E0 [double] false easting (metre)       [normally 600000.0]
** @param [r] N0 [double] false northing (metre)      [normally 200000.0]
** @param [r] a [double] semi-major axis              [normally 6377397.000]
** @param [r] b [double] semi-minor axis              [normally 6356078.823]
**
** @return [void]
*************************************************************************/

void GPS_Math_Swiss_EN_To_LatLon(double E, double N, double* phi,
                                 double* lambda, double phi0, double lambda0,
                                 double E0, double N0, double a, double b)
{
  double a2;
  double b2;
  double esq;
  double e;
  double R;
  double c;
  double po4;
  double phid;
  double phi1;
  double lambdad;
  double lambda1;
  double slambda1;
  double ephi0p;
  double sphip;
  double tol;
  double cr;
  double C;
  double K;

  lambda0 = GPS_Math_Deg_To_Rad(lambda0);
  phi0    = GPS_Math_Deg_To_Rad(phi0);

  po4=GPS_PI/(double)4.0;
  tol=(double)0.00001;

  a2 = a*a;
  b2 = b*b;
  esq = (a2-b2)/a2;
  e   = pow(esq,(double)0.5);

  R = a*sqrt((double)1.-esq) / ((double)1.-esq*sin(phi0) * sin(phi0));

  phid = (double)2.*(atan(exp((N - N0)/R)) - po4);
  lambdad = (E - E0)/R;

  c = sqrt((double)1.+((esq * pow(cos(phi0), (double)4.)) /
                       ((double)1.-esq)));
  ephi0p = asin(sin(phi0) / c);

  sphip = cos(ephi0p)*sin(phid) + sin(ephi0p)*cos(phid)*cos(lambdad);
  phi1 = asin(sphip);

  slambda1 = cos(phid)*sin(lambdad)/cos(phi1);
  lambda1  = asin(slambda1);

  *lambda = GPS_Math_Rad_To_Deg((lambda1/c + lambda0));

  K = log(tan(po4 + ephi0p/(double)2.)) -c*(log(tan(po4 + phi0/(double)2.))
      - e/(double)2. * log(((double)1.+e*sin(phi0)) /
                           ((double)1.-e*sin(phi0))));
  C = (K - log(tan(po4 + phi1/(double)2.)))/c;

  do {
    cr = (C + log(tan(po4 + phi1/(double)2.)) - e/(double)2. *
          log(((double)1.+e*sin(phi1)) / ((double)1.-e*sin(phi1)))) *
         ((((double)1.-esq*sin(phi1)*sin(phi1)) * cos(phi1)) /
          ((double)1.-esq));
    phi1 -= cr;
  } while (fabs(cr) > tol);

  *phi = GPS_Math_Rad_To_Deg(phi1);

  return;
}
