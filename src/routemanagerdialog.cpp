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

enum { rmVISIBLE = 0, rmROUTENAME, rmROUTEDESC };// RMColumns;
enum { colTRKVISIBLE = 0, colTRKNAME, colTRKLENGTH };
enum { colLAYVISIBLE = 0, colLAYNAME, colLAYITEMS };
enum { colWPTICON = 0, colWPTNAME, colWPTDIST };

// GLOBALS :0
extern RouteList *pRouteList;
extern LayerList *pLayerList;
extern wxString GetLayerName(int id);
extern RouteProp *pRoutePropDialog;
extern Routeman  *g_pRouteMan;
extern MyConfig  *pConfig;
extern ChartCanvas *cc1;
extern ChartBase *Current_Ch;
extern Track     *g_pActiveTrack;
extern WayPointman      *pWayPointMan;
extern MarkInfoImpl     *pMarkPropDialog;
extern MyFrame          *gFrame;
extern Select           *pSelect;
extern double           gLat, gLon;
extern double           gCog, gSog;
extern bool             g_bShowLayers;
extern wxString         g_default_wp_icon;

// sort callback. Sort by route name.
int sort_route_name_dir;
#if wxCHECK_VERSION(2, 9, 0)
int wxCALLBACK SortRoutesOnName(long item1, long item2, wxIntPtr list)
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
int wxCALLBACK SortRoutesOnTo(long item1, long item2, wxIntPtr list)
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
int wxCALLBACK SortTracksOnName(long item1, long item2, wxIntPtr list)
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
int wxCALLBACK SortTracksOnDistance(long item1, long item2, wxIntPtr list)
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
int wxCALLBACK SortWaypointsOnName(long item1, long item2, wxIntPtr list)
#else
int wxCALLBACK SortWaypointsOnName(long item1, long item2, long list)
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

    if(sort_wp_name_dir & 1)
    return it2.GetText().CmpNoCase(it1.GetText());
    else
    return it1.GetText().CmpNoCase(it2.GetText());
}

// sort callback. Sort by wpt distance.
int sort_wp_len_dir;
#if wxCHECK_VERSION(2, 9, 0)
int wxCALLBACK SortWaypointsOnDistance(long item1, long item2, wxIntPtr list)
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
int wxCALLBACK SortLayersOnName(long item1, long item2, wxIntPtr list)
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
int wxCALLBACK SortLayersOnSize(long item1, long item2, wxIntPtr list)
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

// event table. Empty, because I find it much easier to see what is connected to what
// using Connect() where possible, so that it is visible in the code.
BEGIN_EVENT_TABLE(RouteManagerDialog, wxDialog)
//EVT_NOTEBOOK_PAGE_CHANGED(wxID_ANY, RouteManagerDialog::OnTabSwitch) // This should work under Windows :-(
END_EVENT_TABLE()

void RouteManagerDialog::OnTabSwitch( wxNotebookEvent &event )
{
    if( !m_pNotebook ) return;
    int current_page = m_pNotebook->GetSelection();
    if( current_page == 3 ) {
        if( btnImport ) btnImport->Enable( false );
        if( btnExport ) btnExport->Enable( false );
    } else {
        if( btnImport ) btnImport->Enable( true );
        if( btnExport ) btnExport->Enable( true );
    }
    event.Skip(); // remove if using event table... why?
}

// implementation
RouteManagerDialog::RouteManagerDialog( wxWindow *parent )
{
    long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER;
#ifdef __WXOSX__
    style |= wxSTAY_ON_TOP;
#endif

    wxDialog::Create( parent, -1, wxString( _("Route Manager") ), wxDefaultPosition, wxDefaultSize,
            style );

    m_lastWptItem = -1;
    m_lastTrkItem = -1;
    m_lastRteItem = -1;

    Create();

}

