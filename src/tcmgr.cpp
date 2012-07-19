/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Tide and Current Manager
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
 *
 *
 */



#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif //precompiled headers

#include "dychart.h"

#include <stdlib.h>
#include <math.h>
#include <time.h>

#include <wx/listimpl.cpp>
#include <wx/datetime.h>
#include <wx/textfile.h>
#include <wx/tokenzr.h>
#include <wx/filename.h>

#include "tcmgr.h"
#include "georef.h"

#ifndef PI
#define PI        3.1415926535897931160E0      /* pi */
#endif

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(ArrayOfAbbrEntry);
WX_DEFINE_OBJARRAY(ArrayOfIDXEntry);
WX_DEFINE_OBJARRAY(ArrayOfTCDSources);
WX_DEFINE_OBJARRAY(ArrayOfStationData);

extern wxArrayString             TideCurrentDataSet;

//--------------------------------------------------------------------------------
//    Some Time Converters
//--------------------------------------------------------------------------------


/* Turn a time displacement of the form [-]HH:MM into the number of seconds. */
static int
hhmm2seconds (char *hhmm)
{
    int h, m;
    char s;
    if (sscanf (hhmm, "%d:%d", &h, &m) != 2)
        return(0);
    if (sscanf (hhmm, "%c", &s) != 1)
        return(0);
    if (h < 0 || s == '-')
        m = -m;
    return h*3600 + m*60;
}


//-----------------------------------------------------------------------------------
//    TIDELIB
//-----------------------------------------------------------------------------------

#define IFF_OPEN  0
#define IFF_CLOSE 1
#define IFF_SEEK  2
#define IFF_TELL  3
#define IFF_READ  4

//      Static variables for the TIDELIB

time_t s_next_epoch      = TIDE_BAD_TIME; /* next years newyears */
time_t s_this_epoch      = TIDE_BAD_TIME; /* this years newyears */
int    s_this_year       = -1;
int    s_year            = -1;

double time2dt_tide (time_t t, int deriv, IDX_entry *pIDX);
int yearoftimet (time_t t);
void happy_new_year (IDX_entry *pIDX, int new_year);
void set_epoch (IDX_entry *pIDX, int year);

double time2tide (time_t t, IDX_entry *pIDX)
{
    return time2dt_tide(t, 0, pIDX);
}



/** BOGUS amplitude stuff - Added mgh
 * For knots^2 current stations, returns square root of (value * amplitude),
 * For normal stations, returns value * amplitude */

double BOGUS_amplitude(double mpy, IDX_entry *pIDX)
{
    Station_Data *pmsd = pIDX->pref_sta_data;

    if (!pmsd->have_BOGUS)                                // || !convert_BOGUS)   // Added mgh
        return(mpy * pIDX->max_amplitude);
    else {
        if (mpy >= 0.0)
            return( sqrt( mpy * pIDX->max_amplitude));
        else
            return(-sqrt(-mpy * pIDX->max_amplitude));
    }
}

/* Calculate the denormalized tide. */
double time2atide (time_t t, IDX_entry *pIDX)
{
    return BOGUS_amplitude(time2tide(t, pIDX), pIDX) + pIDX->pref_sta_data->DATUM;
}


/* Next high tide, low tide, transition of the mark level, or some
 *   combination.
 *       Bit      Meaning
 *        0       low tide
 *        1       high tide
 *        2       falling transition
 *        3       rising transition
 */
int next_big_event (time_t *tm, IDX_entry *pIDX)
{
    double p, q;
    int flags = 0, slope = 0;
    p = time2atide (*tm, pIDX);
    *tm += 60;
    q = time2atide (*tm, pIDX);
    *tm += 60;
    if (p < q)
        slope = 1;
    while (1) {
        if ((slope == 1 && q < p) || (slope == 0 && p < q)) {
            /* Tide event */
            flags |= (1 << slope);
        }
        /* Modes in which to return mark transitions: */
        /*    -text (no -graph)   */
        /*    -graph (no -text)   */
        /*    -ppm                */
        /*    -gif                */
        /*    -ps                 */


        //    if (mark && ((text && !graphmode) || (!text && graphmode)
        //    || ppm || gif || ps))
        //      int marklev = 0;
#if (0)
        if(0)
            if ((p > marklev && q <= marklev) || (p < marklev && q >= marklev)) {
                /* Transition event */
                if (p < q)
                    flags |= 8;
                else
                    flags |= 4;
                if (!(flags & 3)) {
                    /* If we're incredibly unlucky, we could miss a tide event if we
                     *                       don't check for it here:
                     *
                     *                                       . <----   Value that would be returned
                     *                                  -----------    Mark level
                     *                                .           .
                     */
                    p = q;
                    q = time2atide (*tm, pIDX);
                    if ((slope == 1 && q < p) || (slope == 0 && p < q)) {
                        /* Tide event */
                        flags |= (1 << slope);
                    }
                }
            }
#endif

        if (flags) {
            *tm -= 60;
            /* Don't back up over a transition event, but do back up to where the
             *               tide changed if possible.  If they happen at the same time, then
             *               we're off by a minute on the tide, but if we back it up it will
             *               get snagged on the transition event over and over. */
            if (flags < 4)
                *tm -= 60;
            return flags;
        }
        p = q;
        q = time2atide (*tm, pIDX);
        *tm += 60;
    }
}



/* Estimate the normalized mean tide level around a particular time by
 *   summing only the long-term constituents. */
/* Does not do any blending around year's end. */
/* This is used only by time2asecondary for finding the mean tide level */
double time2mean (time_t t, IDX_entry *pIDX)
{
    double tide = 0.0;
    int a;
    int new_year = yearoftimet (t);
    if (s_year != new_year)
        happy_new_year (pIDX, new_year);

    for (a=0; a<pIDX->num_csts; a++) {
        if (pIDX->m_cst_speeds[a] < 6e-6) {
            tide += pIDX->m_work_buffer[a] *
                    cos (pIDX->m_cst_speeds[a] * ((long)(t - pIDX->epoch) + pIDX->pref_sta_data->meridian) +
                         pIDX->m_cst_epochs[a][new_year-pIDX->first_year] - pIDX->pref_sta_data->epoch[a]);
        }
    }

    return tide;
}



/* If offsets are in effect, interpolate the 'corrected' denormalized
 * tide.  The normalized is derived from this, instead of the other way
 * around, because the application of height offsets requires the
 * denormalized tide. */
