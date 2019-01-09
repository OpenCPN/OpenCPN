#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "geodesic.h"

/* These methods implemented using the Vincenty method.
 *
 * Vincenty's original paper is available here:
 * http://www.ngs.noaa.gov/PUBS_LIB/inverse.pdf
 *
 * Great examples of the methods are available at these locations
 * http://www.movable-type.co.uk/scripts/latlong-vincenty.html
 * http://www.codeproject.com/KB/cs/Vincentys_Formula.aspx
 *
 * The movable-type.co.uk page contains code (C) 2002-2008 Chris Veness
 * and available under a LGPL license.
 *
 * The codeproject.com page contains code available under the
 * Code Project Open License (CPOL) 1.02.
 *
 * These references are a courtesy only, as the code here is original
 * and not a derivitive work of the code provided on these pages.
 */

/*
 * The Vincenty method does not converge for antipodal points, but the
 * code here handles this special case.  Since the earth is a flattened
 * sphere, the shortest route is over the poles.  The trip via the North or South
 * Pole is equivalent, so we arbitrarily pick the South Pole.
 *
 * The length of a great circle route halfway around the world through the poles is
 * 0.5 of the circumference of a circle with a radius of the semi minor axis.
 */

double Geodesic::GreatCircleDistBear( double Lon1, double Lat1, double Lon2, double Lat2,
        double *Dist, double *Bear1, double *Bear2 )
{

    /* Geodesic parameters per WGS-84 */
    double a = GEODESIC_WGS84_SEMI_MAJORAXIS; /* in meters */
    double b = GEODESIC_WGS84_SEMI_MINORAXIS; /* in meters */
    double f = ( GEODESIC_WGS84_SEMI_MAJORAXIS - GEODESIC_WGS84_SEMI_MINORAXIS )
            / GEODESIC_WGS84_SEMI_MAJORAXIS; /* Flattening */

    double dLon; /* Change in longitude */
    double rLat1, rLat2; /* Reduced Latitude */
    double lambda, lambdaprime; /* Counting variables */
    double sinrLat1, cosrLat1, sinrLat2, cosrLat2, sinlambda, coslambda; /* Intermediate calculations */
    double sinsigma, cossigma, cos2sigmam, sigma, sinalpha, cos2alpha, C; /* Intermediate calculations */
    double u2, A, B; /* Intermediate calculations */
    double dist; /* Great circle distance */
    int itersleft = 50; /* Convergence attempts */

    /* Initialize the output variables */
    if( Dist ) *Dist = 0.0;
    if( Bear1 ) *Bear1 = 0.0;
    if( Bear2 ) *Bear2 = 0.0;

    if( fabs( Lon1 - Lon2 ) < 1e-12 && fabs( Lat1 - Lat2 ) < 1e-12 ) {
        /* The start and end points are the same - the distance is zero */
        return 0.0;
    }

    /* Convert inputs from degrees to radians */
    Lon1 = GEODESIC_DEG2RAD(Lon1);
    Lat1 = GEODESIC_DEG2RAD(Lat1);
    Lon2 = GEODESIC_DEG2RAD(Lon2);
    Lat2 = GEODESIC_DEG2RAD(Lat2);

    /* Start the algorithm */
    rLat1 = atan( ( 1 - f ) * tan( Lat1 ) );
    rLat2 = atan( ( 1 - f ) * tan( Lat2 ) );
    sinrLat1 = sin( rLat1 );
    cosrLat1 = cos( rLat1 );
    sinrLat2 = sin( rLat2 );
    cosrLat2 = cos( rLat2 );
    dLon = Lon2 - Lon1;

    lambda = dLon;
    lambdaprime = 2 * M_PI;

    do {
        sinlambda = sin( lambda );
        coslambda = cos( lambda );
        sinsigma = sqrt(
                pow( cosrLat2 * sinlambda, 2.0 )
                        + pow( cosrLat1 * sinrLat2 - sinrLat1 * cosrLat2 * coslambda, 2.0 ) );
        if( sinsigma < 1e-12 ) {
            /* The points are antipodal */
            dist = M_PI * b;
            if( Dist ) *Dist = dist;
            if( Bear1 ) *Bear1 = 180.0; /* Start heading for the South Pole */
            if( Bear2 ) *Bear2 = 0.0; /* Wind up heading for the North Pole */

            return dist;
        }
        cossigma = sinrLat1 * sinrLat2 + cosrLat1 * cosrLat2 * coslambda;
        sigma = atan2( sinsigma, cossigma );
        if( sinsigma == 0.0 ) sinalpha = 0.0;
        else
            sinalpha = cosrLat1 * cosrLat2 * sinlambda / sinsigma;
        cos2alpha = 1 - pow( sinalpha, 2.0 );
        if( cos2alpha == 0.0 ) cos2sigmam = 0.0;
        else
            cos2sigmam = cossigma - 2 * sinrLat1 * sinrLat2 / cos2alpha;
        //if( isnan( cos2sigmam ) ) cos2sigmam = 0.0; /* Special case to handle circles at the equator */
        C = f / 16 * cos2alpha * ( 4 + f * ( 4 - 3 * cos2alpha ) );
        lambdaprime = lambda;
        lambda = dLon + ( 1.0 - C ) * f * sinalpha * ( sigma + C * sinsigma *
                ( cos2sigmam + C * cossigma * ( -1.0 + 2.0 * pow( cos2sigmam, 2.0 ) ) ) );
    } while( fabs( lambda - lambdaprime ) > 1e-12 && ( itersleft-- ) );

    if( itersleft == 0 ) {
        /* It didn't converge.  Assume antipodal points. */
        dist = M_PI * b;
        if( Dist ) *Dist = dist;
        if( Bear1 ) *Bear1 = 180.0; /* Start heading for the South Pole */
        if( Bear2 ) *Bear2 = 0.0; /* Wind up heading for the North Pole */

        return dist;
    }
    u2 = cos2alpha * ( pow( a, 2.0 ) - pow( b, 2.0 ) ) / pow( b, 2.0 );
    A = 1 + u2 / 16384 * ( 4096 + u2 * ( -768 + u2 * ( 320 - 175 * u2 ) ) );
    B = u2 / 1024 * ( 256 + u2 * ( -128 + u2 * ( 74 - 74 * u2 ) ) );
    dist = b * A * ( sigma - B * sinsigma * ( cos2sigmam + B / 4 * ( cossigma *
            ( -1.0 + 2.0 * pow( cos2sigmam, 2.0 ) ) - B / 6 * cos2sigmam * ( -3.0 + 4.0 * pow( sinsigma, 2.0 ) )
            * ( -3 + 4 * pow( cos2sigmam, 2.0 ) ) ) ) );
    if( Dist ) *Dist = dist;
    if( Bear1 ) {
        *Bear1 = GEODESIC_RAD2DEG(atan2(cosrLat2*sinlambda,cosrLat1*sinrLat2-sinrLat1*cosrLat2*coslambda));
        while( *Bear1 < 0.0 )
            *Bear1 += 360.0;
    }
    if( Bear2 ) {
        *Bear2 = GEODESIC_RAD2DEG(atan2(cosrLat1*sinlambda,-sinrLat1*cosrLat2+cosrLat1*sinrLat2*coslambda));
        while( *Bear2 < 0.0 )
            *Bear2 += 360.0;
    }
    return dist;
}

