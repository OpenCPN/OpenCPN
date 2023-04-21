/************************************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  personalized GRID
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
 *
 */

#include "CustomGrid.h"

#include <wx/graphics.h>
#include "GribTable.h"
#include "folder.xpm"

#define SCROLL_SENSIBILITY 20

//------------------------------------------------------------------------------
//          custom grid implementation
//------------------------------------------------------------------------------
CustomGrid::CustomGrid(wxWindow* parent, wxWindowID id, const wxPoint& pos,
                       const wxSize& size, long style, const wxString& name)
    : wxGrid(parent, id, pos, size, style, name) {
  // create grid
  SetTable(new wxGridStringTable(0, 0), true, wxGridSelectRows);
  // some general settings
  EnableEditing(false);
  EnableGridLines(true);
  EnableDragGridSize(false);
  SetMargins(0, 0);
  EnableDragColMove(false);
  EnableDragColSize(false);
  EnableDragRowSize(false);
  // init rows pref
  wxFileConfig* pConf = GetOCPNConfigObject();
  if (pConf) {
    pConf->SetPath(_T("/Settings/GRIB"));
    m_IsDigit = pConf->Read(_T("GribDataTableRowPref"), _T("XXX"));
  }
  if (m_IsDigit.Len() != wxString(_T("XXX")).Len()) m_IsDigit = _T("XXX");
  // create structure for all numerical rows
  for (unsigned int i = 0; i < m_IsDigit.Len(); i++) {
    m_NumRow.push_back(wxNOT_FOUND);
    m_NumRowVal.push_back(std::vector<double>());
  }
  // init labels attr
  wxFont labelfont = GetOCPNGUIScaledFont_PlugIn(_T("Dialog")).MakeBold();
  SetLabelFont(labelfont);
  wxColour colour = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);
  if(colour.Red() > 128) {
    GetGlobalColor(_T("DILG0"), &colour);
    GetGlobalColor(_T("GREEN1"), &m_greenColour);
    GetGlobalColor(_T("DILG1"), &m_greyColour);
  } else {
    GetGlobalColor(_T("GREEN2"), &m_greenColour);
    m_greyColour = colour;
  }
  SetLabelBackgroundColour(colour);
  // set row label size
  int w;
  GetTextExtent(_T("Ab"), &w, NULL, 0, 0, &labelfont);
  double x = (double)w * 6.5;
  SetRowLabelSize((int)x);

#ifdef __WXOSX__
  m_bLeftDown = false;
#endif

  // connect events at dialog level
  Connect(wxEVT_SCROLLWIN_THUMBTRACK,
          wxScrollEventHandler(CustomGrid::OnScroll), NULL, this);
  Connect(wxEVT_SIZE, wxSizeEventHandler(CustomGrid::OnResize), NULL, this);
  Connect(wxEVT_GRID_LABEL_LEFT_CLICK,
          wxGridEventHandler(CustomGrid::OnLabeClick), NULL, this);
  // connect events at grid level
  GetGridWindow()->Connect(wxEVT_LEFT_DOWN,
                           wxMouseEventHandler(CustomGrid::OnMouseEvent), NULL,
                           this);
  GetGridWindow()->Connect(
      wxEVT_LEFT_UP, wxMouseEventHandler(CustomGrid::OnMouseEvent), NULL, this);
  GetGridWindow()->Connect(
      wxEVT_MOTION, wxMouseEventHandler(CustomGrid::OnMouseEvent), NULL, this);
  // timer event
  m_tRefreshTimer.Connect(
      wxEVT_TIMER, wxTimerEventHandler(CustomGrid::OnRefreshTimer), NULL, this);
}

CustomGrid::~CustomGrid() {
  wxFileConfig* pConf = GetOCPNConfigObject();
  if (pConf) {
    pConf->SetPath(_T ( "/Settings/GRIB" ));
    pConf->Write(_T ( "GribDataTableRowPref" ), m_IsDigit);
  }
  m_NumRowVal.clear();
  m_NumRow.clear();
}

