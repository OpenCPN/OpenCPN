/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Options Dialog
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
 ***************************************************************************
 *
 *
 */

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"


#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/progdlg.h>
#include <wx/sound.h>
#include <wx/radiobox.h>
#include <wx/listbox.h>
#include <wx/imaglist.h>
#include <wx/display.h>
#include <wx/choice.h>
#include <wx/dirdlg.h>

#include "dychart.h"
#include "chart1.h"
#include "chartdbs.h"
#include "options.h"
#include "styles.h"
#include "datastream.h"
#include "multiplexer.h"

#include "navutil.h"

#ifdef USE_S57
#include "s52plib.h"
#include "s52utils.h"
#include "cm93.h"
#endif

wxString GetOCPNKnownLanguage(wxString lang_canonical, wxString *lang_dir);

extern MyFrame          *gFrame;
extern ChartCanvas      *cc1;

extern bool             g_bShowOutlines;
extern bool             g_bShowDepthUnits;
extern bool             g_bskew_comp;
extern bool             g_bopengl;
extern bool             g_bsmoothpanzoom;

extern FontMgr          *pFontMgr;
extern wxString         *pInit_Chart_Dir;
extern wxArrayOfConnPrm *g_pConnectionParams;
extern Multiplexer      *g_pMUX;
extern bool             g_bfilter_cogsog;
extern int              g_COGFilterSec;
extern int              g_SOGFilterSec;

extern PlugInManager    *g_pi_manager;
extern ocpnStyle::StyleManager*   g_StyleManager;

extern wxString         g_SData_Locn;

extern bool             g_bDisplayGrid;

//    AIS Global configuration
extern bool             g_bCPAMax;
extern double           g_CPAMax_NM;
extern bool             g_bCPAWarn;
extern double           g_CPAWarn_NM;
extern bool             g_bTCPA_Max;
extern double           g_TCPA_Max;
extern bool             g_bMarkLost;
extern double           g_MarkLost_Mins;
extern bool             g_bRemoveLost;
extern double           g_RemoveLost_Mins;
extern bool             g_bShowCOG;
extern double           g_ShowCOG_Mins;
extern bool             g_bAISShowTracks;
extern double           g_AISShowTracks_Mins;
extern bool             g_bShowMoored;
extern double           g_ShowMoored_Kts;
extern bool             g_bAIS_CPA_Alert;
extern bool             g_bAIS_CPA_Alert_Audio;
extern wxString         g_sAIS_Alert_Sound_File;
extern bool             g_bAIS_CPA_Alert_Suppress_Moored;
extern bool             g_bShowAreaNotices;
extern bool             g_bDrawAISSize;

extern int              g_iNavAidRadarRingsNumberVisible;
extern float            g_fNavAidRadarRingsStep;
extern int              g_pNavAidRadarRingsStepUnits;
extern bool             g_bWayPointPreventDragging;

extern bool             g_bPreserveScaleOnX;
extern bool             g_bPlayShipsBells;
extern bool             g_bFullscreenToolbar;
extern bool             g_bTransparentToolbar;

extern int              g_OwnShipIconType;
extern double           g_n_ownship_length_meters;
extern double           g_n_ownship_beam_meters;
extern double           g_n_gps_antenna_offset_y;
extern double           g_n_gps_antenna_offset_x;
extern long             g_n_ownship_min_mm;

extern bool             g_bEnableZoomToCursor;
extern bool             g_bTrackDaily;
extern bool             g_bHighliteTracks;
extern double           g_TrackIntervalSeconds;
extern double           g_TrackDeltaDistance;
extern double           g_TrackDeltaDistance;
extern int              g_nTrackPrecision;

extern int              g_iSDMMFormat;

extern int              g_cm93_zoom_factor;

extern TTYWindow        *g_NMEALogWindow;
extern int              g_NMEALogWindow_x, g_NMEALogWindow_y;
extern int              g_NMEALogWindow_sx, g_NMEALogWindow_sy;

extern int              g_COGAvgSec;

extern bool             g_bCourseUp;
extern bool             g_bLookAhead;

extern double           g_ownship_predictor_minutes;

extern bool             g_bAISRolloverShowClass;
extern bool             g_bAISRolloverShowCOG;
extern bool             g_bAISRolloverShowCPA;

extern bool             g_bAIS_ACK_Timeout;
extern double           g_AckTimeout_Mins;

extern bool             g_bQuiltEnable;
extern bool             g_bFullScreenQuilt;
extern bool             g_bConfirmObjectDelete;

extern wxLocale         *plocale_def_lang;

#ifdef USE_S57
extern s52plib          *ps52plib;
#endif

extern wxString         g_locale;
extern bool             g_bportable;
extern wxString         *pHome_Locn;

extern ChartGroupArray  *g_pGroupArray;
extern ocpnStyle::StyleManager* g_StyleManager;

//    Some constants
#define ID_CHOICE_NMEA  wxID_HIGHEST + 1

extern wxArrayString *EnumerateSerialPorts(void);           // in chart1.cpp

extern wxArrayString    TideCurrentDataSet;
extern wxString         g_TCData_Dir;

options                *g_pOptions;

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(ArrayOfDirCtrls);

IMPLEMENT_DYNAMIC_CLASS( options, wxDialog )

BEGIN_EVENT_TABLE( options, wxDialog )
    EVT_CHECKBOX( ID_DEBUGCHECKBOX1, options::OnDebugcheckbox1Click )
    EVT_BUTTON( ID_BUTTONADD, options::OnButtonaddClick )
    EVT_BUTTON( ID_BUTTONDELETE, options::OnButtondeleteClick )
    EVT_BUTTON( ID_TCDATAADD, options::OnInsertTideDataLocation )
    EVT_BUTTON( ID_TCDATADEL, options::OnRemoveTideDataLocation )
    EVT_BUTTON( ID_APPLY, options::OnApplyClick )
    EVT_BUTTON( xID_OK, options::OnXidOkClick )
    EVT_BUTTON( wxID_CANCEL, options::OnCancelClick )
    EVT_BUTTON( ID_BUTTONFONTCHOOSE, options::OnChooseFont )
    EVT_RADIOBOX(ID_RADARDISTUNIT, options::OnDisplayCategoryRadioButton )
    EVT_BUTTON( ID_CLEARLIST, options::OnButtonClearClick )
    EVT_BUTTON( ID_SELECTLIST, options::OnButtonSelectClick )
    EVT_BUTTON( ID_AISALERTSELECTSOUND, options::OnButtonSelectSound )
    EVT_BUTTON( ID_AISALERTTESTSOUND, options::OnButtonTestSound )
    EVT_CHECKBOX( ID_SHOWGPSWINDOW, options::OnShowGpsWindowCheckboxClick )
    EVT_CHECKBOX( ID_ZTCCHECKBOX, options::OnZTCCheckboxClick )
    EVT_CHOICE( ID_SHIPICONTYPE, options::OnShipTypeSelect )
    EVT_CHOICE( ID_RADARRINGS, options::OnRadarringSelect )
    EVT_CHAR_HOOK( options::OnCharHook )
END_EVENT_TABLE()

options::options()
{
    Init();
}

options::options( MyFrame* parent, wxWindowID id, const wxString& caption, const wxPoint& pos,
        const wxSize& size, long style )
{
    Init();

    pParent = parent;

    long wstyle = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER;
    SetExtraStyle( wxWS_EX_BLOCK_EVENTS );

    wxDialog::Create( parent, id, caption, pos, size, wstyle );

    CreateControls();
    Fit();
    Center();
}

options::~options()
{
    // Disconnect Events
    m_lcSources->Disconnect( wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler( options::OnSelectDatasource ), NULL, this );
    m_buttonAdd->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( options::OnAddDatasourceClick ), NULL, this );
    m_buttonRemove->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( options::OnRemoveDatasourceClick ), NULL, this );
    m_tFilterSec->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( options::OnValChange ), NULL, this );
    m_rbTypeSerial->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( options::OnTypeSerialSelected ), NULL, this );
    m_rbTypeNet->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( options::OnTypeNetSelected ), NULL, this );
    m_rbNetProtoTCP->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( options::OnNetProtocolSelected ), NULL, this );
    m_rbNetProtoUDP->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( options::OnNetProtocolSelected ), NULL, this );
    m_rbNetProtoGPSD->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( options::OnNetProtocolSelected ), NULL, this );
    m_tNetAddress->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( options::OnValChange ), NULL, this );
    m_tNetPort->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( options::OnValChange ), NULL, this );
    m_comboPort->Disconnect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( options::OnValChange ), NULL, this );
    m_comboPort->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( options::OnValChange ), NULL, this );
    m_choiceBaudRate->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( options::OnBaudrateChoice ), NULL, this );
    m_choiceSerialProtocol->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( options::OnProtocolChoice ), NULL, this );
    m_choicePriority->Disconnect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( options::OnValChange ), NULL, this );
    m_cbCheckCRC->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( options::OnCrcCheck ), NULL, this );
    m_cbGarminHost->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( options::OnUploadFormatChange ), NULL, this );
    m_cbFurunoGP3X->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( options::OnUploadFormatChange ), NULL, this );
    m_rbIAccept->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( options::OnRbAcceptInput ), NULL, this );
    m_rbIIgnore->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( options::OnRbIgnoreInput ), NULL, this );
    m_tcInputStc->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( options::OnValChange ), NULL, this );
    m_btnInputStcList->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( options::OnBtnIStcs ), NULL, this );
    m_cbOutput->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( options::OnCbOutput ), NULL, this );
    m_rbOAccept->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( options::OnRbOutput ), NULL, this );
    m_rbOIgnore->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( options::OnRbOutput ), NULL, this );
    m_tcOutputStc->Disconnect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( options::OnValChange ), NULL, this );
    m_btnOutputStcList->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( options::OnBtnOStcs ), NULL, this );
    m_cbNMEADebug->Disconnect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( options::OnShowGpsWindowCheckboxClick ), NULL, this );

    delete m_pSerialArray;
    groupsPanel->EmptyChartGroupArray( m_pGroupArray );
    delete m_pGroupArray;
    m_pGroupArray = NULL;
    g_pOptions = NULL;
    if( m_topImgList ) delete m_topImgList;
}

void options::Init()
{
    m_pWorkDirList = NULL;

    pDebugShowStat = NULL;
    pSelCtl = NULL;
    pActiveChartsList = NULL;
    ps57CtlListBox = NULL;
    pDispCat = NULL;
    m_pSerialArray = NULL;
    pUpdateCheckBox = NULL;
    k_charts = 0;
    k_vectorcharts = 0;
    k_plugins = 0;
    k_tides = 0;

    activeSizer = NULL;
    itemActiveChartStaticBox = NULL;

    m_bVisitLang = false;
    m_itemFontElementListBox = NULL;
    m_topImgList = NULL;
    m_pSerialArray = EnumerateSerialPorts();

    m_pListbook = NULL;
    m_pGroupArray = NULL;
    m_groups_changed = 0;

    m_pageDisplay = -1;
    m_pageConnections = -1;
    m_pageCharts = -1;
    m_pageShips = -1;
    m_pageUI = -1;
    m_pagePlugins = -1;

    lastPage = -1;


    // This variable is used by plugin callback function AddOptionsPage
    g_pOptions = this;
}

bool options::Create( MyFrame* parent, wxWindowID id, const wxString& caption, const wxPoint& pos,
        const wxSize& size, long style )
{
    SetExtraStyle( GetExtraStyle() | wxWS_EX_BLOCK_EVENTS );
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    Fit();
    if( lastWindowPos == wxPoint(0,0) ) {
        Centre();
    } else {
        Move( lastWindowPos );
    }
    lastWindowPos = GetPosition();
    return TRUE;
}

wxWindow* options::GetContentWindow() const
{
    return NULL;
}

size_t options::CreatePanel( wxString title )
{
    size_t id = m_pListbook->GetPageCount();
    /* This is the default empty content for any top tab.
       It'll be replaced when we call AddPage */
    wxPanel *panel = new wxPanel( m_pListbook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, title );
    m_pListbook->AddPage( panel, title, false, id );
    return id;
}

wxScrolledWindow *options::AddPage( size_t parent, wxString title )
{
    if( parent > m_pListbook->GetPageCount() - 1 ) {
        wxLogMessage(
                wxString::Format( _T("Warning: invalid parent in options::AddPage( %d, "),
                        parent ) + title + _T(" )") );
        return NULL;
    }
    wxNotebookPage* page = m_pListbook->GetPage( parent );

    wxScrolledWindow *window;
    int style = wxVSCROLL | wxTAB_TRAVERSAL;
    if( page->IsKindOf( CLASSINFO(wxNotebook))) {
        window = new wxScrolledWindow( page, wxID_ANY, wxDefaultPosition, wxDefaultSize, style );
        window->SetScrollRate(5,5);
        ((wxNotebook *)page)->AddPage( window, title );
    } else if (page->IsKindOf(CLASSINFO(wxScrolledWindow))) {
        wxString toptitle = m_pListbook->GetPageText( parent );
        wxNotebook *nb = new wxNotebook( m_pListbook, wxID_ANY, wxDefaultPosition, wxDefaultSize,wxNB_TOP );
        /* Only remove the tab from listbook, we still have original content in {page} */
        m_pListbook->RemovePage( parent );
        m_pListbook->InsertPage( parent, nb, toptitle, false, parent );
        wxString previoustitle = page->GetName();
        page->Reparent( nb );
        nb->AddPage( page, previoustitle );
        /* wxNotebookPage is hidden under wxGTK after RemovePage/Reparent
         * we must explicitely Show() it */
        page->Show();
        window = new wxScrolledWindow( nb, wxID_ANY, wxDefaultPosition, wxDefaultSize, style );
        window->SetScrollRate(5, 5);
        nb->AddPage( window, title );
        nb->ChangeSelection( 0 );
    } else { // This is the default content, we can replace it now
        window = new wxScrolledWindow( m_pListbook, wxID_ANY, wxDefaultPosition, wxDefaultSize, style, title );
        window->SetScrollRate(5, 5);
        wxString toptitle = m_pListbook->GetPageText( parent );
        m_pListbook->DeletePage( parent );
        m_pListbook->InsertPage( parent, window, toptitle, false, parent );
    }

    return window;
}

bool options::DeletePage( wxScrolledWindow *page  )
{
    for (size_t i = 0; i < m_pListbook->GetPageCount(); i++)
    {
        wxNotebookPage* pg = m_pListbook->GetPage( i );

        if( pg->IsKindOf( CLASSINFO(wxNotebook))) {
            wxNotebook *nb = ((wxNotebook *)pg);
            for (size_t j = 0; j < nb->GetPageCount(); j++)
            {
                wxNotebookPage* spg = nb->GetPage( j );
                if ( spg == page )
                {
                    nb->DeletePage( j );
                    if (nb->GetPageCount()==1)
                    {
                        /* There's only one page, remove inner notebook */
                        spg = nb->GetPage( 0 );
                        spg->Reparent( m_pListbook );
                        nb->RemovePage( 0 );
                        wxString toptitle = m_pListbook->GetPageText( i );
                        m_pListbook->DeletePage( i );
                        m_pListbook->InsertPage( i, spg, toptitle, false, i );
                    }
                    return true;
                }
            }
        } else if (pg->IsKindOf(CLASSINFO(wxScrolledWindow)) && pg == page) {
            /* There's only one page, replace it with empty panel */
            m_pListbook->DeletePage( i );
            wxPanel *panel = new wxPanel( m_pListbook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("") );
            wxString toptitle = m_pListbook->GetPageText( i );
            m_pListbook->InsertPage( i, panel, toptitle, false, i );
            return true;
        }
    }
    return false;
}

