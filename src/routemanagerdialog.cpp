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

#include <wx/listctrl.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/notebook.h>
#include <wx/generic/progdlgg.h>

#include <iostream>

#include "navutil.h"
#include "routeprop.h"
#include "routeman.h"
#include "georef.h"
#include "chartbase.h"
//#include "chartimg.h"               // for ChartBaseBSB

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
enum { colWPTICON = 0, colWPTNAME, colWPTDIST };

// GLOBALS :0
extern RouteList *pRouteList;
extern RouteProp *pRoutePropDialog;
extern Routeman  *g_pRouteMan;
extern MyConfig  *pConfig;
extern ChartCanvas *cc1;
extern ChartBase *Current_Ch;
extern Track     *g_pActiveTrack;
extern WayPointman      *pWayPointMan;
extern MarkProp         *pMarkPropDialog;
extern MyFrame         *gFrame;
extern Select           *pSelect;
extern double           gLat, gLon;
extern double           gCog, gSog;


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
            return it2.GetText().Cmp(it1.GetText());
      else
            return it1.GetText().Cmp(it2.GetText());

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
            return it2.GetText().Cmp(it1.GetText());
      else
            return it1.GetText().Cmp(it2.GetText());
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
            return it2.GetText().Cmp(it1.GetText());
      else
            return it1.GetText().Cmp(it2.GetText());

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

      char s1b[20];
      strncpy(s1b,it1.GetText().mb_str(), 19);
      char s2b[20];
      strncpy(s2b,it2.GetText().mb_str(), 19);

      wxString s1, s2;
      s1.Printf(_T("%11s"), s1b);
      s2.Printf(_T("%11s"), s2b);

      if(sort_track_len_dir & 1)
            return s2.Cmp(s1);
      else
            return s1.Cmp(s2);

}

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
            return it2.GetText().Cmp(it1.GetText());
      else
            return it1.GetText().Cmp(it2.GetText());
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

      char s1b[20];
      strncpy(s1b,it1.GetText().mb_str(), 19);
      char s2b[20];
      strncpy(s2b,it2.GetText().mb_str(), 19);

      wxString s1, s2;
      s1.Printf(_T("%11s"), s1b);
      s2.Printf(_T("%11s"), s2b);

      if(sort_wp_len_dir & 1)
            return it2.GetText().Cmp(it1.GetText());
      else
            return it1.GetText().Cmp(it2.GetText());
}


// event table. Empty, because I find it much easier to see what is connected to what
// using Connect() where possible, so that it is visible in the code.
BEGIN_EVENT_TABLE(RouteManagerDialog, wxDialog)
END_EVENT_TABLE()

// implementation
RouteManagerDialog::RouteManagerDialog(wxWindow *parent)
      : wxDialog(parent, -1, wxString(_("Route Manager")), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER)
{
      Create();
}



