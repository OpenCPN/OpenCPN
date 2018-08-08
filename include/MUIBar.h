/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  MUI Control Bar
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2018 by David S. Register                               *
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


#ifndef __muibar_H__
#define __muibar_H__

#include "chart1.h"

//----------------------------------------------------------------------------
//   constants
//----------------------------------------------------------------------------

enum
{
    ID_MUI_MENU = 21500
};
    

class MyFrame;
class ChartCanvas;
class MUIButton;

//----------------------------------------------------------------------------
// MUIBar
//----------------------------------------------------------------------------
class MUIBar : public wxWindow
{
public:
    MUIBar();
    MUIBar(ChartCanvas* parent, wxWindowID id = wxID_ANY, 
           const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
           long style = 0, const wxString& name = wxPanelNameStr);
    
    ~MUIBar();

    void OnSize( wxSizeEvent& event );
    void OnPaint( wxPaintEvent& event ); 
    void OnToolLeftClick(  wxCommandEvent& event );
    void OnEraseBackground( wxEraseEvent& event );
    
    void SetBestSize( void );
    void SetBestPosition( void );
    
private:
    void Init( void );
    void CreateControls();
    
    ChartCanvas *m_parentCanvas;
    
    MUIButton   *m_zinButton;
    MUIButton   *m_zoutButton;
    MUIButton   *m_menuButton;
    
DECLARE_EVENT_TABLE()
};

#endif