void options::CreatePanel_NMEA( size_t parent, int border_size, int group_item_spacing, wxSize small_button_size )
{
    m_pNMEAForm = AddPage( parent, _("NMEA") );

    wxBoxSizer* bSizer4 = new wxBoxSizer( wxVERTICAL );
    m_pNMEAForm->SetSizer( bSizer4 );
    m_pNMEAForm->SetSizeHints( wxDefaultSize, wxDefaultSize );

    wxBoxSizer* bSizerOuterContainer = new wxBoxSizer( wxVERTICAL );


    wxStaticBoxSizer* sbSizerGeneral;
    sbSizerGeneral = new wxStaticBoxSizer( new wxStaticBox( m_pNMEAForm, wxID_ANY, _("General") ), wxVERTICAL );

    wxBoxSizer* bSizer151;
    bSizer151 = new wxBoxSizer( wxVERTICAL );

    wxBoxSizer* bSizer161;
    bSizer161 = new wxBoxSizer( wxVERTICAL );

    wxBoxSizer* bSizer171;
    bSizer171 = new wxBoxSizer( wxHORIZONTAL );

    m_cbFilterSogCog = new wxCheckBox( m_pNMEAForm, wxID_ANY, _("Filter NMEA Course and Speed data"), wxDefaultPosition, wxDefaultSize, 0 );
    bSizer171->Add( m_cbFilterSogCog, 0, wxALL, 5 );

    m_stFilterSec = new wxStaticText( m_pNMEAForm, wxID_ANY, _("Filter period (sec)"), wxDefaultPosition, wxDefaultSize, 0 );
    m_stFilterSec->Wrap( -1 );

    int nspace = 5;
#ifdef __WXGTK__
    nspace = 9;
#endif
    bSizer171->Add( m_stFilterSec, 0, wxALL, nspace );

    m_tFilterSec = new wxTextCtrl( m_pNMEAForm, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    bSizer171->Add( m_tFilterSec, 0, wxALL, 4 );


    bSizer161->Add( bSizer171, 1, wxEXPAND, 5 );

    wxBoxSizer* bSizer181;
    bSizer181 = new wxBoxSizer( wxVERTICAL );

    m_cbNMEADebug = new wxCheckBox( m_pNMEAForm, wxID_ANY, _("Show NMEA Debug Window"), wxDefaultPosition, wxDefaultSize, 0 );
    bSizer181->Add( m_cbNMEADebug, 0, wxALL, 5 );

    bSizer161->Add( bSizer181, 1, wxEXPAND, 5 );
    bSizer151->Add( bSizer161, 1, wxEXPAND, 5 );
    sbSizerGeneral->Add( bSizer151, 1, wxEXPAND, 5 );
    bSizerOuterContainer->Add( sbSizerGeneral, 0, wxALL|wxEXPAND, 5 );


    //  Connections listbox, etc
    wxStaticBoxSizer* sbSizerLB= new wxStaticBoxSizer( new wxStaticBox( m_pNMEAForm, wxID_ANY, _("Data Connections") ), wxVERTICAL );


    wxBoxSizer* bSizer17;
    bSizer17 = new wxBoxSizer( wxHORIZONTAL );

    m_lcSources = new wxListCtrl( m_pNMEAForm, wxID_ANY, wxDefaultPosition, wxSize(-1, 150), wxLC_REPORT|wxLC_SINGLE_SEL );
    bSizer17->Add( m_lcSources, 1, wxALL|wxEXPAND, 5 );

    wxBoxSizer* bSizer18;
    bSizer18 = new wxBoxSizer( wxVERTICAL );

    m_buttonAdd = new wxButton( m_pNMEAForm, wxID_ANY, _("Add Connection"), wxDefaultPosition, wxDefaultSize, 0 );
    bSizer18->Add( m_buttonAdd, 0, wxALL, 5 );

    m_buttonRemove = new wxButton( m_pNMEAForm, wxID_ANY, _("Remove Connection"), wxDefaultPosition, wxDefaultSize, 0 );
    m_buttonRemove->Enable( false );
    bSizer18->Add( m_buttonRemove, 0, wxALL, 5 );

    bSizer17->Add( bSizer18, 0, wxEXPAND, 5 );
    sbSizerLB->Add( bSizer17, 1, wxEXPAND, 5 );
    bSizerOuterContainer->Add( sbSizerLB, 0, wxEXPAND, 5 );


    //  Connections Properties

    sbSizerConnectionProps = new wxStaticBoxSizer( new wxStaticBox( m_pNMEAForm, wxID_ANY, _("Properties") ), wxVERTICAL );

    wxBoxSizer* bSizer15;
    bSizer15 = new wxBoxSizer( wxHORIZONTAL );

    m_rbTypeSerial = new wxRadioButton( m_pNMEAForm, wxID_ANY, _("Serial"), wxDefaultPosition, wxDefaultSize, 0 );
    m_rbTypeSerial->SetValue( true );
    bSizer15->Add( m_rbTypeSerial, 0, wxALL, 5 );

    m_rbTypeNet = new wxRadioButton( m_pNMEAForm, wxID_ANY, _("Network"), wxDefaultPosition, wxDefaultSize, 0 );
    bSizer15->Add( m_rbTypeNet, 0, wxALL, 5 );


    sbSizerConnectionProps->Add( bSizer15, 0, wxEXPAND, 0 );

    gSizerNetProps = new wxGridSizer( 0, 2, 0, 0 );

    m_stNetProto = new wxStaticText( m_pNMEAForm, wxID_ANY, _("Protocol"), wxDefaultPosition, wxDefaultSize, 0 );
    m_stNetProto->Wrap( -1 );
    gSizerNetProps->Add( m_stNetProto, 0, wxALL, 5 );

    wxBoxSizer* bSizer16;
    bSizer16 = new wxBoxSizer( wxHORIZONTAL );

    m_rbNetProtoTCP = new wxRadioButton( m_pNMEAForm, wxID_ANY, _("TCP"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
    m_rbNetProtoTCP->Enable( true );

    bSizer16->Add( m_rbNetProtoTCP, 0, wxALL, 5 );

    m_rbNetProtoUDP = new wxRadioButton( m_pNMEAForm, wxID_ANY, _("UDP"), wxDefaultPosition, wxDefaultSize, 0 );
    m_rbNetProtoUDP->Enable( true );

    bSizer16->Add( m_rbNetProtoUDP, 0, wxALL, 5 );

    m_rbNetProtoGPSD = new wxRadioButton( m_pNMEAForm, wxID_ANY, _("GPSD"), wxDefaultPosition, wxDefaultSize, 0 );
    m_rbNetProtoGPSD->SetValue( true );
    bSizer16->Add( m_rbNetProtoGPSD, 0, wxALL, 5 );


    gSizerNetProps->Add( bSizer16, 1, wxEXPAND, 5 );

    m_stNetAddr = new wxStaticText( m_pNMEAForm, wxID_ANY, _("Address"), wxDefaultPosition, wxDefaultSize, 0 );
    m_stNetAddr->Wrap( -1 );
    gSizerNetProps->Add( m_stNetAddr, 0, wxALL, 5 );

    m_tNetAddress = new wxTextCtrl( m_pNMEAForm, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    gSizerNetProps->Add( m_tNetAddress, 0, wxEXPAND|wxTOP, 5 );

    m_stNetPort = new wxStaticText( m_pNMEAForm, wxID_ANY, _("Port"), wxDefaultPosition, wxDefaultSize, 0 );
    m_stNetPort->Wrap( -1 );
    gSizerNetProps->Add( m_stNetPort, 0, wxALL, 5 );

    m_tNetPort = new wxTextCtrl( m_pNMEAForm, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    gSizerNetProps->Add( m_tNetPort, 1, wxEXPAND|wxTOP, 5 );


    sbSizerConnectionProps->Add( gSizerNetProps, 0, wxEXPAND, 5 );

    gSizerSerProps = new wxGridSizer( 0, 1, 0, 0 );

    wxFlexGridSizer* fgSizer1;
    fgSizer1 = new wxFlexGridSizer( 0, 4, 0, 0 );
    fgSizer1->SetFlexibleDirection( wxBOTH );
    fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    m_stSerPort = new wxStaticText( m_pNMEAForm, wxID_ANY, _("Port"), wxDefaultPosition, wxDefaultSize, 0 );
    m_stSerPort->Wrap( -1 );
    fgSizer1->Add( m_stSerPort, 0, wxALL, 5 );

    m_comboPort = new wxComboBox( m_pNMEAForm, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, 0 );
    fgSizer1->Add( m_comboPort, 0, wxEXPAND|wxTOP, 5 );

    m_stSerBaudrate = new wxStaticText( m_pNMEAForm, wxID_ANY, _("Baudrate"), wxDefaultPosition, wxDefaultSize, 0 );
    m_stSerBaudrate->Wrap( -1 );
    fgSizer1->Add( m_stSerBaudrate, 0, wxALL, 5 );

    wxString m_choiceBaudRateChoices[] = { _("150"), _("300"), _("600"), _("1200"), _("2400"), _("4800"), _("9600"), _("19200"), _("38400"), _("57600"), _("115200"), _("230400"), _("460800"), _("921600") };
    int m_choiceBaudRateNChoices = sizeof( m_choiceBaudRateChoices ) / sizeof( wxString );
    m_choiceBaudRate = new wxChoice( m_pNMEAForm, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_choiceBaudRateNChoices, m_choiceBaudRateChoices, 0 );
    m_choiceBaudRate->SetSelection( 0 );
    fgSizer1->Add( m_choiceBaudRate, 1, wxEXPAND|wxTOP, 5 );

    m_stSerProtocol = new wxStaticText( m_pNMEAForm, wxID_ANY, _("Protocol"), wxDefaultPosition, wxDefaultSize, 0 );
    m_stSerProtocol->Wrap( -1 );
    fgSizer1->Add( m_stSerProtocol, 0, wxALL, 5 );

    wxString m_choiceSerialProtocolChoices[] = { _("NMEA 0183"), _("NMEA 2000"), _("Seatalk") };
    int m_choiceSerialProtocolNChoices = sizeof( m_choiceSerialProtocolChoices ) / sizeof( wxString );
    m_choiceSerialProtocol = new wxChoice( m_pNMEAForm, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_choiceSerialProtocolNChoices, m_choiceSerialProtocolChoices, 0 );
    m_choiceSerialProtocol->SetSelection( 0 );
    m_choiceSerialProtocol->Enable( false );

    fgSizer1->Add( m_choiceSerialProtocol, 1, wxEXPAND|wxTOP, 5 );

    m_stPriority = new wxStaticText( m_pNMEAForm, wxID_ANY, _("Priority"), wxDefaultPosition, wxDefaultSize, 0 );
    m_stPriority->Wrap( -1 );
    fgSizer1->Add( m_stPriority, 0, wxALL, 5 );

    wxString m_choicePriorityChoices[] = { _("0"), _("1"), _("2"), _("3"), _("4"), _("5"), _("6"), _("7"), _("8"), _("9") };
    int m_choicePriorityNChoices = sizeof( m_choicePriorityChoices ) / sizeof( wxString );
    m_choicePriority = new wxChoice( m_pNMEAForm, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_choicePriorityNChoices, m_choicePriorityChoices, 0 );
    m_choicePriority->SetSelection( 9 );
    fgSizer1->Add( m_choicePriority, 0, wxEXPAND|wxTOP, 5 );


    gSizerSerProps->Add( fgSizer1, 0, wxEXPAND, 5 );

    wxFlexGridSizer* fgSizer5;
    fgSizer5 = new wxFlexGridSizer( 0, 2, 0, 0 );
    fgSizer5->SetFlexibleDirection( wxBOTH );
    fgSizer5->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    m_cbCheckCRC = new wxCheckBox( m_pNMEAForm, wxID_ANY, _("Control checksum"), wxDefaultPosition, wxDefaultSize, 0 );
    m_cbCheckCRC->SetValue(true);
    m_cbCheckCRC->SetToolTip( _("If checked, only the sentences with a valid checksum are passed through") );

    fgSizer5->Add( m_cbCheckCRC, 0, wxALL, 5 );

    m_cbGarminHost = new wxCheckBox( m_pNMEAForm, wxID_ANY, _("Use Garmin GRMN/GRMN (Host) mode for uploads"), wxDefaultPosition, wxDefaultSize, 0 );
    m_cbGarminHost->SetValue(false);
    fgSizer5->Add( m_cbGarminHost, 0, wxALL, 5 );

    m_cbFurunoGP3X = new wxCheckBox( m_pNMEAForm, wxID_ANY, _("Format uploads for Furuno GP3X"), wxDefaultPosition, wxDefaultSize, 0 );
    m_cbFurunoGP3X->SetValue(false);
	fgSizer5->Add( m_cbFurunoGP3X, 0, wxALL, 5 );

    sbSizerConnectionProps->Add( gSizerSerProps, 0, wxEXPAND, 5 );
    sbSizerConnectionProps->Add( fgSizer5, 0, wxEXPAND, 5 );

    sbSizerInFilter = new wxStaticBoxSizer( new wxStaticBox( m_pNMEAForm, wxID_ANY, _("Input filtering") ), wxVERTICAL );

    wxBoxSizer* bSizer9;
    bSizer9 = new wxBoxSizer( wxHORIZONTAL );

    m_rbIAccept = new wxRadioButton( m_pNMEAForm, wxID_ANY, _("Accept only sentences"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
    bSizer9->Add( m_rbIAccept, 0, wxALL, 5 );

    m_rbIIgnore = new wxRadioButton( m_pNMEAForm, wxID_ANY, _("Ignore sentences"), wxDefaultPosition, wxDefaultSize, 0 );
    bSizer9->Add( m_rbIIgnore, 0, wxALL, 5 );


    sbSizerInFilter->Add( bSizer9, 0, wxEXPAND, 5 );

    wxBoxSizer* bSizer11;
    bSizer11 = new wxBoxSizer( wxHORIZONTAL );

    m_tcInputStc = new wxTextCtrl( m_pNMEAForm, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
    bSizer11->Add( m_tcInputStc, 1, wxALL|wxEXPAND, 5 );

    m_btnInputStcList = new wxButton( m_pNMEAForm, wxID_ANY, _("..."), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
    bSizer11->Add( m_btnInputStcList, 0, wxALL, 5 );

    sbSizerInFilter->Add( bSizer11, 0, wxEXPAND, 5 );

    sbSizerConnectionProps->Add( sbSizerInFilter, 0, wxEXPAND, 5 );


    m_cbOutput = new wxCheckBox( m_pNMEAForm, wxID_ANY, _("Output on this port ( as Autopilot or NMEA Repeater)"), wxDefaultPosition, wxDefaultSize, 0 );
    sbSizerConnectionProps->Add( m_cbOutput, 0, wxALL, 5 );

    sbSizerOutFilter = new wxStaticBoxSizer( new wxStaticBox( m_pNMEAForm, wxID_ANY, _("Output filtering") ), wxVERTICAL );

    wxBoxSizer* bSizer10;
    bSizer10 = new wxBoxSizer( wxHORIZONTAL );

    m_rbOAccept = new wxRadioButton( m_pNMEAForm, wxID_ANY, _("Transmit sentences"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP );
    bSizer10->Add( m_rbOAccept, 0, wxALL, 5 );

    m_rbOIgnore = new wxRadioButton( m_pNMEAForm, wxID_ANY, _("Drop sentences"), wxDefaultPosition, wxDefaultSize, 0 );
    bSizer10->Add( m_rbOIgnore, 0, wxALL, 5 );


    sbSizerOutFilter->Add( bSizer10, 0, wxEXPAND, 5 );

    wxBoxSizer* bSizer12;
    bSizer12 = new wxBoxSizer( wxHORIZONTAL );

    m_tcOutputStc = new wxTextCtrl( m_pNMEAForm, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
    bSizer12->Add( m_tcOutputStc, 1, wxALL|wxEXPAND, 5 );

    m_btnOutputStcList = new wxButton( m_pNMEAForm, wxID_ANY, _("..."), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
    bSizer12->Add( m_btnOutputStcList, 0, wxALL, 5 );


    sbSizerOutFilter->Add( bSizer12, 0, wxEXPAND, 5 );
    sbSizerConnectionProps->Add( sbSizerOutFilter, 0, wxEXPAND, 5 );


    bSizerOuterContainer->Add( sbSizerConnectionProps, 1, wxALL|wxEXPAND, 5 );

    bSizer4->Add( bSizerOuterContainer, 1, wxEXPAND, 5 );

    // Connect Events
    m_lcSources->Connect( wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler( options::OnSelectDatasource ), NULL, this );
    m_buttonAdd->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( options::OnAddDatasourceClick ), NULL, this );
    m_buttonRemove->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( options::OnRemoveDatasourceClick ), NULL, this );
    m_rbTypeSerial->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( options::OnTypeSerialSelected ), NULL, this );
    m_rbTypeNet->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( options::OnTypeNetSelected ), NULL, this );
    m_rbNetProtoTCP->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( options::OnNetProtocolSelected ), NULL, this );
    m_rbNetProtoUDP->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( options::OnNetProtocolSelected ), NULL, this );
    m_rbNetProtoGPSD->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( options::OnNetProtocolSelected ), NULL, this );
    m_tNetAddress->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( options::OnConnValChange ), NULL, this );
    m_tNetPort->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( options::OnConnValChange ), NULL, this );
    m_comboPort->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler( options::OnConnValChange ), NULL, this );
    m_comboPort->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( options::OnConnValChange ), NULL, this );
    m_choiceBaudRate->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( options::OnBaudrateChoice ), NULL, this );
    m_choiceSerialProtocol->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( options::OnProtocolChoice ), NULL, this );
    m_choicePriority->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( options::OnConnValChange ), NULL, this );
    m_cbCheckCRC->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( options::OnCrcCheck ), NULL, this );
    m_cbGarminHost->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( options::OnUploadFormatChange ), NULL, this );
    m_cbFurunoGP3X->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( options::OnUploadFormatChange ), NULL, this );
    m_rbIAccept->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( options::OnRbAcceptInput ), NULL, this );
    m_rbIIgnore->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( options::OnRbIgnoreInput ), NULL, this );
    m_tcInputStc->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( options::OnConnValChange ), NULL, this );
    m_btnInputStcList->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( options::OnBtnIStcs ), NULL, this );
    m_cbOutput->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( options::OnCbOutput ), NULL, this );
    m_rbOAccept->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( options::OnRbOutput ), NULL, this );
    m_rbOIgnore->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( options::OnRbOutput ), NULL, this );
    m_tcOutputStc->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( options::OnConnValChange ), NULL, this );
    m_btnOutputStcList->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( options::OnBtnOStcs ), NULL, this );

    m_cbNMEADebug->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( options::OnShowGpsWindowCheckboxClick ), NULL, this );
    m_cbFilterSogCog->Connect( wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( options::OnValChange ), NULL, this );
    m_tFilterSec->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( options::OnValChange ), NULL, this );

    wxListItem col0;
    col0.SetId(0);
    col0.SetText( _("Type") );
    m_lcSources->InsertColumn(0, col0);

    wxListItem col1;
    col1.SetId(1);
    col1.SetText( _("Port") );
    m_lcSources->InsertColumn(1, col1);

    wxListItem col2;
    col2.SetId(2);
    col2.SetText( _("Parameters") );
    m_lcSources->InsertColumn(2, col2);

    wxListItem col3;
    col3.SetId(3);
    col3.SetText( _("Output") );
    m_lcSources->InsertColumn(3, col3);

    wxListItem col4;
    col4.SetId(4);
    col4.SetText( _("Filters") );
    m_lcSources->InsertColumn(4, col4);

    m_lcSources->Refresh();

    m_stcdialog_in = new SentenceListDlg(FILTER_INPUT, this);
    m_stcdialog_out = new SentenceListDlg(FILTER_OUTPUT, this);

    FillSourceList();

    for(size_t i = 0; i < m_pSerialArray->Count() ; i++)
    {
       m_comboPort->Append(m_pSerialArray->Item(i));
    }
    ShowNMEACommon( false );
    ShowNMEASerial( false );
    ShowNMEANet( false );
    connectionsaved = true;
}

void options::CreatePanel_Ownship( size_t parent, int border_size, int group_item_spacing, wxSize small_button_size )
{
    itemPanelShip = AddPage( parent, _("Own Ship") );

    ownShip = new wxBoxSizer( wxVERTICAL );
    itemPanelShip->SetSizer( ownShip );

    //      OwnShip Display options
    wxStaticBox* osdBox = new wxStaticBox( itemPanelShip, wxID_ANY, _("Display Options") );
    dispOptions = new wxStaticBoxSizer( osdBox, wxVERTICAL );
    ownShip->Add( dispOptions, 0, wxTOP | wxALL | wxEXPAND, border_size );

    wxFlexGridSizer* dispOptionsGrid = new wxFlexGridSizer( 2, 2, group_item_spacing, group_item_spacing );
    dispOptionsGrid->AddGrowableCol( 1 );
    dispOptions->Add( dispOptionsGrid, 0, wxALL | wxEXPAND, border_size );

    wxStaticText *pStatic_OSCOG_Predictor = new wxStaticText( itemPanelShip, wxID_ANY, _("COG Predictor Length (min)") );
    dispOptionsGrid->Add( pStatic_OSCOG_Predictor, 0 );

    m_pText_OSCOG_Predictor = new wxTextCtrl( itemPanelShip, wxID_ANY );
    dispOptionsGrid->Add( m_pText_OSCOG_Predictor, 0, wxALIGN_RIGHT );

    wxStaticText *iconTypeTxt = new wxStaticText( itemPanelShip, wxID_ANY, _("Ship Icon Type") );
    dispOptionsGrid->Add( iconTypeTxt, 0 );

    wxString iconTypes[] = { _("Default"), _("Real Scale Bitmap"), _("Real Scale Vector") };
    m_pShipIconType = new wxChoice( itemPanelShip, ID_SHIPICONTYPE, wxDefaultPosition, wxDefaultSize, 3, iconTypes );
    dispOptionsGrid->Add( m_pShipIconType, 0, wxALIGN_RIGHT | wxLEFT|wxRIGHT|wxTOP, group_item_spacing );

    realSizes = new wxFlexGridSizer( 5, 2, group_item_spacing, group_item_spacing );
    realSizes->AddGrowableCol(1);

    dispOptions->Add( realSizes, 0, wxEXPAND | wxLEFT, 30 );

    realSizes->Add( new wxStaticText( itemPanelShip, wxID_ANY, _("Length Over All (m)") ), 1, wxALIGN_LEFT );
    m_pOSLength = new wxTextCtrl( itemPanelShip, 1 );
    realSizes->Add( m_pOSLength, 1, wxALIGN_RIGHT | wxALL, group_item_spacing );

    realSizes->Add( new wxStaticText( itemPanelShip, wxID_ANY, _("Width Over All (m)") ), 1, wxALIGN_LEFT );
    m_pOSWidth = new wxTextCtrl( itemPanelShip, wxID_ANY );
    realSizes->Add( m_pOSWidth, 1, wxALIGN_RIGHT | wxALL, group_item_spacing );

    realSizes->Add( new wxStaticText( itemPanelShip, wxID_ANY, _("GPS Offset from Bow (m)") ), 1, wxALIGN_LEFT );
    m_pOSGPSOffsetY = new wxTextCtrl( itemPanelShip, wxID_ANY );
    realSizes->Add( m_pOSGPSOffsetY, 1, wxALIGN_RIGHT | wxALL, group_item_spacing );

    realSizes->Add( new wxStaticText( itemPanelShip, wxID_ANY, _("GPS Offset from Midship (m)") ), 1, wxALIGN_LEFT );
    m_pOSGPSOffsetX = new wxTextCtrl( itemPanelShip, wxID_ANY );
    realSizes->Add( m_pOSGPSOffsetX, 1, wxALIGN_RIGHT | wxALL, group_item_spacing );

    realSizes->Add( new wxStaticText( itemPanelShip, wxID_ANY, _("Minimum Screen Size (mm)") ), 1, wxALIGN_LEFT );
    m_pOSMinSize = new wxTextCtrl( itemPanelShip, wxID_ANY );
    realSizes->Add( m_pOSMinSize, 1, wxALIGN_RIGHT | wxALL, group_item_spacing );

    // Radar rings

    wxFlexGridSizer* rrSelect = new wxFlexGridSizer( 1, 2, group_item_spacing, group_item_spacing );
    rrSelect->AddGrowableCol( 1 );
    dispOptions->Add( rrSelect, 0, wxLEFT|wxRIGHT | wxEXPAND, border_size );

    wxStaticText *rrTxt = new wxStaticText( itemPanelShip, wxID_ANY, _("Show radar rings") );
    rrSelect->Add( rrTxt, 1, wxEXPAND | wxALL, group_item_spacing );

    wxString rrAlt[] = { _("None"), _T("1"), _T("2"), _T("3"), _T("4"), _T("5"), _T("6"), _T("7"), _T("8"), _T("9"), _T("10") };
    pNavAidRadarRingsNumberVisible = new wxChoice( itemPanelShip, ID_RADARRINGS, wxDefaultPosition, m_pShipIconType->GetSize(), 11, rrAlt );
    rrSelect->Add( pNavAidRadarRingsNumberVisible, 0, wxALIGN_RIGHT | wxALL, group_item_spacing );

    radarGrid = new wxFlexGridSizer( 2, 2, group_item_spacing, group_item_spacing );
    radarGrid->AddGrowableCol( 1 );
    dispOptions->Add( radarGrid, 0, wxLEFT | wxEXPAND, 30 );

    wxStaticText* distanceText = new wxStaticText( itemPanelShip, wxID_STATIC, _("Distance Between Rings") );
    radarGrid->Add( distanceText, 1, wxEXPAND | wxALL, group_item_spacing );

    pNavAidRadarRingsStep = new wxTextCtrl( itemPanelShip, ID_TEXTCTRL, _T(""), wxDefaultPosition, wxSize( 100, -1 ), 0 );
    radarGrid->Add( pNavAidRadarRingsStep, 0, wxALIGN_RIGHT | wxALL, group_item_spacing );

    wxStaticText* unitText = new wxStaticText( itemPanelShip, wxID_STATIC, _("Distance Unit") );
    radarGrid->Add( unitText, 1, wxEXPAND | wxALL, group_item_spacing );

    wxString pDistUnitsStrings[] = { _("Nautical Miles"), _("Kilometers") };
    m_itemRadarRingsUnits = new wxChoice( itemPanelShip, ID_RADARDISTUNIT, wxDefaultPosition, m_pShipIconType->GetSize(), 2, pDistUnitsStrings );
    radarGrid->Add( m_itemRadarRingsUnits, 0, wxALIGN_RIGHT | wxALL, border_size );

    //  Tracks
    wxStaticBox* trackText = new wxStaticBox( itemPanelShip, wxID_ANY, _("Tracks") );
    wxStaticBoxSizer* trackSizer = new wxStaticBoxSizer( trackText, wxVERTICAL );
    ownShip->Add( trackSizer, 0, wxGROW | wxALL, border_size );

    pTrackDaily = new wxCheckBox( itemPanelShip, ID_DAILYCHECKBOX, _("Automatic Daily Tracks") );
    trackSizer->Add( pTrackDaily, 1, wxALL, border_size );

    pTrackHighlite = new wxCheckBox( itemPanelShip, ID_TRACKHILITE, _("Highlight Tracks") );
    trackSizer->Add( pTrackHighlite, 1, wxALL, border_size );

    wxFlexGridSizer *pTrackGrid = new wxFlexGridSizer( 1, 2, group_item_spacing, group_item_spacing );
    pTrackGrid->AddGrowableCol( 1 );
    trackSizer->Add( pTrackGrid, 0, wxALL | wxEXPAND, border_size );

    wxStaticText* tpText = new wxStaticText( itemPanelShip, wxID_STATIC, _("Tracking Precision") );
    pTrackGrid->Add( tpText, 1, wxEXPAND | wxALL, group_item_spacing );

    wxString trackAlt[] = { _("Low"), _("Medium"), _("High") };
    pTrackPrecision = new wxChoice( itemPanelShip, wxID_ANY, wxDefaultPosition, m_pShipIconType->GetSize(), 3, trackAlt );
    pTrackGrid->Add( pTrackPrecision, 0, wxALIGN_RIGHT | wxALL, group_item_spacing );

    DimeControl( itemPanelShip );
}

void options::CreatePanel_ChartsLoad( size_t parent, int border_size, int group_item_spacing,
        wxSize small_button_size )
{
    wxScrolledWindow *chartPanelWin = AddPage( m_pageCharts, _("Loaded Charts") );

    chartPanel = new wxBoxSizer( wxVERTICAL );
    chartPanelWin->SetSizer( chartPanel );

    wxStaticBox* loadedBox = new wxStaticBox( chartPanelWin, wxID_ANY, _("Directories") );
    activeSizer = new wxStaticBoxSizer( loadedBox, wxHORIZONTAL );
    chartPanel->Add( activeSizer, 1, wxALL | wxEXPAND, border_size );

    wxString* pListBoxStrings = NULL;
    pActiveChartsList = new wxListBox( chartPanelWin, ID_LISTBOX, wxDefaultPosition, wxDefaultSize,
             0, pListBoxStrings, wxLB_MULTIPLE );

    activeSizer->Add( pActiveChartsList, 1, wxALL | wxEXPAND, border_size );

    wxBoxSizer* cmdButtonSizer = new wxBoxSizer( wxVERTICAL );
    activeSizer->Add( cmdButtonSizer, 0, wxALL, border_size );

    // Currently loaded chart dirs
    wxString dirname;
    if( pActiveChartsList ) {
        pActiveChartsList->Clear();
        int nDir = m_CurrentDirList.GetCount();
        for( int i = 0; i < nDir; i++ ) {
            dirname = m_CurrentDirList.Item( i ).fullpath;
            if( !dirname.IsEmpty() ) pActiveChartsList->Append( dirname );
        }
    }

    wxButton* addBtn = new wxButton( chartPanelWin, ID_BUTTONADD, _("Add Directory...") );
    cmdButtonSizer->Add( addBtn, 1, wxALL | wxEXPAND, group_item_spacing );

    wxButton* removeBtn = new wxButton( chartPanelWin, ID_BUTTONDELETE, _("Remove Selected") );
    cmdButtonSizer->Add( removeBtn, 1, wxALL | wxEXPAND, group_item_spacing );

    wxStaticBox* itemStaticBoxUpdateStatic = new wxStaticBox( chartPanelWin, wxID_ANY,
            _("Update Control") );
    wxStaticBoxSizer* itemStaticBoxSizerUpdate = new wxStaticBoxSizer( itemStaticBoxUpdateStatic,
            wxVERTICAL );
    chartPanel->Add( itemStaticBoxSizerUpdate, 0, wxGROW | wxALL, 5 );

    pScanCheckBox = new wxCheckBox( chartPanelWin, ID_SCANCHECKBOX,
            _("Scan Charts and Update Database") );
    itemStaticBoxSizerUpdate->Add( pScanCheckBox, 1, wxALL, 5 );

    pUpdateCheckBox = new wxCheckBox( chartPanelWin, ID_UPDCHECKBOX,
            _("Force Full Database Rebuild") );
    itemStaticBoxSizerUpdate->Add( pUpdateCheckBox, 1, wxALL, 5 );

    chartPanel->Layout();
}

