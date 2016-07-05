/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Track Properties Dialog
 * Author:   David Register
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
 **************************************************************************/

#include "navutil.h"
#include "georef.h"
#include "routeman.h"
#include "routemanagerdialog.h"
#include "routeprintout.h"
#include "pluginmanager.h"
#include "OCPNPlatform.h"
#include "TrackPropDlg.h"

#ifdef __OCPN__ANDROID__
#include "androidUTIL.h"
#endif

extern double               gLat, gLon;
extern RouteList           *pRouteList;
extern TrackList           *pTrackList;
extern ActiveTrack         *g_pActiveTrack;
extern Routeman            *g_pRouteMan;
extern Select              *pSelect;
extern RouteManagerDialog  *pRouteManagerDialog;
extern MyConfig            *pConfig;
extern MyFrame             *gFrame;
extern ChartCanvas         *cc1;
extern PlugInManager       *g_pi_manager;

#define    UTCINPUT         0
#define    LTINPUT          1    // i.e. this PC local time
#define    LMTINPUT         2    // i.e. the remote location LMT time
#define    INPUT_FORMAT     1
#define    DISPLAY_FORMAT   2
#define    TIMESTAMP_FORMAT 3

wxString timestamp2s(wxDateTime ts, int tz_selection, long LMT_offset, int format)
{
    wxString s = _T("");
    wxString f;
    if (format == INPUT_FORMAT) f = _T("%m/%d/%Y %H:%M");
    else if (format == TIMESTAMP_FORMAT) f = _T("%m/%d/%Y %H:%M:%S");
    else f = _T(" %m/%d %H:%M");
    switch (tz_selection) {
        case UTCINPUT:
            s.Append(ts.Format(f));
            if (format != INPUT_FORMAT)
                s.Append(_T(" UT"));
            break;
        case LTINPUT:
            s.Append(ts.FromUTC().Format(f));
            break;
        case LMTINPUT:
            wxTimeSpan lmt(0,0,(int)LMT_offset,0);
            s.Append(ts.Add(lmt).Format(f));
            if (format != INPUT_FORMAT) s.Append(_T(" LMT"));
    }
    return(s);
}

///////////////////////////////////////////////////////////////////////////
bool TrackPropDlg::instanceFlag = false;
TrackPropDlg* TrackPropDlg::single = NULL;
TrackPropDlg* TrackPropDlg::getInstance( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style )
{
    if(! instanceFlag)
    {
        single = new TrackPropDlg( parent, id, title, pos, size, style);
        instanceFlag = true;
        return single;
    }
    else
    {
        return single;
    }
}

TrackPropDlg::TrackPropDlg( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style )
  : wxDialog( parent, id, title, pos, size, style )
{

    long wstyle = style;
#ifdef __WXOSX__
    wstyle |= wxSTAY_ON_TOP;
#endif

    wxFont *qFont = GetOCPNScaledFont(_("Dialog"));
    SetFont( *qFont );

    SetWindowStyleFlag( wstyle );

    
    m_scrolledWindowLinks = NULL;
    m_tDescription = NULL;
    
    m_bcompact = false;
    
#ifdef __OCPN__ANDROID__
    m_bcompact = true;
    CreateControlsCompact();
#else
    CreateControls();
#endif
    

    RecalculateSize();


    //Centre( );

    // Connect Events
    m_sdbBtmBtnsSizerCancel->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( TrackPropDlg::OnCancelBtnClick ), NULL, this );
    m_sdbBtmBtnsSizerOK->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( TrackPropDlg::OnOKBtnClick ), NULL, this );
    if(m_sdbBtmBtnsSizerPrint)m_sdbBtmBtnsSizerPrint->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( TrackPropDlg::OnPrintBtnClick ), NULL, this );
    m_sdbBtmBtnsSizerSplit->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( TrackPropDlg::OnSplitBtnClick ), NULL, this );
    m_sdbBtmBtnsSizerExtend->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( TrackPropDlg::OnExtendBtnClick ), NULL, this );
    m_sdbBtmBtnsSizerToRoute->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( TrackPropDlg::OnToRouteBtnClick ), NULL, this );
    if(m_sdbBtmBtnsSizerExport)m_sdbBtmBtnsSizerExport->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( TrackPropDlg::OnExportBtnClick ), NULL, this );
    m_lcPoints->Connect(wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler( TrackPropDlg::OnTrackPropListClick ), NULL, this );
    Connect( wxEVT_COMMAND_LIST_ITEM_RIGHT_CLICK, wxListEventHandler(TrackPropDlg::OnTrackPropRightClick), NULL, this );
    Connect( wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(TrackPropDlg::OnTrackPropMenuSelected), NULL, this );

//    m_buttonAddLink->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
//            wxCommandEventHandler( TrackPropDlg::OnAddLink ), NULL, this );
//    m_toggleBtnEdit->Connect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED,
//            wxCommandEventHandler( TrackPropDlg::OnEditLinkToggle ), NULL, this );

    if(m_rbShowTimeUTC)m_rbShowTimeUTC->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( TrackPropDlg::OnShowTimeTZ), NULL, this );
    if(m_rbShowTimePC)m_rbShowTimePC->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( TrackPropDlg::OnShowTimeTZ), NULL, this );
    if(m_rbShowTimeLocal)m_rbShowTimeLocal->Connect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( TrackPropDlg::OnShowTimeTZ), NULL, this );

    m_pLinkProp = new LinkPropImpl( this );
    m_pMyLinkList = NULL;
    
    
}

TrackPropDlg::~TrackPropDlg()
{
    // Disconnect Events
    m_sdbBtmBtnsSizerCancel->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( TrackPropDlg::OnCancelBtnClick ), NULL, this );
    m_sdbBtmBtnsSizerOK->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( TrackPropDlg::OnOKBtnClick ), NULL, this );
    if(m_sdbBtmBtnsSizerPrint)m_sdbBtmBtnsSizerPrint->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( TrackPropDlg::OnPrintBtnClick ), NULL, this );
    m_sdbBtmBtnsSizerSplit->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( TrackPropDlg::OnSplitBtnClick ), NULL, this );
    m_sdbBtmBtnsSizerExtend->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( TrackPropDlg::OnExtendBtnClick ), NULL, this );
    m_sdbBtmBtnsSizerToRoute->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( TrackPropDlg::OnToRouteBtnClick ), NULL, this );
    if(m_sdbBtmBtnsSizerExport)m_sdbBtmBtnsSizerExport->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( TrackPropDlg::OnExportBtnClick ), NULL, this );
    m_lcPoints->Disconnect(wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler( TrackPropDlg::OnTrackPropListClick ), NULL, this );
    Disconnect( wxEVT_COMMAND_LIST_ITEM_RIGHT_CLICK, wxListEventHandler(TrackPropDlg::OnTrackPropRightClick), NULL, this );
    Disconnect( wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(TrackPropDlg::OnTrackPropMenuSelected), NULL, this );

    Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED,
            wxCommandEventHandler( TrackPropDlg::OnDeleteLink ) );
    Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED,
            wxCommandEventHandler( TrackPropDlg::OnEditLink ) );
    Disconnect( wxID_ANY, wxEVT_COMMAND_MENU_SELECTED,
            wxCommandEventHandler( TrackPropDlg::OnAddLink ) );
//    m_buttonAddLink->Disconnect( wxEVT_COMMAND_BUTTON_CLICKED,
//            wxCommandEventHandler( TrackPropDlg::OnAddLink ), NULL, this );
 //   m_toggleBtnEdit->Disconnect( wxEVT_COMMAND_TOGGLEBUTTON_CLICKED,
//            wxCommandEventHandler( TrackPropDlg::OnEditLinkToggle ), NULL, this );

    if(m_rbShowTimeUTC)m_rbShowTimeUTC->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( TrackPropDlg::OnShowTimeTZ), NULL, this );
    if(m_rbShowTimePC)m_rbShowTimePC->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( TrackPropDlg::OnShowTimeTZ), NULL, this );
    if(m_rbShowTimeLocal)m_rbShowTimeLocal->Disconnect( wxEVT_COMMAND_RADIOBUTTON_SELECTED, wxCommandEventHandler( TrackPropDlg::OnShowTimeTZ), NULL, this );

    //delete m_menuLink;
    instanceFlag = false;
}

void TrackPropDlg::RecalculateSize( void )
{
    //  Make an estimate of the dialog size, without scrollbars showing

    wxSize esize;
    esize.x = GetCharWidth() * 110;
    esize.y = GetCharHeight() * 40;

    wxSize dsize = GetParent()->GetClientSize();
    esize.y = wxMin(esize.y, dsize.y - (2 * GetCharHeight()));
    esize.x = wxMin(esize.x, dsize.x - (2 * GetCharHeight()));
    SetClientSize(esize);

    wxSize fsize = GetSize();
    fsize.y = wxMin(fsize.y, dsize.y - (2 * GetCharHeight()));
    fsize.x = wxMin(fsize.x, dsize.x - (2 * GetCharHeight()));
    SetSize(fsize);

    if(m_bcompact){
        int sy = GetCharHeight() * m_lcPoints->GetItemCount();
        sy = wxMax(sy, 250);
        sy = wxMin(sy, 500);
        m_lcPoints->SetSize(wxSize(GetClientSize().x-40, sy) );
        
        if(m_lcPoints->GetItemCount())
            Layout();
    }
    

    Centre();

}

