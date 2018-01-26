/******************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2013 by David S. Register                               *
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

#include "wx/wxprec.h"

#include <wx/slider.h>

#include "DetailSlider.h"
#include "chart1.h"
#include "chcanv.h"
#include "OCPNPlatform.h"
#include "options.h"
#include "chartdb.h"

extern bool        g_bShowDetailSlider;
extern PopUpDSlide *pPopupDetailSlider;
extern int         g_cm93_zoom_factor;
extern int         g_chart_zoom_modifier;
extern int         g_chart_zoom_modifier_vector;
extern int         g_detailslider_dialog_x;
extern int         g_detailslider_dialog_y;
extern ChartCanvas *cc1;
extern MyFrame     *gFrame;
extern bool        g_bQuiltEnable;
extern ChartStack  *pCurrentStack;

BEGIN_EVENT_TABLE(PopUpDSlide, wxDialog)
    EVT_KEY_DOWN(PopUpDSlide::OnKeyDown )
    EVT_MOVE( PopUpDSlide::OnMove )
    EVT_COMMAND_SCROLL_THUMBRELEASE(-1, PopUpDSlide::OnChangeValue)
    EVT_COMMAND_SCROLL_LINEUP(-1, PopUpDSlide::OnChangeValue)
    EVT_COMMAND_SCROLL_LINEDOWN(-1, PopUpDSlide::OnChangeValue)
    EVT_COMMAND_SCROLL_PAGEUP(-1, PopUpDSlide::OnChangeValue)
    EVT_COMMAND_SCROLL_PAGEDOWN(-1, PopUpDSlide::OnChangeValue)
    EVT_COMMAND_SCROLL_BOTTOM(-1, PopUpDSlide::OnChangeValue)
    EVT_COMMAND_SCROLL_TOP(-1, PopUpDSlide::OnChangeValue)
    EVT_CLOSE( PopUpDSlide::OnClose )
END_EVENT_TABLE()

PopUpDSlide::PopUpDSlide( wxWindow *parent, wxWindowID id, ChartTypeEnum ChartT, ChartFamilyEnum ChartF,
                        const wxPoint& pos, const wxSize& size, long style, const wxString& title )
{
    Init();
    Create( parent, ID_CM93ZOOMG, ChartT, ChartF, pos, size, style, title );

    m_p_DetailSlider->Connect( wxEVT_KEY_DOWN, wxKeyEventHandler(PopUpDSlide::OnKeyDown), NULL, this);
}

PopUpDSlide::~PopUpDSlide()
{
    delete m_p_DetailSlider;
}

void PopUpDSlide::Init( void )
{
    m_p_DetailSlider = NULL;
}

bool PopUpDSlide::Create( wxWindow *parent, wxWindowID id, ChartTypeEnum ChartT, ChartFamilyEnum ChartF,
                         const wxPoint& pos, const wxSize& size, long style, const wxString& title )
{
    ChartType = ChartT;
    ChartFam = ChartF;
    wxString WindowText;
    int value;
    if ( (ChartType == CHART_TYPE_CM93COMP ) || (ChartType == CHART_TYPE_CM93 )){
        value = g_cm93_zoom_factor;
        WindowText=wxT("CM93 Detail Level");
        }
        else if ( (ChartType == CHART_TYPE_KAP) || (ChartType == CHART_TYPE_GEO) || (ChartFam == CHART_FAMILY_RASTER ) ){
            value = g_chart_zoom_modifier;
            WindowText=wxT("Rasterchart Zoom/Scale Weighting");
        }
            else if ( (ChartType == CHART_TYPE_S57) || (ChartFam == CHART_FAMILY_VECTOR ) ){
                value = g_chart_zoom_modifier_vector;
                WindowText=wxT("Vectorchart Zoom/Scale Weighting");
            }
            else{ 
                pPopupDetailSlider = NULL;
                return false;                
            }
    
    long wstyle = wxDEFAULT_DIALOG_STYLE;
#ifdef __WXOSX__
    wstyle |= wxSTAY_ON_TOP;
#endif
    
    if( !wxDialog::Create( parent, id, WindowText, pos, size, wstyle ) ) return false;

    m_pparent = parent;
    
int minValue = -5;
int maxValue = 5;
    m_p_DetailSlider = new wxSlider( this, id, value, minValue, maxValue, wxPoint( 0, 0 ),
                                        wxDefaultSize, wxSL_HORIZONTAL | wxSL_AUTOTICKS | wxSL_LABELS, wxDefaultValidator,
                                        WindowText );

    m_p_DetailSlider->SetSize( wxSize( 350, -1 ) );

    m_p_DetailSlider->InvalidateBestSize();
    wxSize bs = m_p_DetailSlider->GetBestSize();

    m_p_DetailSlider->SetSize( wxSize( 350, bs.y ) );
    Fit();

    m_p_DetailSlider->SetValue( value );

    Hide();

    return true;
}

void PopUpDSlide::OnCancelClick( wxCommandEvent& event )
{
    g_bShowDetailSlider = false;
    Close();
}

void PopUpDSlide::OnClose( wxCloseEvent& event )
{
    g_bShowDetailSlider = false;
    
    m_p_DetailSlider->Disconnect( wxEVT_KEY_DOWN, wxKeyEventHandler(PopUpDSlide::OnKeyDown), NULL, this );
    Destroy();
    pPopupDetailSlider = NULL;
}

void PopUpDSlide::OnKeyDown( wxKeyEvent &event )
{
    int key_char = event.GetKeyCode();
    if(key_char == WXK_ESCAPE || key_char == 'D') {
        g_bShowDetailSlider = false;
        Close();
    }
}

void PopUpDSlide::OnMove( wxMoveEvent& event )
{
    //    Record the dialog position
    wxPoint p = event.GetPosition();
    g_detailslider_dialog_x = p.x;
    g_detailslider_dialog_y = p.y;

    event.Skip();
}

void PopUpDSlide::OnChangeValue( wxScrollEvent& event )

{
    ::wxBeginBusyCursor();
    int delta=0;

    if ( (ChartType == CHART_TYPE_CM93COMP ) || (ChartType == CHART_TYPE_CM93 )){
        g_cm93_zoom_factor = m_p_DetailSlider->GetValue();
        cc1->ReloadVP();
        cc1->Refresh();
        ::wxEndBusyCursor();
        return;
    }
 
    if ( (ChartType == CHART_TYPE_KAP) || (ChartType == CHART_TYPE_GEO) || (ChartFam == CHART_FAMILY_RASTER ) ){
        g_chart_zoom_modifier = m_p_DetailSlider->GetValue();            
    }

    if ( (ChartType == CHART_TYPE_S57) || (ChartFam == CHART_FAMILY_VECTOR ) ){
        g_chart_zoom_modifier_vector = m_p_DetailSlider->GetValue();
    }
    
     gFrame->ProcessOptionsDialog( S52_CHANGED | FORCE_UPDATE, NULL);

    ::wxEndBusyCursor();

}