void options::CreatePanel_VectorCharts( size_t parent, int border_size, int group_item_spacing,
        wxSize small_button_size )
{
    ps57Ctl = AddPage( parent, _("Vector Charts") );

    vectorPanel = new wxFlexGridSizer( 2, 3, border_size, border_size );
    vectorPanel->AddGrowableCol( 0, 1 );

    ps57Ctl->SetSizer( vectorPanel );

    wxStaticBox* marinersBox = new wxStaticBox( ps57Ctl, wxID_ANY,
            _("Mariner's Standard") );
    wxStaticBoxSizer* marinersSizer = new wxStaticBoxSizer( marinersBox,
            wxVERTICAL );
    vectorPanel->Add( marinersSizer, 1, wxALL | wxEXPAND, border_size );

    wxString* ps57CtlListBoxStrings = NULL;
    ps57CtlListBox = new wxCheckListBox( ps57Ctl, ID_CHECKLISTBOX, wxDefaultPosition,
            wxSize( 200, 250 ), 0, ps57CtlListBoxStrings, wxLB_SINGLE | wxLB_HSCROLL | wxLB_SORT );
    marinersSizer->Add( ps57CtlListBox, 1, wxALL | wxEXPAND, group_item_spacing );

    wxBoxSizer* btnRow = new wxBoxSizer( wxHORIZONTAL );
    itemButtonSelectList = new wxButton( ps57Ctl, ID_SELECTLIST, _("Select All") );
    btnRow->Add( itemButtonSelectList, 0, wxALL, group_item_spacing );
    itemButtonClearList = new wxButton( ps57Ctl, ID_CLEARLIST, _("Clear All") );
    btnRow->Add( itemButtonClearList, 0, wxALL, group_item_spacing );
    marinersSizer->Add( btnRow );

    wxBoxSizer* catSizer = new wxBoxSizer( wxVERTICAL );
    vectorPanel->Add( catSizer, 1, wxALL | wxEXPAND, group_item_spacing );

    wxString pDispCatStrings[] = { _("Base"), _("Standard"), _("Other"), _("Mariners Standard") };
    pDispCat = new wxRadioBox( ps57Ctl, ID_RADARDISTUNIT, _("Display Category"), wxDefaultPosition,
            wxDefaultSize, 4, pDispCatStrings, 1, wxRA_SPECIFY_COLS );
    catSizer->Add( pDispCat, 0, wxALL | wxEXPAND, 2 );

    pCheck_SOUNDG = new wxCheckBox( ps57Ctl, ID_SOUNDGCHECKBOX, _("Depth Soundings") );
    pCheck_SOUNDG->SetValue( FALSE );
    catSizer->Add( pCheck_SOUNDG, 1, wxALL | wxEXPAND, group_item_spacing );

    pCheck_META = new wxCheckBox( ps57Ctl, ID_METACHECKBOX, _("Chart Information Objects") );
    pCheck_META->SetValue( FALSE );
    catSizer->Add( pCheck_META, 1, wxALL | wxEXPAND, group_item_spacing );

    pCheck_SHOWIMPTEXT = new wxCheckBox( ps57Ctl, ID_IMPTEXTCHECKBOX,
            _("Important Text Only") );
    pCheck_SHOWIMPTEXT->SetValue( FALSE );
    catSizer->Add( pCheck_SHOWIMPTEXT, 1, wxALL | wxEXPAND, group_item_spacing );

    pCheck_SCAMIN = new wxCheckBox( ps57Ctl, ID_SCAMINCHECKBOX, _("Reduced Detail at Small Scale") );
    pCheck_SCAMIN->SetValue( FALSE );
    catSizer->Add( pCheck_SCAMIN, 1, wxALL | wxEXPAND, group_item_spacing );

    pCheck_ATONTEXT = new wxCheckBox( ps57Ctl, ID_ATONTEXTCHECKBOX, _("Buoy/Light Labels") );
    pCheck_SCAMIN->SetValue( FALSE );
    catSizer->Add( pCheck_ATONTEXT, 1, wxALL | wxEXPAND, group_item_spacing );

    pCheck_LDISTEXT = new wxCheckBox( ps57Ctl, ID_LDISTEXTCHECKBOX, _("Light Descriptions") );
    pCheck_LDISTEXT->SetValue( FALSE );
    catSizer->Add( pCheck_LDISTEXT, 1, wxALL | wxEXPAND, group_item_spacing );

    pCheck_XLSECTTEXT = new wxCheckBox( ps57Ctl, ID_LDISTEXTCHECKBOX, _("Extended Light Sectors") );
    pCheck_XLSECTTEXT->SetValue( FALSE );
    catSizer->Add( pCheck_XLSECTTEXT, 1, wxALL | wxEXPAND, group_item_spacing );

    pCheck_DECLTEXT = new wxCheckBox( ps57Ctl, ID_DECLTEXTCHECKBOX, _("De-Cluttered Text") );
    pCheck_DECLTEXT->SetValue( FALSE );
    catSizer->Add( pCheck_DECLTEXT, 1, wxALL | wxEXPAND, group_item_spacing );

    wxBoxSizer* styleSizer = new wxBoxSizer( wxVERTICAL );
    vectorPanel->Add( styleSizer, 1, wxALL | wxEXPAND, 0 );

    wxString pPointStyleStrings[] = { _("Paper Chart"), _("Simplified"), };
    pPointStyle = new wxRadioBox( ps57Ctl, ID_RADARDISTUNIT, _("Points"), wxDefaultPosition,
            wxDefaultSize, 2, pPointStyleStrings, 1, wxRA_SPECIFY_COLS );
    styleSizer->Add( pPointStyle, 0, wxALL | wxEXPAND, group_item_spacing );

    wxString pBoundStyleStrings[] = { _("Plain"), _("Symbolized"), };
    pBoundStyle = new wxRadioBox( ps57Ctl, ID_RADARDISTUNIT, _("Boundaries"), wxDefaultPosition,
            wxDefaultSize, 2, pBoundStyleStrings, 1, wxRA_SPECIFY_COLS );
    styleSizer->Add( pBoundStyle, 0, wxALL | wxEXPAND, group_item_spacing );

    wxString pColorNumStrings[] = { _("2 Color"), _("4 Color"), };
    p24Color = new wxRadioBox( ps57Ctl, ID_RADARDISTUNIT, _("Colors"), wxDefaultPosition, wxDefaultSize,
            2, pColorNumStrings, 1, wxRA_SPECIFY_COLS );
    styleSizer->Add( p24Color, 0, wxALL | wxEXPAND, group_item_spacing );

    wxStaticBox* depthBox = new wxStaticBox( ps57Ctl, wxID_ANY, _("Depth Settings(m)") );
    wxStaticBoxSizer* depthsSizer = new wxStaticBoxSizer( depthBox, wxVERTICAL );
    vectorPanel->Add( depthsSizer, 0, wxALL | wxEXPAND, border_size );

    wxStaticText* itemStaticText4 = new wxStaticText( ps57Ctl, wxID_STATIC, _("Shallow Depth") );
    depthsSizer->Add( itemStaticText4, 0,
            wxLEFT | wxRIGHT | wxTOP | wxADJUST_MINSIZE, group_item_spacing );

    m_ShallowCtl = new wxTextCtrl( ps57Ctl, ID_TEXTCTRL, _T(""), wxDefaultPosition,
            wxSize( 120, -1 ), 0 );
    depthsSizer->Add( m_ShallowCtl, 0, wxLEFT | wxRIGHT | wxBOTTOM,
            group_item_spacing );

    wxStaticText* itemStaticText5 = new wxStaticText( ps57Ctl, wxID_STATIC, _("Safety Depth") );
    depthsSizer->Add( itemStaticText5, 0,
            wxLEFT | wxRIGHT | wxTOP | wxADJUST_MINSIZE, group_item_spacing );

    m_SafetyCtl = new wxTextCtrl( ps57Ctl, ID_TEXTCTRL, _T(""), wxDefaultPosition,
            wxSize( 120, -1 ), 0 );
    depthsSizer->Add( m_SafetyCtl, 0, wxLEFT | wxRIGHT | wxBOTTOM,
            group_item_spacing );

    wxStaticText* itemStaticText6 = new wxStaticText( ps57Ctl, wxID_STATIC, _("Deep Depth") );
    depthsSizer->Add( itemStaticText6, 0,
            wxLEFT | wxRIGHT | wxTOP | wxADJUST_MINSIZE, group_item_spacing );

    m_DeepCtl = new wxTextCtrl( ps57Ctl, ID_TEXTCTRL, _T(""), wxDefaultPosition, wxSize( 120, -1 ),
            0 );
    depthsSizer->Add( m_DeepCtl, 0, wxLEFT | wxRIGHT | wxBOTTOM,
            group_item_spacing );

    wxString pDepthUnitStrings[] = { _("Feet"), _("Meters"), _("Fathoms"), };

    pDepthUnitSelect = new wxRadioBox( ps57Ctl, ID_RADARDISTUNIT, _("Chart Depth Units"),
            wxDefaultPosition, wxDefaultSize, 3, pDepthUnitStrings, 1, wxRA_SPECIFY_COLS );
    vectorPanel->Add( pDepthUnitSelect, 1, wxALL | wxEXPAND, border_size );

#ifdef USE_S57
    wxStaticBox *cm93DetailBox = new wxStaticBox( ps57Ctl, wxID_ANY, _("CM93 Detail Level") );
    wxStaticBoxSizer* cm93Sizer = new wxStaticBoxSizer( cm93DetailBox, wxVERTICAL );
    m_pSlider_CM93_Zoom = new wxSlider( ps57Ctl, ID_CM93ZOOM, 0, -CM93_ZOOM_FACTOR_MAX_RANGE,
            CM93_ZOOM_FACTOR_MAX_RANGE, wxDefaultPosition, wxSize( 140, 50),
            wxSL_HORIZONTAL | wxSL_AUTOTICKS | wxSL_LABELS );
    cm93Sizer->Add( m_pSlider_CM93_Zoom, 0, wxALL | wxEXPAND, border_size );
    cm93Sizer->SetSizeHints(cm93DetailBox);
    vectorPanel->Add( cm93Sizer, 1, wxALL | wxEXPAND, border_size );
#endif

}

void options::CreatePanel_TidesCurrents( size_t parent, int border_size, int group_item_spacing,
        wxSize small_button_size )
{
    wxScrolledWindow *tcPanel = AddPage( parent, _("Tides && Currents") );

    wxBoxSizer* mainHBoxSizer = new wxBoxSizer( wxVERTICAL );
    tcPanel->SetSizer( mainHBoxSizer );

    wxStaticBox *tcBox = new wxStaticBox( tcPanel, wxID_ANY, _("Active Datasets" ));
    wxStaticBoxSizer* tcSizer = new wxStaticBoxSizer( tcBox, wxHORIZONTAL );
    mainHBoxSizer->Add( tcSizer, 1, wxALL | wxEXPAND, border_size );

    tcDataSelected = new wxListBox( tcPanel, ID_TIDESELECTED, wxDefaultPosition, wxDefaultSize );

    tcSizer->Add( tcDataSelected, 1, wxALL | wxEXPAND, border_size );

    //  Populate Selection List Control with the contents
    //  of the Global static array
    for( unsigned int id = 0 ; id < TideCurrentDataSet.Count() ; id++ ) {
        tcDataSelected->Append( TideCurrentDataSet.Item(id) );
    }

    //    Add the "Insert/Remove" buttons
    wxButton *insertButton = new wxButton( tcPanel, ID_TCDATAADD, _("Add Dataset...") );
    wxButton *removeButton = new wxButton( tcPanel, ID_TCDATADEL, _("Remove Selected") );

    wxBoxSizer* btnSizer = new wxBoxSizer( wxVERTICAL );
    tcSizer->Add( btnSizer );

    btnSizer->Add( insertButton, 1, wxALL | wxEXPAND, group_item_spacing );
    btnSizer->Add( removeButton, 1, wxALL | wxEXPAND, group_item_spacing );
    
}

void options::CreatePanel_ChartGroups( size_t parent, int border_size, int group_item_spacing,
        wxSize small_button_size )
{
    // Special case for adding the tab here. We know this page has multiple tabs,
    // and we have the actual widgets in a separate class (because of its complexity)

    wxNotebookPage* page = m_pListbook->GetPage( parent );
    groupsPanel = new ChartGroupsUI( page );

    groupsPanel->CreatePanel( parent, border_size, group_item_spacing, small_button_size );
    ((wxNotebook *)page)->AddPage( groupsPanel, _("Chart Groups") );

    page->Connect( wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED, wxListbookEventHandler( options::OnChartsPageChange ), NULL, this );

}

void ChartGroupsUI::CreatePanel( size_t parent, int border_size, int group_item_spacing,
        wxSize small_button_size )
{
    modified = false;
    m_border_size = border_size;
    m_group_item_spacing = group_item_spacing;

    groupsSizer = new wxFlexGridSizer( 4, 2, border_size, border_size );
    groupsSizer->AddGrowableCol( 0 );
    groupsSizer->AddGrowableRow( 1, 1 );
    groupsSizer->AddGrowableRow( 3, 1 );

    SetSizer( groupsSizer );

    m_UIcomplete = false;
}

void ChartGroupsUI::CompletePanel( void )
{
    //    The chart file/dir tree
    wxStaticText *allChartsLabel = new wxStaticText( this, wxID_ANY, _("All Available Charts") );
    groupsSizer->Add( allChartsLabel, 0, wxTOP | wxRIGHT | wxLEFT, m_border_size );

    wxStaticText *dummy1 = new wxStaticText( this, -1, _T("") );
    groupsSizer->Add( dummy1 );

    wxBoxSizer* activeListSizer = new wxBoxSizer( wxVERTICAL );
    groupsSizer->Add( activeListSizer, 1, wxALL | wxEXPAND, 5 );

    allAvailableCtl = new wxGenericDirCtrl( this, ID_GROUPAVAILABLE, _T(""), wxDefaultPosition, wxDefaultSize, wxVSCROLL );
    activeListSizer->Add( allAvailableCtl, 1, wxEXPAND );

    m_pAddButton = new wxButton( this, ID_GROUPINSERTDIR, _("Add") );
    m_pAddButton->Disable();
    m_pRemoveButton = new wxButton( this, ID_GROUPREMOVEDIR, _("Remove Chart") );
    m_pRemoveButton->Disable();

    wxBoxSizer* addRemove = new wxBoxSizer( wxVERTICAL );
    addRemove->Add( m_pAddButton, 0, wxALL | wxEXPAND, m_group_item_spacing );
    groupsSizer->Add( addRemove, 0, wxALL | wxEXPAND, m_border_size );

    //    Add the Groups notebook control
    wxStaticText *groupsLabel = new wxStaticText( this, wxID_ANY, _("Chart Groups") );
    groupsSizer->Add( groupsLabel, 0, wxTOP | wxRIGHT | wxLEFT, m_border_size );

    wxStaticText *dummy2 = new wxStaticText( this, -1, _T("") );
    groupsSizer->Add( dummy2 );

    wxBoxSizer* nbSizer = new wxBoxSizer( wxVERTICAL );
    m_GroupNB = new wxNotebook( this, ID_GROUPNOTEBOOK, wxDefaultPosition, wxDefaultSize, wxNB_TOP );
    nbSizer->Add( m_GroupNB, 1, wxEXPAND );
    groupsSizer->Add( nbSizer, 1, wxALL | wxEXPAND, m_border_size );

    //    Add default (always present) Default Chart Group
    wxPanel *allActiveGroup = new wxPanel( m_GroupNB, -1, wxDefaultPosition, wxDefaultSize );
    m_GroupNB->AddPage( allActiveGroup, _("All Charts") );

    wxBoxSizer* page0BoxSizer = new wxBoxSizer( wxHORIZONTAL );
    allActiveGroup->SetSizer( page0BoxSizer );

    defaultAllCtl = new wxGenericDirCtrl( allActiveGroup, -1, _T(""), wxDefaultPosition, wxDefaultSize, wxVSCROLL );

    //    Set the Font for the All Active Chart Group tree to be italic, dimmed
    iFont = wxTheFontList->FindOrCreateFont( 10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_ITALIC,
    wxFONTWEIGHT_LIGHT );

    page0BoxSizer->Add( defaultAllCtl, 1, wxALIGN_TOP | wxALL | wxEXPAND );

    m_DirCtrlArray.Add( defaultAllCtl );

    //    Add the Chart Group (page) "New" and "Delete" buttons
    m_pNewGroupButton = new wxButton( this, ID_GROUPNEWGROUP, _("New Group...") );
    m_pDeleteGroupButton = new wxButton( this, ID_GROUPDELETEGROUP, _("Delete Group") );

    wxBoxSizer* newDeleteGrp = new wxBoxSizer( wxVERTICAL );
    groupsSizer->Add( newDeleteGrp, 0, wxALL, m_border_size );

    newDeleteGrp->AddSpacer( 25 );
    newDeleteGrp->Add( m_pNewGroupButton, 0, wxALL | wxEXPAND, m_group_item_spacing );
    newDeleteGrp->Add( m_pDeleteGroupButton, 0, wxALL | wxEXPAND, m_group_item_spacing );
    newDeleteGrp->AddSpacer( 25 );
    newDeleteGrp->Add( m_pRemoveButton, 0, wxALL | wxEXPAND, m_group_item_spacing );

    // Connect this last, otherwise handler is called before all objects are initialized.
    this->Connect( wxEVT_COMMAND_TREE_SEL_CHANGED,
    wxTreeEventHandler(ChartGroupsUI::OnAvailableSelection), NULL, this );

    m_UIcomplete = true;

}

void options::CreatePanel_Display( size_t parent, int border_size, int group_item_spacing,
        wxSize small_button_size )
{
    wxScrolledWindow *itemPanelUI = AddPage( parent, _("Display") );

    wxBoxSizer* itemBoxSizerUI = new wxBoxSizer( wxVERTICAL );
    itemPanelUI->SetSizer( itemBoxSizerUI );

    // Chart Display Options Box
    wxStaticBox* itemStaticBoxSizerCDOStatic = new wxStaticBox( itemPanelUI, wxID_ANY,
            _("Chart Display Options") );
    wxStaticBoxSizer* itemStaticBoxSizerCDO = new wxStaticBoxSizer( itemStaticBoxSizerCDOStatic,
            wxVERTICAL );
    itemBoxSizerUI->Add( itemStaticBoxSizerCDO, 0, wxEXPAND | wxALL, border_size );

    //  "Course Up" checkbox
    pCBCourseUp = new wxCheckBox( itemPanelUI, ID_COURSEUPCHECKBOX, _("Course UP Mode") );
    itemStaticBoxSizerCDO->Add( pCBCourseUp, 0, wxALL, border_size );

    //  Course Up display update period
    wxFlexGridSizer *pCOGUPFilterGrid = new wxFlexGridSizer( 2 );
    pCOGUPFilterGrid->AddGrowableCol( 1 );
    itemStaticBoxSizerCDO->Add( pCOGUPFilterGrid, 0, wxALL | wxEXPAND, border_size );

    wxStaticText* itemStaticTextCOGUPFilterSecs = new wxStaticText( itemPanelUI, wxID_STATIC,
            _("Course-Up Mode Display Update Period (sec)") );
    pCOGUPFilterGrid->Add( itemStaticTextCOGUPFilterSecs, 0, wxADJUST_MINSIZE,
            border_size );

    pCOGUPUpdateSecs = new wxTextCtrl( itemPanelUI, ID_TEXTCTRL, _T(""), wxDefaultPosition,
            wxDefaultSize );
    pCOGUPFilterGrid->Add( pCOGUPUpdateSecs, 0, wxALIGN_RIGHT | wxALL, border_size );

    //  "LookAhead" checkbox
    pCBLookAhead = new wxCheckBox( itemPanelUI, ID_CHECK_LOOKAHEAD, _("Look Ahead Mode") );
    itemStaticBoxSizerCDO->Add( pCBLookAhead, 0, wxALL, border_size );

    //  Grid display  checkbox
    pSDisplayGrid = new wxCheckBox( itemPanelUI, ID_CHECK_DISPLAYGRID, _("Show Grid") );
    itemStaticBoxSizerCDO->Add( pSDisplayGrid, 1, wxALL, border_size );

    //  Depth Unit checkbox
    pSDepthUnits = new wxCheckBox( itemPanelUI, ID_SHOWDEPTHUNITSBOX1, _("Show Depth Units") );
    itemStaticBoxSizerCDO->Add( pSDepthUnits, 1, wxALL, border_size );

    //  OpenGL Render checkbox
    pOpenGL = new wxCheckBox( itemPanelUI, ID_OPENGLBOX, _("Use Accelerated Graphics (OpenGL)") );
    itemStaticBoxSizerCDO->Add( pOpenGL, 1, wxALL, border_size );

    //  Smooth Pan/Zoom checkbox
    pSmoothPanZoom = new wxCheckBox( itemPanelUI, ID_SMOOTHPANZOOMBOX,
            _("Smooth Panning / Zooming") );
    itemStaticBoxSizerCDO->Add( pSmoothPanZoom, 1, wxALL, border_size );

    pEnableZoomToCursor = new wxCheckBox( itemPanelUI, ID_ZTCCHECKBOX,
            _("Zoom to Cursor") );
    pEnableZoomToCursor->SetValue( FALSE );
    itemStaticBoxSizerCDO->Add( pEnableZoomToCursor, 1, wxALL, border_size );

    pPreserveScale = new wxCheckBox( itemPanelUI, ID_PRESERVECHECKBOX,
            _("Preserve Scale when Switching Charts") );
    itemStaticBoxSizerCDO->Add( pPreserveScale, 1, wxALL, border_size );

    //  Quilting checkbox
    pCDOQuilting = new wxCheckBox( itemPanelUI, ID_QUILTCHECKBOX1, _("Enable Chart Quilting") );
    itemStaticBoxSizerCDO->Add( pCDOQuilting, 1, wxALL, border_size );

    //  Full Screen Quilting Disable checkbox
    pFullScreenQuilt = new wxCheckBox( itemPanelUI, ID_FULLSCREENQUILT,
            _("Disable Full Screen Quilting") );
    itemStaticBoxSizerCDO->Add( pFullScreenQuilt, 1, wxALL, border_size );

    //  Chart Outlines checkbox
    pCDOOutlines = new wxCheckBox( itemPanelUI, ID_OUTLINECHECKBOX1, _("Show Chart Outlines") );
    itemStaticBoxSizerCDO->Add( pCDOOutlines, 1, wxALL, border_size );

    //  Skewed Raster compenstation checkbox
    pSkewComp = new wxCheckBox( itemPanelUI, ID_SKEWCOMPBOX,
            _("Show Skewed Raster Charts as North-Up") );
    itemStaticBoxSizerCDO->Add( pSkewComp, 1, wxALL, border_size );
}

