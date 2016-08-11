/*
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA  02110-1301, USA.

    ---
    Copyright (C) 2010, Anders Lund <anders@alweb.dk>
 */

#include "routemanagerdialog.h"

// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/progdlg.h>
#include <wx/clipbrd.h>

#include <iostream>

#include "styles.h"
#include "dychart.h"
#include "navutil.h"
#include "routeprop.h"
#include "routeman.h"
#include "georef.h"
#include "chartbase.h"
#include "Layer.h"
#include "SendToGpsDlg.h"
#include "TrackPropDlg.h"
#include "AIS_Decoder.h"
#include "OCPNPlatform.h"

#define DIALOG_MARGIN 3

/* XPM */
static const char *eye[]={
"20 20 7 1",
". c none",
"# c #000000",
"a c #333333",
"b c #666666",
"c c #999999",
"d c #cccccc",
"e c #ffffff",
"....................",
"....................",
"....................",
"....................",
".......######.......",
".....#aabccb#a#.....",
"....#deeeddeebcb#...",
"..#aeeeec##aceaec#..",
".#bedaeee####dbcec#.",
"#aeedbdabc###bcceea#",
".#bedad######abcec#.",
"..#be#d######dadb#..",
"...#abac####abba#...",
".....##acbaca##.....",
".......######.......",
"....................",
"....................",
"....................",
"....................",
"...................."};

/* XPM */
static const char *eyex[]={
    "20 20 8 1",
    "# c None",
    "a c #000000",
    "b c #333333",
    "c c #666666",
    "d c #999999",
    "f c #cccccc",
    ". c #ff0000",
    "e c #ffffff",
    ".##################.",
    "..################..",
    "#..##############..#",
    "##..############..##",
    "###..##aaaaaa##..###",
    "####..bbcddcab..####",
    "####a..eeffee..ca###",
    "##abee..daab..beda##",
    "#acefbe..aa..fcdeda#",
    "abeefcfb....acddeeba",
    "#acefbfaa..aabcdeda#",
    "##aceafa....afbfca##",
    "###abcb..aa..ccba###",
    "#####a..dcbd..a#####",
    "#####..aaaaaa..#####",
    "####..########..####",
    "###..##########..###",
    "##..############..##",
    "#..##############..#",
    "..################.."};

enum { rmVISIBLE = 0, rmROUTENAME, rmROUTEDESC };// RMColumns;
enum { colTRKVISIBLE = 0, colTRKNAME, colTRKLENGTH };
enum { colLAYVISIBLE = 0, colLAYNAME, colLAYITEMS };
enum { colWPTICON = 0, colWPTNAME, colWPTDIST };

// GLOBALS :0
extern RouteList *pRouteList;
extern TrackList *pTrackList;
extern LayerList *pLayerList;
extern wxString GetLayerName(int id);
extern RouteProp *pRoutePropDialog;
extern TrackPropDlg *pTrackPropDialog;
extern Routeman  *g_pRouteMan;
extern MyConfig  *pConfig;
extern ChartCanvas *cc1;
extern ChartBase *Current_Ch;
extern ActiveTrack      *g_pActiveTrack;
extern WayPointman      *pWayPointMan;
extern MarkInfoImpl     *pMarkPropDialog;
extern MyFrame          *gFrame;
extern Select           *pSelect;
extern double           gLat, gLon;
extern double           gCog, gSog;
extern bool             g_bShowLayers;
extern wxString         g_default_wp_icon;
extern AIS_Decoder      *g_pAIS;
extern bool             g_bresponsive;

// sort callback. Sort by route name.
int sort_route_name_dir;
#if wxCHECK_VERSION(2, 9, 0)
int wxCALLBACK SortRoutesOnName(wxIntPtr item1, wxIntPtr item2, wxIntPtr list)
#else
int wxCALLBACK SortRoutesOnName(long item1, long item2, long list)
#endif
{
    wxListCtrl *lc = (wxListCtrl*)list;

    wxListItem it1, it2;
    it1.SetId(lc->FindItem(-1, item1));
    it1.SetColumn(1);
    it1.SetMask(it1.GetMask() | wxLIST_MASK_TEXT);

    it2.SetId(lc->FindItem(-1, item2));
    it2.SetColumn(1);
    it2.SetMask(it2.GetMask() | wxLIST_MASK_TEXT);

    lc->GetItem(it1);
    lc->GetItem(it2);

    if(sort_route_name_dir & 1)
    return it2.GetText().CmpNoCase(it1.GetText());
    else
    return it1.GetText().CmpNoCase(it2.GetText());

}

// sort callback. Sort by route Destination.
int sort_route_to_dir;
#if wxCHECK_VERSION(2, 9, 0)
int wxCALLBACK SortRoutesOnTo(wxIntPtr item1, wxIntPtr item2, wxIntPtr list)
#else
int wxCALLBACK SortRoutesOnTo(long item1, long item2, long list)
#endif
{
    wxListCtrl *lc = (wxListCtrl*)list;

    wxListItem it1, it2;
    it1.SetId(lc->FindItem(-1, item1));
    it1.SetColumn(2);
    it1.SetMask(it1.GetMask() | wxLIST_MASK_TEXT);

    it2.SetId(lc->FindItem(-1, item2));
    it2.SetColumn(2);
    it2.SetMask(it2.GetMask() | wxLIST_MASK_TEXT);

    lc->GetItem(it1);
    lc->GetItem(it2);

    if(sort_route_to_dir & 1)
    return it2.GetText().CmpNoCase(it1.GetText());
    else
    return it1.GetText().CmpNoCase(it2.GetText());
}

// sort callback. Sort by track name.
int sort_track_name_dir;
#if wxCHECK_VERSION(2, 9, 0)
int wxCALLBACK SortTracksOnName(wxIntPtr item1, wxIntPtr item2, wxIntPtr list)
#else
int wxCALLBACK SortTracksOnName(long item1, long item2, long list)
#endif
{
    wxListCtrl *lc = (wxListCtrl*)list;

    wxListItem it1, it2;
    it1.SetId(lc->FindItem(-1, item1));
    it1.SetColumn(1);
    it1.SetMask(it1.GetMask() | wxLIST_MASK_TEXT);

    it2.SetId(lc->FindItem(-1, item2));
    it2.SetColumn(1);
    it2.SetMask(it2.GetMask() | wxLIST_MASK_TEXT);

    lc->GetItem(it1);
    lc->GetItem(it2);

    if(sort_track_name_dir & 1)
    return it2.GetText().CmpNoCase(it1.GetText());
    else
    return it1.GetText().CmpNoCase(it2.GetText());

}

// sort callback. Sort by track length.
int sort_track_len_dir;
#if wxCHECK_VERSION(2, 9, 0)
int wxCALLBACK SortTracksOnDistance(wxIntPtr item1, wxIntPtr item2, wxIntPtr list)
#else
int wxCALLBACK SortTracksOnDistance(long item1, long item2, long list)
#endif
{
    wxListCtrl *lc = (wxListCtrl*)list;

    wxListItem it1, it2;
    it1.SetId(lc->FindItem(-1, item1));
    it1.SetColumn(2);
    it1.SetMask(it1.GetMask() | wxLIST_MASK_TEXT);

    it2.SetId(lc->FindItem(-1, item2));
    it2.SetColumn(2);
    it2.SetMask(it2.GetMask() | wxLIST_MASK_TEXT);

    lc->GetItem(it1);
    lc->GetItem(it2);

    wxString s1, s2;
    s1.Printf(_T("%11s"), it1.GetText().c_str());
    s2.Printf(_T("%11s"), it2.GetText().c_str());

    double l1, l2;
    s1.ToDouble(&l1);
    s2.ToDouble(&l2);

    if(sort_track_len_dir & 1)
    return(l1 < l2);
    else
    return(l2 < l1);

}

int sort_wp_key;

// sort callback. Sort by wpt name.
int sort_wp_name_dir;
#if wxCHECK_VERSION(2, 9, 0)
int wxCALLBACK SortWaypointsOnName(wxIntPtr item1, wxIntPtr item2, wxIntPtr list)
#else
int wxCALLBACK SortWaypointsOnName(long item1, long item2, long list)
#endif

{
    wxListCtrl *lc = (wxListCtrl*)list;

    RoutePoint *pRP1 = (RoutePoint *)item1;
    RoutePoint *pRP2 = (RoutePoint *)item2;

    if(pRP1 && pRP2) {
        if(sort_wp_name_dir & 1)
            return pRP2->GetName().CmpNoCase(pRP1->GetName());
        else
            return pRP1->GetName().CmpNoCase(pRP2->GetName());
    }
    else
        return 0;
    
}

// sort callback. Sort by wpt distance.
int sort_wp_len_dir;
#if wxCHECK_VERSION(2, 9, 0)
int wxCALLBACK SortWaypointsOnDistance(wxIntPtr item1, wxIntPtr item2, wxIntPtr list)
#else
int wxCALLBACK SortWaypointsOnDistance(long item1, long item2, long list)
#endif

{
    wxListCtrl *lc = (wxListCtrl*)list;

    wxListItem it1, it2;
    it1.SetId(lc->FindItem(-1, item1));
    it1.SetColumn(2);
    it1.SetMask(it1.GetMask() | wxLIST_MASK_TEXT);

    it2.SetId(lc->FindItem(-1, item2));
    it2.SetColumn(2);
    it2.SetMask(it2.GetMask() | wxLIST_MASK_TEXT);

    lc->GetItem(it1);
    lc->GetItem(it2);

    wxString s1, s2;
    s1.Printf(_T("%11s"), it1.GetText().c_str());
    s2.Printf(_T("%11s"), it2.GetText().c_str());

    double l1, l2;
    s1.ToDouble(&l1);
    s2.ToDouble(&l2);

    if(sort_wp_len_dir & 1)
    return(l1 < l2);
    else
    return(l2 < l1);

}

// sort callback. Sort by layer name.
int sort_layer_name_dir;
#if wxCHECK_VERSION(2, 9, 0)
int wxCALLBACK SortLayersOnName(wxIntPtr item1, wxIntPtr item2, wxIntPtr list)
#else
int wxCALLBACK SortLayersOnName(long item1, long item2, long list)
#endif
{
    wxListCtrl *lc = (wxListCtrl*)list;

    wxListItem it1, it2;
    it1.SetId(lc->FindItem(-1, item1));
    it1.SetColumn(1);
    it1.SetMask(it1.GetMask() | wxLIST_MASK_TEXT);

    it2.SetId(lc->FindItem(-1, item2));
    it2.SetColumn(1);
    it2.SetMask(it2.GetMask() | wxLIST_MASK_TEXT);

    lc->GetItem(it1);
    lc->GetItem(it2);

    if(sort_layer_name_dir & 1)
    return it2.GetText().CmpNoCase(it1.GetText());
    else
    return it1.GetText().CmpNoCase(it2.GetText());

}

// sort callback. Sort by layer size.
int sort_layer_len_dir;
#if wxCHECK_VERSION(2, 9, 0)
int wxCALLBACK SortLayersOnSize(wxIntPtr item1, wxIntPtr item2, wxIntPtr list)
#else
int wxCALLBACK SortLayersOnSize(long item1, long item2, long list)
#endif
{
    wxListCtrl *lc = (wxListCtrl*)list;

    wxListItem it1, it2;
    it1.SetId(lc->FindItem(-1, item1));
    it1.SetColumn(2);
    it1.SetMask(it1.GetMask() | wxLIST_MASK_TEXT);

    it2.SetId(lc->FindItem(-1, item2));
    it2.SetColumn(2);
    it2.SetMask(it2.GetMask() | wxLIST_MASK_TEXT);

    lc->GetItem(it1);
    lc->GetItem(it2);

    wxString s1, s2;
    s1.Printf(_T("%11s"), it1.GetText().c_str());
    s2.Printf(_T("%11s"), it2.GetText().c_str());

    double l1, l2;
    s1.ToDouble(&l1);
    s2.ToDouble(&l2);

    if(sort_layer_len_dir & 1)
    return(l1 < l2);
    else
    return(l2 < l1);

}

// event table. Mostly empty, because I find it much easier to see what is connected to what
// using Connect() where possible, so that it is visible in the code.
BEGIN_EVENT_TABLE(RouteManagerDialog, wxDialog)
EVT_NOTEBOOK_PAGE_CHANGED(wxID_ANY, RouteManagerDialog::OnTabSwitch) // This should work under Windows :-(
EVT_CLOSE(RouteManagerDialog::OnClose)
EVT_COMMAND(wxID_OK, wxEVT_COMMAND_BUTTON_CLICKED, RouteManagerDialog::OnOK)
END_EVENT_TABLE()

void RouteManagerDialog::OnTabSwitch( wxNotebookEvent &event )
{
    if( !m_pNotebook ) return;
    int current_page = m_pNotebook->GetSelection();
    if( current_page == 3 ) {
//        if( btnImport ) btnImport->Enable( false );
//        if( btnExport ) btnExport->Enable( false );
//        if( btnExportViz ) btnExportViz->Enable( false );
    } else {
        if( btnImport ) btnImport->Enable( true );
        if( btnExport ) btnExport->Enable( true );
        if( btnExportViz ) btnExportViz->Enable( true );

    }
    event.Skip(); // remove if using event table... why?
}

// implementation

bool RouteManagerDialog::instanceFlag = false;
RouteManagerDialog* RouteManagerDialog::single = NULL;

RouteManagerDialog* RouteManagerDialog::getInstance(wxWindow *parent)
{
    if(! instanceFlag)
    {
        single = new RouteManagerDialog(parent);
        instanceFlag = true;
        return single;
    }
    else
    {
        return single;
    }
}


