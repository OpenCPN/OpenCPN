#include <wx/dcclient.h>

#include "TTYScroll.h"

TTYScroll::TTYScroll(wxWindow *parent, int n_lines)
	: wxScrolledWindow(parent), m_nLines( n_lines )
{
	bpause = false;
	wxClientDC dc(this);
	dc.GetTextExtent(_T("Line Height"), NULL, &m_hLine);

	SetScrollRate( 0, m_hLine );
	SetVirtualSize( -1, ( m_nLines + 1 ) * m_hLine );
	m_plineArray = new wxArrayString;
	for(unsigned int i=0 ; i < m_nLines ; i++)
		m_plineArray->Add(_T(""));
}

TTYScroll::~TTYScroll()
{
	delete m_plineArray;
}

void TTYScroll::Add( wxString &line )
{
    if(!bpause) {
        if( m_plineArray->GetCount() > m_nLines - 1 ) {                       // shuffle the arraystring
            wxArrayString *p_newArray = new wxArrayString;

            for( unsigned int i = 1; i < m_plineArray->GetCount(); i++ )
                p_newArray->Add( m_plineArray->Item( i ) );

            delete m_plineArray;
            m_plineArray = p_newArray;
        }

    m_plineArray->Add( line );
    Refresh( true );
    }
}

void TTYScroll::OnDraw( wxDC& dc )
{
    // update region is always in device coords, translate to logical ones
    wxRect rectUpdate = GetUpdateRegion().GetBox();
    CalcUnscrolledPosition( rectUpdate.x, rectUpdate.y, &rectUpdate.x, &rectUpdate.y );

    size_t lineFrom = rectUpdate.y / m_hLine, lineTo = rectUpdate.GetBottom() / m_hLine;

    if( lineTo > m_nLines - 1 ) lineTo = m_nLines - 1;

    wxCoord y = lineFrom * m_hLine;
    wxString lss;
    for( size_t line = lineFrom; line <= lineTo; line++ ) {
        wxCoord yPhys;
        CalcScrolledPosition( 0, y, NULL, &yPhys );

        wxString ls = m_plineArray->Item( line );
        if(ls.Mid(0, 7) == _T("<GREEN>") ){
            dc.SetTextForeground( wxColour(_T("DARK GREEN")) );
            lss = ls.Mid(7);
        }
        else if(ls.Mid(0, 7) == _T("<AMBER>") ){
            dc.SetTextForeground( wxColour(_T("#a0832a")) );
            lss = ls.Mid(7);
        }
        else if(ls.Mid(0, 6) == _T("<BLUE>") ){
                dc.SetTextForeground( wxColour(_T("BLUE")) );
                lss = ls.Mid(6);
        }
        else if(ls.Mid(0, 5) == _T("<RED>") ){
            dc.SetTextForeground( wxColour(_T("RED")) );
            lss = ls.Mid(5);
        }

        dc.DrawText( lss, 0, y );
       y += m_hLine;
    }
}