double time2asecondary (time_t t, IDX_entry *pIDX) {

    /* Get rid of the normals. */
    if (!(pIDX->have_offsets))
        return time2atide (t, pIDX);

    {
        /* Intervalwidth of 14 (was originally 13) failed on this input:
         *        -location Dublon -hloff +0.0001 -gstart 1997:09:10:00:00 -raw 1997:09:15:00:00
         */
#define intervalwidth 15
#define stretchfactor 3

        static time_t lowtime=0, hightime=0;
        static double lowlvl, highlvl; /* Normalized tide levels for MIN, MAX */
        time_t T;  /* Adjusted t */
        double S, Z, HI, HS, magicnum;
        time_t interval = 3600 * intervalwidth;
        long difflow, diffhigh;
        int badlowflag=0, badhighflag=0;


        /* Algorithm by Jean-Pierre Lapointe (scipur@collegenotre-dame.qc.ca) */
        /* as interpreted, munged, and implemented by DWF */

        /* This is the initial guess (average of time offsets) */
        //    T = t - (httimeoff + lttimeoff) / 2;
        T = t - (pIDX->IDX_ht_time_off * 60 + pIDX->IDX_lt_time_off * 60) / 2;
        /* The usage of an estimate of mean tide level here is to correct
         *           for seasonal changes in tide level.  Previously I had simply used
         *           the zero of the tide function as the mean, but this gave bad
         *           results around summer and winter for locations with large seasonal
         *           variations. */
//        printf("-----time2asecondary  %ld %ld %d %d\n", t, T, pIDX->IDX_ht_time_off ,pIDX->IDX_lt_time_off);

        Z = time2mean(T, pIDX);
        S = time2tide(T, pIDX) - Z;

        /* Find MAX and MIN.  I use the highest high tide and the lowest
         *           low tide over a 26 hour period, but I allow the interval to stretch
         *           a lot if necessary to avoid creating discontinuities.  The
         *           heuristic used is not perfect but will hopefully be good enough.
         *
         *           It is an assumption in the algorithm that the tide level will
         *           be above the mean tide level for MAX and below it for MIN.  A
         *           changeover occurs at mean tide level.  It would be nice to
         *           always use the two tides that immediately bracket T and to put
         *           the changeover at mid tide instead of always at mean tide
         *           level, since this would eliminate much of the inaccuracy.
         *           Unfortunately if you change the location of the changeover it
         *           causes the tide function to become discontinuous.
         *
         *           Now that I'm using time2mean, the changeover does move, but so
         *           slowly that it makes no difference.
         */

        if (lowtime < T)
            difflow = T - lowtime;
        else
            difflow = lowtime - T;
        if (hightime < T)
            diffhigh = T - hightime;
        else
            diffhigh = hightime - T;

        /* Update MIN? */
        if (difflow > interval * stretchfactor)
            badlowflag = 1;
        if (badlowflag || (difflow > interval && S > 0)) {
            time_t tt;
            double tl;
            tt = T - interval;
            next_big_event (&tt, pIDX);
            lowlvl = time2tide (tt, pIDX);
            lowtime = tt;
            while (tt < T + interval) {
                next_big_event (&tt, pIDX);
                tl = time2tide (tt, pIDX);
                if (tl < lowlvl && tt < T + interval) {
                    lowlvl = tl;
                    lowtime = tt;
                }
            }
        }
        /* Update MAX? */
        if (diffhigh > interval * stretchfactor)
            badhighflag = 1;
        if (badhighflag || (diffhigh > interval && S < 0)) {
            time_t tt;
            double tl;
            tt = T - interval;
            next_big_event (&tt, pIDX);
            highlvl = time2tide (tt, pIDX);
            hightime = tt;
            while (tt < T + interval) {
                next_big_event (&tt, pIDX);
                tl = time2tide (tt, pIDX);
                if (tl > highlvl && tt < T + interval) {
                    highlvl = tl;
                    hightime = tt;
                }
            }
        }

#if 0
        /* UNFORTUNATELY there are times when the tide level NEVER CROSSES
         *           THE MEAN for extended periods of time.  ARRRGH!  */
        if (lowlvl >= 0.0)
            lowlvl = -1.0;
        if (highlvl <= 0.0)
            highlvl = 1.0;
#endif
        /* Now that I'm using time2mean, I should be guaranteed to get
         *           an appropriate low and high. */


        /* Improve the initial guess. */
        if (S > 0)
            magicnum = 0.5 * S / fabs(highlvl - Z);
        else
            magicnum = 0.5 * S / fabs(lowlvl - Z);
        //    T = T - magicnum * (httimeoff - lttimeoff);
        T = T - (time_t)(magicnum * ((pIDX->IDX_ht_time_off * 60) - (pIDX->IDX_lt_time_off * 60)));
        HI = time2tide(T, pIDX);

        //    Correct the amplitude offsets for BOGUS knot^2 units
        double ht_off, lt_off;
        if (pIDX->pref_sta_data->have_BOGUS)
        {
            ht_off = pIDX->IDX_ht_off * pIDX->IDX_ht_off;         // Square offset in kts to adjust for kts^2
            lt_off = pIDX->IDX_lt_off * pIDX->IDX_lt_off;
        }
        else
        {
            ht_off = pIDX->IDX_ht_off;
            lt_off = pIDX->IDX_lt_off;
        }


        /* Denormalize and apply the height offsets. */
        HI = BOGUS_amplitude(HI, pIDX) + pIDX->pref_sta_data->DATUM;
        {
            double RH=1.0, RL=1.0, HH=0.0, HL=0.0;
            RH = pIDX->IDX_ht_mpy;
            HH = ht_off;
            RL = pIDX->IDX_lt_mpy;
            HL = lt_off;

            /* I patched the usage of RH and RL to avoid big ugly
             *            discontinuities when they are not equal.  -- DWF */

            HS =  HI * ((RH+RL)/2 + (RH-RL)*magicnum)
                  + (HH+HL)/2 + (HH-HL)*magicnum;
        }

        return HS;
    }
}






/*
 * We will need a function for tidal height as a function of time
 * which is continuous (and has continuous first and second derivatives)
 * for all times.
 *
 * Since the epochs & multipliers for the tidal constituents change
 * with the year, the regular time2tide(t) function has small
 * discontinuities at new years.  These discontinuities really
 * fry the fast root-finders.
 *
 * We will eliminate the new-years discontinuities by smoothly
 * interpolating (or "blending") between the tides calculated with one
 * year's coefficients, and the tides calculated with the next year's
 * coefficients.
 *
 * i.e. for times near a new years, we will "blend" a tide
 * as follows:
 *
 * tide(t) = tide(year-1, t)
 *                  + w((t - t0) / Tblend) * (tide(year,t) - tide(year-1,t))
 *
 * Here:  t0 is the time of the nearest new-year.
 *        tide(year-1, t) is the tide calculated using the coefficients
 *           for the year just preceding t0.
 *        tide(year, t) is the tide calculated using the coefficients
 *           for the year which starts at t0.
 *        Tblend is the "blending" time scale.  This is set by
 *           the macro TIDE_BLEND_TIME, currently one hour.
 *        w(x) is the "blending function", whice varies smoothly
 *           from 0, for x < -1 to 1 for x > 1.
 *
 * Derivatives of the blended tide can be evaluated in terms of derivatives
 * of w(x), tide(year-1, t), and tide(year, t).  The blended tide is
 * guaranteed to have as many continuous derivatives as w(x).  */

/* time2dt_tide(time_t t, int n)
 *
 *   Calculate nth time derivative the normalized tide.
 *
 * Notes: This function does not check for changes in year.
 *  This is important to our algorithm, since for times near
 *  new years, we interpolate between the tides calculated
 *  using one years coefficients, and the next years coefficients.
 *
 *  Except for this detail, time2dt_tide(t,0) should return a value
 *  identical to time2tide(t).
 */
double _time2dt_tide (time_t t, int deriv, IDX_entry *pIDX)
{
    double dt_tide = 0.0;
    int a, b;
    double term, tempd;

    int year = yearoftimet(t);

    tempd = M_PI / 2.0 * deriv;
    for (a=0; a<pIDX->num_csts; a++)
    {
        term = pIDX->m_work_buffer[a] *
               cos(tempd +
                   pIDX->m_cst_speeds[a] * ((long)(t - pIDX->epoch) + pIDX->pref_sta_data->meridian) +
                   pIDX->m_cst_epochs[a][year-pIDX->first_year] - pIDX->pref_sta_data->epoch[a]);
        for (b = deriv; b > 0; b--)
            term *= pIDX->m_cst_speeds[a];
        dt_tide += term;
    }
    return dt_tide;
}

/* blend_weight (double x, int deriv)
 *
 * Returns the value nth derivative of the "blending function" w(x):
 *
 *   w(x) =  0,     for x <= -1
 *
 *   w(x) =  1/2 + (15/16) x - (5/8) x^3 + (3/16) x^5,
 *                  for  -1 < x < 1
 *
 *   w(x) =  1,     for x >= 1
 *
 * This function has the following desirable properties:
 *
 *    w(x) is exactly either 0 or 1 for |x| > 1
 *
 *    w(x), as well as its first two derivatives are continuous for all x.
 */
static double
blend_weight (double x, int deriv)
{
    double x2 = x * x;

    if (x2 >= 1.0)
        return deriv == 0 && x > 0.0 ? 1.0 : 0.0;

    switch (deriv) {
    case 0:
        return ((3.0 * x2 -10.0) * x2 + 15.0) * x / 16.0 + 0.5;
    case 1:
        return ((x2 - 2.0) * x2 + 1.0) * (15.0/16.0);
    case 2:
        return (x2 - 1.0) * x * (15.0/4.0);
    }
    return(0); // mgh+ to get rid of compiler warning
}

