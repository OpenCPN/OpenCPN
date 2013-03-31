/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  GRIB Plugin Friends
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************
 */

#include "GribRecord.h"

WX_DECLARE_OBJARRAY( GribRecord *, ArrayOfGribRecordPtrs );

    // These are indexes into the array
enum { Idx_WIND_VX, Idx_WIND_VY, Idx_WIND_GUST, Idx_PRESSURE, Idx_HTSIGW, Idx_WVDIR,
       Idx_WINDSCAT_VY, Idx_WINDSCAT_VX,
       Idx_SEACURRENT_VX, Idx_SEACURRENT_VY,
       Idx_PRECIP_TOT, Idx_CLOUD_TOT,
       Idx_AIR_TEMP_2M, Idx_SEA_TEMP,
       Idx_COUNT };

class GribRecordSet {
public:
    GribRecordSet() {
        for(int i=0; i<Idx_COUNT; i++)
            m_GribRecordPtrArray[i] = NULL;
    }

    time_t m_Reference_Time;
    GribRecord *m_GribRecordPtrArray[Idx_COUNT];
};
