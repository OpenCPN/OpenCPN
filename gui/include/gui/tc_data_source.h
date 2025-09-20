/**************************************************************************
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
 *   along with this program; if not, see <https://www.gnu.org/licenses/>. *
 **************************************************************************/

/**
 * \file
 *
 * Tide datasource container on top of TCDataSource
 */

#ifndef TCDATASOURCE_H_
#define TCDATASOURCE_H_

#include <wx/string.h>
#include <wx/dynarray.h>

#include "tc_error_code.h"
#include "idx_entry.h"
#include "tc_data_factory.h"
#include "tcds_ascii_harmonic.h"
#include "tcds_binary_harmonic.h"

class TCDataSource {
public:
  TCDataSource();
  ~TCDataSource();

  TC_Error_Code LoadData(const wxString &data_file_path);

  int GetMaxIndex();
  IDX_entry *GetIndexEntry(int n_index);
  TC_Error_Code LoadHarmonicData(IDX_entry *pIDX);

private:
  wxString m_data_source_path;

  TCDataFactory *m_pfactory;
  TCDS_Ascii_Harmonic *pTCDS_Ascii_Harmonic;
  TCDS_Binary_Harmonic *pTCDS_Binary_Harmonic;
};

WX_DECLARE_OBJARRAY(TCDataSource, ArrayOfTCDSources);

#endif