RouteManagerDialog::RouteManagerDialog( wxWindow *parent )
{
    long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER;
#ifdef __WXOSX__
    style |= wxSTAY_ON_TOP;
#endif

    wxDialog::Create( parent, -1, wxString( _("Route & Mark Manager") ), wxDefaultPosition, wxDefaultSize,
            style );
    
    wxFont *qFont = GetOCPNScaledFont(_("Dialog"));
    SetFont( *qFont );

    m_lastWptItem = -1;
    m_lastTrkItem = -1;
    m_lastRteItem = -1;

    btnImport = NULL;
    btnExport = NULL;
    btnExportViz = NULL;

    Create();

}

void RouteManagerDialog::Create()
{
    //  Get a text height metric for reference
    int char_width, char_height;
    GetTextExtent(_T("W"), &char_width, &char_height);
    m_charWidth = char_width;
        
    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer( wxVERTICAL );
    SetSizer( itemBoxSizer1 );
    
    m_pNotebook = new wxNotebook( this, wxID_ANY, wxDefaultPosition, wxSize( -1, -1 ), wxNB_TOP );
    itemBoxSizer1->Add( m_pNotebook, 1,
                        wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL | wxALL | wxEXPAND, 5 );
    
    //  Create "Routes" panel
    m_pPanelRte = new wxPanel( m_pNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                               wxNO_BORDER | wxTAB_TRAVERSAL);
    
    wxBoxSizer *sbsRoutes = new wxBoxSizer( wxHORIZONTAL );
    m_pPanelRte->SetSizer( sbsRoutes );
    m_pNotebook->AddPage( m_pPanelRte, _("Routes") );
    
    sort_wp_len_dir = 1;
    sort_wp_name_dir = 0;
    sort_track_len_dir = 1;
    sort_route_to_dir = 0;
    sort_track_name_dir = 0;
    sort_route_name_dir = 0;
    sort_layer_name_dir = 0;
    sort_layer_len_dir = 1;
    
    // Setup GUI
    m_pRouteListCtrl = new wxListCtrl( m_pPanelRte, -1, wxDefaultPosition, wxSize(-1, -1),
                                       wxLC_REPORT  | wxLC_SORT_ASCENDING | wxLC_HRULES
                                       | wxBORDER_SUNKEN/*|wxLC_VRULES*/);
    #ifdef __OCPN__ANDROID__    
    m_pRouteListCtrl->GetHandle()->setStyleSheet(getQtStyleSheet());
    #endif    
    
    m_pRouteListCtrl->Connect( wxEVT_COMMAND_LIST_ITEM_SELECTED,
                               wxListEventHandler(RouteManagerDialog::OnRteSelected), NULL, this );
    m_pRouteListCtrl->Connect( wxEVT_COMMAND_LIST_ITEM_DESELECTED,
                               wxListEventHandler(RouteManagerDialog::OnRteSelected), NULL, this );
    m_pRouteListCtrl->Connect( wxEVT_COMMAND_LIST_ITEM_ACTIVATED,
                               wxListEventHandler(RouteManagerDialog::OnRteDefaultAction), NULL, this );
    m_pRouteListCtrl->Connect( wxEVT_LEFT_DOWN,
                               wxMouseEventHandler(RouteManagerDialog::OnRteToggleVisibility), NULL, this );
    m_pRouteListCtrl->Connect( wxEVT_COMMAND_LIST_COL_CLICK,
                               wxListEventHandler(RouteManagerDialog::OnRteColumnClicked), NULL, this );
    sbsRoutes->Add( m_pRouteListCtrl, 1, wxEXPAND | wxALL, DIALOG_MARGIN );
    
    // Columns: visibility ctrl, name
    // note that under MSW for SetColumnWidth() to work we need to create the
    // items with images initially even if we specify dummy image id
    m_pRouteListCtrl->InsertColumn( rmVISIBLE, _("Show"), wxLIST_FORMAT_LEFT, 10 /*4 * char_width*/ );
    m_pRouteListCtrl->InsertColumn( rmROUTENAME, _("Route Name"), wxLIST_FORMAT_LEFT, 15 * char_width );
    m_pRouteListCtrl->InsertColumn( rmROUTEDESC, _("From <-> To"), wxLIST_FORMAT_LEFT, 10 * char_width );
    
    // Buttons: Delete, Properties...
    wxBoxSizer *bsRouteButtons = new wxBoxSizer( wxVERTICAL );
    sbsRoutes->Add( bsRouteButtons, 0, wxALIGN_RIGHT | wxEXPAND );
    
    wxScrolledWindow *winr = new wxScrolledWindow( m_pPanelRte, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                                   wxNO_BORDER | wxTAB_TRAVERSAL | wxVSCROLL);
    winr->SetScrollRate(0,5);
    
    bsRouteButtons->Add( winr, 1, wxALL | wxEXPAND, DIALOG_MARGIN );
    
    wxBoxSizer *bsRouteButtonsInner = new wxBoxSizer( wxVERTICAL );
    winr->SetSizer(bsRouteButtonsInner);
    
    btnRteProperties = new wxButton( winr, -1, _("&Properties...") );
    bsRouteButtonsInner->Add( btnRteProperties, 0, wxALL | wxEXPAND, DIALOG_MARGIN );
    btnRteProperties->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
                               wxCommandEventHandler(RouteManagerDialog::OnRtePropertiesClick), NULL, this );
    
    btnRteActivate = new wxButton( winr, -1, _("&Activate") );
    bsRouteButtonsInner->Add( btnRteActivate, 0, wxALL | wxEXPAND, DIALOG_MARGIN );
    btnRteActivate->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
                             wxCommandEventHandler(RouteManagerDialog::OnRteActivateClick), NULL, this );
    btnRteActivate->Connect( wxEVT_LEFT_DOWN,
                             wxMouseEventHandler(RouteManagerDialog::OnRteBtnLeftDown), NULL, this );
    
    btnRteZoomto = new wxButton( winr, -1, _("&Center View") );
    bsRouteButtonsInner->Add( btnRteZoomto, 0, wxALL | wxEXPAND, DIALOG_MARGIN );
    btnRteZoomto->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
                           wxCommandEventHandler(RouteManagerDialog::OnRteZoomtoClick), NULL, this );
    btnRteZoomto->Connect( wxEVT_LEFT_DOWN,
                           wxMouseEventHandler(RouteManagerDialog::OnRteBtnLeftDown), NULL, this );
    
    btnRteReverse = new wxButton( winr, -1, _("&Reverse") );
    bsRouteButtonsInner->Add( btnRteReverse, 0, wxALL | wxEXPAND, DIALOG_MARGIN );
    btnRteReverse->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
                            wxCommandEventHandler(RouteManagerDialog::OnRteReverseClick), NULL, this );
    
    btnRteDelete = new wxButton( winr, -1, _("&Delete") );
    bsRouteButtonsInner->Add( btnRteDelete, 0, wxALL | wxEXPAND, DIALOG_MARGIN );
    btnRteDelete->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
                           wxCommandEventHandler(RouteManagerDialog::OnRteDeleteClick), NULL, this );
    
    btnRteExport = new wxButton( winr, -1, _("&Export selected...") );
    bsRouteButtonsInner->Add( btnRteExport, 0, wxALL | wxEXPAND, DIALOG_MARGIN );
    btnRteExport->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
                           wxCommandEventHandler(RouteManagerDialog::OnRteExportClick), NULL, this );
    
    btnRteSendToGPS = new wxButton( winr, -1, _("&Send to GPS") );
    bsRouteButtonsInner->Add( btnRteSendToGPS, 0, wxALL | wxEXPAND, DIALOG_MARGIN );
    btnRteSendToGPS->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
                              wxCommandEventHandler(RouteManagerDialog::OnRteSendToGPSClick), NULL, this );
    
    bsRouteButtonsInner->AddSpacer( 10 );
    
    btnRteDeleteAll = new wxButton( winr, -1, _("&Delete All") );
    bsRouteButtonsInner->Add( btnRteDeleteAll, 0, wxALL | wxEXPAND, DIALOG_MARGIN );
    btnRteDeleteAll->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
                              wxCommandEventHandler(RouteManagerDialog::OnRteDeleteAllClick), NULL, this );
    
    //  Create "Tracks" panel
    m_pPanelTrk = new wxPanel( m_pNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                               wxNO_BORDER | wxTAB_TRAVERSAL );
    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer( wxHORIZONTAL );
    m_pPanelTrk->SetSizer( itemBoxSizer3 );
    m_pNotebook->AddPage( m_pPanelTrk, _("Tracks") );
    
    m_pTrkListCtrl = new wxListCtrl( m_pPanelTrk, -1, wxDefaultPosition, wxDefaultSize,
                                     wxLC_REPORT | wxLC_SORT_ASCENDING | wxLC_HRULES | wxBORDER_SUNKEN/*|wxLC_VRULES*/);
    
    #ifdef __OCPN__ANDROID__    
    m_pTrkListCtrl->GetHandle()->setStyleSheet(getQtStyleSheet());
    #endif    
    
    m_pTrkListCtrl->Connect( wxEVT_COMMAND_LIST_ITEM_SELECTED,
                             wxListEventHandler(RouteManagerDialog::OnTrkSelected), NULL, this );
    m_pTrkListCtrl->Connect( wxEVT_COMMAND_LIST_ITEM_DESELECTED,
                             wxListEventHandler(RouteManagerDialog::OnTrkSelected), NULL, this );
    m_pTrkListCtrl->Connect( wxEVT_COMMAND_LIST_ITEM_ACTIVATED,
                             wxListEventHandler(RouteManagerDialog::OnTrkDefaultAction), NULL, this );
    m_pTrkListCtrl->Connect( wxEVT_LEFT_DOWN,
                             wxMouseEventHandler(RouteManagerDialog::OnTrkToggleVisibility), NULL, this );
    m_pTrkListCtrl->Connect( wxEVT_COMMAND_LIST_COL_CLICK,
                             wxListEventHandler(RouteManagerDialog::OnTrkColumnClicked), NULL, this );
    m_pTrkListCtrl->Connect( wxEVT_COMMAND_LIST_ITEM_RIGHT_CLICK,
                             wxListEventHandler(RouteManagerDialog::OnTrkRightClick), NULL, this );
    this->Connect( wxEVT_COMMAND_MENU_SELECTED,
                   wxCommandEventHandler(RouteManagerDialog::OnTrkMenuSelected), NULL, this );
    
    itemBoxSizer3->Add( m_pTrkListCtrl, 1, wxEXPAND | wxALL, DIALOG_MARGIN );
    
    m_pTrkListCtrl->InsertColumn( colTRKVISIBLE, _("Show"), wxLIST_FORMAT_LEFT, 4 * char_width );
    m_pTrkListCtrl->InsertColumn( colTRKNAME, _("Track Name"), wxLIST_FORMAT_LEFT, 20 * char_width );
    m_pTrkListCtrl->InsertColumn( colTRKLENGTH, _("Length"), wxLIST_FORMAT_LEFT, 5 * char_width );
    
    wxBoxSizer *bsTrkButtons = new wxBoxSizer( wxVERTICAL );
    itemBoxSizer3->Add( bsTrkButtons, 0, wxALIGN_RIGHT | wxEXPAND );
    
    wxScrolledWindow *wint = new wxScrolledWindow( m_pPanelTrk, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                                   wxNO_BORDER | wxTAB_TRAVERSAL | wxVSCROLL);
    wint->SetScrollRate(0,5);
    
    bsTrkButtons->Add( wint, 1, wxALL | wxEXPAND, DIALOG_MARGIN );
    
    wxBoxSizer *bsTrkButtonsInner = new wxBoxSizer( wxVERTICAL );
    wint->SetSizer(bsTrkButtonsInner);
    
    btnTrkNew = new wxButton( wint, -1, _("&Start Track") );
    bsTrkButtonsInner->Add( btnTrkNew, 0, wxALL | wxEXPAND, DIALOG_MARGIN );
    btnTrkNew->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
                        wxCommandEventHandler(RouteManagerDialog::OnTrkNewClick), NULL, this );
    
    btnTrkProperties = new wxButton( wint, -1, _("&Properties") );
    bsTrkButtonsInner->Add( btnTrkProperties, 0, wxALL | wxEXPAND, DIALOG_MARGIN );
    btnTrkProperties->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
                               wxCommandEventHandler(RouteManagerDialog::OnTrkPropertiesClick), NULL, this );
    
    btnTrkDelete = new wxButton( wint, -1, _("&Delete") );
    bsTrkButtonsInner->Add( btnTrkDelete, 0, wxALL | wxEXPAND, DIALOG_MARGIN );
    btnTrkDelete->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
                           wxCommandEventHandler(RouteManagerDialog::OnTrkDeleteClick), NULL, this );
    
    btnTrkExport = new wxButton( wint, -1, _("&Export selected...") );
    bsTrkButtonsInner->Add( btnTrkExport, 0, wxALL | wxEXPAND, DIALOG_MARGIN );
    btnTrkExport->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
                           wxCommandEventHandler(RouteManagerDialog::OnTrkExportClick), NULL, this );
    
    btnTrkRouteFromTrack = new wxButton( wint, -1, _("Route from Track") );
    bsTrkButtonsInner->Add( btnTrkRouteFromTrack, 0, wxALL | wxEXPAND, DIALOG_MARGIN );
    btnTrkRouteFromTrack->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
                                   wxCommandEventHandler(RouteManagerDialog::OnTrkRouteFromTrackClick), NULL, this );
    
    bsTrkButtonsInner->AddSpacer( 10 );
    
    btnTrkDeleteAll = new wxButton( wint, -1, _("&Delete All") );
    bsTrkButtonsInner->Add( btnTrkDeleteAll, 0, wxALL | wxEXPAND, DIALOG_MARGIN );
    btnTrkDeleteAll->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
                              wxCommandEventHandler(RouteManagerDialog::OnTrkDeleteAllClick), NULL, this );
    
    //  Create "Waypoints" panel
    m_pPanelWpt = new wxPanel( m_pNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                               wxNO_BORDER | wxTAB_TRAVERSAL );
    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer( wxHORIZONTAL );
    m_pPanelWpt->SetSizer( itemBoxSizer4 );
    m_pNotebook->AddPage( m_pPanelWpt, _("Waypoints") );
    
    m_pWptListCtrl = new wxListCtrl( m_pPanelWpt, -1, wxDefaultPosition, wxDefaultSize,
                                     wxLC_REPORT | wxLC_SORT_ASCENDING | wxLC_HRULES | wxBORDER_SUNKEN/*|wxLC_VRULES*/);
    #ifdef __OCPN__ANDROID__    
    m_pWptListCtrl->GetHandle()->setStyleSheet(getQtStyleSheet());
    #endif    
    
    m_pWptListCtrl->Connect( wxEVT_COMMAND_LIST_ITEM_SELECTED,
                             wxListEventHandler(RouteManagerDialog::OnWptSelected), NULL, this );
    m_pWptListCtrl->Connect( wxEVT_COMMAND_LIST_ITEM_DESELECTED,
                             wxListEventHandler(RouteManagerDialog::OnWptSelected), NULL, this );
    m_pWptListCtrl->Connect( wxEVT_COMMAND_LIST_ITEM_ACTIVATED,
                             wxListEventHandler(RouteManagerDialog::OnWptDefaultAction), NULL, this );
    m_pWptListCtrl->Connect( wxEVT_LEFT_DOWN,
                             wxMouseEventHandler(RouteManagerDialog::OnWptToggleVisibility), NULL, this );
    m_pWptListCtrl->Connect( wxEVT_COMMAND_LIST_COL_CLICK,
                             wxListEventHandler(RouteManagerDialog::OnWptColumnClicked), NULL, this );
    itemBoxSizer4->Add( m_pWptListCtrl, 1, wxEXPAND | wxALL, DIALOG_MARGIN );
    
    m_pWptListCtrl->InsertColumn( colWPTICON, _("Icon"), wxLIST_FORMAT_LEFT, 4 * char_width );
    m_pWptListCtrl->InsertColumn( colWPTNAME, _("Waypoint Name"), wxLIST_FORMAT_LEFT, 15 * char_width );
    m_pWptListCtrl->InsertColumn( colWPTDIST, _("Distance from Ownship"), wxLIST_FORMAT_LEFT, 14 * char_width );
    
    wxBoxSizer *bsWptButtons = new wxBoxSizer( wxVERTICAL );
    itemBoxSizer4->Add( bsWptButtons, 0, wxALIGN_RIGHT | wxEXPAND );
    
    wxScrolledWindow *winw = new wxScrolledWindow( m_pPanelWpt, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                                   wxNO_BORDER | wxTAB_TRAVERSAL | wxVSCROLL);
    winw->SetScrollRate(0,5);
    
    bsWptButtons->Add( winw, 1, wxALL | wxEXPAND, DIALOG_MARGIN );
    
    wxBoxSizer *bsWptButtonsInner = new wxBoxSizer( wxVERTICAL );
    winw->SetSizer(bsWptButtonsInner);
    
    btnWptNew = new wxButton( winw, -1, _("&New") );
    bsWptButtonsInner->Add( btnWptNew, 0, wxALL | wxEXPAND, DIALOG_MARGIN );
    btnWptNew->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
                        wxCommandEventHandler(RouteManagerDialog::OnWptNewClick), NULL, this );
    
    btnWptProperties = new wxButton( winw, -1, _("&Properties") );
    bsWptButtonsInner->Add( btnWptProperties, 0, wxALL | wxEXPAND, DIALOG_MARGIN );
    btnWptProperties->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
                               wxCommandEventHandler(RouteManagerDialog::OnWptPropertiesClick), NULL, this );
    
    btnWptZoomto = new wxButton( winw, -1, _("&Center View") );
    bsWptButtonsInner->Add( btnWptZoomto, 0, wxALL | wxEXPAND, DIALOG_MARGIN );
    btnWptZoomto->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
                           wxCommandEventHandler(RouteManagerDialog::OnWptZoomtoClick), NULL, this );
    
    btnWptDelete = new wxButton( winw, -1, _("&Delete") );
    bsWptButtonsInner->Add( btnWptDelete, 0, wxALL | wxEXPAND, DIALOG_MARGIN );
    btnWptDelete->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
                           wxCommandEventHandler(RouteManagerDialog::OnWptDeleteClick), NULL, this );
    
    btnWptGoTo = new wxButton( winw, -1, _("&Go To") );
    bsWptButtonsInner->Add( btnWptGoTo, 0, wxALL | wxEXPAND, DIALOG_MARGIN );
    btnWptGoTo->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
                         wxCommandEventHandler(RouteManagerDialog::OnWptGoToClick), NULL, this );
    
    btnWptExport = new wxButton( winw, -1, _("&Export selected...") );
    bsWptButtonsInner->Add( btnWptExport, 0, wxALL | wxEXPAND, DIALOG_MARGIN );
    btnWptExport->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
                           wxCommandEventHandler(RouteManagerDialog::OnWptExportClick), NULL, this );
    
    btnWptSendToGPS = new wxButton( winw, -1, _("&Send to GPS") );
    bsWptButtonsInner->Add( btnWptSendToGPS, 0, wxALL | wxEXPAND, DIALOG_MARGIN );
    btnWptSendToGPS->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
                              wxCommandEventHandler(RouteManagerDialog::OnWptSendToGPSClick), NULL, this );
    
    bsWptButtonsInner->AddSpacer( 10 );
    
    btnWptDeleteAll = new wxButton( winw, -1, _("Delete All") );
    bsWptButtonsInner->Add( btnWptDeleteAll, 0, wxALL | wxEXPAND, DIALOG_MARGIN );
    btnWptDeleteAll->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
                              wxCommandEventHandler(RouteManagerDialog::OnWptDeleteAllClick), NULL, this );
    
    wxBoxSizer *itemBoxSizer5 = new wxBoxSizer( wxHORIZONTAL );
    itemBoxSizer1->Add( itemBoxSizer5, 0, wxALL | wxEXPAND );
    
    wxBoxSizer *itemBoxSizer6 = new wxBoxSizer( wxHORIZONTAL );
    itemBoxSizer5->Add( itemBoxSizer6, 1, wxALL | wxEXPAND | wxALIGN_LEFT );
    
    btnImport = new wxButton( this, -1, _("I&mport GPX...") );
    itemBoxSizer6->Add( btnImport, 0, wxALL | wxALIGN_LEFT, DIALOG_MARGIN );
    btnImport->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
                        wxCommandEventHandler(RouteManagerDialog::OnImportClick), NULL, this );
    /*
     *    btnExport = new wxButton( this, -1, _("E&xport All...") );
     *    itemBoxSizer6->Add( btnExport, 0, wxALL | wxALIGN_LEFT, DIALOG_MARGIN );
     *    btnExport->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
     *            wxCommandEventHandler(RouteManagerDialog::OnExportClick), NULL, this );
     */
    btnExportViz = new wxButton( this, -1, _("Export All Visible...") );
    itemBoxSizer6->Add( btnExportViz, 0, wxALL | wxALIGN_LEFT, DIALOG_MARGIN );
    btnExportViz->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
                           wxCommandEventHandler(RouteManagerDialog::OnExportVizClick), NULL, this );
    
    //  Create "Layers" panel
    m_pPanelLay = new wxPanel( m_pNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                               wxNO_BORDER | wxTAB_TRAVERSAL );
    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer( wxHORIZONTAL );
    m_pPanelLay->SetSizer( itemBoxSizer7 );
    m_pNotebook->AddPage( m_pPanelLay, _("Layers") );
    
    m_pLayListCtrl = new wxListCtrl( m_pPanelLay, -1, wxDefaultPosition, wxDefaultSize,
                                     wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_SORT_ASCENDING | wxLC_HRULES
                                     | wxBORDER_SUNKEN/*|wxLC_VRULES*/);
    #ifdef __OCPN__ANDROID__    
    m_pLayListCtrl->GetHandle()->setStyleSheet(getQtStyleSheet());
    #endif    
    
    m_pLayListCtrl->Connect( wxEVT_COMMAND_LIST_ITEM_SELECTED,
                             wxListEventHandler(RouteManagerDialog::OnLaySelected), NULL, this );
    m_pLayListCtrl->Connect( wxEVT_COMMAND_LIST_ITEM_DESELECTED,
                             wxListEventHandler(RouteManagerDialog::OnLaySelected), NULL, this );
    m_pLayListCtrl->Connect( wxEVT_COMMAND_LIST_ITEM_ACTIVATED,
                             wxListEventHandler(RouteManagerDialog::OnLayDefaultAction), NULL, this );
    m_pLayListCtrl->Connect( wxEVT_LEFT_DOWN,
                             wxMouseEventHandler(RouteManagerDialog::OnLayToggleVisibility), NULL, this );
    m_pLayListCtrl->Connect( wxEVT_COMMAND_LIST_COL_CLICK,
                             wxListEventHandler(RouteManagerDialog::OnLayColumnClicked), NULL, this );
    itemBoxSizer7->Add( m_pLayListCtrl, 1, wxEXPAND | wxALL, DIALOG_MARGIN );
    
    m_pLayListCtrl->InsertColumn( colLAYVISIBLE, _T(""), wxLIST_FORMAT_LEFT, 4 * char_width );
    m_pLayListCtrl->InsertColumn( colLAYNAME, _("Layer Name"), wxLIST_FORMAT_LEFT, 10 * char_width );
    m_pLayListCtrl->InsertColumn( colLAYITEMS, _("No. of items"), wxLIST_FORMAT_LEFT, 10 * char_width );
    
    wxBoxSizer *bsLayButtons = new wxBoxSizer( wxVERTICAL );
    itemBoxSizer7->Add( bsLayButtons, 0, wxALIGN_RIGHT | wxEXPAND);
    
    wxScrolledWindow *winl = new wxScrolledWindow( m_pPanelLay, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                                   wxNO_BORDER | wxTAB_TRAVERSAL | wxVSCROLL);
    winl->SetScrollRate(0,5);
    
    bsLayButtons->Add( winl, 1, wxALL | wxEXPAND, DIALOG_MARGIN );
    
    wxBoxSizer *bsLayButtonsInner = new wxBoxSizer( wxVERTICAL );
    winl->SetSizer(bsLayButtonsInner);
    
    btnLayNew = new wxButton( winl, -1, _("Temporary layer") );
    bsLayButtonsInner->Add( btnLayNew, 0, wxALL | wxEXPAND, DIALOG_MARGIN );
    btnLayNew->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
                        wxCommandEventHandler(RouteManagerDialog::OnLayNewClick), NULL, this );
    
    btnLayDelete = new wxButton( winl, -1, _("&Delete") );
    bsLayButtonsInner->Add( btnLayDelete, 0, wxALL | wxEXPAND, DIALOG_MARGIN );
    btnLayDelete->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
                           wxCommandEventHandler(RouteManagerDialog::OnLayDeleteClick), NULL, this );
    
    btnLayToggleChart = new wxButton( winl, -1, _("Show on chart") );
    bsLayButtonsInner->Add( btnLayToggleChart, 0, wxALL | wxEXPAND, DIALOG_MARGIN );
    btnLayToggleChart->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
                                wxCommandEventHandler(RouteManagerDialog::OnLayToggleChartClick), NULL, this );
    
    btnLayToggleNames = new wxButton( winl, -1, _("Show WPT names") );
    bsLayButtonsInner->Add( btnLayToggleNames, 0, wxALL | wxEXPAND, DIALOG_MARGIN );
    btnLayToggleNames->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
                                wxCommandEventHandler(RouteManagerDialog::OnLayToggleNamesClick), NULL, this );
    
    btnLayToggleListing = new wxButton( winl, -1, _("List contents") );
    bsLayButtonsInner->Add( btnLayToggleListing, 0, wxALL | wxEXPAND, DIALOG_MARGIN );
    btnLayToggleListing->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
                                  wxCommandEventHandler(RouteManagerDialog::OnLayToggleListingClick), NULL, this );
    
    // Dialog buttons
    wxSizer *szButtons = CreateButtonSizer( wxOK );
    
    itemBoxSizer5->Add( szButtons, 0, wxALL | wxALIGN_RIGHT, DIALOG_MARGIN );
    
    RecalculateSize();

    // create a image list for the list with just the eye icon
    wxImageList *imglist = new wxImageList( 20, 20, true, 1 );
    imglist->Add( wxBitmap( eye ) );
    imglist->Add( wxBitmap( eyex ) );
    m_pRouteListCtrl->AssignImageList( imglist, wxIMAGE_LIST_SMALL );
    // Assign will handle destroy, Set will not. It's OK, that's what we want
    m_pTrkListCtrl->SetImageList( imglist, wxIMAGE_LIST_SMALL );
    m_pWptListCtrl->SetImageList( pWayPointMan->Getpmarkicon_image_list(), wxIMAGE_LIST_SMALL );
    m_pLayListCtrl->SetImageList( imglist, wxIMAGE_LIST_SMALL );
    
    SetColorScheme();
    
    UpdateRouteListCtrl();
    UpdateTrkListCtrl();
    UpdateWptListCtrl();
    UpdateLayListCtrl();
    
    // This should work under Linux :-(
    //m_pNotebook->Connect(wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED, wxNotebookEventHandler(RouteManagerDialog::OnTabSwitch), NULL, this);
    
    m_bNeedConfigFlush = false;
}

