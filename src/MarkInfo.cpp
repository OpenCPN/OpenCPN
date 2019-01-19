/**************************************************************************
*
* Project:  OpenCPN
* Purpose:  MarkProperties Support
* Author:   David Register
*
***************************************************************************
*   Copyright (C) 2010 by David S. Register                               *
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
**************************************************************************/

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/datetime.h>
#include <wx/clipbrd.h>
#include <wx/print.h>
#include <wx/printdlg.h>
#include <wx/stattext.h>
#include <wx/clrpicker.h>
#include <wx/bmpbuttn.h>

#include "styles.h"
#include "MarkInfo.h"
#include "navutil.h"                // for Route
#include "georef.h"
#include "chart1.h"
#include "routeman.h"
#include "routemanagerdialog.h"
#include "routeprintout.h"
#include "RoutePropDlgImpl.h"
#include "chcanv.h"
#include "PositionParser.h"
#include "pluginmanager.h"
#include "OCPNPlatform.h"
#include "Route.h"
#include "chart1.h"
#include "TCWin.h"

#ifdef __OCPN__ANDROID__
#include "androidUTIL.h"
#include <QtWidgets/QScroller>
#endif


extern double             gLat, gLon, gSog, gCog;
extern TCMgr              *ptcmgr;
extern long               gStart_LMT_Offset;
extern MyConfig           *pConfig;
extern WayPointman        *pWayPointMan;
extern Select             *pSelect;
extern Routeman           *g_pRouteMan;
extern RouteManagerDialog *pRouteManagerDialog;
extern RoutePropDlgImpl          *pRoutePropDialog;
extern Track              *g_pActiveTrack;
extern RouteList          *pRouteList;
extern PlugInManager      *g_pi_manager;
extern bool                g_bShowTrue, g_bShowMag;

extern MyFrame            *gFrame;
extern OCPNPlatform       *g_Platform;
extern wxString            g_default_wp_icon;

// Global print data, to remember settings during the session
extern wxPrintData        *g_printData;

// Global page setup data
extern wxPageSetupData*   g_pageSetupData;

extern float              g_ChartScaleFactorExp;
extern int                g_GUIScaleFactor;
extern int                g_iDistanceFormat;
extern int                g_iSpeedFormat;
extern MarkInfoDlg       *g_pMarkInfoDialog;
extern double             g_n_arrival_circle_radius;
extern int                g_iWaypointRangeRingsNumber;
extern float              g_fWaypointRangeRingsStep;
extern int                g_iWaypointRangeRingsStepUnits;
extern wxColour           g_colourWaypointRangeRingsColour;

extern int                g_iWpt_ScaMin;
extern bool               g_bUseWptScaMin;
extern bool               g_bShowWptName;

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

WX_DECLARE_LIST(wxBitmap, BitmapList);
#include <wx/listimpl.cpp>
WX_DEFINE_LIST(BitmapList);

#include <wx/arrimpl.cpp> 
#include <chart1.h>
WX_DEFINE_OBJARRAY(ArrayOfBitmaps);

#define EXTENDED_PROP_PAGE 2 // Index of the extended properties page


OCPNIconCombo::OCPNIconCombo (wxWindow* parent, wxWindowID id, const wxString& value,
                                  const wxPoint& pos, const wxSize& size, int n, const wxString choices[],
                                  long style, const wxValidator& validator, const wxString& name)
                        :wxOwnerDrawnComboBox(parent, id, value, pos, size, n, choices, style, validator, name)
{
    double fontHeight = GetFont().GetPointSize() / g_Platform->getFontPointsperPixel();
    itemHeight = (int)wxRound(fontHeight);
    
}

OCPNIconCombo::~OCPNIconCombo ()
{
}

void OCPNIconCombo::OnDrawItem( wxDC& dc,
                                       const wxRect& rect,
                                       int item,
                                       int flags ) const
{
    
    int offset_x = bmpArray[item].GetWidth();
    int bmpHeight = bmpArray[item].GetHeight();
    dc.DrawBitmap(bmpArray[item], rect.x, rect.y + (rect.height - bmpHeight)/2, true);
    
    if ( flags & wxODCB_PAINTING_CONTROL )
    {
        wxString text = GetValue();
        int margin_x = 2;
        
#if wxCHECK_VERSION(2, 9, 0)
        if ( ShouldUseHintText() )
        {
            text = GetHint();
            wxColour col = wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT);
            dc.SetTextForeground(col);
        }
        
        margin_x = GetMargins().x;
#endif

        dc.DrawText( text,
                     rect.x + margin_x + offset_x,
                     (rect.height-dc.GetCharHeight())/2 + rect.y );
    }
    else
    {
        dc.DrawText( GetVListBoxComboPopup()->GetString(item), rect.x + 2 + offset_x, (rect.height-dc.GetCharHeight())/2 + rect.y );
    }
}

wxCoord OCPNIconCombo::OnMeasureItem( size_t item ) const
{
    int bmpHeight = bmpArray[item].GetHeight();
    
    return wxMax(itemHeight, bmpHeight);
}

wxCoord OCPNIconCombo::OnMeasureItemWidth( size_t item ) const
{
    return -1;
}

int OCPNIconCombo::Append(const wxString& item, wxBitmap bmp)
{
    bmpArray.Add(bmp);
    int idx = wxOwnerDrawnComboBox::Append(item);
    
    return idx;
}

void OCPNIconCombo::Clear( void )
{
    wxOwnerDrawnComboBox::Clear();
    bmpArray.Clear();
}

//-------------------------------------------------------------------------------
//
//    Mark Properties Dialog Implementation
//
//-------------------------------------------------------------------------------
/*!
 * MarkProp type definition
 */

//DEFINE_EVENT_TYPE(EVT_LLCHANGE)           // events from LatLonTextCtrl
const wxEventType EVT_LLCHANGE = wxNewEventType();
//------------------------------------------------------------------------------
//    LatLonTextCtrl Window Implementation
//------------------------------------------------------------------------------
BEGIN_EVENT_TABLE(LatLonTextCtrl, wxWindow)
END_EVENT_TABLE()

// constructor
LatLonTextCtrl::LatLonTextCtrl( wxWindow* parent, wxWindowID id, const wxString& value,
        const wxPoint& pos, const wxSize& size, long style, const wxValidator& validator,
        const wxString& name ) :
        wxTextCtrl( parent, id, value, pos, size, style, validator, name )
{
    m_pParentEventHandler = parent->GetEventHandler();
}

void LatLonTextCtrl::OnKillFocus( wxFocusEvent& event )
{
    //    Send an event to the Parent Dialog
    wxCommandEvent up_event( EVT_LLCHANGE, GetId() );
    up_event.SetEventObject( (wxObject *) this );
    m_pParentEventHandler->AddPendingEvent( up_event );
}

//-------------------------------------------------------------------------------
//
//    Mark Information Dialog Implementation
//
//-------------------------------------------------------------------------------
BEGIN_EVENT_TABLE( MarkInfoDlg, wxFrame )
     EVT_BUTTON( wxID_OK, MarkInfoDlg::OnMarkInfoOKClick)
     EVT_BUTTON( wxID_CANCEL, MarkInfoDlg::OnMarkInfoCancelClick)
     EVT_BUTTON( ID_BTN_DESC_BASIC, MarkInfoDlg::OnExtDescriptionClick)
     EVT_BUTTON( ID_DEFAULT, MarkInfoDlg::DefautlBtnClicked)
     EVT_BUTTON( ID_BTN_SHOW_TIDES, MarkInfoDlg::ShowTidesBtnClicked)
     EVT_COMBOBOX(ID_BITMAPCOMBOCTRL, MarkInfoDlg::OnBitmapCombClick)
     EVT_CHECKBOX( ID_SHOWNAMECHECKBOXBASIC, MarkInfoDlg::OnShowWaypointNameSelectBasic )
     EVT_CHECKBOX( ID_SHOWNAMECHECKBOX_EXT, MarkInfoDlg::OnShowWaypointNameSelectExt )
     EVT_CHECKBOX( ID_CHECKBOX_SCAMIN_VIS, MarkInfoDlg::OnSelectScaMinExt )
     EVT_TEXT(ID_DESCR_CTR_DESC, MarkInfoDlg::OnDescChangedExt )
     EVT_TEXT(ID_DESCR_CTR_BASIC, MarkInfoDlg::OnDescChangedBasic )
     EVT_TEXT(ID_LATCTRL, MarkInfoDlg::OnPositionCtlUpdated )
     EVT_TEXT(ID_LONCTRL, MarkInfoDlg::OnPositionCtlUpdated )
     EVT_SPINCTRL(ID_WPT_RANGERINGS_NO, MarkInfoDlg::OnWptRangeRingsNoChange)
     // the HTML listbox's events
     EVT_HTML_LINK_CLICKED(wxID_ANY, MarkInfoDlg::OnHtmlLinkClicked)
     EVT_CLOSE(MarkInfoDlg::OnClose)
     
     //EVT_CHOICE( ID_WAYPOINTRANGERINGS, MarkInfoDef::OnWaypointRangeRingSelect )
END_EVENT_TABLE()

