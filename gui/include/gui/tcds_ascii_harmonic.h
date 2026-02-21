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
 * Load harmonic data from ascii source TCDataFactory
 */

#ifndef TCDS_ASCII_HARMONIC_H_
#define TCDS_ASCII_HARMONIC_H_

#include <stdio.h>
#include <vector>

#include <wx/string.h>

#include "tc_data_factory.h"
#include "station_data.h"
#include "idx_entry.h"

#define linelen 300

class TCDS_Ascii_Harmonic : public TCDataFactory {
public:
  TCDS_Ascii_Harmonic();
  ~TCDS_Ascii_Harmonic();

  TC_Error_Code LoadData(const wxString &data_file_path);

  int GetMaxIndex() { return num_IDX; };
  IDX_entry *GetIndexEntry(int n_index);
  TC_Error_Code LoadHarmonicData(IDX_entry *pIDX);

  int pIDX_Ref;

private:
  long IndexFileIO(int func, long value);
  TC_Error_Code init_index_file();
  TC_Error_Code build_IDX_entry(IDX_entry *pIDX);
  TC_Error_Code LoadHarmonicConstants(const wxString &data_file_path);
  int read_next_line(FILE *fp, char linrec[linelen], int end_ok);
  int skipnl(FILE *fp);
  char *nojunk(char *line);
  int slackcmp(char *a, char *b);

  void free_cst();
  void free_nodes();
  void free_epochs();
  void free_data();

  ArrayOfStationData m_msd_array;

  wxString m_indexfile_name;
  wxString m_harmfile_name;
  wxString m_last_reference_not_found;

  char index_line_buffer[1024];
  FILE *m_IndexFile;
  std::vector<abbr_entry> m_abbreviation_array;
  ArrayOfIDXEntry m_IDX_array;

  int num_IDX;
  int num_nodes;
  int num_csts;
  int num_epochs;
  double *m_cst_speeds;
  double **m_cst_nodes;
  double **m_cst_epochs;
  double *m_work_buffer;
  int m_first_year;
};

#endif  // TCDS_ASCII_HARMONIC_H_
