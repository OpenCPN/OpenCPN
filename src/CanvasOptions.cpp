/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Canvas Options Window/Dialog
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
 *
 *
 */

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif //precompiled headers

#include "wx/artprov.h"
#include <wx/statline.h>


#include "dychart.h"

#include "chcanv.h"
#include "CanvasOptions.h"
#include "OCPNPlatform.h"

#ifdef ocpnUSE_SVG
#include "wxsvg/include/wxSVG/svg.h"
#endif // ocpnUSE_SVG

#ifdef __OCPN__ANDROID__
#include "androidUTIL.h"
#endif



//------------------------------------------------------------------------------
//    External Static Storage
//------------------------------------------------------------------------------

extern OCPNPlatform              *g_Platform;

//  Helper utilities


//  Helper classes


//------------------------------------------------------------------------------
//    CanvasOptions
//------------------------------------------------------------------------------

BEGIN_EVENT_TABLE(CanvasOptions, wxScrolledWindow)
EVT_CLOSE(CanvasOptions::OnClose)
//EVT_CHECKBOX(ID_QUILTCHECKBOX1, CanvasOptions::OnOptionChange)
END_EVENT_TABLE()


//: wxPanel( parent, wxID_ANY, caption, pos, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxSTAY_ON_TOP )

CanvasOptions::CanvasOptions( wxWindow *parent)
: wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL | wxSUNKEN_BORDER)

{
    wxFont *qFont = GetOCPNScaledFont(_("Dialog"));
    SetFont( *qFont );

#if 0    
    wxBoxSizer *topsizer = new wxBoxSizer( wxVERTICAL );
    
    wxBoxSizer *icon_text = new wxBoxSizer( wxHORIZONTAL );
    
    wxCheckBox *pcb = new wxCheckBox(this, 49, _T("Test CB with very long name cccccccccccccccccccccccccc"));
    topsizer->Add(pcb, 0 );;
#endif
    
    int border_size = 5;
    int group_item_spacing = 5;
    
    wxSizerFlags groupLabelFlags = wxSizerFlags(0).Align(wxALIGN_RIGHT | wxALIGN_TOP).Border(wxALL, group_item_spacing);
    wxSizerFlags groupInputFlags = wxSizerFlags(0).Align(wxALIGN_LEFT | wxALIGN_TOP).Border(wxBOTTOM, group_item_spacing * 2).Expand();
    wxSizerFlags verticleInputFlags = wxSizerFlags(0).Align(wxALIGN_LEFT).Border(wxALL, group_item_spacing);
    wxSizerFlags inputFlags = wxSizerFlags(0).Align(wxALIGN_LEFT | wxALIGN_CENTRE_VERTICAL).Border(wxALL, group_item_spacing);
    
    wxScrolledWindow *pDisplayPanel = this;

#if 0    
    wxFlexGridSizer* generalSizer = new wxFlexGridSizer(2);
    generalSizer->SetHGap(border_size);
    //    generalSizer->AddGrowableCol( 0, 1 );
    //    generalSizer->AddGrowableCol( 1, 1 );
    //    pDisplayPanel->SetSizer( generalSizer );

    // wxFlexGridSizer grows wrongly in wx2.8, so we need to centre it in
    // another sizer instead of letting it grow.
    wxBoxSizer* wrapperSizer = new wxBoxSizer(wxVERTICAL);
    pDisplayPanel->SetSizer(wrapperSizer);
    wrapperSizer->Add(generalSizer, 1, wxALL | wxALIGN_CENTER, border_size);
#endif

    wxBoxSizer* generalSizer = new wxBoxSizer(wxVERTICAL);
    pDisplayPanel->SetSizer(generalSizer);

    //  Options Label
    generalSizer->Add(0, border_size * 2);
    wxStaticText* optionsLabelBox = new wxStaticText(pDisplayPanel, wxID_ANY, _("Chart View Options"));
    generalSizer->Add(optionsLabelBox, 0, wxALL | wxEXPAND | wxALIGN_CENTER, border_size);
    wxStaticLine *m_staticLine121 = new wxStaticLine(pDisplayPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
    generalSizer->Add(m_staticLine121, 0, wxALL | wxEXPAND | wxALIGN_CENTER, border_size);
    
    // spacer
    generalSizer->Add(0, border_size * 4);

    // Nav Mode
    wxStaticBoxSizer* boxNavMode = new wxStaticBoxSizer(new wxStaticBox(pDisplayPanel, wxID_ANY, _("Navigation Mode")), wxVERTICAL);
    generalSizer->Add(boxNavMode, 0, wxALL | wxEXPAND, border_size);
    
    wxBoxSizer* rowOrientation = new wxBoxSizer(wxHORIZONTAL);
    boxNavMode->Add(rowOrientation);

    pCBNorthUp = new wxRadioButton(pDisplayPanel, wxID_ANY, _("North Up"));
    rowOrientation->Add(pCBNorthUp, inputFlags);
    pCBCourseUp = new wxRadioButton(pDisplayPanel, ID_COURSEUPCHECKBOX, _("Course Up"));
    rowOrientation->Add(pCBCourseUp, wxSizerFlags(0).Align(wxALIGN_CENTRE_VERTICAL).Border(wxLEFT, group_item_spacing * 2));

    pCBLookAhead = new wxCheckBox(pDisplayPanel, ID_CHECK_LOOKAHEAD, _("Look Ahead Mode"));
    boxNavMode->Add(pCBLookAhead, verticleInputFlags);

    // spacer
    generalSizer->Add(0, border_size * 4);

    
    // Display Options
    wxStaticBoxSizer* boxDisp = new wxStaticBoxSizer(new wxStaticBox(pDisplayPanel, wxID_ANY, _("Display Options")), wxVERTICAL);
    generalSizer->Add(boxDisp, 0, wxALL | wxEXPAND, border_size);
    
    pCDOQuilting = new wxCheckBox(pDisplayPanel, ID_QUILTCHECKBOX1, _("Enable Chart Quilting"));
    boxDisp->Add(pCDOQuilting, verticleInputFlags);
    pCDOQuilting->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CanvasOptions::OnOptionChange ), NULL, this );
    
    pSDisplayGrid = new wxCheckBox(pDisplayPanel, ID_CHECK_DISPLAYGRID, _("Show Grid"));
    boxDisp->Add(pSDisplayGrid, verticleInputFlags);
    pSDisplayGrid->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CanvasOptions::OnOptionChange ), NULL, this );
    
    pCDOOutlines = new wxCheckBox(pDisplayPanel, ID_OUTLINECHECKBOX1, _("Show Chart Outlines"));
    boxDisp->Add(pCDOOutlines, verticleInputFlags);
    pCDOOutlines->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CanvasOptions::OnOptionChange ), NULL, this );
    
    pSDepthUnits = new wxCheckBox(pDisplayPanel, ID_SHOWDEPTHUNITSBOX1, _("Show Depth Units"));
    boxDisp->Add(pSDepthUnits, verticleInputFlags);
    pSDepthUnits->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( CanvasOptions::OnOptionChange ), NULL, this );
    
    RefreshControlValues();
    
    SetAutoLayout( true );
    
    generalSizer->SetSizeHints( this );
    generalSizer->Fit( this );