/*
 * This function does the actual "blending" of the tide
 * and its derivatives.
 */
double blend_tide (time_t t, unsigned int deriv, int first_year, double blend, IDX_entry *pIDX)
{
    double        fl[TIDE_MAX_DERIV + 1];
    double        fr[TIDE_MAX_DERIV + 1];
    double *      fp      = fl;
    double        w[TIDE_MAX_DERIV + 1];
    double        fact = 1.0;
    double        f;
    unsigned int  n;


    /*
     * If we are already happy_new_year()ed into one of the two years
     * of interest, compute that years tide values first.
     */
    int year = yearoftimet(t);
    if (year == first_year + 1)
        fp = fr;
    else if (year != first_year)
        happy_new_year(pIDX, first_year);
    for (n = 0; n <= deriv; n++)
        fp[n] = _time2dt_tide(t, n, pIDX);

    /*
     * Compute tide values for the other year of interest,
     *  and the needed values of w(x) and its derivatives.
     */
    if (fp == fl)
    {
        happy_new_year(pIDX, first_year + 1);
        fp = fr;
    }
    else
    {
        happy_new_year(pIDX, first_year);
        fp = fl;
    }
    for (n = 0; n <= deriv; n++)
    {
        fp[n] = _time2dt_tide(t, n, pIDX);
        w[n] = blend_weight(blend, n);
    }

    /*
     * Do the blending.
     */
    f = fl[deriv];
    for (n = 0; n <= deriv; n++)
    {
        f += fact * w[n] * (fr[deriv-n] - fl[deriv-n]);
        fact *= (double)(deriv - n)/(n+1) * (1.0/TIDE_BLEND_TIME);
    }
    return f;
}

double  time2dt_tide (time_t t, int deriv, IDX_entry *pIDX)
{
    int           new_year;
    time_t tt = time(NULL);
    int yott = ((gmtime (&tt))->tm_year) + 1900;
    new_year = yott;                    //= yearoftimet(t);

    /* Make sure our values of next_epoch and epoch are up to date. */
    if (new_year != s_this_year)
    {
        if (new_year + 1 < pIDX->first_year + pIDX->num_epochs)
        {
            set_epoch(pIDX, new_year + 1);
            s_next_epoch = pIDX->epoch;
        }
        else
            s_next_epoch = TIDE_BAD_TIME;

        happy_new_year(pIDX, s_this_year = new_year);
        s_this_epoch = pIDX->epoch;
    }


    /*
     * If we're close to either the previous or the next
     * new years we must blend the two years tides.
     */
    if (t - s_this_epoch <= TIDE_BLEND_TIME && s_this_year > pIDX->first_year)
        return blend_tide(t, deriv,
                          s_this_year - 1,
                          (double)(t - s_this_epoch)/TIDE_BLEND_TIME,
                          pIDX );
    else if (s_next_epoch - t <= TIDE_BLEND_TIME
             && s_this_year + 1 < pIDX->first_year + pIDX->num_epochs)
        return blend_tide(t, deriv,
                          s_this_year,
                          -(double)(s_next_epoch - t)/TIDE_BLEND_TIME,
                          pIDX );

    /*
     * Else, we're far enough from newyears to ignore the blending.
     */
    if (s_year != new_year)
        happy_new_year(pIDX, new_year);

    return _time2dt_tide(t, deriv, pIDX);
}



/* Figure out max amplitude over all the years in the node factors table. */
/* This function by Geoffrey T. Dairiki */
void figure_max_amplitude (IDX_entry *pIDX)
{
    int       i, a;

    if (pIDX->max_amplitude == 0.0) {
        for (i = 0; i < pIDX->num_nodes; i++) {
            double year_amp = 0.0;

            for (a=0; a < pIDX->num_csts; a++)
                year_amp += pIDX->pref_sta_data->amplitude[a] * pIDX->m_cst_nodes[a][i];
            if (year_amp > pIDX->max_amplitude)
                pIDX->max_amplitude = year_amp;
        }
    }
}

/* Figure out normalized multipliers for constituents for a particular year. */
void figure_multipliers (IDX_entry *pIDX, int year)
{
    int a;

    figure_max_amplitude( pIDX );
    for (a = 0; a < pIDX->num_csts; a++) {
        pIDX->m_work_buffer[a] = pIDX->pref_sta_data->amplitude[a] * pIDX->m_cst_nodes[a][year-pIDX->first_year] / pIDX->max_amplitude;  // BOGUS_amplitude?
    }
}


/* This idiotic function is needed by the new tm2gmt. */
#define compare_int(a,b) (((int)(a))-((int)(b)))
int compare_tm (struct tm *a, struct tm *b) {
    int temp;
    /* printf ("A is %d:%d:%d:%d:%d:%d   B is %d:%d:%d:%d:%d:%d\n",
     *      a->tm_year+1900, a->tm_mon+1, a->tm_mday, a->tm_hour,
     *      a->tm_min, a->tm_sec,
     *      b->tm_year+1900, b->tm_mon+1, b->tm_mday, b->tm_hour,
     *      b->tm_min, b->tm_sec); */

    temp = compare_int (a->tm_year, b->tm_year);
    if (temp)
        return temp;
    temp = compare_int (a->tm_mon, b->tm_mon);
    if (temp)
        return temp;
    temp = compare_int (a->tm_mday, b->tm_mday);
    if (temp)
        return temp;
    temp = compare_int (a->tm_hour, b->tm_hour);
    if (temp)
        return temp;
    temp = compare_int (a->tm_min, b->tm_min);
    if (temp)
        return temp;
    return compare_int (a->tm_sec, b->tm_sec);
}

/* Convert a struct tm in GMT back to a time_t.  isdst is ignored, since
 *   it never should have been needed by mktime in the first place.
 */
time_t tm2gmt (struct tm *ht)
{
    time_t guess, newguess, thebit;
    int loopcounter, compare;
    struct tm *gt;

    guess = 0;
    loopcounter = (sizeof(time_t) * 8)-1;
    thebit = ((time_t)1) << (loopcounter-1);

    /* For simplicity, I'm going to insist that the time_t we want is
     *       positive.  If time_t is signed, skip the sign bit.
     */
    if ((signed long)thebit < (time_t)(0)) {
        /* You can't just shift thebit right because it propagates the sign bit. */
        loopcounter--;
        thebit = ((time_t)1) << (loopcounter-1);
    }

    for (; loopcounter; loopcounter--) {
        newguess = guess | thebit;
        gt = gmtime(&newguess);
        if(NULL != gt)
        {
            compare = compare_tm (gt, ht);
            if (compare <= 0)
                guess = newguess;
        }
        thebit >>= 1;
    }

    return guess;
}

int yearoftimet (time_t t)
{
    return ((gmtime (&t))->tm_year) + 1900;
}


/* Calculate time_t of the epoch. */
void set_epoch (IDX_entry *pIDX, int year)
{
    struct tm ht;

    ht.tm_year = year - 1900;
    ht.tm_sec = ht.tm_min = ht.tm_hour = ht.tm_mon = 0;
    ht.tm_mday = 1;
    pIDX->epoch = tm2gmt (&ht);
}

/* Re-initialize for a different year */
void happy_new_year (IDX_entry *pIDX, int new_year)
{
    s_year = new_year;
    figure_multipliers ( pIDX, new_year );
    set_epoch ( pIDX, new_year );
}



//      Station_Data Implementation

Station_Data::Station_Data()
{
    station_name = NULL;
    amplitude = NULL;
    epoch = NULL;
}

Station_Data::~Station_Data()
{
    free( station_name );
    free( amplitude );
    free( epoch );
}


//      IDX_entry Implementation
IDX_entry::IDX_entry()
{
    max_amplitude = 0.;

}

IDX_entry::~IDX_entry()
{
}

//      TCMgr Implementation
TCMgr::TCMgr()
{
}