void RouteManagerDialog::Create()
{
      wxBoxSizer* itemBoxSizer1 = new wxBoxSizer(wxVERTICAL);
      SetSizer(itemBoxSizer1);

      wxNotebook *itemNotebook1 = new wxNotebook( this, wxID_ANY, wxDefaultPosition,
            wxSize(-1, -1), wxNB_TOP );
      itemBoxSizer1->Add(itemNotebook1, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL|wxEXPAND, 5);

      //  Create "Routes" panel
      wxPanel *itemPanel1 = new wxPanel( itemNotebook1, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                       wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
      wxBoxSizer *sbsRoutes = new wxBoxSizer(wxHORIZONTAL);
      itemPanel1->SetSizer(sbsRoutes);
      itemNotebook1->AddPage(itemPanel1, _("Routes"));

      sort_wp_len_dir = 0;
      sort_wp_name_dir = 0;
      sort_track_len_dir = 0;
      sort_route_to_dir = 0;
      sort_track_name_dir = 0;
      sort_route_name_dir = 0;

      // Setup GUI
      m_pRouteListCtrl = new wxListCtrl(itemPanel1, -1, wxDefaultPosition, wxSize(400, -1),
          wxLC_REPORT|wxLC_SINGLE_SEL|wxLC_SORT_ASCENDING|wxLC_HRULES/*|wxLC_VRULES*/);
      m_pRouteListCtrl->Connect(wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler(RouteManagerDialog::OnRteSelected), NULL, this);
      m_pRouteListCtrl->Connect(wxEVT_COMMAND_LIST_ITEM_DESELECTED, wxListEventHandler(RouteManagerDialog::OnRteSelected), NULL, this);
      m_pRouteListCtrl->Connect(wxEVT_COMMAND_LIST_ITEM_ACTIVATED, wxListEventHandler(RouteManagerDialog::OnRteDefaultAction), NULL, this);
      m_pRouteListCtrl->Connect(wxEVT_LEFT_DOWN, wxMouseEventHandler(RouteManagerDialog::OnRteToggleVisibility), NULL, this);
	  m_pRouteListCtrl->Connect(wxEVT_COMMAND_LIST_COL_CLICK, wxListEventHandler(RouteManagerDialog::OnRteColumnClicked), NULL, this);
      sbsRoutes->Add(m_pRouteListCtrl, 1, wxEXPAND|wxALL, DIALOG_MARGIN);

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

      // Buttons: Delete, Properties..., Zoom to
      wxBoxSizer *bsRouteButtons = new wxBoxSizer(wxVERTICAL);
      sbsRoutes->Add(bsRouteButtons, 0, wxALIGN_RIGHT);

      btnRteProperties = new wxButton(itemPanel1, -1, _("&Properties..."));
      bsRouteButtons->Add(btnRteProperties, 0, wxALL|wxEXPAND, DIALOG_MARGIN);
      btnRteProperties->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                             wxCommandEventHandler(RouteManagerDialog::OnRtePropertiesClick), NULL, this);

      btnRteActivate = new wxButton(itemPanel1, -1, _("&Activate"));
      bsRouteButtons->Add(btnRteActivate, 0, wxALL|wxEXPAND, DIALOG_MARGIN);
      btnRteActivate->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                           wxCommandEventHandler(RouteManagerDialog::OnRteActivateClick), NULL, this);
      btnRteActivate->Connect(wxEVT_LEFT_DOWN,
                           wxMouseEventHandler(RouteManagerDialog::OnRteBtnLeftDown), NULL, this);

      btnRteZoomto = new wxButton(itemPanel1, -1, _("&Zoom to"));
      bsRouteButtons->Add(btnRteZoomto, 0, wxALL|wxEXPAND, DIALOG_MARGIN);
      btnRteZoomto->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                         wxCommandEventHandler(RouteManagerDialog::OnRteZoomtoClick), NULL, this);
      btnRteZoomto->Connect(wxEVT_LEFT_DOWN,
                           wxMouseEventHandler(RouteManagerDialog::OnRteBtnLeftDown), NULL, this);

      btnRteReverse = new wxButton(itemPanel1, -1, _("&Reverse"));
      bsRouteButtons->Add(btnRteReverse, 0, wxALL|wxEXPAND, DIALOG_MARGIN);
      btnRteReverse->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                          wxCommandEventHandler(RouteManagerDialog::OnRteReverseClick), NULL, this);

      btnRteDelete = new wxButton(itemPanel1, -1, _("&Delete"));
      bsRouteButtons->Add(btnRteDelete, 0, wxALL|wxEXPAND, DIALOG_MARGIN);
      btnRteDelete->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                         wxCommandEventHandler(RouteManagerDialog::OnRteDeleteClick), NULL, this);

      btnRteExport = new wxButton(itemPanel1, -1, _("&Export Route..."));
      bsRouteButtons->Add(btnRteExport, 0, wxALL|wxEXPAND, DIALOG_MARGIN);
      btnRteExport->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                         wxCommandEventHandler(RouteManagerDialog::OnRteExportClick), NULL, this);

      btnRteSendToGPS = new wxButton(itemPanel1, -1, _("&Send to GPS"));
      bsRouteButtons->Add(btnRteSendToGPS, 0, wxALL|wxEXPAND, DIALOG_MARGIN);
      btnRteSendToGPS->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                         wxCommandEventHandler(RouteManagerDialog::OnRteSendToGPSClick), NULL, this);

      bsRouteButtons->AddSpacer(10);

      wxButton *btnRteDeleteAll = new wxButton(itemPanel1, -1, _("&Delete All"));
      bsRouteButtons->Add(btnRteDeleteAll, 0, wxALL|wxEXPAND, DIALOG_MARGIN);
      btnRteDeleteAll->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                         wxCommandEventHandler(RouteManagerDialog::OnRteDeleteAllClick), NULL, this);

      //  Create "Tracks" panel
      wxPanel *itemPanel2 = new wxPanel( itemNotebook1, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                       wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
      wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
      itemPanel2->SetSizer(itemBoxSizer3);
      itemNotebook1->AddPage(itemPanel2, _("Tracks"));

      m_pTrkListCtrl = new wxListCtrl(itemPanel2, -1, wxDefaultPosition, wxSize(400, -1),
          wxLC_REPORT|wxLC_SINGLE_SEL|wxLC_SORT_ASCENDING|wxLC_HRULES/*|wxLC_VRULES*/);
      m_pTrkListCtrl->Connect(wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler(RouteManagerDialog::OnTrkSelected), NULL, this);
      m_pTrkListCtrl->Connect(wxEVT_COMMAND_LIST_ITEM_DESELECTED, wxListEventHandler(RouteManagerDialog::OnTrkSelected), NULL, this);
      m_pTrkListCtrl->Connect(wxEVT_COMMAND_LIST_ITEM_ACTIVATED, wxListEventHandler(RouteManagerDialog::OnTrkDefaultAction), NULL, this);
      m_pTrkListCtrl->Connect(wxEVT_LEFT_DOWN, wxMouseEventHandler(RouteManagerDialog::OnTrkToggleVisibility), NULL, this);
	  m_pTrkListCtrl->Connect(wxEVT_COMMAND_LIST_COL_CLICK, wxListEventHandler(RouteManagerDialog::OnTrkColumnClicked), NULL, this);
      itemBoxSizer3->Add(m_pTrkListCtrl, 1, wxEXPAND|wxALL, DIALOG_MARGIN);

      m_pTrkListCtrl->InsertColumn( colTRKVISIBLE, _T(""), wxLIST_FORMAT_LEFT, 28 );
      m_pTrkListCtrl->InsertColumn( colTRKNAME, _("Track Name"), wxLIST_FORMAT_LEFT, 250 );
      m_pTrkListCtrl->InsertColumn( colTRKLENGTH, _("Length"), wxLIST_FORMAT_LEFT, 100 );

      wxBoxSizer *bsTrkButtons = new wxBoxSizer(wxVERTICAL);
      itemBoxSizer3->Add(bsTrkButtons, 0, wxALIGN_RIGHT);

      wxButton *btnTrkNew = new wxButton(itemPanel2, -1, _("&Start Track"));
      bsTrkButtons->Add(btnTrkNew, 0, wxALL|wxEXPAND, DIALOG_MARGIN);
      btnTrkNew->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                           wxCommandEventHandler(RouteManagerDialog::OnTrkNewClick), NULL, this);

      btnTrkProperties = new wxButton(itemPanel2, -1, _("&Properties"));
      bsTrkButtons->Add(btnTrkProperties, 0, wxALL|wxEXPAND, DIALOG_MARGIN);
      btnTrkProperties->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                           wxCommandEventHandler(RouteManagerDialog::OnTrkPropertiesClick), NULL, this);

      btnTrkDelete = new wxButton(itemPanel2, -1, _("&Delete"));
      bsTrkButtons->Add(btnTrkDelete, 0, wxALL|wxEXPAND, DIALOG_MARGIN);
      btnTrkDelete->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                           wxCommandEventHandler(RouteManagerDialog::OnTrkDeleteClick), NULL, this);

      btnTrkExport = new wxButton(itemPanel2, -1, _("&Export Track..."));
      bsTrkButtons->Add(btnTrkExport, 0, wxALL|wxEXPAND, DIALOG_MARGIN);
      btnTrkExport->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                         wxCommandEventHandler(RouteManagerDialog::OnTrkExportClick), NULL, this);

      btnTrkRouteFromTrack = new wxButton(itemPanel2, -1, _("Route from Track"));
      bsTrkButtons->Add(btnTrkRouteFromTrack, 0, wxALL|wxEXPAND, DIALOG_MARGIN);
      btnTrkRouteFromTrack->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                         wxCommandEventHandler(RouteManagerDialog::OnTrkRouteFromTrackClick), NULL, this);

      bsTrkButtons->AddSpacer(10);

      wxButton *btnTrkDeleteAll = new wxButton(itemPanel2, -1, _("&Delete All"));
      bsTrkButtons->Add(btnTrkDeleteAll, 0, wxALL|wxEXPAND, DIALOG_MARGIN);
      btnTrkDeleteAll->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                         wxCommandEventHandler(RouteManagerDialog::OnTrkDeleteAllClick), NULL, this);

      //  Create "Waypoints" panel
      wxPanel *itemPanel3 = new wxPanel( itemNotebook1, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                       wxSUNKEN_BORDER|wxTAB_TRAVERSAL );
      wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
      itemPanel3->SetSizer(itemBoxSizer4);
      itemNotebook1->AddPage(itemPanel3, _("Waypoints"));

      m_pWptListCtrl = new wxListCtrl(itemPanel3, -1, wxDefaultPosition, wxSize(400, -1),
          wxLC_REPORT|wxLC_SINGLE_SEL|wxLC_SORT_ASCENDING|wxLC_HRULES/*|wxLC_VRULES*/);
      m_pWptListCtrl->Connect(wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler(RouteManagerDialog::OnWptSelected), NULL, this);
      m_pWptListCtrl->Connect(wxEVT_COMMAND_LIST_ITEM_DESELECTED, wxListEventHandler(RouteManagerDialog::OnWptSelected), NULL, this);
      m_pWptListCtrl->Connect(wxEVT_COMMAND_LIST_ITEM_ACTIVATED, wxListEventHandler(RouteManagerDialog::OnWptDefaultAction), NULL, this);
      m_pWptListCtrl->Connect(wxEVT_LEFT_DOWN, wxMouseEventHandler(RouteManagerDialog::OnWptToggleVisibility), NULL, this);
	  m_pWptListCtrl->Connect(wxEVT_COMMAND_LIST_COL_CLICK, wxListEventHandler(RouteManagerDialog::OnWptColumnClicked), NULL, this);
      itemBoxSizer4->Add(m_pWptListCtrl, 1, wxEXPAND|wxALL, DIALOG_MARGIN);

      m_pWptListCtrl->InsertColumn( colWPTICON, _("Icon"), wxLIST_FORMAT_LEFT, 44 );
      m_pWptListCtrl->InsertColumn( colWPTNAME, _("Waypoint Name"), wxLIST_FORMAT_LEFT, 180 );
      m_pWptListCtrl->InsertColumn( colWPTDIST, _("Distance"), wxLIST_FORMAT_LEFT, 180 );

      wxBoxSizer *bsWptButtons = new wxBoxSizer(wxVERTICAL);
      itemBoxSizer4->Add(bsWptButtons, 0, wxALIGN_RIGHT);

      wxButton *btnWptNew = new wxButton(itemPanel3, -1, _("&New"));
      bsWptButtons->Add(btnWptNew, 0, wxALL|wxEXPAND, DIALOG_MARGIN);
      btnWptNew->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                           wxCommandEventHandler(RouteManagerDialog::OnWptNewClick), NULL, this);

      btnWptProperties = new wxButton(itemPanel3, -1, _("&Properties"));
      bsWptButtons->Add(btnWptProperties, 0, wxALL|wxEXPAND, DIALOG_MARGIN);
      btnWptProperties->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                           wxCommandEventHandler(RouteManagerDialog::OnWptPropertiesClick), NULL, this);

      btnWptZoomto = new wxButton(itemPanel3, -1, _("&Zoom to"));
      bsWptButtons->Add(btnWptZoomto, 0, wxALL|wxEXPAND, DIALOG_MARGIN);
      btnWptZoomto->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                         wxCommandEventHandler(RouteManagerDialog::OnWptZoomtoClick), NULL, this);

      btnWptDelete = new wxButton(itemPanel3, -1, _("&Delete"));
      bsWptButtons->Add(btnWptDelete, 0, wxALL|wxEXPAND, DIALOG_MARGIN);
      btnWptDelete->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                           wxCommandEventHandler(RouteManagerDialog::OnWptDeleteClick), NULL, this);

      btnWptGoTo = new wxButton(itemPanel3, -1, _("&Go To"));
      bsWptButtons->Add(btnWptGoTo, 0, wxALL|wxEXPAND, DIALOG_MARGIN);
      btnWptGoTo->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                           wxCommandEventHandler(RouteManagerDialog::OnWptGoToClick), NULL, this);

      btnWptExport = new wxButton(itemPanel3, -1, _("&Export Wpt..."));
      bsWptButtons->Add(btnWptExport, 0, wxALL|wxEXPAND, DIALOG_MARGIN);
      btnWptExport->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                         wxCommandEventHandler(RouteManagerDialog::OnWptExportClick), NULL, this);

      btnWptSendToGPS = new wxButton(itemPanel3, -1, _("&Send to GPS"));
      bsWptButtons->Add(btnWptSendToGPS, 0, wxALL|wxEXPAND, DIALOG_MARGIN);
      btnWptSendToGPS->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                         wxCommandEventHandler(RouteManagerDialog::OnWptSendToGPSClick), NULL, this);

      bsWptButtons->AddSpacer(10);

      wxButton *btnWptDeleteAll = new wxButton(itemPanel3, -1, _("Delete All"));
      bsWptButtons->Add(btnWptDeleteAll, 0, wxALL|wxEXPAND, DIALOG_MARGIN);
      btnWptDeleteAll->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                           wxCommandEventHandler(RouteManagerDialog::OnWptDeleteAllClick), NULL, this);

      wxBoxSizer *itemBoxSizer5 = new wxBoxSizer(wxHORIZONTAL);
      itemBoxSizer1->Add(itemBoxSizer5, 0, wxALL|wxEXPAND);

      wxBoxSizer *itemBoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
      itemBoxSizer5->Add(itemBoxSizer6, 1, wxALL|wxEXPAND|wxALIGN_LEFT);

      btnImport = new wxButton(this, -1, _("I&mport..."));
      itemBoxSizer6->Add(btnImport, 0, wxALL|wxALIGN_LEFT, DIALOG_MARGIN);
      btnImport->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                         wxCommandEventHandler(RouteManagerDialog::OnImportClick), NULL, this);
      btnExport = new wxButton(this, -1, _("E&xport All..."));
      itemBoxSizer6->Add(btnExport, 0, wxALL|wxALIGN_LEFT, DIALOG_MARGIN);
      btnExport->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
                         wxCommandEventHandler(RouteManagerDialog::OnExportClick), NULL, this);

      // Dialog buttons
      wxSizer *szButtons = CreateButtonSizer(wxOK);

      itemBoxSizer5->Add(szButtons, 0, wxALL|wxALIGN_RIGHT, DIALOG_MARGIN);

      Fit();

      SetMinSize(GetBestSize());

      // create a image list for the list with just the eye icon
      wxImageList *imglist = new wxImageList(20, 20, true, 1);
      imglist->Add(wxBitmap(eye));
      m_pRouteListCtrl->AssignImageList(imglist, wxIMAGE_LIST_SMALL);
      // Assign will handle destroy, Set will not. It's OK, that's what we want
      m_pTrkListCtrl->SetImageList(imglist, wxIMAGE_LIST_SMALL);
      m_pWptListCtrl->SetImageList(pWayPointMan->Getpmarkicon_image_list(), wxIMAGE_LIST_SMALL);

      UpdateRouteListCtrl();
      UpdateTrkListCtrl();
      UpdateWptListCtrl();

      m_bNeedConfigFlush = false;
}

