// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#include <wx/dcclient.h>

#include "ChInfoWin.h"
#include "chart1.h"

BEGIN_EVENT_TABLE(ChInfoWin, wxWindow)
    EVT_PAINT ( ChInfoWin::OnPaint )
    EVT_ERASE_BACKGROUND(ChInfoWin::OnEraseBackground)
END_EVENT_TABLE()

// Define a constructor
ChInfoWin::ChInfoWin( wxWindow *parent ) :
    wxWindow( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER )
{
    int ststyle = wxALIGN_LEFT | wxST_NO_AUTORESIZE;
    m_pInfoTextCtl = new wxStaticText( this, -1, _T ( "" ), wxDefaultPosition, wxDefaultSize,
                                       ststyle );

    Hide();
}

ChInfoWin::~ChInfoWin()
{
    delete m_pInfoTextCtl;
}

void ChInfoWin::OnEraseBackground( wxEraseEvent& event )
{
}

void ChInfoWin::OnPaint( wxPaintEvent& event )
{
    int width, height;
    GetClientSize( &width, &height );
    wxPaintDC dc( this );

    dc.SetBrush( wxBrush( GetGlobalColor( _T ( "UIBCK" ) ) ) );
    dc.SetPen( wxPen( GetGlobalColor( _T ( "UITX1" ) ) ) );
    dc.DrawRectangle( 0, 0, width, height );
}

void ChInfoWin::SetBitmap()
{
    SetBackgroundColour( GetGlobalColor( _T ( "UIBCK" ) ) );

    m_pInfoTextCtl->SetBackgroundColour( GetGlobalColor( _T ( "UIBCK" ) ) );
    m_pInfoTextCtl->SetForegroundColour( GetGlobalColor( _T ( "UITX1" ) ) );

    m_pInfoTextCtl->SetSize( 1, 1, m_size.x - 2, m_size.y - 2 );
    m_pInfoTextCtl->SetLabel( m_string );

    SetSize( m_position.x, m_position.y, m_size.x, m_size.y );
}

void ChInfoWin::FitToChars( int char_width, int char_height )
{
    wxSize size;

    size.x = GetCharWidth() * char_width;
    size.y = GetCharHeight() * ( char_height + 1 );
    SetWinSize( size );
}

