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
 *
 *
 */

#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif //precompiled headers
#include "dychart.h"

#include "chcanv.h"
#include "MUIBar.h"
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
static wxBitmap LoadSVG( const wxString filename, unsigned int width, unsigned int height )
{
    #ifdef ocpnUSE_SVG
    wxSVGDocument svgDoc;
    if( svgDoc.Load(filename) )
        return wxBitmap( svgDoc.Render( width, height, NULL, true, true ) );
    else
        return wxBitmap(width, height);
    #else
        return wxBitmap(width, height);
    #endif // ocpnUSE_SVG
}



//  Helper classes


//------------------------------------------------------------------------------
//    MUIButton Static Storage
//------------------------------------------------------------------------------

class MUIButton: public wxWindow
{    
    DECLARE_DYNAMIC_CLASS( MUIButton )
    DECLARE_EVENT_TABLE()
    
    wxSize DoGetBestSize() const;
public:
    MUIButton();
    MUIButton(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString & bitmap = wxEmptyString,
              const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxNO_BORDER);
    
    bool Create(wxWindow* parent, wxWindowID id = wxID_ANY, const wxString & bitmap = wxEmptyString,
                const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxNO_BORDER);
    
    ~MUIButton();
    
    void Init();
    void CreateControls();
    
    void OnSize( wxSizeEvent& event );
    void OnPaint( wxPaintEvent& event );
    void OnEraseBackground( wxEraseEvent& event );
    void OnLeftDown( wxMouseEvent& event );
    void OnLeftUp( wxMouseEvent& event );
    
    
    wxBitmap GetBitmapResource( const wxString& name );
    
    wxIcon GetIconResource( const wxString& name );
    
    /// Should we show tooltips?
    static bool ShowToolTips();
    
private:
    wxString m_bitmapFile;
    wxBitmap m_bitmap;
    
};


IMPLEMENT_DYNAMIC_CLASS( MUIButton, wxWindow )



BEGIN_EVENT_TABLE( MUIButton, wxWindow )

EVT_SIZE( MUIButton::OnSize )
EVT_PAINT( MUIButton::OnPaint )
EVT_ERASE_BACKGROUND( MUIButton::OnEraseBackground )
EVT_LEFT_DOWN( MUIButton::OnLeftDown )
EVT_LEFT_UP( MUIButton::OnLeftUp )

END_EVENT_TABLE()


MUIButton::MUIButton()
{
    Init();
}

MUIButton::MUIButton(wxWindow* parent, wxWindowID id, const wxString & bitmap, const wxPoint& pos, const wxSize& size, long style)
{
    Init();
    Create(parent, id, bitmap, pos, size, style);
}


bool MUIButton::Create(wxWindow* parent, wxWindowID id, const wxString & bitmap, const wxPoint& pos, const wxSize& size, long style)
{
    wxWindow::Create(parent, id, pos, size, style);
    m_bitmapFile = bitmap;
    
    CreateControls();
    return true;
}


MUIButton::~MUIButton()
{
}


void MUIButton::Init()
{
}


void MUIButton::CreateControls()
{    
    this->SetForegroundColour(wxColour(255, 255, 255));
    SetBackgroundColour(wxColour(40,40,40));
    this->SetFont(wxFont(8, wxSWISS, wxNORMAL, wxBOLD, false, wxT("Tahoma")));
}


bool MUIButton::ShowToolTips()
{
    return true;
}

void MUIButton::OnSize( wxSizeEvent& event )
{
    if(m_bitmap.IsOk()){
        if( event.GetSize() == m_bitmap.GetSize())
            return;
    }

    if(!m_bitmapFile.IsEmpty())
        m_bitmap = LoadSVG( m_bitmapFile, event.GetSize().x, event.GetSize().y );
        
    Refresh();
}

wxBitmap MUIButton::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
}

wxIcon MUIButton::GetIconResource( const wxString& name )
{
    // Icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
}

wxSize MUIButton::DoGetBestSize() const
{
//     wxSize labelSize = wxDefaultSize;
//     GetTextExtent(m_Label, &labelSize.x, &labelSize.y);
//     return wxSize(wxMax(40, labelSize.x + 20), wxMax(20, labelSize.y + 10));
    return wxSize(100, 100);
}


void MUIButton::OnPaint( wxPaintEvent& event )
{
    wxPaintDC dc( this );
    
    if(m_bitmap.IsOk()){
        dc.DrawBitmap(m_bitmap, 0, 0);
    }
    
        
#if 0
    wxBufferedPaintDC dc(this);
    
    wxRect clientRect = GetClientRect();
    wxRect gradientRect = clientRect;
    gradientRect.SetHeight(gradientRect.GetHeight()/2 + ((GetCapture() == this) ? 1 : 0));
    if(GetCapture() != this)
    {
        dc.GradientFillLinear(gradientRect, 
                              m_GradientTopStartColour, m_GradientTopEndColour, wxSOUTH);
    }
    else
    {
        dc.SetPen(wxPen(m_PressedColourTop));
        dc.SetBrush(wxBrush(m_PressedColourTop));
        dc.DrawRectangle(gradientRect);
    }
    
    gradientRect.Offset(0, gradientRect.GetHeight());
    
    if(GetCapture() != this)
    {
        dc.GradientFillLinear(gradientRect, 
                              m_GradientBottomStartColour, m_GradientBottomEndColour, wxSOUTH);
    }
    else
    {
        dc.SetPen(wxPen(m_PressedColourBottom));
        dc.SetBrush(wxBrush(m_PressedColourBottom));
        dc.DrawRectangle(gradientRect);
    }
    dc.SetPen(wxPen(GetBackgroundColour()));
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.DrawRectangle(0, 0, clientRect.GetWidth(), clientRect.GetHeight());
    dc.SetFont(GetFont());
    dc.SetTextForeground(GetForegroundColour());
    if(GetCapture() == this)
    {
        clientRect.Offset(1, 1);
    }
    dc.DrawLabel(m_Label, clientRect, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL);
#endif    
}