void TrackPropDlg::CreateControlsCompact()
{


    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer( wxVERTICAL );
    SetSizer( itemBoxSizer1 );

    itemDialog1 = new wxScrolledWindow( this, wxID_ANY,
                                      wxDefaultPosition, wxSize(-1, -1), wxVSCROLL);
    itemDialog1->SetScrollRate(0, 1);


#ifdef __OCPN__ANDROID__
    //  Set Dialog Font by custom crafted Qt Stylesheet.
    wxFont *qFont = GetOCPNScaledFont(_("Dialog"));

    wxString wqs = getFontQtStylesheet(qFont);
    wxCharBuffer sbuf = wqs.ToUTF8();
    QString qsb = QString(sbuf.data());

    QString qsbq = getQtStyleSheet();           // basic scrollbars, etc

    itemDialog1->GetHandle()->setStyleSheet( qsb + qsbq );      // Concatenated style sheets

#endif
    itemBoxSizer1->Add( itemDialog1, 1, wxEXPAND | wxALL, 0 );

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer( wxVERTICAL );
    itemDialog1->SetSizer( itemBoxSizer2 );


    wxStaticText* itemStaticText4 = new wxStaticText( itemDialog1, wxID_STATIC, _("Name"),
            wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add( itemStaticText4, 0,
                              wxALIGN_LEFT | wxLEFT | wxRIGHT | wxTOP, 5 );

    m_tName = new wxTextCtrl( itemDialog1, wxID_ANY, _T(""), wxDefaultPosition,
            wxSize( 400, -1 ), 0 );
    itemBoxSizer2->Add( m_tName, 0,
                        wxALIGN_LEFT | wxLEFT | wxRIGHT | wxBOTTOM , 5 );

    wxStaticText* itemStaticText7 = new wxStaticText( itemDialog1, wxID_STATIC, _("Depart From"),
            wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add( itemStaticText7, 0,
            wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 5 );

    m_tFrom = new wxTextCtrl( itemDialog1, wxID_ANY, _T(""), wxDefaultPosition,
                                      wxSize( -1, -1 ), 0 );
    itemBoxSizer2->Add( m_tFrom, 0,
                              wxEXPAND | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM, 5 );

    wxStaticText* itemStaticText8 = new wxStaticText( itemDialog1, wxID_STATIC, _("Destination"),
            wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add( itemStaticText8, 0,
            wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 5 );


    m_tTo = new wxTextCtrl( itemDialog1, wxID_ANY, _T(""), wxDefaultPosition,
            wxSize( -1, -1 ), 0 );
    itemBoxSizer2->Add( m_tTo, 0,
                        wxEXPAND | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM, 5 );

    m_cbShow = new wxCheckBox( itemDialog1, wxID_ANY, _("Show on chart"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add( m_cbShow, 0, wxALL, 5 );
    
    

    wxFlexGridSizer* itemFlexGridSizer6a = new wxFlexGridSizer( 4, 2, 0, 0 );
    itemFlexGridSizer6a->AddGrowableCol(1, 0);

    itemBoxSizer2->Add( itemFlexGridSizer6a, 0, wxEXPAND | wxALIGN_LEFT | wxALL, 5 );

    wxStaticText* itemStaticText11 = new wxStaticText( itemDialog1, wxID_STATIC,
            _("Total Distance"), wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer6a->Add( itemStaticText11, 0,
            wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxTOP,
            5 );

    m_tTotDistance = new wxTextCtrl( itemDialog1, wxID_ANY, _T(""), wxDefaultPosition,
                                     wxSize( -1, -1 ), wxTE_READONLY );
    itemFlexGridSizer6a->Add( m_tTotDistance, 0,
                              wxEXPAND | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM, 5 );




    m_stAvgSpeed = new wxStaticText( itemDialog1, wxID_STATIC, _("Avg. speed"),
            wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer6a->Add( m_stAvgSpeed, 0,
            wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxTOP,
            5 );

    m_tAvgSpeed = new wxTextCtrl( itemDialog1, wxID_ANY, _T(""), wxDefaultPosition,
                                     wxSize( 150, -1 ), wxTE_PROCESS_ENTER );
    itemFlexGridSizer6a->Add( m_tAvgSpeed, 0,
                              wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM, 5 );




   m_stTimeEnroute = new wxStaticText( itemDialog1, wxID_STATIC, _("Time Enroute"),
            wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer6a->Add( m_stTimeEnroute, 0,
            wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxTOP,
            5 );

    m_tTimeEnroute = new wxTextCtrl( itemDialog1, wxID_ANY, _T(""), wxDefaultPosition,
                                       wxSize( -1, -1 ), wxTE_READONLY );
    itemFlexGridSizer6a->Add( m_tTimeEnroute, 0,
                              wxEXPAND | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM, 5 );

/*
    m_StartTimeLabel = new wxStaticText( itemDialog1, wxID_STATIC, _("Departure Time"),
            wxDefaultPosition, wxDefaultSize, 0 );
    itemFlexGridSizer6a->Add( m_StartTimeLabel, 0,
            wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxTOP,
            5 );

    m_StartTimeCtl = new wxTextCtrl( itemDialog1, ID_STARTTIMECTL, _T(""), wxDefaultPosition,
            wxSize( -1, -1 ), wxTE_PROCESS_ENTER );
    itemFlexGridSizer6a->Add( m_StartTimeCtl, 0,
            wxEXPAND | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM, 5 );
*/


    wxString pDispTimeZone[] = { _("UTC"), _("Local @ PC"), _("LMT @ Location") };

    wxStaticText* itemStaticText12b = new wxStaticText( itemDialog1, wxID_STATIC, _("Times shown as"),
                                                                                    wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer2->Add( itemStaticText12b, 0, wxEXPAND | wxALL, 5 );


    m_rbShowTimeUTC = new wxRadioButton(itemDialog1, wxID_ANY, _("UTC"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    itemBoxSizer2->Add( m_rbShowTimeUTC, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM,5 );

    m_rbShowTimePC = new wxRadioButton(itemDialog1, wxID_ANY, _("Local @ PC"));
    itemBoxSizer2->Add( m_rbShowTimePC, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM,5 );

    m_rbShowTimeLocal = new wxRadioButton(itemDialog1, wxID_ANY, _("LMT @ Location"));
    itemBoxSizer2->Add( m_rbShowTimeLocal, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM,5 );


    wxFlexGridSizer* itemFlexGridSizer6b = new wxFlexGridSizer( 3, 2, 0, 0 );
    itemBoxSizer2->Add( itemFlexGridSizer6b, 0, wxEXPAND | wxALIGN_LEFT | wxALL, 5 );

    wxStaticText *m_staticText1 = new wxStaticText( itemDialog1, wxID_ANY, _("Color:"), wxDefaultPosition, wxDefaultSize,
            0 );
    itemFlexGridSizer6b->Add( m_staticText1, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5 );

    wxString m_chColorChoices[] = { _("Default color"), _("Black"), _("Dark Red"), _("Dark Green"),
            _("Dark Yellow"), _("Dark Blue"), _("Dark Magenta"), _("Dark Cyan"),
            _("Light Gray"), _("Dark Gray"), _("Red"), _("Green"), _("Yellow"), _("Blue"),
            _("Magenta"), _("Cyan"), _("White") };
    int m_chColorNChoices = sizeof( m_chColorChoices ) / sizeof(wxString);
    m_cColor = new wxChoice( itemDialog1, wxID_ANY, wxDefaultPosition, wxSize(250, -1), m_chColorNChoices,
            m_chColorChoices, 0 );
    m_cColor->SetSelection( 0 );
    itemFlexGridSizer6b->Add( m_cColor, 0,  wxALIGN_CENTER_VERTICAL | wxALL, 5 );

    wxStaticText *staticTextStyle = new wxStaticText( itemDialog1, wxID_ANY, _("Style:"), wxDefaultPosition, wxDefaultSize,
            0 );
    itemFlexGridSizer6b->Add( staticTextStyle, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5 );

    wxString m_chStyleChoices[] = { _("Default"), _("Solid"), _("Dot"), _("Long dash"),
            _("Short dash"), _("Dot dash") };
    int m_chStyleNChoices = sizeof( m_chStyleChoices ) / sizeof(wxString);
    m_cStyle = new wxChoice( itemDialog1, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_chStyleNChoices,
            m_chStyleChoices, 0 );
    m_cStyle->SetSelection( 0 );
    itemFlexGridSizer6b->Add( m_cStyle, 0, wxEXPAND | wxALIGN_CENTER_VERTICAL | wxALL, 5 );

#ifdef ocpnUSE_GLES // linestipple is emulated poorly
    staticTextStyle->Hide();
    m_cStyle->Hide();
#endif


    m_stWidth = new wxStaticText( itemDialog1, wxID_ANY, _("Width:"), wxDefaultPosition, wxDefaultSize,
            0 );
    itemFlexGridSizer6b->Add( m_stWidth, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5 );

    wxString m_chWidthChoices[] = { _("Default"), _("1 pixel"), _("2 pixels"), _("3 pixels"),
            _("4 pixels"), _("5 pixels"), _("6 pixels"), _("7 pixels"), _("8 pixels"),
            _("9 pixels"), _("10 pixels") };
    int m_chWidthNChoices = sizeof( m_chWidthChoices ) / sizeof(wxString);
    m_cWidth = new wxChoice( itemDialog1, wxID_ANY, wxDefaultPosition, wxSize(150, -1), m_chWidthNChoices,
            m_chWidthChoices, 0 );
    m_cWidth->SetSelection( 0 );
    itemFlexGridSizer6b->Add( m_cWidth, 0, wxEXPAND | wxALIGN_CENTER_VERTICAL | wxALL, 5 );



    wxStaticBox* itemStaticBoxSizer14Static = new wxStaticBox( itemDialog1, wxID_ANY, _("Waypoints") );
    wxStaticBoxSizer *m_pListSizer = new wxStaticBoxSizer( itemStaticBoxSizer14Static, wxVERTICAL );
    itemBoxSizer2->Add( m_pListSizer, 1, wxEXPAND | wxALL, 1 );


    wxScrolledWindow *itemlistWin = new wxScrolledWindow( itemDialog1, wxID_ANY,
                                                          wxDefaultPosition, wxDefaultSize, wxHSCROLL | wxVSCROLL);
    itemlistWin->SetScrollRate(2, 2);



    m_pListSizer->Add( itemlistWin, 0, wxEXPAND | wxALL, 6 );


    long flags = wxLC_REPORT | wxLC_HRULES | wxLC_VRULES | wxLC_EDIT_LABELS;
#ifndef __WXQT__                // Does not support Virtual list boxes....
      flags |= wxLC_VIRTUAL;
#endif

      m_lcPoints = new OCPNTrackListCtrl( itemlistWin, wxID_ANY, wxDefaultPosition, wxSize( 100, 500 ), flags);


     // sbSizerPoints->Add( m_lcPoints, 1, wxALL|wxEXPAND, 5 );

      int dx = GetCharWidth();

      m_lcPoints->InsertColumn( 0, _("Leg"), wxLIST_FORMAT_LEFT, dx * 6/*45*/ );
      m_lcPoints->InsertColumn( 1, _("Distance"), wxLIST_FORMAT_LEFT, dx * 10/*70*/ );
      m_lcPoints->InsertColumn( 2, _("Bearing"), wxLIST_FORMAT_LEFT, dx * 8/*70*/ );
      m_lcPoints->InsertColumn( 3, _("Latitude"), wxLIST_FORMAT_LEFT, dx * 11/*85*/ );
      m_lcPoints->InsertColumn( 4, _("Longitude"), wxLIST_FORMAT_LEFT, dx * 11/*90*/ );
      m_lcPoints->InsertColumn( 5, _("Timestamp"), wxLIST_FORMAT_LEFT, dx * 14/*135*/ );
      m_lcPoints->InsertColumn( 6, _("Speed"), wxLIST_FORMAT_CENTER, dx * 8/*100*/ );

      m_lcPoints->SetMinSize(wxSize(-1, 50) );

#ifdef __OCPN__ANDROID__
      m_lcPoints->GetHandle()->setStyleSheet( getQtStyleSheet());
#endif


         //  Buttons, etc...

      wxBoxSizer* itemBoxSizerBottom = new wxBoxSizer( wxVERTICAL );
      itemBoxSizer1->Add( itemBoxSizerBottom, 0, wxALIGN_LEFT | wxALL | wxEXPAND, 5 );


      int n_col = 3;

      wxFlexGridSizer* itemBoxSizerAux = new wxFlexGridSizer( 0, n_col, 0, 0 );
      itemBoxSizerAux->SetFlexibleDirection( wxBOTH );
      itemBoxSizerAux->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );



      itemBoxSizerBottom->Add( itemBoxSizerAux, 1, wxALIGN_LEFT | wxALL, 5 );

#ifndef __OCPN__ANDROID__

      m_sdbBtmBtnsSizerPrint = new wxButton( this, wxID_ANY, _("Print"),
                                                                wxDefaultPosition, wxDefaultSize, 0 );
      itemBoxSizerAux->Add( m_sdbBtmBtnsSizerPrint, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 5 );
      m_sdbBtmBtnsSizerPrint->Enable( true );
#else
      m_sdbBtmBtnsSizerPrint = NULL;
#endif

      m_sdbBtmBtnsSizerSplit = new wxButton( this, wxID_ANY, _("Split"),
      wxDefaultPosition, wxDefaultSize, 0 );
      itemBoxSizerAux->Add( m_sdbBtmBtnsSizerSplit, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 5 );
      m_sdbBtmBtnsSizerSplit->Enable( false );

      m_sdbBtmBtnsSizerExtend = new wxButton( this, wxID_ANY, _("Extend"),
                                     wxDefaultPosition, wxDefaultSize, 0 );
      itemBoxSizerAux->Add( m_sdbBtmBtnsSizerExtend, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 5 );

      m_sdbBtmBtnsSizerToRoute = new wxButton( this, wxID_ANY, _("To route"),
                                              wxDefaultPosition, wxDefaultSize, 0 );
      itemBoxSizerAux->Add( m_sdbBtmBtnsSizerToRoute, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 5 );

      m_sdbBtmBtnsSizerExport = NULL;
//      m_sdbBtmBtnsSizerExport = new wxButton( this, wxID_ANY, _("Export"), wxDefaultPosition, wxDefaultSize, 0 );
//      itemBoxSizerAux->Add( m_sdbBtmBtnsSizerExport, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 5 );

      wxBoxSizer* itemBoxSizer16 = new wxBoxSizer( wxHORIZONTAL );
      itemBoxSizerBottom->Add( itemBoxSizer16, 0, wxALIGN_RIGHT | wxALL, 5 );

      m_sdbBtmBtnsSizerCancel = new wxButton( this, wxID_CANCEL, _("Cancel"), wxDefaultPosition,
      wxDefaultSize, 0 );
      itemBoxSizer16->Add( m_sdbBtmBtnsSizerCancel, 0, wxALIGN_RIGHT | wxALIGN_BOTTOM | wxALL, 5 );

      m_sdbBtmBtnsSizerOK = new wxButton( this, wxID_OK, _("OK"), wxDefaultPosition,
      wxDefaultSize, 0 );
      itemBoxSizer16->Add( m_sdbBtmBtnsSizerOK, 0, wxALIGN_RIGHT | wxALIGN_BOTTOM | wxALL, 5 );
      m_sdbBtmBtnsSizerOK->SetDefault();

/*
      //Make it look nice and add the needed non-standard buttons
      int w1, w2, h;
      ((wxWindowBase *)m_stName)->GetSize( &w1, &h );
      ((wxWindowBase *)m_stFrom)->GetSize( &w2, &h );
      ((wxWindowBase *)m_stName)->SetMinSize( wxSize(wxMax(w1, w2), h) );
      ((wxWindowBase *)m_stFrom)->SetMinSize( wxSize(wxMax(w1, w2), h) );
*/
#if 0
      Connect( m_menuItemDelete->GetId(), wxEVT_COMMAND_MENU_SELECTED,
               wxCommandEventHandler( TrackPropDlg::OnDeleteLink ) );
      Connect( m_menuItemEdit->GetId(), wxEVT_COMMAND_MENU_SELECTED,
               wxCommandEventHandler( TrackPropDlg::OnEditLink ) );
      Connect( m_menuItemAdd->GetId(), wxEVT_COMMAND_MENU_SELECTED,
               wxCommandEventHandler( TrackPropDlg::OnAddLink ) );
#endif



    int char_size = GetCharWidth();
    //Set the maximum size of the entire  dialog
    int width, height;
    ::wxDisplaySize( &width, &height );
    SetSizeHints( -1, -1, width-100, height-100 );

}


void TrackPropDlg::CreateControls( void )
{

    wxBoxSizer* bSizerMain;
    bSizerMain = new wxBoxSizer( wxVERTICAL );
    SetSizer( bSizerMain );

    m_notebook1 = new wxNotebook( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );

    bSizerMain->Add( m_notebook1, 1, wxEXPAND | wxALL, 5 );

    wxPanel *m_panel0 = new wxPanel( m_notebook1, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    m_notebook1->AddPage( m_panel0, _("Basic"), true );

    wxBoxSizer* bSizer0 = new wxBoxSizer( wxVERTICAL );
    m_panel0->SetSizer( bSizer0 );

    wxBoxSizer* bSizer1 = new wxBoxSizer( wxVERTICAL );
    bSizer0->Add(bSizer1, 1, wxEXPAND | wxALL, 5 );

    m_panelBasic = new wxScrolledWindow( m_panel0, wxID_ANY,
                                         wxDefaultPosition, wxSize(-1, -1), wxHSCROLL | wxVSCROLL | wxTAB_TRAVERSAL);

#ifdef __OCPN__ANDROID__
    //  Set Dialog Font by custom crafted Qt Stylesheet.
    wxFont *qFont = GetOCPNScaledFont(_("Dialog"));

    wxString wqs = getFontQtStylesheet(qFont);
    wxCharBuffer sbuf = wqs.ToUTF8();
    QString qsb = QString(sbuf.data());

    QString qsbq = getQtStyleSheet();           // basic scrollbars, etc

    m_panelBasic->GetHandle()->setStyleSheet( qsb + qsbq );      // Concatenated style sheets

#endif

//#ifdef __OCPN__ANDROID__
//    m_panelBasic->GetHandle()->setStyleSheet( getQtStyleSheet());
//#endif

    wxBoxSizer* bSizerBasic = new wxBoxSizer( wxVERTICAL );
    m_panelBasic->SetSizer( bSizerBasic );

    bSizer1->Add(m_panelBasic, 1, wxEXPAND | wxALL, 5 );


    wxBoxSizer* bSizerName;
    bSizerName = new wxBoxSizer( wxHORIZONTAL );
    bSizerBasic->Add( bSizerName, 0, wxALL|wxEXPAND, 5 );

    m_stName = new wxStaticText( m_panelBasic, wxID_ANY, _("Name"), wxDefaultPosition, wxDefaultSize, 0 );
    //m_stName->Wrap( -1 );
    bSizerName->Add( m_stName, 0, wxALL, 5 );

    m_tName = new wxTextCtrl( m_panelBasic, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    bSizerName->Add( m_tName, 1, 0, 5 );



    wxBoxSizer* bSizerFromTo;
    bSizerFromTo = new wxBoxSizer( wxHORIZONTAL );
    bSizerBasic->Add( bSizerFromTo, 0, wxALL|wxEXPAND, 5 );

    m_stFrom = new wxStaticText( m_panelBasic, wxID_ANY, _("From"), wxDefaultPosition, wxDefaultSize, 0 );
    //m_stFrom->Wrap( -1 );
    bSizerFromTo->Add( m_stFrom, 0, wxALL, 5 );

    m_tFrom = new wxTextCtrl( m_panelBasic, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    bSizerFromTo->Add( m_tFrom, 1, 0, 5 );

    m_stTo = new wxStaticText( m_panelBasic, wxID_ANY, _("To"), wxDefaultPosition, wxDefaultSize, 0 );
    //m_stTo->Wrap( -1 );
    bSizerFromTo->Add( m_stTo, 0, wxALL, 5 );

    m_tTo = new wxTextCtrl( m_panelBasic, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
    bSizerFromTo->Add( m_tTo, 1, 0, 5 );



    wxStaticBoxSizer* sbSizerParams;
    sbSizerParams = new wxStaticBoxSizer( new wxStaticBox( m_panelBasic, wxID_ANY, _("Display parameters") ), wxHORIZONTAL );
    bSizerBasic->Add( sbSizerParams, 0, wxALL|wxEXPAND, 5 );

    m_cbShow = new wxCheckBox( m_panelBasic, wxID_ANY, _("Show on chart"), wxDefaultPosition, wxDefaultSize, 0 );
    sbSizerParams->Add( m_cbShow, 0, wxALL, 5 );

    m_stColor = new wxStaticText( m_panelBasic, wxID_ANY, _("Color"), wxDefaultPosition, wxDefaultSize, 0 );
    //m_stColor->Wrap( -1 );
    sbSizerParams->Add( m_stColor, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

    wxString m_cColorChoices[] = { _("Default color"), _("Black"), _("Dark Red"), _("Dark Green"),
    _("Dark Yellow"), _("Dark Blue"), _("Dark Magenta"), _("Dark Cyan"),
      _("Light Gray"), _("Dark Gray"), _("Red"), _("Green"), _("Yellow"), _("Blue"),
      _("Magenta"), _("Cyan"), _("White") };
      int m_cColorNChoices = sizeof( m_cColorChoices ) / sizeof(wxString);
      m_cColor = new wxChoice( m_panelBasic, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_cColorNChoices, m_cColorChoices, 0 );
      m_cColor->SetSelection( 0 );
      sbSizerParams->Add( m_cColor, 1, 0, 5 );

      m_stStyle = new wxStaticText( m_panelBasic, wxID_ANY, _("Style"), wxDefaultPosition, wxDefaultSize, 0 );
      //m_stStyle->Wrap( -1 );
      sbSizerParams->Add( m_stStyle, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

      wxString m_cStyleChoices[] = { _("Default"), _("Solid"), _("Dot"), _("Long dash"), _("Short dash"), _("Dot dash") };
      int m_cStyleNChoices = sizeof( m_cStyleChoices ) / sizeof(wxString);
      m_cStyle = new wxChoice( m_panelBasic, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_cStyleNChoices, m_cStyleChoices, 0 );
      m_cStyle->SetSelection( 0 );
      sbSizerParams->Add( m_cStyle, 1, 0, 5 );

      m_stWidth = new wxStaticText( m_panelBasic, wxID_ANY, _("Width"), wxDefaultPosition, wxDefaultSize, 0 );
      //m_stWidth->Wrap( -1 );
      sbSizerParams->Add( m_stWidth, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );

      wxString m_cWidthChoices[] = { _("Default"), _("1 pixel"), _("2 pixels"), _("3 pixels"),
      _("4 pixels"), _("5 pixels"), _("6 pixels"), _("7 pixels"), _("8 pixels"),
        _("9 pixels"), _("10 pixels") };
        int m_cWidthNChoices = sizeof( m_cWidthChoices ) / sizeof(wxString);
        m_cWidth = new wxChoice( m_panelBasic, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_cWidthNChoices, m_cWidthChoices, 0 );
        m_cWidth->SetSelection( 0 );
        sbSizerParams->Add( m_cWidth, 1, 0, 5 );


        wxStaticBoxSizer* sbSizerStats;
        sbSizerStats = new wxStaticBoxSizer( new wxStaticBox( m_panelBasic, wxID_ANY, _("Statistics") ), wxVERTICAL );
        bSizerBasic->Add( sbSizerStats, 0, wxALL|wxEXPAND, 5 );

        wxBoxSizer* bSizerStats;
        bSizerStats = new wxBoxSizer( wxHORIZONTAL );

        m_stTotDistance = new wxStaticText( m_panelBasic, wxID_ANY, _("Total distance"), wxDefaultPosition, wxDefaultSize, 0 );
        //m_stTotDistance->Wrap( -1 );
        bSizerStats->Add( m_stTotDistance, 0, wxALL, 5 );

        m_tTotDistance = new wxTextCtrl( m_panelBasic, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
        bSizerStats->Add( m_tTotDistance, 1, 0, 5 );

        m_stAvgSpeed = new wxStaticText( m_panelBasic, wxID_ANY, _("Avg. speed"), wxDefaultPosition, wxDefaultSize, 0 );
      //m_stAvgSpeed->Wrap( -1 );
      bSizerStats->Add( m_stAvgSpeed, 0, wxALL, 5 );

      m_tAvgSpeed = new wxTextCtrl( m_panelBasic, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
      bSizerStats->Add( m_tAvgSpeed, 1, 0, 5 );

      m_stTimeEnroute = new wxStaticText( m_panelBasic, wxID_ANY, _("Time enroute"), wxDefaultPosition, wxDefaultSize, 0 );
      //m_stTimeEnroute->Wrap( -1 );
      bSizerStats->Add( m_stTimeEnroute, 0, wxALL, 5 );

      m_tTimeEnroute = new wxTextCtrl( m_panelBasic, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
      bSizerStats->Add( m_tTimeEnroute, 2, 0, 5 );


      sbSizerStats->Add( bSizerStats, 0, wxEXPAND, 5 );




      wxStaticBoxSizer* sbSizerPoints;
      sbSizerPoints = new wxStaticBoxSizer( new wxStaticBox( m_panel0, wxID_ANY, _("Recorded points") ), wxVERTICAL );
      bSizer1->Add( sbSizerPoints, 1, wxALL|wxEXPAND, 5 );

      wxBoxSizer* bSizerShowTime;
      bSizerShowTime = new wxBoxSizer( wxHORIZONTAL );

      m_stShowTime = new wxStaticText( m_panelBasic, wxID_ANY, _("Time shown as"), wxDefaultPosition, wxDefaultSize, 0 );
      //m_stShowTime->Wrap( -1 );
      bSizerShowTime->Add( m_stShowTime, 0, wxALL, 5 );

      m_rbShowTimeUTC = new wxRadioButton( m_panelBasic, wxID_ANY, _("UTC"), wxDefaultPosition, wxDefaultSize, 0 );
      bSizerShowTime->Add( m_rbShowTimeUTC, 0, 0, 5 );

      m_rbShowTimePC = new wxRadioButton( m_panelBasic, wxID_ANY, _("Local @ PC"), wxDefaultPosition, wxDefaultSize, 0 );
      bSizerShowTime->Add( m_rbShowTimePC, 0, 0, 5 );

      m_rbShowTimeLocal = new wxRadioButton( m_panelBasic, wxID_ANY, _("LMT @ Track Start"), wxDefaultPosition, wxDefaultSize, 0 );
      bSizerShowTime->Add( m_rbShowTimeLocal, 0, 0, 5 );

      m_rbShowTimePC->SetValue(true);

      sbSizerPoints->Add( bSizerShowTime, 0, wxEXPAND, 5 );

      long flags = wxLC_REPORT | wxLC_HRULES | wxLC_VRULES | wxLC_EDIT_LABELS;
#ifndef __WXQT__                // Does not support Virtual list boxes....
      flags |= wxLC_VIRTUAL;
#endif

      m_lcPoints = new OCPNTrackListCtrl( m_panel0, wxID_ANY, wxDefaultPosition, wxDefaultSize, flags);


      sbSizerPoints->Add( m_lcPoints, 1, wxALL|wxEXPAND, 5 );

      int dx = GetCharWidth();

      m_lcPoints->InsertColumn( 0, _("Leg"), wxLIST_FORMAT_LEFT, dx * 6/*45*/ );
      m_lcPoints->InsertColumn( 1, _("Distance"), wxLIST_FORMAT_LEFT, dx * 10/*70*/ );
      m_lcPoints->InsertColumn( 2, _("Bearing"), wxLIST_FORMAT_LEFT, dx * 8/*70*/ );
      m_lcPoints->InsertColumn( 3, _("Latitude"), wxLIST_FORMAT_LEFT, dx * 11/*85*/ );
      m_lcPoints->InsertColumn( 4, _("Longitude"), wxLIST_FORMAT_LEFT, dx * 11/*90*/ );
      m_lcPoints->InsertColumn( 5, _("Timestamp"), wxLIST_FORMAT_LEFT, dx * 14/*135*/ );
      m_lcPoints->InsertColumn( 6, _("Speed"), wxLIST_FORMAT_CENTER, dx * 8/*100*/ );

      m_lcPoints->SetMinSize(wxSize(-1, 50) );

#ifdef __OCPN__ANDROID__
      m_lcPoints->GetHandle()->setStyleSheet( getQtStyleSheet());
#endif


//       m_lcPoints = new OCPNTrackListCtrl( m_panel0, wxID_ANY, wxDefaultPosition, wxDefaultSize,
//                                           wxLC_REPORT | wxLC_HRULES | wxLC_VRULES | wxLC_EDIT_LABELS | wxLC_VIRTUAL );
//
//       m_lcPoints->Hide();
//       m_rbShowTimeUTC = NULL;
//       m_rbShowTimePC = NULL;
//       m_rbShowTimeLocal = NULL;




      m_panelAdvanced = new wxScrolledWindow( m_notebook1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );

      wxBoxSizer* bSizerAdvanced;
      bSizerAdvanced = new wxBoxSizer( wxVERTICAL );

      m_stDescription = new wxStaticText( m_panelAdvanced, wxID_ANY, _("Description"), wxDefaultPosition, wxDefaultSize, 0 );
      //m_stDescription->Wrap( -1 );
      bSizerAdvanced->Add( m_stDescription, 0, wxALL, 5 );

      m_tDescription = new wxTextCtrl( m_panelAdvanced, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE );
      bSizerAdvanced->Add( m_tDescription, 1, wxALL|wxEXPAND, 5 );

      sbSizerLinks = new wxStaticBoxSizer( new wxStaticBox( m_panelAdvanced, wxID_ANY, _("Links") ), wxVERTICAL );

      m_scrolledWindowLinks = new wxScrolledWindow( m_panelAdvanced, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHSCROLL|wxVSCROLL );
      m_scrolledWindowLinks->SetScrollRate( 5, 5 );
      bSizerLinks = new wxBoxSizer( wxVERTICAL );

      m_hyperlink1 = new wxHyperlinkCtrl( m_scrolledWindowLinks, wxID_ANY, _("wxFB Website"), wxT("http://www.wxformbuilder.org"), wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
      m_menuLink = new wxMenu();
      wxMenuItem* m_menuItemEdit;
      m_menuItemEdit = new wxMenuItem( m_menuLink, wxID_ANY, wxString( _("Edit") ) , wxEmptyString, wxITEM_NORMAL );
      m_menuLink->Append( m_menuItemEdit );

      wxMenuItem* m_menuItemAdd;
      m_menuItemAdd = new wxMenuItem( m_menuLink, wxID_ANY, wxString( _("Add new") ) , wxEmptyString, wxITEM_NORMAL );
      m_menuLink->Append( m_menuItemAdd );

      wxMenuItem* m_menuItemDelete;
      m_menuItemDelete = new wxMenuItem( m_menuLink, wxID_ANY, wxString( _("Delete") ) , wxEmptyString, wxITEM_NORMAL );
      m_menuLink->Append( m_menuItemDelete );

      m_hyperlink1->Connect( wxEVT_RIGHT_DOWN, wxMouseEventHandler( TrackPropDlg::m_hyperlink1OnContextMenu ), NULL, this );

      bSizerLinks->Add( m_hyperlink1, 0, wxALL, 5 );


      m_scrolledWindowLinks->SetSizer( bSizerLinks );
      m_scrolledWindowLinks->Layout();
      bSizerLinks->Fit( m_scrolledWindowLinks );
      sbSizerLinks->Add( m_scrolledWindowLinks, 1, wxEXPAND | wxALL, 5 );

      wxBoxSizer* bSizer27;
      bSizer27 = new wxBoxSizer( wxHORIZONTAL );

      m_buttonAddLink = new wxButton( m_panelAdvanced, wxID_ANY, _("Add"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT );
      bSizer27->Add( m_buttonAddLink, 0, wxALL, 5 );

      m_toggleBtnEdit = new wxToggleButton( m_panelAdvanced, wxID_ANY, _("Edit"), wxDefaultPosition, wxDefaultSize, 0 );
      bSizer27->Add( m_toggleBtnEdit, 0, wxALL, 5 );

      m_staticTextEditEnabled = new wxStaticText( m_panelAdvanced, wxID_ANY, _("Links are opened in the default browser."), wxDefaultPosition, wxDefaultSize, 0 );
      //m_staticTextEditEnabled->Wrap( -1 );
      bSizer27->Add( m_staticTextEditEnabled, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5 );


      sbSizerLinks->Add( bSizer27, 0, wxEXPAND, 5 );


      bSizerAdvanced->Add( sbSizerLinks, 1, wxEXPAND, 5 );

      m_panelAdvanced->SetSizer( bSizerAdvanced );
      m_notebook1->AddPage( m_panelAdvanced, _("Advanced"), false );



      //  Buttons, etc...

      wxBoxSizer* itemBoxSizerBottom = new wxBoxSizer( wxVERTICAL );
      bSizerMain->Add( itemBoxSizerBottom, 0, wxALIGN_LEFT | wxALL | wxEXPAND, 5 );

      //wxBoxSizer* itemBoxSizerAux = new wxBoxSizer( wxHORIZONTAL );

      int n_col = 5;

      wxFlexGridSizer* itemBoxSizerAux = new wxFlexGridSizer( 0, n_col, 0, 0 );
      itemBoxSizerAux->SetFlexibleDirection( wxBOTH );
      itemBoxSizerAux->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );



      itemBoxSizerBottom->Add( itemBoxSizerAux, 1, wxALIGN_LEFT | wxALL, 5 );

#ifndef __OCPN__ANDROID__

      m_sdbBtmBtnsSizerPrint = new wxButton( this, wxID_ANY, _("Print"),
                                                                wxDefaultPosition, wxDefaultSize, 0 );
      itemBoxSizerAux->Add( m_sdbBtmBtnsSizerPrint, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 5 );
      m_sdbBtmBtnsSizerPrint->Enable( true );
#else
      m_sdbBtmBtnsSizerPrint = NULL;
#endif

      m_sdbBtmBtnsSizerSplit = new wxButton( this, wxID_ANY, _("Split"),
      wxDefaultPosition, wxDefaultSize, 0 );
      itemBoxSizerAux->Add( m_sdbBtmBtnsSizerSplit, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 5 );
      m_sdbBtmBtnsSizerSplit->Enable( false );

      m_sdbBtmBtnsSizerExtend = new wxButton( this, wxID_ANY, _("Extend Route"),
                                     wxDefaultPosition, wxDefaultSize, 0 );
      itemBoxSizerAux->Add( m_sdbBtmBtnsSizerExtend, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 5 );

      m_sdbBtmBtnsSizerToRoute = new wxButton( this, wxID_ANY, _("To route"),
                                              wxDefaultPosition, wxDefaultSize, 0 );
      itemBoxSizerAux->Add( m_sdbBtmBtnsSizerToRoute, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 5 );

      m_sdbBtmBtnsSizerExport = new wxButton( this, wxID_ANY, _("Export"),
                                                                           wxDefaultPosition, wxDefaultSize, 0 );
      itemBoxSizerAux->Add( m_sdbBtmBtnsSizerExport, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL, 5 );

      wxBoxSizer* itemBoxSizer16 = new wxBoxSizer( wxHORIZONTAL );
      itemBoxSizerBottom->Add( itemBoxSizer16, 0, wxALIGN_RIGHT | wxALL, 5 );

      m_sdbBtmBtnsSizerCancel = new wxButton( this, wxID_CANCEL, _("Cancel"), wxDefaultPosition,
      wxDefaultSize, 0 );
      itemBoxSizer16->Add( m_sdbBtmBtnsSizerCancel, 0, wxALIGN_RIGHT | wxALIGN_BOTTOM | wxALL, 5 );

      m_sdbBtmBtnsSizerOK = new wxButton( this, wxID_OK, _("OK"), wxDefaultPosition,
      wxDefaultSize, 0 );
      itemBoxSizer16->Add( m_sdbBtmBtnsSizerOK, 0, wxALIGN_RIGHT | wxALIGN_BOTTOM | wxALL, 5 );
      m_sdbBtmBtnsSizerOK->SetDefault();


      //Make it look nice and add the needed non-standard buttons
      int w1, w2, h;
      ((wxWindowBase *)m_stName)->GetSize( &w1, &h );
      ((wxWindowBase *)m_stFrom)->GetSize( &w2, &h );
      ((wxWindowBase *)m_stName)->SetMinSize( wxSize(wxMax(w1, w2), h) );
      ((wxWindowBase *)m_stFrom)->SetMinSize( wxSize(wxMax(w1, w2), h) );

#if 0
      Connect( m_menuItemDelete->GetId(), wxEVT_COMMAND_MENU_SELECTED,
               wxCommandEventHandler( TrackPropDlg::OnDeleteLink ) );
      Connect( m_menuItemEdit->GetId(), wxEVT_COMMAND_MENU_SELECTED,
               wxCommandEventHandler( TrackPropDlg::OnEditLink ) );
      Connect( m_menuItemAdd->GetId(), wxEVT_COMMAND_MENU_SELECTED,
               wxCommandEventHandler( TrackPropDlg::OnAddLink ) );
#endif

      m_panelBasic->SetScrollRate(5, 5);
      m_panelAdvanced->SetScrollRate(5, 5);

}

void TrackPropDlg::SetTrackAndUpdate( Track *pt )
{
    m_pTrack = pt;

    m_lcPoints->DeleteAllItems();

    if( m_pMyLinkList )
        delete m_pMyLinkList;
    m_pMyLinkList = new HyperlinkList();

    int NbrOfLinks = m_pTrack->m_HyperlinkList->GetCount();
    if( NbrOfLinks > 0 ) {
        wxHyperlinkListNode *linknode = m_pTrack->m_HyperlinkList->GetFirst();
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

    InitializeList();
    UpdateProperties();
    
    RecalculateSize();
    
}

void TrackPropDlg::InitializeList()
{
    if( NULL == m_pTrack )
        return;

    m_lcPoints->m_pTrack = m_pTrack;

    if(m_pTrack->GetnPoints()){
        TrackPoint *prp = m_pTrack->GetPoint(0);
        if(prp)
            m_lcPoints->m_LMT_Offset = long(( prp->m_lon ) * 3600. / 15. );  // estimated
        else
            m_lcPoints->m_LMT_Offset = 0;
    }

    if( m_lcPoints->IsVirtual() )
        m_lcPoints->SetItemCount( m_pTrack->GetnPoints() );

    else{
        for(int in = 0; in < m_pTrack->GetnPoints(); in++) {
            wxListItem item;
            item.SetId(in);

            m_lcPoints->InsertItem( item );
            for(int j=0 ; j < 7 ; j++){
                item.SetColumn(j);
                item.SetText(m_lcPoints->OnGetItemText( in, j) );
                m_lcPoints->SetItem(item);
            }
            in++;
        }
    }

}



bool TrackPropDlg::UpdateProperties()
{
    if( NULL == m_pTrack )
        return false;

    ::wxBeginBusyCursor();

    if(m_scrolledWindowLinks){
        wxWindowList kids = m_scrolledWindowLinks->GetChildren();
        for( unsigned int i = 0; i < kids.GetCount(); i++ ) {
            wxWindowListNode *node = kids.Item( i );
            wxWindow *win = node->GetData();

            if( win->IsKindOf( CLASSINFO(wxHyperlinkCtrl) ) ) {
                ( (wxHyperlinkCtrl*) win )->Disconnect( wxEVT_COMMAND_HYPERLINK,
                        wxHyperlinkEventHandler( TrackPropDlg::OnHyperLinkClick ) );
                ( (wxHyperlinkCtrl*) win )->Disconnect( wxEVT_RIGHT_DOWN,
                        wxMouseEventHandler( TrackPropDlg::m_hyperlinkContextMenu ) );
                win->Destroy();
            }
        }
///        m_scrolledWindowLinks->DestroyChildren();
        int NbrOfLinks = m_pTrack->m_HyperlinkList->GetCount();
            HyperlinkList *hyperlinklist = m_pTrack->m_HyperlinkList;
    //            int len = 0;
            if( NbrOfLinks > 0 ) {
            wxHyperlinkListNode *linknode = hyperlinklist->GetFirst();
            while( linknode ) {
                Hyperlink *link = linknode->GetData();
                wxString Link = link->Link;
                wxString Descr = link->DescrText;

                wxHyperlinkCtrl* ctrl = new wxHyperlinkCtrl( m_scrolledWindowLinks, wxID_ANY, Descr,
                        Link, wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
                ctrl->Connect( wxEVT_COMMAND_HYPERLINK,
                        wxHyperlinkEventHandler( TrackPropDlg::OnHyperLinkClick ), NULL, this );
                if( !m_pTrack->m_bIsInLayer ) ctrl->Connect( wxEVT_RIGHT_DOWN,
                        wxMouseEventHandler( TrackPropDlg::m_hyperlinkContextMenu ), NULL, this );

                bSizerLinks->Add( ctrl, 0, wxALL, 5 );

                linknode = linknode->GetNext();
            }
        }
        bSizerLinks->Fit( m_scrolledWindowLinks );
    }

    m_tName->SetValue( m_pTrack->m_TrackNameString );
    m_tFrom->SetValue( m_pTrack->m_TrackStartString );
    m_tTo->SetValue( m_pTrack->m_TrackEndString );
    if(m_tDescription)m_tDescription->SetValue( m_pTrack->m_TrackDescription );

    m_tTotDistance->SetValue( _T("") );
    m_tTimeEnroute->SetValue( _T("") );

    m_sdbBtmBtnsSizerSplit->Enable( false );
    m_sdbBtmBtnsSizerExtend->Enable( false );

    // Calculate AVG speed if we are showing a track and total time
    TrackPoint *last_point = m_pTrack->GetLastPoint();
    TrackPoint *first_point = m_pTrack->GetPoint( 0 );
    double total_seconds = 0.;

    wxString speed( _T("--") );

    if(last_point && first_point){
        if( last_point->GetCreateTime().IsValid() && first_point->GetCreateTime().IsValid() ) {
            total_seconds =
                    last_point->GetCreateTime().Subtract( first_point->GetCreateTime() ).GetSeconds().ToDouble();
            if( total_seconds != 0. ) {
                m_avgspeed = m_pTrack->Length() / total_seconds * 3600;
            } else {
                m_avgspeed = 0;
            }
            speed.Printf( _T("%5.2f"), toUsrSpeed( m_avgspeed ) );
        }
    }

    m_tAvgSpeed->SetValue( speed );

    //  Total length
    wxString slen;
    slen.Printf( wxT("%5.2f ") + getUsrDistanceUnit(), toUsrDistance( m_pTrack->Length() ) );

    m_tTotDistance->SetValue( slen );

    //  Time
    wxString time_form;
    wxTimeSpan time( 0, 0, (int) total_seconds, 0 );
    if( total_seconds > 3600. * 24. )
        time_form = time.Format( _("%D Days, %H:%M") );
    else
        if( total_seconds > 0. )
            time_form = time.Format( _("%H:%M") );
        else
            time_form = _T("--");
    m_tTimeEnroute->SetValue( time_form );

    m_cbShow->SetValue( m_pTrack->IsVisible() );

    if( m_pTrack->m_Colour == wxEmptyString )
        m_cColor->Select( 0 );
    else {
        for( unsigned int i = 0; i < sizeof( ::GpxxColorNames ) / sizeof(wxString); i++ ) {
            if( m_pTrack->m_Colour == ::GpxxColorNames[i] ) {
                m_cColor->Select( i + 1 );
                break;
            }
        }
    }

    for( unsigned int i = 0; i < sizeof( ::StyleValues ) / sizeof(int); i++ ) {
        if( m_pTrack->m_style == ::StyleValues[i] ) {
            m_cStyle->Select( i );
            break;
        }
    }

    for( unsigned int i = 0; i < sizeof( ::WidthValues ) / sizeof(int); i++ ) {
        if( m_pTrack->m_width == ::WidthValues[i] ) {
            m_cWidth->Select( i );
            break;
        }
    }

    if( m_pTrack->m_bIsInLayer )
    {
        m_tName->SetEditable( false );
        m_tFrom->SetEditable( false );
        m_tTo->SetEditable( false );
        if(m_tDescription)m_tDescription->SetEditable( false );
        m_cbShow->Enable( false );
        m_cColor->Enable( false );
        m_cStyle->Enable( false );
        m_cWidth->Enable( false );
        m_sdbBtmBtnsSizerExtend->Enable( false );
        m_sdbBtmBtnsSizerSplit->Enable( false );
        SetTitle( wxString::Format( _("Track Properties, Layer: %d"), m_pTrack->m_LayerID ) );
    } else {
        m_tName->SetEditable( true );
        m_tFrom->SetEditable( true );
        m_tTo->SetEditable( true );
        if(m_tDescription)m_tDescription->SetEditable( true );
        m_cbShow->Enable( true );
        m_cColor->Enable( true );
        m_cStyle->Enable( true );
        m_cWidth->Enable( true );

        m_sdbBtmBtnsSizerExtend->Enable( IsThisTrackExtendable() );
        //m_sdbBtmBtnsSizerSplit->Enable( false );
        SetTitle( _("Track Properties") );
    }

    ::wxEndBusyCursor();

    return true;
}

bool TrackPropDlg::IsThisTrackExtendable()
{
    m_pExtendTrack = NULL;
    m_pExtendPoint = NULL;
    if( m_pTrack == g_pActiveTrack || m_pTrack->m_bIsInLayer ) return false;

    TrackPoint *pLastPoint = m_pTrack->GetPoint( 0 );
    if( !pLastPoint->GetCreateTime().IsValid() ) return false;

    wxTrackListNode *track_node = pTrackList->GetFirst();
    while( track_node ) {
        Track *ptrack = track_node->GetData();
        if( ptrack->IsVisible() && ( ptrack->m_GUID != m_pTrack->m_GUID ) ) {
            TrackPoint *track_node = ptrack->GetLastPoint();
            if( track_node ){
                if( track_node->GetCreateTime().IsValid() ) {
                    if( track_node->GetCreateTime() <= pLastPoint->GetCreateTime() ) {
                        if( !m_pExtendPoint || track_node->GetCreateTime() > m_pExtendPoint->GetCreateTime() ) {
                            m_pExtendPoint = track_node;
                            m_pExtendTrack = ptrack;
                        }
                    }
                }
            }
        }
        track_node = track_node->GetNext();                         // next track
    }
    if( m_pExtendTrack ) return ( !m_pExtendTrack->m_bIsInLayer );
    else
        return false;
}

void TrackPropDlg::OnExtendBtnClick( wxCommandEvent& event )
{
    TrackPoint *pLastPoint = m_pTrack->GetPoint( 0 );

    if( IsThisTrackExtendable() ) {
        int begin = 1;
        if( pLastPoint->GetCreateTime() == m_pExtendPoint->GetCreateTime() ) begin = 2;
        pSelect->DeleteAllSelectableTrackSegments( m_pExtendTrack );
        m_pExtendTrack->Clone( m_pTrack, begin, m_pTrack->GetnPoints(), _("_plus") );
        pSelect->AddAllSelectableTrackSegments( m_pExtendTrack );
        pSelect->DeleteAllSelectableTrackSegments( m_pTrack );
        g_pRouteMan->DeleteTrack( m_pTrack );

        SetTrackAndUpdate( m_pExtendTrack );
        UpdateProperties();

        if( pRouteManagerDialog && pRouteManagerDialog->IsShown() )
            pRouteManagerDialog->UpdateTrkListCtrl();
    }
}

void TrackPropDlg::OnSplitBtnClick( wxCommandEvent& event )
{
    m_sdbBtmBtnsSizerSplit->Enable( false );

    if( m_pTrack->m_bIsInLayer )
        return;

    if( ( m_nSelected > 1 ) && ( m_nSelected < m_pTrack->GetnPoints() ) ) {
        Track *pHead = new Track();
        Track *pTail = new Track();
        pHead->Clone( m_pTrack, 1, m_nSelected, _("_A") );
        pTail->Clone( m_pTrack, m_nSelected, m_pTrack->GetnPoints(), _("_B") );
        pTrackList->Append( pHead );
        pConfig->AddNewTrack( pHead );
//        pHead->RebuildGUIDList();

        pTrackList->Append( pTail );
        pConfig->AddNewTrack( pTail );
//        pTail->RebuildGUIDList();

        pConfig->DeleteConfigTrack( m_pTrack );

        pSelect->DeleteAllSelectableTrackSegments( m_pTrack );
        g_pRouteMan->DeleteTrack( m_pTrack );
        pSelect->AddAllSelectableTrackSegments( pTail );
        pSelect->AddAllSelectableTrackSegments( pHead );

        SetTrackAndUpdate( pTail );
        UpdateProperties();

        if( pRouteManagerDialog && pRouteManagerDialog->IsShown() )
            pRouteManagerDialog->UpdateTrkListCtrl();
    }
}

void TrackPropDlg::OnTrackPropCopyTxtClick( wxCommandEvent& event )
{
    wxString tab("\t", wxConvUTF8);
    wxString eol("\n", wxConvUTF8);
    wxString csvString;

    csvString << this->GetTitle() << eol
            << _("Name") << tab << m_pTrack->m_TrackNameString << eol
            << _("Depart From") << tab << m_pTrack->m_TrackStartString << eol
            << _("Destination") << tab << m_pTrack->m_TrackEndString << eol
            << _("Total Distance") << tab << m_tTotDistance->GetValue() << eol
            << _("Speed") << tab << m_tAvgSpeed->GetValue() << eol
            << _("Departure Time (m/d/y h:m)") << tab << m_pTrack->GetPoint(1)->GetCreateTime().Format() << eol
            << _("Time Enroute") << tab << m_tTimeEnroute->GetValue() << eol << eol;

    int noCols;
    int noRows;
    noCols = m_lcPoints->GetColumnCount();
    noRows = m_lcPoints->GetItemCount();
    wxListItem item;
    item.SetMask( wxLIST_MASK_TEXT );

    for( int i = 0; i < noCols; i++ ) {
        m_lcPoints->GetColumn( i, item );
        csvString << item.GetText() << tab;
    }
    csvString << eol;

    for( int j = 0; j < noRows; j++ ) {
        item.SetId( j );
        for( int i = 0; i < noCols; i++ ) {
            item.SetColumn( i );
            m_lcPoints->GetItem( item );
            csvString << item.GetText() << tab;
        }
        csvString << eol;
    }

    if( wxTheClipboard->Open() ) {
        wxTextDataObject* data = new wxTextDataObject;
        data->SetText( csvString );
        wxTheClipboard->SetData( data );
        wxTheClipboard->Close();
    }
}

void TrackPropDlg::OnPrintBtnClick( wxCommandEvent& event )
{
//    RoutePrintSelection dlg( this, m_pTrack );
//    dlg.ShowModal();
}

void TrackPropDlg::OnTrackPropRightClick( wxListEvent &event )
{
    wxMenu menu;

    wxMenuItem* copyItem = menu.Append( ID_RCLK_MENU_COPY_TEXT, _("&Copy all as text") );

    PopupMenu( &menu );
}

void TrackPropDlg::OnTrackPropListClick( wxListEvent& event )
{
    long itemno = -1;
    m_nSelected = 0;

    int selected_no;
    itemno = m_lcPoints->GetNextItem( itemno, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( itemno == -1 )
        selected_no = 0;
    else
        selected_no = itemno;

//    m_pRoute->ClearHighlights();

    if( itemno >= 0 ) {
        TrackPoint *prp = m_pTrack->GetPoint(itemno);
        if( prp ) {
//            prp->m_bPtIsSelected = true;                // highlight the trackpoint

            if( !( m_pTrack->m_bIsInLayer ) && !( m_pTrack == g_pActiveTrack ) ) {
                m_nSelected = selected_no + 1;
                m_sdbBtmBtnsSizerSplit->Enable( true );
            }
            gFrame->JumpToPosition( prp->m_lat, prp->m_lon, cc1->GetVPScale() );
        }
    }
    if( selected_no == 0 || selected_no == m_pTrack->GetnPoints() - 1)
        m_sdbBtmBtnsSizerSplit->Enable( false );
}

void TrackPropDlg::OnTrackPropMenuSelected( wxCommandEvent& event )
{
    switch( event.GetId() ) {
        case ID_RCLK_MENU_COPY_TEXT:
            OnTrackPropCopyTxtClick( event );
            break;
    }
}

void TrackPropDlg::OnToRouteBtnClick( wxCommandEvent& event )
{
    pRouteManagerDialog->TrackToRoute( m_pTrack );
    if( NULL != pRouteManagerDialog && pRouteManagerDialog->IsVisible() )
        pRouteManagerDialog->UpdateRouteListCtrl();
}

void TrackPropDlg::OnExportBtnClick( wxCommandEvent& event )
{
    wxString suggested_name = _("track");
    TrackList list;
    list.Append( m_pTrack );
    if( m_pTrack->m_TrackNameString != wxEmptyString )
        suggested_name = m_pTrack->m_TrackNameString;
    pConfig->ExportGPXTracks( this, &list, suggested_name );
}

void TrackPropDlg::m_hyperlinkContextMenu( wxMouseEvent &event )
{
    m_pEditedLink = (wxHyperlinkCtrl*) event.GetEventObject();
    m_scrolledWindowLinks->PopupMenu( m_menuLink,
            m_pEditedLink->GetPosition().x + event.GetPosition().x,
            m_pEditedLink->GetPosition().y + event.GetPosition().y );

}

void TrackPropDlg::OnDeleteLink( wxCommandEvent& event )
{
    wxHyperlinkListNode* nodeToDelete = NULL;
    wxString findurl = m_pEditedLink->GetURL();
    wxString findlabel = m_pEditedLink->GetLabel();

    wxWindowList kids = m_scrolledWindowLinks->GetChildren();
    for( unsigned int i = 0; i < kids.GetCount(); i++ ) {
        wxWindowListNode *node = kids.Item( i );
        wxWindow *win = node->GetData();

        if( win->IsKindOf( CLASSINFO(wxHyperlinkCtrl) ) ) {
            ( (wxHyperlinkCtrl*) win )->Disconnect( wxEVT_COMMAND_HYPERLINK,
                                                    wxHyperlinkEventHandler( TrackPropDlg::OnHyperLinkClick ) );
            ( (wxHyperlinkCtrl*) win )->Disconnect( wxEVT_RIGHT_DOWN,
                                                    wxMouseEventHandler( TrackPropDlg::m_hyperlinkContextMenu ) );
            win->Destroy();
        }
    }

///    m_scrolledWindowLinks->DestroyChildren();
    int NbrOfLinks = m_pTrack->m_HyperlinkList->GetCount();
    HyperlinkList *hyperlinklist = m_pTrack->m_HyperlinkList;
//      int len = 0;
    if( NbrOfLinks > 0 ) {
        wxHyperlinkListNode *linknode = hyperlinklist->GetFirst();
        while( linknode ) {
            Hyperlink *link = linknode->GetData();
            wxString Link = link->Link;
            wxString Descr = link->DescrText;
            if( Link == findurl
                    && ( Descr == findlabel || ( Link == findlabel && Descr == wxEmptyString ) ) ) nodeToDelete =
                    linknode;
            else {
                wxHyperlinkCtrl* ctrl = new wxHyperlinkCtrl( m_scrolledWindowLinks, wxID_ANY, Descr,
                        Link, wxDefaultPosition, wxDefaultSize, wxHL_DEFAULT_STYLE );
                ctrl->Connect( wxEVT_COMMAND_HYPERLINK,
                        wxHyperlinkEventHandler( TrackPropDlg::OnHyperLinkClick ), NULL, this );
                ctrl->Connect( wxEVT_RIGHT_DOWN,
                        wxMouseEventHandler( TrackPropDlg::m_hyperlinkContextMenu ), NULL, this );

                bSizerLinks->Add( ctrl, 0, wxALL, 5 );
            }
            linknode = linknode->GetNext();
        }
    }
    if( nodeToDelete ) hyperlinklist->DeleteNode( nodeToDelete );
    m_scrolledWindowLinks->InvalidateBestSize();
    m_scrolledWindowLinks->Layout();
    sbSizerLinks->Layout();
    event.Skip();
}

void TrackPropDlg::OnEditLink( wxCommandEvent& event )
{
    wxString findurl = m_pEditedLink->GetURL();
    wxString findlabel = m_pEditedLink->GetLabel();
    m_pLinkProp->m_textCtrlLinkDescription->SetValue( findlabel );
    m_pLinkProp->m_textCtrlLinkUrl->SetValue( findurl );
    if( m_pLinkProp->ShowModal() == wxID_OK ) {
        int NbrOfLinks = m_pTrack->m_HyperlinkList->GetCount();
        HyperlinkList *hyperlinklist = m_pTrack->m_HyperlinkList;
//            int len = 0;
        if( NbrOfLinks > 0 ) {
            wxHyperlinkListNode *linknode = hyperlinklist->GetFirst();
            while( linknode ) {
                Hyperlink *link = linknode->GetData();
                wxString Link = link->Link;
                wxString Descr = link->DescrText;
                if( Link == findurl
                        && ( Descr == findlabel || ( Link == findlabel && Descr == wxEmptyString ) ) ) {
                    link->Link = m_pLinkProp->m_textCtrlLinkUrl->GetValue();
                    link->DescrText = m_pLinkProp->m_textCtrlLinkDescription->GetValue();
                    wxHyperlinkCtrl* h =
                            (wxHyperlinkCtrl*) m_scrolledWindowLinks->FindWindowByLabel(
                                    findlabel );
                    if( h ) {
                        h->SetLabel( m_pLinkProp->m_textCtrlLinkDescription->GetValue() );
                        h->SetURL( m_pLinkProp->m_textCtrlLinkUrl->GetValue() );
                    }
                }
                linknode = linknode->GetNext();
            }
        }

        m_scrolledWindowLinks->InvalidateBestSize();
        m_scrolledWindowLinks->Layout();
        sbSizerLinks->Layout();
        event.Skip();
    }
    event.Skip();
}

void TrackPropDlg::OnAddLink( wxCommandEvent& event )
{
    m_pLinkProp->m_textCtrlLinkDescription->SetValue( wxEmptyString );
    m_pLinkProp->m_textCtrlLinkUrl->SetValue( wxEmptyString );
    if( m_pLinkProp->ShowModal() == wxID_OK ) {
        wxString desc = m_pLinkProp->m_textCtrlLinkDescription->GetValue();
        if( desc == wxEmptyString ) desc = m_pLinkProp->m_textCtrlLinkUrl->GetValue();
        wxHyperlinkCtrl* ctrl = new wxHyperlinkCtrl( m_scrolledWindowLinks, wxID_ANY, desc,
                m_pLinkProp->m_textCtrlLinkUrl->GetValue(), wxDefaultPosition, wxDefaultSize,
                wxHL_DEFAULT_STYLE );
        ctrl->Connect( wxEVT_COMMAND_HYPERLINK,
                wxHyperlinkEventHandler( TrackPropDlg::OnHyperLinkClick ), NULL, this );
        ctrl->Connect( wxEVT_RIGHT_DOWN,
                wxMouseEventHandler( TrackPropDlg::m_hyperlinkContextMenu ), NULL, this );

        bSizerLinks->Add( ctrl, 0, wxALL, 5 );
        bSizerLinks->Fit( m_scrolledWindowLinks );
        this->Fit();

        Hyperlink* h = new Hyperlink();
        h->DescrText = m_pLinkProp->m_textCtrlLinkDescription->GetValue();
        h->Link = m_pLinkProp->m_textCtrlLinkUrl->GetValue();
        h->LType = wxEmptyString;
        m_pTrack->m_HyperlinkList->Append( h );
    }

    sbSizerLinks->Layout();

    event.Skip();
}

void TrackPropDlg::OnEditLinkToggle( wxCommandEvent& event )
{
    if( m_toggleBtnEdit->GetValue() ) m_staticTextEditEnabled->SetLabel(
            _("Links are opened for editing.") );
    else
        m_staticTextEditEnabled->SetLabel( _("Links are opened in the default browser.") );
    event.Skip();
}

void TrackPropDlg::OnHyperLinkClick( wxHyperlinkEvent &event )
{
    if( m_toggleBtnEdit->GetValue() ) {
        m_pEditedLink = (wxHyperlinkCtrl*) event.GetEventObject();
        OnEditLink( event );
        event.Skip( false );
        return;
    }
    //    Windows has trouble handling local file URLs with embedded anchor points, e.g file://testfile.html#point1
    //    The trouble is with the wxLaunchDefaultBrowser with verb "open"
    //    Workaround is to probe the registry to get the default browser, and open directly
    //
    //    But, we will do this only if the URL contains the anchor point charater '#'
    //    What a hack......

#ifdef __WXMSW__

    wxString cc = event.GetURL();
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
    } else
        event.Skip();
#else
    wxString url = event.GetURL();
    url.Replace(_T(" "), _T("%20") );
    ::wxLaunchDefaultBrowser(url);
//    event.Skip();
#endif
}

void TrackPropDlg::OnShowTimeTZ ( wxCommandEvent &event )
{
    if (m_rbShowTimeUTC && m_rbShowTimeUTC->GetValue())
        m_lcPoints->m_tz_selection = UTCINPUT;
    else if (m_rbShowTimePC && m_rbShowTimePC->GetValue())
        m_lcPoints->m_tz_selection = LTINPUT;
    else
        m_lcPoints->m_tz_selection = LMTINPUT;
    m_lcPoints->DeleteAllItems();
    InitializeList();
}

bool TrackPropDlg::SaveChanges( void )
{
    if( m_pTrack && !m_pTrack->m_bIsInLayer ) {
        //  Get User input Text Fields
        m_pTrack->m_TrackNameString = m_tName->GetValue();
        m_pTrack->m_TrackStartString = m_tFrom->GetValue();
        m_pTrack->m_TrackEndString = m_tTo->GetValue();
        if(m_tDescription) m_pTrack->m_TrackDescription = m_tDescription->GetValue();
        m_pTrack->SetVisible( m_cbShow->GetValue() );
        if( m_cColor->GetSelection() == 0 )
            m_pTrack->m_Colour = wxEmptyString;
        else
            m_pTrack->m_Colour = ::GpxxColorNames[m_cColor->GetSelection() - 1];
        m_pTrack->m_style = (wxPenStyle)::StyleValues[m_cStyle->GetSelection()];
        m_pTrack->m_width = ::WidthValues[m_cWidth->GetSelection()];

        pConfig->UpdateTrack( m_pTrack );
        pConfig->UpdateSettings();
    }

    if( m_pTrack && m_pTrack->IsRunning() )
    {
        wxJSONValue v;
        v[_T("Name")] =  m_pTrack->m_TrackNameString;
        v[_T("GUID")] =  m_pTrack->m_GUID;
        wxString msg_id( _T("OCPN_TRK_ACTIVATED") );
        g_pi_manager->SendJSONMessageToAllPlugins( msg_id, v );
    }

    return true;
}

void TrackPropDlg::OnOKBtnClick( wxCommandEvent& event )
{
    //    Look in the track list to be sure the track is still available
    //    (May have been deleted by RouteManagerDialog...)

    bool b_found_track = false;
    wxTrackListNode *node = pTrackList->GetFirst();
    while( node ) {
        Track *ptrack = node->GetData();

        if( ptrack == m_pTrack ) {
            b_found_track = true;
            break;
        }
        node = node->GetNext();
    }

    if( b_found_track ) {
        SaveChanges();              // write changes to globals and update config
//        m_pRoute->ClearHighlights();
    }

    m_bStartNow = false;

    if( pRouteManagerDialog && pRouteManagerDialog->IsShown() )
        pRouteManagerDialog->UpdateTrkListCtrl();

    Hide();
    cc1->InvalidateGL();
    cc1->Refresh( false );

    event.Skip();
}

void TrackPropDlg::OnCancelBtnClick( wxCommandEvent& event )
{
    Hide();
    cc1->Refresh( false );

    event.Skip();
}

//--------------------------------------------------------------------------------------
//          OCPNTrackListCtrl Implementation
//---------------------------------------------------------------------------------------


OCPNTrackListCtrl::OCPNTrackListCtrl( wxWindow* parent, wxWindowID id, const wxPoint& pos,
        const wxSize& size, long style ) :
        wxListCtrl( parent, id, pos, size, style )
{
    m_parent = parent;
    m_tz_selection = LTINPUT;
    m_LMT_Offset = 0;
}

OCPNTrackListCtrl::~OCPNTrackListCtrl()
{
}

wxString OCPNTrackListCtrl::OnGetItemText( long item, long column ) const
{
    wxString ret;

    if(item < 0 || item >= m_pTrack->GetnPoints())
        return wxEmptyString;
    
    TrackPoint              *this_point = m_pTrack->GetPoint(item);
    TrackPoint              *prev_point = item > 0 ? m_pTrack->GetPoint(item-1) : NULL;

    if( ! this_point )
        return wxEmptyString;

    double                  gt_brg, gt_leg_dist;
    double slat, slon;
    if( item == 0 )
    {
        slat = gLat;
        slon = gLon;
    }
    else if( prev_point )
    {
        slat = prev_point->m_lat;
        slon = prev_point->m_lon;
    }
    else
    {
        slat = gLat;
        slon = gLon;
    }

    switch( column )
    {
        case 0:
            if( item == 0 )
                ret = _T("---");
            else
                ret.Printf( _T("%ld"), item );
            break;

        case 1:
            DistanceBearingMercator( this_point->m_lat, this_point->m_lon, slat, slon, &gt_brg, &gt_leg_dist );

            ret.Printf( _T("%6.2f ") + getUsrDistanceUnit(), toUsrDistance( gt_leg_dist ) );
            break;

        case 2:
            DistanceBearingMercator( this_point->m_lat, this_point->m_lon, slat, slon, &gt_brg, &gt_leg_dist );
            ret.Printf( _T("%03.0f \u00B0T"), gt_brg );
            break;

        case 3:
            ret = toSDMM( 1, this_point->m_lat, 1 );
            break;

        case 4:
            ret = toSDMM( 2, this_point->m_lon, 1 );
            break;

        case 5:
            {
                wxDateTime timestamp = this_point->GetCreateTime();
                if( timestamp.IsValid() )
                    ret = timestamp2s( timestamp, m_tz_selection, m_LMT_Offset, TIMESTAMP_FORMAT );
                else
                    ret = _T("----");
            }
            break;

        case 6:
            if( ( item > 0 ) && this_point->GetCreateTime().IsValid()
                    && prev_point->GetCreateTime().IsValid() )
            {
                DistanceBearingMercator( this_point->m_lat, this_point->m_lon, slat, slon, &gt_brg, &gt_leg_dist );
                double speed = 0.;
                double seconds =
                        this_point->GetCreateTime().Subtract( prev_point->GetCreateTime() ).GetSeconds().ToDouble();

                if( seconds > 0. )
                    speed = gt_leg_dist / seconds * 3600;

                ret.Printf( _T("%5.2f"), toUsrSpeed( speed ) );
            } else
                ret = _("--");
            break;

        default:
            break;
    }

    return ret;
}

int OCPNTrackListCtrl::OnGetItemColumnImage( long item, long column ) const
{
    return -1;
}