/* Vincenty's direct solution.  The equation numbers related to
 * the equation numbers in the following:
 * http://www.ngs.noaa.gov/PUBS_LIB/inverse.pdf
 */
void Geodesic::GreatCircleTravel( double Lon1, double Lat1, double Dist, double Bear1, double *Lon2,
        double *Lat2, double *Bear2 )
{
    /* Geodesic parameters per WGS-84 */
    double a = GEODESIC_WGS84_SEMI_MAJORAXIS; /* in meters */
    double b = GEODESIC_WGS84_SEMI_MINORAXIS; /* in meters */
    double f = ( GEODESIC_WGS84_SEMI_MAJORAXIS - GEODESIC_WGS84_SEMI_MINORAXIS )
            / GEODESIC_WGS84_SEMI_MAJORAXIS; /* Flattening */

    /* Initialize to where we started from */
    if( Lon2 ) *Lon2 = Lon1;
    if( Lat2 ) *Lat2 = Lat1;
    if( Bear2 ) *Bear2 = Bear1;

    if( Dist < 1e-12 ) {
        /* There's no distance to travel, so we're done. */
        return;
    }

    /* Convert inputs from degrees to radians */
    Lon1 = GEODESIC_DEG2RAD(Lon1);
    Lat1 = GEODESIC_DEG2RAD(Lat1);
    Bear1 = GEODESIC_DEG2RAD(Bear1);
    if( Lon2 ) *Lon2 = Lon1;
    if( Lat2 ) *Lat2 = Lat1;
    if( Bear2 ) *Bear2 = Bear1;

    double sigma1; /* Angular distance on the sphere from equator to Lon1/Lat1 */
    double rLat1; /* Reduced Latitude */
    double sinrLat1; /* Sine of reduced latitude */
    double cosrLat1; /* Cosine of reduced latitude */
    double sinalpha1; /* Sine of the initial bearing */
    double cosalpha1; /* Cosine of the initial bearing */
    double sinalpha; /* Sine of the azimuth of the geodesic at the equator */
    double sin2alpha; /* sinalpha^2 */
    double cos2alpha; /* cosalpha^2 */
    double sigma; /* Angular distance on the sphere */
    double sinsigma; /* Sine of the angular distance on the sphere */
    double cossigma; /* Cosine of the angular distance on the sphere */
    double sigmaprime; /* Previous value of sigma */
    double lambda; /* Difference in longitude on an auxiliary sphere */
    double L; /* Difference in longitude, positive East */
    double u2, A, B, C, twosigmam, costwosigmam, cos2twosigmam, deltasigma, distoverba; /* Intermediate calculations */

    /* Start the algorithm */
    rLat1 = ( 1.0 - f ) * tan( Lat1 ); /* tan U=(1-f)*tan(phi) */
    cosrLat1 = 1.0 / sqrt( 1.0 + rLat1 * rLat1 ); /* via trig identity */
    sinrLat1 = rLat1 * cosrLat1; /* via trig identity */
    sinalpha1 = sin( Bear1 );
    cosalpha1 = cos( Bear1 );

    sigma1 = atan2( rLat1, cosalpha1 ); /* Eq. 1 */

    sinalpha = cosrLat1 * sinalpha1; /* Eq. 2 */
    sin2alpha = sinalpha * sinalpha;

    cos2alpha = 1 - ( sin2alpha ); /* cos2=1-sin2 */
    u2 = cos2alpha * ( ( a * a ) - ( b * b ) ) / ( b * b );
    A = 1 + ( u2 / 16384 ) * ( 4096 + u2 * ( -768 + u2 * ( 320 - 175 * u2 ) ) ); /* Eq. 3 */

    B = ( u2 / 1024 ) * ( 256 + u2 * ( -128 + u2 * ( 74 - 47 * u2 ) ) ); /* Eq. 4 */

    distoverba = Dist / ( b * A );
    sigma = distoverba;
    sigmaprime = sigma - 1.0; /* Something to get the loop started */
    while( fabs( sigmaprime - sigma ) > 1e-12 ) {
        twosigmam = 2 * sigma1 + sigma; /* Eq. 5 */
        costwosigmam = cos( twosigmam );
        cos2twosigmam = costwosigmam * costwosigmam;
        sinsigma = sin( sigma );

        deltasigma = B * sinsigma
                * ( costwosigmam
                        + ( B / 4.0 )
                                * /* Eq. 6 */
                                ( cos( sigma ) * ( -1 + 2 * cos2twosigmam )
                                        - ( B / 6.0 ) * costwosigmam
                                                * ( -3 + 4 * sinsigma * sinsigma )
                                                * ( -3 + 4 * cos2twosigmam ) ) );

        sigmaprime = sigma;
        sigma = distoverba + deltasigma; /* Eq. 7 */
    }
    sinsigma = sin( sigma );
    cossigma = cos( sigma );
    twosigmam = 2 * sigma1 + sigma; /* Eq. 5 */
    costwosigmam = cos( twosigmam );
    cos2twosigmam = costwosigmam * costwosigmam;

    if( Lat2 ) {
        *Lat2 = atan2( /* Eq. 8 */
        sinrLat1 * cossigma + cosrLat1 * sinsigma * cosalpha1,
                ( 1 - f )
                        * sqrt(
                                sin2alpha
                                        + pow(
                                                sinrLat1 * sinsigma
                                                        - cosrLat1 * cossigma * cosalpha1,
                                                2.0 ) ) );
        *Lat2 = GEODESIC_RAD2DEG(*Lat2);
    }

    if( Lon2 ) {
        lambda = atan2( sinsigma * sinalpha1, /* Eq. 9 */
        cosrLat1 * cossigma - sinrLat1 * sinsigma * cosalpha1 );

        C = ( f / 16.0 ) * cos2alpha * ( 4 + f * ( 4 - 3 * cos2alpha ) ); /* Eq. 10 */

        L = lambda - ( 1 - C ) * f * sinalpha * ( sigma + C * sinsigma * /* Eq. 11 */
        ( costwosigmam + C * cossigma * ( -1 + 2 * cos2twosigmam ) ) );
        *Lon2 = GEODESIC_RAD2DEG(Lon1+L);
    }

    if( Bear2 ) {
        *Bear2 = atan2( sinalpha, /* Eq. 12 */
        -sinrLat1 * sinsigma + cosrLat1 * cossigma * cosalpha1 );
        *Bear2 = GEODESIC_RAD2DEG(*Bear2);
    }

    return;
}
