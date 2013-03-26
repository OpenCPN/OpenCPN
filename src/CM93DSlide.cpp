// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#include <wx/slider.h>

#include "CM93DSlide.h"
#include "chart1.h"
#include "chcanv.h"

extern bool g_bShowCM93DetailSlider;
extern CM93DSlide *pCM93DetailSlider;
extern int g_cm93_zoom_factor;
extern int g_cm93detail_dialog_x;
extern int g_cm93detail_dialog_y;
extern ChartCanvas *cc1;

BEGIN_EVENT_TABLE(CM93DSlide, wxDialog)
    EVT_MOVE( CM93DSlide::OnMove )
    EVT_COMMAND_SCROLL_THUMBRELEASE(-1, CM93DSlide::OnChangeValue)
    EVT_COMMAND_SCROLL_LINEUP(-1, CM93DSlide::OnChangeValue)
    EVT_COMMAND_SCROLL_LINEDOWN(-1, CM93DSlide::OnChangeValue)
    EVT_COMMAND_SCROLL_PAGEUP(-1, CM93DSlide::OnChangeValue)
    EVT_COMMAND_SCROLL_PAGEDOWN(-1, CM93DSlide::OnChangeValue)
    EVT_COMMAND_SCROLL_BOTTOM(-1, CM93DSlide::OnChangeValue)
    EVT_COMMAND_SCROLL_TOP(-1, CM93DSlide::OnChangeValue)
    EVT_CLOSE( CM93DSlide::OnClose )
END_EVENT_TABLE()

CM93DSlide::CM93DSlide( wxWindow *parent, wxWindowID id, int value, int minValue, int maxValue,
                        const wxPoint& pos, const wxSize& size, long style, const wxString& title )
{
    Init();
    Create( parent, ID_CM93ZOOMG, value, minValue, maxValue, pos, size, style, title );
}

CM93DSlide::~CM93DSlide()
{
    delete m_pCM93DetailSlider;
}

void CM93DSlide::Init( void )
{
    m_pCM93DetailSlider = NULL;
}

bool CM93DSlide::Create( wxWindow *parent, wxWindowID id, int value, int minValue, int maxValue,
                         const wxPoint& pos, const wxSize& size, long style, const wxString& title )
{
    if( !wxDialog::Create( parent, id, title, pos, size, wxDEFAULT_DIALOG_STYLE ) ) return false;

    m_pparent = parent;

    m_pCM93DetailSlider = new wxSlider( this, id, value, minValue, maxValue, wxPoint( 0, 0 ),
                                        wxDefaultSize, wxSL_HORIZONTAL | wxSL_AUTOTICKS | wxSL_LABELS, wxDefaultValidator,
                                        title );

    m_pCM93DetailSlider->SetSize( wxSize( 200, -1 ) );

    m_pCM93DetailSlider->InvalidateBestSize();
    wxSize bs = m_pCM93DetailSlider->GetBestSize();

    m_pCM93DetailSlider->SetSize( wxSize( 200, bs.y ) );
    Fit();

    m_pCM93DetailSlider->SetValue( g_cm93_zoom_factor );

    Hide();

    return true;
}

void CM93DSlide::OnCancelClick( wxCommandEvent& event )
{
    g_bShowCM93DetailSlider = false;
    Close();
}

void CM93DSlide::OnClose( wxCloseEvent& event )
{
    g_bShowCM93DetailSlider = false;

    Destroy();
    pCM93DetailSlider = NULL;
}

void CM93DSlide::OnMove( wxMoveEvent& event )
{
    //    Record the dialog position
    wxPoint p = event.GetPosition();
    g_cm93detail_dialog_x = p.x;
    g_cm93detail_dialog_y = p.y;

    event.Skip();
}

void CM93DSlide::OnChangeValue( wxScrollEvent& event )
{
    g_cm93_zoom_factor = m_pCM93DetailSlider->GetValue();

    ::wxBeginBusyCursor();

    cc1->ReloadVP();
    cc1->Refresh( false );

    ::wxEndBusyCursor();
}