RouteManagerDialog::~RouteManagerDialog()
{
    delete m_pRouteListCtrl;
    delete m_pTrkListCtrl;
    delete m_pWptListCtrl;
    delete m_pLayListCtrl;

    delete btnRteDelete;
    delete btnRteExport;
    delete btnRteZoomto;
    delete btnRteProperties;
    delete btnRteActivate;
    delete btnRteReverse;
    delete btnRteSendToGPS;
    delete btnRteDeleteAll;
    delete btnTrkNew;
    delete btnTrkProperties;
    delete btnTrkDelete;
    delete btnTrkExport;
    delete btnTrkRouteFromTrack;
    delete btnTrkDeleteAll;
    delete btnWptNew;
    delete btnWptProperties;
    delete btnWptZoomto;
    delete btnWptDelete;
    delete btnWptGoTo;
    delete btnWptExport;
    delete btnWptSendToGPS;
    delete btnWptDeleteAll;
    delete btnLayNew;
    //delete btnLayProperties;
    delete btnLayToggleChart;
    delete btnLayToggleListing;
    delete btnLayToggleNames;
    delete btnLayDelete;
    delete btnImport;
    delete btnExport;
    delete btnExportViz;
    btnImport = NULL;
    btnExport = NULL;
    btnExportViz = NULL;

    delete m_pNotebook;
    instanceFlag = false;
    

}

