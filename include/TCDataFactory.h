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

#ifndef __TCDATAFACTORY_H__
#define __TCDATAFACTORY_H__

#include <wx/string.h>
#include <wx/dynarray.h>
#include "TC_Error_Code.h"

#define NUMUNITS 4

#define REGION 1
#define COUNTRY 2
#define STATE 3

class IDX_entry;

typedef enum {LENGTH, VELOCITY, BOGUS} unit_type;

typedef struct {
    char *name;
    char *abbrv;
    unit_type type;
    double conv_factor;
} unit;

class abbr_entry
{
public:
    int         type;
    wxString    short_s;
    wxString    long_s;
};

class TCDataFactory
{
public:
    TCDataFactory();
    virtual ~TCDataFactory();

    virtual TC_Error_Code LoadData(const wxString &data_file_path) = 0;

    virtual int GetMaxIndex(void) = 0;
    virtual IDX_entry *GetIndexEntry(int n_index) = 0;

    int findunit (const char *unit);
    unit  known_units[NUMUNITS];

    wxString source_ident;
};

WX_DECLARE_OBJARRAY(abbr_entry, ArrayOfAbbrEntry);

#endif