TCMgr::~TCMgr()
{
    PurgeData();
}

void TCMgr::PurgeData()
{
    //  Index entries are owned by the data sources
    //  so we need to clear them from the combined list without
    //  deleting them
    while(m_Combined_IDX_array.GetCount()) {
        m_Combined_IDX_array.Detach(0);
    }

    //  Delete all the data sources
    m_source_array.Clear();
}

TC_Error_Code TCMgr::LoadDataSources(wxArrayString &sources)
{
    PurgeData();

    //  Take a copy of dataset file name array
    m_sourcefile_array.Clear();
    m_sourcefile_array = sources;

    //  Arrange for the index array to begin counting at "one"
    m_Combined_IDX_array.Add((IDX_entry *)(NULL));

    for(unsigned int i=0 ; i < sources.GetCount() ; i++) {
        TCDataSource *s = new TCDataSource;
        TC_Error_Code r = s->LoadData(sources.Item(i));
        if(r != TC_NO_ERROR) {
            wxString msg;
            msg.Printf(_T("   Error loading Tide/Currect data source %s "), sources.Item(i).c_str());
            if( r == TC_FILE_NOT_FOUND) 
                msg += _T("Error Code: TC_FILE_NOT_FOUND");
            else {
                wxString msg1;
                msg1.Printf(_T("Error code: %d"), r);
                msg += msg1;
            }
            wxLogMessage(msg);
            delete s;
        }
        else {
            m_source_array.Add(s);

            for( int k=0 ; k < s->GetMaxIndex() ; k++ ) {
                IDX_entry *pIDX = s->GetIndexEntry(k);
                m_Combined_IDX_array.Add(pIDX);
            }
        }
    }

    bTCMReady = true;

    return  TC_NO_ERROR ;
}

IDX_entry *TCMgr::GetIDX_entry(int index)
{
    if((unsigned int)index < m_Combined_IDX_array.GetCount())
        return &m_Combined_IDX_array.Item(index);
    else
        return NULL;
}


bool TCMgr::GetTideOrCurrent(time_t t, int idx, float &tcvalue, float& dir)
{
    //    Return a sensible value of 0,0 by default
    dir = 0;
    tcvalue = 0;

    //    Load up this location data
    IDX_entry *pIDX = &m_Combined_IDX_array.Item( idx );    // point to the index entry

    if( !pIDX ) {
        dir = 0;
        tcvalue = 0;
        return false;
    }

    if( !pIDX->IDX_Useable ) {
        dir = 0;
        tcvalue = 0;
        return(false);                                        // no error, but unuseable
    }

    if(pIDX->pDataSource) {
        if(pIDX->pDataSource->LoadHarmonicData(pIDX) != TC_NO_ERROR)
            return false;
    }

    pIDX->max_amplitude = 0.0;                // Force multiplier re-compute
    time_t tt = time(NULL);
    int yott = ((gmtime (&tt))->tm_year) + 1900;
    happy_new_year (pIDX, yott);              //Calculate new multipliers

    //    Finally, calculate the tide/current

    double level = time2asecondary (t, pIDX);
    if(level >= 0)
        dir = pIDX->IDX_flood_dir;
    else
        dir = pIDX->IDX_ebb_dir;

    tcvalue = level;

    return(true); // Got it!
}

bool TCMgr::GetTideOrCurrent15(time_t t, int idx, float &tcvalue, float& dir, bool &bnew_val)
{
    int ret;
    IDX_entry *pIDX = &m_Combined_IDX_array.Item( idx );             // point to the index entry

    if( !pIDX ) {
        dir = 0;
        tcvalue = 0;
        return false;
    }

    //    Figure out this computer timezone minute offset
    wxDateTime this_now = wxDateTime::Now();
    wxDateTime this_gmt = this_now.ToGMT();
    wxTimeSpan diff = this_gmt.Subtract(this_now);
    int diff_mins = diff.GetMinutes();

    int station_offset = pIDX->IDX_time_zone;
    if(this_now.IsDST())
        station_offset += 60;
    int corr_mins = station_offset - diff_mins;

    wxDateTime today_00 = wxDateTime::Today();
    int t_today_00 = today_00.GetTicks();
    int t_today_00_at_station = t_today_00 - (corr_mins * 60);

    int t_at_station = this_gmt.GetTicks() - (station_offset * 60) + (corr_mins * 60);

    int t_mins = (t_at_station - t_today_00_at_station) / 60;
    int t_15s = t_mins / 15;

    if(pIDX->Valid15)                               // valid data available
    {

        int tref1 = t_today_00_at_station + t_15s * 15 * 60;
        if(tref1 == pIDX->Valid15)
        {
            tcvalue = pIDX->Value15;
            dir = pIDX->Dir15;
            bnew_val = false;
            return pIDX->Ret15;
        }
        else
        {
            int tref = t_today_00_at_station + t_15s * 15 * 60;
            ret = GetTideOrCurrent(tref, idx, tcvalue, dir);

            pIDX->Valid15 = tref;
            pIDX->Value15 = tcvalue;
            pIDX->Dir15 = dir;
            pIDX->Ret15 = !(ret == 0);
            bnew_val = true;

            return !(ret == 0);
        }
    }

    else {
        int tref = t_today_00_at_station + t_15s * 15 * 60;
        ret = GetTideOrCurrent(tref, idx, tcvalue, dir);

        pIDX->Valid15 = tref;
        pIDX->Value15 = tcvalue;
        pIDX->Dir15 = dir;
        pIDX->Ret15 = !(ret == 0);
        bnew_val = true;
    }

    return !(ret == 0);

}

bool TCMgr::GetTideFlowSens(time_t t, int sch_step, int idx, float &tcvalue_now, float &tcvalue_prev, bool &w_t)
{
    //    Return a sensible value of 0 by default
    tcvalue_now = 0;
    tcvalue_prev = 0;
    w_t = false;


    //    Load up this location data
    IDX_entry *pIDX = &m_Combined_IDX_array.Item( idx );             // point to the index entry

    if( !pIDX )
        return false;

    if( !pIDX->IDX_Useable )
        return false;                                        // no error, but unuseable

    if(pIDX->pDataSource) {
        if(pIDX->pDataSource->LoadHarmonicData(pIDX) != TC_NO_ERROR)
            return false;
    }

    pIDX->max_amplitude = 0.0;                // Force multiplier re-compute
    time_t tt = time(NULL);
    int yott = ((gmtime (&tt))->tm_year) + 1900;
    happy_new_year (pIDX, yott);              //Force new multipliers

    //    Finally, process the tide flow sens

    tcvalue_now = time2asecondary (t , pIDX);
    tcvalue_prev = time2asecondary (t + sch_step , pIDX);

    w_t = tcvalue_now > tcvalue_prev;           // w_t = true --> flood , w_t = false --> ebb

    return true;
}

void TCMgr::GetHightOrLowTide(time_t t, int sch_step_1, int sch_step_2, float tide_val ,bool w_t , int idx, float &tcvalue, time_t &tctime)
{
    //    Return a sensible value of 0,0 by default
    tcvalue = 0;
    tctime = t;

    //    Load up this location data
    IDX_entry *pIDX = &m_Combined_IDX_array.Item( idx );             // point to the index entry

    if( !pIDX )
        return;

    if( !pIDX->IDX_Useable )
        return;                                        // no error, but unuseable

    if(pIDX->pDataSource) {
        if(pIDX->pDataSource->LoadHarmonicData(pIDX) != TC_NO_ERROR)
            return;
    }

    pIDX->max_amplitude = 0.0;                // Force multiplier re-compute
    time_t tt = time(NULL);
    int yott = ((gmtime (&tt))->tm_year) + 1900;
    happy_new_year (pIDX, yott);//Force new multipliers

    // Finally, calculate the Hight and low tides
    double newval = tide_val;
    double oldval = ( w_t ) ? newval - 1: newval + 1 ;
    int j = 0 ;
    int k = 0 ;
    int ttt = 0 ;
    while ( (newval > oldval) == w_t )                  //searching each ten minute
    {
        j++;
        oldval = newval;
        ttt = t + ( sch_step_1 * j );
        newval = time2asecondary (ttt, pIDX);
    }
    oldval = ( w_t ) ? newval - 1: newval + 1 ;
    while ( (newval > oldval) == w_t )                  // searching back each minute
    {
        oldval = newval ;
        k++;
        ttt = t +  ( sch_step_1 * j ) - ( sch_step_2 * k ) ;
        newval = time2asecondary (ttt, pIDX);
    }
    tcvalue = newval;
    tctime = ttt + sch_step_2 ;

}