void options::CreatePanel_AIS( size_t parent, int border_size, int group_item_spacing, wxSize small_button_size )
{
    wxScrolledWindow *panelAIS = AddPage( parent, _("AIS Targets") );

    wxBoxSizer* aisSizer = new wxBoxSizer( wxVERTICAL );
    panelAIS->SetSizer( aisSizer );

    //      CPA Box
    wxStaticBox* itemStaticBoxCPA = new wxStaticBox( panelAIS, wxID_ANY, _("CPA Calculation") );
    wxStaticBoxSizer* itemStaticBoxSizerCPA = new wxStaticBoxSizer( itemStaticBoxCPA, wxVERTICAL );
    aisSizer->Add( itemStaticBoxSizerCPA, 0, wxALL | wxEXPAND, border_size );

    wxFlexGridSizer *pCPAGrid = new wxFlexGridSizer( 2 );
    pCPAGrid->AddGrowableCol( 1 );
    itemStaticBoxSizerCPA->Add( pCPAGrid, 0, wxALL | wxEXPAND, border_size );

    m_pCheck_CPA_Max = new wxCheckBox( panelAIS, -1,
            _("No CPA Calculation if target range is greater than (NMi)") );
    pCPAGrid->Add( m_pCheck_CPA_Max, 0, wxALL, group_item_spacing );

    m_pText_CPA_Max = new wxTextCtrl( panelAIS, -1 );
    pCPAGrid->Add( m_pText_CPA_Max, 0, wxALL | wxALIGN_RIGHT, group_item_spacing );

    m_pCheck_CPA_Warn = new wxCheckBox( panelAIS, -1, _("Warn if CPA less than (NMi)") );
    pCPAGrid->Add( m_pCheck_CPA_Warn, 0, wxALL, group_item_spacing );

    m_pText_CPA_Warn = new wxTextCtrl( panelAIS, -1, _T(""), wxDefaultPosition,
            wxSize( -1, -1 ) );
    pCPAGrid->Add( m_pText_CPA_Warn, 0, wxALL | wxALIGN_RIGHT, group_item_spacing );

    m_pCheck_CPA_WarnT = new wxCheckBox( panelAIS, -1,
            _("...and TCPA is less than (min)") );
    pCPAGrid->Add( m_pCheck_CPA_WarnT, 0, wxALL, group_item_spacing );

    m_pText_CPA_WarnT = new wxTextCtrl( panelAIS, -1 );
    pCPAGrid->Add( m_pText_CPA_WarnT, 0, wxALL | wxALIGN_RIGHT, group_item_spacing );

    //      Lost Targets
    wxStaticBox* lostBox = new wxStaticBox( panelAIS, wxID_ANY, _("Lost Targets") );
    wxStaticBoxSizer* lostSizer = new wxStaticBoxSizer( lostBox, wxVERTICAL );
    aisSizer->Add( lostSizer, 0, wxALL | wxEXPAND, 3 );

    wxFlexGridSizer *pLostGrid = new wxFlexGridSizer( 2 );
    pLostGrid->AddGrowableCol( 1 );
    lostSizer->Add( pLostGrid, 0, wxALL | wxEXPAND, border_size );

    m_pCheck_Mark_Lost = new wxCheckBox( panelAIS, -1, _("Mark targets as lost after (min)") );
    pLostGrid->Add( m_pCheck_Mark_Lost, 1, wxALL, group_item_spacing );

    m_pText_Mark_Lost = new wxTextCtrl( panelAIS, -1 );
    pLostGrid->Add( m_pText_Mark_Lost, 1, wxALL | wxALIGN_RIGHT, group_item_spacing );

    m_pCheck_Remove_Lost = new wxCheckBox( panelAIS, -1,
            _("Remove lost targets after (min)") );
    pLostGrid->Add( m_pCheck_Remove_Lost, 1, wxALL, group_item_spacing );

    m_pText_Remove_Lost = new wxTextCtrl( panelAIS, -1 );
    pLostGrid->Add( m_pText_Remove_Lost, 1, wxALL | wxALIGN_RIGHT, group_item_spacing );

    //      Display
    wxStaticBox* displBox = new wxStaticBox( panelAIS, wxID_ANY, _("Display") );
    wxStaticBoxSizer* displSizer = new wxStaticBoxSizer( displBox, wxHORIZONTAL );
    aisSizer->Add( displSizer, 0, wxALL | wxEXPAND, border_size );

    wxFlexGridSizer *pDisplayGrid = new wxFlexGridSizer( 2 );
    pDisplayGrid->AddGrowableCol( 1 );
    displSizer->Add( pDisplayGrid, 1, wxALL | wxEXPAND, border_size );

    m_pCheck_Show_COG = new wxCheckBox( panelAIS, -1, _("Show target COG predictor arrow, length (min)") );
    pDisplayGrid->Add( m_pCheck_Show_COG, 1, wxALL | wxEXPAND, group_item_spacing );

    m_pText_COG_Predictor = new wxTextCtrl( panelAIS, -1 );
    pDisplayGrid->Add( m_pText_COG_Predictor, 1, wxALL | wxALIGN_RIGHT, group_item_spacing );

    m_pCheck_Show_Tracks = new wxCheckBox( panelAIS, -1, _("Show target tracks, length (min)") );
    pDisplayGrid->Add( m_pCheck_Show_Tracks, 1, wxALL, group_item_spacing );

    m_pText_Track_Length = new wxTextCtrl( panelAIS, -1 );
    pDisplayGrid->Add( m_pText_Track_Length, 1, wxALL | wxALIGN_RIGHT, group_item_spacing );

    m_pCheck_Show_Moored = new wxCheckBox( panelAIS, -1, _("Hide anchored/moored targets, speed max (kn)") );
    pDisplayGrid->Add( m_pCheck_Show_Moored, 1, wxALL, group_item_spacing );

    m_pText_Moored_Speed = new wxTextCtrl( panelAIS, -1 );
    pDisplayGrid->Add( m_pText_Moored_Speed, 1, wxALL | wxALIGN_RIGHT, group_item_spacing );

    m_pCheck_Show_Area_Notices = new wxCheckBox( panelAIS, -1,
            _("Show area notices (from AIS binary messages)") );
    pDisplayGrid->Add( m_pCheck_Show_Area_Notices, 1, wxALL, group_item_spacing );

    wxStaticText *pStatic_Dummy5 = new wxStaticText( panelAIS, -1, _T("") );
    pDisplayGrid->Add( pStatic_Dummy5, 1, wxALL | wxALL, group_item_spacing );

    m_pCheck_Draw_Target_Size = new wxCheckBox( panelAIS, -1, _("Show AIS targets real size") );
    pDisplayGrid->Add( m_pCheck_Draw_Target_Size, 1, wxALL, group_item_spacing );

    wxStaticText *pStatic_Dummy5a = new wxStaticText( panelAIS, -1, _T("") );
    pDisplayGrid->Add( pStatic_Dummy5a, 1, wxALL | wxALL, group_item_spacing );

    // Rollover
    wxStaticBox* rolloverBox = new wxStaticBox( panelAIS, wxID_ANY, _("Rollover") );
    wxStaticBoxSizer* rolloverSizer = new wxStaticBoxSizer( rolloverBox, wxVERTICAL );
    aisSizer->Add( rolloverSizer, 0, wxALL | wxEXPAND, border_size );

    wxStaticText *pStatic_Dummy4 = new wxStaticText( panelAIS, -1, _("\"Ship Name\" MMSI (Call Sign)") );
    rolloverSizer->Add( pStatic_Dummy4, 1, wxALL, 2 * group_item_spacing );

    m_pCheck_Rollover_Class = new wxCheckBox( panelAIS, -1, _("[Class] Type (Status)") );
    rolloverSizer->Add( m_pCheck_Rollover_Class, 1, wxALL, 2 * group_item_spacing );

    m_pCheck_Rollover_COG = new wxCheckBox( panelAIS, -1, _("SOG COG") );
    rolloverSizer->Add( m_pCheck_Rollover_COG, 1, wxALL, 2 * group_item_spacing );

    m_pCheck_Rollover_CPA = new wxCheckBox( panelAIS, -1, _("CPA TCPA") );
    rolloverSizer->Add( m_pCheck_Rollover_CPA, 1, wxALL, 2 * group_item_spacing );

    //      Alert Box
    wxStaticBox* alertBox = new wxStaticBox( panelAIS, wxID_ANY, _("CPA/TCPA Alerts") );
    wxStaticBoxSizer* alertSizer = new wxStaticBoxSizer( alertBox, wxVERTICAL );
    aisSizer->Add( alertSizer, 0, wxALL | wxEXPAND, group_item_spacing );

    wxFlexGridSizer *pAlertGrid = new wxFlexGridSizer( 2 );
    pAlertGrid->AddGrowableCol( 1 );
    alertSizer->Add( pAlertGrid, 0, wxALL | wxEXPAND, group_item_spacing );

    m_pCheck_AlertDialog = new wxCheckBox( panelAIS, ID_AISALERTDIALOG,
            _("Show CPA/TCPA Alert Dialog") );
    pAlertGrid->Add( m_pCheck_AlertDialog, 0, wxALL, group_item_spacing );

    wxButton *m_SelSound = new wxButton( panelAIS, ID_AISALERTSELECTSOUND,
            _("Select Alert Sound"), wxDefaultPosition, small_button_size, 0 );
    pAlertGrid->Add( m_SelSound, 0, wxALL | wxALIGN_RIGHT, group_item_spacing );

    m_pCheck_AlertAudio = new wxCheckBox( panelAIS, ID_AISALERTAUDIO,
            _("Play Sound on CPA/TCPA Alerts") );
    pAlertGrid->Add( m_pCheck_AlertAudio, 0, wxALL, group_item_spacing );

    wxButton *m_pPlay_Sound = new wxButton( panelAIS, ID_AISALERTTESTSOUND,
            _("Test Alert Sound"), wxDefaultPosition, small_button_size, 0 );
    pAlertGrid->Add( m_pPlay_Sound, 0, wxALL | wxALIGN_RIGHT, group_item_spacing );

    m_pCheck_Alert_Moored = new wxCheckBox( panelAIS, -1,
            _("Supress Alerts for anchored/moored targets") );
    pAlertGrid->Add( m_pCheck_Alert_Moored, 1, wxALL, group_item_spacing );

    wxStaticText *pStatic_Dummy2 = new wxStaticText( panelAIS, -1, _T("") );
    pAlertGrid->Add( pStatic_Dummy2, 1, wxALL | wxALL, group_item_spacing );

    m_pCheck_Ack_Timout = new wxCheckBox( panelAIS, -1,
            _("Enable Target Alert Acknowledge timeout (min)") );
    pAlertGrid->Add( m_pCheck_Ack_Timout, 1, wxALL, group_item_spacing );

    m_pText_ACK_Timeout = new wxTextCtrl( panelAIS, -1 );
    pAlertGrid->Add( m_pText_ACK_Timeout, 1, wxALL | wxALIGN_RIGHT, group_item_spacing );

    panelAIS->Layout();
}


void options::CreatePanel_UI( size_t parent, int border_size, int group_item_spacing,
        wxSize small_button_size )
{
    wxScrolledWindow *itemPanelFont = AddPage( parent, _("General Options") );


    m_itemBoxSizerFontPanel = new wxBoxSizer( wxVERTICAL );
    itemPanelFont->SetSizer( m_itemBoxSizerFontPanel );

    wxBoxSizer* langStyleBox = new wxBoxSizer( wxHORIZONTAL );
    m_itemBoxSizerFontPanel->Add( langStyleBox, 0, wxEXPAND | wxALL, border_size );

    wxStaticBox* itemLangStaticBox = new wxStaticBox( itemPanelFont, wxID_ANY,
            _("Language") );
    wxStaticBoxSizer* itemLangStaticBoxSizer = new wxStaticBoxSizer( itemLangStaticBox,
            wxVERTICAL );

    langStyleBox->Add( itemLangStaticBoxSizer, 1, wxEXPAND | wxALL, border_size );

    m_itemLangListBox = new wxComboBox( itemPanelFont, ID_CHOICE_LANG );

    itemLangStaticBoxSizer->Add( m_itemLangListBox, 0, wxEXPAND | wxALL, border_size );

    wxStaticBox* itemFontStaticBox = new wxStaticBox( itemPanelFont, wxID_ANY,
            _("Fonts") );
    wxStaticBoxSizer* itemFontStaticBoxSizer = new wxStaticBoxSizer( itemFontStaticBox,
            wxHORIZONTAL );
    m_itemBoxSizerFontPanel->Add( itemFontStaticBoxSizer, 0, wxEXPAND | wxALL, border_size );

    m_itemFontElementListBox = new wxComboBox( itemPanelFont, ID_CHOICE_FONTELEMENT );

    int nFonts = pFontMgr->GetNumFonts();
    for( int it = 0; it < nFonts; it++ ) {
        wxString *t = pFontMgr->GetDialogString( it );

        if( pFontMgr->GetConfigString( it )->StartsWith( g_locale ) ) {
            m_itemFontElementListBox->Append( *t );
        }
    }

    if( nFonts ) m_itemFontElementListBox->SetSelection( 0 );

    itemFontStaticBoxSizer->Add( m_itemFontElementListBox, 0, wxALL, border_size );

    wxButton* itemFontChooseButton = new wxButton( itemPanelFont, ID_BUTTONFONTCHOOSE,
            _("Choose Font..."), wxDefaultPosition, wxDefaultSize, 0 );
    itemFontStaticBoxSizer->Add( itemFontChooseButton, 0, wxALL, border_size );

    wxStaticBox* itemStyleStaticBox = new wxStaticBox( itemPanelFont, wxID_ANY,
            _("Toolbar and Window Style") );
    wxStaticBoxSizer* itemStyleStaticBoxSizer = new wxStaticBoxSizer( itemStyleStaticBox,
            wxVERTICAL );
    langStyleBox->Add( itemStyleStaticBoxSizer, 1, wxEXPAND | wxALL, border_size );

    m_itemStyleListBox = new wxComboBox( itemPanelFont, ID_STYLESCOMBOBOX );

    wxArrayPtrVoid styles = g_StyleManager->GetArrayOfStyles();
    for( unsigned int i = 0; i < styles.Count(); i++ ) {
        ocpnStyle::Style* style = (ocpnStyle::Style*) ( styles.Item( i ) );
        m_itemStyleListBox->Append( style->name );
    }
    m_itemStyleListBox->SetValue( g_StyleManager->GetCurrentStyle()->name );
    itemStyleStaticBoxSizer->Add( m_itemStyleListBox, 1, wxEXPAND | wxALL, border_size );

    wxStaticBox* miscOptionsBox = new wxStaticBox( itemPanelFont, wxID_ANY, _("Miscellaneous Options") );
    wxStaticBoxSizer* miscOptions = new wxStaticBoxSizer( miscOptionsBox, wxVERTICAL );
    m_itemBoxSizerFontPanel->Add( miscOptions, 0, wxALL | wxEXPAND, border_size );

    pDebugShowStat = new wxCheckBox( itemPanelFont, ID_DEBUGCHECKBOX1, _("Show Status Bar") );
    pDebugShowStat->SetValue( FALSE );
    miscOptions->Add( pDebugShowStat, 0, wxALL, border_size );

    pFullScreenToolbar = new wxCheckBox( itemPanelFont, ID_FSTOOLBARCHECKBOX,
            _("Show Toolbar in Fullscreen Mode") );
    miscOptions->Add( pFullScreenToolbar, 0, wxALL, border_size );

    pTransparentToolbar = new wxCheckBox( itemPanelFont, ID_TRANSTOOLBARCHECKBOX,
            _("Enable Transparent Toolbar") );
    miscOptions->Add( pTransparentToolbar, 0, wxALL, border_size );
    if( g_bopengl ) pTransparentToolbar->Disable();

    wxFlexGridSizer *pFormatGrid = new wxFlexGridSizer( 2 );
    pFormatGrid->AddGrowableCol( 1 );
    miscOptions->Add( pFormatGrid, 0, wxALL | wxEXPAND, border_size );

    wxStaticText* itemStaticTextSDMMFormat = new wxStaticText( itemPanelFont, wxID_STATIC,
            _("Show Lat/Long as") );
    pFormatGrid->Add( itemStaticTextSDMMFormat, 0,
            wxLEFT | wxRIGHT | wxTOP | wxADJUST_MINSIZE, border_size );

    wxString pSDMMFormats[] = { _("Degrees, Decimal Minutes"), _("Decimal Degrees"),
            _("Degrees, Minutes, Seconds") };
    int m_SDMMFormatsNChoices = sizeof( pSDMMFormats ) / sizeof(wxString);
    pSDMMFormat = new wxChoice( itemPanelFont, ID_SDMMFORMATCHOICE, wxDefaultPosition,
            wxDefaultSize, m_SDMMFormatsNChoices, pSDMMFormats );
    pFormatGrid->Add( pSDMMFormat, 0, wxALIGN_RIGHT, 2 );

    pPlayShipsBells = new wxCheckBox( itemPanelFont, ID_BELLSCHECKBOX, _("Play Ships Bells"));
    miscOptions->Add( pPlayShipsBells, 0, wxALIGN_LEFT|wxALL, border_size);

    pWayPointPreventDragging = new wxCheckBox( itemPanelFont, ID_DRAGGINGCHECKBOX,
            _("Lock Waypoints (Unless waypoint property dialog visible)") );
    pWayPointPreventDragging->SetValue( FALSE );
    miscOptions->Add( pWayPointPreventDragging, 0, wxALL,
            border_size );

    pConfirmObjectDeletion = new wxCheckBox( itemPanelFont, ID_DELETECHECKBOX,
                                               _("Confirm deletion of tracks and routes") );
    pConfirmObjectDeletion->SetValue( FALSE );
    miscOptions->Add( pConfirmObjectDeletion, 0, wxALL, border_size );
                                               
                                               
}

void options::CreateControls()
{
    int border_size = 4;
    int check_spacing = 4;
    int group_item_spacing = 2;           // use for items within one group, with Add(...wxALL)

    wxFont *qFont = wxTheFontList->FindOrCreateFont( 10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
            wxFONTWEIGHT_NORMAL );
    SetFont( *qFont );

    int font_size_y, font_descent, font_lead;
    GetTextExtent( _T("0"), NULL, &font_size_y, &font_descent, &font_lead );
    wxSize small_button_size( -1, (int) ( 1.4 * ( font_size_y + font_descent + font_lead ) ) );

    //      Some members (pointers to controls) need to initialized
    pEnableZoomToCursor = NULL;
    pSmoothPanZoom = NULL;

    //      Check the display size.
    //      If "small", adjust some factors to squish out some more white space
    int width, height;
    ::wxDisplaySize( &width, &height );

    if( height <= 800 ) {
        border_size = 2;
        check_spacing = 2;
        group_item_spacing = 1;

        wxFont *sFont = wxTheFontList->FindOrCreateFont( 8, wxFONTFAMILY_DEFAULT,
                wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL );
        SetFont( *sFont );

        int font_size_y, font_descent, font_lead;
        GetTextExtent( _T("0"), NULL, &font_size_y, &font_descent, &font_lead );
        small_button_size = wxSize( -1,
                (int) ( 1.5 * ( font_size_y + font_descent + font_lead ) ) );
    }

    options* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer( wxVERTICAL );
    itemDialog1->SetSizer( itemBoxSizer2 );

    m_pListbook = new wxListbook( itemDialog1, ID_NOTEBOOK, wxDefaultPosition, wxSize(-1, -1),
            wxLB_TOP );
    m_topImgList = new wxImageList( 40, 40, true, 1 );
    ocpnStyle::Style* style = g_StyleManager->GetCurrentStyle();

#if wxCHECK_VERSION(2, 8, 12)
    m_topImgList->Add( style->GetIcon( _T("Display") ) );
    m_topImgList->Add( style->GetIcon( _T("Connections") ) );
    m_topImgList->Add( style->GetIcon( _T("Charts") ) );
    m_topImgList->Add( style->GetIcon( _T("Ship") ) );
    m_topImgList->Add( style->GetIcon( _T("UI") ) );
    m_topImgList->Add( style->GetIcon( _T("Plugins") ) );
#else
    wxBitmap bmp;
    wxImage img;
    bmp = style->GetIcon( _T("Display") ); img = bmp.ConvertToImage(); img.ConvertAlphaToMask(128);
    bmp = wxBitmap( img ); m_topImgList->Add( bmp );
    bmp = style->GetIcon( _T("Connections") ); img = bmp.ConvertToImage(); img.ConvertAlphaToMask(128);
    bmp = wxBitmap( img ); m_topImgList->Add( bmp );
    bmp = style->GetIcon( _T("Charts") ); img = bmp.ConvertToImage(); img.ConvertAlphaToMask(128);
    bmp = wxBitmap( img ); m_topImgList->Add( bmp );
    bmp = style->GetIcon( _T("Ship") ); img = bmp.ConvertToImage(); img.ConvertAlphaToMask(128);
    bmp = wxBitmap( img ); m_topImgList->Add( bmp );
    bmp = style->GetIcon( _T("UI") ); img = bmp.ConvertToImage(); img.ConvertAlphaToMask(128);
    bmp = wxBitmap( img ); m_topImgList->Add( bmp );
    bmp = style->GetIcon( _T("Plugins") ); img = bmp.ConvertToImage(); img.ConvertAlphaToMask(128);
    bmp = wxBitmap( img ); m_topImgList->Add( bmp );
#endif

    m_pListbook->SetImageList( m_topImgList );
    itemBoxSizer2->Add( m_pListbook, 1,
            wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL | wxALL | wxEXPAND, border_size );

    wxBoxSizer* buttons = new wxBoxSizer( wxHORIZONTAL );
    itemBoxSizer2->Add( buttons, 0, wxALIGN_RIGHT | wxALL, border_size );

    m_OKButton = new wxButton( itemDialog1, xID_OK, _("Ok") );
    m_OKButton->SetDefault();
    buttons->Add( m_OKButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, border_size );

    m_CancelButton = new wxButton( itemDialog1, wxID_CANCEL, _("&Cancel") );
    buttons->Add( m_CancelButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, border_size );

    m_ApplyButton = new wxButton( itemDialog1, ID_APPLY, _("Apply") );
    buttons->Add( m_ApplyButton, 0, wxALIGN_CENTER_VERTICAL | wxALL, border_size );

    m_pageDisplay = CreatePanel( _("Display") );
    CreatePanel_Display( m_pageDisplay, border_size, group_item_spacing, small_button_size );

    m_pageConnections = CreatePanel( _("Connections") );
    CreatePanel_NMEA( m_pageConnections, border_size, group_item_spacing, small_button_size );

    m_pageCharts = CreatePanel( _("Charts") );
    CreatePanel_ChartsLoad( m_pageCharts, border_size, group_item_spacing, small_button_size );
    CreatePanel_VectorCharts( m_pageCharts, border_size, group_item_spacing, small_button_size );
    // ChartGroups must be created after ChartsLoad
    CreatePanel_ChartGroups( m_pageCharts, border_size, group_item_spacing, small_button_size );
    CreatePanel_TidesCurrents( m_pageCharts, border_size, group_item_spacing, small_button_size );

    m_pageShips = CreatePanel( _("Ships") );
    CreatePanel_Ownship( m_pageShips, border_size, group_item_spacing, small_button_size );
    CreatePanel_AIS( m_pageShips, border_size, group_item_spacing, small_button_size );

    m_pageUI = CreatePanel( _("User Interface") );
    CreatePanel_UI( m_pageUI, border_size, group_item_spacing, small_button_size );

    m_pagePlugins = CreatePanel( _("Plugins") );
    wxScrolledWindow *itemPanelPlugins = AddPage( m_pagePlugins, _("Plugins") );

    wxBoxSizer* itemBoxSizerPanelPlugins = new wxBoxSizer( wxVERTICAL );
    itemPanelPlugins->SetSizer( itemBoxSizerPanelPlugins );

    //      Build the PlugIn Manager Panel
    m_pPlugInCtrl = new PluginListPanel( itemPanelPlugins, ID_PANELPIM, wxDefaultPosition,
            wxDefaultSize, g_pi_manager->GetPlugInArray() );
    m_pPlugInCtrl->SetScrollRate( 5, 5 );

    itemBoxSizerPanelPlugins->Add( m_pPlugInCtrl, 1, wxEXPAND|wxALL, border_size );

    //      PlugIns can add panels, too
    if( g_pi_manager ) g_pi_manager->NotifySetupOptions();

    pSettingsCB1 = pDebugShowStat;

    SetColorScheme( (ColorScheme) 0 );

    if( height < 768 ) {
        SetSizeHints( width-200, height-200, -1, -1 );
    } else {
        vectorPanel->SetSizeHints( ps57Ctl );
    }

    m_pListbook->Connect( wxEVT_COMMAND_LISTBOOK_PAGE_CHANGED, wxListbookEventHandler( options::OnPageChange ), NULL, this );
}

