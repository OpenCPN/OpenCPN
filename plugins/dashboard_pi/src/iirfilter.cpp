/******************************************************************************
* iirfilter.h
*
* Project:  Many
* Purpose:  Class that implements single order inifinite-impulse-response filter
* Author:   Transmitterdan
***************************************************************************
*   Copyright (C) 2016                                                    *
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

#include "iirfilter.h"

#include <wx/math.h>

iirfilter::iirfilter(double fc, int tp) {
    wxASSERT(tp == IIRFILTER_TYPE_DEG || tp == IIRFILTER_TYPE_LINEAR || tp == IIRFILTER_TYPE_RAD);
    setFC(fc);
    type = tp;
    reset();
}

double iirfilter::filter(double data) {
    if (!std::isnan(data) && !std::isnan(b1)) {
        if (std::isnan(accum))
            accum = 0.0;
        switch (type) {
            case IIRFILTER_TYPE_LINEAR:
                accum = accum * b1 + a0 * data;
                break;

            case IIRFILTER_TYPE_DEG:
                unwrapDeg(data);
                accum = accum * b1 + a0 * (oldDeg + 360.0*wraps);
                break;
                   
            case IIRFILTER_TYPE_RAD:
                unwrapRad(data);
                accum = accum * b1 + a0 * (oldRad + 2.0*M_PI*wraps);
                break;
                    
            default:
                wxASSERT(false);
        }
    }
    else
        accum = data;
    return get();
}

void iirfilter::reset(double a) {
    accum = a;
    oldDeg = NAN;
    oldRad = NAN;
    wraps = 0;
}

void iirfilter::setFC(double fc) {
    if (std::isnan(fc) || fc <= 0.0)
        a0 = b1 = NAN;  // NAN means no filtering will be done
    else {
        reset();
        b1 = exp(-2.0 * 3.1415926535897932384626433832795 * fc);
        a0 = 1 - b1;
    }
}

void iirfilter::setType(int tp)
{
    wxASSERT(tp == IIRFILTER_TYPE_DEG || tp == IIRFILTER_TYPE_LINEAR || tp == IIRFILTER_TYPE_RAD);
    type = tp;
}

double iirfilter::getFc(void)
{
    if (std::isnan(b1))
        return 0.0;
    double fc = log(b1) / (-2.0 * 3.1415926535897932384626433832795);
    return fc;
}

int iirfilter::getType(void)
{
    return type;
}

double iirfilter::get(void) {
    if (std::isnan(accum))
        return accum;
    double res = accum;
    switch (type) {
        case IIRFILTER_TYPE_DEG:
            while (res < 0) res += 360.0;
            while (res > 360) res -= 360.0;
            break;
                
        case IIRFILTER_TYPE_RAD:
            while (res < 0) res += 2.0*M_PI;
            while (res > 2.0*M_PI) res -= 2.0*M_PI;
            break;
    }
    return res;
}

void iirfilter::unwrapDeg(double deg) {
    if (deg - oldDeg > 180) {
        wraps--;
    }
    else if (deg - oldDeg < -180) {
        wraps++;
    }
    oldDeg = deg;
}

void iirfilter::unwrapRad(double rad) {
    if (rad - oldRad > M_PI) {
        wraps--;
    }
    else if (rad - oldRad < M_PI) {
        wraps++;
    }
    oldRad = rad;
}