void RouteManagerDialog::RecalculateSize()
{
    
    //  All of this dialog layout is expandable, so we need to set a specific size target
    //  for the onscreen display.
    //  The size will then be adjusted so that it fits within the parent's client area, with some padding
    
    //  Get a text height metric for reference
    int char_width, char_height;
    GetTextExtent(_T("W"), &char_width, &char_height);
    
    wxSize sz;
    sz.x = 60 * char_width;
    sz.y = 30 * char_height;
    
    wxSize dsize = GetParent()->GetClientSize();
    sz.y = wxMin(sz.y, dsize.y - (1 * char_height));
    sz.x = wxMin(sz.x, dsize.x - (1 * char_height));
    SetClientSize(sz);
    
    wxSize fsize = GetSize();
    fsize.y = wxMin(fsize.y, dsize.y - (1 * char_height));
    fsize.x = wxMin(fsize.x, dsize.x - (1 * char_height));
    SetSize(fsize);
    
    CentreOnParent();
    
}

void RouteManagerDialog::OnClose(wxCloseEvent& event)
{
    #ifdef __WXGTK__ 
    gFrame->Raise();
    #endif
    Hide();
    //    pRouteManagerDialog = NULL;
}

void RouteManagerDialog::OnOK(wxCommandEvent& event)
{
    #ifdef __WXGTK__ 
    gFrame->Raise();
    #endif
    Hide();
    
}



void RouteManagerDialog::SetColorScheme()
{
    DimeControl( this );
}

void RouteManagerDialog::UpdateRouteListCtrl()
{
    // if an item was selected, make it selected again if it still exist
    long item = -1;
    item = m_pRouteListCtrl->GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    long selected_id = -1;
    if( item != -1 ) selected_id = m_pRouteListCtrl->GetItemData( item );

    // Delete existing items
    m_pRouteListCtrl->DeleteAllItems();

    // then add routes to the listctrl
    RouteList::iterator it;
    int index = 0;
    int list_index = 0;
    for( it = ( *pRouteList ).begin(); it != ( *pRouteList ).end(); ++it, ++index ) {
        if( !( *it )->IsListed() ) continue;

        wxListItem li;
        li.SetId( list_index );
        li.SetImage( ( *it )->IsVisible() ? 0 : 1 );
        li.SetData( index );
        li.SetText( _T("") );
        li.SetAlign(wxLIST_FORMAT_LEFT);

        if( ( *it )->m_bRtIsActive ) {
            wxFont font = *wxNORMAL_FONT;
            font.SetWeight( wxFONTWEIGHT_BOLD );
            li.SetFont( font );
        }

        long idx = m_pRouteListCtrl->InsertItem( li );

        wxString name = ( *it )->m_RouteNameString;
        if( name.IsEmpty() ) name = _("(Unnamed Route)");
        m_pRouteListCtrl->SetItem( idx, rmROUTENAME, name );

        wxString startend = ( *it )->m_RouteStartString;
        if( !( *it )->m_RouteEndString.IsEmpty() )
            startend.append(_(" - ") + ( *it )->m_RouteEndString );
        m_pRouteListCtrl->SetItem( idx, rmROUTEDESC, startend );
        
        wxListItem lic;
        lic.SetId( list_index );
        lic.SetColumn(1);
        lic.SetAlign(wxLIST_FORMAT_LEFT);
        m_pRouteListCtrl->SetItem( lic );
        
        lic.SetColumn(2);
        lic.SetAlign(wxLIST_FORMAT_LEFT);
        m_pRouteListCtrl->SetItem( lic );
        
        list_index++;
    }

    m_pRouteListCtrl->SortItems( SortRoutesOnName, (wxIntPtr) m_pRouteListCtrl );

    m_pRouteListCtrl->SetColumnWidth(0, 4 * m_charWidth);
    
    
    // restore selection if possible
    // NOTE this will select a different item, if one is deleted
    // (the next route will get that index).
    if( selected_id > -1 ) {
        item = m_pRouteListCtrl->FindItem( -1, selected_id );
        m_pRouteListCtrl->SetItemState( item, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );
    }

    if( (m_lastRteItem >= 0) && (m_pRouteListCtrl->GetItemCount()) )
        m_pRouteListCtrl->EnsureVisible( m_lastRteItem );
    UpdateRteButtons();
}

void RouteManagerDialog::UpdateRteButtons()
{
    // enable/disable buttons
    long selected_index_index = m_pRouteListCtrl->GetNextItem( -1, wxLIST_NEXT_ALL,
            wxLIST_STATE_SELECTED );
    bool enable1 =  m_pRouteListCtrl->GetSelectedItemCount() == 1;
    bool enablemultiple =  m_pRouteListCtrl->GetSelectedItemCount() >= 1;

    m_lastRteItem = selected_index_index;

    btnRteDelete->Enable( m_pRouteListCtrl->GetSelectedItemCount() > 0 );
    btnRteZoomto->Enable( enable1 ); // && !cc1->m_bFollow);
    btnRteProperties->Enable( enable1 );
    btnRteReverse->Enable( enable1 );
    btnRteExport->Enable( enablemultiple );
    btnRteSendToGPS->Enable( enable1 );
    btnRteDeleteAll->Enable( enablemultiple );

    // set activate button text
    Route *route = NULL;
    if( enable1 ) route =
            pRouteList->Item( m_pRouteListCtrl->GetItemData( selected_index_index ) )->GetData();

    if( !g_pRouteMan->IsAnyRouteActive() ) {
        btnRteActivate->Enable( enable1 );
        if( enable1 ) btnRteActivate->SetLabel( _("Activate") );

    } else {
        if( enable1 ) {
            if( route && route->m_bRtIsActive ) {
                btnRteActivate->Enable( enable1 );
                btnRteActivate->SetLabel( _("Deactivate") );
            } else
                btnRteActivate->Enable( false );
        } else
            btnRteActivate->Enable( false );
    }
}

void RouteManagerDialog::MakeAllRoutesInvisible()
{
    RouteList::iterator it;
    long index = 0;
    for( it = ( *pRouteList ).begin(); it != ( *pRouteList ).end(); ++it, ++index ) {
        if( ( *it )->IsVisible() ) { // avoid config updating as much as possible!
            ( *it )->SetVisible( false );
            m_pRouteListCtrl->SetItemImage( m_pRouteListCtrl->FindItem( -1, index ), 1 ); // Likely not same order :0
            pConfig->UpdateRoute( *it ); // auch, flushes config to disk. FIXME
        }
    }
}

void RouteManagerDialog::ZoomtoRoute( Route *route )
{

    // Calculate bbox center
    LLBBox RBBox = route->GetBBox();
    double clat = (RBBox.GetMinLat() + RBBox.GetMaxLat()) / 2;
    double clon = (RBBox.GetMinLon() + RBBox.GetMaxLon()) / 2;

    // Calculate ppm
    double rw, rh, ppm; // route width, height, final ppm scale to use
    int ww, wh; // chart window width, height
    // route bbox width in nm
    DistanceBearingMercator( RBBox.GetMinLat(), RBBox.GetMinLon(), RBBox.GetMinLat(),
                             RBBox.GetMaxLon(), NULL, &rw );
    // route bbox height in nm
    DistanceBearingMercator( RBBox.GetMinLat(), RBBox.GetMinLon(), RBBox.GetMaxLat(),
                             RBBox.GetMinLon(), NULL, &rh );

    cc1->GetSize( &ww, &wh );

    ppm = wxMin(ww/(rw*1852), wh/(rh*1852)) * ( 100 - fabs( clat ) ) / 90;

    ppm = wxMin(ppm, 1.0);

//      cc1->ClearbFollow();
//      cc1->SetViewPoint(clat, clon, ppm, 0, cc1->GetVPRotation(), CURRENT_RENDER);
//      cc1->Refresh();

    gFrame->JumpToPosition( clat, clon, ppm );

    m_bNeedConfigFlush = true;
}

//BEGIN Event handlers
void RouteManagerDialog::OnRteDeleteClick( wxCommandEvent &event )
{
    RouteList list;

    int answer = OCPNMessageBox( this, _("Are you sure you want to delete the selected object(s)"), wxString( _("OpenCPN Alert") ), wxYES_NO );
    if ( answer != wxID_YES )
        return;

    bool busy = false;
    if( m_pRouteListCtrl->GetSelectedItemCount() ) {
        ::wxBeginBusyCursor();
        cc1->CancelMouseRoute();
        m_bNeedConfigFlush = true;
        busy = true;
    }

    long item = -1;
    for ( ;; )
    {
        item = m_pRouteListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if ( item == -1 )
            break;

        Route *proute_to_delete = pRouteList->Item( m_pRouteListCtrl->GetItemData( item ) )->GetData();

        if( proute_to_delete )
            list.Append( proute_to_delete );
    }

    if( busy ) {

        for(unsigned int i=0 ; i < list.GetCount() ; i++) {
            Route *route = list.Item(i)->GetData();
            if( route ) {
                pConfig->DeleteConfigRoute( route );
                g_pRouteMan->DeleteRoute( route );
            }
        }

        m_lastRteItem = -1;
        UpdateRouteListCtrl();
        UpdateTrkListCtrl();

        cc1->undo->InvalidateUndo();
        cc1->Refresh();
        ::wxEndBusyCursor();
    }

}

void RouteManagerDialog::OnRteDeleteAllClick( wxCommandEvent &event )
{
    int dialog_ret = OCPNMessageBox( this, _("Are you sure you want to delete <ALL> routes?"),
            wxString( _("OpenCPN Alert") ), wxYES_NO );

    if( dialog_ret == wxID_YES ) {
        if( g_pRouteMan->GetpActiveRoute() ) g_pRouteMan->DeactivateRoute();

        cc1->CancelMouseRoute();

        g_pRouteMan->DeleteAllRoutes();
// TODO Seth
//            m_pSelectedRoute = NULL;
//            m_pFoundRoutePoint = NULL;
//            m_pFoundRoutePointSecond = NULL;

        m_lastRteItem = -1;
        UpdateRouteListCtrl();

        //    Also need to update the track list control, since routes and tracks share a common global list (pRouteList)
        UpdateTrkListCtrl();

        if( pRoutePropDialog ) pRoutePropDialog->Hide();
        cc1->undo->InvalidateUndo();
        cc1->Refresh();

        m_bNeedConfigFlush = true;
    }
}

void RouteManagerDialog::OnRtePropertiesClick( wxCommandEvent &event )
{
    // Show routeproperties dialog for selected route
    long item = -1;
    item = m_pRouteListCtrl->GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( item == -1 ) return;

    Route *route = pRouteList->Item( m_pRouteListCtrl->GetItemData( item ) )->GetData();

    if( !route ) return;

    pRoutePropDialog = RouteProp::getInstance( GetParent() );

    pRoutePropDialog->SetRouteAndUpdate( route );
    pRoutePropDialog->UpdateProperties();
    if( !route->m_bIsInLayer )
        pRoutePropDialog->SetDialogTitle( _("Route Properties") );
    else {
        wxString caption( _T("Route Properties, Layer: ") );
        caption.Append( GetLayerName( route->m_LayerID ) );
        pRoutePropDialog->SetDialogTitle( caption );
    }

    if( !pRoutePropDialog->IsShown() )
        pRoutePropDialog->Show();

    m_bNeedConfigFlush = true;
}

void RouteManagerDialog::OnRteZoomtoClick( wxCommandEvent &event )
{
//      if (cc1->m_bFollow)
//            return;

    // Zoom into the bounding box of the selected route
    long item = -1;
    item = m_pRouteListCtrl->GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( item == -1 ) return;

    // optionally make this route exclusively visible
    if( m_bCtrlDown ) MakeAllRoutesInvisible();

    Route *route = pRouteList->Item( m_pRouteListCtrl->GetItemData( item ) )->GetData();

    if( !route ) return;

    // Ensure route is visible
    if( !route->IsVisible() ) {
        route->SetVisible( true );
        m_pRouteListCtrl->SetItemImage( item, route->IsVisible() ? 0 : 1 );
        pConfig->UpdateRoute( route );
    }

    ZoomtoRoute( route );
}