void options::SetColorScheme( ColorScheme cs )
{
    DimeControl( this );
}

void options::SetInitialSettings()
{
    wxString s;
    wxString dirname;

    // ChartsLoad

    int nDir = m_CurrentDirList.GetCount();

    for( int i = 0; i < nDir; i++ ) {
        dirname = m_CurrentDirList.Item( i ).fullpath;
        if( !dirname.IsEmpty() ) {
            if( pActiveChartsList ) {
                pActiveChartsList->Append( dirname );
            }
        }
    }

    // ChartGroups

    if( pActiveChartsList ) {
        UpdateWorkArrayFromTextCtl();
        groupsPanel->SetDBDirs( *m_pWorkDirList );

        // Make a deep copy of the current global Group Array
        groupsPanel->EmptyChartGroupArray( m_pGroupArray );
        delete m_pGroupArray;
        m_pGroupArray = groupsPanel->CloneChartGroupArray( g_pGroupArray );
        groupsPanel->SetGroupArray( m_pGroupArray );
        groupsPanel->SetInitialSettings();
    }

    if( m_pConfig ) pSettingsCB1->SetValue( m_pConfig->m_bShowDebugWindows );

    if( g_NMEALogWindow ) m_cbNMEADebug->SetValue( true );
/*TODO
    if( g_bGarminHost ) pGarminHost->SetValue( true );
*/
    m_cbFilterSogCog->SetValue( g_bfilter_cogsog );

    s.Printf( _T("%d"), g_COGFilterSec );
    m_tFilterSec->SetValue( s );

    s.Printf( _T("%d"), g_COGAvgSec );
    pCOGUPUpdateSecs->SetValue( s );

    pCDOOutlines->SetValue( g_bShowOutlines );
    pCDOQuilting->SetValue( g_bQuiltEnable );
    pFullScreenQuilt->SetValue( !g_bFullScreenQuilt );
    pSDepthUnits->SetValue( g_bShowDepthUnits );
    pSkewComp->SetValue( g_bskew_comp );
    pOpenGL->SetValue( g_bopengl );
    pSmoothPanZoom->SetValue( g_bsmoothpanzoom );
    if( g_bEnableZoomToCursor || pEnableZoomToCursor->GetValue() ) {
        pSmoothPanZoom->SetValue( false );
        pSmoothPanZoom->Disable();
    }

    pSDisplayGrid->SetValue( g_bDisplayGrid );

    pCBCourseUp->SetValue( g_bCourseUp );
    pCBLookAhead->SetValue( g_bLookAhead );

    if( fabs( wxRound( g_ownship_predictor_minutes ) - g_ownship_predictor_minutes ) > 1e-4 ) s.Printf(
            _T("%6.2f"), g_ownship_predictor_minutes );
    else
        s.Printf( _T("%4.0f"), g_ownship_predictor_minutes );
    m_pText_OSCOG_Predictor->SetValue( s );

    m_pShipIconType->SetSelection( g_OwnShipIconType );
    wxCommandEvent eDummy;
    OnShipTypeSelect( eDummy );
    m_pOSLength->SetValue( wxString::Format( _T("%.1f"), g_n_ownship_length_meters ) );
    m_pOSWidth->SetValue( wxString::Format( _T("%.1f"), g_n_ownship_beam_meters ) );
    m_pOSGPSOffsetX->SetValue( wxString::Format( _T("%.1f"), g_n_gps_antenna_offset_x ) );
    m_pOSGPSOffsetY->SetValue( wxString::Format( _T("%.1f"), g_n_gps_antenna_offset_y ) );
    m_pOSMinSize->SetValue( wxString::Format( _T("%d"), g_n_ownship_min_mm ) );

    wxString buf;
    if( g_iNavAidRadarRingsNumberVisible > 10 ) g_iNavAidRadarRingsNumberVisible = 10;
    pNavAidRadarRingsNumberVisible->SetSelection( g_iNavAidRadarRingsNumberVisible );
    buf.Printf( _T("%.3f"), g_fNavAidRadarRingsStep );
    pNavAidRadarRingsStep->SetValue( buf );
    m_itemRadarRingsUnits->SetSelection( g_pNavAidRadarRingsStepUnits );
    OnRadarringSelect( eDummy );

    pWayPointPreventDragging->SetValue( g_bWayPointPreventDragging );
    pConfirmObjectDeletion->SetValue( g_bConfirmObjectDelete );
    
    pEnableZoomToCursor->SetValue( g_bEnableZoomToCursor );
    if( pEnableZoomToCursor->GetValue() ) {
        pSmoothPanZoom->Disable();
    } else {
        pSmoothPanZoom->Enable();
    }

    pPreserveScale->SetValue( g_bPreserveScaleOnX );
    pPlayShipsBells->SetValue( g_bPlayShipsBells );
    pFullScreenToolbar->SetValue( g_bFullscreenToolbar );
    pTransparentToolbar->SetValue( g_bTransparentToolbar );
    pSDMMFormat->Select( g_iSDMMFormat );

    pTrackDaily->SetValue( g_bTrackDaily );
    pTrackHighlite->SetValue( g_bHighliteTracks );

    pTrackPrecision->SetSelection( g_nTrackPrecision );

    //    AIS Parameters
    //      CPA Box
    m_pCheck_CPA_Max->SetValue( g_bCPAMax );

    s.Printf( _T("%4.1f"), g_CPAMax_NM );
    m_pText_CPA_Max->SetValue( s );

    m_pCheck_CPA_Warn->SetValue( g_bCPAWarn );

    s.Printf( _T("%4.1f"), g_CPAWarn_NM );
    m_pText_CPA_Warn->SetValue( s );

    m_pCheck_CPA_WarnT->SetValue( g_bTCPA_Max );

    s.Printf( _T("%4.0f"), g_TCPA_Max );
    m_pText_CPA_WarnT->SetValue( s );

    //      Lost Targets
    m_pCheck_Mark_Lost->SetValue( g_bMarkLost );

    s.Printf( _T("%4.0f"), g_MarkLost_Mins );
    m_pText_Mark_Lost->SetValue( s );

    m_pCheck_Remove_Lost->SetValue( g_bRemoveLost );

    s.Printf( _T("%4.0f"), g_RemoveLost_Mins );
    m_pText_Remove_Lost->SetValue( s );

    //      Display
    m_pCheck_Show_COG->SetValue( g_bShowCOG );

    s.Printf( _T("%4.0f"), g_ShowCOG_Mins );
    m_pText_COG_Predictor->SetValue( s );

    m_pCheck_Show_Tracks->SetValue( g_bAISShowTracks );

    s.Printf( _T("%4.0f"), g_AISShowTracks_Mins );
    m_pText_Track_Length->SetValue( s );

    m_pCheck_Show_Moored->SetValue( !g_bShowMoored );

    s.Printf( _T("%4.1f"), g_ShowMoored_Kts );
    m_pText_Moored_Speed->SetValue( s );

    m_pCheck_Show_Area_Notices->SetValue( g_bShowAreaNotices );

    m_pCheck_Draw_Target_Size->SetValue( g_bDrawAISSize );

    //      Alerts
    m_pCheck_AlertDialog->SetValue( g_bAIS_CPA_Alert );
    m_pCheck_AlertAudio->SetValue( g_bAIS_CPA_Alert_Audio );
    m_pCheck_Alert_Moored->SetValue( g_bAIS_CPA_Alert_Suppress_Moored );

    m_pCheck_Ack_Timout->SetValue( g_bAIS_ACK_Timeout );
    s.Printf( _T("%4.0f"), g_AckTimeout_Mins );
    m_pText_ACK_Timeout->SetValue( s );

    // Rollover
    m_pCheck_Rollover_Class->SetValue( g_bAISRolloverShowClass );
    m_pCheck_Rollover_COG->SetValue( g_bAISRolloverShowCOG );
    m_pCheck_Rollover_CPA->SetValue( g_bAISRolloverShowCPA );

#ifdef USE_S57
    m_pSlider_CM93_Zoom->SetValue( g_cm93_zoom_factor );

//    Diplay Category
    if( ps52plib ) {

        //    S52 Primary Filters
        ps57CtlListBox->Clear();
        marinersStdXref.clear();

        for( unsigned int iPtr = 0; iPtr < ps52plib->pOBJLArray->GetCount(); iPtr++ ) {
            OBJLElement *pOLE = (OBJLElement *) ( ps52plib->pOBJLArray->Item( iPtr ) );
            wxString item;
            if( iPtr < ps52plib->OBJLDescriptions.size() ) {
                item = ps52plib->OBJLDescriptions[iPtr];
            } else {
                item = wxString( pOLE->OBJLName, wxConvUTF8 );
            }

            // The ListBox control will insert entries in sorted order, which means we need to
            // keep track of already inseted items that gets pushed down the line.
            int newpos = ps57CtlListBox->Append( item );
            marinersStdXref.push_back( newpos );
            for( size_t i=0; i<iPtr; i++ ) {
                if( marinersStdXref[i] >= newpos ) marinersStdXref[i]++;
            }

            ps57CtlListBox->Check( newpos, !( pOLE->nViz == 0 ) );
        }

        int nset = 2;                             // default OTHER
        switch( ps52plib->m_nDisplayCategory ){
            case ( DISPLAYBASE ):
                nset = 0;
                break;
            case ( STANDARD ):
                nset = 1;
                break;
            case ( OTHER ):
                nset = 2;
                break;
            case ( MARINERS_STANDARD ):
                nset = 3;
                break;
            default:
                nset = 3;
                break;
        }

        pDispCat->SetSelection( nset );

        ps57CtlListBox->Enable( MARINERS_STANDARD == ps52plib->m_nDisplayCategory );
        itemButtonClearList->Enable( MARINERS_STANDARD == ps52plib->m_nDisplayCategory );
        itemButtonSelectList->Enable( MARINERS_STANDARD == ps52plib->m_nDisplayCategory );

        //  Other Display Filters
        pCheck_SOUNDG->SetValue( ps52plib->m_bShowSoundg );
        pCheck_META->SetValue( ps52plib->m_bShowMeta );
        pCheck_SHOWIMPTEXT->SetValue( ps52plib->m_bShowS57ImportantTextOnly );
        pCheck_SCAMIN->SetValue( ps52plib->m_bUseSCAMIN );
        pCheck_ATONTEXT->SetValue( ps52plib->m_bShowAtonText );
        pCheck_LDISTEXT->SetValue( ps52plib->m_bShowLdisText );
        pCheck_XLSECTTEXT->SetValue( ps52plib->m_bExtendLightSectors );
        pCheck_DECLTEXT->SetValue( ps52plib->m_bDeClutterText );

        // Chart Display Style
        if( ps52plib->m_nSymbolStyle == PAPER_CHART ) pPointStyle->SetSelection( 0 );
        else
            pPointStyle->SetSelection( 1 );

        if( ps52plib->m_nBoundaryStyle == PLAIN_BOUNDARIES ) pBoundStyle->SetSelection( 0 );
        else
            pBoundStyle->SetSelection( 1 );

        if( S52_getMarinerParam( S52_MAR_TWO_SHADES ) == 1.0 ) p24Color->SetSelection( 0 );
        else
            p24Color->SetSelection( 1 );

        wxString s;
        s.Printf( _T("%6.2f"), S52_getMarinerParam( S52_MAR_SAFETY_CONTOUR ) );
        m_SafetyCtl->SetValue( s );

        s.Printf( _T("%6.2f"), S52_getMarinerParam( S52_MAR_SHALLOW_CONTOUR ) );
        m_ShallowCtl->SetValue( s );

        s.Printf( _T("%6.2f"), S52_getMarinerParam( S52_MAR_DEEP_CONTOUR ) );
        m_DeepCtl->SetValue( s );

        pDepthUnitSelect->SetSelection( ps52plib->m_nDepthUnitDisplay );
    }
#endif

}

void options::OnShowGpsWindowCheckboxClick( wxCommandEvent& event )
{
    if( !m_cbNMEADebug->GetValue() ) {
        if( g_NMEALogWindow ) g_NMEALogWindow->Destroy();
    } else {
        g_NMEALogWindow = new TTYWindow( pParent, 35 );
        wxString com_string = _("NMEA Debug Window");
        g_NMEALogWindow->SetTitle( com_string );

        //    Make sure the window is well on the screen
        g_NMEALogWindow_x = wxMax(g_NMEALogWindow_x, 40);
        g_NMEALogWindow_y = wxMax(g_NMEALogWindow_y, 40);

        g_NMEALogWindow->SetSize( g_NMEALogWindow_x, g_NMEALogWindow_y, g_NMEALogWindow_sx,
                g_NMEALogWindow_sy );
        g_NMEALogWindow->Show();
    }
}

void options::OnZTCCheckboxClick( wxCommandEvent& event )
{
    if( pEnableZoomToCursor->GetValue() ) {
        pSmoothPanZoom->Disable();
    } else {
        pSmoothPanZoom->Enable();
    }
}

void options::OnShipTypeSelect( wxCommandEvent& event )
{
    if( m_pShipIconType->GetSelection() == 0 ) {
        realSizes->ShowItems( false );
    } else {
        realSizes->ShowItems( true );
    }
    dispOptions->Layout();
    ownShip->Layout();
    itemPanelShip->Layout();
    itemPanelShip->Refresh();
    event.Skip();
}

void options::OnRadarringSelect( wxCommandEvent& event )
{
    if( pNavAidRadarRingsNumberVisible->GetSelection() == 0 ) {
        radarGrid->ShowItems( false );
    } else {
        radarGrid->ShowItems( true );
    }
    dispOptions->Layout();
    ownShip->Layout();
    itemPanelShip->Layout();
    itemPanelShip->Refresh();
    event.Skip();
}

void options::OnDisplayCategoryRadioButton( wxCommandEvent& event )
{
    int select = pDispCat->GetSelection();

    if( 3 == select ) {
        ps57CtlListBox->Enable();
        itemButtonClearList->Enable();
        itemButtonSelectList->Enable();
    }

    else {
        ps57CtlListBox->Disable();
        itemButtonClearList->Disable();
        itemButtonSelectList->Disable();
    }

    event.Skip();
}

void options::OnButtonClearClick( wxCommandEvent& event )
{
    int nOBJL = ps57CtlListBox->GetCount();
    for( int iPtr = 0; iPtr < nOBJL; iPtr++ )
        ps57CtlListBox->Check( iPtr, false );

    event.Skip();
}

void options::OnButtonSelectClick( wxCommandEvent& event )
{
    int nOBJL = ps57CtlListBox->GetCount();
    for( int iPtr = 0; iPtr < nOBJL; iPtr++ )
        ps57CtlListBox->Check( iPtr, true );

    event.Skip();
}

bool options::ShowToolTips()
{
    return TRUE;
}

void options::OnCharHook( wxKeyEvent& event ) {
    if( event.GetKeyCode() == WXK_RETURN ) {
        if( event.GetModifiers() == wxMOD_CONTROL ) {
            wxCommandEvent okEvent;
            okEvent.SetId( xID_OK );
            okEvent.SetEventType( wxEVT_COMMAND_BUTTON_CLICKED );
            GetEventHandler()->AddPendingEvent( okEvent );
        }
    }
	event.Skip();
}

void options::OnButtonaddClick( wxCommandEvent& event )
{
    wxString selDir;
    wxFileName dirname;
    wxDirDialog *dirSelector = new wxDirDialog( this, _("Add a directory containing chart files"),
            *pInit_Chart_Dir, wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST );

    if( dirSelector->ShowModal() == wxID_CANCEL ) goto done;

    selDir = dirSelector->GetPath();
    dirname = wxFileName( selDir );

    pInit_Chart_Dir->Empty();
    pInit_Chart_Dir->Append( dirname.GetPath() );

    if( g_bportable ) {
        wxFileName f( selDir );
        f.MakeRelativeTo( *pHome_Locn );
        pActiveChartsList->Append( f.GetFullPath() );
    } else
        pActiveChartsList->Append( selDir );

    k_charts |= CHANGE_CHARTS;

    pScanCheckBox->Disable();

    done:
    delete dirSelector;
    event.Skip();
}

void options::UpdateWorkArrayFromTextCtl()
{
    wxString dirname;

    int n = pActiveChartsList->GetCount();
    if( m_pWorkDirList ) {
        m_pWorkDirList->Clear();
        for( int i = 0; i < n; i++ ) {
            dirname = pActiveChartsList->GetString( i );
            if( !dirname.IsEmpty() ) {
                //    This is a fix for OSX, which appends EOL to results of GetLineText()
                while( ( dirname.Last() == wxChar( _T('\n') ) )
                        || ( dirname.Last() == wxChar( _T('\r') ) ) )
                    dirname.RemoveLast();

                //    scan the current array to find a match
                //    if found, add the info to the work list, preserving the magic number
                //    If not found, make a new ChartDirInfo, and add it
                bool b_added = false;
//                        if(m_pCurrentDirList)
                {
                    int nDir = m_CurrentDirList.GetCount();

                    for( int i = 0; i < nDir; i++ ) {
                        if( m_CurrentDirList.Item( i ).fullpath == dirname ) {
                            ChartDirInfo cdi = m_CurrentDirList.Item( i );
                            m_pWorkDirList->Add( cdi );
                            b_added = true;
                            break;
                        }
                    }
                }
                if( !b_added ) {
                    ChartDirInfo cdin;
                    cdin.fullpath = dirname;
                    m_pWorkDirList->Add( cdin );
                }
            }
        }
    }
}

ConnectionParams * options::SaveConnectionParams()
{
    if ( connectionsaved )
        return NULL;

    //  Special encoding for deleted connection
    if ( m_rbTypeSerial->GetValue() && m_comboPort->GetValue() == _T("Deleted" ))
        return NULL;
        
    if ( m_rbTypeSerial->GetValue() && m_comboPort->GetValue() == wxEmptyString )
    {
        wxMessageBox( _("You must select or enter the port..."), _("Error!") );
        return NULL;
    }
    else if ( m_rbTypeNet->GetValue() && m_tNetAddress->GetValue() == wxEmptyString )
    {
        wxMessageBox( _("You must enter the address..."), _("Error!") );
        return NULL;
    }

    ConnectionParams * m_pConnectionParams = new ConnectionParams();
    
    m_pConnectionParams->Valid = true;
    if ( m_rbTypeSerial->GetValue() )
        m_pConnectionParams->Type = Serial;
    else
        m_pConnectionParams->Type = Network;
    m_pConnectionParams->NetworkAddress = m_tNetAddress->GetValue();
    m_pConnectionParams->NetworkPort = wxAtoi(m_tNetPort->GetValue());
    if ( m_rbNetProtoTCP->GetValue() )
        m_pConnectionParams->NetProtocol = TCP;
    else if ( m_rbNetProtoUDP->GetValue() )
        m_pConnectionParams->NetProtocol = UDP;
    else
        m_pConnectionParams->NetProtocol = GPSD;

    m_pConnectionParams->Baudrate = wxAtoi( m_choiceBaudRate->GetStringSelection() );
    m_pConnectionParams->Priority = wxAtoi( m_choicePriority->GetStringSelection() );
    m_pConnectionParams->ChecksumCheck = m_cbCheckCRC->GetValue();
    m_pConnectionParams->Garmin = m_cbGarminHost->GetValue();
    m_pConnectionParams->FurunoGP3X = m_cbFurunoGP3X->GetValue();
    m_pConnectionParams->InputSentenceList = wxStringTokenize( m_tcInputStc->GetValue() );
    if ( m_rbIAccept->GetValue() )
        m_pConnectionParams->InputSentenceListType = WHITELIST;
    else
        m_pConnectionParams->InputSentenceListType = BLACKLIST;
    m_pConnectionParams->Output = m_cbOutput->GetValue();
    m_pConnectionParams->OutputSentenceList = wxStringTokenize( m_tcOutputStc->GetValue() );
    if ( m_rbOAccept->GetValue() )
        m_pConnectionParams->OutputSentenceListType = WHITELIST;
    else
        m_pConnectionParams->OutputSentenceListType = BLACKLIST;
    m_pConnectionParams->Port = m_comboPort->GetValue();
    m_pConnectionParams->Protocol = PROTO_NMEA0183;
    return m_pConnectionParams;
}

