/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  OpenCPN Route table printout
 * Author:   Pavel Saviankou
 *
 ***************************************************************************
 *   Copyright (C) 2012 by David S. Register                               *
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
 *
 */
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif  // precompiled headers
#ifdef __WXMSW__
// #include "c:\\Program Files\\visual leak detector\\include\\vld.h"
#endif

#include <wx/print.h>
#include <wx/printdlg.h>
#include <wx/artprov.h>
#include <wx/stdpaths.h>
#include <wx/intl.h>
#include <wx/listctrl.h>
#include <wx/aui/aui.h>
#include <wx/dialog.h>
#include <wx/progdlg.h>
#include <wx/brush.h>
#include <wx/colour.h>
#include <wx/tokenzr.h>

#if wxCHECK_VERSION(2, 9, 0)
#include <wx/dialog.h>
#else
//  #include "scrollingdialog.h"
#endif

#include "dychart.h"

#ifdef __WXMSW__
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <psapi.h>
#endif

#ifndef __WXMSW__
#include <signal.h>
#include <setjmp.h>
#endif

#include "printtable.h"

using namespace std;

void PrintCell::Init(const wxString& _content, wxDC* _dc, int _width,
                     int _cellpadding, bool _bold_font) {
  bold_font = _bold_font;
  dc = _dc;
  width = _width;
  cellpadding = _cellpadding;
  content = _content;
  page = 1;
  Adjust();
};

void PrintCell::Adjust() {
  wxFont orig_font = dc->GetFont();
  wxFont _font = orig_font;
  if (bold_font) {
    _font.SetWeight(wxFONTWEIGHT_BOLD);
  }
  dc->SetFont(_font);
  vector<wxString> list;
  list.push_back(wxString());
  wxString separator = wxT(" ");
  wxStringTokenizer tokenizer(content, separator, wxTOKEN_RET_DELIMS);
  int words_number = 0;
  while (tokenizer.HasMoreTokens()) {
    wxString token = tokenizer.GetNextToken();
    wxCoord h = 0;
    wxCoord w = 0;
    wxString tmp = list[list.size() - 1];
    wxString tmp2 = tmp + token;
    words_number++;
    dc->GetMultiLineTextExtent(tmp2, &w, &h);
    if ((w < width - 2 * cellpadding) || words_number == 1) {
      list[list.size() - 1] = tmp2;
    } else {
      list.push_back(token);
    }
  }

  for (size_t i = 0; i < list.size() - 1; i++) {
    modified_content = modified_content + list[i] + _T('\n');
  }
  // now add last element without new line
  modified_content = modified_content + list[list.size() - 1];

  wxCoord h = 0;
  wxCoord w = 0;
  dc->GetMultiLineTextExtent(modified_content, &w, &h);
  SetHeight(h + 8);

  dc->SetFont(orig_font);
}

Table::Table() {
  nrows = 0;
  ncols = 0;
  data.clear();
  state = TABLE_SETUP_WIDTHS;
  create_next_row = true;
}

Table::~Table() {
  for (vector<vector<wxString> >::iterator iter = data.begin();
       iter != data.end(); iter++) {
    (*iter).clear();
  }
  data.clear();
}

void Table::Start() {
  if (create_next_row) {
    NewRow();
    create_next_row = false;
  }
}

void Table::NewRow() {
  vector<wxString> empty_row;
  data.push_back(empty_row);
}

Table& Table::operator<<(const double& cellcontent) {
  if (state == TABLE_SETUP_WIDTHS) {
    widths.push_back(cellcontent);
    return *this;
  }
  if (state == TABLE_FILL_DATA) {
    stringstream sstr;
    sstr << cellcontent;
    string _cellcontent = sstr.str();
    Start();
    wxString _str(_cellcontent.c_str(), wxConvUTF8);
    data[data.size() - 1].push_back(_str);
  }
  return *this;
}

Table& Table::operator<<(const wxString& cellcontent) {
  Start();
  if (state == TABLE_FILL_HEADER) {  // if we start to fill with string data, we
                                     // change state automatically.
    header.push_back(cellcontent);
    return *this;
  }
  if (state == TABLE_SETUP_WIDTHS) {  // if we start to fill with string data,
                                      // we change state automatically.
    state = TABLE_FILL_DATA;
  }

  if ((cellcontent == "\n")) {
    create_next_row = true;
    return *this;
  }
  data[data.size() - 1].push_back(cellcontent);
  return *this;
}

Table& Table::operator<<(const int& cellcontent) {
  if (state == TABLE_SETUP_WIDTHS) {
    widths.push_back((double)cellcontent);
    return *this;
  }
  if (state == TABLE_FILL_DATA) {
    stringstream sstr;
    sstr << cellcontent;
    string _cellcontent = sstr.str();
    Start();
    wxString _str(_cellcontent.c_str(), wxConvUTF8);
    data[data.size() - 1].push_back(_str);
  }
  return *this;
}

ostream& operator<<(ostream& out, Table& table) {
  vector<vector<wxString> >& data = table.GetData();

  for (vector<vector<wxString> >::iterator iter = data.begin();
       iter != data.end(); iter++) {
    vector<wxString> row = (*iter);
    for (vector<wxString>::iterator rowiter = row.begin(); rowiter != row.end();
         rowiter++) {
      out << (*rowiter).fn_str() << " ";
    }
    out << endl;
  }
  return out;
}

PrintTable::PrintTable() : Table() { rows_heights.clear(); }

void PrintTable::AdjustCells(wxDC* dc, int marginX, int marginY) {
  number_of_pages = -1;
  contents.clear();
  int sum = 0;
  for (size_t j = 0; j < widths.size(); j++) {
    sum += widths[j];
  }

  int w, h;
  dc->GetSize(&w, &h);

  double scale_x, scale_y;
  dc->GetUserScale(&scale_x, &scale_y);
  w /= scale_x;
  h /= scale_y;

  int width = w - 4 * marginX;
  header_height = -1;
  for (size_t j = 0; j < header.size(); j++) {
    int cell_width = (int)((double)width * widths[j] / sum);
    PrintCell cell_content;
    cell_content.Init(header[j], dc, cell_width, 10, true);
    header_content.push_back(cell_content);
    header_height = std::max(header_height, cell_content.GetHeight());
  }

  for (size_t i = 0; i < data.size(); i++) {
    vector<wxString> row = data[i];
    vector<PrintCell> contents_row;
    int max_height = -1;
    for (size_t j = 0; j < row.size(); j++) {
      int cell_width = (int)((double)width * widths[j] / sum);
      PrintCell cell_content;
      cell_content.Init(row[j], dc, cell_width, 10);
      contents_row.push_back(cell_content);
      max_height = std::max(max_height, cell_content.GetHeight());
    }
    rows_heights.push_back(max_height);
    contents.push_back(contents_row);
  }

  int stripped_page = h - 4 * marginY - header_height;
  int current_page = 1;
  int current_y = 0;
  for (size_t i = 0; i < data.size(); i++) {
    int row_height = rows_heights[i];
    if (row_height + current_y > stripped_page) {
      current_page++;
      current_y = row_height;
    } else {
      current_y += row_height;
    }
    int row_page = current_page;
    vector<PrintCell>& contents_row = contents[i];
    for (size_t j = 0; j < contents_row.size(); j++) {
      contents_row[j].SetPage(row_page);
      contents_row[j].SetHeight(row_height);
    }
    number_of_pages = std::max(row_page, number_of_pages);
  }
}