void RouteManagerDialog::OnRteReverseClick( wxCommandEvent &event )
{
    // Reverse selected route
    long item = -1;
    item = m_pRouteListCtrl->GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( item == -1 ) return;

    Route *route = pRouteList->Item( m_pRouteListCtrl->GetItemData( item ) )->GetData();

    if( !route ) return;
    if( route->m_bIsInLayer ) return;

    int ask_return = OCPNMessageBox( this, g_pRouteMan->GetRouteReverseMessage(), _("Rename Waypoints?"),
            wxYES_NO | wxCANCEL );
    if( ask_return != wxID_CANCEL ) {
        bool rename = ( ask_return == wxID_YES );

        pSelect->DeleteAllSelectableRouteSegments( route );
        route->Reverse( rename );
        pSelect->AddAllSelectableRouteSegments( route );

    // update column 2 - create a UpdateRouteItem(index) instead?
        wxString startend = route->m_RouteStartString;
        if( !route->m_RouteEndString.IsEmpty() ) startend.append( _(" - ") + route->m_RouteEndString );
        m_pRouteListCtrl->SetItem( item, 2, startend );

        pConfig->UpdateRoute( route );
        cc1->Refresh();
    }

    m_bNeedConfigFlush = true;
}

void RouteManagerDialog::OnRteExportClick( wxCommandEvent &event )
{
    RouteList list;

    wxString suggested_name = _T("routes");

    long item = -1;
    for ( ;; )
    {
        item = m_pRouteListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if ( item == -1 )
            break;

        Route *proute_to_export = pRouteList->Item( m_pRouteListCtrl->GetItemData( item ) )->GetData();

        if( proute_to_export ) {
            list.Append( proute_to_export );
            if( proute_to_export->m_RouteNameString != wxEmptyString )
                suggested_name = proute_to_export->m_RouteNameString;
        }
    }

    pConfig->ExportGPXRoutes( this, &list, suggested_name );
}

void RouteManagerDialog::OnRteActivateClick( wxCommandEvent &event )
{
    // Activate the selected route, unless it already is
    long item = -1;
    item = m_pRouteListCtrl->GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( item == -1 ) return;

    if( m_bCtrlDown ) MakeAllRoutesInvisible();

    Route *route = pRouteList->Item( m_pRouteListCtrl->GetItemData( item ) )->GetData();

    if( !route ) return;

    if( !route->m_bRtIsActive ) {
        if( !route->IsVisible() ) {
            route->SetVisible( true );
            m_pRouteListCtrl->SetItemImage( item, 0, 0 );
        }

        ZoomtoRoute( route );

        RoutePoint *best_point = g_pRouteMan->FindBestActivatePoint( route, gLat, gLon, gCog,
                gSog );
        g_pRouteMan->ActivateRoute( route, best_point );
//            g_pRouteMan->ActivateRoute(route);
    } else
        g_pRouteMan->DeactivateRoute();

    UpdateRouteListCtrl();

    pConfig->UpdateRoute( route );

    cc1->Refresh();

//      btnRteActivate->SetLabel(route->m_bRtIsActive ? _("Deactivate") : _("Activate"));

    m_bNeedConfigFlush = true;
}

void RouteManagerDialog::OnRteToggleVisibility( wxMouseEvent &event )
{
    wxPoint pos = event.GetPosition();
    int flags = 0;
    long clicked_index = m_pRouteListCtrl->HitTest( pos, flags );

    //    Clicking Visibility column?
    if( clicked_index > -1 && event.GetX() < m_pRouteListCtrl->GetColumnWidth( rmVISIBLE ) ) {
        // Process the clicked item
        Route *route = pRouteList->Item( m_pRouteListCtrl->GetItemData( clicked_index ) )->GetData();

        int wpts_set_viz = wxID_YES;
        bool togglesharedwpts = true;
        bool has_shared_wpts = g_pRouteMan->DoesRouteContainSharedPoints(route);
        
        if( has_shared_wpts && route->IsVisible() ) {
            wpts_set_viz = OCPNMessageBox(  this, _("Do you also want to make the shared waypoints being part of this route invisible?"), _("Question"), wxYES_NO );
            togglesharedwpts = (wpts_set_viz == wxID_YES);
        }
        route->SetVisible( !route->IsVisible(), togglesharedwpts );
        m_pRouteListCtrl->SetItemImage( clicked_index, route->IsVisible() ? 0 : 1 );

        ::wxBeginBusyCursor();

        pConfig->UpdateRoute( route );
        cc1->Refresh();

        //   We need to update the waypoint list control only if the visibility of shared waypoints might have changed.
        if( has_shared_wpts )
            UpdateWptListCtrlViz();

        ::wxEndBusyCursor();

    }

    // Allow wx to process...
    event.Skip();
}

// FIXME add/remove route segments/waypoints from selectable items, so there are no
// hidden selectables! This should probably be done outside this class!
// The problem is that the current waypoint class does not provide good support
// for this, there is a "visible" property, but no means for proper management.
// Jan. 28 2010: Ideas:
// - Calculate on the fly how many visible routes use a waypoint.
//   This requires a semidouble loop (routes, waypoints in visible routes). It could
//   be done by the function getting the selection. Potentially somewhat slow?
// - OR keep a property in waypoints telling that
//   (A number, increased/decreased for each waypoint by Route::SetVisible()).
//   Immediate result when detecting the selectable object, small overhead in
//   Route::SetVisible(). I prefer this.
// - We also need to know if the waypoint should otherwise be visible,
//   ie it is a "normal" waypoint used in the route (then it should be visible
//   in all cases). Is this possible with current code?
// - Get rid of the Select objects, they do no good! They should be replaced with a function
//   in the application, the search would reqire equal amount of looping, but less
//   dereferencing pointers, and it would remove the overhead of keeping and maintaining
//   the extra pointer lists.

void RouteManagerDialog::OnRteBtnLeftDown( wxMouseEvent &event )
{
    m_bCtrlDown = event.ControlDown();
    event.Skip();
}

void RouteManagerDialog::OnRteSelected( wxListEvent &event )
{
    long clicked_index = event.m_itemIndex;
    // Process the clicked item
    Route *route = pRouteList->Item( m_pRouteListCtrl->GetItemData( clicked_index ) )->GetData();
//    route->SetVisible(!route->IsVisible());
    m_pRouteListCtrl->SetItemImage( clicked_index, route->IsVisible() ? 0 : 1 );
//    pConfig->UpdateRoute(route);

    if( cc1 )
        cc1->Refresh();

    UpdateRteButtons();

}

void RouteManagerDialog::OnRteColumnClicked( wxListEvent &event )
{
    if( event.m_col == 1 ) {
        sort_route_name_dir++;
        m_pRouteListCtrl->SortItems( SortRoutesOnName, (wxIntPtr) m_pRouteListCtrl );
    } else
        if( event.m_col == 2 ) {
            sort_route_to_dir++;
            m_pRouteListCtrl->SortItems( SortRoutesOnTo, (wxIntPtr) m_pRouteListCtrl );
        }
}

void RouteManagerDialog::OnRteSendToGPSClick( wxCommandEvent &event )
{
    long item = -1;
    item = m_pRouteListCtrl->GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( item == -1 ) return;

    Route *route = pRouteList->Item( m_pRouteListCtrl->GetItemData( item ) )->GetData();

    if( !route ) return;

    SendToGpsDlg *pdlg = new SendToGpsDlg();
    pdlg->SetRoute( route );

    wxString source;
    pdlg->Create( NULL, -1, _( "Send To GPS..." ), source );
    
#ifdef __WXOSX__
    HideWithEffect(wxSHOW_EFFECT_BLEND );
#endif
    
        pdlg->ShowModal();
        
#ifdef __WXOSX__
    ShowWithEffect(wxSHOW_EFFECT_BLEND );
#endif
        
    pdlg->Destroy();
}

void RouteManagerDialog::OnRteDefaultAction( wxListEvent &event )
{
    wxCommandEvent evt;
    OnRtePropertiesClick( evt );
}

void RouteManagerDialog::OnTrkDefaultAction( wxListEvent &event )
{
    wxCommandEvent evt;
    OnTrkPropertiesClick( evt );
}

void RouteManagerDialog::OnTrkRightClick( wxListEvent &event )
{
    wxMenu menu;
    wxMenuItem* mergeItem = menu.Append( TRACK_MERGE, _("&Merge Selected Tracks") );
    mergeItem->Enable( m_pTrkListCtrl->GetSelectedItemCount() > 1 );
    wxMenuItem* cleanItem = menu.Append( TRACK_CLEAN, _("Reduce Data...") );
    cleanItem->Enable( m_pTrkListCtrl->GetSelectedItemCount() == 1 );
    wxMenuItem* copyItem = menu.Append( TRACK_COPY_TEXT, _("&Copy as text") );
    copyItem->Enable( m_pTrkListCtrl->GetSelectedItemCount() > 0 );
    PopupMenu( &menu );
}

WX_DEFINE_ARRAY( Track*, TrackArray );

static int CompareTracks( Track** track1, Track** track2 )
{
    TrackPoint* start1 = ( *track1 )->GetPoint(0);
    TrackPoint* start2 = ( *track2 )->GetPoint(0);
    if( start1->GetCreateTime() > start2->GetCreateTime() ) return 1;
    return -1; // Two tracks starting at the same time is not possible.
}

void RouteManagerDialog::OnTrkMenuSelected( wxCommandEvent &event )
{
    int item = -1;

    switch( event.GetId() ) {

        case TRACK_CLEAN: {
            item = m_pTrkListCtrl->GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
            if( item == -1 ) break;
            Track* track = pTrackList->Item( m_pTrkListCtrl->GetItemData( item ) )->GetData();
            if( track->IsRunning() ) {
                wxBell();
                break;
            }

            wxString choices[] = { _T("5.0"), _T("10.0"), _T("20.0"), _T("50.0"), _T("100.0") };
            wxSingleChoiceDialog precisionDlg ( this,
                    _("Select the maximum error allowed (in meters)\nafter data reduction:"),
                    _("Reduce Data Precision"), 5, choices );

            int result = precisionDlg.ShowModal();
            if( result == wxID_CANCEL ) break;
            double precision = 5.0;
            switch( precisionDlg.GetSelection() ) {
                case 0: precision = 5.0; break;
                case 1: precision = 10.0; break;
                case 2: precision = 20.0; break;
                case 3: precision = 50.0; break;
                case 4: precision = 100.0; break;
            }

            int pointsBefore = track->GetnPoints();

            int reduction = track->Simplify( precision );
            gFrame->Refresh( false );

            reduction = 100 * reduction / pointsBefore;
            wxString msg = wxString::Format( _("The amount of data used by the track\n was reduced by %d%%."),
                    reduction );
            OCPNMessageBox( this, msg, _("OpenCPN info"), wxICON_INFORMATION | wxOK );

            UpdateTrkListCtrl();
            UpdateRouteListCtrl();

            break;
        }

        case TRACK_COPY_TEXT: {
            wxString csvString;
            while( 1 ) {
                item = m_pTrkListCtrl->GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
                if( item == -1 ) break;
                Track *track = pTrackList->Item( m_pTrkListCtrl->GetItemData( item ) )->GetData();
                csvString << track->m_TrackNameString << _T("\t")
                          << wxString::Format( _T("%.1f"), track->Length() ) << _T("\t")
                        << _T("\n");
            }

            if( wxTheClipboard->Open() ) {
                wxTextDataObject* data = new wxTextDataObject;
                data->SetText( csvString );
                wxTheClipboard->SetData( data );
                wxTheClipboard->Close();
            }

            break;
        }

        case TRACK_MERGE: {
            Track* targetTrack = NULL;
            Track* mergeTrack = NULL;
            TrackPoint* tPoint;
            TrackPoint* newPoint;
            TrackPoint* lastPoint;
            TrackArray mergeList;
            TrackArray deleteList;
            bool runningSkipped = false;

            ::wxBeginBusyCursor();

            while( 1 ) {
                item = m_pTrkListCtrl->GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
                if( item == -1 ) break;
                Track* track = pTrackList->Item( m_pTrkListCtrl->GetItemData( item ) )->GetData();
                mergeList.Add( track );
            }

            mergeList.Sort( (CMPFUNC_wxArrayTrackArray) CompareTracks );

            targetTrack = mergeList.Item( 0 );
            lastPoint = targetTrack->GetLastPoint();

            for( unsigned int t = 1; t < mergeList.Count(); t++ ) {

                mergeTrack = mergeList.Item( t );

                if( mergeTrack->IsRunning() ) {
                    runningSkipped = true;
                    continue;
                }

                for(int i=0; i<mergeTrack->GetnPoints(); i++) {
                    tPoint = mergeTrack->GetPoint(i);
                    newPoint = new TrackPoint( tPoint->m_lat, tPoint->m_lon );
                    newPoint->m_GPXTrkSegNo = 1;

                    newPoint->SetCreateTime(tPoint->GetCreateTime());

                    targetTrack->AddPoint( newPoint );

                    pSelect->AddSelectableTrackSegment( lastPoint->m_lat, lastPoint->m_lon, newPoint->m_lat,
                            newPoint->m_lon, lastPoint, newPoint, targetTrack );

                    lastPoint = newPoint;
                }
                deleteList.Add( mergeTrack );
            }

            for( unsigned int i = 0; i < deleteList.Count(); i++ ) {
                Track* deleteTrack = deleteList.Item( i );
                g_pAIS->DeletePersistentTrack( deleteTrack );
                pConfig->DeleteConfigTrack( deleteTrack );
                g_pRouteMan->DeleteTrack( deleteTrack );
            }

            mergeList.Clear();
            deleteList.Clear();

            ::wxEndBusyCursor();

            UpdateTrkListCtrl();
            UpdateRouteListCtrl();
            cc1->Refresh();

            if( runningSkipped ) {
                wxMessageDialog skipWarning( this,
                        _("The currently running Track was not merged.\nYou can merge it later when it is completed."),
                        _T("Warning"), wxCANCEL | wxICON_WARNING );
                skipWarning.ShowModal();
            }

            break;
        }
    }
}