void options::OnApplyClick( wxCommandEvent& event )
{
    ::wxBeginBusyCursor();

    m_returnChanges = 0;

    // Start with the stuff that requires intelligent validation.

    if( m_pShipIconType->GetSelection() > 0 ) {
        double n_ownship_length_meters;
        double n_ownship_beam_meters;
        double n_gps_antenna_offset_y;
        double n_gps_antenna_offset_x;
        long n_ownship_min_mm;
        m_pOSLength->GetValue().ToDouble( &n_ownship_length_meters );
        m_pOSWidth->GetValue().ToDouble( &n_ownship_beam_meters );
        m_pOSGPSOffsetX->GetValue().ToDouble( &n_gps_antenna_offset_x );
        m_pOSGPSOffsetY->GetValue().ToDouble( &n_gps_antenna_offset_y );
        m_pOSMinSize->GetValue().ToLong( &n_ownship_min_mm );
        wxString msg;
        if( n_ownship_length_meters <= 0 )
            msg += _("\n - your ship's length must be > 0");
        if( n_ownship_beam_meters <= 0 )
            msg += _("\n - your ship's beam must be > 0");
        if( fabs(n_gps_antenna_offset_x) > n_ownship_beam_meters/2.0 )
            msg += _("\n - your GPS offset from midship must be within your ship's beam");
        if( n_gps_antenna_offset_y < 0 || n_gps_antenna_offset_y > n_ownship_length_meters )
            msg += _("\n - your GPS offset from bow must be within your ship's length");
        if( n_ownship_min_mm <= 0 || n_ownship_min_mm > 100 )
            msg += _("\n - your minimum ship icon size must be between 1 and 100 mm");
        if( ! msg.IsEmpty() ) {
            msg.Prepend( _("The settings for own ship real size are not correct:") );
            OCPNMessageDialog* dlg = new OCPNMessageDialog( this,
                    msg, _("OpenCPN info"),
                    wxICON_ERROR );
            ::wxEndBusyCursor();
            dlg->ShowModal();
            delete dlg;
            event.SetInt( wxID_STOP );
            return;
        }
        g_n_ownship_length_meters = n_ownship_length_meters;
        g_n_ownship_beam_meters = n_ownship_beam_meters;
        g_n_gps_antenna_offset_y = n_gps_antenna_offset_y;
        g_n_gps_antenna_offset_x = n_gps_antenna_offset_x;
        g_n_ownship_min_mm = n_ownship_min_mm;
    }
    g_OwnShipIconType = m_pShipIconType->GetSelection();

    //    Handle Chart Tab
    wxString dirname;

    if( pActiveChartsList ) {
        UpdateWorkArrayFromTextCtl();
    } else {
        m_pWorkDirList->Clear();
        int nDir = m_CurrentDirList.GetCount();

        for( int i = 0; i < nDir; i++ ) {
            ChartDirInfo cdi = m_CurrentDirList.Item( i );
            m_pWorkDirList->Add( cdi );
        }
    }

    int k_force = FORCE_UPDATE;
    if( pUpdateCheckBox ) {
        if( !pUpdateCheckBox->GetValue() ) k_force = 0;
        pUpdateCheckBox->Enable();
        pUpdateCheckBox->SetValue( false );
    } else {
        k_force = 0;
    }

    m_returnChanges |= k_force;

    int k_scan = SCAN_UPDATE;
    if( pScanCheckBox ) {
        if( !pScanCheckBox->GetValue() ) k_scan = 0;
        pScanCheckBox->Enable();
        pScanCheckBox->SetValue( false );
    } else {
        k_scan = 0;
    }

    m_returnChanges |= k_scan;

    // Chart Groups

    if( groupsPanel->modified ) {
        groupsPanel->EmptyChartGroupArray( g_pGroupArray );
        delete g_pGroupArray;
        g_pGroupArray = groupsPanel->CloneChartGroupArray( m_pGroupArray );
        m_returnChanges |= GROUPS_CHANGED;
    }

    // Handle Settings Tab

    if( m_pConfig ) m_pConfig->m_bShowDebugWindows = pSettingsCB1->GetValue();

//TODO    g_bGarminHost = pGarminHost->GetValue();

    g_bShowOutlines = pCDOOutlines->GetValue();
    g_bDisplayGrid = pSDisplayGrid->GetValue();

    g_bQuiltEnable = pCDOQuilting->GetValue();
    g_bFullScreenQuilt = !pFullScreenQuilt->GetValue();

    g_bShowDepthUnits = pSDepthUnits->GetValue();
    g_bskew_comp = pSkewComp->GetValue();
    bool temp_bopengl = pOpenGL->GetValue();
    g_bsmoothpanzoom = pSmoothPanZoom->GetValue();

    g_bfilter_cogsog = m_cbFilterSogCog->GetValue();

    long filter_val = 1;
    m_tFilterSec->GetValue().ToLong( &filter_val );
    g_COGFilterSec = wxMin((int)filter_val, MAX_COGSOG_FILTER_SECONDS);
    g_COGFilterSec = wxMax(g_COGFilterSec, 1);
    g_SOGFilterSec = g_COGFilterSec;

    long update_val = 1;
    pCOGUPUpdateSecs->GetValue().ToLong( &update_val );
    g_COGAvgSec = wxMin((int)update_val, MAX_COG_AVERAGE_SECONDS);

    g_bCourseUp = pCBCourseUp->GetValue();
    g_bLookAhead = pCBLookAhead->GetValue();

    m_pText_OSCOG_Predictor->GetValue().ToDouble( &g_ownship_predictor_minutes );

    g_iNavAidRadarRingsNumberVisible = pNavAidRadarRingsNumberVisible->GetSelection();
    g_fNavAidRadarRingsStep = atof( pNavAidRadarRingsStep->GetValue().mb_str() );
    g_pNavAidRadarRingsStepUnits = m_itemRadarRingsUnits->GetSelection();
    g_bWayPointPreventDragging = pWayPointPreventDragging->GetValue();
    g_bConfirmObjectDelete = pConfirmObjectDeletion->GetValue();
    
    g_bPreserveScaleOnX = pPreserveScale->GetValue();

    g_bPlayShipsBells = pPlayShipsBells->GetValue();
    g_bFullscreenToolbar = pFullScreenToolbar->GetValue();
    g_bTransparentToolbar = pTransparentToolbar->GetValue();
    g_iSDMMFormat = pSDMMFormat->GetSelection();

    g_nTrackPrecision = pTrackPrecision->GetSelection();

    g_bTrackDaily = pTrackDaily->GetValue();
    g_bHighliteTracks = pTrackHighlite->GetValue();

    g_bEnableZoomToCursor = pEnableZoomToCursor->GetValue();

    //    AIS Parameters
    //      CPA Box
    g_bCPAMax = m_pCheck_CPA_Max->GetValue();
    m_pText_CPA_Max->GetValue().ToDouble( &g_CPAMax_NM );
    g_bCPAWarn = m_pCheck_CPA_Warn->GetValue();
    m_pText_CPA_Warn->GetValue().ToDouble( &g_CPAWarn_NM );
    g_bTCPA_Max = m_pCheck_CPA_WarnT->GetValue();
    m_pText_CPA_WarnT->GetValue().ToDouble( &g_TCPA_Max );

    //      Lost Targets
    g_bMarkLost = m_pCheck_Mark_Lost->GetValue();
    m_pText_Mark_Lost->GetValue().ToDouble( &g_MarkLost_Mins );
    g_bRemoveLost = m_pCheck_Remove_Lost->GetValue();
    m_pText_Remove_Lost->GetValue().ToDouble( &g_RemoveLost_Mins );

    //      Display
    g_bShowCOG = m_pCheck_Show_COG->GetValue();
    m_pText_COG_Predictor->GetValue().ToDouble( &g_ShowCOG_Mins );

    g_bAISShowTracks = m_pCheck_Show_Tracks->GetValue();
    m_pText_Track_Length->GetValue().ToDouble( &g_AISShowTracks_Mins );

    g_bShowMoored = !m_pCheck_Show_Moored->GetValue();
    m_pText_Moored_Speed->GetValue().ToDouble( &g_ShowMoored_Kts );

    g_bShowAreaNotices = m_pCheck_Show_Area_Notices->GetValue();
    g_bDrawAISSize = m_pCheck_Draw_Target_Size->GetValue();
    
    //      Alert
    g_bAIS_CPA_Alert = m_pCheck_AlertDialog->GetValue();
    g_bAIS_CPA_Alert_Audio = m_pCheck_AlertAudio->GetValue();
    g_bAIS_CPA_Alert_Suppress_Moored = m_pCheck_Alert_Moored->GetValue();

    g_bAIS_ACK_Timeout = m_pCheck_Ack_Timout->GetValue();
    m_pText_ACK_Timeout->GetValue().ToDouble( &g_AckTimeout_Mins );

    // Rollover
    g_bAISRolloverShowClass = m_pCheck_Rollover_Class->GetValue();
    g_bAISRolloverShowCOG = m_pCheck_Rollover_COG->GetValue();
    g_bAISRolloverShowCPA = m_pCheck_Rollover_CPA->GetValue();

    // NMEA Source
    long itemIndex = m_lcSources->GetNextItem( -1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );

    if(!connectionsaved)
    {
        ConnectionParams * cp = SaveConnectionParams();
        if(cp != NULL)
        {
            if (itemIndex >= 0)
            {
                g_pConnectionParams->RemoveAt(itemIndex);
                g_pConnectionParams->Insert(cp, itemIndex);
            }
            else
            {
                g_pConnectionParams->Add(cp);
                itemIndex = g_pConnectionParams->Count() - 1;
            }
            FillSourceList();
            m_lcSources->SetItemState(itemIndex, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
            m_lcSources->Refresh();
            connectionsaved = true;
        }
    }

    //Recreate all the datasource connections
    g_pMUX->ClearStreams();
    for ( size_t i = 0; i < g_pConnectionParams->Count(); i++ )
    {
        ConnectionParams *cp = g_pConnectionParams->Item(i);
        dsPortType port_type;
        if (cp->Output)
            port_type = DS_TYPE_INPUT_OUTPUT;
        else
            port_type = DS_TYPE_INPUT;
        DataStream *dstr = new DataStream( g_pMUX, cp->GetDSPort(), wxString::Format(wxT("%i"), cp->Baudrate), port_type, cp->Priority );
        dstr->SetInputFilter(cp->InputSentenceList);
        dstr->SetInputFilterType(cp->InputSentenceListType);
        dstr->SetOutputFilter(cp->OutputSentenceList);
        dstr->SetOutputFilterType(cp->OutputSentenceListType);
        dstr->SetChecksumCheck(cp->ChecksumCheck);
        g_pMUX->AddStream(dstr);
    }
#ifdef USE_S57
    //    Handle Vector Charts Tab

    g_cm93_zoom_factor = m_pSlider_CM93_Zoom->GetValue();

    int nOBJL = ps57CtlListBox->GetCount();

    for( int iPtr = 0; iPtr < nOBJL; iPtr++ ) {
        int itemIndex;
        for( size_t i=0; i<marinersStdXref.size(); i++ ) {
            if( marinersStdXref[ i ] == iPtr ) {
                itemIndex = i;
                break;
            }
        }
        OBJLElement *pOLE = (OBJLElement *) ( ps52plib->pOBJLArray->Item( itemIndex ) );
        pOLE->nViz = ps57CtlListBox->IsChecked( iPtr );
    }

    if( ps52plib ) {
        if( temp_bopengl != g_bopengl ) {
            //    We need to do this now to handle the screen refresh that
            //    is automatically generated on Windows at closure of the options dialog...
            ps52plib->FlushSymbolCaches();
            ps52plib->ClearCNSYLUPArray();      // some CNSY depends on renderer (e.g. CARC)
            ps52plib->GenerateStateHash();

            g_bopengl = temp_bopengl;
        }

        enum _DisCat nset = OTHER;
        switch( pDispCat->GetSelection() ){
            case 0:
                nset = DISPLAYBASE;
                break;
            case 1:
                nset = STANDARD;
                break;
            case 2:
                nset = OTHER;
                break;
            case 3:
                nset = MARINERS_STANDARD;
                break;
        }
        ps52plib->m_nDisplayCategory = nset;

        ps52plib->m_bShowSoundg = pCheck_SOUNDG->GetValue();
        ps52plib->m_bShowMeta = pCheck_META->GetValue();
        ps52plib->m_bShowS57ImportantTextOnly = pCheck_SHOWIMPTEXT->GetValue();
        ps52plib->m_bUseSCAMIN = pCheck_SCAMIN->GetValue();
        ps52plib->m_bShowAtonText = pCheck_ATONTEXT->GetValue();
        ps52plib->m_bShowLdisText = pCheck_LDISTEXT->GetValue();
        ps52plib->m_bExtendLightSectors = pCheck_XLSECTTEXT->GetValue();
        ps52plib->m_bDeClutterText = pCheck_DECLTEXT->GetValue();

        if( 0 == pPointStyle->GetSelection() ) ps52plib->m_nSymbolStyle = PAPER_CHART;
        else
            ps52plib->m_nSymbolStyle = SIMPLIFIED;

        if( 0 == pBoundStyle->GetSelection() ) ps52plib->m_nBoundaryStyle = PLAIN_BOUNDARIES;
        else
            ps52plib->m_nBoundaryStyle = SYMBOLIZED_BOUNDARIES;

        if( 0 == p24Color->GetSelection() ) S52_setMarinerParam( S52_MAR_TWO_SHADES, 1.0 );
        else
            S52_setMarinerParam( S52_MAR_TWO_SHADES, 0.0 );

        double dval;

        if( ( m_SafetyCtl->GetValue() ).ToDouble( &dval ) ) {
            S52_setMarinerParam( S52_MAR_SAFETY_DEPTH, dval );          // controls sounding display
            S52_setMarinerParam( S52_MAR_SAFETY_CONTOUR, dval );          // controls colour
        }

        if( ( m_ShallowCtl->GetValue() ).ToDouble( &dval ) ) S52_setMarinerParam(
                S52_MAR_SHALLOW_CONTOUR, dval );

        if( ( m_DeepCtl->GetValue() ).ToDouble( &dval ) ) S52_setMarinerParam( S52_MAR_DEEP_CONTOUR,
                dval );

        ps52plib->UpdateMarinerParams();

        ps52plib->m_nDepthUnitDisplay = pDepthUnitSelect->GetSelection();

        ps52plib->GenerateStateHash();
    }
#endif

//    User Interface Panel
    if( m_bVisitLang ) {
        wxString new_canon = _T("en_US");
        wxString lang_sel = m_itemLangListBox->GetValue();

        int nLang = sizeof( lang_list ) / sizeof(int);
        for( int it = 0; it < nLang; it++ ) {
            wxString lang_canonical = wxLocale::GetLanguageInfo( lang_list[it] )->CanonicalName;
            wxString test_string = GetOCPNKnownLanguage( lang_canonical, NULL );
            if( lang_sel == test_string ) {
                new_canon = lang_canonical;
                break;
            }
        }

        wxString locale_old = g_locale;
        g_locale = new_canon;

        if( g_locale != locale_old ) m_returnChanges |= LOCALE_CHANGED;

        wxString oldStyle = g_StyleManager->GetCurrentStyle()->name;
        g_StyleManager->SetStyleNextInvocation( m_itemStyleListBox->GetValue() );
        if( g_StyleManager->GetStyleNextInvocation() != oldStyle ) {
            m_returnChanges |= STYLE_CHANGED;
        }
        wxSizeEvent nullEvent;
        gFrame->OnSize( nullEvent );
    }

    //      PlugIn Manager Panel

    //      Pick up any changes to selections
    bool bnew_settings = g_pi_manager->UpdatePlugIns();
    if( bnew_settings ) m_returnChanges |= TOOLBAR_CHANGED;

    //      And keep config in sync
    g_pi_manager->UpdateConfig();

    //      PlugIns may have added panels
    if( g_pi_manager ) g_pi_manager->CloseAllPlugInPanels( (int) wxOK );

    //      Could be a lot smarter here
    m_returnChanges |= GENERIC_CHANGED;
    m_returnChanges |= k_vectorcharts;
    m_returnChanges |= k_charts;
    m_returnChanges |= m_groups_changed;
    m_returnChanges |= k_plugins;
    m_returnChanges |= k_tides;

    //  Pick up all the entries in the DataSelected control
    //  and update the global static array
    TideCurrentDataSet.Clear();
    int nEntry = tcDataSelected->GetCount();

    for( int i = 0; i < nEntry; i++ ) {
        wxString s = tcDataSelected->GetString( i );
        TideCurrentDataSet.Add( s );
    }

    if( event.GetId() == ID_APPLY ) {
        gFrame->ProcessOptionsDialog( m_returnChanges, this );
        cc1->ReloadVP();
    }

    k_charts = 0;
    ::wxEndBusyCursor();
}

void options::OnXidOkClick( wxCommandEvent& event )
{
    // When closing the form with Ctrl-Enter sometimes we get double events, the second is empty??
    if( event.GetEventObject() == NULL ) return;

    OnApplyClick( event );
    if( event.GetInt() == wxID_STOP ) return;

    //  Required to avoid intermittent crash on wxGTK
    m_pListbook->ChangeSelection(0);
    for (size_t i = 0; i < m_pListbook->GetPageCount(); i++)
    {
        wxNotebookPage* pg = m_pListbook->GetPage( i );
            
        if( pg->IsKindOf( CLASSINFO(wxNotebook))) {
                wxNotebook *nb = ((wxNotebook *)pg);
                nb->ChangeSelection(0);
        }
    }
                            
    delete pActiveChartsList;
    delete ps57CtlListBox;
    delete tcDataSelected;
    
    lastWindowPos = GetPosition();
    lastWindowSize = GetSize();
    EndModal( m_returnChanges );
}

void options::OnButtondeleteClick( wxCommandEvent& event )
{

    wxString dirname;

    wxArrayInt pListBoxSelections;
    pActiveChartsList->GetSelections( pListBoxSelections );
    int nSelections = pListBoxSelections.GetCount();
    for( int i = 0; i < nSelections; i++ ) {
        pActiveChartsList->Delete( pListBoxSelections.Item( ( nSelections - i ) - 1 ) );
    }

    UpdateWorkArrayFromTextCtl();

    if( m_pWorkDirList ) {
        pActiveChartsList->Clear();

        int nDir = m_pWorkDirList->GetCount();
        for( int id = 0; id < nDir; id++ ) {
            dirname = m_pWorkDirList->Item( id ).fullpath;
            if( !dirname.IsEmpty() ) {
                pActiveChartsList->Append( dirname );
            }
        }
    }

    k_charts |= CHANGE_CHARTS;

    pScanCheckBox->Disable();

    event.Skip();
}

void options::OnDebugcheckbox1Click( wxCommandEvent& event )
{
    event.Skip();
}

void options::OnCancelClick( wxCommandEvent& event )
{
    //  Required to avoid intermittent crash on wxGTK
    m_pListbook->ChangeSelection(0);
    delete pActiveChartsList;
    delete ps57CtlListBox;

    lastWindowPos = GetPosition();
    lastWindowSize = GetSize();
    EndModal(0);
}

void options::OnChooseFont( wxCommandEvent& event )
{
    wxString sel_text_element = m_itemFontElementListBox->GetStringSelection();

    wxFont *psfont;
    wxFontData font_data;

    wxFont *pif = pFontMgr->GetFont( sel_text_element );
    wxColour init_color = pFontMgr->GetFontColor( sel_text_element );

    wxFontData init_font_data;
    if( pif ) init_font_data.SetInitialFont( *pif );
    init_font_data.SetColour( init_color );

#ifdef __WXX11__
    X11FontPicker dg(pParent, init_font_data);
#else
    wxFontDialog dg( pParent, init_font_data );
#endif
    int retval = dg.ShowModal();
    if( wxID_CANCEL != retval ) {
        font_data = dg.GetFontData();
        wxFont font = font_data.GetChosenFont();
        psfont = new wxFont( font );
        wxColor color = font_data.GetColour();
        pFontMgr->SetFont( sel_text_element, psfont, color );

        pParent->UpdateAllFonts();
    }

    event.Skip();
}

void options::OnChartsPageChange( wxListbookEvent& event )
{
    unsigned int i = event.GetSelection();

    //    User selected Chart Groups Page?
    //    If so, build the remaining UI elements
    if( 2 == i ) {                       // 2 is the index of "Chart Groups" page
        if(!groupsPanel->m_UIcomplete)
            groupsPanel->CompletePanel();

        if(!groupsPanel->m_settingscomplete) {
            ::wxBeginBusyCursor();
            groupsPanel->CompleteInitialSettings();
            ::wxEndBusyCursor();
        }
    }

    event.Skip();               // Allow continued event processing
}


void options::OnPageChange( wxListbookEvent& event )
{
    unsigned int i = event.GetSelection();
    lastPage = i;

    //    User selected Chart Page?
    //    If so, build the "Charts" page variants
    if( 2 == i ) {                       // 2 is the index of "Charts" page
        k_charts = VISIT_CHARTS;
    }

    //    User selected Vector Chart Page?
    else if( 3 == i ) {                      // 3 is the index of "VectorCharts" page
        k_vectorcharts = S52_CHANGED;
    }

    else if( m_pageUI == i ) {                       // 5 is the index of "User Interface" page
        if( !m_bVisitLang ) {
            ::wxBeginBusyCursor();

            int current_language = plocale_def_lang->GetLanguage();
            wxString current_sel = wxLocale::GetLanguageName( current_language );

            current_sel = GetOCPNKnownLanguage( g_locale, NULL );

            int nLang = sizeof( lang_list ) / sizeof(int);

#ifdef __WXMSW__
            // always add us english
            m_itemLangListBox->Append( _T("English (U.S.)") );

            wxString lang_dir = g_SData_Locn + _T("share/locale/");
            for( int it = 1; it < nLang; it++ ) {
                if( wxLocale::IsAvailable( lang_list[it] ) ) {
                    wxLocale ltest( lang_list[it], 0 );
                    ltest.AddCatalog( _T("opencpn") );
                    if( !ltest.IsLoaded( _T("opencpn") ) ) continue;

                    // Defaults
                    wxString loc_lang_name = wxLocale::GetLanguageName( lang_list[it] );
                    wxString widgets_lang_name = loc_lang_name;
                    wxString lang_canonical =
                            wxLocale::GetLanguageInfo( lang_list[it] )->CanonicalName;

                    //  Make opencpn substitutions
                    wxString lang_suffix;
                    loc_lang_name = GetOCPNKnownLanguage( lang_canonical, &lang_suffix );

                    //  Look explicitely to see if .mo is available
                    wxString test_dir = lang_dir + lang_suffix;
                    if( !wxDir::Exists( test_dir ) ) continue;

                    m_itemLangListBox->Append( loc_lang_name );
                }
            }
#else
            wxArrayString lang_array;

            // always add us english
            lang_array.Add(_T("en_US"));

            for( int it = 0; it < nLang; it++)
            {
                {
                    wxLog::EnableLogging(false);  // avoid "Cannot set locale to..." log message

                    wxLocale ltest(lang_list[it], 0);
                    ltest.AddCatalog(_T("opencpn"));

                    wxLog::EnableLogging(true);

                    if(ltest.IsLoaded(_T("opencpn")))
                    {
                        wxString s0 = wxLocale::GetLanguageInfo(lang_list[it])->CanonicalName;
                        wxString sl = wxLocale::GetLanguageName(lang_list[it]);
                        if(wxNOT_FOUND == lang_array.Index(s0))
                        lang_array.Add(s0);

                    }
                }
            }

            for(unsigned int i=0; i < lang_array.GetCount(); i++)
            {
                //  Make opencpn substitutions
                wxString loc_lang_name = GetOCPNKnownLanguage(lang_array[i], NULL);
                m_itemLangListBox->Append( loc_lang_name );
            }
#endif

            // BUGBUG
            //  Remember that wxLocale ctor has the effect of changing the system locale, including the "C" libraries.
            //  It should then also happen that the locale should be switched back to ocpn initial load setting
            //  upon the dtor of the above wxLocale instantiations....
            //  wxWidgets may do so internally, but there seems to be no effect upon the system libraries, so that
            //  functions like strftime() do not revert to the correct locale setting.
            //  Also, the catalog for the application is not reloaded by the ctor, so we must reload them directly
            //  So as workaround, we reset the locale explicitely.

            delete plocale_def_lang;
            plocale_def_lang = new wxLocale( current_language );

            setlocale( LC_NUMERIC, "C" );
            plocale_def_lang->AddCatalog( _T("opencpn") );

            m_itemLangListBox->SetStringSelection( current_sel );

            //      Initialize Language tab
            const wxLanguageInfo *pli = wxLocale::FindLanguageInfo( g_locale );
            if( pli ) {
                wxString clang = pli->Description;
//                        m_itemLangListBox->SetValue(clang);
            }

            m_bVisitLang = true;

            ::wxEndBusyCursor();
        }
    }

    else if( m_pagePlugins == i ) {                    // 7 is the index of "Plugins" page
        k_plugins = TOOLBAR_CHANGED;
    }
}

//void options::OnNMEASourceChoice( wxCommandEvent& event )
//{
/*TODO
    int i = event.GetSelection();
    wxString src( m_itemNMEAListBox->GetString( i ) );
    if( ( src.Upper().Find( _T("GPSD") ) != wxNOT_FOUND )
            || ( src.Upper().Find( _T("LIBGPS") ) != wxNOT_FOUND ) ) {
        m_itemNMEA_TCPIP_StaticBox->Enable();
        m_itemNMEA_TCPIP_Source->Enable();

        m_itemNMEA_TCPIP_Source->Clear();
        m_itemNMEA_TCPIP_Source->WriteText( _T("localhost") ); // default

        wxString source;
        source = *pNMEADataSource;
        if( source.Upper().StartsWith( _T("GPSD") ) || source.Upper().StartsWith( _T("LIBGPS") ) ) {
            wxString ip;
            ip = source.AfterFirst( ':' );

            if( ip.Len() ) {
                m_itemNMEA_TCPIP_Source->Clear();
                m_itemNMEA_TCPIP_Source->WriteText( ip );
            }
        }
    } else {
        m_itemNMEA_TCPIP_StaticBox->Disable();
        m_itemNMEA_TCPIP_Source->Disable();
    }
*/
//}

void options::OnButtonSelectSound( wxCommandEvent& event )
{
    wxString sound_dir = g_SData_Locn;
    sound_dir.Append( _T("sounds") );

    wxFileDialog *openDialog = new wxFileDialog( this, _("Select Sound File"), sound_dir, wxT(""),
            _("WAV files (*.wav)|*.wav|All files (*.*)|*.*"), wxFD_OPEN );
    int response = openDialog->ShowModal();
    if( response == wxID_OK ) {
        g_sAIS_Alert_Sound_File = openDialog->GetPath();
    }

}

void options::OnButtonTestSound( wxCommandEvent& event )
{
#if wxUSE_LIBSDL
// printf("wxUSE_LIBSDL true\n");
#endif

    wxSound AIS_Sound( g_sAIS_Alert_Sound_File );

    if( AIS_Sound.IsOk() ) {
//            printf("playing sound\n");
        AIS_Sound.Play();
    }
//      else
//            printf("sound is NOT ok\n");

}

wxString GetOCPNKnownLanguage( wxString lang_canonical, wxString *lang_dir )
{
    wxString return_string;
    wxString dir_suffix;

    if( lang_canonical == _T("en_US") ) {
        dir_suffix = _T("en");
        return_string = wxString( "English (U.S.)", wxConvUTF8 );
    } else if( lang_canonical == _T("cs_CZ") ) {
        dir_suffix = _T("cs");
        return_string = wxString( "Čeština", wxConvUTF8 );
    } else if( lang_canonical == _T("da_DK") ) {
        dir_suffix = _T("da");
        return_string = wxString( "Dansk", wxConvUTF8 );
    } else if( lang_canonical == _T("de_DE") ) {
        dir_suffix = _T("de");
        return_string = wxString( "Deutsch", wxConvUTF8 );
    } else if( lang_canonical == _T("et_EE") ) {
        dir_suffix = _T("et");
        return_string = wxString( "Eesti", wxConvUTF8 );
    } else if( lang_canonical == _T("es_ES") ) {
        dir_suffix = _T("es");
        return_string = wxString( "Español", wxConvUTF8 );
    } else if( lang_canonical == _T("fr_FR") ) {
        dir_suffix = _T("fr");
        return_string = wxString( "Français", wxConvUTF8 );
    } else if( lang_canonical == _T("it_IT") ) {
        dir_suffix = _T("it");
        return_string = wxString( "Italiano", wxConvUTF8 );
    } else if( lang_canonical == _T("nl_NL") ) {
        dir_suffix = _T("nl");
        return_string = wxString( "Nederlands", wxConvUTF8 );
    } else if( lang_canonical == _T("pl_PL") ) {
        dir_suffix = _T("pl");
        return_string = wxString( "Polski", wxConvUTF8 );
    } else if( lang_canonical == _T("pt_PT") ) {
        dir_suffix = _T("pt_PT");
        return_string = wxString( "Português", wxConvUTF8 );
    } else if( lang_canonical == _T("pt_BR") ) {
        dir_suffix = _T("pt_BR");
        return_string = wxString( "Português Brasileiro", wxConvUTF8 );
    } else if( lang_canonical == _T("ru_RU") ) {
        dir_suffix = _T("ru");
        return_string = wxString( "Русский", wxConvUTF8 );
    } else if( lang_canonical == _T("sv_SE") ) {
        dir_suffix = _T("sv");
        return_string = wxString( "Svenska", wxConvUTF8 );
    } else if( lang_canonical == _T("fi_FI") ) {
        dir_suffix = _T("fi_FI");
        return_string = wxString( "Suomi", wxConvUTF8 );
    } else if( lang_canonical == _T("nb_NO") ) {
        dir_suffix = _T("nb_NO");
        return_string = wxString( "Norsk", wxConvUTF8 );
    } else if( lang_canonical == _T("tr_TR") ) {
        dir_suffix = _T("tr_TR");
        return_string = wxString( "Türkçe", wxConvUTF8 );
    } else if( lang_canonical == _T("el_GR") ) {
        dir_suffix = _T("el_GR");
        return_string = wxString( "Ελληνικά", wxConvUTF8 );
    } else if( lang_canonical == _T("hu_HU") ) {
        dir_suffix = _T("hu_HU");
        return_string = wxString( "Magyar", wxConvUTF8 );
    } else if( lang_canonical == _T("zh_TW") ) {
        dir_suffix = _T("zh_TW");
        return_string = wxString( "正體字", wxConvUTF8 );
    } else if( lang_canonical == _T("ca_ES") ) {
        dir_suffix = _T("ca_ES");
        return_string = wxString( "Catalan", wxConvUTF8 );
    } else if( lang_canonical == _T("gl_ES") ) {
        dir_suffix = _T("gl_ES");
        return_string = wxString( "Galician", wxConvUTF8 );
    } else {
        dir_suffix = lang_canonical;
        const wxLanguageInfo *info = wxLocale::FindLanguageInfo( lang_canonical );
        return_string = info->Description;
    }

    if( NULL != lang_dir ) *lang_dir = dir_suffix;

    return return_string;

}

ChartGroupArray* ChartGroupsUI::CloneChartGroupArray( ChartGroupArray* s )
{
    ChartGroupArray *d = new ChartGroupArray;
    for( unsigned int i = 0; i < s->GetCount(); i++ ) {
        ChartGroup *psg = s->Item( i );
        ChartGroup *pdg = new ChartGroup;
        pdg->m_group_name = psg->m_group_name;

        for( unsigned int j = 0; j < psg->m_element_array.GetCount(); j++ ) {
            ChartGroupElement *pde = new ChartGroupElement;
            pde->m_element_name = psg->m_element_array.Item( j )->m_element_name;
            for( unsigned int k = 0;
                    k < psg->m_element_array.Item( j )->m_missing_name_array.GetCount(); k++ ) {
                wxString missing_name = psg->m_element_array.Item( j )->m_missing_name_array.Item(k);
                pde->m_missing_name_array.Add( missing_name );
            }
            pdg->m_element_array.Add( pde );
        }
        d->Add( pdg );
    }
    return d;
}

void ChartGroupsUI::EmptyChartGroupArray( ChartGroupArray* s )
{
    if( !s ) return;
    for( unsigned int i = 0; i < s->GetCount(); i++ ) {
        ChartGroup *psg = s->Item( i );

        for( unsigned int j = 0; j < psg->m_element_array.GetCount(); j++ ) {
            ChartGroupElement *pe = psg->m_element_array.Item( j );
            pe->m_missing_name_array.Clear();
            psg->m_element_array.RemoveAt( j );
            delete pe;

        }
        s->RemoveAt( i );
        delete psg;
    }

    s->Clear();
}

//    Chart Groups dialog implementation

BEGIN_EVENT_TABLE( ChartGroupsUI, wxScrolledWindow )
    EVT_TREE_ITEM_EXPANDED( wxID_TREECTRL, ChartGroupsUI::OnNodeExpanded )
    EVT_BUTTON( ID_GROUPINSERTDIR, ChartGroupsUI::OnInsertChartItem )
    EVT_BUTTON( ID_GROUPREMOVEDIR, ChartGroupsUI::OnRemoveChartItem )
    EVT_NOTEBOOK_PAGE_CHANGED(ID_GROUPNOTEBOOK, ChartGroupsUI::OnGroupPageChange)
    EVT_BUTTON( ID_GROUPNEWGROUP, ChartGroupsUI::OnNewGroup )
    EVT_BUTTON( ID_GROUPDELETEGROUP, ChartGroupsUI::OnDeleteGroup )
END_EVENT_TABLE()

ChartGroupsUI::ChartGroupsUI( wxWindow* parent )
{
    Create( parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL, _("Chart Groups") );

    m_GroupSelectedPage = -1;
    m_pActiveChartsTree = 0;
    pParent = parent;
    lastSelectedCtl = NULL;
    allAvailableCtl = NULL;
    defaultAllCtl = NULL;
    iFont = NULL;
    m_pAddButton = NULL;
    m_pRemoveButton = NULL;
    m_pDeleteGroupButton = NULL;
    m_pNewGroupButton = NULL;
    m_pGroupArray= NULL;
    m_GroupNB = NULL;
    modified = false;
    m_UIcomplete = false;
}

ChartGroupsUI::~ChartGroupsUI()
{
    m_DirCtrlArray.Clear();
}

void ChartGroupsUI::SetInitialSettings()
{
    m_settingscomplete = false;
}

void ChartGroupsUI::CompleteInitialSettings()
{
    //    Fill in the "Active chart" tree control
    //    from the options dialog "Active Chart Directories" list
    wxArrayString dir_array;
    int nDir = m_db_dirs.GetCount();
    for( int i = 0; i < nDir; i++ ) {
        wxString dirname = m_db_dirs.Item( i ).fullpath;
        if( !dirname.IsEmpty() ) dir_array.Add( dirname );
    }


    PopulateTreeCtrl( allAvailableCtl->GetTreeCtrl(), dir_array, wxColour( 0, 0, 0 ) );
    m_pActiveChartsTree = allAvailableCtl->GetTreeCtrl();


    //    Fill in the Page 0 tree control
    //    from the options dialog "Active Chart Directories" list
    wxArrayString dir_array0;
    int nDir0 = m_db_dirs.GetCount();
    for( int i = 0; i < nDir0; i++ ) {
        wxString dirname = m_db_dirs.Item( i ).fullpath;
        if( !dirname.IsEmpty() ) dir_array0.Add( dirname );
    }
    PopulateTreeCtrl( defaultAllCtl->GetTreeCtrl(), dir_array0, wxColour( 128, 128, 128 ),
                      iFont );

    BuildNotebookPages( m_pGroupArray );

    groupsSizer->Layout();

    m_settingscomplete = true;

}



void ChartGroupsUI::PopulateTreeCtrl( wxTreeCtrl *ptc, const wxArrayString &dir_array,
        const wxColour &col, wxFont *pFont )
{
    ptc->DeleteAllItems();

    wxDirItemData* rootData = new wxDirItemData( _T("Dummy"), _T("Dummy"), true );
    wxString rootName;
    rootName = _T("Dummy");
    wxTreeItemId m_rootId = ptc->AddRoot( rootName, 3, -1, rootData );
    ptc->SetItemHasChildren( m_rootId );

    wxString dirname;
    int nDir = dir_array.GetCount();
    for( int i = 0; i < nDir; i++ ) {
        wxString dirname = dir_array.Item( i );
        if( !dirname.IsEmpty() ) {
            wxDirItemData *dir_item = new wxDirItemData( dirname, dirname, true );
            wxTreeItemId id = ptc->AppendItem( m_rootId, dirname, 0, -1, dir_item );
            if( pFont ) ptc->SetItemFont( id, *pFont );
            ptc->SetItemTextColour( id, col );
            ptc->SetItemHasChildren( id );
        }
    }
}

void ChartGroupsUI::OnInsertChartItem( wxCommandEvent &event )
{
    wxString insert_candidate = allAvailableCtl->GetPath();
    if( !insert_candidate.IsEmpty() ) {
        if( m_DirCtrlArray.GetCount() ) {
            wxGenericDirCtrl *pDirCtrl = ( m_DirCtrlArray.Item( m_GroupSelectedPage ) );
            ChartGroup *pGroup = m_pGroupArray->Item( m_GroupSelectedPage - 1 );
            if( pDirCtrl ) {
                wxTreeCtrl *ptree = pDirCtrl->GetTreeCtrl();
                if( ptree ) {
                    if( ptree->IsEmpty() ) {
                        wxDirItemData* rootData = new wxDirItemData( wxEmptyString, wxEmptyString,
                                true );
                        wxString rootName = _T("Dummy");
                        wxTreeItemId rootId = ptree->AddRoot( rootName, 3, -1, rootData );

                        ptree->SetItemHasChildren( rootId );
                    }

                    wxTreeItemId root_Id = ptree->GetRootItem();
                    wxDirItemData *dir_item = new wxDirItemData( insert_candidate, insert_candidate,
                            true );
                    wxTreeItemId id = ptree->AppendItem( root_Id, insert_candidate, 0, -1,
                            dir_item );
                    if( wxDir::Exists( insert_candidate ) ) ptree->SetItemHasChildren( id );
                }

                ChartGroupElement *pnew_element = new ChartGroupElement;
                pnew_element->m_element_name = insert_candidate;
                pGroup->m_element_array.Add( pnew_element );
            }
        }
    }
    modified = true;
    allAvailableCtl->GetTreeCtrl()->UnselectAll();
    m_pAddButton->Disable();
}

void ChartGroupsUI::OnRemoveChartItem( wxCommandEvent &event )
{
    if( m_DirCtrlArray.GetCount() ) {
        wxGenericDirCtrl *pDirCtrl = ( m_DirCtrlArray.Item( m_GroupSelectedPage ) );
        ChartGroup *pGroup = m_pGroupArray->Item( m_GroupSelectedPage - 1 );

        if( pDirCtrl ) {
            wxString sel_item = pDirCtrl->GetPath();

            wxTreeCtrl *ptree = pDirCtrl->GetTreeCtrl();
            if( ptree && ptree->GetCount() ) {
                wxTreeItemId id = ptree->GetSelection();
                lastDeletedItem = id;
                if( id.IsOk() ) {
                    wxString branch_adder;
                    int group_item_index = FindGroupBranch( pGroup, ptree, id, &branch_adder );
                    if( group_item_index >= 0 ) {
                        ChartGroupElement *pelement = pGroup->m_element_array.Item(
                                group_item_index );
                        bool b_duplicate = false;
                        for( unsigned int k = 0; k < pelement->m_missing_name_array.GetCount();
                                k++ ) {
                            if( pelement->m_missing_name_array.Item( k ) == sel_item ) {
                                b_duplicate = true;
                                break;
                            }
                        }
                        if( !b_duplicate ) {
                            pelement->m_missing_name_array.Add( sel_item );
                        }

                        //    Special case...
                        //    If the selection is a branch itself,
                        //    Then delete from the tree, and delete from the group
                        if( branch_adder == _T("") ) {
                            ptree->Delete( id );
                            pGroup->m_element_array.RemoveAt( group_item_index );
                        } else {
                            ptree->SetItemTextColour( id, wxColour( 128, 128, 128 ) );
                            //   what about toggle back?
                        }
                    }
                }
                modified = true;
                lastSelectedCtl->Unselect();
                m_pRemoveButton->Disable();
                wxLogMessage( _T("Disable"));
            }
        }
    }
    event.Skip();
}

void ChartGroupsUI::OnGroupPageChange( wxNotebookEvent& event )
{
    m_GroupSelectedPage = event.GetSelection();
    allAvailableCtl->GetTreeCtrl()->UnselectAll();
    if( lastSelectedCtl ) lastSelectedCtl->UnselectAll();
    m_pRemoveButton->Disable();
    m_pAddButton->Disable();
}

void ChartGroupsUI::OnAvailableSelection( wxTreeEvent& event ) {
    if( allAvailableCtl && (event.GetEventObject() == allAvailableCtl->GetTreeCtrl()) ){
        wxTreeItemId item = allAvailableCtl->GetTreeCtrl()->GetSelection();
        if( item && item.IsOk() && m_GroupSelectedPage>0 ) {
            m_pAddButton->Enable();
        } else {
            m_pAddButton->Disable();
        }
    } else {
        lastSelectedCtl = (wxTreeCtrl*) event.GetEventObject();
        wxTreeItemId item = lastSelectedCtl->GetSelection();
        if( item && item.IsOk() && m_GroupSelectedPage>0 ) {
            // We need a trick for wxGTK here, since it gives us a Selection event with
            // the just deleted empty element after OnRemoveChartItem()
            wxString itemPath = ((wxGenericDirCtrl*) lastSelectedCtl->GetParent())->GetPath();
            if( itemPath.Length() ) m_pRemoveButton->Enable();
        } else {
            m_pRemoveButton->Disable();
        }
    }
    event.Skip();
}

void ChartGroupsUI::OnNewGroup( wxCommandEvent &event )
{
    wxTextEntryDialog *pd = new wxTextEntryDialog( this, _("Enter Group Name"),
            _("New Chart Group") );

    if( pd->ShowModal() == wxID_OK ) {
        AddEmptyGroupPage( pd->GetValue() );
        ChartGroup *pGroup = new ChartGroup;
        pGroup->m_group_name = pd->GetValue();
        if( m_pGroupArray ) m_pGroupArray->Add( pGroup );

        m_GroupSelectedPage = m_GroupNB->GetPageCount() - 1;      // select the new page
        m_GroupNB->ChangeSelection( m_GroupSelectedPage );
        modified = true;
    }
    delete pd;
}

void ChartGroupsUI::OnDeleteGroup( wxCommandEvent &event )
{
    if( 0 != m_GroupSelectedPage ) {
        m_DirCtrlArray.RemoveAt( m_GroupSelectedPage );
        if( m_pGroupArray ) m_pGroupArray->RemoveAt( m_GroupSelectedPage - 1 );
        m_GroupNB->DeletePage( m_GroupSelectedPage );
        modified = true;
    }
}

WX_DEFINE_OBJARRAY( ChartGroupElementArray );
WX_DEFINE_OBJARRAY( ChartGroupArray );

int ChartGroupsUI::FindGroupBranch( ChartGroup *pGroup, wxTreeCtrl *ptree, wxTreeItemId item,
        wxString *pbranch_adder )
{
    wxString branch_name;
    wxString branch_adder;

    wxTreeItemId current_node = item;
    while( current_node.IsOk() ) {

        wxTreeItemId parent_node = ptree->GetItemParent( current_node );
        if( !parent_node ) break;

        if( parent_node == ptree->GetRootItem() ) {
            branch_name = ptree->GetItemText( current_node );
            break;
        }

        branch_adder.Prepend( ptree->GetItemText( current_node ) );
        branch_adder.Prepend( wxString( wxFILE_SEP_PATH ) );

        current_node = ptree->GetItemParent( current_node );
    }

    //    Find the index and element pointer of the target branch in the Group
    ChartGroupElement *target_element = NULL;
    unsigned int target_item_index = -1;

    for( unsigned int i = 0; i < pGroup->m_element_array.GetCount(); i++ ) {
        wxString target = pGroup->m_element_array.Item( i )->m_element_name;
        if( branch_name == target ) {
            target_element = pGroup->m_element_array.Item( i );
            target_item_index = i;
            break;
        }
    }

    if( pbranch_adder ) *pbranch_adder = branch_adder;

    return target_item_index;
}

void ChartGroupsUI::OnNodeExpanded( wxTreeEvent& event )
{
    wxTreeItemId node = event.GetItem();

    if( m_GroupSelectedPage > 0 ) {
        wxGenericDirCtrl *pDirCtrl = ( m_DirCtrlArray.Item( m_GroupSelectedPage ) );
        ChartGroup *pGroup = m_pGroupArray->Item( m_GroupSelectedPage - 1 );
        if( pDirCtrl ) {
            wxTreeCtrl *ptree = pDirCtrl->GetTreeCtrl();

            wxString branch_adder;
            int target_item_index = FindGroupBranch( pGroup, ptree, node, &branch_adder );
            if( target_item_index >= 0 ) {
                ChartGroupElement *target_element = pGroup->m_element_array.Item(
                        target_item_index );
                wxString branch_name = target_element->m_element_name;

                //    Walk the children of the expanded node, marking any items which appear in
                //    the "missing" list
                if( ( target_element->m_missing_name_array.GetCount() ) ) {
                    wxString full_root = branch_name;
                    full_root += branch_adder;
                    full_root += wxString( wxFILE_SEP_PATH );

                    wxTreeItemIdValue cookie;
                    wxTreeItemId child = ptree->GetFirstChild( node, cookie );
                    while( child.IsOk() ) {
                        wxString target_string = full_root;
                        target_string += ptree->GetItemText( child );

                        for( unsigned int k = 0;
                                k < target_element->m_missing_name_array.GetCount(); k++ ) {
                            if( target_element->m_missing_name_array.Item( k ) == target_string ) {
                                ptree->SetItemTextColour( child, wxColour( 128, 128, 128 ) );
                                break;
                            }
                        }
                        child = ptree->GetNextChild( node, cookie );
                    }
                }
            }
        }
    }
}

void ChartGroupsUI::BuildNotebookPages( ChartGroupArray *pGroupArray )
{
    for( unsigned int i = 0; i < pGroupArray->GetCount(); i++ ) {
        ChartGroup *pGroup = pGroupArray->Item( i );
        wxTreeCtrl *ptc = AddEmptyGroupPage( pGroup->m_group_name );

        wxString itemname;
        int nItems = pGroup->m_element_array.GetCount();
        for( int i = 0; i < nItems; i++ ) {
            wxString itemname = pGroup->m_element_array.Item( i )->m_element_name;
            if( !itemname.IsEmpty() ) {
                wxDirItemData *dir_item = new wxDirItemData( itemname, itemname, true );
                wxTreeItemId id = ptc->AppendItem( ptc->GetRootItem(), itemname, 0, -1, dir_item );

                if( wxDir::Exists( itemname ) ) ptc->SetItemHasChildren( id );
            }
        }
    }
}

wxTreeCtrl *ChartGroupsUI::AddEmptyGroupPage( const wxString& label )
{
    wxGenericDirCtrl *GroupDirCtl = new wxGenericDirCtrl( m_GroupNB, wxID_ANY, _T("TESTDIR") );
    m_GroupNB->AddPage( GroupDirCtl, label );

    wxTreeCtrl *ptree = GroupDirCtl->GetTreeCtrl();
    ptree->DeleteAllItems();

    wxDirItemData* rootData = new wxDirItemData( wxEmptyString, wxEmptyString, true );
    wxString rootName = _T("Dummy");
    wxTreeItemId rootId = ptree->AddRoot( rootName, 3, -1, rootData );
    ptree->SetItemHasChildren( rootId );

    m_DirCtrlArray.Add( GroupDirCtl );

    return ptree;
}

void options::OnInsertTideDataLocation( wxCommandEvent &event )
{
    wxString sel_file;
    int response = wxID_CANCEL;

    wxFileDialog openDialog( this, _( "Select Tide/Current Data" ), g_TCData_Dir, wxT ( "" ),
                             wxT ( "Tide/Current Data files (*.IDX; *.TCD)|*.IDX;*.idx;*.TCD;*.tcd|All files (*.*)|*.*" ),
                                    wxFD_OPEN  );
    response = openDialog.ShowModal();
    if( response == wxID_OK ) {
        sel_file = openDialog.GetPath();

        if( g_bportable ) {
            wxFileName f( sel_file );
            f.MakeRelativeTo( *pHome_Locn );
            tcDataSelected->Append( f.GetFullPath() );
        } else
            tcDataSelected->Append( sel_file );

        //    Record the currently selected directory for later use
        wxFileName fn( sel_file );
        g_TCData_Dir = fn.GetPath();
    }
}

void options::OnRemoveTideDataLocation( wxCommandEvent &event )
{
    wxArrayInt sels;
    int nSel = tcDataSelected->GetSelections(sels);
    wxArrayString a;
    for( int i=0 ; i < nSel ; i++) {
        a.Add( tcDataSelected->GetString(sels.Item( i )));
    }

    for (unsigned int i=0 ; i < a.Count() ; i++) {
        int b = tcDataSelected->FindString(a.Item(i));
        tcDataSelected->Delete( b );
    }
}

void options::OnValChange( wxCommandEvent& event )
{
    event.Skip();
}

void options::OnConnValChange( wxCommandEvent& event )
{
    connectionsaved = false;
    event.Skip();
}

void options::OnTypeSerialSelected( wxCommandEvent& event )
{
    OnConnValChange(event);
    SetNMEAFormToSerial();
}

void options::OnTypeNetSelected( wxCommandEvent& event )
{
    OnConnValChange(event);
    SetNMEAFormToNet();
}

void options::OnUploadFormatChange( wxCommandEvent& event )
{
    if ( event.GetEventObject() == m_cbGarminHost && event.IsChecked() )
        m_cbFurunoGP3X->SetValue(false);
    else if ( event.GetEventObject() == m_cbFurunoGP3X && event.IsChecked() )
        m_cbGarminHost->SetValue(false);
    event.Skip();
}

void options::ShowNMEACommon(bool visible)
{
    if ( visible )
    {
        m_rbTypeSerial->Show();
        m_rbTypeNet->Show();
        m_rbIAccept->Show();
        m_rbIIgnore->Show();
        m_rbOAccept->Show();
        m_rbOIgnore->Show();
        m_tcInputStc->Show();
        m_btnInputStcList->Show();
        m_tcOutputStc->Show();
        m_btnOutputStcList->Show();
        m_cbOutput->Show();
        m_choicePriority->Show();
        m_stPriority->Show();
        m_cbCheckCRC->Show();
        m_cbFurunoGP3X->Show();
    }
    else
    {
        m_rbTypeSerial->Hide();
        m_rbTypeNet->Hide();
        m_rbIAccept->Hide();
        m_rbIIgnore->Hide();
        m_rbOAccept->Hide();
        m_rbOIgnore->Hide();
        m_tcInputStc->Hide();
        m_btnInputStcList->Hide();
        m_tcOutputStc->Hide();
        m_btnOutputStcList->Hide();
        m_cbOutput->Hide();
        m_choicePriority->Hide();
        m_stPriority->Hide();
        m_cbCheckCRC->Hide();
        m_cbFurunoGP3X->Hide();
        sbSizerOutFilter->SetDimension(0,0,0,0);
        sbSizerInFilter->SetDimension(0,0,0,0);
        sbSizerConnectionProps->SetDimension(0,0,0,0);
    }
}

void options::ShowNMEANet(bool visible)
{
    if ( visible )
    {
        m_stNetAddr->Show();
        m_tNetAddress->Show();
        m_stNetPort->Show();
        m_tNetPort->Show();
        m_stNetProto->Show();
        m_rbNetProtoGPSD->Show();
        m_rbNetProtoTCP->Show();
        m_rbNetProtoUDP->Show();
    }
    else
    {
        m_stNetAddr->Hide();
        m_tNetAddress->Hide();
        m_stNetPort->Hide();
        m_tNetPort->Hide();
        m_stNetProto->Hide();
        m_rbNetProtoGPSD->Hide();
        m_rbNetProtoTCP->Hide();
        m_rbNetProtoUDP->Hide();
    }
}

void options::ShowNMEASerial(bool visible)
{
    if ( visible )
    {
        m_stSerBaudrate->Show();
        m_choiceBaudRate->Show();
        m_stSerPort->Show();
        m_comboPort->Show();
        m_stSerProtocol->Show();
        m_choiceSerialProtocol->Show();
        m_cbGarminHost->Show();
        gSizerNetProps->SetDimension(0,0,0,0);
    }
    else
    {
        m_stSerBaudrate->Hide();
        m_choiceBaudRate->Hide();
        m_stSerPort->Hide();
        m_comboPort->Hide();
        m_stSerProtocol->Hide();
        m_choiceSerialProtocol->Hide();
        m_cbGarminHost->Hide();
        gSizerSerProps->SetDimension(0,0,0,0);
    }
}

void options::SetNMEAFormToSerial()
{
    ShowNMEACommon( true );
    ShowNMEANet( false );
    ShowNMEASerial( true );
    m_pNMEAForm->Layout();
    Fit();
    Layout();
    SetDSFormRWStates();
}

void options::SetNMEAFormToNet()
{
    ShowNMEACommon( true );
    ShowNMEANet( true );
    ShowNMEASerial( false );
    m_pNMEAForm->Layout();
    Fit();
    Layout();
    SetDSFormRWStates();
}

wxString StringArrayToString(wxArrayString arr)
{
    wxString ret = wxEmptyString;
    for (size_t i = 0; i < arr.Count(); i++)
    {
        if (i > 0)
            ret.Append(_T(","));
        ret.Append(arr[i]);
    }
    return ret;
}

void options::SetDSFormRWStates()
{
    if (m_rbNetProtoGPSD->GetValue() && !m_rbTypeSerial->GetValue())
    {
        if (m_tNetPort->GetValue() == wxEmptyString)
            m_tNetPort->SetValue(_T("2947"));
        m_cbOutput->SetValue(false);
        m_cbOutput->Enable(false);
        m_rbOAccept->Enable(false);
        m_rbOIgnore->Enable(false);
        m_btnOutputStcList->Enable(false);
    }
    else
    {
        m_cbOutput->Enable(true);
        m_rbOAccept->Enable(true);
        m_rbOIgnore->Enable(true);
        m_btnOutputStcList->Enable(true);
    }
}

void options::SetConnectionParams(ConnectionParams *cp)
{
    m_comboPort->Select(m_comboPort->FindString(cp->Port));
    m_comboPort->SetValue(cp->Port);
    m_cbCheckCRC->SetValue(cp->ChecksumCheck);
    m_cbGarminHost->SetValue(cp->Garmin);
    m_cbFurunoGP3X->SetValue(cp->FurunoGP3X);
    m_cbOutput->SetValue(cp->Output);
    if(cp->InputSentenceListType == WHITELIST)
        m_rbIAccept->SetValue(true);
    else
        m_rbIIgnore->SetValue(true);
    if(cp->OutputSentenceListType == WHITELIST)
        m_rbOAccept->SetValue(true);
    else
        m_rbOIgnore->SetValue(true);
    m_tcInputStc->SetValue(StringArrayToString(cp->InputSentenceList));
    m_tcOutputStc->SetValue(StringArrayToString(cp->OutputSentenceList));
    m_choiceBaudRate->Select(m_choiceBaudRate->FindString(wxString::Format(_T("%d"),cp->Baudrate)));
    m_choiceSerialProtocol->Select(cp->Protocol); //TODO
    m_choicePriority->Select(m_choicePriority->FindString(wxString::Format(_T("%d"),cp->Priority)));

    m_tNetAddress->SetValue(cp->NetworkAddress);
    m_tNetPort->SetValue(wxString::Format(wxT("%i"), cp->NetworkPort));
    if(cp->NetProtocol == TCP)
        m_rbNetProtoTCP->SetValue(true);
    else if (cp->NetProtocol == UDP)
        m_rbNetProtoUDP->SetValue(true);
    else {
        m_rbNetProtoGPSD->SetValue(true);
        if( cp->NetworkPort == 0)
            m_tNetPort->SetValue(_T(""));
    }

    if ( cp->Type == Serial )
    {
        m_rbTypeSerial->SetValue( true );
        SetNMEAFormToSerial();
    }
    else
    {
        m_rbTypeNet->SetValue( true );
        SetNMEAFormToNet();
    }
}

void options::OnAddDatasourceClick( wxCommandEvent& event )
{
    connectionsaved = false;
    ConnectionParams *cp = new ConnectionParams();
    SetConnectionParams( cp );
    SetNMEAFormToSerial();
//    params_saved = false;

    long itemIndex = -1;
    for ( ;; )
    {
        itemIndex = m_lcSources->GetNextItem( itemIndex, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
        if ( itemIndex == -1 )
            break;
        m_lcSources->SetItemState( itemIndex, 0, wxLIST_STATE_SELECTED|wxLIST_STATE_FOCUSED );
    }
    m_buttonRemove->Enable( false );
}

void options::FillSourceList()
{
    m_buttonRemove->Enable(false);
    m_lcSources->DeleteAllItems();
    for (size_t i = 0; i < g_pConnectionParams->Count(); i++)
    {
        long itemIndex = m_lcSources->InsertItem(i, g_pConnectionParams->Item(i)->GetSourceTypeStr());
        m_lcSources->SetItem(itemIndex, 1, g_pConnectionParams->Item(i)->GetAddressStr());
        m_lcSources->SetItem(itemIndex, 2, g_pConnectionParams->Item(i)->GetParametersStr());
        m_lcSources->SetItem(itemIndex, 3, g_pConnectionParams->Item(i)->GetOutputValueStr());
        m_lcSources->SetItem(itemIndex, 4, g_pConnectionParams->Item(i)->GetFiltersStr());
    }

    m_lcSources->SetColumnWidth( 0, wxLIST_AUTOSIZE );
    m_lcSources->SetColumnWidth( 1, wxLIST_AUTOSIZE );
    m_lcSources->SetColumnWidth( 2, wxLIST_AUTOSIZE_USEHEADER );
    m_lcSources->SetColumnWidth( 3, wxLIST_AUTOSIZE_USEHEADER );
    m_lcSources->SetColumnWidth( 4, wxLIST_AUTOSIZE );
}

void options::OnRemoveDatasourceClick( wxCommandEvent& event )
{
    long itemIndex = -1;
    for ( ;; )
    {
        itemIndex = m_lcSources->GetNextItem( itemIndex, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
        if ( itemIndex == -1 )
            break;
        g_pConnectionParams->RemoveAt( itemIndex );

        //  Mark connection deleted  
        m_rbTypeSerial->SetValue(true);
        m_comboPort->SetValue( _T("Deleted") );
    }
    FillSourceList();
    ShowNMEACommon( false );
    ShowNMEANet( false );
    ShowNMEASerial( false );
}

void options::OnSelectDatasource( wxListEvent& event )
{
    connectionsaved = false;
    SetConnectionParams(g_pConnectionParams->Item(event.GetIndex()));
    m_buttonRemove->Enable();
    event.Skip();
}

void options::OnBtnIStcs( wxCommandEvent& event )
{
    m_stcdialog_in->SetSentenceList(wxStringTokenize(m_tcInputStc->GetValue(), _T(",")));
    m_stcdialog_in->SetType(0, (m_rbIAccept->GetValue() == true) ? WHITELIST:BLACKLIST);

    if (m_stcdialog_in->ShowModal() == wxID_OK)
        m_tcInputStc->SetValue(m_stcdialog_in->GetSentencesAsText());
}

void options::OnBtnOStcs( wxCommandEvent& event )
{
    m_stcdialog_out->SetSentenceList(wxStringTokenize(m_tcOutputStc->GetValue(), _T(",")));
    m_stcdialog_out->SetType(1, (m_rbOAccept->GetValue() == true) ? WHITELIST:BLACKLIST);

    if (m_stcdialog_out->ShowModal() == wxID_OK)
        m_tcOutputStc->SetValue(m_stcdialog_out->GetSentencesAsText());
}

void options::OnNetProtocolSelected( wxCommandEvent& event )
{
    if (m_rbNetProtoGPSD->GetValue())
    {
        if (m_tNetPort->GetValue() == wxEmptyString)
            m_tNetPort->SetValue(_T("2947"));
    }
    SetDSFormRWStates();
    OnConnValChange(event);
}

void options::OnRbAcceptInput( wxCommandEvent& event )
{
    OnConnValChange(event);
}
void options::OnRbIgnoreInput( wxCommandEvent& event )
{
    OnConnValChange(event);
}

void options::OnRbOutput( wxCommandEvent& event )
{
    OnConnValChange(event);
}

//SentenceListDlg

SentenceListDlg::SentenceListDlg( FilterDirection dir, wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
    m_dir = dir;
    this->SetSizeHints( wxDefaultSize, wxDefaultSize );

    wxBoxSizer* bSizer16;
    bSizer16 = new wxBoxSizer( wxVERTICAL );

    wxBoxSizer* bSizer17;
    bSizer17 = new wxBoxSizer( wxHORIZONTAL );

    NMEA0183 nmea;
    standard_sentences = nmea.GetRecognizedArray();
    if(m_dir == FILTER_OUTPUT) {
        standard_sentences.Add(_T("ECRMB"));
        standard_sentences.Add(_T("ECRMC"));
    }

    m_pclbBox = new wxStaticBox( this,  wxID_ANY, _T("")) ;

    wxStaticBoxSizer* sbSizerclb;
    sbSizerclb = new wxStaticBoxSizer( m_pclbBox , wxVERTICAL );
    bSizer17->Add( sbSizerclb, 1, wxALL|wxEXPAND, 5 );

    m_clbSentences = new wxCheckListBox( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, standard_sentences );


    sbSizerclb->Add( m_clbSentences, 1, wxALL|wxEXPAND, 5 );

    wxBoxSizer* bSizer18;
    bSizer18 = new wxBoxSizer( wxVERTICAL );

    m_btnCheckAll = new wxButton( this, wxID_ANY, _("Select All"), wxDefaultPosition, wxDefaultSize, 0 );
    bSizer18->Add( m_btnCheckAll, 0, wxALL, 5 );

    m_btnClearAll = new wxButton( this, wxID_ANY, _("Clear All"), wxDefaultPosition, wxDefaultSize, 0 );
    bSizer18->Add( m_btnClearAll, 0, wxALL, 5 );

    bSizer18->AddSpacer(1);

    m_btnAdd = new wxButton( this, wxID_ANY, _("Add"), wxDefaultPosition, wxDefaultSize, 0 );
    bSizer18->Add( m_btnAdd, 0, wxALL, 5 );

    m_btnDel = new wxButton( this, wxID_ANY, _("Delete"), wxDefaultPosition, wxDefaultSize, 0 );
    m_btnDel->Enable( false );

    bSizer18->Add( m_btnDel, 0, wxALL, 5 );


    bSizer17->Add( bSizer18, 0, wxALL|wxEXPAND, 5 );


    bSizer16->Add( bSizer17, 1, wxEXPAND, 5 );

    m_sdbSizer4 = new wxStdDialogButtonSizer();
    m_sdbSizer4OK = new wxButton( this, wxID_OK );
    m_sdbSizer4->AddButton( m_sdbSizer4OK );
    m_sdbSizer4Cancel = new wxButton( this, wxID_CANCEL );
    m_sdbSizer4->AddButton( m_sdbSizer4Cancel );
    m_sdbSizer4->Realize();

    bSizer16->Add( m_sdbSizer4, 0, wxALL|wxEXPAND, 5 );


    this->SetSizer( bSizer16 );
    this->Layout();

    this->Centre( wxBOTH );

    // Connect Events
    m_btnAdd->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SentenceListDlg::OnAddClick ), NULL, this );
    m_btnDel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SentenceListDlg::OnDeleteClick ), NULL, this );
    m_sdbSizer4Cancel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SentenceListDlg::OnCancelClick ), NULL, this );
    m_sdbSizer4OK->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SentenceListDlg::OnOkClick ), NULL, this );
    m_clbSentences->Connect( wxEVT_COMMAND_CHECKLISTBOX_TOGGLED, wxCommandEventHandler( SentenceListDlg::OnCLBToggle ), NULL, this );
    m_clbSentences->Connect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( SentenceListDlg::OnCLBSelect ), NULL, this );
    m_btnCheckAll->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SentenceListDlg::OnCheckAllClick ), NULL, this );
    m_btnClearAll->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SentenceListDlg::OnClearAllClick ), NULL, this );

}