void CustomGrid::DrawColLabel(wxDC& dc, int col) {
  // init dc font and colours
  dc.SetFont(m_labelFont);
  if (col == m_gParent->m_pIndex) {
    dc.SetBrush(wxBrush(m_greenColour, wxBRUSHSTYLE_SOLID));
    dc.SetPen(wxPen(m_greenColour, 1));
  } else {
    dc.SetBrush(wxBrush(m_labelBackgroundColour, wxBRUSHSTYLE_SOLID));
    dc.SetPen(wxPen(m_labelBackgroundColour, 1));
  }
  // draw retangle
  wxRect tRect(GetColLeft(col), 1, GetColWidth(col) - 2, m_colLabelHeight - 2);
  dc.DrawRectangle(tRect);
  // draw lines aroud label
  dc.SetPen(GetDefaultGridLinePen());
  dc.DrawLine(GetColLeft(col) - 1, 0, GetColRight(col), 0);
  if (col > -1 && (col == 0 || GetColLabelValue(col).BeforeFirst('-') !=
                                   GetColLabelValue(col - 1).BeforeFirst('-')))
    dc.SetPen(wxPen(*wxBLACK, 4));
  dc.DrawLine(GetColLeft(col) - 1, 0, GetColLeft(col) - 1, m_colLabelHeight);
  if (col == m_numCols - 1) {
    dc.SetPen(wxPen(*wxBLACK, 4));
    dc.DrawLine(GetColRight(col), 0, GetColRight(col), m_colLabelHeight);
  }
  // then draw label
  dc.DrawLabel(GetColLabelValue(col), tRect,
               wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);
}

void CustomGrid::DrawRowLabel(wxDC& dc, int row) {
  // init dc font and colours
  dc.SetFont(m_labelFont);
  dc.SetPen(GetDefaultGridLinePen());
  dc.SetBrush(wxBrush(m_labelBackgroundColour, wxBRUSHSTYLE_SOLID));
  int w = dc.GetTextExtent(_T("Speed")).x;
  wxString label1, label2;
  label1 = GetRowLabelValue(row).BeforeFirst(',', &label2);
  bool pline = true;
  // row is the first of 3 for the same parameter (wind ... waves ...)
  if (GetNumberRows() > row + 2 &&
      label1 == GetRowLabelValue(row + 2).BeforeFirst(',')) {
    pline = false;
    if (IsRowVisible(row + 2)) label1 = _T(" ");
  }
  // row is the second of 3 or the first of 2
  else if (GetNumberRows() > row + 1 &&
           label1 == GetRowLabelValue(row + 1).BeforeFirst(',')) {
    pline = false;
    if (row > 0 &&
        label1 == GetRowLabelValue(row - 1).BeforeFirst(',')) {  // second of 3
      if (!IsRowVisible(row + 1)) label1 = _T(" ");
    }
  }
  // row is the last of 3
  else if (row > 1 && label1 == GetRowLabelValue(row - 2).BeforeFirst(',')) {
    if (IsRowVisible(row - 1)) label1 = _T(" ");
  }
  // row is the last of 2
  else if (row > 0 && label1 == GetRowLabelValue(row - 1).BeforeFirst(',')) {
    if (IsRowVisible(row - 1)) label1 = _T(" ");
  }
  // draw first part of the label
  wxRect aRect(5, GetRowTop(row), m_rowLabelWidth - w, GetRowHeight(row));
  dc.DrawLabel(label1, aRect, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);
  // draw second part of the label
  wxRect bRect(m_rowLabelWidth - w, GetRowTop(row), w, GetRowHeight(row));
  dc.SetFont(wxFont(m_labelFont).Scale(0.85));
  dc.DrawLabel(label2, bRect, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);
  // draw row lines around labels
  if (pline)
    dc.DrawLine(0, GetRowBottom(row) - 1, m_rowLabelWidth,
                GetRowBottom(row) - 1);
  dc.DrawLine(0, GetRowTop(row), 0, GetRowBottom(row));
  dc.DrawLine(m_rowLabelWidth - 1, GetRowTop(row), m_rowLabelWidth - 1,
              GetRowBottom(row));
}