void RouteManagerDialog::UpdateTrkListCtrl()
{
    // if an item was selected, make it selected again if it still exist
    long item = -1;
    item = m_pTrkListCtrl->GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );

    long selected_id = -1;
    if( item != -1 ) selected_id = m_pTrkListCtrl->GetItemData( item );

    // Delete existing items
    m_pTrkListCtrl->DeleteAllItems();

    // then add tracks to the listctrl
    TrackList::iterator it;
    int index = 0;
    int list_index = 0;
    for( it = ( *pTrackList ).begin(); it != ( *pTrackList ).end(); ++it, ++index ) {;
        Track *trk = *it;
        if( !trk->IsListed() ) continue;

        wxListItem li;
        li.SetId( list_index );
        li.SetImage( trk->IsVisible() ? 0 : 1 );
        li.SetData( index );
        li.SetText( _T("") );

        if( g_pActiveTrack == trk ) {
            wxFont font = *wxNORMAL_FONT;
            font.SetWeight( wxFONTWEIGHT_BOLD );
            li.SetFont( font );
        }
        long idx = m_pTrkListCtrl->InsertItem( li );

        wxString name = trk->m_TrackNameString;
        if( name.IsEmpty() ) {
            TrackPoint *rp = trk->GetPoint( 0 );
            if( rp && rp->GetCreateTime().IsValid() ) name = rp->GetCreateTime().FormatISODate() + _T(" ")
                    + rp->GetCreateTime().FormatISOTime();   //name = rp->m_CreateTime.Format();
            else
                name = _("(Unnamed Track)");
        }
        m_pTrkListCtrl->SetItem( idx, colTRKNAME, name );

        wxString len;
        len.Printf( wxT("%5.2f"), trk->Length() );
        m_pTrkListCtrl->SetItem( idx, colTRKLENGTH, len );
        
        wxListItem lic;
        lic.SetId( list_index );
        lic.SetColumn(1);
        lic.SetAlign(wxLIST_FORMAT_LEFT);
        m_pTrkListCtrl->SetItem( lic );

        lic.SetColumn(2);
        lic.SetAlign(wxLIST_FORMAT_LEFT);
        m_pTrkListCtrl->SetItem( lic );

        list_index++;
        
    }

    m_pTrkListCtrl->SortItems( SortRoutesOnName, (wxIntPtr) m_pTrkListCtrl );

    m_pTrkListCtrl->SortItems( SortRoutesOnName, (wxIntPtr) m_pTrkListCtrl );
    m_pTrkListCtrl->SetColumnWidth(0, 4 * m_charWidth);
    
    // restore selection if possible
    // NOTE this will select a different item, if one is deleted
    // (the next route will get that index).
    if( selected_id > -1 ) {
        item = m_pTrkListCtrl->FindItem( -1, selected_id );
        m_pTrkListCtrl->SetItemState( item, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );
    }

    if( (m_lastTrkItem >= 0 ) && (m_pTrkListCtrl->GetItemCount()) )
        m_pTrkListCtrl->EnsureVisible( m_lastTrkItem );
    UpdateTrkButtons();
}

void RouteManagerDialog::OnTrkSelected( wxListEvent &event )
{
    UpdateTrkButtons();
}

void RouteManagerDialog::OnTrkColumnClicked( wxListEvent &event )
{
    if( event.m_col == 1 ) {
        sort_track_name_dir++;
        m_pTrkListCtrl->SortItems( SortTracksOnName, (wxIntPtr) m_pTrkListCtrl );
    } else
        if( event.m_col == 2 ) {
            sort_track_len_dir++;
            m_pTrkListCtrl->SortItems( SortTracksOnDistance, (wxIntPtr) m_pTrkListCtrl );
        }
}

void RouteManagerDialog::UpdateTrkButtons()
{
    long item = -1;
    item = m_pTrkListCtrl->GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    int items = m_pTrkListCtrl->GetSelectedItemCount();

    m_lastTrkItem = item;

    btnTrkProperties->Enable( items == 1 );
    btnTrkDelete->Enable( items >= 1 );
    btnTrkExport->Enable( items >= 1 );
    btnTrkRouteFromTrack->Enable( items == 1 );
    btnTrkDeleteAll->Enable( items >= 1 );
}

void RouteManagerDialog::OnTrkToggleVisibility( wxMouseEvent &event )
{
    wxPoint pos = event.GetPosition();
    int flags = 0;
    long clicked_index = m_pTrkListCtrl->HitTest( pos, flags );

    //    Clicking Visibility column?
    if( clicked_index > -1 && event.GetX() < m_pTrkListCtrl->GetColumnWidth( colTRKVISIBLE ) ) {
        // Process the clicked item
        wxTrackListNode *node = pTrackList->Item( m_pTrkListCtrl->GetItemData( clicked_index ) );
        if(node){
            Track *track = node->GetData();
            if(track){
                track->SetVisible( !track->IsVisible() );
                m_pTrkListCtrl->SetItemImage( clicked_index, track->IsVisible() ? 0 : 1 );
            }
        }
        
        cc1->Refresh();
    }

    // Allow wx to process...
    event.Skip();
}

void RouteManagerDialog::OnTrkNewClick( wxCommandEvent &event )
{
    gFrame->TrackOff();
    gFrame->TrackOn();

    UpdateTrkListCtrl();
}

void RouteManagerDialog::OnTrkPropertiesClick( wxCommandEvent &event )
{
    // Show trackproperties dialog for selected track
    long item = -1;
    item = m_pTrkListCtrl->GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( item == -1 ) return;

    Track *track = pTrackList->Item( m_pTrkListCtrl->GetItemData( item ) )->GetData();

    if( !track ) return;

    pTrackPropDialog = TrackPropDlg::getInstance( GetParent() );
    pTrackPropDialog->SetTrackAndUpdate( track );

    if( !pTrackPropDialog->IsShown() )
        pTrackPropDialog->Show();
    UpdateTrkListCtrl();

    m_bNeedConfigFlush = true;
}

void RouteManagerDialog::OnTrkDeleteClick( wxCommandEvent &event )
{
    TrackList list;

    int answer = OCPNMessageBox( this, _("Are you sure you want to delete the selected object(s)"), wxString( _("OpenCPN Alert") ), wxYES_NO );
    if ( answer != wxID_YES )
        return;

    bool busy = false;
    if( m_pTrkListCtrl->GetSelectedItemCount() ) {
        ::wxBeginBusyCursor();
        m_bNeedConfigFlush = true;
        busy = true;
    }

    long item = -1;
    for ( ;; )
    {
        item = m_pTrkListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if ( item == -1 )
            break;

        Track *ptrack_to_delete = pTrackList->Item( m_pTrkListCtrl->GetItemData( item ) )->GetData();

        if( ptrack_to_delete )
            list.Append( ptrack_to_delete );
    }

    if( busy ) {
        for(unsigned int i=0 ; i < list.GetCount() ; i++) {
            Track *track = (list.Item(i)->GetData());
            if( track ) {
                g_pAIS->DeletePersistentTrack( track );
                pConfig->DeleteConfigTrack( track );
                g_pRouteMan->DeleteTrack( track );
            }
        }

        m_lastTrkItem = -1;
//        UpdateRouteListCtrl();
        UpdateTrkListCtrl();

        cc1->undo->InvalidateUndo();
        cc1->Refresh();
        ::wxEndBusyCursor();
    }
}

void RouteManagerDialog::OnTrkExportClick( wxCommandEvent &event )
{
    TrackList list;
    wxString suggested_name = _T("tracks");

    long item = -1;
    for ( ;; )
    {
        item = m_pTrkListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if ( item == -1 )
            break;

        Track *ptrack_to_export = pTrackList->Item( m_pTrkListCtrl->GetItemData( item ) )->GetData();

        if( ptrack_to_export ) {
            list.Append( ptrack_to_export );
            if( ptrack_to_export->m_TrackNameString != wxEmptyString )
                suggested_name = ptrack_to_export->m_TrackNameString;
        }
    }

    pConfig->ExportGPXTracks( this, &list, suggested_name );
}

void RouteManagerDialog::TrackToRoute( Track *track )
{
    if( !track ) return;
    if( track->m_bIsInLayer ) return;

    wxProgressDialog *pprog = new wxProgressDialog( _("OpenCPN Converting Track to Route...."),
            _("Processing Waypoints..."), 101, NULL,
            wxPD_AUTO_HIDE | wxPD_SMOOTH | wxPD_ELAPSED_TIME | wxPD_ESTIMATED_TIME
                    | wxPD_REMAINING_TIME );

    ::wxBeginBusyCursor();

    Route *route = track->RouteFromTrack( pprog );

    pRouteList->Append( route );

    pprog->Update( 101, _("Done.") );
    delete pprog;

    cc1->Refresh();

    ::wxEndBusyCursor();
}

void RouteManagerDialog::OnTrkRouteFromTrackClick( wxCommandEvent &event )
{
    long item = -1;
    item = m_pTrkListCtrl->GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( item == -1 ) return;

    Track *track = pTrackList->Item( m_pTrkListCtrl->GetItemData( item ) )->GetData();
    
    TrackToRoute( track );
    
    UpdateRouteListCtrl();
}

void RouteManagerDialog::OnTrkDeleteAllClick( wxCommandEvent &event )
{
    int dialog_ret = OCPNMessageBox( this, _("Are you sure you want to delete <ALL> tracks?"),
            wxString( _("OpenCPN Alert") ), wxYES_NO );

    if( dialog_ret == wxID_YES ) {
        g_pRouteMan->DeleteAllTracks();
    }

    m_lastTrkItem = -1;
    m_lastRteItem = -1;

    UpdateTrkListCtrl();

    //    Also need to update the route list control, since routes and tracks share a common global list (pRouteList)
    UpdateRouteListCtrl();

    if( pRoutePropDialog ) pRoutePropDialog->Hide();

    cc1->Refresh();

    m_bNeedConfigFlush = true;
}

void RouteManagerDialog::UpdateWptListCtrl( RoutePoint *rp_select, bool b_retain_sort )
{
    wxIntPtr selected_id = -1;
    long item = -1;

    if( NULL == rp_select ) {
        // if an item was selected, make it selected again if it still exists
        item = m_pWptListCtrl->GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );

        if( item != -1 ) selected_id = m_pWptListCtrl->GetItemData( item );
    }

    //  Freshen the image list
    m_pWptListCtrl->SetImageList( pWayPointMan->Getpmarkicon_image_list(), wxIMAGE_LIST_SMALL );
    
    m_pWptListCtrl->DeleteAllItems();

    wxRoutePointListNode *node = pWayPointMan->GetWaypointList()->GetFirst();

    int index = 0;
    while( node ) {
        RoutePoint *rp = node->GetData();
        if( rp && rp->IsListed() ) {
            if( rp->m_bIsInRoute && !rp->m_bKeepXRoute ) {
                node = node->GetNext();
                continue;
            }

            wxListItem li;
            li.SetId( index );
            li.SetImage( rp->IsVisible() ? pWayPointMan->GetIconIndex( rp->GetIconBitmap() )
                                    : pWayPointMan->GetXIconIndex( rp->GetIconBitmap() ) );
            li.SetData( rp );
            li.SetText( _T("") );
            long idx = m_pWptListCtrl->InsertItem( li );

            wxString name = rp->GetName();
            if( name.IsEmpty() ) name = _("(Unnamed Waypoint)");
            m_pWptListCtrl->SetItem( idx, colWPTNAME, name );

            double dst;
            DistanceBearingMercator( rp->m_lat, rp->m_lon, gLat, gLon, NULL, &dst );
            wxString dist;
            dist.Printf( _T("%5.2f ") + getUsrDistanceUnit(), toUsrDistance( dst ) );
            m_pWptListCtrl->SetItem( idx, colWPTDIST, dist );

            if( rp == rp_select ) selected_id = (wxIntPtr) rp_select; //index; //m_pWptListCtrl->GetItemData(item);

            wxListItem lic;
            lic.SetId( index );
            lic.SetColumn(1);
            lic.SetAlign(wxLIST_FORMAT_LEFT);
            m_pWptListCtrl->SetItem( lic );
        
            lic.SetColumn(2);
            lic.SetAlign(wxLIST_FORMAT_LEFT);
            m_pWptListCtrl->SetItem( lic );
        
            index++;
        }

        node = node->GetNext();
    }

    if( !b_retain_sort ) {
        m_pWptListCtrl->SortItems( SortWaypointsOnName, (wxIntPtr) m_pWptListCtrl );
        sort_wp_key = SORT_ON_NAME;
    } else {
        switch( sort_wp_key ){
            case SORT_ON_NAME:
                m_pWptListCtrl->SortItems( SortWaypointsOnName, (wxIntPtr) m_pWptListCtrl );
                break;
            case SORT_ON_DISTANCE:
                m_pWptListCtrl->SortItems( SortWaypointsOnDistance, (wxIntPtr) m_pWptListCtrl );
                break;
        }
    }

    if( selected_id > -1 ) {
        item = m_pWptListCtrl->FindItem( -1, selected_id );
        m_pWptListCtrl->SetItemState( item, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );
    }

    if( (m_lastWptItem >= 0) && (m_pWptListCtrl->GetItemCount()) )
        m_pWptListCtrl->EnsureVisible( m_lastWptItem );
    
    m_pWptListCtrl->SetColumnWidth(0, 4 * m_charWidth);
    
    UpdateWptButtons();
}