SentenceListDlg::~SentenceListDlg()
{
    // Disconnect Events
    m_btnAdd->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SentenceListDlg::OnAddClick ), NULL, this );
    m_btnDel->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SentenceListDlg::OnDeleteClick ), NULL, this );
    m_sdbSizer4Cancel->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SentenceListDlg::OnCancelClick ), NULL, this );
    m_sdbSizer4OK->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SentenceListDlg::OnOkClick ), NULL, this );
    m_clbSentences->Disconnect( wxEVT_COMMAND_CHECKLISTBOX_TOGGLED, wxCommandEventHandler( SentenceListDlg::OnCLBToggle ), NULL, this );
    m_clbSentences->Disconnect( wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler( SentenceListDlg::OnCLBSelect ), NULL, this );
    m_btnCheckAll->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SentenceListDlg::OnCheckAllClick ), NULL, this );
    m_btnClearAll->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( SentenceListDlg::OnClearAllClick ), NULL, this );
}

void SentenceListDlg::SetSentenceList(wxArrayString sentences)
{
    m_sentences = sentences;

    if( (m_sentences.Count() == 0) && (m_type == WHITELIST) ){
        for (size_t i = 0; i < m_clbSentences->GetCount(); i++)
            m_clbSentences->Check(i, true);
    }
    if( (m_sentences.Count() == 0) && (m_type == BLACKLIST) ){
        for (size_t i = 0; i < m_clbSentences->GetCount(); i++)
            m_clbSentences->Check(i, false);
    }

    FillSentences();
}

