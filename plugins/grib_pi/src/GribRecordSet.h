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

    // These are indexes into the array
enum { Idx_WIND_VX, Idx_WIND_VX850, Idx_WIND_VX700, Idx_WIND_VX500, Idx_WIND_VX300,
    Idx_WIND_VY, Idx_WIND_VY850, Idx_WIND_VY700, Idx_WIND_VY500, Idx_WIND_VY300,
    Idx_WIND_GUST, Idx_PRESSURE, Idx_HTSIGW, Idx_WVDIR, Idx_WVPER,
    Idx_SEACURRENT_VX, Idx_SEACURRENT_VY, Idx_PRECIP_TOT, Idx_CLOUD_TOT,
    Idx_AIR_TEMP, Idx_AIR_TEMP850, Idx_AIR_TEMP700, Idx_AIR_TEMP500, Idx_AIR_TEMP300,
    Idx_SEA_TEMP, Idx_CAPE,
    Idx_HUMID_RE, Idx_HUMID_RE850, Idx_HUMID_RE700, Idx_HUMID_RE500, Idx_HUMID_RE300,
    Idx_GEOP_HGT,Idx_GEOP_HGT850, Idx_GEOP_HGT700, Idx_GEOP_HGT500, Idx_GEOP_HGT300,
    Idx_COUNT };

class GribRecordSet {
public:
    GribRecordSet(unsigned int id) : m_Reference_Time(-1), m_ID(id) {
        for(int i=0; i<Idx_COUNT; i++) {
            m_GribRecordPtrArray[i] = 0;
            m_GribRecordUnref[i] = false;
        }
    }

    virtual ~GribRecordSet()
    {
         RemoveGribRecords();
    }

    /* copy and paste by plugins, keep functions in header */
    void SetUnRefGribRecord(int i, GribRecord *pGR ) { 
        assert (i >= 0 && i < Idx_COUNT);
        if (m_GribRecordUnref[i] == true) {
            delete m_GribRecordPtrArray[i];
        }
        m_GribRecordPtrArray[i] = pGR;
        m_GribRecordUnref[i] = true;
    }

    void RemoveGribRecords( ) { 
        for(int i=0; i<Idx_COUNT; i++) {
            if (m_GribRecordUnref[i] == true) {
                delete m_GribRecordPtrArray[i];
            }
        }
    }

    time_t m_Reference_Time;
    unsigned int m_ID;

    GribRecord *m_GribRecordPtrArray[Idx_COUNT];
private:
    // grib records files are stored and owned by reader mapGribRecords
    // interpolated grib are not, keep track of them
    bool        m_GribRecordUnref[Idx_COUNT];
};