void RouteManagerDialog::UpdateWptListCtrlViz( )
{
    long item = -1;
    for ( ;; )
    {
        item = m_pWptListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_DONTCARE);
        if ( item == -1 )
            break;
        
        RoutePoint *pRP = (RoutePoint *)m_pWptListCtrl->GetItemData(item);
        int image = pRP->IsVisible() ? pWayPointMan->GetIconIndex( pRP->GetIconBitmap() )
        : pWayPointMan->GetXIconIndex( pRP->GetIconBitmap() ) ;
                        
        m_pWptListCtrl->SetItemImage(item, image);
    }
}


void RouteManagerDialog::OnWptDefaultAction( wxListEvent &event )
{
    wxCommandEvent evt;
    OnWptPropertiesClick( evt );
}

void RouteManagerDialog::OnWptSelected( wxListEvent &event )
{
    UpdateWptButtons();
}

void RouteManagerDialog::OnWptColumnClicked( wxListEvent &event )
{
    if( event.m_col == 1 ) {
        sort_wp_name_dir++;
        m_pWptListCtrl->SortItems( SortWaypointsOnName, (wxIntPtr) m_pWptListCtrl );
        sort_wp_key = SORT_ON_NAME;
    } else
        if( event.m_col == 2 ) {
            sort_wp_len_dir++;
            m_pWptListCtrl->SortItems( SortWaypointsOnDistance, (wxIntPtr) m_pWptListCtrl );
            sort_wp_key = SORT_ON_DISTANCE;
        }
}

void RouteManagerDialog::UpdateWptButtons()
{
    long item = -1;
    item = m_pWptListCtrl->GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    bool enable1 = ( m_pWptListCtrl->GetSelectedItemCount() == 1 );
    bool enablemultiple = ( m_pWptListCtrl->GetSelectedItemCount() >= 1 );

    if( enable1 )
        m_lastWptItem = item;
    else
        m_lastWptItem = -1;

    //  Check selection to see if it is in a layer
    //  If so, disable the "delete" button
    bool b_delete_enable = true;
    item = -1;
    for ( ;; )
    {
        item = m_pWptListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if ( item == -1 )
            break;

        RoutePoint *wp = (RoutePoint *) m_pWptListCtrl->GetItemData( item );

        if( wp && wp->m_bIsInLayer) {
            b_delete_enable = false;
            break;
        }
    }


    btnWptProperties->Enable( enable1 );
    btnWptZoomto->Enable( enable1 );
    btnWptDeleteAll->Enable( enablemultiple );
    btnWptDelete->Enable( b_delete_enable && enablemultiple );
    btnWptGoTo->Enable( enable1 );
    btnWptExport->Enable( enablemultiple );
    btnWptSendToGPS->Enable( enable1 );
}

void RouteManagerDialog::OnWptToggleVisibility( wxMouseEvent &event )
{
    wxPoint pos = event.GetPosition();
    int flags = 0;
    long clicked_index = m_pWptListCtrl->HitTest( pos, flags );

    //    Clicking Visibility column?
    if( clicked_index > -1 && event.GetX() < m_pWptListCtrl->GetColumnWidth( colTRKVISIBLE ) ) {
        // Process the clicked item
        RoutePoint *wp = (RoutePoint *) m_pWptListCtrl->GetItemData( clicked_index );

        wp->SetVisible( !wp->IsVisible() );
        m_pWptListCtrl->SetItemImage( clicked_index,
                                      wp->IsVisible() ? pWayPointMan->GetIconIndex( wp->GetIconBitmap() )
                                                      : pWayPointMan->GetXIconIndex( wp->GetIconBitmap() ) );

        pConfig->UpdateWayPoint( wp );

        cc1->Refresh();
    }

    // Allow wx to process...
    event.Skip();
}

void RouteManagerDialog::OnWptNewClick( wxCommandEvent &event )
{
    RoutePoint *pWP = new RoutePoint( gLat, gLon, g_default_wp_icon, wxEmptyString,
            GPX_EMPTY_STRING );
    pWP->m_bIsolatedMark = true;                      // This is an isolated mark
    pSelect->AddSelectableRoutePoint( gLat, gLon, pWP );
    pConfig->AddNewWayPoint( pWP, -1 );    // use auto next num
    cc1->Refresh( false );      // Needed for MSW, why not GTK??

    pMarkPropDialog = MarkInfoImpl::getInstance( GetParent() );
    
    pMarkPropDialog->SetRoutePoint( pWP );
    pMarkPropDialog->UpdateProperties();

    WptShowPropertiesDialog( pWP, GetParent() );
}

void RouteManagerDialog::OnWptPropertiesClick( wxCommandEvent &event )
{
    long item = -1;
    item = m_pWptListCtrl->GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( item == -1 ) return;

    RoutePoint *wp = (RoutePoint *) m_pWptListCtrl->GetItemData( item );

    if( !wp ) return;

    WptShowPropertiesDialog( wp, GetParent() );

    UpdateWptListCtrl();
    m_bNeedConfigFlush = true;
}

void RouteManagerDialog::WptShowPropertiesDialog( RoutePoint* wp, wxWindow* parent )
{
    // There is one global instance of the MarkProp Dialog
    pMarkPropDialog = MarkInfoImpl::getInstance( parent );

    pMarkPropDialog->SetRoutePoint( wp );
    pMarkPropDialog->UpdateProperties();
    if( wp->m_bIsInLayer ) {
        wxString caption( _("Waypoint Properties, Layer: ") );
        caption.Append( GetLayerName( wp->m_LayerID ) );
        pMarkPropDialog->SetDialogTitle( caption );
    } else
        pMarkPropDialog->SetDialogTitle( _("Waypoint Properties") );

    if( !pMarkPropDialog->IsShown() )
        pMarkPropDialog->Show();

}

void RouteManagerDialog::OnWptZoomtoClick( wxCommandEvent &event )
{
    long item = -1;
    item = m_pWptListCtrl->GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( item == -1 ) return;

    RoutePoint *wp = (RoutePoint *) m_pWptListCtrl->GetItemData( item );

    if( !wp ) return;

//      cc1->ClearbFollow();
//      cc1->SetViewPoint(wp->m_lat, wp->m_lon, cc1->GetVPScale(), 0, cc1->GetVPRotation(), CURRENT_RENDER);
//      cc1->Refresh();
    gFrame->JumpToPosition( wp->m_lat, wp->m_lon, cc1->GetVPScale() );

}

void RouteManagerDialog::OnWptDeleteClick( wxCommandEvent &event )
{
    RoutePointList list;

    int answer = OCPNMessageBox( this, _("Are you sure you want to delete the selected object(s)"), wxString( _("OpenCPN Alert") ), wxYES_NO );
    if ( answer != wxID_YES )
        return;

    bool busy = false;
    if( m_pWptListCtrl->GetSelectedItemCount() ) {
        ::wxBeginBusyCursor();
        m_bNeedConfigFlush = true;
        busy = true;
    }

    long item = -1;
    long item_last_selected = -1;
    for ( ;; )
    {
        item = m_pWptListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if ( item == -1 )
            break;

        item_last_selected = item;
        RoutePoint *wp = (RoutePoint *) m_pWptListCtrl->GetItemData( item );

        if( wp && !wp->m_bIsInLayer)
            list.Append( wp );
    }

    if( busy ) {
        for(unsigned int i=0 ; i < list.GetCount() ; i++) {
            RoutePoint *wp = list.Item(i)->GetData();
            if( wp ) {

                if ( wp->m_bIsInRoute )
                {
                    if ( wxID_YES == OCPNMessageBox(this,  _( "The waypoint you want to delete is used in a route, do you really want to delete it?" ), _( "OpenCPN Alert" ), wxYES_NO ))
                            pWayPointMan->DestroyWaypoint( wp );
                }
                else
                    pWayPointMan->DestroyWaypoint( wp );

            }
        }

        long item_next = m_pWptListCtrl->GetNextItem( item_last_selected );         // next in list
        RoutePoint *wp_next = NULL;
        if( item_next > -1 )
            wp_next = (RoutePoint *) m_pWptListCtrl->GetItemData( item_next );

        m_lastWptItem = item_next;

        UpdateRouteListCtrl();
        UpdateTrkListCtrl();
        UpdateWptListCtrl( wp_next, true );

        if( pMarkPropDialog ) {
            pMarkPropDialog->SetRoutePoint( NULL );
            pMarkPropDialog->UpdateProperties();
        }

        cc1->undo->InvalidateUndo();
        cc1->Refresh();
        ::wxEndBusyCursor();
    }

}

void RouteManagerDialog::OnWptGoToClick( wxCommandEvent &event )
{
    long item = -1;
    item = m_pWptListCtrl->GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( item == -1 ) return;

    RoutePoint *wp = (RoutePoint *) m_pWptListCtrl->GetItemData( item );

    if( !wp ) return;

    RoutePoint *pWP_src = new RoutePoint( gLat, gLon, g_default_wp_icon, wxEmptyString,
            GPX_EMPTY_STRING );
    pSelect->AddSelectableRoutePoint( gLat, gLon, pWP_src );

    Route *temp_route = new Route();
    pRouteList->Append( temp_route );

    temp_route->AddPoint( pWP_src );
    temp_route->AddPoint( wp );

    pSelect->AddSelectableRouteSegment( gLat, gLon, wp->m_lat, wp->m_lon, pWP_src, wp, temp_route );

    wxString name = wp->GetName();
    if( name.IsEmpty() ) name = _("(Unnamed Waypoint)");
    wxString rteName = _("Go to ");
    rteName.Append( name );
    temp_route->m_RouteNameString = rteName;
    temp_route->m_RouteStartString = _("Here");
    
    temp_route->m_RouteEndString = name;
    temp_route->m_bDeleteOnArrival = true;

    if( g_pRouteMan->GetpActiveRoute() ) g_pRouteMan->DeactivateRoute();

    g_pRouteMan->ActivateRoute( temp_route, wp );

    UpdateRouteListCtrl();
}

void RouteManagerDialog::OnWptExportClick( wxCommandEvent &event )
{
    RoutePointList list;

    wxString suggested_name = _T("waypoints");

    long item = -1;
    for ( ;; )
    {
        item = m_pWptListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if ( item == -1 )
            break;

        RoutePoint *wp = (RoutePoint *) m_pWptListCtrl->GetItemData( item );

        if( wp && !wp->m_bIsInLayer) {
            list.Append( wp );
            if( wp->GetName() != wxEmptyString )
                suggested_name = wp->GetName();
        }
    }

    pConfig->ExportGPXWaypoints( this, &list, suggested_name );
}

void RouteManagerDialog::OnWptSendToGPSClick( wxCommandEvent &event )
{
    long item = -1;
    item = m_pWptListCtrl->GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( item == -1 ) return;

    RoutePoint *wp = (RoutePoint *) m_pWptListCtrl->GetItemData( item );

    if( !wp ) return;

    SendToGpsDlg *pdlg = new SendToGpsDlg();
    pdlg->SetWaypoint( wp );

    wxString source;
    pdlg->Create( NULL, -1, _( "Send To GPS..." ), source );
    pdlg->ShowModal();

    delete pdlg;
}

void RouteManagerDialog::OnWptDeleteAllClick( wxCommandEvent &event )
{
    wxString prompt;
    int buttons, type;
    if ( !pWayPointMan->SharedWptsExist() )
    {
        prompt = _("Are you sure you want to delete <ALL> waypoints?");
        buttons = wxYES_NO;
        type = 1;
    }
    else
    {
        prompt = _("There are some waypoints used in routes or anchor alarms.\n Do you want to delete them as well?\n This will change the routes and disable the anchor alarms.\n Answering No keeps the waypoints used in routes or alarms.");
        buttons = wxYES_NO | wxCANCEL;
        type = 2;
    }
    int answer = OCPNMessageBox( this, prompt, wxString( _("OpenCPN Alert") ), buttons );
    if ( answer == wxID_YES )
        pWayPointMan->DeleteAllWaypoints( true );
    if ( answer == wxID_NO && type == 2 )
        pWayPointMan->DeleteAllWaypoints( false );          // only delete unused waypoints

    if( pMarkPropDialog ) {
        pMarkPropDialog->SetRoutePoint( NULL );
        pMarkPropDialog->UpdateProperties();
    }

    m_lastWptItem = -1;
    UpdateRouteListCtrl();
    UpdateWptListCtrl();
    cc1->undo->InvalidateUndo();
    cc1->Refresh();
}

void RouteManagerDialog::OnLaySelected( wxListEvent &event )
{
    UpdateLayButtons();
}

void RouteManagerDialog::OnLayColumnClicked( wxListEvent &event )
{
    if( event.m_col == 1 ) {
        sort_layer_name_dir++;
        m_pLayListCtrl->SortItems( SortLayersOnName, (wxIntPtr) m_pLayListCtrl );
    } else
        if( event.m_col == 2 ) {
            sort_layer_len_dir++;
            m_pLayListCtrl->SortItems( SortLayersOnSize, (wxIntPtr) m_pLayListCtrl );
        }
}

void RouteManagerDialog::UpdateLayButtons()
{
    long item = -1;
    item = m_pLayListCtrl->GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    bool enable = ( item != -1 );

    //btnLayProperties->Enable(false);
    btnLayDelete->Enable( enable );
    btnLayToggleChart->Enable( enable );
    btnLayToggleListing->Enable( enable );
    btnLayToggleNames->Enable( enable );

    if( item >= 0 ) {
        if( pLayerList->Item( m_pLayListCtrl->GetItemData( item ) )->GetData()->IsVisibleOnChart() ) btnLayToggleChart->SetLabel(
                _("Hide from chart") );
        else
            btnLayToggleChart->SetLabel( _("Show on chart") );

        if( pLayerList->Item( m_pLayListCtrl->GetItemData( item ) )->GetData()->HasVisibleNames() ) btnLayToggleNames->SetLabel(
                _("Hide WPT names") );
        else
            btnLayToggleNames->SetLabel( _("Show WPT names") );

        if( pLayerList->Item( m_pLayListCtrl->GetItemData( item ) )->GetData()->IsVisibleOnListing() ) btnLayToggleListing->SetLabel(
                _("Unlist contents") );
        else
            btnLayToggleListing->SetLabel( _("List contents") );
    } else {
        btnLayToggleChart->SetLabel( _("Show on chart") );
        btnLayToggleNames->SetLabel( _("Show WPT names") );
        btnLayToggleListing->SetLabel( _("List contents") );
    }
}