MarkInfoDlg::MarkInfoDlg( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style )
{    
    long wstyle = style;
#ifdef __WXOSX__
    wstyle |= wxSTAY_ON_TOP;
#endif
    
    wxFrame::Create( parent, id, title, pos, size, wstyle );

    wxFont *qFont = GetOCPNScaledFont(_("Dialog"));
    SetFont( *qFont );
    int metric = GetCharHeight();
    
#ifdef __OCPN__ANDROID__
    //  Set Dialog Font by custom crafted Qt Stylesheet.
    wxString wqs = getFontQtStylesheet(qFont);
    wxCharBuffer sbuf = wqs.ToUTF8();
    QString qsb = QString(sbuf.data());    
    QString qsbq = getQtStyleSheet();          // basic scrollbars, etc    
    this->GetHandle()->setStyleSheet( qsb + qsbq );      // Concatenated style sheets    
    wxScreenDC sdc;
    if(sdc.IsOk())
        sdc.GetTextExtent(_T("W"), NULL, &metric, NULL, NULL, qFont);
#endif
    Create();
    m_pMyLinkList = NULL;
    SetColorScheme( (ColorScheme) 0 );
}


void MarkInfoDlg::initialize_images(void)
{  // TODO use svg instead of bitmap
	{
		wxMemoryInputStream sm("\211PNG\r\n\032\n\000\000\000\rIHDR\000\000\000\030\000\000\000\030\b\002\000\000\000o\025\252\257\000\000\000\006bKGD\000\377\000\377\000\377\240\275\247\223\000\000\003.IDAT8\215\255\225\313O\352L\030\306\337\266\230\212\244\325\002\006k\214Z\253\201\205\267\270\360F$\321\260\320\304\177\225\235\013bX!1\310\016E\264\255!\210\030n\205\332r\251\205\322\371\026\223\234O\214\3509\211\317\252\351;\363\364\367^:C \204\3407D\376\212\013\000\270\276\211!\204.//\337\336\336\000`vv\366\344\344\344\337\2104M\303\017\345r\271\327\353\035\034\034\354\354\354\350\272\336h4\360{UU\177&\272\271\271\251V\2534M\213\242\370\362\362211\261\270\270\b\000\271\\.\227\313\315\317\317?==\r\006\203\271\271\271\375\375\375\261D\331l\266V\253\205B!\267\333}\177\177\3578\316\306\306\006\016moo\017\207\303|>\3170\314\372\372z\275^\317f\263c\2114M\3438.\030\014\006\203A\3030X\226\375\023\022\004A\020\204n\267\353\361x\000\340\365\365\3250\214\261D\242(6\233\315\347\347g\000\300.\212\242d2\231L&\243(\n\000`\227B\241\240i\232(\212c\211\002\201\000A\020\235N\a\000\336\337\337\223\311\244eY\014\303\000@\255V+\026\213\221H\204\246i\3234\011\202\360\373\375\037\367\022x -\313\222$IU\325v\273}zzJ\323t*\225\352t:{{{\034\307\341\254\323\3514\313\262\341p\330\262\254x<\356\361x|>\037.\350\377\251\335\335\335\225\313\345\311\311\311\315\315M\374MUUWWW\261\013\000p\034\267\266\266\326h4L\323\244izkk\213e\331j\265z{{;R#\313\262\274^\357\341\341\241 \b\000\320j\265\000\200\347\371\217\360<\317#\204phyyyww\327\353\365\332\266=bDQT\273\335\226$\011\3171\313\262\004A|\032<UUI\222\304Mh6\233\371|\3360\014\202 F\214VVV(\212\222e\371\352\352\312q\034\206aX\226\225e\271\337\357\343\005\375~_\226\345\231\231\031\206a\034\307\271\276\276.\024\nn\267;\024\n\215\024\033\253\333\355&\022\211P(\024\014\006u]O\245R\004A\004\002\001\204P\275^\a\200\243\243#\206adY~xx\210F\243x\032\276h\277\3438$IZ\226\005\000\323\323\323\307\307\307\217\217\2178Y\236\347\3774h0\030\220$9\030\014\276h?V:\2356\014#\032\215\222\344\017\307K\"\221p\273\335\341p\370k\"\212\242\206\303\241\256\353\225J\245X,\372\375~Q\024}>\037\256\256\242(\252\252.,,,--Y\226\365\361\a\372Ld\333v2\231\354t:\b!\236\347[\255\226m\333\347\347\347\000pqq\341r\271\374~\177\245R\301\335\210D\".\327\a\0164*\333\266%I\3224\r!T*\225b\261\230i\232\246i\306b\261R\251\204\0202\014CQ\224\341p\370i\343\b\321'\331\266\035\217\307q\355i\232>;;\243(j\334\342\357\214\000\240\327\353\341\003\223\343\270\251\251\251oV\376`\364\367\372\265[\344?Y\017\315\3503.x<\000\000\000\000IEND\256B`\202", 889);
		_img_MUI_settings_svg = new wxBitmap(wxImage(sm));
	}
	return;
}