int TCMgr::GetStationTimeOffset(IDX_entry *pIDX)
{
    return pIDX->IDX_time_zone;
}

int TCMgr::GetNextBigEvent(time_t *tm, int idx)
{
    float tcvalue[1];
    float dir;
    bool ret;
    double p, q;
    int flags = 0, slope = 0;
    ret = GetTideOrCurrent(*tm, idx, tcvalue[0],  dir);
    p = tcvalue[0];
    *tm += 60;
    ret = GetTideOrCurrent(*tm, idx, tcvalue[0],  dir);
    q = tcvalue[0];
    *tm += 60;
    if (p < q)
        slope = 1;
    while (1) {
        if ((slope == 1 && q < p) || (slope == 0 && p < q)) {
            /* Tide event */
            flags |= (1 << slope);
        }
        if (flags) {
            *tm -= 60;
            if (flags < 4)
                *tm -= 60;
            return flags;
        }
        p = q;
        ret = GetTideOrCurrent(*tm, idx, tcvalue[0],  dir);
        q = tcvalue[0];
        *tm += 60;
    }
    return 0;
}

int TCMgr::GetStationIDXbyName(wxString prefix, double xlat, double xlon )
{
    IDX_entry *lpIDX;
    int jx = 0;
    wxString locn;
    double distx = 100000.;

    int jmax = Get_max_IDX();

    for ( int j=1 ; j<Get_max_IDX() +1 ; j++ ) {
        lpIDX = GetIDX_entry ( j );
        char type = lpIDX->IDX_type;             // Entry "TCtcIUu" identifier
        wxString locnx ( lpIDX->IDX_station_name, wxConvUTF8 );

        if ( (( type == 't' ) ||  ( type == 'T' ) )   // only Tides
                && (locnx.StartsWith(prefix))) {
            double brg, dist;
            DistanceBearingMercator(xlat, xlon, lpIDX->IDX_lat, lpIDX->IDX_lon, &brg, &dist);
            if (dist < distx) {
                distx = dist;
                jx = j;
            }
        }
    } // end for loop
    //} // end if @~~ found in WP
    return(jx);
}


int TCMgr::GetStationIDXbyNameType(wxString prefix, double xlat, double xlon, char type)
{
    IDX_entry *lpIDX;
    int jx = 0;
    wxString locn;
    double distx = 100000.;

    // if (prp->m_MarkName.Find(_T("@~~")) != wxNOT_FOUND) {
    //tide_form = prp->m_MarkName.Mid(prp->m_MarkName.Find(_T("@~~"))+3);
    int jmax = Get_max_IDX();

    for ( int j=1 ; j<Get_max_IDX() +1 ; j++ ) {
        lpIDX = GetIDX_entry ( j );
        char typep = lpIDX->IDX_type;             // Entry "TCtcIUu" identifier
        wxString locnx ( lpIDX->IDX_station_name, wxConvUTF8 );

        if ( ( type == typep ) && (locnx.StartsWith(prefix))) {
            double brg, dist;
            DistanceBearingMercator(xlat, xlon, lpIDX->IDX_lat, lpIDX->IDX_lon, &brg, &dist);
            if (dist < distx) {
                distx = dist;
                jx = j;
            }
        }
    } // end for loop
    return(jx);
}




//      TCDataFactory implementation

TCDataSource::TCDataSource()
{
    m_pfactory = NULL;
    pTCDS_Ascii_Harmonic = NULL;
    pTCDS_Binary_Harmonic = NULL;

}

TCDataSource::~TCDataSource()
{
    wxLogMessage(_T("UnLoading Tide/Current data source: %s"), m_data_source_path.c_str());

    delete pTCDS_Ascii_Harmonic;
    delete pTCDS_Binary_Harmonic;
}

TC_Error_Code TCDataSource::LoadData(wxString &data_file_path)
{
    m_data_source_path = data_file_path;
    wxLogMessage(_T("Loading Tide/Current data source: %s"), m_data_source_path.c_str());

    wxFileName fname(data_file_path);

    if(!fname.FileExists()) return TC_FILE_NOT_FOUND;

    if(fname.GetExt() == _T("IDX")) {
        TCDS_Ascii_Harmonic *pdata = new TCDS_Ascii_Harmonic;
        m_pfactory = dynamic_cast<TCDataFactory*>(pdata);
        pTCDS_Ascii_Harmonic = pdata;
    }
    else if(fname.GetExt() == _T("tcd") || fname.GetExt() == _T("TCD")) {
        TCDS_Binary_Harmonic *pdata = new TCDS_Binary_Harmonic;
        m_pfactory = dynamic_cast<TCDataFactory*>(pdata);
        pTCDS_Binary_Harmonic = pdata;
    }

    TC_Error_Code err_code = m_pfactory->LoadData(data_file_path);

    //  Mark the index entries indiviually with owner
    unsigned int max_index = GetMaxIndex();
    for(unsigned int i=0 ; i < max_index ; i++) {
        IDX_entry *pIDX = GetIndexEntry( i );
        if(pIDX)
            pIDX->pDataSource = this;
    }

    return err_code;
}

int TCDataSource::GetMaxIndex(void)
{
    if(m_pfactory)
        return m_pfactory->GetMaxIndex();
    else
        return 0;
}

IDX_entry *TCDataSource::GetIndexEntry(int n_index)
{
    if(m_pfactory) {
        if(n_index < m_pfactory->GetMaxIndex())
            return m_pfactory->GetIndexEntry(n_index);
        else
            return NULL;
    }
    else
        return NULL;
}

TC_Error_Code TCDataSource::LoadHarmonicData(IDX_entry *pIDX)
{
    switch( pIDX->source_data_type)
    {
    case SOURCE_TYPE_ASCII_HARMONIC:
        return pTCDS_Ascii_Harmonic->LoadHarmonicData(pIDX);
        break;

    case SOURCE_TYPE_BINARY_HARMONIC:
        return pTCDS_Binary_Harmonic->LoadHarmonicData(pIDX);
        break;

    default:
        return TC_GENERIC_ERROR;
    }
}




//      TCDataFactory implementation

TCDataFactory::TCDataFactory()
{
}

TCDataFactory::~TCDataFactory()
{
}


//      TCDS_Ascii_Harmonic implementation

TCDS_Ascii_Harmonic::TCDS_Ascii_Harmonic()
{
    //  Initialize member variables
    m_IndexFile = NULL;

    m_cst_speeds = NULL;
    m_cst_nodes = NULL;
    m_cst_epochs = NULL;

    num_IDX = 0;
    num_nodes = 0;
    num_csts = 0;
    num_epochs = 0;

    //  Build the units array
    known_units[0].name = (char *) malloc(strlen("feet") +1);
    strcpy(known_units[0].name, "feet");
    known_units[0].abbrv = (char *) malloc(strlen("ft") +1);
    strcpy(known_units[0].abbrv, "ft");
    known_units[0].type = LENGTH;
    known_units[0].conv_factor = 0.3048;

    known_units[1].name = (char *) malloc(strlen("meters") +1);
    strcpy(known_units[1].name, "meters");
    known_units[1].abbrv = (char *) malloc(strlen("m") +1);
    strcpy(known_units[1].abbrv, "m");
    known_units[1].type = LENGTH;
    known_units[1].conv_factor = 1.0;

    known_units[2].name = (char *) malloc(strlen("knots") +1);
    strcpy(known_units[2].name, "knots");
    known_units[2].abbrv = (char *) malloc(strlen("ky") +1);
    strcpy(known_units[2].abbrv, "kt");
    known_units[2].type = VELOCITY;
    known_units[2].conv_factor = 1.0;

    known_units[3].name = (char *) malloc(strlen("knots^2") +1);
    strcpy(known_units[3].name, "knots^2");
    known_units[3].abbrv = (char *) malloc(strlen("kt^2") +1);
    strcpy(known_units[3].abbrv, "kt^2");
    known_units[3].type = BOGUS;
    known_units[3].conv_factor = 1.0;
}