RouteManagerDialog::~RouteManagerDialog()
{
      delete m_pRouteListCtrl;
      delete m_pTrkListCtrl;
      delete m_pWptListCtrl;

      delete btnTrkProperties;
      delete btnRteDelete;
      delete btnRteExport;
      delete btnRteZoomto;
      delete btnRteProperties;
      delete btnRteActivate;
      delete btnRteReverse;
      delete btnRteSendToGPS;
      delete btnTrkDelete;
      delete btnTrkExport;
      delete btnTrkRouteFromTrack;
      delete btnWptProperties;
      delete btnWptZoomto;
      delete btnWptDelete;
      delete btnWptGoTo;
      delete btnWptExport;
      delete btnWptSendToGPS;
      delete btnImport;
      delete btnExport;

      //    Does not need to be done here at all, since this dialog is autommatically deleted as a child of the frame.
      //    By that time, the config has already been updated for shutdown.

      // Do this just once!!
//      if (m_bNeedConfigFlush)
//            pConfig->UpdateSettings();
}

void RouteManagerDialog::UpdateRouteListCtrl()
{
      // if an item was selected, make it selected again if it still exist
      long item = -1;
      item = m_pRouteListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
      long selected_id = -1;
      if (item != -1)
            selected_id = m_pRouteListCtrl->GetItemData(item);

      // Delete existing items
      m_pRouteListCtrl->DeleteAllItems();

      // then add routes to the listctrl
      RouteList::iterator it;
      int index = 0;
      for (it = (*pRouteList).begin(); it != (*pRouteList).end(); ++it, ++index)
      {
            if ((*it)->m_bIsTrack)
                  continue;

            wxListItem li;
            li.SetId(index);
            li.SetImage((*it)->IsVisible() ? 0 : -1);
            li.SetData(index);
            li.SetText(_T(""));

            if((*it)->m_bRtIsActive)
            {
                  wxFont font = *wxNORMAL_FONT;
                  font.SetWeight(wxFONTWEIGHT_BOLD);
                  li.SetFont(font);
            }

            long idx = m_pRouteListCtrl->InsertItem(li);

            wxString name = (*it)->m_RouteNameString;
            if (name.IsEmpty())
                  name = _("(Unnamed Route)");
            m_pRouteListCtrl->SetItem(idx, rmROUTENAME, name);

            wxString startend = (*it)->m_RouteStartString;
            if (!(*it)->m_RouteEndString.IsEmpty())
                  startend.append(_(" - ") + (*it)->m_RouteEndString);
            m_pRouteListCtrl->SetItem(idx, rmROUTEDESC,  startend);
      }

      m_pRouteListCtrl->SortItems(SortRoutesOnName, (long)m_pRouteListCtrl);

      // restore selection if possible
      // NOTE this will select a different item, if one is deleted
      // (the next route will get that index).
      if (selected_id > -1)
      {
            item = m_pRouteListCtrl->FindItem(-1, selected_id);
            m_pRouteListCtrl->SetItemState(item, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
      }

      UpdateRteButtons();
}

void RouteManagerDialog::UpdateRteButtons()
{
      // enable/disable buttons
      long selected_index_index = m_pRouteListCtrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
      bool enable = !(selected_index_index < 0);

      btnRteDelete->Enable(enable);
      btnRteZoomto->Enable(enable); // && !cc1->m_bFollow);
      btnRteProperties->Enable(enable);
      btnRteReverse->Enable(enable);
      btnRteExport->Enable(enable);
      btnRteSendToGPS->Enable(enable);

      // set activate button text
      Route *route = NULL;
      if(enable)
            route = pRouteList->Item(m_pRouteListCtrl->GetItemData(selected_index_index))->GetData();

      if(!g_pRouteMan->IsAnyRouteActive())
      {
            btnRteActivate->Enable(enable);
            if (enable)
                   btnRteActivate->SetLabel(_("Activate"));

      }
      else
      {
            if(enable)
            {
                 if(route && route->m_bRtIsActive)
                  {
                        btnRteActivate->Enable(enable);
                        btnRteActivate->SetLabel(_("Deactivate"));
                  }
                  else
                        btnRteActivate->Enable(false);
            }
            else
                  btnRteActivate->Enable(false);
      }
}

void RouteManagerDialog::MakeAllRoutesInvisible()
{
      RouteList::iterator it;
      long index = 0;
      for (it = (*pRouteList).begin(); it != (*pRouteList).end(); ++it, ++index)
      {
            if ((*it)->IsVisible()) { // avoid config updating as much as possible!
                  (*it)->SetVisible(false);
                  m_pRouteListCtrl->SetItemImage(m_pRouteListCtrl->FindItem(-1, index), -1); // Likely not same order :0
                  pConfig->UpdateRoute(*it); // auch, flushes config to disk. FIXME
            }
      }
}

void RouteManagerDialog::ZoomtoRoute(Route *route)
{

      // Calculate bbox center
      double clat = route->RBBox.GetMinY() + (route->RBBox.GetHeight()/2);
      double clon = route->RBBox.GetMinX() + (route->RBBox.GetWidth()/2);

      // Calculate ppm
      double rw, rh, ppm; // route width, height, final ppm scale to use
      int ww, wh; // chart window width, height
      // route bbox width in nm
      DistanceBearingMercator(route->RBBox.GetMinY(), route->RBBox.GetMinX(),
                              route->RBBox.GetMinY(), route->RBBox.GetMaxX(),
                              NULL, &rw);
      // route bbox height in nm
      DistanceBearingMercator(route->RBBox.GetMinY(), route->RBBox.GetMinX(),
                              route->RBBox.GetMaxY(), route->RBBox.GetMinX(),
                              NULL, &rh);

      cc1->GetSize(&ww, &wh);

      ppm = wxMin(ww/(rw*1852), wh/(rh*1852))*(100-fabs(clat))/90;

      ppm = wxMin(ppm, 1.0);

      cc1->ClearbFollow();
      cc1->SetViewPoint(clat, clon, ppm, 0, cc1->GetVPRotation(), CURRENT_RENDER);
      cc1->Refresh();

      m_bNeedConfigFlush = true;
}

//BEGIN Event handlers
void RouteManagerDialog::OnRteDeleteClick(wxCommandEvent &event)
{
      long item = -1;
      item = m_pRouteListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
      if (item == -1) return;

      Route *proute_to_delete = pRouteList->Item(m_pRouteListCtrl->GetItemData(item))->GetData();

      if (!proute_to_delete) return;

      ::wxBeginBusyCursor();

      cc1->CancelMouseRoute();

      g_pRouteMan->DeleteRoute(proute_to_delete);

      pConfig->DeleteConfigRoute ( proute_to_delete );

      ::wxEndBusyCursor();

      UpdateRouteListCtrl();

      //    Also need to update the track list control, since routes and tracks share a common global list (pRouteList)
      UpdateTrkListCtrl();

      cc1->Refresh();

      m_bNeedConfigFlush = true;
}

void RouteManagerDialog::OnRteDeleteAllClick(wxCommandEvent &event)
{
      wxMessageDialog mdlg(this, _("Are you sure you want to delete <ALL> routes?"), wxString(_("OpenCPN Alert")),wxYES_NO  );
      if(mdlg.ShowModal() == wxID_YES)
      {
            if ( g_pRouteMan->GetpActiveRoute() )
                  g_pRouteMan->DeactivateRoute();

            cc1->CancelMouseRoute();

            g_pRouteMan->DeleteAllRoutes();
// TODO Seth
//            m_pSelectedRoute = NULL;
//            m_pFoundRoutePoint = NULL;
//            m_pFoundRoutePointSecond = NULL;

            UpdateRouteListCtrl();

            //    Also need to update the track list control, since routes and tracks share a common global list (pRouteList)
            UpdateTrkListCtrl();

            if(pRoutePropDialog)
                  pRoutePropDialog->Hide();

            cc1->Refresh();

            m_bNeedConfigFlush = true;
      }
}

void RouteManagerDialog::OnRtePropertiesClick(wxCommandEvent &event)
{
      // Show routeproperties dialog for selected route
      long item = -1;
      item = m_pRouteListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
      if (item == -1) return;

      Route *route = pRouteList->Item(m_pRouteListCtrl->GetItemData(item))->GetData();

      if (!route) return;

      if ( NULL == pRoutePropDialog )          // There is one global instance of the RouteProp Dialog
            pRoutePropDialog  = new RouteProp(GetParent());

      pRoutePropDialog->SetRouteAndUpdate(route);
      pRoutePropDialog->UpdateProperties();
      pRoutePropDialog->SetDialogTitle(_("Route Properties"));

      if(!pRoutePropDialog->IsShown())
            pRoutePropDialog->ShowModal();
      // route might have changed
      UpdateRouteListCtrl();

      m_bNeedConfigFlush = true;
}

void RouteManagerDialog::OnRteZoomtoClick(wxCommandEvent &event)
{
//      if (cc1->m_bFollow)
//            return;

      // Zoom into the bounding box of the selected route
      long item = -1;
      item = m_pRouteListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
      if (item == -1) return;

      // optionally make this route exclusively visible
      if (m_bCtrlDown)
            MakeAllRoutesInvisible();

      Route *route = pRouteList->Item(m_pRouteListCtrl->GetItemData(item))->GetData();

      if (!route) return;

      // Ensure route is visible
      if (!route->IsVisible())
      {
            route->SetVisible(true);
            m_pRouteListCtrl->SetItemImage(item, route->IsVisible() ? 0 : -1);
            pConfig->UpdateRoute(route);
      }

      ZoomtoRoute(route);
}

void RouteManagerDialog::OnRteReverseClick(wxCommandEvent &event)
{
      // Reverse selected route
      long item = -1;
      item = m_pRouteListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
      if (item == -1) return;

      Route *route = pRouteList->Item(m_pRouteListCtrl->GetItemData(item))->GetData();

      if (!route) return;

      wxMessageDialog ask(this, g_pRouteMan->GetRouteReverseMessage(),
                          _("Rename Waypoints?"), wxYES_NO);
      bool rename = (ask.ShowModal() == wxID_YES);

      pSelect->DeleteAllSelectableRouteSegments ( route );
      route->Reverse(rename);
      pSelect->AddAllSelectableRouteSegments ( route );

      // update column 2 - create a UpdateRouteItem(index) instead?
      wxString startend = route->m_RouteStartString;
      if (!route->m_RouteEndString.IsEmpty())
            startend.append(_(" - ") + route->m_RouteEndString);
      m_pRouteListCtrl->SetItem(item, 2,  startend);

      pConfig->UpdateRoute(route);
//       pConfig->UpdateSettings(); // NOTE done once in destructor
      cc1->Refresh();

      m_bNeedConfigFlush = true;
}

void RouteManagerDialog::OnRteExportClick(wxCommandEvent &event)
{
      // Export selected route
      long item = -1;
      item = m_pRouteListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
      if (item == -1) return;

      Route *route = pRouteList->Item(m_pRouteListCtrl->GetItemData(item))->GetData();

      if (!route) return;

      pConfig->ExportGPXRoute(this, route);
}

void RouteManagerDialog::OnRteActivateClick(wxCommandEvent &event)
{
      // Activate the selected route, unless it already is
      long item = -1;
      item = m_pRouteListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
      if (item == -1) return;

      if (m_bCtrlDown)
            MakeAllRoutesInvisible();

      Route *route = pRouteList->Item(m_pRouteListCtrl->GetItemData(item))->GetData();

      if (!route) return;

      if (!route->m_bRtIsActive)
      {
            if (!route->IsVisible())
            {
                  route->SetVisible(true);
                  m_pRouteListCtrl->SetItemImage(item, 0, 0);
            }

            ZoomtoRoute(route);

            RoutePoint *best_point = g_pRouteMan->FindBestActivatePoint(route, gLat, gLon, gCog, gSog);
            g_pRouteMan->ActivateRoute ( route, best_point );
//            g_pRouteMan->ActivateRoute(route);
      }
      else
            g_pRouteMan->DeactivateRoute();

      UpdateRouteListCtrl();

      pConfig->UpdateRoute(route);

      cc1->Refresh();

//      btnRteActivate->SetLabel(route->m_bRtIsActive ? _("Deactivate") : _("Activate"));

      m_bNeedConfigFlush = true;
}

void RouteManagerDialog::OnRteToggleVisibility(wxMouseEvent &event)
{
      wxPoint pos = event.GetPosition();
      int flags = 0;
      long clicked_index = m_pRouteListCtrl->HitTest(pos, flags);

      //    Clicking Visibility column?
      if (clicked_index > -1 && event.GetX() < m_pRouteListCtrl->GetColumnWidth(rmVISIBLE))
      {
            // Process the clicked item
            Route *route = pRouteList->Item(m_pRouteListCtrl->GetItemData(clicked_index))->GetData();
            route->SetVisible(!route->IsVisible());
            m_pRouteListCtrl->SetItemImage(clicked_index, route->IsVisible() ? 0 : -1);

            ::wxBeginBusyCursor();

            pConfig->UpdateRoute(route);
            cc1->Refresh();

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

void RouteManagerDialog::OnRteBtnLeftDown(wxMouseEvent &event)
{
      m_bCtrlDown = event.ControlDown();
      event.Skip();
}

void RouteManagerDialog::OnRteSelected(wxListEvent &event)
{
    long clicked_index = event.m_itemIndex;
    // Process the clicked item
    Route *route = pRouteList->Item(m_pRouteListCtrl->GetItemData(clicked_index))->GetData();
//    route->SetVisible(!route->IsVisible());
    m_pRouteListCtrl->SetItemImage(clicked_index, route->IsVisible() ? 0 : -1);
//    pConfig->UpdateRoute(route);
    cc1->Refresh();

    UpdateRteButtons();

}

void RouteManagerDialog::OnRteColumnClicked(wxListEvent &event)
{
      if(event.m_col == 1)
      {
            m_pRouteListCtrl->SortItems(SortRoutesOnName, (long)m_pRouteListCtrl);
            sort_route_name_dir++;
      }
      else if(event.m_col == 2)
      {
            m_pRouteListCtrl->SortItems(SortRoutesOnTo, (long)m_pRouteListCtrl);
            sort_route_to_dir++;
      }
}

void RouteManagerDialog::OnRteSendToGPSClick(wxCommandEvent &event)
{
      long item = -1;
      item = m_pRouteListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
      if (item == -1) return;

      Route *route = pRouteList->Item(m_pRouteListCtrl->GetItemData(item))->GetData();

      if (!route) return;

      SendToGpsDlg *pdlg = new SendToGpsDlg();
      pdlg->SetRoute(route);

      pdlg->Create ( NULL, -1, _( "Send To GPS..." ) );
      pdlg->ShowModal();

      delete pdlg;
}

void RouteManagerDialog::OnRteDefaultAction(wxListEvent &event)
{
      wxCommandEvent evt;
      OnRtePropertiesClick(evt);
}

void RouteManagerDialog::OnTrkDefaultAction(wxListEvent &event)
{
      wxCommandEvent evt;
      OnTrkPropertiesClick(evt);
}

void RouteManagerDialog::UpdateTrkListCtrl()
{
      // if an item was selected, make it selected again if it still exist
      long item = -1;
      item = m_pTrkListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

      long selected_id = -1;
      if (item != -1)
            selected_id = m_pTrkListCtrl->GetItemData(item);

      // Delete existing items
      m_pTrkListCtrl->DeleteAllItems();

      // then add routes to the listctrl
      RouteList::iterator it;
      int index = 0;
      for (it = (*pRouteList).begin(); it != (*pRouteList).end(); ++it, ++index)
      {
            Route *trk = (Route *)(*it);
            if (!trk->m_bIsTrack)
                  continue;

            wxListItem li;
            li.SetId(index);
            li.SetImage(trk->IsVisible() ? 0 : -1);
            li.SetData(index);
            li.SetText(_T(""));

            if (g_pActiveTrack == trk)
            {
                  wxFont font = *wxNORMAL_FONT;
                  font.SetWeight(wxFONTWEIGHT_BOLD);
                  li.SetFont(font);
            }
            long idx = m_pTrkListCtrl->InsertItem(li);

            wxString name = trk->m_RouteNameString;
            if (name.IsEmpty())
            {
                  RoutePoint *rp = trk->GetPoint(1);
                  if (rp)
                        name = rp->m_CreateTime.FormatISODate() + _T(" ") + rp->m_CreateTime.FormatISOTime();   //name = rp->m_CreateTime.Format();
                  else
                        name = _("(Unnamed Track)");
            }
            m_pTrkListCtrl->SetItem(idx, colTRKNAME, name);

            wxString len;
            len.Printf(wxT("%5.2f"), trk->m_route_length);
            m_pTrkListCtrl->SetItem(idx, colTRKLENGTH,  len);
      }

      m_pTrkListCtrl->SortItems(SortRoutesOnName, (long)m_pTrkListCtrl);

      // restore selection if possible
      // NOTE this will select a different item, if one is deleted
      // (the next route will get that index).
      if (selected_id > -1)
      {
            item = m_pTrkListCtrl->FindItem(-1, selected_id);
            m_pTrkListCtrl->SetItemState(item, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
      }

      UpdateTrkButtons();
}

void RouteManagerDialog::OnTrkSelected(wxListEvent &event)
{
    UpdateTrkButtons();
}

void RouteManagerDialog::OnTrkColumnClicked(wxListEvent &event)
{
      if(event.m_col == 1)
      {
            m_pTrkListCtrl->SortItems(SortTracksOnName, (long)m_pTrkListCtrl);
            sort_track_name_dir++;
      }
      else if(event.m_col == 2)
      {
            m_pTrkListCtrl->SortItems(SortTracksOnDistance, (long)m_pTrkListCtrl);
            sort_track_len_dir++;
      }
}

void RouteManagerDialog::UpdateTrkButtons()
{
      long item = -1;
      item = m_pTrkListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
      bool enable = (item != -1);

      btnTrkProperties->Enable(enable);
      btnTrkDelete->Enable(enable);
      btnTrkExport->Enable(enable);
      btnTrkRouteFromTrack->Enable(enable);
}

void RouteManagerDialog::OnTrkToggleVisibility(wxMouseEvent &event)
{
      wxPoint pos = event.GetPosition();
      int flags = 0;
      long clicked_index = m_pTrkListCtrl->HitTest(pos, flags);

      //    Clicking Visibility column?
      if (clicked_index > -1 && event.GetX() < m_pTrkListCtrl->GetColumnWidth(colTRKVISIBLE))
      {
            // Process the clicked item
            Route *route = pRouteList->Item(m_pTrkListCtrl->GetItemData(clicked_index))->GetData();
            route->SetVisible(!route->IsVisible());
            m_pTrkListCtrl->SetItemImage(clicked_index, route->IsVisible() ? 0 : -1);

//            pConfig->UpdateRoute(route);
            cc1->Refresh();
      }

      // Allow wx to process...
      event.Skip();
}

void RouteManagerDialog::OnTrkNewClick(wxCommandEvent &event)
{
      gFrame->TrackOff();
      gFrame->TrackOn();

      UpdateTrkListCtrl();
}

void RouteManagerDialog::OnTrkPropertiesClick(wxCommandEvent &event)
{
      // Show routeproperties dialog for selected route
      long item = -1;
      item = m_pTrkListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
      if (item == -1) return;

      Route *route = pRouteList->Item(m_pTrkListCtrl->GetItemData(item))->GetData();

      if (!route) return;

      if ( NULL == pRoutePropDialog )          // There is one global instance of the RouteProp Dialog
            pRoutePropDialog  = new RouteProp(GetParent());

      pRoutePropDialog->SetRouteAndUpdate(route);
      pRoutePropDialog->UpdateProperties();
      pRoutePropDialog->SetDialogTitle(_("Track Properties"));

      if(!pRoutePropDialog->IsShown())
            pRoutePropDialog->ShowModal();
      // track might have changed
      UpdateTrkListCtrl();

      m_bNeedConfigFlush = true;
}

void RouteManagerDialog::OnTrkDeleteClick(wxCommandEvent &event)
{

      long item = -1;
      item = m_pTrkListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
      if (item == -1) return;

      Track *track = (Track *)pRouteList->Item(m_pTrkListCtrl->GetItemData(item))->GetData();

//TODO Seth
//                    if(track == g_pActiveTrack)
//                        parent_frame->TrackOff();
      if (!track) return;

      g_pRouteMan->DeleteTrack(track);

      pConfig->DeleteConfigRoute(track);
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

void RouteManagerDialog::OnTrkExportClick(wxCommandEvent &event)
{
      // Export selected track
      long item = -1;
      item = m_pTrkListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
      if (item == -1) return;

      Route *route = pRouteList->Item(m_pTrkListCtrl->GetItemData(item))->GetData();

      if (!route) return;

      pConfig->ExportGPXRoute(this, route);
}

void RouteManagerDialog::OnTrkRouteFromTrackClick(wxCommandEvent &event)
{
      long item = -1;
      item = m_pTrkListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
      if (item == -1) return;

      Track *track = (Track *)pRouteList->Item(m_pTrkListCtrl->GetItemData(item))->GetData();

      if (!track) return;

      wxProgressDialog *pprog = new wxProgressDialog(_("OpenCPN Converting Track to Route...."),
                  _("Processing Waypoints..."), 200, NULL,
                    wxPD_AUTO_HIDE | wxPD_SMOOTH |wxPD_ELAPSED_TIME | wxPD_ESTIMATED_TIME |wxPD_REMAINING_TIME);

      ::wxBeginBusyCursor();

      Route *route = track->RouteFromTrack(pprog);

      pRouteList->Append(route);

      pprog->Update(150, _("Saving new Route..."));
      pConfig->UpdateRoute(route);
      pprog->Update(200, _("Done."));
      delete pprog;


      pRouteList->Append(route);
      cc1->Refresh();

      UpdateRouteListCtrl();

      ::wxEndBusyCursor();
}

void RouteManagerDialog::OnTrkDeleteAllClick(wxCommandEvent &event)
{
      wxMessageDialog mdlg(this, _("Are you sure you want to delete <ALL> tracks?"), wxString(_("OpenCPN Alert")),wxYES_NO  );
      if(mdlg.ShowModal() == wxID_YES)
      {
            g_pRouteMan->DeleteAllTracks();
      }

      UpdateTrkListCtrl();

      //    Also need to update the route list control, since routes and tracks share a common global list (pRouteList)
      UpdateRouteListCtrl();

      if(pRoutePropDialog)
            pRoutePropDialog->Hide();

      cc1->Refresh();

      m_bNeedConfigFlush = true;
}

void RouteManagerDialog::UpdateWptListCtrl()
{
      // if an item was selected, make it selected again if it still exist
      long item = -1;
      item = m_pWptListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

      long selected_id = -1;
      if (item != -1)
            selected_id = m_pWptListCtrl->GetItemData(item);

      m_pWptListCtrl->DeleteAllItems();

      wxRoutePointListNode *node = pWayPointMan->m_pWayPointList->GetFirst();

      int index = 0;
      while ( node )
      {
            RoutePoint *rp = node->GetData();
            if ( rp )
            {
                  if (rp->m_bIsInTrack || rp->m_bIsInRoute)
                  {
                        if(!rp->m_bKeepXRoute)
                        {
                              node = node->GetNext();
                              continue;
                        }
                  }

                  wxListItem li;
                  li.SetId(index);
//                  li.SetImage(pWayPointMan->GetIconIndex(rp->m_pbmIcon));
                  li.SetImage( rp->IsVisible() ? pWayPointMan->GetIconIndex(rp->m_pbmIcon) : 0);
                  li.SetData(rp);
                  li.SetText(_T(""));
                  long idx = m_pWptListCtrl->InsertItem(li);

                  wxString name = rp->m_MarkName;
                  if (name.IsEmpty())
                        name = _("(Unnamed Waypoint)");
                  m_pWptListCtrl->SetItem(idx, colWPTNAME, name);

                  double dst;
                  DistanceBearingMercator(rp->m_lat, rp->m_lon, gLat, gLon, NULL, &dst);
                  wxString dist;
                  dist.Printf(_T("%5.2f Nm"), dst);
                  m_pWptListCtrl->SetItem(idx, colWPTDIST,  dist);

                  index++;
            }

            node = node->GetNext();
      }

      m_pWptListCtrl->SortItems(SortWaypointsOnName, (long)m_pWptListCtrl);

      if (selected_id > -1)
      {
            item = m_pWptListCtrl->FindItem(-1, selected_id);
            m_pWptListCtrl->SetItemState(item, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
      }

      UpdateWptButtons();
}

void RouteManagerDialog::OnWptDefaultAction(wxListEvent &event)
{
      wxCommandEvent evt;
      OnWptPropertiesClick(evt);
}

void RouteManagerDialog::OnWptSelected(wxListEvent &event)
{
    UpdateWptButtons();
}

void RouteManagerDialog::OnWptColumnClicked(wxListEvent &event)
{
      if(event.m_col == 1)
      {
            m_pWptListCtrl->SortItems(SortWaypointsOnName, (long)m_pWptListCtrl);
            sort_wp_name_dir++;
      }
      else if(event.m_col == 2)
      {
            m_pWptListCtrl->SortItems(SortWaypointsOnDistance, (long)m_pWptListCtrl);
            sort_wp_len_dir++;
      }
}

void RouteManagerDialog::UpdateWptButtons()
{
      long item = -1;
      item = m_pWptListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
      bool enable = (item != -1);

      btnWptProperties->Enable(enable);
      btnWptZoomto->Enable(enable);
      btnWptDelete->Enable(enable);
      btnWptGoTo->Enable(enable);
      btnWptExport->Enable(enable);
      btnWptSendToGPS->Enable(enable);
}

void RouteManagerDialog::OnWptToggleVisibility(wxMouseEvent &event)
{
      wxPoint pos = event.GetPosition();
      int flags = 0;
      long clicked_index = m_pWptListCtrl->HitTest(pos, flags);

      //    Clicking Visibility column?
      if (clicked_index > -1 && event.GetX() < m_pWptListCtrl->GetColumnWidth(colTRKVISIBLE))
      {
            // Process the clicked item
            RoutePoint *wp = (RoutePoint *)m_pWptListCtrl->GetItemData(clicked_index);

            wp->SetVisible(!wp->IsVisible());
            m_pWptListCtrl->SetItemImage(clicked_index, wp->IsVisible() ?  pWayPointMan->GetIconIndex(wp->m_pbmIcon) : 0);

            pConfig->UpdateWayPoint(wp);

            cc1->Refresh();
      }


      // Allow wx to process...
      event.Skip();
}

void RouteManagerDialog::OnWptNewClick(wxCommandEvent &event)
{
      RoutePoint *pWP = new RoutePoint ( gLat, gLon, wxString ( _T ( "triangle" ) ), wxString ( _T ( "" ) ), NULL );
      pWP->m_bIsolatedMark = true;                      // This is an isolated mark
      pSelect->AddSelectableRoutePoint ( gLat, gLon, pWP );
      pConfig->AddNewWayPoint ( pWP, -1 );    // use auto next num
      cc1->Refresh ( false );      // Needed for MSW, why not GTK??

      if ( NULL == pMarkPropDialog )          // There is one global instance of the MarkProp Dialog
              pMarkPropDialog = new MarkProp ( GetParent() );

      pMarkPropDialog->SetRoutePoint ( pWP );
      pMarkPropDialog->UpdateProperties();

      if(!pMarkPropDialog->IsShown())
            pMarkPropDialog->ShowModal();

      // waypoint might have changed
      UpdateWptListCtrl();
}

void RouteManagerDialog::OnWptPropertiesClick(wxCommandEvent &event)
{
      long item = -1;
      item = m_pWptListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
      if (item == -1) return;

      RoutePoint *wp = (RoutePoint *)m_pWptListCtrl->GetItemData(item);

      if (!wp) return;

      if ( NULL == pMarkPropDialog )          // There is one global instance of the MarkProp Dialog
              pMarkPropDialog = new MarkProp ( GetParent() );

      pMarkPropDialog->SetRoutePoint ( wp );
      pMarkPropDialog->UpdateProperties();

      if(!pMarkPropDialog->IsShown())
            pMarkPropDialog->ShowModal();

      // waypoint might have changed
      UpdateWptListCtrl();

      m_bNeedConfigFlush = true;
}

void RouteManagerDialog::OnWptZoomtoClick(wxCommandEvent &event)
{
      long item = -1;
      item = m_pWptListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
      if (item == -1) return;

      RoutePoint *wp = (RoutePoint *)m_pWptListCtrl->GetItemData(item);

      if (!wp) return;

      cc1->ClearbFollow();
      cc1->SetViewPoint(wp->m_lat, wp->m_lon, cc1->GetVPScale(), 0, cc1->GetVPRotation(), CURRENT_RENDER);
      cc1->Refresh();
}

void RouteManagerDialog::OnWptDeleteClick(wxCommandEvent &event)
{
      long item = -1;
      item = m_pWptListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
      if (item == -1) return;

      RoutePoint *wp = (RoutePoint *)m_pWptListCtrl->GetItemData(item);

      if (!wp) return;

/* Seth: is this necessary here? Suggestion: move this to pconfig->DeleteWayPoint
      if (m_pFoundRoutePoint == pAnchorWatchPoint1) pAnchorWatchPoint1 = NULL;
      else if (m_pFoundRoutePoint == pAnchorWatchPoint2) pAnchorWatchPoint2 = NULL;
*/
//      pConfig->DeleteWayPoint ( wp );
//      pSelect->DeleteSelectablePoint ( wp, SELTYPE_ROUTEPOINT );
//      delete wp;

      pWayPointMan->DestroyWaypoint(wp);

      if(pMarkPropDialog)
      {
            pMarkPropDialog->SetRoutePoint ( NULL );
            pMarkPropDialog->UpdateProperties();
      }

      UpdateWptListCtrl();
      cc1->Refresh();
}

void RouteManagerDialog::OnWptGoToClick(wxCommandEvent &event)
{
      long item = -1;
      item = m_pWptListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
      if (item == -1) return;

      RoutePoint *wp = (RoutePoint *)m_pWptListCtrl->GetItemData(item);

      if (!wp) return;

      RoutePoint *pWP_src = new RoutePoint ( gLat, gLon, wxString ( _T ( "triangle" ) ), wxString ( _T ( "" ) ), NULL );
      pSelect->AddSelectableRoutePoint ( gLat, gLon, pWP_src );

      Route *temp_route = new Route();
      pRouteList->Append ( temp_route );

      temp_route->AddPoint(pWP_src);
      temp_route->AddPoint(wp);

      pSelect->AddSelectableRouteSegment ( gLat, gLon, wp->m_lat, wp->m_lon, pWP_src, wp, temp_route );

      wxString name = wp->m_MarkName;
      if (name.IsEmpty())
            name = _("(Unnamed Waypoint)");
      wxString rteName = _("Go to "); rteName.Append(name);
      temp_route->m_RouteNameString = rteName;
      temp_route->m_RouteStartString = _("Here");;
      temp_route->m_RouteEndString = name;
      temp_route->m_bDeleteOnArrival = true;

      if ( g_pRouteMan->GetpActiveRoute() )
        g_pRouteMan->DeactivateRoute();

      g_pRouteMan->ActivateRoute( temp_route, wp );

      UpdateRouteListCtrl();
}

void RouteManagerDialog::OnWptExportClick(wxCommandEvent &event)
{
      long item = -1;
      item = m_pWptListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
      if (item == -1) return;

      RoutePoint *wp = (RoutePoint *)m_pWptListCtrl->GetItemData(item);

      if (!wp) return;

      pConfig->ExportGPXWaypoint(this, wp);
}

void RouteManagerDialog::OnWptSendToGPSClick(wxCommandEvent &event)
{
      long item = -1;
      item = m_pWptListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
      if (item == -1) return;

      RoutePoint *wp = (RoutePoint *)m_pWptListCtrl->GetItemData(item);

      if (!wp) return;

      SendToGpsDlg *pdlg = new SendToGpsDlg();
      pdlg->SetWaypoint(wp);

      pdlg->Create ( NULL, -1, _( "Send To GPS..." ) );
      pdlg->ShowModal();

      delete pdlg;
}

void RouteManagerDialog::OnWptDeleteAllClick(wxCommandEvent &event)
{
      wxMessageDialog mdlg(this, _("Are you sure you want to delete <ALL> waypoints?"), wxString(_("OpenCPN Alert")),wxYES_NO  );
      if(mdlg.ShowModal() == wxID_YES)
      {
          pWayPointMan->DeleteAllWaypoints(false);          // only delete unused waypoints
//          m_pFoundRoutePoint = NULL;
      }

      if(pMarkPropDialog)
      {
            pMarkPropDialog->SetRoutePoint ( NULL );
            pMarkPropDialog->UpdateProperties();
      }

      UpdateWptListCtrl();
      cc1->Refresh();
}

void RouteManagerDialog::OnImportClick(wxCommandEvent &event)
{
      // Import routes
      // FIXME there is no way to instruct this function about what to import.
      // Suggest to add that!
      pConfig->ImportGPX(this);

      UpdateRouteListCtrl();
      UpdateTrkListCtrl();
      UpdateWptListCtrl();
      cc1->Refresh();
}
void RouteManagerDialog::OnExportClick(wxCommandEvent &event)
{
      pConfig->ExportGPX(this);
}

//END Event handlers


// kate: indent-width 6; space-indent on; indent-mode cstyle;