void MUIButton::OnLeftDown( wxMouseEvent& event )
{
    if(GetCapture() != this)
    {
        CaptureMouse();
        Refresh();
    }
}


void MUIButton::OnLeftUp( wxMouseEvent& event )
{
    if(GetCapture() == this)
    {
        ReleaseMouse();
        Refresh();
        if(GetClientRect().Contains(event.GetPosition()))
        {
            wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, GetId());
            GetParent()->GetEventHandler()->AddPendingEvent( evt );
        }
    }
}


void MUIButton::OnEraseBackground( wxEraseEvent& event )
{
}





//------------------------------------------------------------------------------
//          MUIBar Window Implementation
//------------------------------------------------------------------------------
BEGIN_EVENT_TABLE(MUIBar, wxWindow)
//EVT_TIMER ( MUIBAR_EVENT_TIMER, MUIBar::onTimerEvent )
EVT_PAINT ( MUIBar::OnPaint )
EVT_SIZE( MUIBar::OnSize )
EVT_MENU(-1, MUIBar::OnToolLeftClick)

END_EVENT_TABLE()

// Define a constructor
MUIBar::MUIBar()
{
}

MUIBar::MUIBar(ChartCanvas* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
        : wxWindow()
{
    m_parentCanvas = parent;
    SetBackgroundStyle( wxBG_STYLE_TRANSPARENT );
    wxWindow::Create(parent, id, pos, size, style, name);
    
    Init();
    CreateControls();
}




MUIBar::~MUIBar()
{
}

void MUIBar::Init()
{
    m_zinButton = NULL;
}

void MUIBar::CreateControls()
{
    //SetBackgroundStyle( wxBG_STYLE_TRANSPARENT );
    
    //SetBackgroundColour(wxColour(40,40,40));
    SetBackgroundColour(wxColour(40,40,40, 0));
    
    wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(topSizer);

    //topSizer->AddSpacer(50);
    
    wxBoxSizer *barSizer = new wxBoxSizer(wxHORIZONTAL);
    topSizer->Add(barSizer, 0, wxEXPAND );
    
    // Buttons
    //wxString iconDir = g_Platform->GetSharedDataDir() + _T("uidata/traditional/");
    wxString iconDir = _T("/home/dsr/Projects/opencpn/data/svg/MUI/");
    
     m_zinButton = new MUIButton( this, ID_ZOOMIN, iconDir + _T("zoom-in.svg"));
     barSizer->Add(m_zinButton, 1, wxSHAPED);
 
     m_zoutButton = new MUIButton( this, ID_ZOOMOUT, iconDir + _T("zoom-out.svg"));
     barSizer->Add(m_zoutButton, 1, wxSHAPED);
  
     barSizer->AddSpacer(5);
     
     m_menuButton = new MUIButton( this, ID_MUI_MENU, iconDir + _T("menu.svg"));
     barSizer->Add(m_menuButton, 1,  wxALIGN_RIGHT | wxSHAPED);
    
    //topSizer->AddSpacer(50);
    
}

void MUIBar::SetBestSize( void )
{
    SetSize( 400, 50);
}

void MUIBar::SetBestPosition( void )
{
    int x = (m_parent->GetClientSize().x - GetSize().x) / 2;
    if(x > 0){
        int bottomOffset = 0;
    
        ChartCanvas *pcc = wxDynamicCast(m_parent, ChartCanvas);
        bottomOffset += pcc->GetPianoHeight();
        
        int y = m_parent->GetClientSize().y - GetSize().y - bottomOffset;
        SetSize(x, y, -1, -1, wxSIZE_USE_EXISTING);
    }
}

void MUIBar::OnSize( wxSizeEvent& event )
{
    int buttonSize = event.GetSize().y / 2;
    Layout();
//     if(m_zinButton)
//         m_zinButton->SetSize(buttonSize, buttonSize);
}

void MUIBar::OnToolLeftClick(  wxCommandEvent& event )
{
    //  Handle the MUIButton clicks here
    
    switch( event.GetId() ){
        
        case ID_ZOOMIN:
        case ID_ZOOMOUT:
        {
            wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, event.GetId());
            GetParent()->GetEventHandler()->AddPendingEvent( evt );
            break;
        }
     
        default:
            break;
    }        
}


void MUIBar::OnEraseBackground( wxEraseEvent& event )
{
}


void MUIBar::OnPaint( wxPaintEvent& event )
{
    int width, height;
    GetClientSize( &width, &height );
    wxPaintDC dc( this );
   
    dc.SetBackgroundMode(wxTRANSPARENT);
    dc.SetBackground(*wxTRANSPARENT_BRUSH);
    
//     dc.SetPen( *wxTRANSPARENT_PEN );
//     dc.SetBrush( *wxTRANSPARENT_BRUSH );
//     dc.DrawRectangle( 0, 0, width, height);
    
    wxColour backColor = wxColour(40,40,40);
    dc.SetBrush( wxBrush( wxColour(200, 0, 0)) );
    dc.SetPen( wxPen( backColor) );
    dc.DrawRoundedRectangle( 0, 0, width - 10, height - 10, 8 );
}