void RouteManagerDialog::Create()
{
    wxBoxSizer* itemBoxSizer1 = new wxBoxSizer( wxVERTICAL );
    SetSizer( itemBoxSizer1 );

    m_pNotebook = new wxNotebook( this, wxID_ANY, wxDefaultPosition, wxSize( -1, -1 ), wxNB_TOP );
    itemBoxSizer1->Add( m_pNotebook, 1,
            wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL | wxALL | wxEXPAND, 5 );

    //  Create "Routes" panel
    m_pPanelRte = new wxPanel( m_pNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize,
            wxNO_BORDER | wxTAB_TRAVERSAL );
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
    m_pRouteListCtrl = new wxListCtrl( m_pPanelRte, -1, wxDefaultPosition, wxSize( 400, -1 ),
            wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_SORT_ASCENDING | wxLC_HRULES
                    | wxBORDER_SUNKEN/*|wxLC_VRULES*/);
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

    m_pRouteListCtrl->InsertColumn( rmVISIBLE, _T(""), wxLIST_FORMAT_LEFT, 28 );
    m_pRouteListCtrl->InsertColumn( rmROUTENAME, _("Route Name"), wxLIST_FORMAT_LEFT, 120 );
    m_pRouteListCtrl->InsertColumn( rmROUTEDESC, _("To"), wxLIST_FORMAT_LEFT, 230 );
    /*Seth
     wxListItem itemCol;
     itemCol.SetImage(-1);
     m_pRouteListCtrl->InsertColumn(rmVISIBLE, itemCol);
     //     m_pRouteListCtrl->InsertColumn(rmVISIBLE, wxT(""));
     m_pRouteListCtrl->SetColumnWidth(rmVISIBLE, 28);

     m_pRouteListCtrl->InsertColumn(rmROUTENAME, _("Route Name"));
     m_pRouteListCtrl->InsertColumn(rmROUTEDESC, _("To"));
     */

    // Buttons: Delete, Properties...
    wxBoxSizer *bsRouteButtons = new wxBoxSizer( wxVERTICAL );
    sbsRoutes->Add( bsRouteButtons, 0, wxALIGN_RIGHT );

    btnRteProperties = new wxButton( m_pPanelRte, -1, _("&Properties...") );
    bsRouteButtons->Add( btnRteProperties, 0, wxALL | wxEXPAND, DIALOG_MARGIN );
    btnRteProperties->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(RouteManagerDialog::OnRtePropertiesClick), NULL, this );

    btnRteActivate = new wxButton( m_pPanelRte, -1, _("&Activate") );
    bsRouteButtons->Add( btnRteActivate, 0, wxALL | wxEXPAND, DIALOG_MARGIN );
    btnRteActivate->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(RouteManagerDialog::OnRteActivateClick), NULL, this );
    btnRteActivate->Connect( wxEVT_LEFT_DOWN,
            wxMouseEventHandler(RouteManagerDialog::OnRteBtnLeftDown), NULL, this );

    btnRteZoomto = new wxButton( m_pPanelRte, -1, _("&Center View") );
    bsRouteButtons->Add( btnRteZoomto, 0, wxALL | wxEXPAND, DIALOG_MARGIN );
    btnRteZoomto->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(RouteManagerDialog::OnRteZoomtoClick), NULL, this );
    btnRteZoomto->Connect( wxEVT_LEFT_DOWN,
            wxMouseEventHandler(RouteManagerDialog::OnRteBtnLeftDown), NULL, this );

    btnRteReverse = new wxButton( m_pPanelRte, -1, _("&Reverse") );
    bsRouteButtons->Add( btnRteReverse, 0, wxALL | wxEXPAND, DIALOG_MARGIN );
    btnRteReverse->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(RouteManagerDialog::OnRteReverseClick), NULL, this );

    btnRteDelete = new wxButton( m_pPanelRte, -1, _("&Delete") );
    bsRouteButtons->Add( btnRteDelete, 0, wxALL | wxEXPAND, DIALOG_MARGIN );
    btnRteDelete->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(RouteManagerDialog::OnRteDeleteClick), NULL, this );

    btnRteExport = new wxButton( m_pPanelRte, -1, _("&Export Route...") );
    bsRouteButtons->Add( btnRteExport, 0, wxALL | wxEXPAND, DIALOG_MARGIN );
    btnRteExport->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(RouteManagerDialog::OnRteExportClick), NULL, this );

    btnRteSendToGPS = new wxButton( m_pPanelRte, -1, _("&Send to GPS") );
    bsRouteButtons->Add( btnRteSendToGPS, 0, wxALL | wxEXPAND, DIALOG_MARGIN );
    btnRteSendToGPS->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(RouteManagerDialog::OnRteSendToGPSClick), NULL, this );

    bsRouteButtons->AddSpacer( 10 );

    btnRteDeleteAll = new wxButton( m_pPanelRte, -1, _("&Delete All") );
    bsRouteButtons->Add( btnRteDeleteAll, 0, wxALL | wxEXPAND, DIALOG_MARGIN );
    btnRteDeleteAll->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(RouteManagerDialog::OnRteDeleteAllClick), NULL, this );

    //  Create "Tracks" panel
    m_pPanelTrk = new wxPanel( m_pNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize,
            wxNO_BORDER | wxTAB_TRAVERSAL );
    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer( wxHORIZONTAL );
    m_pPanelTrk->SetSizer( itemBoxSizer3 );
    m_pNotebook->AddPage( m_pPanelTrk, _("Tracks") );

    m_pTrkListCtrl = new wxListCtrl( m_pPanelTrk, -1, wxDefaultPosition, wxSize( 400, -1 ),
            wxLC_REPORT | wxLC_SORT_ASCENDING | wxLC_HRULES | wxBORDER_SUNKEN/*|wxLC_VRULES*/);
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

    m_pTrkListCtrl->InsertColumn( colTRKVISIBLE, _T(""), wxLIST_FORMAT_LEFT, 28 );
    m_pTrkListCtrl->InsertColumn( colTRKNAME, _("Track Name"), wxLIST_FORMAT_LEFT, 250 );
    m_pTrkListCtrl->InsertColumn( colTRKLENGTH, _("Length"), wxLIST_FORMAT_LEFT, 100 );

    wxBoxSizer *bsTrkButtons = new wxBoxSizer( wxVERTICAL );
    itemBoxSizer3->Add( bsTrkButtons, 0, wxALIGN_RIGHT );

    btnTrkNew = new wxButton( m_pPanelTrk, -1, _("&Start Track") );
    bsTrkButtons->Add( btnTrkNew, 0, wxALL | wxEXPAND, DIALOG_MARGIN );
    btnTrkNew->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(RouteManagerDialog::OnTrkNewClick), NULL, this );

    btnTrkProperties = new wxButton( m_pPanelTrk, -1, _("&Properties") );
    bsTrkButtons->Add( btnTrkProperties, 0, wxALL | wxEXPAND, DIALOG_MARGIN );
    btnTrkProperties->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(RouteManagerDialog::OnTrkPropertiesClick), NULL, this );

    btnTrkDelete = new wxButton( m_pPanelTrk, -1, _("&Delete") );
    bsTrkButtons->Add( btnTrkDelete, 0, wxALL | wxEXPAND, DIALOG_MARGIN );
    btnTrkDelete->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(RouteManagerDialog::OnTrkDeleteClick), NULL, this );

    btnTrkExport = new wxButton( m_pPanelTrk, -1, _("&Export Track...") );
    bsTrkButtons->Add( btnTrkExport, 0, wxALL | wxEXPAND, DIALOG_MARGIN );
    btnTrkExport->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(RouteManagerDialog::OnTrkExportClick), NULL, this );

    btnTrkRouteFromTrack = new wxButton( m_pPanelTrk, -1, _("Route from Track") );
    bsTrkButtons->Add( btnTrkRouteFromTrack, 0, wxALL | wxEXPAND, DIALOG_MARGIN );
    btnTrkRouteFromTrack->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(RouteManagerDialog::OnTrkRouteFromTrackClick), NULL, this );

    bsTrkButtons->AddSpacer( 10 );

    btnTrkDeleteAll = new wxButton( m_pPanelTrk, -1, _("&Delete All") );
    bsTrkButtons->Add( btnTrkDeleteAll, 0, wxALL | wxEXPAND, DIALOG_MARGIN );
    btnTrkDeleteAll->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(RouteManagerDialog::OnTrkDeleteAllClick), NULL, this );

    //  Create "Waypoints" panel
    m_pPanelWpt = new wxPanel( m_pNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize,
            wxNO_BORDER | wxTAB_TRAVERSAL );
    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer( wxHORIZONTAL );
    m_pPanelWpt->SetSizer( itemBoxSizer4 );
    m_pNotebook->AddPage( m_pPanelWpt, _("Waypoints") );

    m_pWptListCtrl = new wxListCtrl( m_pPanelWpt, -1, wxDefaultPosition, wxSize( 400, -1 ),
            wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_SORT_ASCENDING | wxLC_HRULES
                    | wxBORDER_SUNKEN/*|wxLC_VRULES*/);
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

    m_pWptListCtrl->InsertColumn( colWPTICON, _("Icon"), wxLIST_FORMAT_LEFT, 44 );
    m_pWptListCtrl->InsertColumn( colWPTNAME, _("Waypoint Name"), wxLIST_FORMAT_LEFT, 180 );
    m_pWptListCtrl->InsertColumn( colWPTDIST, _("Distance"), wxLIST_FORMAT_LEFT, 180 );

    wxBoxSizer *bsWptButtons = new wxBoxSizer( wxVERTICAL );
    itemBoxSizer4->Add( bsWptButtons, 0, wxALIGN_RIGHT );

    btnWptNew = new wxButton( m_pPanelWpt, -1, _("&New") );
    bsWptButtons->Add( btnWptNew, 0, wxALL | wxEXPAND, DIALOG_MARGIN );
    btnWptNew->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(RouteManagerDialog::OnWptNewClick), NULL, this );

    btnWptProperties = new wxButton( m_pPanelWpt, -1, _("&Properties") );
    bsWptButtons->Add( btnWptProperties, 0, wxALL | wxEXPAND, DIALOG_MARGIN );
    btnWptProperties->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(RouteManagerDialog::OnWptPropertiesClick), NULL, this );

    btnWptZoomto = new wxButton( m_pPanelWpt, -1, _("&Center View") );
    bsWptButtons->Add( btnWptZoomto, 0, wxALL | wxEXPAND, DIALOG_MARGIN );
    btnWptZoomto->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(RouteManagerDialog::OnWptZoomtoClick), NULL, this );

    btnWptDelete = new wxButton( m_pPanelWpt, -1, _("&Delete") );
    bsWptButtons->Add( btnWptDelete, 0, wxALL | wxEXPAND, DIALOG_MARGIN );
    btnWptDelete->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(RouteManagerDialog::OnWptDeleteClick), NULL, this );

    btnWptGoTo = new wxButton( m_pPanelWpt, -1, _("&Go To") );
    bsWptButtons->Add( btnWptGoTo, 0, wxALL | wxEXPAND, DIALOG_MARGIN );
    btnWptGoTo->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(RouteManagerDialog::OnWptGoToClick), NULL, this );

    btnWptExport = new wxButton( m_pPanelWpt, -1, _("&Export Wpt...") );
    bsWptButtons->Add( btnWptExport, 0, wxALL | wxEXPAND, DIALOG_MARGIN );
    btnWptExport->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(RouteManagerDialog::OnWptExportClick), NULL, this );

    btnWptSendToGPS = new wxButton( m_pPanelWpt, -1, _("&Send to GPS") );
    bsWptButtons->Add( btnWptSendToGPS, 0, wxALL | wxEXPAND, DIALOG_MARGIN );
    btnWptSendToGPS->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(RouteManagerDialog::OnWptSendToGPSClick), NULL, this );

    bsWptButtons->AddSpacer( 10 );

    btnWptDeleteAll = new wxButton( m_pPanelWpt, -1, _("Delete All") );
    bsWptButtons->Add( btnWptDeleteAll, 0, wxALL | wxEXPAND, DIALOG_MARGIN );
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
    btnExport = new wxButton( this, -1, _("E&xport All...") );
    itemBoxSizer6->Add( btnExport, 0, wxALL | wxALIGN_LEFT, DIALOG_MARGIN );
    btnExport->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(RouteManagerDialog::OnExportClick), NULL, this );

    //  Create "Layers" panel
    m_pPanelLay = new wxPanel( m_pNotebook, wxID_ANY, wxDefaultPosition, wxDefaultSize,
            wxNO_BORDER | wxTAB_TRAVERSAL );
    wxBoxSizer* itemBoxSizer7 = new wxBoxSizer( wxHORIZONTAL );
    m_pPanelLay->SetSizer( itemBoxSizer7 );
    m_pNotebook->AddPage( m_pPanelLay, _("Layers") );

    m_pLayListCtrl = new wxListCtrl( m_pPanelLay, -1, wxDefaultPosition, wxSize( 400, -1 ),
            wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_SORT_ASCENDING | wxLC_HRULES
                    | wxBORDER_SUNKEN/*|wxLC_VRULES*/);
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

    m_pLayListCtrl->InsertColumn( colLAYVISIBLE, _T(""), wxLIST_FORMAT_LEFT, 28 );
    m_pLayListCtrl->InsertColumn( colLAYNAME, _("Layer Name"), wxLIST_FORMAT_LEFT, 250 );
    m_pLayListCtrl->InsertColumn( colLAYITEMS, _("No. of items"), wxLIST_FORMAT_LEFT, 100 );

    wxBoxSizer *bsLayButtons = new wxBoxSizer( wxVERTICAL );
    itemBoxSizer7->Add( bsLayButtons, 0, wxALIGN_RIGHT );

    btnLayNew = new wxButton( m_pPanelLay, -1, _("Temporary layer") );
    bsLayButtons->Add( btnLayNew, 0, wxALL | wxEXPAND, DIALOG_MARGIN );
    btnLayNew->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(RouteManagerDialog::OnLayNewClick), NULL, this );

    //btnLayProperties = new wxButton(m_pPanelLay, -1, _("&Properties"));
    //bsLayButtons->Add(btnLayProperties, 0, wxALL|wxEXPAND, DIALOG_MARGIN);
    //btnLayProperties->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
    //                     wxCommandEventHandler(RouteManagerDialog::OnLayPropertiesClick), NULL, this);

    btnLayDelete = new wxButton( m_pPanelLay, -1, _("&Delete") );
    bsLayButtons->Add( btnLayDelete, 0, wxALL | wxEXPAND, DIALOG_MARGIN );
    btnLayDelete->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(RouteManagerDialog::OnLayDeleteClick), NULL, this );

    btnLayToggleChart = new wxButton( m_pPanelLay, -1, _("Show on chart") );
    bsLayButtons->Add( btnLayToggleChart, 0, wxALL | wxEXPAND, DIALOG_MARGIN );
    btnLayToggleChart->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(RouteManagerDialog::OnLayToggleChartClick), NULL, this );

    btnLayToggleNames = new wxButton( m_pPanelLay, -1, _("Show WPT names") );
    bsLayButtons->Add( btnLayToggleNames, 0, wxALL | wxEXPAND, DIALOG_MARGIN );
    btnLayToggleNames->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(RouteManagerDialog::OnLayToggleNamesClick), NULL, this );

    btnLayToggleListing = new wxButton( m_pPanelLay, -1, _("List contents") );
    bsLayButtons->Add( btnLayToggleListing, 0, wxALL | wxEXPAND, DIALOG_MARGIN );
    btnLayToggleListing->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler(RouteManagerDialog::OnLayToggleListingClick), NULL, this );

    // Dialog buttons
    wxSizer *szButtons = CreateButtonSizer( wxOK );

    itemBoxSizer5->Add( szButtons, 0, wxALL | wxALIGN_RIGHT, DIALOG_MARGIN );

    Fit();

    SetMinSize( GetBestSize() );

    // create a image list for the list with just the eye icon
    wxImageList *imglist = new wxImageList( 20, 20, true, 1 );
    imglist->Add( wxBitmap( eye ) );
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

    delete m_pNotebook;

    //    Does not need to be done here at all, since this dialog is autommatically deleted as a child of the frame.
    //    By that time, the config has already been updated for shutdown.

    // Do this just once!!
