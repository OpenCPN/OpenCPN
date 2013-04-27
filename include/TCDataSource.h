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

#ifndef __TCDATASOURCE_H__
#define __TCDATASOURCE_H__

#include <wx/string.h>
#include <wx/dynarray.h>

#include "TC_Error_Code.h"

class IDX_entry;
class TCDataFactory;
class TCDS_Ascii_Harmonic;
class TCDS_Binary_Harmonic;

class TCDataSource
{
public:
    TCDataSource();
    ~TCDataSource();

    TC_Error_Code LoadData(const wxString &data_file_path);

    int GetMaxIndex(void);
    IDX_entry *GetIndexEntry(int n_index);
    TC_Error_Code LoadHarmonicData(IDX_entry *pIDX);

private:
    wxString             m_data_source_path;

    TCDataFactory        *m_pfactory;
    TCDS_Ascii_Harmonic  *pTCDS_Ascii_Harmonic;
    TCDS_Binary_Harmonic *pTCDS_Binary_Harmonic;

};


WX_DECLARE_OBJARRAY(TCDataSource, ArrayOfTCDSources);

#endif
