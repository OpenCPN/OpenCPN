#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/bmpbuttn.h>
#include <wx/dcmemory.h>

#include "TTYWindow.h"
#include "TTYScroll.h"
#include "chart1.h"

extern TTYWindow *g_NMEALogWindow;
extern int g_NMEALogWindow_sx;
extern int g_NMEALogWindow_sy;
extern int g_NMEALogWindow_x;
extern int g_NMEALogWindow_y;

IMPLEMENT_DYNAMIC_CLASS( TTYWindow, wxDialog )

BEGIN_EVENT_TABLE( TTYWindow, wxDialog )
	EVT_CLOSE(TTYWindow::OnCloseWindow)
	EVT_MOVE( TTYWindow::OnMove )
	EVT_SIZE( TTYWindow::OnSize )
END_EVENT_TABLE()

TTYWindow::TTYWindow()
{
}

TTYWindow::TTYWindow(wxWindow *parent, int n_lines)
{
    wxDialog::Create( parent, -1, _T("Title"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER );

    wxBoxSizer* bSizerOuterContainer = new wxBoxSizer( wxVERTICAL );
    SetSizer( bSizerOuterContainer );

    m_pScroll = new TTYScroll(this, n_lines);
    m_pScroll->Scroll(-1, 1000);        // start with full scroll down

    bSizerOuterContainer->Add( m_pScroll, 1, wxEXPAND, 5 );

    wxBoxSizer* bSizerBottomContainer = new wxBoxSizer( wxHORIZONTAL );
    bSizerOuterContainer->Add( bSizerBottomContainer, 0, wxEXPAND, 5 );


    wxStaticBox *psb = new wxStaticBox( this,  wxID_ANY, _("Legend")) ;
    wxStaticBoxSizer* sbSizer1 = new wxStaticBoxSizer( psb , wxVERTICAL );

    CreateLegendBitmap();
    wxBitmapButton *bb = new wxBitmapButton(this, wxID_ANY, m_bm_legend);
    sbSizer1->Add( bb, 1, wxALL|wxEXPAND, 5 );
    bSizerBottomContainer->Add( sbSizer1, 0, wxALIGN_LEFT | wxALL, 5 );

    m_buttonPause = new wxButton( this, wxID_ANY, _("Pause"), wxDefaultPosition, wxDefaultSize, 0 );
    bSizerBottomContainer->Add( m_buttonPause, 0, wxALIGN_RIGHT | wxALL, 5 );

    m_buttonPause->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( TTYWindow::OnPauseClick ), NULL, this );

    bpause = false;
}

TTYWindow::~TTYWindow()
{
    delete m_pScroll;
    g_NMEALogWindow = NULL;
}

void TTYWindow::CreateLegendBitmap()
{
    m_bm_legend.Create(400, 80);
    wxMemoryDC dc;
    dc.SelectObject( m_bm_legend );
    if( m_bm_legend.IsOk()) {

        dc.SetBackground( wxBrush(GetGlobalColor(_T("DILG1"))) );
        dc.Clear();

        wxFont f(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
        dc.SetFont(f);

        int yp = 25;
        int y = 5;

        wxBrush b1(wxColour( _T("DARK GREEN")) );
        dc.SetBrush(b1);
        dc.DrawRectangle( 5, y, 20, 20 );
        dc.SetTextForeground( wxColour(_T("DARK GREEN")) );
        dc.DrawText(  _("Message accepted"), 30, y );

        y += yp;
        wxBrush b2(wxColour( _T("#a0832a")) );
        dc.SetBrush(b2);
        dc.DrawRectangle( 5, y, 20, 20 );
        dc.SetTextForeground( wxColour(_T("#a0832a")) );
        dc.DrawText(  _("Message filtered and dropped"), 30, y );

        y += yp;
        wxBrush b3(wxColour( _T("BLUE")) );
        dc.SetBrush(b3);
        dc.DrawRectangle( 5, y, 20, 20 );
        dc.SetTextForeground( wxColour(_T("BLUE")) );
        dc.DrawText(  _("Output Message"), 30, y );
    }
    dc.SelectObject( wxNullBitmap );
}




void TTYWindow::OnPauseClick( wxCommandEvent& event )
{
    if(!bpause) {
        bpause = true;
        m_pScroll->Pause( true );
        m_buttonPause->SetLabel( _("Resume") );
    } else {
        bpause = false;
        m_pScroll->Pause( false );

        m_buttonPause->SetLabel( _("Pause") );
    }
}


void TTYWindow::OnCloseWindow( wxCloseEvent& event )

{
    Destroy();
}

void TTYWindow::OnSize( wxSizeEvent& event )
{
    //    Record the dialog size
    wxSize p = event.GetSize();
    g_NMEALogWindow_sx = p.x;
    g_NMEALogWindow_sy = p.y;

    event.Skip();
}

void TTYWindow::OnMove( wxMoveEvent& event )
{
    //    Record the dialog position
    wxPoint p = event.GetPosition();
    g_NMEALogWindow_x = p.x;
    g_NMEALogWindow_y = p.y;

    event.Skip();
}

void TTYWindow::Add( wxString &line )
{
    if( m_pScroll ) m_pScroll->Add( line );
}