TCDS_Ascii_Harmonic::~TCDS_Ascii_Harmonic()
{
    free_data();

    m_msd_array.Clear();
}

TC_Error_Code TCDS_Ascii_Harmonic::LoadData(wxString &data_file_path)
{
    if(m_IndexFile) IndexFileIO( IFF_CLOSE, 0 );

    m_indexfile_name = data_file_path;

    TC_Error_Code error_return = init_index_file();
    if(error_return != TC_NO_ERROR)
        return error_return;

    wxFileName f(data_file_path);
    m_harmfile_name = f.GetPath( wxPATH_GET_SEPARATOR | wxPATH_GET_VOLUME );
    m_harmfile_name += f.GetName();
    error_return = LoadHarmonicConstants(m_harmfile_name);

    //  Mark the index entries individually with invariant harmonic constants
    unsigned int max_index = GetMaxIndex();
    for(unsigned int i=0 ; i < max_index ; i++) {
        IDX_entry *pIDX = GetIndexEntry( i );
        if(pIDX) {
            pIDX->num_nodes = num_nodes;
            pIDX->num_csts = num_csts;
            pIDX->num_epochs = num_epochs;
            pIDX->m_cst_speeds = m_cst_speeds;
            pIDX->m_cst_nodes = m_cst_nodes;
            pIDX->m_cst_epochs = m_cst_epochs;
            pIDX->first_year = m_first_year;
            pIDX->m_work_buffer = m_work_buffer;
        }
    }

    return error_return;
}


IDX_entry *TCDS_Ascii_Harmonic::GetIndexEntry(int n_index)
{
    return &m_IDX_array.Item(n_index);
}


TC_Error_Code TCDS_Ascii_Harmonic::init_index_file()
{
    long int xref_start=0;
    int doing_xref=0;

    num_IDX=0;

    m_abbreviation_array.Clear();
    m_IDX_array.Clear();
    //   free_harmonic_file_list();
    int have_index = 0;
    int index_in_memory = 0;

    if (IndexFileIO(IFF_OPEN, 0)) {
        while (IndexFileIO(IFF_READ, 0)) {
            if ((index_line_buffer[0] == '#') || (index_line_buffer[0] <= ' '));  // Skip comment lines
            else if (!have_index && !xref_start) {
                if (!strncmp(index_line_buffer, "XREF", 4))
                    xref_start = IndexFileIO(IFF_TELL, 0);
            }
            else if (!have_index && !strncmp(index_line_buffer, "*END*", 5)) {
                if (m_abbreviation_array.GetCount() == 0) {
                    IndexFileIO(IFF_CLOSE, 0);
                    return(TC_INDEX_FILE_CORRUPT); // missing at least some data so no valid index
                }
                // We're done with abbreviation list (and no errors)
                else have_index = 1;
            } // found *END* of cross reference

            else if (!have_index && xref_start) {
                wxString line( index_line_buffer, wxConvUTF8 );

                abbr_entry *entry  = new abbr_entry;

                wxStringTokenizer tkz(line, _T(" "));
                wxString token = tkz.GetNextToken();
                if(token.IsSameAs(_T("REGION"), FALSE))
                    entry->type = REGION;
                else if(token.IsSameAs(_T("COUNTRY"), FALSE))
                    entry->type = COUNTRY;
                else if(token.IsSameAs(_T("STATE"), FALSE))
                    entry->type = STATE;

                token = tkz.GetNextToken();
                entry->short_s = token;

                entry->long_s = line.Mid(tkz.GetPosition()).Strip();

                m_abbreviation_array.Add(entry);

            }

            else if (have_index && (strchr("TtCcIUu", index_line_buffer[0]))) {
                // Load index file data .
                num_IDX++; // Keep counting entries for harmonic file stuff
                IDX_entry *pIDX = new IDX_entry;
                pIDX->source_data_type = SOURCE_TYPE_ASCII_HARMONIC;
                pIDX->pDataSource = NULL;

                index_in_memory   = TRUE;
                pIDX->IDX_rec_num = num_IDX;
                pIDX->Valid15 = 0;

                if(TC_NO_ERROR != build_IDX_entry(pIDX ) ) {
                }

                m_IDX_array.Add(pIDX);
            }

#if 0
            else if (have_index && (index_line_buffer[0] == 'H')) {
                // This is a new harmonic file name.
                sscanf(index_line, "Harmonic %s", s1);
                pHarmonic = harmonic_file_list;
                while (pHarmonic && pHarmonic->next)
                    pHarmonic = (harmonic_file_entry *)pHarmonic->next;
                pHarmonic_prev = pHarmonic;
                pHarmonic = (harmonic_file_entry *)malloc(sizeof(harmonic_file_entry));
                if (NULL == pHarmonic) {
                    //               no_mem_msg();
                    free_harmonic_file_list();
                }
                else {
                    if (!harmonic_file_list)
                        harmonic_file_list = pHarmonic;
                    else pHarmonic_prev->next = pHarmonic;
                    pHarmonic->next = NULL;
                    pHarmonic->rec_start = num_IDX;
                    if (allocate_copy_string(&pHarmonic->name,s1)) {
                        //                  no_mem_msg();
                        free_harmonic_file_list();
                    }
                }
            }
#endif
        } // while (more file)
        if (index_in_memory) IndexFileIO(IFF_CLOSE, 0); // All done with file
    } // index file can't be opened
    //   if (hwndBusy) DestroyWindow(hwndBusy);

//   max_IDX = num_IDX;
    return( TC_NO_ERROR );
} // init_index_file()




// ----------------------------------
//   Decode an index data line into an IDX_entry
// ----------------------------------

