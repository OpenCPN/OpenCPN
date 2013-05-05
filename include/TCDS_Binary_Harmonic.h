/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2013 by David S. Register                               *
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
 **************************************************************************/

#ifndef __TCDS_BINARY_HARMONIC_H__
#define __TCDS_BINARY_HARMONIC_H__

#include <wx/string.h>

#include "TCDataFactory.h"
#include "Station_Data.h"
#include "IDX_entry.h"

class TCDS_Binary_Harmonic : public TCDataFactory
{
public:
    TCDS_Binary_Harmonic();
    ~TCDS_Binary_Harmonic();

    TC_Error_Code LoadData(const wxString &data_file_path);

    int GetMaxIndex(void) {
        return num_IDX;
    };
    IDX_entry *GetIndexEntry(int n_index);
    TC_Error_Code LoadHarmonicData(IDX_entry *pIDX);

private:
    ArrayOfStationData  m_msd_array;

    wxString            m_last_reference_not_found;

    ArrayOfAbbrEntry    m_abbreviation_array;
    ArrayOfIDXEntry     m_IDX_array;

    int         num_IDX;
    int         num_nodes;
    int         num_csts;
    int         num_epochs;
    double      *m_cst_speeds;
    double      **m_cst_nodes;
    double      **m_cst_epochs;
    double      *m_work_buffer;
    int         m_first_year;
};

#endif