void MarkInfoDlg::Create()
{
    //(*Initialize(waypoint_propertiesDialog)
    
    bMainSizer = new wxBoxSizer( wxVERTICAL );
     
    m_notebookProperties = new wxNotebook(this, wxID_ANY);
    m_PanelBasicProperties = new wxPanel( m_notebookProperties, wxID_ANY, wxDefaultPosition, wxDefaultSize,  wxTAB_TRAVERSAL);
    #ifdef __OCPN__ANDROID__
        m_PanelBasicProperties->GetHandle()->setStyleSheet( getQtStyleSheet());
    #endif
    fSizerBasicProperties = new wxFlexGridSizer(0, 1, 0, 0);
    
//////////////////////////////////////////////////BASIC/////////////////////////////////////////////////
    
    sbSizerBasicProperties = new wxStaticBoxSizer(wxVERTICAL, m_PanelBasicProperties, _("Properties"));
    m_staticTextLayer = new wxStaticText(m_PanelBasicProperties, wxID_ANY, _("This waypoint is part of a layer and can\'t be edited"));
    sbSizerBasicProperties->Add(m_staticTextLayer, 0, wxALL|wxEXPAND, 5);

    gbSizerInnerProperties = new wxFlexGridSizer(3, 0, 0);
    gbSizerInnerProperties->AddGrowableCol(2);

    m_checkBoxShowName = new wxCheckBox(m_PanelBasicProperties, ID_SHOWNAMECHECKBOXBASIC, wxEmptyString);
    m_checkBoxShowName->SetValue(false);
    gbSizerInnerProperties->Add(m_checkBoxShowName, 0, wxALIGN_CENTRE_VERTICAL, 0);
    m_staticTextName = new wxStaticText(m_PanelBasicProperties, wxID_ANY, _("Name"));
    gbSizerInnerProperties->Add(m_staticTextName, 0, wxALIGN_CENTRE_VERTICAL, 0);
    m_textName = new wxTextCtrl(m_PanelBasicProperties, ID_NAMECTRL);
    gbSizerInnerProperties->Add(m_textName, 0, wxALL|wxEXPAND, 5);

    gbSizerInnerProperties->Add( 0, 0, 1, wxEXPAND, 0 );
    m_staticTextIcon = new wxStaticText(m_PanelBasicProperties, wxID_ANY, _("Icon"));
    gbSizerInnerProperties->Add(m_staticTextIcon, 0, wxALIGN_CENTRE_VERTICAL, 0);
    m_bcomboBoxIcon = new OCPNIconCombo( m_PanelBasicProperties, ID_BITMAPCOMBOCTRL, wxEmptyString,                                      wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY );
    m_bcomboBoxIcon->SetPopupMaxHeight(::wxGetDisplaySize().y / 2);        
    //  Accomodate scaling of icon
    int min_size = GetCharHeight() * 2;
    min_size = wxMax( min_size, (32 *g_ChartScaleFactorExp) + 4 );
    m_bcomboBoxIcon->SetMinSize( wxSize(-1, min_size) );
    gbSizerInnerProperties->Add(m_bcomboBoxIcon, 0, wxALL|wxEXPAND, 5);
    
    gbSizerInnerProperties->Add( 0, 0, 1, wxEXPAND, 0 );
    m_staticTextLatitude = new wxStaticText(m_PanelBasicProperties, wxID_ANY, _("Latitude"));
    gbSizerInnerProperties->Add(m_staticTextLatitude, 0, wxALIGN_CENTRE_VERTICAL, 0);
    m_textLatitude = new wxTextCtrl(m_PanelBasicProperties, wxID_ANY);
    gbSizerInnerProperties->Add(m_textLatitude, 0, wxALL|wxEXPAND, 5);
    
    gbSizerInnerProperties->Add( 0, 0, 1, wxEXPAND, 5 );
    m_staticTextLongitude = new wxStaticText(m_PanelBasicProperties, wxID_ANY, _("Longitude"));
    gbSizerInnerProperties->Add(m_staticTextLongitude, 0, wxALIGN_CENTRE_VERTICAL, 0);
    m_textLongitude = new wxTextCtrl(m_PanelBasicProperties, wxID_ANY);
    gbSizerInnerProperties->Add(m_textLongitude, 0, wxALL|wxEXPAND, 5);
    
    sbS_Description = new wxStaticBoxSizer( wxHORIZONTAL, m_PanelBasicProperties, _("Description"));
    m_textDescription = new wxTextCtrl( sbS_Description->GetStaticBox(), ID_DESCR_CTR_BASIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE );
    sbS_Description->Add( m_textDescription, 1, wxBOTTOM|wxLEFT|wxEXPAND, 5 );
    m_buttonExtDescription = new wxButton( sbS_Description->GetStaticBox(), ID_BTN_DESC_BASIC, _T("\u2261"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
    sbS_Description->Add( m_buttonExtDescription, 0, wxEXPAND, 5 );
    sbS_Description->SetSizeHints( sbS_Description->GetStaticBox() );
    
    wxStaticBoxSizer* sbS_SizerLinks1 = new wxStaticBoxSizer(wxHORIZONTAL, m_PanelBasicProperties, _("Links")  );
    m_htmlList = new  	wxSimpleHtmlListBox(sbS_SizerLinks1->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);
    sbS_SizerLinks1->Add(m_htmlList, 1,  wxBOTTOM|wxLEFT|wxEXPAND, 5 );

    sbS_SizerLinks1->SetSizeHints( sbS_Description->GetStaticBox() );
  
    sbSizerBasicProperties->Add(gbSizerInnerProperties, 1, wxEXPAND, 5);
    sbSizerBasicProperties->Add(sbS_Description, 1, wxEXPAND, 5);
    sbSizerBasicProperties->Add(sbS_SizerLinks1, 1, wxEXPAND, 5);
    m_PanelBasicProperties->Layout();

        
//////////////////////////////////////////////////////DESCRIPTION ///////////////////////////////////////////////////////
        
    m_PanelDescription = new wxPanel(m_notebookProperties, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    sbSizerDescription = new wxStaticBoxSizer(wxVERTICAL, m_PanelDescription, _("Description"));
    m_textCtrlExtDescription = new wxTextCtrl( sbSizerDescription->GetStaticBox(), ID_DESCR_CTR_DESC, _("wxEmptyString "),
            wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE );
    sbSizerDescription->Add( m_textCtrlExtDescription, 1, wxALL | wxEXPAND, 5 );
    
    
/////////////////////////////////////////////////////// EXTENDED ///////////////////////////////////////////////////////
    
    m_PanelExtendedProperties = new wxPanel(m_notebookProperties, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    wxBoxSizer* fSizerExtProperties = new wxBoxSizer(wxVERTICAL);
    sbSizerExtProperties = new wxStaticBoxSizer(wxVERTICAL, m_PanelExtendedProperties, _("Extended Properties"));
    wxFlexGridSizer* gbSizerInnerExtProperties = new wxFlexGridSizer(3, 0, 0);
    gbSizerInnerExtProperties->AddGrowableCol(2);
    gbSizerInnerExtProperties->SetFlexibleDirection( wxBOTH );
    gbSizerInnerExtProperties->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    m_checkBoxVisible = new wxCheckBox(sbSizerExtProperties->GetStaticBox(), ID_CHECKBOX_VIS_EXT, wxEmptyString);
    gbSizerInnerExtProperties->Add(m_checkBoxVisible);
    wxStaticText* m_staticTextVisible = new wxStaticText(sbSizerExtProperties->GetStaticBox(), wxID_ANY, _("Show on chart"));
    gbSizerInnerExtProperties->Add(m_staticTextVisible);
    gbSizerInnerExtProperties->Add( 0, 0, 1, wxEXPAND, 0 );
    
    m_checkBoxScaMin = new wxCheckBox(sbSizerExtProperties->GetStaticBox(), ID_CHECKBOX_SCAMIN_VIS, wxEmptyString);
    gbSizerInnerExtProperties->Add(m_checkBoxScaMin, 0, wxALIGN_CENTRE_VERTICAL, 0);
    m_staticTextScaMin = new wxStaticText(sbSizerExtProperties->GetStaticBox(), wxID_ANY, _("Show at scale > 1 :"));
    gbSizerInnerExtProperties->Add(m_staticTextScaMin, 0, wxALIGN_CENTRE_VERTICAL, 0);
    m_textScaMin = new wxTextCtrl(sbSizerExtProperties->GetStaticBox(), wxID_ANY);
    gbSizerInnerExtProperties->Add(m_textScaMin, 0, wxALL|wxEXPAND, 5);
    
    m_checkBoxShowNameExt = new wxCheckBox(sbSizerExtProperties->GetStaticBox(), ID_SHOWNAMECHECKBOX_EXT, wxEmptyString);
    gbSizerInnerExtProperties->Add(m_checkBoxShowNameExt);
    m_staticTextShowNameExt = new wxStaticText(sbSizerExtProperties->GetStaticBox(), wxID_ANY, _("Show waypoint name"));
    gbSizerInnerExtProperties->Add(m_staticTextShowNameExt);
    gbSizerInnerExtProperties->Add( 0, 0, 1, wxEXPAND, 0 );
    
    sbRangeRingsExtProperties = new wxStaticBoxSizer(wxVERTICAL, sbSizerExtProperties->GetStaticBox(), _("Range rings"));
    wxFlexGridSizer* gbRRExtProperties = new wxFlexGridSizer(4, 0, 0);
    gbRRExtProperties->AddGrowableCol(0);
    gbRRExtProperties->AddGrowableCol(1);
    gbRRExtProperties->AddGrowableCol(3);
    m_staticTextRR1 = new wxStaticText( sbSizerExtProperties->GetStaticBox(), wxID_ANY, _("Number"));
    gbRRExtProperties->Add(m_staticTextRR1, 0, wxLEFT, 5);
    m_staticTextRR2 = new wxStaticText( sbSizerExtProperties->GetStaticBox(), wxID_ANY, _("Distance"));
    gbRRExtProperties->Add(m_staticTextRR2, 0, wxLEFT, 5);
    gbRRExtProperties->Add( 0, 0, 1, wxEXPAND, 5 ); // a spacer
    m_staticTextRR4 = new wxStaticText( sbSizerExtProperties->GetStaticBox(), wxID_ANY, _("Color"));
    gbRRExtProperties->Add(m_staticTextRR4, 0, wxLEFT, 5);
    
    m_SpinWaypointRangeRingsNumber = new wxSpinCtrl( sbSizerExtProperties->GetStaticBox(), ID_WPT_RANGERINGS_NO, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_VERTICAL, 0, 10, 0);
    gbRRExtProperties->Add(m_SpinWaypointRangeRingsNumber, 0, wxALL|wxEXPAND, 5);
    m_textWaypointRangeRingsStep = new wxTextCtrl(sbSizerExtProperties->GetStaticBox(), wxID_ANY, _("0.05"), wxDefaultPosition, wxDefaultSize, 0);
    gbRRExtProperties->Add(m_textWaypointRangeRingsStep, 0, wxALL|wxEXPAND, 5);
    m_staticTextRR3 = new wxStaticText( sbSizerExtProperties->GetStaticBox(), wxID_ANY, getUsrDistanceUnit());
    gbRRExtProperties->Add(m_staticTextRR3, 0, wxALIGN_CENTRE_VERTICAL|wxALIGN_LEFT, 0);
    
    m_PickColor = new wxColourPickerCtrl(sbSizerExtProperties->GetStaticBox(), wxID_ANY, wxColour(0,0,0), wxDefaultPosition, wxDefaultSize, 0);
    gbRRExtProperties->Add(m_PickColor, 0, wxALL|wxEXPAND, 5);
    sbRangeRingsExtProperties->Add(gbRRExtProperties, 1, wxLEFT|wxTOP|wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5);

    sbSizerExtProperties->GetStaticBox()->Layout();

    wxFlexGridSizer* gbSizerInnerExtProperties2 = new wxFlexGridSizer(2, 0, 0);
    gbSizerInnerExtProperties2->AddGrowableCol(1);
    
    m_staticTextGuid = new wxStaticText( sbSizerExtProperties->GetStaticBox(), wxID_ANY, _("GUID"),
                                        wxDefaultPosition, wxDefaultSize, 0 );
    gbSizerInnerExtProperties2->Add(m_staticTextGuid, 0, wxALIGN_CENTRE_VERTICAL, 0);
    m_textCtrlGuid = new wxTextCtrl( sbSizerExtProperties->GetStaticBox(), wxID_ANY, wxEmptyString,
                                    wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
    m_textCtrlGuid->SetEditable(false);
    gbSizerInnerExtProperties2->Add(m_textCtrlGuid, 0, wxALL|wxEXPAND, 5);
    
    wxFlexGridSizer* gbSizerInnerExtProperties1 = new wxFlexGridSizer(3, 0, 0);
    gbSizerInnerExtProperties1->AddGrowableCol(1);
    
    m_staticTextTideStation = new wxStaticText( sbSizerExtProperties->GetStaticBox(), wxID_ANY, _("Tide Station"),
                                               wxDefaultPosition, wxDefaultSize, 0 );
    gbSizerInnerExtProperties1->Add(m_staticTextTideStation, 0, wxALIGN_CENTRE_VERTICAL, 5);
    
    m_comboBoxTideStation = new wxComboBox( sbSizerExtProperties->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY);
    gbSizerInnerExtProperties1->Add(m_comboBoxTideStation, 0, wxALL|wxEXPAND|wxALIGN_CENTRE_VERTICAL, 5);
    
    m_buttonShowTides = new wxButton( sbSizerExtProperties->GetStaticBox(), ID_BTN_SHOW_TIDES, _("\u223f"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
    gbSizerInnerExtProperties1->Add(m_buttonShowTides, 0, wxALL|wxALIGN_CENTRE_VERTICAL, 5);
    
    m_staticTextArrivalRadius = new wxStaticText( sbSizerExtProperties->GetStaticBox(), wxID_ANY, _("Arrival Radius") );
    gbSizerInnerExtProperties1->Add(m_staticTextArrivalRadius, 0, wxALIGN_CENTRE_VERTICAL, 0);
    m_textArrivalRadius = new wxTextCtrl( sbSizerExtProperties->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    gbSizerInnerExtProperties1->Add(m_textArrivalRadius, 0, wxALL|wxEXPAND, 5);
    m_staticTextArrivalUnits = new wxStaticText( sbSizerExtProperties->GetStaticBox(), wxID_ANY, wxEmptyString,
            wxDefaultPosition, wxDefaultSize, 0 );
    gbSizerInnerExtProperties1->Add(m_staticTextArrivalUnits, 0, wxALIGN_CENTRE_VERTICAL, 0);

    m_staticTextPlSpeed = new wxStaticText( sbSizerExtProperties->GetStaticBox(), wxID_ANY, _("Planned Speed"),
            wxDefaultPosition, wxDefaultSize, 0 );
    gbSizerInnerExtProperties1->Add(m_staticTextPlSpeed, 0, wxALIGN_CENTRE_VERTICAL, 0);
    m_textCtrlPlSpeed = new wxTextCtrl( sbSizerExtProperties->GetStaticBox(), wxID_ANY, wxEmptyString,
            wxDefaultPosition, wxDefaultSize, 0 );
    gbSizerInnerExtProperties1->Add(m_textCtrlPlSpeed, 0, wxALL|wxEXPAND, 5);
    m_staticTextPlSpeedUnits = new wxStaticText( sbSizerExtProperties->GetStaticBox(), wxID_ANY, getUsrSpeedUnit(),
            wxDefaultPosition, wxDefaultSize, 0 );
    gbSizerInnerExtProperties1->Add(m_staticTextPlSpeedUnits, 0, wxALIGN_CENTRE_VERTICAL, 0);
    
    m_staticTextEta = new wxStaticText( sbSizerExtProperties->GetStaticBox(), wxID_ANY, _("ETD") );
    gbSizerInnerExtProperties1->Add(m_staticTextEta, 0, wxALIGN_CENTRE_VERTICAL, 0);
    wxBoxSizer* bsTimestamp = new wxBoxSizer(wxHORIZONTAL);
    m_cbEtaPresent = new wxCheckBox(sbSizerExtProperties->GetStaticBox(), wxID_ANY, wxEmptyString);
    bsTimestamp->Add(m_cbEtaPresent, 0, wxALL|wxEXPAND, 5);
    m_EtaDatePickerCtrl = new wxDatePickerCtrl(sbSizerExtProperties->GetStaticBox(), ID_ETA_DATEPICKERCTRL, wxDefaultDateTime, wxDefaultPosition, wxDefaultSize, wxDP_DEFAULT, wxDefaultValidator);
    bsTimestamp->Add(m_EtaDatePickerCtrl, 0, wxALL|wxEXPAND, 5);
    m_EtaTimePickerCtrl = new wxTimePickerCtrl(sbSizerExtProperties->GetStaticBox(), ID_ETA_TIMEPICKERCTRL, wxDefaultDateTime, wxDefaultPosition, wxDefaultSize, wxDP_DEFAULT, wxDefaultValidator);
    bsTimestamp->Add(m_EtaTimePickerCtrl, 0, wxALL|wxEXPAND, 5);
    gbSizerInnerExtProperties1->Add(bsTimestamp, 0, wxEXPAND, 0);

    sbSizerExtProperties->Add(gbSizerInnerExtProperties, 0, wxALL|wxEXPAND, 5);
    sbSizerExtProperties->Add(sbRangeRingsExtProperties, 0, wxALL|wxEXPAND, 5);
    sbSizerExtProperties->Add(gbSizerInnerExtProperties2, 0, wxALL|wxEXPAND, 5);
    sbSizerExtProperties->Add(gbSizerInnerExtProperties1, 0, wxALL|wxEXPAND, 5);
    
    fSizerExtProperties->Add(sbSizerExtProperties, 1, wxALL|wxEXPAND);

    sbSizerExtProperties->Fit(sbSizerExtProperties->GetStaticBox() );
    sbSizerExtProperties->SetSizeHints(sbSizerExtProperties->GetStaticBox());
    
    RecalculateSize();
    
    m_PanelBasicProperties->SetSizer(sbSizerBasicProperties);
    m_PanelDescription->SetSizer(sbSizerDescription);
    m_PanelExtendedProperties->SetSizer(fSizerExtProperties);
    
    sbSizerDescription->Fit(m_PanelDescription);
    
    m_notebookProperties->AddPage(m_PanelBasicProperties, _("Basic"), false);
    m_notebookProperties->AddPage(m_PanelDescription, _("Description"), false);
    m_notebookProperties->AddPage(m_PanelExtendedProperties, _("Extended"), false);
    bMainSizer->Add(m_notebookProperties, 1, wxLEFT|wxTOP|wxEXPAND, 5);
    
    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    //Get and convert picture.
    initialize_images();
    DefaultsBtn = new wxBitmapButton( this, ID_DEFAULT, *_img_MUI_settings_svg, wxDefaultPosition, _img_MUI_settings_svg->GetSize(), 0);
    btnSizer->Add(DefaultsBtn, 0, wxALL|wxALIGN_LEFT|wxALIGN_BOTTOM, 5);
    btnSizer->Add(0, 0, 1, wxEXPAND);  //spacer
    
    m_sdbSizerButtons = new wxStdDialogButtonSizer();
    m_sdbSizerButtons->AddButton(new wxButton(this, wxID_OK));
    m_sdbSizerButtons->AddButton(new wxButton(this, wxID_CANCEL));
    m_sdbSizerButtons->Realize();
    btnSizer->Add(m_sdbSizerButtons, 0, wxALL, 5);
    bMainSizer->Add( btnSizer, 0, wxEXPAND, 0);
    SetSizer(bMainSizer);
    Layout();
     
     // Connect Events
    m_textLatitude->Connect( wxEVT_CONTEXT_MENU,
            wxCommandEventHandler( MarkInfoDlg::OnRightClickLatLon ), NULL, this );
    m_textLongitude->Connect( wxEVT_CONTEXT_MENU,
            wxCommandEventHandler( MarkInfoDlg::OnRightClickLatLon ), NULL, this );
    // catch the right up event...
    m_htmlList->Connect(wxEVT_RIGHT_DOWN, wxMouseEventHandler(MarkInfoDlg::m_htmlListContextMenu), NULL, this);
    m_notebookProperties->Connect( wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED, wxNotebookEventHandler( MarkInfoDlg::OnNotebookPageChanged ), NULL, this );
    m_EtaTimePickerCtrl->Connect( wxEVT_TIME_CHANGED, wxDateEventHandler( MarkInfoDlg::OnTimeChanged ), NULL, this );
    m_EtaDatePickerCtrl->Connect( wxEVT_DATE_CHANGED, wxDateEventHandler( MarkInfoDlg::OnTimeChanged ), NULL, this );
    m_comboBoxTideStation->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( MarkInfoDlg::OnTideStationCombobox ), NULL, this );
}

#define TIDESTATION_BATCH_SIZE 10

void MarkInfoDlg::OnTideStationCombobox( wxCommandEvent& event)
{
    int count = m_comboBoxTideStation->GetCount();
    int sel = m_comboBoxTideStation->GetSelection();
    if( sel == count - 1 ) {
        wxString n;
        int i = 0;
        for( auto ts : m_tss) {
            if( i == count + TIDESTATION_BATCH_SIZE ) {
                break;
            }
            if( i > count ) {
                n = wxString::Format("%s", ts.second->IDX_station_name);
                m_comboBoxTideStation->Append(n);
            }
            i++;
        }
    }
}

void MarkInfoDlg::OnNotebookPageChanged( wxNotebookEvent& event )
{
    if( event.GetSelection() == EXTENDED_PROP_PAGE ) {
        if( m_lasttspos.IsSameAs(m_textLatitude->GetValue() + m_textLongitude->GetValue()) ) {
            return;
        }
        m_lasttspos = m_textLatitude->GetValue() + m_textLongitude->GetValue();
        double lat = fromDMM(m_textLatitude->GetValue());
        double lon = fromDMM(m_textLongitude->GetValue());
        m_tss = ptcmgr->GetStationsForLL(lat, lon);
        wxString s = m_comboBoxTideStation->GetStringSelection();
        wxString n;
        int i = 0;
        m_comboBoxTideStation->Clear();
        m_comboBoxTideStation->Append(wxEmptyString);
        for( auto ts : m_tss) {
            if( i == TIDESTATION_BATCH_SIZE ) {
                break;
            }
            i++;
            n = wxString::Format("%s", ts.second->IDX_station_name);
            m_comboBoxTideStation->Append(n);
            if( s == n ) {
                m_comboBoxTideStation->SetSelection(i);
            }
        }
        if( m_comboBoxTideStation->GetStringSelection() != s ) {
            m_comboBoxTideStation->Insert(s, 1);
            m_comboBoxTideStation->SetSelection(1);
        }
    }
}

void MarkInfoDlg::RecalculateSize( void )
{
    wxSize b = sbSizerBasicProperties->GetMinSize();
    wxSize e = sbSizerExtProperties->GetMinSize();
    wxSize w =  wxSize( std::max(b.x, e.x), std::max(b.y, e.y));
    m_PanelBasicProperties->SetSize(w);
    sbSizerBasicProperties->FitInside(m_PanelBasicProperties);
    m_PanelDescription->SetSize(w);
    sbSizerDescription->FitInside(m_PanelDescription);
    m_PanelExtendedProperties->SetSize(w);
    
    m_defaultClientSize = GetClientSize();
}



MarkInfoDlg::~MarkInfoDlg()
{
    // Disconnect Events
    m_textLatitude->Disconnect( wxEVT_CONTEXT_MENU,
                            wxCommandEventHandler( MarkInfoDlg::OnRightClickLatLon ), NULL, this );
    m_textLongitude->Disconnect( wxEVT_CONTEXT_MENU,
                             wxCommandEventHandler( MarkInfoDlg::OnRightClickLatLon ), NULL, this );
    m_htmlList->Disconnect(wxEVT_RIGHT_DOWN, wxMouseEventHandler(MarkInfoDlg::m_htmlListContextMenu), NULL, this);
    m_notebookProperties->Disconnect( wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED, wxNotebookEventHandler( MarkInfoDlg::OnNotebookPageChanged ), NULL, this );
    m_EtaTimePickerCtrl->Disconnect( wxEVT_TIME_CHANGED, wxDateEventHandler( MarkInfoDlg::OnTimeChanged ), NULL, this );
    m_EtaDatePickerCtrl->Disconnect( wxEVT_DATE_CHANGED, wxDateEventHandler( MarkInfoDlg::OnTimeChanged ), NULL, this );

#ifdef __OCPN__ANDROID__
    androidEnableBackButton( true );
#endif
}

void MarkInfoDlg::InitialFocus(void)
{
    m_textName->SetFocus();
    m_textName->SetInsertionPointEnd();
}

void MarkInfoDlg::SetColorScheme( ColorScheme cs )
{
    DimeControl( this );
}

void MarkInfoDlg::SetRoutePoint( RoutePoint *pRP )
{
    m_pRoutePoint = pRP;
    if( m_pRoutePoint ) {
        m_lat_save = m_pRoutePoint->m_lat;
        m_lon_save = m_pRoutePoint->m_lon;
        m_IconName_save = m_pRoutePoint->GetIconName();
        m_bShowName_save = m_pRoutePoint->m_bShowName;
        m_bIsVisible_save = m_pRoutePoint->m_bIsVisible;
        m_Name_save = m_pRoutePoint->GetName();
        m_Description_save = m_pRoutePoint->m_MarkDescription;
        m_bUseScaMin_save = m_pRoutePoint->GetUseSca();
        m_iScaminVal_save = m_pRoutePoint->GetScaMin();
        
        if( m_pMyLinkList )
            delete m_pMyLinkList;
        m_pMyLinkList = new HyperlinkList();
        int NbrOfLinks = m_pRoutePoint->m_HyperlinkList->GetCount();
        if( NbrOfLinks > 0 ) {
            wxHyperlinkListNode *linknode = m_pRoutePoint->m_HyperlinkList->GetFirst();
            while( linknode ) {
                Hyperlink *link = linknode->GetData();

                Hyperlink* h = new Hyperlink();
                h->DescrText = link->DescrText;
                h->Link = link->Link;
                h->LType = link->LType;

                m_pMyLinkList->Append( h );

                linknode = linknode->GetNext();
            }
        }
    }
}


void MarkInfoDlg::UpdateHtmlList()
{
    GetSimpleBox()->Clear();
    int NbrOfLinks = m_pRoutePoint->m_HyperlinkList->GetCount();
    
    if( NbrOfLinks > 0 ) {
        wxHyperlinkListNode *linknode = m_pRoutePoint->m_HyperlinkList->GetFirst();
        while( linknode ) {
            Hyperlink *link = linknode->GetData();
            wxString s = wxString::Format(wxT("<a href='%s'>%s</a>"),link->Link, link->DescrText );
            GetSimpleBox()->AppendString(s);
            linknode = linknode->GetNext();
        }       
    }
}

void MarkInfoDlg::OnHtmlLinkClicked(wxHtmlLinkEvent &event)
{
//        Windows has trouble handling local file URLs with embedded anchor points, e.g file://testfile.html#point1
//        The trouble is with the wxLaunchDefaultBrowser with verb "open"
//        Workaround is to probe the registry to get the default browser, and open directly
//     
//        But, we will do this only if the URL contains the anchor point charater '#'
//        What a hack......

#ifdef __WXMSW__
    wxString cc = event.GetLinkInfo().GetHref().c_str();
    if( cc.Find( _T("#") ) != wxNOT_FOUND ) {
        wxRegKey RegKey( wxString( _T("HKEY_CLASSES_ROOT\\HTTP\\shell\\open\\command") ) );
        if( RegKey.Exists() ) {
            wxString command_line;
            RegKey.QueryValue( wxString( _T("") ), command_line );

            //  Remove "
            command_line.Replace( wxString( _T("\"") ), wxString( _T("") ) );

            //  Strip arguments
            int l = command_line.Find( _T(".exe") );
            if( wxNOT_FOUND == l ) l = command_line.Find( _T(".EXE") );

            if( wxNOT_FOUND != l ) {
                wxString cl = command_line.Mid( 0, l + 4 );
                cl += _T(" ");
                cc.Prepend( _T("\"") );
                cc.Append( _T("\"") );
                cl += cc;
                wxExecute( cl );        // Async, so Fire and Forget...
            }
        }
	}
	else {
		wxString url = event.GetLinkInfo().GetHref().c_str();
		url.Replace(_T(" "), _T("%20"));
		::wxLaunchDefaultBrowser(url);
		event.Skip();
	}
#else
    wxString url = event.GetLinkInfo().GetHref().c_str();
    url.Replace(_T(" "), _T("%20") );
    ::wxLaunchDefaultBrowser(url);
    event.Skip();
#endif
}

void MarkInfoDlg::OnDescChangedExt( wxCommandEvent& event )
{
    if( m_PanelDescription->IsShownOnScreen() ){
        m_textDescription->ChangeValue( m_textCtrlExtDescription->GetValue() );
    }
    event.Skip();
}
void MarkInfoDlg::OnDescChangedBasic( wxCommandEvent& event )
{
    if( m_PanelBasicProperties->IsShownOnScreen() ){
        m_textCtrlExtDescription->ChangeValue( m_textDescription->GetValue() );
    }
    event.Skip();
}

void MarkInfoDlg::OnExtDescriptionClick( wxCommandEvent& event )
{
    long pos = m_textDescription->GetInsertionPoint();
    m_notebookProperties->SetSelection( 1 );
    m_textCtrlExtDescription->SetInsertionPoint( pos);
    event.Skip();
}

void MarkInfoDlg::OnShowWaypointNameSelectBasic( wxCommandEvent& event )
{
    if( m_PanelBasicProperties->IsShownOnScreen() )
        m_checkBoxShowNameExt->SetValue( m_checkBoxShowName->GetValue() );
    event.Skip();
}
void MarkInfoDlg::OnShowWaypointNameSelectExt( wxCommandEvent& event )
{
    if( m_PanelExtendedProperties->IsShownOnScreen() )
        m_checkBoxShowName->SetValue( m_checkBoxShowNameExt->GetValue() );
    event.Skip();
}

void MarkInfoDlg::OnWptRangeRingsNoChange( wxSpinEvent& event )
{
    if( !m_pRoutePoint->m_bIsInLayer ){
        m_textWaypointRangeRingsStep->Enable( (bool)(m_SpinWaypointRangeRingsNumber->GetValue() != 0) );
        m_PickColor->Enable( (bool)(m_SpinWaypointRangeRingsNumber->GetValue() != 0) );        
    }        
}

void MarkInfoDlg::OnSelectScaMinExt( wxCommandEvent& event )
{
    if( !m_pRoutePoint->m_bIsInLayer ){
        m_textScaMin->Enable(m_checkBoxScaMin->GetValue());
    }
}

void MarkInfoDlg::OnPositionCtlUpdated( wxCommandEvent& event )
{
    // Fetch the control values, convert to degrees
    double lat = fromDMM( m_textLatitude->GetValue() );
    double lon = fromDMM( m_textLongitude->GetValue() );
        if( !m_pRoutePoint->m_bIsInLayer ) {
            m_pRoutePoint->SetPosition( lat, lon );
            pSelect->ModifySelectablePoint( lat, lon, (void *) m_pRoutePoint, SELTYPE_ROUTEPOINT );
        }
    // Update the mark position dynamically
    gFrame->RefreshAllCanvas();
}

void MarkInfoDlg::m_htmlListContextMenu( wxMouseEvent &event )
{
    //SimpleHtmlList->HitTest doesn't seem to work under msWin, so we use a custom made version
    wxPoint pos = event.GetPosition();
    i_htmlList_item = -1; 
    for( int i=0; i <  (int)GetSimpleBox()->GetCount(); i++ )
    {
        wxRect rect = GetSimpleBox()->GetItemRect( i );
        if( rect.Contains( pos) ){
            i_htmlList_item = i;
            break;
        }            
    }

    wxMenu* popup = new wxMenu();
    if( ( GetSimpleBox()->GetCount() ) > 0 && 
            ( i_htmlList_item > -1 ) &&
            ( i_htmlList_item < (int)GetSimpleBox()->GetCount() ) ) {
        popup->Append( ID_RCLK_MENU_DELETE_LINK, _("Delete") );
        popup->Append( ID_RCLK_MENU_EDIT_LINK, _("Edit") );
    }
    popup->Append( ID_RCLK_MENU_ADD_LINK, _("Add New") );
    
    m_contextObject = event.GetEventObject();
    popup->Connect(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MarkInfoDlg::On_html_link_popupmenu_Click), NULL, this);
    PopupMenu( popup );
    delete popup;        
}

void MarkInfoDlg::On_html_link_popupmenu_Click( wxCommandEvent& event )
{
    switch(event.GetId()) {
 		case ID_RCLK_MENU_DELETE_LINK:
        {
            wxHyperlinkListNode* node = m_pRoutePoint->m_HyperlinkList->Item(i_htmlList_item);
            m_pRoutePoint->m_HyperlinkList->DeleteNode(node);
            UpdateHtmlList();
 			break;
        }
 		case ID_RCLK_MENU_EDIT_LINK:
        {
            Hyperlink *link = m_pRoutePoint->m_HyperlinkList->Item(i_htmlList_item)->GetData();
            LinkPropImpl* LinkPropDlg = new LinkPropImpl( this );
            LinkPropDlg->m_textCtrlLinkDescription->SetValue( link->DescrText );
            LinkPropDlg->m_textCtrlLinkUrl->SetValue( link->Link );
            DimeControl( LinkPropDlg );
            LinkPropDlg->ShowWindowModalThenDo([this,LinkPropDlg,link](int retcode){
                if ( retcode == wxID_OK ) {
                    link->DescrText = LinkPropDlg->m_textCtrlLinkDescription->GetValue();
                    link->Link = LinkPropDlg->m_textCtrlLinkUrl->GetValue();
                    m_pRoutePoint->m_HyperlinkList->Item(i_htmlList_item)->SetData(link);
                    UpdateHtmlList();
                }
            });
 			break;
        }
        case ID_RCLK_MENU_ADD_LINK:
        {
            LinkPropImpl* LinkPropDlg = new LinkPropImpl( this );
            LinkPropDlg->m_textCtrlLinkDescription->SetValue( wxEmptyString );
            LinkPropDlg->m_textCtrlLinkUrl->SetValue( wxEmptyString );
            DimeControl( LinkPropDlg );
            LinkPropDlg->ShowWindowModalThenDo([this,LinkPropDlg](int retcode){
                if ( retcode == wxID_OK ) {
                    Hyperlink *link =new Hyperlink;
                    link->DescrText = LinkPropDlg->m_textCtrlLinkDescription->GetValue();
                    link->Link = LinkPropDlg->m_textCtrlLinkUrl->GetValue();
                    //Check if decent
                    if ( link->DescrText == wxEmptyString ) {
                        link->DescrText = link->Link;
                    }
                    if ( link->Link == wxEmptyString ) {
                        delete link;
                    } else {
                        m_pRoutePoint->m_HyperlinkList->Append(link);
                    }
                    UpdateHtmlList();
                }
            });
 			break;
        }
    }
    event.Skip();
}

void MarkInfoDlg::OnRightClickLatLon( wxCommandEvent& event )
{
    wxMenu* popup = new wxMenu();
    popup->Append( ID_RCLK_MENU_COPY, _("Copy") );
    popup->Append( ID_RCLK_MENU_COPY_LL, _("Copy lat/long") );
    popup->Append( ID_RCLK_MENU_PASTE, _("Paste") );
    popup->Append( ID_RCLK_MENU_PASTE_LL, _("Paste lat/long") );
    m_contextObject = event.GetEventObject();
    PopupMenu( popup );
    delete popup;
}

void MarkInfoDlg::OnCopyPasteLatLon( wxCommandEvent& event )
{
    // Fetch the control values, convert to degrees
    double lat = fromDMM( m_textLatitude->GetValue() );
    double lon = fromDMM( m_textLongitude->GetValue() );

    wxString result;

    switch( event.GetId() ) {
        case ID_RCLK_MENU_PASTE: {
            if( wxTheClipboard->Open() ) {
                wxTextDataObject data;
                wxTheClipboard->GetData( data );
                result = data.GetText();
                ((wxTextCtrl*)m_contextObject)->SetValue( result );
                wxTheClipboard->Close();
            }
            return;
        }
        case ID_RCLK_MENU_PASTE_LL: {
            if( wxTheClipboard->Open() ) {
                wxTextDataObject data;
                wxTheClipboard->GetData( data );
                result = data.GetText();

                PositionParser pparse( result );

                if( pparse.IsOk() ) {
                    m_textLatitude->SetValue( pparse.GetLatitudeString() );
                    m_textLongitude->SetValue( pparse.GetLongitudeString() );
                }
                wxTheClipboard->Close();
            }
            return;
        }
        case ID_RCLK_MENU_COPY: {
            result = ((wxTextCtrl*)m_contextObject)->GetValue();
            break;
        }
        case ID_RCLK_MENU_COPY_LL: {
            result << toSDMM( 1, lat, true ) <<_T('\t');
            result << toSDMM( 2, lon, true );
            break;
        }
    }

    if( wxTheClipboard->Open() ) {
        wxTextDataObject* data = new wxTextDataObject;
        data->SetText( result );
        wxTheClipboard->SetData( data );
        wxTheClipboard->Close();
    }
}

void MarkInfoDlg::DefautlBtnClicked( wxCommandEvent& event )
{
    SaveDefaultsDialog* SaveDefaultDlg = new SaveDefaultsDialog( this );
    DimeControl( SaveDefaultDlg );
    SaveDefaultDlg->ShowWindowModalThenDo([this,SaveDefaultDlg](int retcode){
        if ( retcode == wxID_OK ) {
            double value;
            if ( SaveDefaultDlg->IconCB->GetValue() ) {
                g_default_wp_icon = *pWayPointMan->GetIconKey( m_bcomboBoxIcon->GetSelection() );
            }
            if ( SaveDefaultDlg->RangRingsCB->GetValue() ) {
                g_iWaypointRangeRingsNumber = m_SpinWaypointRangeRingsNumber->GetValue();
                if(m_textWaypointRangeRingsStep->GetValue().ToDouble(&value))
                    g_fWaypointRangeRingsStep = fromUsrDistance(value, -1) ;
                g_colourWaypointRangeRingsColour = m_PickColor->GetColour();
            }
            if ( SaveDefaultDlg->ArrivalRCB->GetValue() )
                if(m_textArrivalRadius->GetValue().ToDouble(&value))
                    g_n_arrival_circle_radius = fromUsrDistance(value, -1);
            if ( SaveDefaultDlg->ScaleCB->GetValue() ) {
                g_iWpt_ScaMin = wxAtoi( m_textScaMin->GetValue() );
                g_bUseWptScaMin = m_checkBoxScaMin->GetValue() ;
            }
            if ( SaveDefaultDlg->NameCB->GetValue() ) {
                g_iWpt_ScaMin = m_checkBoxShowName->GetValue();
            }
        }
    });
}

void MarkInfoDlg::OnMarkInfoCancelClick( wxCommandEvent& event )
{
    if( m_pRoutePoint ) {
        m_pRoutePoint->SetVisible( m_bIsVisible_save );
        m_pRoutePoint->SetNameShown( m_bShowName_save );
        m_pRoutePoint->SetPosition( m_lat_save, m_lon_save );
        m_pRoutePoint->SetIconName( m_IconName_save );
        m_pRoutePoint->ReLoadIcon();
        m_pRoutePoint->SetName(m_Name_save);
        m_pRoutePoint->m_MarkDescription = m_Description_save;
        m_pRoutePoint->SetUseSca(m_bUseScaMin_save);
        m_pRoutePoint->SetScaMin(m_iScaminVal_save);

        m_pRoutePoint->m_HyperlinkList->Clear();

        int NbrOfLinks = m_pMyLinkList->GetCount();
        if( NbrOfLinks > 0 ) {
            wxHyperlinkListNode *linknode = m_pMyLinkList->GetFirst();
            while( linknode ) {
                Hyperlink *link = linknode->GetData();
                Hyperlink* h = new Hyperlink();
                h->DescrText = link->DescrText;
                h->Link = link->Link;
                h->LType = link->LType;

                m_pRoutePoint->m_HyperlinkList->Append( h );

                linknode = linknode->GetNext();
            }
        }
    }

    #ifdef __WXGTK__ 
        gFrame->Raise();
    #endif
    
    Show( false );
    delete m_pMyLinkList;
    m_pMyLinkList = NULL;
    SetClientSize(m_defaultClientSize);

    #ifdef __OCPN__ANDROID__
    androidEnableBackButton( true );
    #endif
    
    event.Skip();
}

void MarkInfoDlg::OnMarkInfoOKClick( wxCommandEvent& event )
{
    if( m_pRoutePoint ) {
         m_pRoutePoint->m_wxcWaypointRangeRingsColour = m_PickColor->GetColour();

        OnPositionCtlUpdated( event );
        SaveChanges(); // write changes to globals and update config
    }

    #ifdef __WXGTK__ 
        gFrame->Raise();
    #endif
    
    Show( false );

    if( pRouteManagerDialog && pRouteManagerDialog->IsShown() )
        pRouteManagerDialog->UpdateWptListCtrl();
        
    if( pRoutePropDialog && pRoutePropDialog->IsShown() )
        pRoutePropDialog->UpdatePoints();

    SetClientSize(m_defaultClientSize);
    
    #ifdef __OCPN__ANDROID__
    androidEnableBackButton( true );
    #endif
    
    event.Skip();
}

bool MarkInfoDlg::UpdateProperties( bool positionOnly )
{
    if (m_pRoutePoint) {

        m_textLatitude->SetValue( ::toSDMM( 1, m_pRoutePoint->m_lat ) );
        m_textLongitude->SetValue( ::toSDMM( 2, m_pRoutePoint->m_lon ) );
        m_lat_save = m_pRoutePoint->m_lat;
        m_lon_save = m_pRoutePoint->m_lon;
        m_textName->SetValue( m_pRoutePoint->GetName() );
        m_textDescription->ChangeValue( m_pRoutePoint->m_MarkDescription );
        m_textCtrlExtDescription->ChangeValue( m_pRoutePoint->m_MarkDescription );
        m_checkBoxShowName->SetValue( m_pRoutePoint->m_bShowName );
        m_checkBoxShowNameExt->SetValue( m_pRoutePoint->m_bShowName );
        m_checkBoxVisible->SetValue( m_pRoutePoint->m_bIsVisible );
        m_checkBoxScaMin->SetValue( m_pRoutePoint->GetUseSca() );
        m_textScaMin->SetValue( wxString::Format(wxT("%i"), (int)m_pRoutePoint->GetScaMin()) );
        m_textCtrlGuid->SetValue( m_pRoutePoint->m_GUID );
        m_SpinWaypointRangeRingsNumber->SetValue( m_pRoutePoint->GetWaypointRangeRingsNumber() );
        m_staticTextRR3->SetLabel( getUsrDistanceUnit() );
        wxString buf;
        buf.Printf( _T("%.3f"), toUsrDistance( m_pRoutePoint->GetWaypointRangeRingsStep(), -1) );
        m_textWaypointRangeRingsStep->SetValue( buf );
        m_staticTextArrivalUnits->SetLabel( getUsrDistanceUnit() );
        buf.Printf( _T("%.3f"), toUsrDistance(m_pRoutePoint->GetWaypointArrivalRadius(), -1) );
        m_textArrivalRadius->SetValue(buf);

        wxColour col = m_pRoutePoint->m_wxcWaypointRangeRingsColour;
        m_PickColor->SetColour(col);

        if( m_comboBoxTideStation->GetStringSelection() != m_pRoutePoint->m_TideStation) {
            m_comboBoxTideStation->Clear();
            m_comboBoxTideStation->Append(wxEmptyString);
            if (!m_pRoutePoint->m_TideStation.IsEmpty()) {
                m_comboBoxTideStation->Append(m_pRoutePoint->m_TideStation);
                m_comboBoxTideStation->SetSelection(1);
            }
        }
        
        if( m_pRoutePoint->GetPlannedSpeed() > .01f ) {
            m_textCtrlPlSpeed->SetValue(wxString::Format("%.1f", toUsrSpeed(m_pRoutePoint->GetPlannedSpeed())));
        } else {
            m_textCtrlPlSpeed->SetValue(wxEmptyString);
        }
        
        wxDateTime etd;
        etd = m_pRoutePoint->GetManualETD();
        if( etd.IsValid() ) {
            m_cbEtaPresent->SetValue(true);
            m_EtaDatePickerCtrl->SetValue(etd.GetDateOnly());
            m_EtaTimePickerCtrl->SetValue(etd);
        } else {
            m_cbEtaPresent->SetValue(false);
        }

        m_staticTextPlSpeed->Show(m_pRoutePoint->m_bIsInRoute);
        m_textCtrlPlSpeed->Show(m_pRoutePoint->m_bIsInRoute);
        m_staticTextEta->Show(m_pRoutePoint->m_bIsInRoute);
        m_EtaDatePickerCtrl->Show(m_pRoutePoint->m_bIsInRoute);
        m_EtaTimePickerCtrl->Show(m_pRoutePoint->m_bIsInRoute);
        m_staticTextPlSpeedUnits->Show(m_pRoutePoint->m_bIsInRoute);
        m_cbEtaPresent->Show(m_pRoutePoint->m_bIsInRoute);
        m_staticTextArrivalRadius->Show(m_pRoutePoint->m_bIsInRoute);
        m_staticTextArrivalUnits->Show(m_pRoutePoint->m_bIsInRoute);
        m_textArrivalRadius->Show(m_pRoutePoint->m_bIsInRoute);
        
        if( positionOnly ) return true;

        //Layer or not?
        if( m_pRoutePoint->m_bIsInLayer ) {
            m_staticTextLayer->Enable();
            m_staticTextLayer->Show( true );
            m_textName->SetEditable( false );
            m_textDescription->SetEditable( false );
            m_textCtrlExtDescription->SetEditable( false );
            m_textLatitude->SetEditable( false );
            m_textLongitude->SetEditable( false );
            m_bcomboBoxIcon->Enable( false );
            m_checkBoxShowName->Enable( false );
            m_checkBoxVisible->Enable( false );
            m_textArrivalRadius->SetEditable( false );
            m_checkBoxScaMin->Enable( false );
            m_textScaMin->SetEditable ( false );
            m_checkBoxShowNameExt->Enable( false );
            m_SpinWaypointRangeRingsNumber->Enable( false );
            m_textWaypointRangeRingsStep->SetEditable( false );
            m_PickColor->Enable( false );
            DefaultsBtn->Enable( false );
            m_EtaDatePickerCtrl->Enable(false);
            m_EtaTimePickerCtrl->Enable(false);
            m_cbEtaPresent->Enable(false);
            if( !m_textDescription->IsEmpty() ){
                m_notebookProperties->SetSelection(1); //Show Description page
            }
        } else {
            m_staticTextLayer->Enable( false );
            m_staticTextLayer->Show( false );
            m_textName->SetEditable( true );
            m_textDescription->SetEditable( true );
            m_textCtrlExtDescription->SetEditable( true );
            m_textLatitude->SetEditable( true );
            m_textLongitude->SetEditable( true );
            m_bcomboBoxIcon->Enable( true );
            m_checkBoxShowName->Enable( true );
            m_checkBoxVisible->Enable( true );
            m_textArrivalRadius->SetEditable ( true );
            m_checkBoxScaMin->Enable( true );
            m_textScaMin->SetEditable ( true );
            m_checkBoxShowNameExt->Enable( true );
            m_SpinWaypointRangeRingsNumber->Enable( true );
            m_textWaypointRangeRingsStep->SetEditable( true );
            m_PickColor->Enable( true );
            DefaultsBtn->Enable( true );
            m_EtaDatePickerCtrl->Enable(true);
            m_EtaTimePickerCtrl->Enable(true);
            m_cbEtaPresent->Enable(true);
            m_notebookProperties->SetSelection(0);
        }
               
 
        // Fill the icon selector combo box
        m_bcomboBoxIcon->Clear();
        //      Iterate on the Icon Descriptions, filling in the combo control
        bool fillCombo = m_bcomboBoxIcon->GetCount() == 0;
        
        if( fillCombo ){
            for( int i = 0; i < pWayPointMan->GetNumIcons(); i++ ) {
                wxString *ps = pWayPointMan->GetIconDescription( i );
                wxBitmap bmp = pWayPointMan->GetIconBitmapForList(i, 2 * GetCharHeight());
                    
                m_bcomboBoxIcon->Append( *ps, bmp );
            }
        }                
        // find the correct item in the combo box
        int iconToSelect = -1;
        for( int i = 0; i < pWayPointMan->GetNumIcons(); i++ ) {
            if( *pWayPointMan->GetIconKey( i ) == m_pRoutePoint->GetIconName() ){
                iconToSelect = i;
                m_bcomboBoxIcon->Select( iconToSelect );
                break;
            }
        }
        wxCommandEvent ev;
        wxSpinEvent se;
        OnShowWaypointNameSelectBasic( ev );
        OnWptRangeRingsNoChange( se );
        OnSelectScaMinExt(ev);
        UpdateHtmlList();       
    }
    

    #ifdef __OCPN__ANDROID__
        androidEnableBackButton( false );
    #endif
    
    Fit();
    //SetMinSize(wxSize(-1, 600));
    RecalculateSize();
    
    return true;
}

void MarkInfoDlg::OnBitmapCombClick(wxCommandEvent& event)
{
       wxString *icon_name = pWayPointMan->GetIconKey( m_bcomboBoxIcon->GetSelection() );
        if(icon_name && icon_name->Length())
            m_pRoutePoint->SetIconName( *icon_name );
        m_pRoutePoint->ReLoadIcon();
        SaveChanges();
        //pConfig->UpdateWayPoint( m_pRoutePoint );
}

void MarkInfoDlg::ValidateMark( void )
{
    //    Look in the master list of Waypoints to see if the currently selected waypoint is still valid
    //    It may have been deleted as part of a route
    wxRoutePointListNode *node = pWayPointMan->GetWaypointList()->GetFirst();

    bool b_found = false;
    while( node ) {
        RoutePoint *rp = node->GetData();
        if( m_pRoutePoint == rp ) {
            b_found = true;
            break;
        }
        node = node->GetNext();
    }
    if( !b_found ) m_pRoutePoint = NULL;
}

bool MarkInfoDlg::SaveChanges()
{
    if( m_pRoutePoint ) {
        if( m_pRoutePoint->m_bIsInLayer ) return true;

        // Get User input Text Fields
        m_pRoutePoint->SetName( m_textName->GetValue() );
        m_pRoutePoint->SetWaypointArrivalRadius( m_textArrivalRadius->GetValue() );
        m_pRoutePoint->SetScaMin( m_textScaMin->GetValue() );
        m_pRoutePoint->SetUseSca( m_checkBoxScaMin->GetValue() );
        m_pRoutePoint->m_MarkDescription = m_textDescription->GetValue();
        m_pRoutePoint->SetVisible( m_checkBoxVisible->GetValue() );
        m_pRoutePoint->m_bShowName = m_checkBoxShowName->GetValue();
        m_pRoutePoint->SetPosition( fromDMM( m_textLatitude->GetValue() ),
                 fromDMM( m_textLongitude->GetValue() ) );
        wxString *icon_name = pWayPointMan->GetIconKey( m_bcomboBoxIcon->GetSelection() );
        if(icon_name && icon_name->Length())
            m_pRoutePoint->SetIconName( *icon_name );
        m_pRoutePoint->ReLoadIcon();
        m_pRoutePoint->SetShowWaypointRangeRings( (bool)(m_SpinWaypointRangeRingsNumber->GetValue() != 0) );
        m_pRoutePoint->SetWaypointRangeRingsNumber(m_SpinWaypointRangeRingsNumber->GetValue() );
        double value;
        if(m_textWaypointRangeRingsStep->GetValue().ToDouble(&value))
            m_pRoutePoint->SetWaypointRangeRingsStep(fromUsrDistance(value, -1) );
        if(m_textArrivalRadius->GetValue().ToDouble(&value))
            m_pRoutePoint->SetWaypointArrivalRadius(fromUsrDistance(value, -1) );
        
        m_pRoutePoint->m_TideStation = m_comboBoxTideStation->GetStringSelection();
        if( m_textCtrlPlSpeed->GetValue() == wxEmptyString ) {
            m_pRoutePoint->SetPlannedSpeed(0.0f);
        } else {
            double spd;
            if( m_textCtrlPlSpeed->GetValue().ToDouble(&spd) ) {
                m_pRoutePoint->SetPlannedSpeed(fromUsrSpeed(spd));
            }
        }
        
        if( m_cbEtaPresent->GetValue() ) {
            wxDateTime dt = m_EtaDatePickerCtrl->GetValue();
            dt.SetHour(m_EtaTimePickerCtrl->GetValue().GetHour());
            dt.SetMinute(m_EtaTimePickerCtrl->GetValue().GetMinute());
            dt.SetSecond(m_EtaTimePickerCtrl->GetValue().GetSecond());
            if( dt.IsValid() ) {
                m_pRoutePoint->SetETD(dt.FormatISOCombined());
            }
        } else {
            m_pRoutePoint->SetETD(wxEmptyString);
        }

        // Here is some logic....
        // If the Markname is completely numeric, and is part of a route,
        // Then declare it to be of attribute m_bDynamicName = true
        // This is later used for re-numbering points on actions like
        // Insert Point, Delete Point, Append Point, etc

        if( m_pRoutePoint->m_bIsInRoute ) {
            bool b_name_is_numeric = true;
            for( unsigned int i = 0; i < m_pRoutePoint->GetName().Len(); i++ ) {
                if( wxChar( '0' ) > m_pRoutePoint->GetName()[i] ) b_name_is_numeric = false;
                if( wxChar( '9' ) < m_pRoutePoint->GetName()[i] ) b_name_is_numeric = false;
            }

            m_pRoutePoint->m_bDynamicName = b_name_is_numeric;
        } else
            m_pRoutePoint->m_bDynamicName = false;

        if( m_pRoutePoint->m_bIsInRoute ) {
            // Update the route segment selectables
            pSelect->UpdateSelectableRouteSegments( m_pRoutePoint );

            // Get an array of all routes using this point
            wxArrayPtrVoid *pEditRouteArray = g_pRouteMan->GetRouteArrayContaining( m_pRoutePoint );

            if( pEditRouteArray ) {
                for( unsigned int ir = 0; ir < pEditRouteArray->GetCount(); ir++ ) {
                    Route *pr = (Route *) pEditRouteArray->Item( ir );
                    pr->FinalizeForRendering();
                    pr->UpdateSegmentDistances();

                    pConfig->UpdateRoute( pr );
                }
                delete pEditRouteArray;
            }
        } else
            pConfig->UpdateWayPoint( m_pRoutePoint );
        // No general settings need be saved pConfig->UpdateSettings();
     }
     //gFrame->GetFocusCanvas()->Refresh(false);
    return true;
}

SaveDefaultsDialog::SaveDefaultsDialog(MarkInfoDlg* parent) : wxDialog(parent, wxID_ANY, _("Save some defaults"))
{
    //(*Initialize(SaveDefaultsDialog)
    this->SetSizeHints( wxDefaultSize, wxDefaultSize );
    
    wxBoxSizer* bSizer1 = new wxBoxSizer(wxVERTICAL);
    wxStdDialogButtonSizer* StdDialogButtonSizer1;
    
    StaticText1 = new wxStaticText(this, wxID_ANY, _("Check which properties of current waypoint\n should be set as default for NEW waypoints."));
    bSizer1->Add(StaticText1, 0, wxALL|wxALIGN_CENTER_HORIZONTAL, 5);
    
    wxFlexGridSizer* fgSizer1 = new wxFlexGridSizer(2);

    wxString s = ( g_pMarkInfoDialog->m_checkBoxShowName->GetValue() ? _("Do use"): _("Don't use") );
    NameCB = new wxCheckBox(this, wxID_ANY, _("Show Waypoint Name"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);
    fgSizer1->Add(NameCB, 0, wxALL, 5);
    stName = new wxStaticText( this, wxID_ANY, _T("[") + s +_T("]"), wxDefaultPosition, wxDefaultSize, 0 );
    stName->Wrap( -1 );
    fgSizer1->Add( stName, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5 );
    
    s = g_pMarkInfoDialog->m_pRoutePoint->GetIconName();
    IconCB = new wxCheckBox(this, wxID_ANY, _("Icon"));
    fgSizer1->Add(IconCB, 0, wxALL, 5);
    stIcon = new wxStaticText( this, wxID_ANY, _T("[") + s +_T("]"), wxDefaultPosition, wxDefaultSize, 0 );
    stIcon->Wrap( -1 );
    fgSizer1->Add( stIcon, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5 );
    
    s = ( g_pMarkInfoDialog->m_SpinWaypointRangeRingsNumber->GetValue() ?
         _("Do use") + wxString::Format(_T(" (%i) "), g_pMarkInfoDialog->m_SpinWaypointRangeRingsNumber->GetValue() ): _("Don't use") );
    RangRingsCB = new wxCheckBox(this, wxID_ANY, _("Range rings"));
    fgSizer1->Add(RangRingsCB, 0, wxALL, 5);
    stRR = new wxStaticText( this, wxID_ANY, _T("[") + s +_T("]"), wxDefaultPosition, wxDefaultSize, 0 );
    stRR->Wrap( -1 );
    fgSizer1->Add( stRR, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5 );
    
    s = ( g_pMarkInfoDialog->m_textArrivalRadius->GetValue());
    ArrivalRCB = new wxCheckBox(this, wxID_ANY, _("Arrival radius"));
    fgSizer1->Add(ArrivalRCB, 0, wxALL, 5);
    stArrivalR = new wxStaticText( this, wxID_ANY, wxString::Format(_T("[%s %s]"), s.c_str(), getUsrDistanceUnit().c_str()), wxDefaultPosition, wxDefaultSize, 0 );
    stArrivalR->Wrap( -1 );
    fgSizer1->Add( stArrivalR, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5 );
    
    s = ( g_pMarkInfoDialog->m_checkBoxScaMin->GetValue() ?
         _("Show only if")+_T(" < ") + g_pMarkInfoDialog->m_textScaMin->GetValue():
         _("Show always") );
    ScaleCB = new wxCheckBox(this, wxID_ANY, _("Show only at scale"));
    fgSizer1->Add(ScaleCB, 0, wxALL, 5);
    stScale = new wxStaticText( this, wxID_ANY, _T("[") + s +_T("]"), wxDefaultPosition, wxDefaultSize, 0 );
    stScale->Wrap( -1 );
    fgSizer1->Add( stScale, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5 );
    
    bSizer1->Add(fgSizer1, 0, wxALL|wxEXPAND, 5);

    StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
    StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK));
    StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL));
    StdDialogButtonSizer1->Realize();
    bSizer1->Add(StdDialogButtonSizer1, 0, wxALL|wxEXPAND, 5);

    SetSizer(bSizer1);
    Layout();
}

void MarkInfoDlg::ShowTidesBtnClicked( wxCommandEvent& event )
{
    if( m_comboBoxTideStation->GetSelection() < 1 ) {
        return;
    }
    IDX_entry* pIDX = (IDX_entry*)ptcmgr->GetIDX_entry(ptcmgr->GetStationIDXbyName(m_comboBoxTideStation->GetStringSelection(), fromDMM(m_textLatitude->GetValue()), fromDMM(m_textLongitude->GetValue())));
    TCWin* pCwin = new TCWin( gFrame->GetPrimaryCanvas(), 0, 0, pIDX );
    pCwin->Show();
}