void CustomGrid::DrawCornerLabel(wxDC& dc) {
  dc.SetPen(GetDefaultGridLinePen());
  dc.SetBrush(wxBrush(m_labelBackgroundColour, wxBRUSHSTYLE_SOLID));
  wxRect rect(0, 0, m_rowLabelWidth, m_colLabelHeight);
  dc.DrawRectangle(rect);
  ////scale bitmap to near col label height
  double hc = m_colLabelHeight;
  double hb = wxBitmap(now).GetHeight();
  double scfac = ((hc / hb) * 4) / 4;
  wxBitmap bmp = m_gParent->GetScaledBitmap(wxBitmap(now), _T("now"), scfac);
  // center bitmap
  int x = (m_rowLabelWidth - bmp.GetWidth()) / 2;
  int y = (m_colLabelHeight == bmp.GetHeight())
              ? 0
              : wxMax(0, (m_colLabelHeight - bmp.GetHeight()) / 2);
  dc.DrawBitmap(bmp, x, y);
}

void CustomGrid::OnScroll(wxScrollEvent& event) {
  m_tRefreshTimer.Start(10, wxTIMER_ONE_SHOT);
  event.Skip();
}

void CustomGrid::OnRefreshTimer(wxTimerEvent& event) { ForceRefresh(); }

void CustomGrid::OnResize(wxSizeEvent& event) {
  m_tRefreshTimer.Start(10, wxTIMER_ONE_SHOT);
  event.Skip();
}

void CustomGrid::OnLabeClick(wxGridEvent& event) {
  int row = event.GetRow();
  int col = event.GetCol();
  wxPoint p = event.GetPosition();
  ClearSelection();
  if (row == wxNOT_FOUND && event.GetCol() == wxNOT_FOUND) {  // corner label
    // find the first visible row/col
    int frow = 0, fcol = 0;
    GetFirstVisibleCell(frow, fcol);
    // then scroll as requested;
    MakeCellVisible(frow, m_numCols - 1);
    MakeCellVisible(frow, m_gParent->m_pIndex);
  } else if (row != wxNOT_FOUND && col == wxNOT_FOUND) {  // numerical row label
    int idx = GetRowIndex(row);
    if (idx != wxNOT_FOUND) {
      if (m_IsDigit.GetChar(idx) == 'X')
        m_IsDigit.SetChar(idx, '.');
      else
        m_IsDigit.SetChar(idx, 'X');
      for (int c = 0; c < m_numCols; c++) {
        double value = m_NumRowVal[idx][c];
        /*Current direction is generally reported as the "flow" direction, which
         * is opposite from wind convention. So, adjust.*/
        if (idx == R_CURRENT && m_IsDigit.GetChar(idx) == 'X' &&
            value != GRIB_NOTDEF) {
          value += 180;
          if (value >= 360) value -= 360;
          if (value < 0) value += 360;
        }
        SetCellRenderer(
            row, c, new CustomRenderer(value, m_IsDigit.GetChar(idx) == 'X'));
      }
      m_tRefreshTimer.Start(10, wxTIMER_ONE_SHOT);
    }
  }
}

int CustomGrid::GetRowIndex(int row) {
  int idx = wxNOT_FOUND;
  for (unsigned int i = 0; i < m_NumRow.size(); i++) {
    if (m_NumRow[i] == row) idx = i;
  }
  return idx;
}

void CustomGrid::SetNumericalRow(int row, int col, int datatype, double value) {
  m_NumRow[datatype] = row;
  m_NumRowVal[datatype].push_back(value);
  /*Current direction is generally reported as the "flow" direction,which is
   * opposite from wind convention. So, adjust.*/
  if (datatype == R_CURRENT && m_IsDigit.GetChar(datatype) == 'X' &&
      value != GRIB_NOTDEF) {
    value += 180;
    if (value >= 360) value -= 360;
    if (value < 0) value += 360;
  }
  SetCellRenderer(
      row, col, new CustomRenderer(value, m_IsDigit.GetChar(datatype) == 'X'));
}