TC_Error_Code TCDS_Ascii_Harmonic::build_IDX_entry(IDX_entry *pIDX )
{
    int TZHr, TZMin ;
    char stz[80];

    pIDX->pref_sta_data = NULL;                     // no reference data yet
    pIDX->IDX_Useable = 1;                          // but assume data is OK

    pIDX->IDX_tzname = NULL;
    if (7 != sscanf( index_line_buffer, "%c%s%lf%lf%d:%d%*c%[^\r\n]",
                     &pIDX->IDX_type,&pIDX->IDX_zone[0],&pIDX->IDX_lon,&pIDX->IDX_lat,&TZHr,&TZMin,
                     &pIDX->IDX_station_name[0])) return(TC_INDEX_ENTRY_BAD);

    pIDX->IDX_time_zone = TZHr*60 + TZMin;

    if (strchr("tcUu",index_line_buffer[0])) { // Substation so get second line of info
        IndexFileIO(IFF_READ, 0);

        if(index_line_buffer[0] == '^')                  // Opencpn special
        {
            if (11 != sscanf(index_line_buffer, "%*c%d %f %f %d %f %f %d %d %d %d%*c%[^\r\n]",
                             &pIDX->IDX_ht_time_off, &pIDX->IDX_ht_mpy, &pIDX->IDX_ht_off,
                             &pIDX->IDX_lt_time_off, &pIDX->IDX_lt_mpy, &pIDX->IDX_lt_off,
                             &pIDX->IDX_sta_num, &pIDX->IDX_flood_dir, &pIDX->IDX_ebb_dir,
                             &pIDX->IDX_ref_file_num, pIDX->IDX_reference_name))
                return(TC_INDEX_ENTRY_BAD);

            if(abs(pIDX->IDX_ht_time_off) > 1000)           // useable?
                pIDX->IDX_Useable = 0;

            if(abs(pIDX->IDX_flood_dir) > 360)           // useable?
                pIDX->IDX_Useable = 0;
            if(abs(pIDX->IDX_ebb_dir) > 360)           // useable?
                pIDX->IDX_Useable = 0;

            //    Fix up the secondaries which are identical to masters
            if(pIDX->IDX_ht_mpy == 0.0)
                pIDX->IDX_ht_mpy = 1.0;
            if(pIDX->IDX_lt_mpy == 0.0)
                pIDX->IDX_lt_mpy = 1.0;

        }
        else
        {
            if (9 != sscanf(index_line_buffer, "%*c%d %f %f %d %f %f %d %d%*c%[^\r\n]",
                            &pIDX->IDX_ht_time_off, &pIDX->IDX_ht_mpy, &pIDX->IDX_ht_off,
                            &pIDX->IDX_lt_time_off, &pIDX->IDX_lt_mpy, &pIDX->IDX_lt_off,
                            &pIDX->IDX_sta_num, &pIDX->IDX_ref_file_num, pIDX->IDX_reference_name))
            {
                // Had an error so try alternate with timezone name before ref file number
                if (10 != sscanf(index_line_buffer, "%*c%d %f %f %d %f %f %d %s %d%*c%[^\r\n]",
                                 &pIDX->IDX_ht_time_off, &pIDX->IDX_ht_mpy, &pIDX->IDX_ht_off,
                                 &pIDX->IDX_lt_time_off, &pIDX->IDX_lt_mpy, &pIDX->IDX_lt_off,
                                 &pIDX->IDX_sta_num, stz, &pIDX->IDX_ref_file_num, pIDX->IDX_reference_name))
                    return(TC_INDEX_ENTRY_BAD);
            }


            if (NULL!=(pIDX->IDX_tzname = (char *)malloc(strlen(stz)+1)))
                strcpy(pIDX->IDX_tzname, stz);
        }           // else


        //  We only consider 1 reference file per index file
        pIDX->IDX_ref_file_num = 0;
        /*
                if (pIDX->IDX_ref_file_num <= 0)
                {   // Find harmonic reference file number
                    pIDX->IDX_ref_file_num= 0;
                    // Find reference station in index, if no index, it had better be in the first one
                    pIDXh = pIDX_first;
                    while (pIDXh!=NULL && strcmp(pIDXh->IDX_reference_name,pIDX->IDX_reference_name))
                        pIDXh = (IDX_entry *)pIDXh->IDX_next;

                    // Copy reference station harmonic file number
                    if (pIDXh!=NULL)
                        pIDX->IDX_ref_file_num = pIDXh->IDX_ref_file_num;
                }
        */
    }

    else
    {   // Reference stations have no offsets
        pIDX->IDX_ht_time_off = pIDX->IDX_lt_time_off = 0;
        pIDX->IDX_ht_mpy      = pIDX->IDX_lt_mpy = 1.0;
        pIDX->IDX_ht_off      = pIDX->IDX_lt_off = 0.0;
        pIDX->IDX_sta_num     = 0;
        strcpy(pIDX->IDX_reference_name, pIDX->IDX_station_name);

        /*
                pIDX->IDX_ref_file_num= 0;
                pHarmonic = harmonic_file_list;
                while (pHarmonic && (pHarmonic->rec_start <= pIDX->IDX_rec_num))
                {
                    pHarmonic = (harmonic_file_entry *)pHarmonic->next;
                    pIDX->IDX_ref_file_num++;
                }
        */
    }

    if( pIDX->IDX_ht_time_off ||
            pIDX->IDX_ht_off != 0.0 ||
            pIDX->IDX_lt_off != 0.0 ||
            pIDX->IDX_ht_mpy != 1.0 ||
            pIDX->IDX_lt_mpy != 1.0)
        pIDX->have_offsets = 1;

    return(TC_NO_ERROR);
}


//    Load the Harmonic Constant Invariants
TC_Error_Code TCDS_Ascii_Harmonic::LoadHarmonicConstants(wxString &data_file_path)
{
    FILE *fp;
    char linrec[linelen];
    char junk[80];
    int a, b;

    free_data();

    fp = fopen (data_file_path.mb_str(), "r");
    if (NULL == fp)
        return TC_FILE_NOT_FOUND;

    read_next_line (fp, linrec, 0);
    sscanf (linrec, "%d", &num_csts);

    m_cst_speeds = (double *) malloc (num_csts * sizeof (double));
    m_work_buffer = (double *) malloc (num_csts * sizeof (double));

    /* Load constituent speeds */
    for (a=0; a<num_csts; a++) {
        read_next_line (fp, linrec, 0);
        sscanf (linrec, "%s %lf", junk, &(m_cst_speeds[a]));
        m_cst_speeds[a] *= M_PI / 648000; /* Convert to radians per second */
    }

    /* Get first year for nodes and epochs */
    read_next_line (fp, linrec, 0);
    sscanf (linrec, "%d", &m_first_year);

    /* Load epoch table */
    read_next_line (fp, linrec, 0);
    sscanf (linrec, "%d", &num_epochs);

    m_cst_epochs = (double **) malloc (num_csts * sizeof (double *));
    for (int i=0; i<num_csts; i++)
        m_cst_epochs[i] = (double *) malloc (num_epochs * sizeof (double));

    for (int i=0; i<num_csts; i++)
    {
        if(EOF == fscanf (fp, "%s", linrec))
            return TC_HARM_FILE_CORRUPT;
        for (int b=0; b<num_epochs; b++)
        {
            if(EOF == fscanf (fp, "%lf", &(m_cst_epochs[i][b])))
                return TC_HARM_FILE_CORRUPT;
            m_cst_epochs[i][b] *= M_PI / 180.0;
        }
    }


    /* Sanity check */
    if(EOF == fscanf (fp, "%s", linrec))
        return TC_HARM_FILE_CORRUPT;
    skipnl (fp);

    /* Load node factor table */
    read_next_line (fp, linrec, 0);
    sscanf (linrec, "%d", &num_nodes);

    m_cst_nodes = (double **) malloc (num_csts * sizeof (double *));
    for (int a=0; a<num_csts; a++)
        m_cst_nodes[a] = (double *) malloc (num_nodes * sizeof (double));

    for (int a=0; a<num_csts; a++) {
        int ignore = fscanf (fp, "%s", linrec);
        for (b=0; b<num_nodes; b++)
            ignore = fscanf (fp, "%lf", &(m_cst_nodes[a][b]));
    }

    fclose(fp);

    return TC_NO_ERROR;
}