wxString SentenceListDlg::GetSentencesAsText()
{
    return StringArrayToString(m_sentences);
}

void SentenceListDlg::BuildSentenceArray()
{
    m_sentences.Clear();
    wxString s;
    for (size_t i = 0; i < m_clbSentences->GetCount(); i++) {
        if( m_clbSentences->IsChecked(i))
            m_sentences.Add( m_clbSentences->GetString(i) );
    }
}

void SentenceListDlg::FillSentences()
{
    if(m_sentences.Count() == 0)
        return;

    for (size_t i = 0; i < m_clbSentences->GetCount(); i++)
        m_clbSentences->Check(i, false);

    for (size_t i = 0; i < m_sentences.Count(); i++) {
        int item = m_clbSentences->FindString(m_sentences[i]);
        if( wxNOT_FOUND != item )
            m_clbSentences->Check(item);
        else {
            m_clbSentences->Append(m_sentences[i]);
            int item = m_clbSentences->FindString(m_sentences[i]);
            m_clbSentences->Check(item);
        }

    }

    m_btnDel->Enable(false);
}

void SentenceListDlg::OnStcSelect( wxCommandEvent& event )
{
    m_btnDel->Enable();
}

void SentenceListDlg::OnCLBSelect( wxCommandEvent& event )
{
    // Only active the "Delete" button if the selection is not in the standard list
    int isel = m_clbSentences->GetSelection();
    bool bdelete = true;
    if(isel >= 0) {
        wxString s = m_clbSentences->GetString(isel);
        for (size_t i = 0; i < standard_sentences.Count(); i++) {
            if(standard_sentences[i] == s){
                bdelete = false;
                break;
            }
        }
    }
    else
        bdelete = false;
    m_btnDel->Enable( bdelete );

}

void SentenceListDlg::OnCLBToggle( wxCommandEvent& event )
{
    BuildSentenceArray();
}

void SentenceListDlg::OnAddClick( wxCommandEvent& event )
{
    wxString stc = wxGetTextFromUser(_("Enter the NMEA sentence (2, 3 or 5 characters)"), _("Enter the NMEA sentence"));
    if (stc.Length() == 2 ||stc.Length() == 3 || stc.Length() == 5)
    {
        m_sentences.Add(stc);
        m_clbSentences->Append(stc);
        int item = m_clbSentences->FindString(stc);
        m_clbSentences->Check(item);
    }
    else
        wxMessageBox(_("An NMEA sentence is generally 3 characters long (like RMC, GGA etc.) It can also have a two letter prefix identifying the source, or TALKER, of the message (The whole sentences then looks like GPGGA or AITXT). You may filter out all the sentences with certain TALKER prefix (like GP, AI etc.). The filter accepts just these three formats."), _("Wrong length of the NMEA filter value"));
}

void SentenceListDlg::OnDeleteClick( wxCommandEvent& event )
{
    BuildSentenceArray();

    // One can only delete items that do not appear in the standard sentence list
    int isel = m_clbSentences->GetSelection();
    wxString s = m_clbSentences->GetString(isel);
    bool bdelete = true;
    for (size_t i = 0; i < standard_sentences.Count(); i++) {
        if(standard_sentences[i] == s){
            bdelete = false;
            break;
        }
    }

    if(bdelete) {
        m_sentences.Remove( s );
        m_clbSentences->Delete(isel);
    }

    FillSentences();
}

void SentenceListDlg::OnClearAllClick( wxCommandEvent& event )
{
    for (size_t i = 0; i < m_clbSentences->GetCount(); i++)
        m_clbSentences->Check(i, false);

    BuildSentenceArray();
}

void SentenceListDlg::OnCheckAllClick( wxCommandEvent& event )
{
    for (size_t i = 0; i < m_clbSentences->GetCount(); i++)
        m_clbSentences->Check(i, true);

    BuildSentenceArray();
}

void SentenceListDlg::SetType(int io, ListType type)
{
    m_type = type;

    switch (io)
    {
        case 0:                 // input
        default:
            if(type == WHITELIST)
                m_pclbBox->SetLabel(_("Accept Sentences"));
            else
                m_pclbBox->SetLabel(_("Ignore Sentences"));
            break;
        case 1:                 // output
            if(type == WHITELIST)
                m_pclbBox->SetLabel(_("Transmit Sentences"));
            else
                m_pclbBox->SetLabel(_("Drop Sentences"));
            break;
    }
    Refresh();
}



void SentenceListDlg::OnCancelClick( wxCommandEvent& event ) { event.Skip(); }
void SentenceListDlg::OnOkClick( wxCommandEvent& event ) { event.Skip(); }