void CustomGrid::OnMouseEvent(wxMouseEvent& event) {
  static wxPoint s_pevt;
  wxPoint pevt = event.GetPosition();
#ifdef __WXOSX__
  if (!m_bLeftDown && event.LeftIsDown()) {
    m_bLeftDown = true;
    s_pevt = pevt;
  } else if (m_bLeftDown && !event.LeftIsDown()) {
    m_bLeftDown = false;
    if (HasCapture()) ReleaseMouse();
  }
#else
  if (event.LeftDown()) s_pevt = pevt;
  if (event.LeftUp()) {
    if (HasCapture()) ReleaseMouse();
  }
#endif
  if (event.Dragging()) {
    int frow, fcol, lrow, lcol;
    GetFirstVisibleCell(frow, fcol);
    GetLastVisibleCell(lrow, lcol);
    if (pevt != s_pevt) {
      bool rfh = false;
      int diff = pevt.x - s_pevt.x;
      // scrolling right
      if (diff > SCROLL_SENSIBILITY) {
        s_pevt.x = pevt.x;
        if (fcol > 0) {
          MakeCellVisible(frow, fcol - 1);
          rfh = true;
        }
      }
      // scrolling left
      else if (-diff > SCROLL_SENSIBILITY) {
        s_pevt.x = pevt.x;
        if (lcol < m_numCols - 1) {
          MakeCellVisible(frow, lcol + 1);
          rfh = true;
        }
      }
      // scrolling down
      diff = pevt.y - s_pevt.y;
      if (diff > SCROLL_SENSIBILITY) {
        s_pevt.y = pevt.y;
        if (frow > 0) {
          MakeCellVisible(frow - 1, fcol);
          rfh = true;
        }
      }
      // scrolling up
      else if (-diff > SCROLL_SENSIBILITY) {
        s_pevt.y = pevt.y;
        if (lrow < m_numRows - 1) {
          MakeCellVisible(lrow + 1, fcol);
          MakeCellVisible(frow + 1,
                          fcol);  // workaroud for what seems curious moving 2
                                  // rows instead of 1 in previous function
          rfh = true;
        }
      }
      if (rfh) m_tRefreshTimer.Start(10, wxTIMER_ONE_SHOT);
    }
  }
}

bool CustomGrid::IsRowVisible(int row) {
  for (int i = 0; i < m_numCols; i++) {
    if (IsVisible(row, i, false)) return true;
  }
  return false;
}

// find the first top/left visible cell coords
void CustomGrid::GetFirstVisibleCell(int& frow, int& fcol) {
  bool vis = false;
  frow = 0;
  for (fcol = 0; fcol < m_numCols; fcol++) {
    for (frow = 0; frow < m_numRows; frow++) {
      if (IsVisible(frow, fcol)) {  // find the first row/col
        vis = true;
        break;
      }
    }
    if (vis) break;
  }
}

// find the visible cell coords
void CustomGrid::GetLastVisibleCell(int& lrow, int& lcol) {
  bool vis = false;
  lrow = wxMax(m_numRows - 1, 0);
  for (lcol = wxMax(m_numCols - 1, 0); lcol > -1; lcol--) {
    for (lrow = m_numRows - 1; lrow > -1; lrow--) {
      if (IsVisible(lrow, lcol)) {
        vis = true;
        break;
      }
    }
    if (vis) break;
  }
}