TC_Error_Code TCDS_Ascii_Harmonic::LoadHarmonicData(IDX_entry *pIDX)
{
    Station_Data *psd = NULL;

    //    Look in the index first
    if(pIDX->pref_sta_data)
        return TC_NO_ERROR;         //easy


    // Try the member array of "already-looked-at" master stations
    for(unsigned int i=0 ; i < m_msd_array.GetCount() ; i++)
    {
        psd = &m_msd_array.Item(i);
        //    In the following comparison, it is allowed that the sub-station reference_name may be
        //          a pre-subset of the master station name.
        //          e.g  IDX_refence_name:  The Narrows midchannel New York
        //                            as found in HARMONIC.IDX
        //                 psd_station_name:      The Narrows, Midchannel, New York Harbor, New York Current
        //                            as found in HARMONIC
        if( (!slackcmp(psd->station_name, pIDX->IDX_reference_name)) && (toupper(pIDX->IDX_type) == psd->station_type) )
        {
            pIDX->pref_sta_data = psd;                // save for later
            return TC_NO_ERROR;
        }
    }


    //    OK, have to read and create from the raw file
    psd = NULL;

    //    If reference station was recently sought, and not found, don't bother
    //            if(!strcmp(pIDX->IDX_reference_name, plast_reference_not_found->mb_str()))
    if(m_last_reference_not_found.IsSameAs(wxString(pIDX->IDX_reference_name, wxConvUTF8)))
        return TC_MASTER_HARMONICS_NOT_FOUND;

    //    Clear for this looking
    m_last_reference_not_found.Clear();

    //    Find and load appropriate constituents
    FILE *fp;
    char linrec[linelen];
    fp = fopen (m_harmfile_name.mb_str(), "r");

    while (read_next_line (fp, linrec, 1))
    {
        nojunk (linrec);
        int curonly = 0;
        if (curonly)
            if (!strstr (linrec, "Current"))
                continue;
        //    See the note above about station names
        //                  if(!strncmp(linrec, "Rivi", 4))
        //                        int ggl = 4;

        if (slackcmp (linrec, pIDX->IDX_reference_name))
            continue;

        //    Got the right location, so load the data

        psd = new Station_Data;

        psd->amplitude = (double *)malloc(num_csts * sizeof(double));
        psd->epoch     = (double *)malloc(num_csts * sizeof(double));
        psd->station_name = (char *)malloc(strlen(linrec) +1);

        char junk[80];
        int a;
        strcpy (psd->station_name, linrec);

        //    Establish Station Type
        wxString caplin(linrec, wxConvUTF8);
        caplin.MakeUpper();
        if(caplin.Contains(_T("CURRENT")))
            psd->station_type = 'C';
        else
            psd->station_type = 'T';



        /* Get meridian */
        read_next_line (fp, linrec, 0);
        psd->meridian = hhmm2seconds (linrec);

        /* Get tzfile, if present */
        if (sscanf (nojunk(linrec), "%s %s", junk, psd->tzfile) < 2)
            strcpy (psd->tzfile, "UTC0");

        /* Get DATUM and units */
        read_next_line (fp, linrec, 0);
        if (sscanf (nojunk(linrec), "%lf %s", &(psd->DATUM), psd->unit) < 2)
            strcpy (psd->unit, "unknown");

        if ((a = findunit (psd->unit)) == -1)
        {
            // Nonsense....
            //                        strcpy (psd->units_abbrv, psd->unit);
            //                        strcpy (psd->units_conv, known_units[a].name);
        }

        psd->have_BOGUS = (findunit(psd->unit) != -1) && (known_units[findunit(psd->unit)].type == BOGUS);

        int unit_c;
        if (psd->have_BOGUS)
            unit_c = findunit("knots");
        else
            unit_c = findunit(psd->unit);

        if (unit_c != -1)
        {
            strcpy (psd->units_conv,       known_units[unit_c].name);
            strcpy (psd->units_abbrv,      known_units[unit_c].abbrv);
        }

        /* Get constituents */
        double loca, loce;
        for (a=0; a<num_csts; a++)
        {
            read_next_line (fp, linrec, 0);
            sscanf (linrec, "%s %lf %lf", junk, &loca, &loce);
            //          loc_epoch[a] *= M_PI / 180.0;
            psd->amplitude[a] = loca;
            psd->epoch[a] = loce * M_PI / 180.;
        }
        fclose (fp);

        break;
    }

    if(!psd) {
        m_last_reference_not_found = wxString(pIDX->IDX_reference_name, wxConvUTF8);
        return TC_MASTER_HARMONICS_NOT_FOUND;
    }
    else {
        m_msd_array.Add(psd);                     // add it to the member array
        pIDX->pref_sta_data = psd;
        return TC_NO_ERROR;
    }

}


/*---------------------------------
 * Low level Index file I/O
 *-------------------------------*/


long TCDS_Ascii_Harmonic::IndexFileIO(int func, long value) {
    char *str;

    switch ( func ) {
        // Close either/both if open
    case IFF_CLOSE :
        if (m_IndexFile) fclose(m_IndexFile);
        m_IndexFile = NULL;
        return(0);

        // Open
    case IFF_OPEN :
        m_IndexFile = fopen( m_indexfile_name.mb_str(), "rt");
        if (m_IndexFile == NULL) return(0);
        return(1);

        // Return file pointer only happens with master file
    case IFF_TELL :
        return(ftell(m_IndexFile));

        // Seek
    case IFF_SEEK :
        return(fseek(m_IndexFile,value,SEEK_SET));

        // Read until EOF .
    case IFF_READ :
        str = fgets( index_line_buffer, 1024, m_IndexFile);

        if (str != NULL)
            return(1);
        else return(0);

    }
    return(0);
}

/* Read a line from the harmonics file, skipping comment lines */
int TCDS_Ascii_Harmonic::read_next_line (FILE *fp, char linrec[linelen], int end_ok)
{
    do {
        if (!fgets (linrec, linelen, fp)) {
            if (end_ok)
                return 0;
            else {
                exit (-1);
            }
        }
    } while (linrec[0] == '#' || linrec[0] == '\r' || linrec[0] == '\n');
    return 1;
}

/* Remove lingering carriage return, but do nothing else */
int TCDS_Ascii_Harmonic::skipnl (FILE *fp)
{
    char linrec[linelen];
    if(NULL == fgets (linrec, linelen, fp))
        return 0;
    return 1;
}

/* Get rid of trailing garbage in buffer */
char * TCDS_Ascii_Harmonic::nojunk (char *line)
{
    char *a;
    a = &(line[strlen(line)]);
    while (a > line)
        if (*(a-1) == '\n' || *(a-1) == '\r' || *(a-1) == ' ')
            *(--a) = '\0';
        else
            break;
    return line;
}

/* Slackful strcmp; 0 = match.  It's case-insensitive and accepts a
 *   prefix instead of the entire string.  The second argument is the
 *   one that can be shorter. Second argument can contain '?' as wild
 *   card character.
 */
int TCDS_Ascii_Harmonic::slackcmp (char *a, char *b)
{
    int c, cmp, n;
    n = strlen (b);
    if ((int)(strlen (a)) < n)
        return 1;
    for (c=0; c<n; c++)
    {
        if(b[c] == '?')
            continue;

        cmp = ((a[c] >= 'A' && a[c] <= 'Z') ? a[c] - 'A' + 'a' : a[c])
              -
              ((b[c] >= 'A' && b[c] <= 'Z') ? b[c] - 'A' + 'a' : b[c]);
        if (cmp)
            return cmp;
    }
    return 0;
}

/* Find a unit; returns -1 if not found. */
int TCDS_Ascii_Harmonic::findunit (const char *unit)
{
    int a;
    for (a=0; a<NUMUNITS; a++) {
        if (!strcmp (unit, known_units[a].name) ||
                !strcmp (unit, known_units[a].abbrv))
            return a;
    }
    return -1;
}

void TCDS_Ascii_Harmonic::free_cst()
{
    free(m_cst_speeds);
    m_cst_speeds = NULL;
}
void TCDS_Ascii_Harmonic::free_nodes()
{
    int a;
    if (num_csts && m_cst_nodes)
        for(a=0; a<num_csts; a++)
            free(m_cst_nodes[a]);
    free(m_cst_nodes);

    m_cst_nodes = NULL;
}

void TCDS_Ascii_Harmonic::free_epochs()
{
    int a;
    if (num_csts && m_cst_epochs)
        for(a=0; a<num_csts; a++)
            free(m_cst_epochs[a]);
    free(m_cst_epochs);

    m_cst_epochs = NULL;
}

/* free harmonics data */
void TCDS_Ascii_Harmonic::free_data ()
{
    free_nodes();
    free_epochs();
    free_cst();
}




//      TCDS_Binary_Harmonic implementation

TCDS_Binary_Harmonic::TCDS_Binary_Harmonic()
{
}

TCDS_Binary_Harmonic::~TCDS_Binary_Harmonic()
{
}

TC_Error_Code TCDS_Binary_Harmonic::LoadData(wxString &data_file_path)
{
    return TC_NO_ERROR;
}

int TCDS_Binary_Harmonic::GetMaxIndex(void)
{
    return 0;
}

IDX_entry *TCDS_Binary_Harmonic::GetIndexEntry(int n_index)
{
    return NULL;
}

TC_Error_Code TCDS_Binary_Harmonic::LoadHarmonicData(IDX_entry *pIDX)
{
    return TC_MASTER_HARMONICS_NOT_FOUND;
}