//     wxSize size( GetSize() );
//     if (size.x < size.y*3/2)
//     {
//         size.x = size.y*3/2;
//         SetSize( size );
//     }
    
}


void CanvasOptions::OnClose( wxCloseEvent& event )
{
//     SetReturnCode(wxID_CANCEL);
//     EndModal( wxID_CANCEL );
}

void CanvasOptions::OnOptionChange( wxCommandEvent &event)
{
    UpdateCanvasOptions();
}



void CanvasOptions::RefreshControlValues( void )
{
    ChartCanvas *parentCanvas = wxDynamicCast(m_parent, ChartCanvas);
    if(!parentCanvas)
        return;
    
    //  Display options
 
    pCDOQuilting->SetValue(parentCanvas->GetQuiltMode());
    pSDisplayGrid->SetValue(parentCanvas->GetShowGrid());
    pCDOOutlines->SetValue(parentCanvas->GetShowOutlines());
    pSDepthUnits->SetValue(parentCanvas->GetShowDepthUnits());
    
    
}

void CanvasOptions::UpdateCanvasOptions( void )
{
    ChartCanvas *parentCanvas = wxDynamicCast(m_parent, ChartCanvas);
    if(!parentCanvas)
        return;
    
    bool b_needRefresh = false;
    if(pCDOQuilting->GetValue() != parentCanvas->GetQuiltMode()){
        parentCanvas->ToggleCanvasQuiltMode();
    }
    
    if(pSDisplayGrid->GetValue() != parentCanvas->GetShowGrid()){
        parentCanvas->SetShowGrid(pSDisplayGrid->GetValue());
        b_needRefresh = true;
    }
    
    if(pCDOOutlines->GetValue() != parentCanvas->GetShowOutlines()){
        parentCanvas->SetShowOutlines(pCDOOutlines->GetValue());
        b_needRefresh = true;
    }
    if(pSDepthUnits->GetValue() != parentCanvas->GetShowDepthUnits()){
        parentCanvas->SetShowDepthUnits(pSDepthUnits->GetValue());
        b_needRefresh = true;
    }
    
    if(b_needRefresh){
        parentCanvas->Refresh();
        parentCanvas->InvalidateGL();
    }
        
}