//      if (m_bNeedConfigFlush)
//            pConfig->UpdateSettings();
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
    for( it = ( *pRouteList ).begin(); it != ( *pRouteList ).end(); ++it, ++index ) {
        if( ( *it )->m_bIsTrack || !( *it )->IsListed() ) continue;

        wxListItem li;
        li.SetId( index );
        li.SetImage( ( *it )->IsVisible() ? 0 : -1 );
        li.SetData( index );
        li.SetText( _T("") );

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
        if( !( *it )->m_RouteEndString.IsEmpty() ) startend.append(
                _(" - ") + ( *it )->m_RouteEndString );
        m_pRouteListCtrl->SetItem( idx, rmROUTEDESC, startend );
    }

    m_pRouteListCtrl->SortItems( SortRoutesOnName, (long) m_pRouteListCtrl );

    // restore selection if possible
    // NOTE this will select a different item, if one is deleted
    // (the next route will get that index).
    if( selected_id > -1 ) {
        item = m_pRouteListCtrl->FindItem( -1, selected_id );
        m_pRouteListCtrl->SetItemState( item, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );
    }

    if( m_lastRteItem >= 0 ) m_pRouteListCtrl->EnsureVisible( m_lastRteItem );
    UpdateRteButtons();
}

void RouteManagerDialog::UpdateRteButtons()
{
    // enable/disable buttons
    long selected_index_index = m_pRouteListCtrl->GetNextItem( -1, wxLIST_NEXT_ALL,
            wxLIST_STATE_SELECTED );
    bool enable = !( selected_index_index < 0 );

    m_lastRteItem = selected_index_index;

    btnRteDelete->Enable( enable );
    btnRteZoomto->Enable( enable ); // && !cc1->m_bFollow);
    btnRteProperties->Enable( enable );
    btnRteReverse->Enable( enable );
    btnRteExport->Enable( enable );
    btnRteSendToGPS->Enable( enable );

    // set activate button text
    Route *route = NULL;
    if( enable ) route =
            pRouteList->Item( m_pRouteListCtrl->GetItemData( selected_index_index ) )->GetData();

    if( !g_pRouteMan->IsAnyRouteActive() ) {
        btnRteActivate->Enable( enable );
        if( enable ) btnRteActivate->SetLabel( _("Activate") );

    } else {
        if( enable ) {
            if( route && route->m_bRtIsActive ) {
                btnRteActivate->Enable( enable );
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
            m_pRouteListCtrl->SetItemImage( m_pRouteListCtrl->FindItem( -1, index ), -1 ); // Likely not same order :0
            pConfig->UpdateRoute( *it ); // auch, flushes config to disk. FIXME
        }
    }
}

void RouteManagerDialog::ZoomtoRoute( Route *route )
{

    // Calculate bbox center
    double clat = route->RBBox.GetMinY() + ( route->RBBox.GetHeight() / 2 );
    double clon = route->RBBox.GetMinX() + ( route->RBBox.GetWidth() / 2 );

    if( clon > 180. ) clon -= 360.;
    else
        if( clon < -180. ) clon += 360.;

    // Calculate ppm
    double rw, rh, ppm; // route width, height, final ppm scale to use
    int ww, wh; // chart window width, height
    // route bbox width in nm
    DistanceBearingMercator( route->RBBox.GetMinY(), route->RBBox.GetMinX(), route->RBBox.GetMinY(),
            route->RBBox.GetMaxX(), NULL, &rw );
    // route bbox height in nm
    DistanceBearingMercator( route->RBBox.GetMinY(), route->RBBox.GetMinX(), route->RBBox.GetMaxY(),
            route->RBBox.GetMinX(), NULL, &rh );

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
    long item = -1;
    item = m_pRouteListCtrl->GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( item == -1 ) return;

    Route *proute_to_delete = pRouteList->Item( m_pRouteListCtrl->GetItemData( item ) )->GetData();

    if( !proute_to_delete ) return;

    ::wxBeginBusyCursor();

    cc1->CancelMouseRoute();

    pConfig->DeleteConfigRoute( proute_to_delete );

    g_pRouteMan->DeleteRoute( proute_to_delete );

    ::wxEndBusyCursor();

    UpdateRouteListCtrl();

    //    Also need to update the track list control, since routes and tracks share a common global list (pRouteList)
    UpdateTrkListCtrl();

    cc1->Refresh();

    m_bNeedConfigFlush = true;
}

void RouteManagerDialog::OnRteDeleteAllClick( wxCommandEvent &event )
{
    OCPNMessageDialog mdlg( this, _("Are you sure you want to delete <ALL> routes?"),
            wxString( _("OpenCPN Alert") ), wxYES_NO );
    int dialog_ret = mdlg.ShowModal();

    if( dialog_ret == wxID_YES ) {
        if( g_pRouteMan->GetpActiveRoute() ) g_pRouteMan->DeactivateRoute();

        cc1->CancelMouseRoute();

        g_pRouteMan->DeleteAllRoutes();
// TODO Seth
//            m_pSelectedRoute = NULL;
//            m_pFoundRoutePoint = NULL;
//            m_pFoundRoutePointSecond = NULL;

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

    if( NULL == pRoutePropDialog )          // There is one global instance of the RouteProp Dialog
    pRoutePropDialog = new RouteProp( GetParent() );

    pRoutePropDialog->SetRouteAndUpdate( route );
    pRoutePropDialog->UpdateProperties();
    if( !route->m_bIsInLayer ) pRoutePropDialog->SetDialogTitle( _("Route Properties") );
    else {
        wxString caption( _T("Route Properties, Layer: ") );
        caption.Append( GetLayerName( route->m_LayerID ) );
        pRoutePropDialog->SetDialogTitle( caption );
    }

    if( !pRoutePropDialog->IsShown() ) pRoutePropDialog->ShowModal();
    // route might have changed
    UpdateRouteListCtrl();

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
        m_pRouteListCtrl->SetItemImage( item, route->IsVisible() ? 0 : -1 );
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

    OCPNMessageDialog ask( this, g_pRouteMan->GetRouteReverseMessage(), _("Rename Waypoints?"),
            wxYES_NO );
    bool rename = ( ask.ShowModal() == wxID_YES );

    pSelect->DeleteAllSelectableRouteSegments( route );
    route->Reverse( rename );
    pSelect->AddAllSelectableRouteSegments( route );

    // update column 2 - create a UpdateRouteItem(index) instead?
    wxString startend = route->m_RouteStartString;
    if( !route->m_RouteEndString.IsEmpty() ) startend.append( _(" - ") + route->m_RouteEndString );
    m_pRouteListCtrl->SetItem( item, 2, startend );

    pConfig->UpdateRoute( route );
//       pConfig->UpdateSettings(); // NOTE done once in destructor
    cc1->Refresh();

    m_bNeedConfigFlush = true;
}

void RouteManagerDialog::OnRteExportClick( wxCommandEvent &event )
{
    // Export selected route
    long item = -1;
    item = m_pRouteListCtrl->GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( item == -1 ) return;

    Route *route = pRouteList->Item( m_pRouteListCtrl->GetItemData( item ) )->GetData();

    if( !route ) return;

    pConfig->ExportGPXRoute( this, route );
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
        Route *route =
                pRouteList->Item( m_pRouteListCtrl->GetItemData( clicked_index ) )->GetData();
        bool wpts_set_viz = wxYES == wxMessageBox( _("Do you also want to toggle the visibility of shared waypoints being part of this route?"), _("Question"), wxYES_NO );
        route->SetVisible( !route->IsVisible(), wpts_set_viz );
        m_pRouteListCtrl->SetItemImage( clicked_index, route->IsVisible() ? 0 : -1 );

        ::wxBeginBusyCursor();

        pConfig->UpdateRoute( route );
        cc1->Refresh();

        UpdateWptListCtrl();

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
    m_pRouteListCtrl->SetItemImage( clicked_index, route->IsVisible() ? 0 : -1 );
//    pConfig->UpdateRoute(route);
    cc1->Refresh();

    UpdateRteButtons();

}

void RouteManagerDialog::OnRteColumnClicked( wxListEvent &event )
{
    if( event.m_col == 1 ) {
        sort_route_name_dir++;
        m_pRouteListCtrl->SortItems( SortRoutesOnName, (long) m_pRouteListCtrl );
    } else
        if( event.m_col == 2 ) {
            sort_route_to_dir++;
            m_pRouteListCtrl->SortItems( SortRoutesOnTo, (long) m_pRouteListCtrl );
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
//    if( g_pnmea ) g_pnmea->GetSource( source );

    pdlg->Create( NULL, -1, _( "Send To GPS..." ), source );
    pdlg->ShowModal();

    delete pdlg;
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

static int CompareTracks( const Track** track1, const Track** track2 )
{
    RoutePoint* start1 = ( *track1 )->pRoutePointList->GetFirst()->GetData();
    RoutePoint* start2 = ( *track2 )->pRoutePointList->GetFirst()->GetData();
    if( start1->m_CreateTime > start2->m_CreateTime ) return 1;
    return -1; // Two tracks starting at the same time is not possible.
}

void RouteManagerDialog::OnTrkMenuSelected( wxCommandEvent &event )
{
    int item = -1;

    switch( event.GetId() ) {

        case TRACK_CLEAN: {
            item = m_pTrkListCtrl->GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
            if( item == -1 ) break;
            Track* track = (Track*) pRouteList->Item( m_pTrkListCtrl->GetItemData( item ) )->GetData();
            if( track->IsRunning() ) {
                wxBell();
                break;
            }

            wxString choices[] = { _T("5.0"), _T("10.0"), _T("20.0"), _T("50.0"), _T("100.0") };
            wxSingleChoiceDialog* precisionDlg = new wxSingleChoiceDialog( this,
                    _("Select the maximum error allowed (in meters)\nafter data reduction:"),
                    _("Reduce Data Precision"), 5, choices );

            int result = precisionDlg->ShowModal();
            if( result == wxID_CANCEL ) break;
            double precision;
            switch( precisionDlg->GetSelection() ) {
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
            OCPNMessageDialog* dlg = new OCPNMessageDialog( this, msg, _("OpenCPN info"), wxICON_INFORMATION | wxOK );
            dlg->ShowModal();
            break;
        }

        case TRACK_COPY_TEXT: {
            wxString csvString;
            while( 1 ) {
                item = m_pTrkListCtrl->GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
                if( item == -1 ) break;
                Track* track = (Track*) pRouteList->Item( m_pTrkListCtrl->GetItemData( item ) )->GetData();
                csvString << track->m_RouteNameString << _T("\t")
                        << wxString::Format( _T("%.1f"), track->m_route_length ) << _T("\t")
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
            RoutePoint* rPoint;
            RoutePoint* newPoint;
            RoutePoint* lastPoint;
            wxRoutePointListNode* routePointNode;
            TrackArray mergeList;
            TrackArray deleteList;
            bool runningSkipped = false;

            ::wxBeginBusyCursor();

            while( 1 ) {
                item = m_pTrkListCtrl->GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
                if( item == -1 ) break;
                Track* track = (Track*) pRouteList->Item( m_pTrkListCtrl->GetItemData( item ) )->GetData();
                mergeList.Add( track );
            }

            mergeList.Sort( (CMPFUNC_wxArrayTrackArray) CompareTracks );

            targetTrack = (Track *) mergeList.Item( 0 );
            lastPoint = targetTrack->GetLastPoint();

            for( unsigned int t = 1; t < mergeList.Count(); t++ ) {

                mergeTrack = (Track *) mergeList.Item( t );

                if( mergeTrack->IsRunning() ) {
                    runningSkipped = true;
                    continue;
                }

                routePointNode = mergeTrack->pRoutePointList->GetFirst();

                while( routePointNode ) {
                    rPoint = routePointNode->GetData();
                    newPoint = new RoutePoint( rPoint->m_lat, rPoint->m_lon, wxString( _T("empty") ),
                            wxString( _T("") ), GPX_EMPTY_STRING );
                    newPoint->m_bShowName = false;
                    newPoint->m_bIsVisible = true;
                    newPoint->m_GPXTrkSegNo = 1;

                    newPoint->m_CreateTime = rPoint->m_CreateTime;

                    targetTrack->AddPoint( newPoint );

                    newPoint->m_bIsInRoute = false;
                    newPoint->m_bIsInTrack = true;

                    pSelect->AddSelectableTrackSegment( lastPoint->m_lat, lastPoint->m_lon, newPoint->m_lat,
                            newPoint->m_lon, lastPoint, newPoint, targetTrack );

                    lastPoint = newPoint;

                    routePointNode = routePointNode->GetNext();
                }
                deleteList.Add( mergeTrack );
            }

            for( unsigned int i = 0; i < deleteList.Count(); i++ ) {
                Track* deleteTrack = (Track*) deleteList.Item( i );
                pConfig->DeleteConfigRoute( deleteTrack );
                g_pRouteMan->DeleteTrack( deleteTrack );
            }

            mergeList.Clear();
            deleteList.Clear();

            ::wxEndBusyCursor();

            UpdateTrkListCtrl();
            UpdateRouteListCtrl();
            cc1->Refresh();

            if( runningSkipped ) {
                wxMessageDialog skipWarning( NULL,
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

    // then add routes to the listctrl
    RouteList::iterator it;
    int index = 0;
    for( it = ( *pRouteList ).begin(); it != ( *pRouteList ).end(); ++it, ++index ) {
        Route *trk = (Route *) ( *it );
        if( !trk->m_bIsTrack || !trk->IsListed() ) continue;

        wxListItem li;
        li.SetId( index );
        li.SetImage( trk->IsVisible() ? 0 : -1 );
        li.SetData( index );
        li.SetText( _T("") );

        if( g_pActiveTrack == trk ) {
            wxFont font = *wxNORMAL_FONT;
            font.SetWeight( wxFONTWEIGHT_BOLD );
            li.SetFont( font );
        }
        long idx = m_pTrkListCtrl->InsertItem( li );

        wxString name = trk->m_RouteNameString;
        if( name.IsEmpty() ) {
            RoutePoint *rp = trk->GetPoint( 1 );
            if( rp && rp->m_CreateTime.IsValid() ) name = rp->m_CreateTime.FormatISODate() + _T(" ")
                    + rp->m_CreateTime.FormatISOTime();   //name = rp->m_CreateTime.Format();
            else
                name = _("(Unnamed Track)");
        }
        m_pTrkListCtrl->SetItem( idx, colTRKNAME, name );

        wxString len;
        len.Printf( wxT("%5.2f"), trk->m_route_length );
        m_pTrkListCtrl->SetItem( idx, colTRKLENGTH, len );
    }

    m_pTrkListCtrl->SortItems( SortRoutesOnName, (long) m_pTrkListCtrl );

    // restore selection if possible
    // NOTE this will select a different item, if one is deleted
    // (the next route will get that index).
    if( selected_id > -1 ) {
        item = m_pTrkListCtrl->FindItem( -1, selected_id );
        m_pTrkListCtrl->SetItemState( item, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );
    }

    if( m_lastTrkItem >= 0 ) m_pTrkListCtrl->EnsureVisible( m_lastTrkItem );
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
        m_pTrkListCtrl->SortItems( SortTracksOnName, (long) m_pTrkListCtrl );
    } else
        if( event.m_col == 2 ) {
            sort_track_len_dir++;
            m_pTrkListCtrl->SortItems( SortTracksOnDistance, (long) m_pTrkListCtrl );
        }
}

void RouteManagerDialog::UpdateTrkButtons()
{
    long item = -1;
    item = m_pTrkListCtrl->GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    int items = m_pTrkListCtrl->GetSelectedItemCount();

    m_lastTrkItem = item;

    btnTrkProperties->Enable( items == 1 );
    btnTrkDelete->Enable( items == 1 );
    btnTrkExport->Enable( items == 1 );
    btnTrkRouteFromTrack->Enable( items == 1 );
}

void RouteManagerDialog::OnTrkToggleVisibility( wxMouseEvent &event )
{
    wxPoint pos = event.GetPosition();
    int flags = 0;
    long clicked_index = m_pTrkListCtrl->HitTest( pos, flags );

    //    Clicking Visibility column?
    if( clicked_index > -1 && event.GetX() < m_pTrkListCtrl->GetColumnWidth( colTRKVISIBLE ) ) {
        // Process the clicked item
        Route *route = pRouteList->Item( m_pTrkListCtrl->GetItemData( clicked_index ) )->GetData();
        route->SetVisible( !route->IsVisible() );
        m_pTrkListCtrl->SetItemImage( clicked_index, route->IsVisible() ? 0 : -1 );

//            pConfig->UpdateRoute(route);
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
    // Show routeproperties dialog for selected route
    long item = -1;
    item = m_pTrkListCtrl->GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( item == -1 ) return;

    Route *route = pRouteList->Item( m_pTrkListCtrl->GetItemData( item ) )->GetData();

    if( !route ) return;

    if( NULL == pRoutePropDialog )          // There is one global instance of the RouteProp Dialog
    pRoutePropDialog = new RouteProp( GetParent() );

    pRoutePropDialog->SetRouteAndUpdate( route );
    pRoutePropDialog->UpdateProperties();
    if( !route->m_bIsInLayer ) pRoutePropDialog->SetDialogTitle( _("Track Properties") );
    else {
        wxString caption( _T("Track Properties, Layer: ") );
        caption.Append( GetLayerName( route->m_LayerID ) );
        pRoutePropDialog->SetDialogTitle( caption );
    }

    if( !pRoutePropDialog->IsShown() ) pRoutePropDialog->ShowModal();
    // track might have changed
    UpdateTrkListCtrl();

    m_bNeedConfigFlush = true;
}

void RouteManagerDialog::OnTrkDeleteClick( wxCommandEvent &event )
{

    long item = -1;
    item = m_pTrkListCtrl->GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( item == -1 ) return;

    Track *track = (Track *) pRouteList->Item( m_pTrkListCtrl->GetItemData( item ) )->GetData();

//TODO Seth
//                    if(track == g_pActiveTrack)
//                        parent_frame->TrackOff();
    if( !track ) return;

    pConfig->DeleteConfigRoute( track );

    g_pRouteMan->DeleteTrack( track );

//                    m_pSelectedRoute = NULL;
//                    m_pSelectedTrack = NULL;
//                    m_pFoundRoutePoint = NULL;
//                    m_pFoundRoutePointSecond = NULL;
//
//                    if ( pRoutePropDialog )
//                    {
//                          pRoutePropDialog->SetRouteAndUpdate ( track );
//                          pRoutePropDialog->UpdateProperties();
//                    }

    UpdateTrkListCtrl();

    //    Also need to update the route list control, since routes and tracks share a common global list (pRouteList)
    UpdateRouteListCtrl();

    cc1->Refresh();

    m_bNeedConfigFlush = true;
}

void RouteManagerDialog::OnTrkExportClick( wxCommandEvent &event )
{
    // Export selected track
    long item = -1;
    item = m_pTrkListCtrl->GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( item == -1 ) return;

    Route *route = pRouteList->Item( m_pTrkListCtrl->GetItemData( item ) )->GetData();

    if( !route ) return;

    pConfig->ExportGPXRoute( this, route );
}

void RouteManagerDialog::OnTrkRouteFromTrackClick( wxCommandEvent &event )
{
    long item = -1;
    item = m_pTrkListCtrl->GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( item == -1 ) return;

    Track *track = (Track *) pRouteList->Item( m_pTrkListCtrl->GetItemData( item ) )->GetData();

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

    UpdateRouteListCtrl();

    ::wxEndBusyCursor();
}

void RouteManagerDialog::OnTrkDeleteAllClick( wxCommandEvent &event )
{
    OCPNMessageDialog mdlg( this, _("Are you sure you want to delete <ALL> tracks?"),
            wxString( _("OpenCPN Alert") ), wxYES_NO );
    int dialog_ret = mdlg.ShowModal();

    if( dialog_ret == wxID_YES ) {
        g_pRouteMan->DeleteAllTracks();
    }

    UpdateTrkListCtrl();

    //    Also need to update the route list control, since routes and tracks share a common global list (pRouteList)
    UpdateRouteListCtrl();

    if( pRoutePropDialog ) pRoutePropDialog->Hide();

    cc1->Refresh();

    m_bNeedConfigFlush = true;
}

void RouteManagerDialog::UpdateWptListCtrl( RoutePoint *rp_select, bool b_retain_sort )
{
    long selected_id = -1;
    long item = -1;

    if( NULL == rp_select ) {
        // if an item was selected, make it selected again if it still exists
        item = m_pWptListCtrl->GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );

        if( item != -1 ) selected_id = m_pWptListCtrl->GetItemData( item );
    }

    m_pWptListCtrl->DeleteAllItems();

    wxRoutePointListNode *node = pWayPointMan->m_pWayPointList->GetFirst();

    int index = 0;
    while( node ) {
        RoutePoint *rp = node->GetData();
        if( rp && rp->IsListed() ) {
            if( rp->m_bIsInTrack || rp->m_bIsInRoute ) {
                if( !rp->m_bKeepXRoute ) {
                    node = node->GetNext();
                    continue;
                }
            }

            wxListItem li;
            li.SetId( index );
            li.SetImage( rp->IsVisible() ? pWayPointMan->GetIconIndex( rp->m_pbmIcon ) : 0 );
            li.SetData( rp );
            li.SetText( _T("") );
            long idx = m_pWptListCtrl->InsertItem( li );

            wxString name = rp->GetName();
            if( name.IsEmpty() ) name = _("(Unnamed Waypoint)");
            m_pWptListCtrl->SetItem( idx, colWPTNAME, name );

            double dst;
            DistanceBearingMercator( rp->m_lat, rp->m_lon, gLat, gLon, NULL, &dst );
            wxString dist;
            dist.Printf( _T("%5.2f Nm"), dst );
            m_pWptListCtrl->SetItem( idx, colWPTDIST, dist );

            if( rp == rp_select ) selected_id = (long) rp_select; //index; //m_pWptListCtrl->GetItemData(item);

            index++;
        }

        node = node->GetNext();
    }

    if( !b_retain_sort ) {
        m_pWptListCtrl->SortItems( SortWaypointsOnName, (long) m_pWptListCtrl );
        sort_wp_key = SORT_ON_NAME;
    } else {
        switch( sort_wp_key ){
            case SORT_ON_NAME:
                m_pWptListCtrl->SortItems( SortWaypointsOnName, (long) m_pWptListCtrl );
                break;
            case SORT_ON_DISTANCE:
                m_pWptListCtrl->SortItems( SortWaypointsOnDistance, (long) m_pWptListCtrl );
                break;
        }
    }

    if( selected_id > -1 ) {
        item = m_pWptListCtrl->FindItem( -1, selected_id );
        m_pWptListCtrl->SetItemState( item, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED );
    }

    if( m_lastWptItem >= 0 ) m_pWptListCtrl->EnsureVisible( m_lastWptItem );
    UpdateWptButtons();
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
        m_pWptListCtrl->SortItems( SortWaypointsOnName, (long) m_pWptListCtrl );
        sort_wp_key = SORT_ON_NAME;
    } else
        if( event.m_col == 2 ) {
            sort_wp_len_dir++;
            m_pWptListCtrl->SortItems( SortWaypointsOnDistance, (long) m_pWptListCtrl );
            sort_wp_key = SORT_ON_DISTANCE;
        }
}

void RouteManagerDialog::UpdateWptButtons()
{
    long item = -1;
    item = m_pWptListCtrl->GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    bool enable = ( item != -1 );

    if( enable ) m_lastWptItem = item;

    btnWptProperties->Enable( enable );
    btnWptZoomto->Enable( enable );
    btnWptDelete->Enable( enable );
    btnWptGoTo->Enable( enable );
    btnWptExport->Enable( enable );
    btnWptSendToGPS->Enable( enable );
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
                wp->IsVisible() ? pWayPointMan->GetIconIndex( wp->m_pbmIcon ) : 0 );

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

    if( NULL == pMarkPropDialog )          // There is one global instance of the MarkProp Dialog
    pMarkPropDialog = new MarkInfoImpl( GetParent() );

    pMarkPropDialog->SetRoutePoint( pWP );
    pMarkPropDialog->UpdateProperties();

    if( !pMarkPropDialog->IsShown() ) pMarkPropDialog->ShowModal();

    // waypoint might have changed
    UpdateWptListCtrl();
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
    if( NULL == pMarkPropDialog ) pMarkPropDialog = new MarkInfoImpl( parent );

    pMarkPropDialog->SetRoutePoint( wp );
    pMarkPropDialog->UpdateProperties();
    if( wp->m_bIsInLayer ) {
        wxString caption( _T("Mark Properties, Layer: ") );
        caption.Append( GetLayerName( wp->m_LayerID ) );
        pMarkPropDialog->SetDialogTitle( caption );
    } else
        pMarkPropDialog->SetDialogTitle( _T("Mark Properties") );

    if( !pMarkPropDialog->IsShown() ) pMarkPropDialog->ShowModal();

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
    long item = -1;
    item = m_pWptListCtrl->GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( item == -1 ) return;

    RoutePoint *wp = (RoutePoint *) m_pWptListCtrl->GetItemData( item );
    if( !wp ) return;
    if( wp->m_bIsInLayer ) return;

    long item_next = m_pWptListCtrl->GetNextItem( item );         // next in list
    RoutePoint *wp_next = NULL;
    if( item_next > -1 ) wp_next = (RoutePoint *) m_pWptListCtrl->GetItemData( item_next );

    if ( wp->m_bIsInRoute || wp->m_bIsInTrack )
    {
        if ( wxYES == wxMessageBox( _( "The waypoint you want to delete is used in a route, do you really want to delete it?" ), _( "OpenCPN Alert" ), wxYES_NO ))
            pWayPointMan->DestroyWaypoint( wp );
    }
    else
        pWayPointMan->DestroyWaypoint( wp );

    if( pMarkPropDialog ) {
        pMarkPropDialog->SetRoutePoint( NULL );
        pMarkPropDialog->UpdateProperties();
    }

    UpdateWptListCtrl( wp_next, true );
    UpdateRouteListCtrl();
    cc1->Refresh();
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
    ;
    temp_route->m_RouteEndString = name;
    temp_route->m_bDeleteOnArrival = true;

    if( g_pRouteMan->GetpActiveRoute() ) g_pRouteMan->DeactivateRoute();

    g_pRouteMan->ActivateRoute( temp_route, wp );

    UpdateRouteListCtrl();
}

void RouteManagerDialog::OnWptExportClick( wxCommandEvent &event )
{
    long item = -1;
    item = m_pWptListCtrl->GetNextItem( item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED );
    if( item == -1 ) return;

    RoutePoint *wp = (RoutePoint *) m_pWptListCtrl->GetItemData( item );

    if( !wp ) return;

    pConfig->ExportGPXWaypoint( this, wp );
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
//    if( g_pnmea ) g_pnmea->GetSource( source );

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
        prompt = _("There are some waypoints used in routes or anchor alarms. Do you want to delete them as well? This will change the routes and disable the anchor alarms. Answering No keeps the waypoints used in routes or alarms.");
        buttons = wxYES_NO | wxCANCEL;
        type = 2;
    }
    OCPNMessageDialog mdlg( this, prompt,
            wxString( _("OpenCPN Alert") ), buttons );
    int answer = mdlg.ShowModal();
    if ( answer == wxID_YES )
        pWayPointMan->DeleteAllWaypoints( true );
    if ( answer == wxID_NO && type == 2 )
        pWayPointMan->DeleteAllWaypoints( false );          // only delete unused waypoints

    if( pMarkPropDialog ) {
        pMarkPropDialog->SetRoutePoint( NULL );
        pMarkPropDialog->UpdateProperties();
    }

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
        m_pLayListCtrl->SortItems( SortLayersOnName, (long) m_pLayListCtrl );
    } else
        if( event.m_col == 2 ) {
            sort_layer_len_dir++;
            m_pLayListCtrl->SortItems( SortLayersOnSize, (long) m_pLayListCtrl );
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
        m_pLayListCtrl->SetItemImage( clicked_index, layer->IsVisibleOnChart() ? 0 : -1 );

        ToggleLayerContentsOnChart( layer );
    }

    // Allow wx to process...
    event.Skip();
}

void RouteManagerDialog::OnLayNewClick( wxCommandEvent &event )
{
    bool show_flag = g_bShowLayers;
    g_bShowLayers = true;
    pConfig->ImportGPX( this, true, _T(""), false );
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

    // Process Tracks and Routes in this layer
    wxRouteListNode *node1 = pRouteList->GetFirst();
    wxRouteListNode *node2;
    while( node1 ) {
        Route *pRoute = node1->GetData();
        node2 = node1->GetNext();
        if( pRoute->m_bIsInLayer && ( pRoute->m_LayerID == layer->m_LayerID ) ) {
            pRoute->m_bIsInLayer = false;
            pRoute->m_LayerID = 0;
            if( !pRoute->m_bIsTrack ) {
                //pConfig->DeleteConfigRoute(pRoute);
                g_pRouteMan->DeleteRoute( pRoute );
            } else {
                //pConfig->DeleteConfigRoute(pRoute);
                g_pRouteMan->DeleteTrack( pRoute );
            }
        }
        node1 = node2;
        node2 = NULL;
    }

    //m_pSelectedRoute = NULL;
    //m_pSelectedTrack = NULL;
    //m_pFoundRoutePoint = NULL;
    //m_pFoundRoutePointSecond = NULL;

    //if ( pRoutePropDialog )
    //{
    //      //pRoutePropDialog->SetRouteAndUpdate ( track );
    //      pRoutePropDialog->UpdateProperties();
    //}

    // Process waypoints in this layer
    wxRoutePointListNode *node = pWayPointMan->m_pWayPointList->GetFirst();
    wxRoutePointListNode *node3;

    while( node ) {
        node3 = node->GetNext();
        RoutePoint *rp = node->GetData();
        if( rp && ( rp->m_LayerID == layer->m_LayerID ) ) {
            rp->m_bIsInLayer = false;
            rp->m_LayerID = 0;
            pWayPointMan->DestroyWaypoint( rp );
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
    m_pLayListCtrl->SetItemImage( item, layer->IsVisibleOnChart() ? 0 : -1 );

    ToggleLayerContentsOnChart( layer );
}

void RouteManagerDialog::ToggleLayerContentsOnChart( Layer *layer )
{
    // Process Tracks and Routes in this layer
    wxRouteListNode *node1 = pRouteList->GetFirst();
    while( node1 ) {
        Route *pRoute = node1->GetData();
        if( pRoute->m_bIsInLayer && ( pRoute->m_LayerID == layer->m_LayerID ) ) {
            if( !pRoute->m_bIsTrack ) {
                pRoute->SetVisible( layer->IsVisibleOnChart() );
                pConfig->UpdateRoute( pRoute );
            } else {
                pRoute->SetVisible( layer->IsVisibleOnChart() );
            }
        }
        node1 = node1->GetNext();
    }

    // Process waypoints in this layer
    wxRoutePointListNode *node = pWayPointMan->m_pWayPointList->GetFirst();

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
    wxRoutePointListNode *node = pWayPointMan->m_pWayPointList->GetFirst();

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
    // Process Tracks and Routes in this layer
    wxRouteListNode *node1 = pRouteList->GetFirst();
    while( node1 ) {
        Route *pRoute = node1->GetData();
        if( pRoute->m_bIsInLayer && ( pRoute->m_LayerID == layer->m_LayerID ) ) {
            if( !pRoute->m_bIsTrack ) {
                pRoute->SetListed( layer->IsVisibleOnListing() );
                //pConfig->UpdateRoute(pRoute);
            } else {
                pRoute->SetListed( layer->IsVisibleOnListing() );
            }
        }
        node1 = node1->GetNext();
    }

    // Process waypoints in this layer
    wxRoutePointListNode *node = pWayPointMan->m_pWayPointList->GetFirst();

    while( node ) {
        RoutePoint *rp = node->GetData();
        if( rp && ( rp->m_LayerID == layer->m_LayerID ) ) {
            rp->SetListed( layer->IsVisibleOnListing() );
        }

        node = node->GetNext();
    }

    UpdateRouteListCtrl();
    UpdateTrkListCtrl();
    UpdateWptListCtrl();
    UpdateLayListCtrl();

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
        li.SetImage( lay->IsVisibleOnChart() ? 0 : -1 );
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
    }

    m_pLayListCtrl->SortItems( SortLayersOnName, (long) m_pLayListCtrl );

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
    pConfig->ImportGPX( this );

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

//END Event handlers