void RouteManagerDialog::OnLayToggleVisibility( wxMouseEvent &event )
{
    wxPoint pos = event.GetPosition();
    int flags = 0;
    long clicked_index = m_pLayListCtrl->HitTest( pos, flags );

    //    Clicking Visibility column?
    if( clicked_index > -1 && event.GetX() < m_pLayListCtrl->GetColumnWidth( colLAYVISIBLE ) ) {
        // Process the clicked item
        Layer *layer = pLayerList->Item( m_pLayListCtrl->GetItemData( clicked_index ) )->GetData();

        layer->SetVisibleOnChart( !layer->IsVisibleOnChart() );
        m_pLayListCtrl->SetItemImage( clicked_index, layer->IsVisibleOnChart() ? 0 : 1 );

        ToggleLayerContentsOnChart( layer );
    }

    // Allow wx to process...
    event.Skip();
}

void RouteManagerDialog::OnLayNewClick( wxCommandEvent &event )
{
    bool show_flag = g_bShowLayers;
    g_bShowLayers = true;
    
    pConfig->UI_ImportGPX( this, true, _T("") );
    
    g_bShowLayers = show_flag;

    UpdateRouteListCtrl();
    UpdateTrkListCtrl();
    UpdateWptListCtrl();
    UpdateLayListCtrl();
    cc1->Refresh();
}

void RouteManagerDialog::OnLayPropertiesClick( wxCommandEvent &event )
{
    // Show layer properties dialog for selected layer - todo
    long item = -1;
    item = m_pLayListCtrl->GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( item == -1 ) return;
}

void RouteManagerDialog::OnLayDeleteClick( wxCommandEvent &event )
{
    long item = -1;
    item = m_pLayListCtrl->GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( item == -1 ) return;

    Layer *layer = pLayerList->Item( m_pLayListCtrl->GetItemData( item ) )->GetData();

    if( !layer ) return;

    wxString prompt = _("Are you sure you want to delete this layer and <ALL> of its contents?");
    int answer = OCPNMessageBox( this, prompt, wxString( _("OpenCPN Alert") ), wxYES_NO );
    if ( answer == wxID_NO )
        return;
    
    // Process Tracks and Routes in this layer
    wxRouteListNode *node1 = pRouteList->GetFirst();
    while( node1 ) {
        Route *pRoute = node1->GetData();
        wxRouteListNode *next_node = node1->GetNext();
        if( pRoute->m_bIsInLayer && ( pRoute->m_LayerID == layer->m_LayerID ) ) {
            pRoute->m_bIsInLayer = false;
            pRoute->m_LayerID = 0;
            g_pRouteMan->DeleteRoute( pRoute );
        }
        node1 = next_node;
    }

    wxTrackListNode *node2 = pTrackList->GetFirst();
    while( node2 ) {
        Track *pTrack = node2->GetData();
        wxTrackListNode *next_node = node2->GetNext();
        if( pTrack->m_bIsInLayer && ( pTrack->m_LayerID == layer->m_LayerID ) ) {
            pTrack->m_bIsInLayer = false;
            pTrack->m_LayerID = 0;
            g_pRouteMan->DeleteTrack( pTrack );
        }
        node2 = next_node;
    }

    // Process waypoints in this layer
    wxRoutePointListNode *node = pWayPointMan->GetWaypointList()->GetFirst();
    wxRoutePointListNode *node3;

    while( node ) {
        node3 = node->GetNext();
        RoutePoint *rp = node->GetData();
        if( rp && ( rp->m_LayerID == layer->m_LayerID ) ) {
            rp->m_bIsInLayer = false;
            rp->m_LayerID = 0;
            pWayPointMan->DestroyWaypoint( rp, false );         // no need to update the change set on layer ops
        }

        node = node3;
        node3 = NULL;
    }

    if( pMarkPropDialog ) {
        pMarkPropDialog->SetRoutePoint( NULL );
        pMarkPropDialog->UpdateProperties();
    }

    pLayerList->DeleteObject( layer );

    UpdateRouteListCtrl();
    UpdateTrkListCtrl();
    UpdateWptListCtrl();
    UpdateLayListCtrl();

    cc1->Refresh();

    m_bNeedConfigFlush = false;
}

void RouteManagerDialog::OnLayToggleChartClick( wxCommandEvent &event )
{
    // Toggle  visibility on chart for selected layer
    long item = -1;
    item = m_pLayListCtrl->GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( item == -1 ) return;

    Layer *layer = pLayerList->Item( m_pLayListCtrl->GetItemData( item ) )->GetData();

    if( !layer ) return;

    layer->SetVisibleOnChart( !layer->IsVisibleOnChart() );
    m_pLayListCtrl->SetItemImage( item, layer->IsVisibleOnChart() ? 0 : 1 );

    ToggleLayerContentsOnChart( layer );
}

void RouteManagerDialog::ToggleLayerContentsOnChart( Layer *layer )
{
    // Process Tracks and Routes in this layer
    wxRouteListNode *node1 = pRouteList->GetFirst();
    while( node1 ) {
        Route *pRoute = node1->GetData();
        if( pRoute->m_bIsInLayer && ( pRoute->m_LayerID == layer->m_LayerID ) ) {
            pRoute->SetVisible( layer->IsVisibleOnChart() );
            pConfig->UpdateRoute( pRoute );
        }
        node1 = node1->GetNext();
    }

    wxTrackListNode *node2 = pTrackList->GetFirst();
    while( node2 ) {
        Track *pTrack = node2->GetData();
        if( pTrack->m_bIsInLayer && ( pTrack->m_LayerID == layer->m_LayerID ) )
            pTrack->SetVisible( layer->IsVisibleOnChart() );
        node2 = node2->GetNext();
    }

    // Process waypoints in this layer
    wxRoutePointListNode *node = pWayPointMan->GetWaypointList()->GetFirst();

    while( node ) {
        RoutePoint *rp = node->GetData();
        if( rp && ( rp->m_LayerID == layer->m_LayerID ) ) {
            rp->SetVisible( layer->IsVisibleOnChart() );
        }

        node = node->GetNext();
    }

    UpdateRouteListCtrl();
    UpdateTrkListCtrl();
    UpdateWptListCtrl();
    UpdateLayListCtrl();
    UpdateLayButtons();

    cc1->Refresh();
}

void RouteManagerDialog::OnLayToggleNamesClick( wxCommandEvent &event )
{
    // Toggle WPT names visibility on chart for selected layer
    long item = -1;
    item = m_pLayListCtrl->GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( item == -1 ) return;

    Layer *layer = pLayerList->Item( m_pLayListCtrl->GetItemData( item ) )->GetData();

    if( !layer ) return;

    layer->SetVisibleNames( !layer->HasVisibleNames() );

    ToggleLayerContentsNames( layer );
}

void RouteManagerDialog::ToggleLayerContentsNames( Layer *layer )
{
    // Process Tracks and Routes in this layer
    wxRouteListNode *node1 = pRouteList->GetFirst();
    while( node1 ) {
        Route *pRoute = node1->GetData();
        if( pRoute->m_bIsInLayer && ( pRoute->m_LayerID == layer->m_LayerID ) ) {
            wxRoutePointListNode *node = pRoute->pRoutePointList->GetFirst();
            RoutePoint *prp1 = node->GetData();
            while( node ) {
                prp1->m_bShowName = layer->HasVisibleNames();
                node = node->GetNext();
            }
        }
        node1 = node1->GetNext();
    }

    // Process waypoints in this layer
    wxRoutePointListNode *node = pWayPointMan->GetWaypointList()->GetFirst();

    while( node ) {
        RoutePoint *rp = node->GetData();
        if( rp && ( rp->m_LayerID == layer->m_LayerID ) ) {
            rp->SetNameShown( layer->HasVisibleNames() );
        }

        node = node->GetNext();
    }

    UpdateLayButtons();

    cc1->Refresh();
}

void RouteManagerDialog::OnLayToggleListingClick( wxCommandEvent &event )
{
    // Toggle  visibility on listing for selected layer
    long item = -1;
    item = m_pLayListCtrl->GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( item == -1 ) return;

    Layer *layer = pLayerList->Item( m_pLayListCtrl->GetItemData( item ) )->GetData();

    if( !layer ) return;

    layer->SetVisibleOnListing( !layer->IsVisibleOnListing() );

    ToggleLayerContentsOnListing( layer );
}

void RouteManagerDialog::ToggleLayerContentsOnListing( Layer *layer )
{
    ::wxBeginBusyCursor();

    // Process Tracks and Routes in this layer
    wxRouteListNode *node1 = pRouteList->GetFirst();
    while( node1 ) {
        Route *pRoute = node1->GetData();
        if( pRoute->m_bIsInLayer && ( pRoute->m_LayerID == layer->m_LayerID ) ) {
                pRoute->SetListed( layer->IsVisibleOnListing() );
        }
        node1 = node1->GetNext();
    }

    wxTrackListNode *node2 = pTrackList->GetFirst();
    while( node2 ) {
        Track *pTrack = node2->GetData();
        if( pTrack->m_bIsInLayer && ( pTrack->m_LayerID == layer->m_LayerID ) )
            pTrack->SetListed( layer->IsVisibleOnListing() );
        node2 = node2->GetNext();
    }

    // Process waypoints in this layer
    //  n.b.  If the waypoint belongs to a track, and is not shared, then do not list it.
    //  This is a performance optimization, allowing large track support.

    wxRoutePointListNode *node = pWayPointMan->GetWaypointList()->GetFirst();

    while( node ) {
        RoutePoint *rp = node->GetData();
        if( rp && rp->m_bIsolatedMark && ( rp->m_LayerID == layer->m_LayerID ) ) {
            rp->SetListed( layer->IsVisibleOnListing() );
        }

        node = node->GetNext();
    }

    UpdateRouteListCtrl();
    UpdateTrkListCtrl();
    UpdateWptListCtrl();
    UpdateLayListCtrl();

    ::wxEndBusyCursor();

    cc1->Refresh();
}

void RouteManagerDialog::OnLayDefaultAction( wxListEvent &event )
{
    wxCommandEvent evt;
    OnLayPropertiesClick( evt );
}

void RouteManagerDialog::UpdateLayListCtrl()
{
    // if an item was selected, make it selected again if it still exist
    long item = -1;
    item = m_pLayListCtrl->GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );

    long selected_id = -1;
    if( item != -1 ) selected_id = m_pLayListCtrl->GetItemData( item );

    // Delete existing items
    m_pLayListCtrl->DeleteAllItems();

    // then add routes to the listctrl
    LayerList::iterator it;
    int index = 0;
    for( it = ( *pLayerList ).begin(); it != ( *pLayerList ).end(); ++it, ++index ) {
        Layer *lay = (Layer *) ( *it );

        wxListItem li;
        li.SetId( index );
        li.SetImage( lay->IsVisibleOnChart() ? 0 : 1 );
        li.SetData( index );
        li.SetText( _T("") );

        long idx = m_pLayListCtrl->InsertItem( li );

        wxString name = lay->m_LayerName;
        if( name.IsEmpty() ) {
            //RoutePoint *rp = trk->GetPoint(1);
            //if (rp)
            //      name = rp->m_CreateTime.FormatISODate() + _T(" ") + rp->m_CreateTime.FormatISOTime();   //name = rp->m_CreateTime.Format();
            //else
            name = _("(Unnamed Layer)");
        }
        m_pLayListCtrl->SetItem( idx, colLAYNAME, name );

        wxString len;
        len.Printf( wxT("%d"), (int) lay->m_NoOfItems );
        m_pLayListCtrl->SetItem( idx, colLAYITEMS, len );
        
        wxListItem lic;
        lic.SetId( index );
        lic.SetColumn(1);
        lic.SetAlign(wxLIST_FORMAT_LEFT);
        m_pLayListCtrl->SetItem( lic );
        
        lic.SetColumn(2);
        lic.SetAlign(wxLIST_FORMAT_LEFT);
        m_pLayListCtrl->SetItem( lic );
        
    }

    m_pLayListCtrl->SortItems( SortLayersOnName, (wxIntPtr) m_pLayListCtrl );
    m_pLayListCtrl->SetColumnWidth(0, 4 * m_charWidth);
    
    // restore selection if possible
    // NOTE this will select a different item, if one is deleted
    // (the next route will get that index).
    if( selected_id > -1 ) {
        item = m_pLayListCtrl->FindItem( -1, selected_id );
        m_pLayListCtrl->SetItemState( item, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );
    }
    UpdateLayButtons();
}

void RouteManagerDialog::OnImportClick( wxCommandEvent &event )
{
    // Import routes
    // FIXME there is no way to instruct this function about what to import.
    // Suggest to add that!
    
    pConfig->UI_ImportGPX( this );
    
    UpdateRouteListCtrl();
    UpdateTrkListCtrl();
    UpdateWptListCtrl();
    UpdateLayListCtrl();

    cc1->Refresh();
}
void RouteManagerDialog::OnExportClick( wxCommandEvent &event )
{
    pConfig->ExportGPX( this );
}

void RouteManagerDialog::OnExportVizClick( wxCommandEvent &event )
{
    pConfig->ExportGPX( this, true, true );     // only visible objects, layers included
}

//END Event handlers
