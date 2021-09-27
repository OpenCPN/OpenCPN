/******************************************************************************
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
 */
#ifndef __CUSTOMGRID_H__
#define __CUSTOMGRID_H__

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif //precompiled headers

#include <wx/grid.h>
#include "ocpn_plugin.h"
#include <wx/graphics.h>
#include <vector>

class GRIBUICtrlBar;
class CustomRenderer;
class GRIBTable;

//------------------------------------------------------------------------------------
//    A derived class from wxGrid
//----------------------------------------------------------------------------------------------------------
class CustomGrid: public wxGrid
{
public:

    CustomGrid( wxWindow *parent, wxWindowID id, const wxPoint &pos,
                const wxSize &size, long style,
                const wxString &name );

    ~CustomGrid();

    void GetFirstVisibleCell( int& frow, int& fcol );
    void GetLastVisibleCell( int& lrow, int& lcol );
    void SetNumericalRow( int row, int col, int datatype, double value );

    GRIBTable* m_gParent;

private:
    void DrawColLabel( wxDC& dc, int col );
    void DrawRowLabel( wxDC& dc, int row );
    void DrawCornerLabel(wxDC & dc);
    void OnScroll( wxScrollEvent& event );
    void OnMouseEvent( wxMouseEvent& event );
    void OnResize( wxSizeEvent& event );
    void OnLabeClick( wxGridEvent& event);
    void OnRefreshTimer( wxTimerEvent& event );
    bool IsRowVisible( int row );
    int  GetRowIndex( int row );

    wxTimer m_tRefreshTimer;
    wxColour m_greenColour;
    wxColour m_greyColour;

    std::vector < std::vector <double> > m_NumRowVal;
    std::vector <int> m_NumRow;
    wxString m_IsDigit;

#ifdef __WXOSX__
    bool     m_bLeftDown;
#endif

};

//----------------------------------------------------------------------------------------------------------
//    A derived class from wxGridCellRenderer
//----------------------------------------------------------------------------------------------------------
class CustomRenderer : public wxGridCellRenderer
{
public:
    CustomRenderer( double dir, bool isdigit)
        : wxGridCellRenderer(), m_dDir(dir), m_IsDigit(isdigit) {}

    virtual void Draw(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, const wxRect& rect, int row, int col, bool isSelected);

    wxSize GetBestSize(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc, int row, int col) {
        return wxSize(-1, -1);
    }
    wxGridCellRenderer *Clone() const { return new CustomRenderer(m_dDir, m_IsDigit);}

private:
    void GetArrowsPoints( double si, double co, int di, int dj, int i, int j, int k, int l, double& ii, double& jj, double& kk, double& ll );

    double m_dDir;
    bool m_IsDigit;
};

#endif //__CUSTOMGRID_H__