//------------------------------------------------------------------------------
//          custom renderer
//------------------------------------------------------------------------------
void CustomRenderer::Draw(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc,
                          const wxRect& rect, int row, int col,
                          bool isSelected) {
  dc.SetPen(wxPen(attr.GetBackgroundColour(), 1));
  dc.SetBrush(wxBrush(attr.GetBackgroundColour(), wxBRUSHSTYLE_SOLID));
  dc.DrawRectangle(rect);
  if (m_IsDigit || m_dDir == GRIB_NOTDEF) {  // digital format
    wxString text(wxEmptyString);
    if (m_dDir != GRIB_NOTDEF) text.Printf(_T("%03d%c"), (int)m_dDir, 0x00B0);
    dc.DrawLabel(text, rect,
                 wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);
  } else {  // graphical format
    double si = sin((m_dDir - 90) * M_PI / 180.);
    double co = cos((m_dDir - 90) * M_PI / 180.);

    int i = rect.GetTopLeft().x + (rect.GetWidth() / 2);
    int j = rect.GetTopLeft().y + (rect.GetHeight() / 2);

    int arrowSize = rect.GetHeight() - 3;
    int dec = -arrowSize / 2;

#if wxUSE_GRAPHICS_CONTEXT
    wxGraphicsContext* gdc;
    wxClientDC* cdc = new wxClientDC(wxDynamicCast(&grid, wxWindow));
    cdc = wxDynamicCast(&dc, wxClientDC);
    if (cdc) {
      gdc = wxGraphicsContext::Create(*cdc);
#ifdef __WXGTK__
      /*platforms don't manage the same way the gdc origin
      for linux, we have to re-compute the good one.
      To DO : verify it works on all other plateforms (done for MSW*/
      bool vis = false;
      int r = 0;
      for (int c = 0; c < grid.GetNumberCols(); c++) {
        for (r = 0; r < grid.GetNumberRows(); r++) {
          if (grid.IsVisible(r, c)) {  // find the first row/col
            vis = true;
            i -= (c * grid.GetColSize(0));
            j -= (r * grid.GetRowHeight(0));
            break;
          }
        }
        if (vis) break;
      }
#endif
      gdc->SetPen(wxPen(attr.GetTextColour(), 3));
      gdc->SetBrush(wxBrush(attr.GetBackgroundColour(), wxBRUSHSTYLE_SOLID));

      double ii, jj, kk, ll;
      GetArrowsPoints(si, co, i, j, dec, 0, dec + arrowSize, 0, ii, jj, kk, ll);
      gdc->StrokeLine(ii, jj, kk, ll);
      GetArrowsPoints(si, co, i, j, dec - 3, 0, dec + 5, 3, ii, jj, kk, ll);
      gdc->StrokeLine(ii, jj, kk, ll);
      GetArrowsPoints(si, co, i, j, dec - 3, 0, dec + 5, -3, ii, jj, kk, ll);
      gdc->StrokeLine(ii, jj, kk, ll);
      delete gdc;
    } else
#endif
    {
      dc.SetPen(wxPen(attr.GetTextColour(), 3));
      double ii, jj, kk, ll;
      GetArrowsPoints(si, co, i, j, dec, 0, dec + arrowSize, 0, ii, jj, kk, ll);
      dc.DrawLine((int)ii, (int)jj, (int)kk, (int)ll);
      GetArrowsPoints(si, co, i, j, dec - 3, 0, dec + 5, 3, ii, jj, kk, ll);
      dc.DrawLine((int)ii, (int)jj, (int)kk, (int)ll);
      GetArrowsPoints(si, co, i, j, dec - 3, 0, dec + 5, -3, ii, jj, kk, ll);
      dc.DrawLine((int)ii, (int)jj, (int)kk, (int)ll);
    }
  }
}

void CustomRenderer::GetArrowsPoints(double si, double co, int di, int dj,
                                     int i, int j, int k, int l, double& ii,
                                     double& jj, double& kk, double& ll) {
  ii = (i * co - j * si + 0.5) + di;
  jj = (i * si + j * co + 0.5) + dj;
  kk = (k * co - l * si + 0.5) + di;
  ll = (k * si + l * co + 0.5) + dj;
}
